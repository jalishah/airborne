
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <util.h>
#include <debug_data.pb-c.h>
#include <opcd_params.h>
#include <threadsafe_types.h>

#include "navi.h"
#include "../geometry/rot2d.h"
#include "../geometry/vector2d.h"


/* configurable parameters: */
static tsfloat_t speed_p;
static tsfloat_t speed_min;
static tsfloat_t speed_std;
static tsfloat_t speed_max;
static tsfloat_t sqrt_shift;
static tsfloat_t sqrt_scale;
static tsfloat_t square_shift;
static tsfloat_t pos_i;
static tsfloat_t pos_i_max;
static tsfloat_t ortho_p;
static tsint_t direct_speed_ctrl;


/* vectors for use in navigation algorithm: */
static vector2d_t dest_pos;
static vector2d_t virt_dest_pos;
static vector2d_t curr_pos;
static vector2d_t curr_speed;
static vector2d_t speed_setpoint;
static vector2d_t world_thrust;
static vector2d_t speed_err;
static vector2d_t pos_err;
static vector2d_t dest_dir;
static vector2d_t pos_err_sum;
static vector2d_t pos_err_addend;
static vector2d_t prev_dest_pos;
static vector2d_t ortho_vec;
static vector2d_t ortho_pos_err;
static vector2d_t ortho_thrust;
static vector2d_t setpoints_dir;
static vector2d_t virt_pos_err;

static rot2d_context_t *rot2d_context;
/* setpoints: */
static tsfloat_t travel_speed;
static tsfloat_t dest_x; /* destination in lon direction, in m */
static tsfloat_t dest_y; /* destination in lat direction, in m */


float desired_speed(float dist)
{
   float _square_shift = tsfloat_get(&square_shift);
   if (dist < _square_shift)
   {
      return 0.0f;
   }
   float _sqrt_shift = tsfloat_get(&sqrt_shift);
   float _sqrt_scale = tsfloat_get(&sqrt_scale);
   float meet_dist = 1.0f / 3.0f * (4.0f * _sqrt_shift - _square_shift);
   if (dist < meet_dist)
   {
      float square_scale = _sqrt_scale / (2.0f * pow(meet_dist - _square_shift, 3.0f / 2.0f));
      return square_scale * pow(dist - _square_shift, 2.0f);
   }
   float speed = _sqrt_scale * sqrt(dist - _sqrt_shift);
   float _speed_max = tsfloat_get(&speed_max);
   if (speed > _speed_max)
   {
      speed = _speed_max;
   }
   return speed;
}


/*
 * allocates and initializes memory for navigation control subsystem
 */
void navi_init(void)
{
   ASSERT_ONCE();
   opcd_param_t params[] =
   {
      {"speed_p", &speed_p},
      {"sqrt_shift", &sqrt_shift},
      {"sqrt_scale", &sqrt_scale},
      {"square_shift", &square_shift},
      {"speed_min", &speed_min},
      {"speed_std", &speed_std},
      {"speed_max", &speed_max},
      {"pos_i", &pos_i},
      {"pos_i_max", &pos_i_max},
      {"ortho_p", &ortho_p},
      {"direct_speed_ctrl", &direct_speed_ctrl},
      OPCD_PARAMS_END
   };
   opcd_params_apply("controllers.navigation.", params);
   
   vector2d_alloc(&dest_pos);
   vector2d_alloc(&virt_dest_pos);
   vector2d_alloc(&curr_pos);
   vector2d_alloc(&curr_speed);
   vector2d_alloc(&speed_setpoint);
   vector2d_alloc(&world_thrust);
   vector2d_alloc(&speed_err);
   vector2d_alloc(&pos_err);
   vector2d_alloc(&dest_dir);
   vector2d_alloc(&pos_err_sum);
   vector2d_alloc(&pos_err_addend);
   vector2d_alloc(&prev_dest_pos);
   vector2d_alloc(&ortho_vec);
   vector2d_alloc(&ortho_pos_err);
   vector2d_alloc(&ortho_thrust);
   vector2d_alloc(&setpoints_dir);
   vector2d_alloc(&virt_pos_err);

   vector2d_set(&pos_err_sum, 0.0, 0.0);
   vector2d_set(&dest_pos, 0.0, 0.0);
   vector2d_set(&prev_dest_pos, 0.0, 0.0);
   rot2d_context = rot2d_create();

   tsfloat_init(&travel_speed, 0.0f);
   tsfloat_init(&dest_x, 0.0f);
   tsfloat_init(&dest_y, 0.0f);

   navi_reset_travel_speed();
}


void navi_reset(void)
{
   vector2d_set(&pos_err_sum, 0.0f, 0.0f);
}


void navi_set_dest_x(float x)
{
   tsfloat_set(&dest_x, x);
}


void navi_set_dest_y(float y)
{
   tsfloat_set(&dest_y, y);
}


float navi_get_dest_x(void)
{
   return tsfloat_get(&dest_x);
}


float navi_get_dest_y(void)
{
   return tsfloat_get(&dest_y);
}


void navi_reset_travel_speed(void)
{
   tsfloat_set(&travel_speed, tsfloat_get(&speed_std));
}


int navi_set_travel_speed(float speed)
{
   if (speed > tsfloat_get(&speed_max) || speed < tsfloat_get(&speed_min))
   {
      return -1;
   }
   tsfloat_set(&travel_speed, speed);
   return 0;
}


/*
 * executes navigation control subsystem
 */
void navi_run(navi_output_t *output, const navi_input_t *input)
{
   ASSERT_NOT_NULL(output);
   ASSERT_NOT_NULL(input);

   /* set-up input vectors: */
   vector2d_set(&curr_speed, input->speed_x, input->speed_y);
   
   float _dest_x = tsfloat_get(&dest_x);
   float _dest_y = tsfloat_get(&dest_y);

   if (vector2d_get_x(&dest_pos) != _dest_x ||
       vector2d_get_y(&dest_pos) != _dest_y)
   {
      vector2d_copy(&prev_dest_pos, &dest_pos);
      vector2d_set(&dest_pos, _dest_x, _dest_y);
   }
   vector2d_set(&curr_pos, input->pos_x, input->pos_y);
   vector2d_sub(&pos_err, &dest_pos, &curr_pos);
 
   /* add correction for inter-setpoint trajectory */
   vector2d_sub(&setpoints_dir, &dest_pos, &prev_dest_pos);
   vector2d_orthogonal_right(&ortho_vec, &setpoints_dir);
   vector2d_project(&ortho_pos_err, &pos_err, &ortho_vec);
   vector2d_scalar_multiply(&ortho_thrust, tsfloat_get(&ortho_p), &ortho_pos_err);
 
   /* calculate speed setpoint vector: */
   vector2d_copy(&virt_dest_pos, &dest_pos);
   vector2d_add(&virt_dest_pos, &dest_pos, &pos_err_sum);
   vector2d_add(&virt_dest_pos, &virt_dest_pos, &ortho_thrust);
   vector2d_sub(&virt_pos_err, &virt_dest_pos, &curr_pos);
   vector2d_sub(&speed_setpoint, &virt_dest_pos, &curr_pos);
   float target_dist = vector2d_length(&speed_setpoint);

   /* caluclate error sum for "i-part" of controller,
      if sum is below pos_i_max: */
   if (vector2d_length(&pos_err_sum) < tsfloat_get(&pos_i_max))
   {
      float _speed_max = tsfloat_get(&speed_max);
      float i_weight = (_speed_max - desired_speed(target_dist)) / _speed_max;
      vector2d_scalar_multiply(&pos_err_addend, input->dt * tsfloat_get(&pos_i) * i_weight, &pos_err);
      vector2d_add(&pos_err_sum, &pos_err_sum, &pos_err_addend);
   }

   float speed_val = desired_speed(target_dist) * tsfloat_get(&travel_speed);
   vector2d_normalize(&dest_dir, &virt_pos_err);
   vector2d_scalar_multiply(&speed_setpoint, speed_val, &dest_dir);

   /* calculate controller thrust vector: */
   if (tsint_get(&direct_speed_ctrl))
   {
      vector2d_set(&speed_setpoint, _dest_x, _dest_y);
   }
   vector2d_sub(&speed_err, &speed_setpoint, &curr_speed);
   vector2d_scalar_multiply(&world_thrust, tsfloat_get(&speed_p), &speed_err);

   /* transform thrust vector to global coordinate system: */
   float in[2] = {vector2d_get_x(&world_thrust), vector2d_get_y(&world_thrust)};
   float out[2];
   rot2d_calc(rot2d_context, out, in, input->yaw);

   /* fill output structure: */
   output->roll = out[0];
   output->pitch = out[1];
}

