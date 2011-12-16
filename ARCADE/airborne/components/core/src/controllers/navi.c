
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "util.h"
#include "navi.h"
#include "rot2d.h"
#include "debug_data.pb-c.h"
#include "../interfaces/params.h"
#include "../util/config/config.h"
#include "../util/threads/threadsafe_types.h"
#include "../util/math/vector2d.h"



/* configurable parameters: */
static threadsafe_float_t speed_p;
static threadsafe_float_t speed_min;
static threadsafe_float_t speed_std;
static threadsafe_float_t speed_max;
static threadsafe_float_t sqrt_shift;
static threadsafe_float_t sqrt_scale;
static threadsafe_float_t square_shift;
static threadsafe_float_t pos_i;
static threadsafe_float_t pos_i_max;
static threadsafe_float_t ortho_p;


static config_t options[] =
{
   {"speed_p", &speed_p.value},
   {"sqrt_shift", &sqrt_shift.value},
   {"sqrt_scale", &sqrt_scale.value},
   {"square_shift", &square_shift},
   {"speed_min", &speed_min.value},
   {"speed_std", &speed_std.value},
   {"speed_max", &speed_max.value},
   {"pos_i", &pos_i.value},
   {"pos_i_max", &pos_i_max.value},
   {"ortho_p", &ortho_p.value},
   {NULL, NULL}
};

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
static threadsafe_float_t travel_speed;


float desired_speed(float dist)
{
   float _square_shift = threadsafe_float_get(&square_shift);
   if (dist < _square_shift)
   {
      return 0.0f;
   }
   float _sqrt_shift = threadsafe_float_get(&sqrt_shift);
   float _sqrt_scale = threadsafe_float_get(&sqrt_scale);
   float meet_dist = 1.0f / 3.0f * (4.0f * _sqrt_shift - _square_shift);
   if (dist < meet_dist)
   {
      float square_scale = _sqrt_scale / (2.0f * pow(meet_dist - _square_shift, 3.0f / 2.0f));
      return square_scale * pow(dist - _square_shift, 2.0f);
   }
   float speed = _sqrt_scale * sqrt(dist - _sqrt_shift);
   float _speed_max = threadsafe_float_get(&speed_max);
   if (speed > _speed_max)
   {
      speed = _speed_max;
   }
   return speed;
}


/*
 * allocates and initializes memory for navigation control subsystem
 */
void navigator_init(void)
{
   ASSERT_ONCE();

   threadsafe_float_init(&speed_p, 0.0f);
   threadsafe_float_init(&speed_min, 0.0f);
   threadsafe_float_init(&speed_max, 0.0f);
   threadsafe_float_init(&speed_std, 0.0f);
   threadsafe_float_init(&sqrt_shift, 0.0f);
   threadsafe_float_init(&sqrt_scale, 0.0f);
   threadsafe_float_init(&square_shift, 0.0f);
   threadsafe_float_init(&pos_i, 0.0f);
   threadsafe_float_init(&pos_i_max, 0.0f);
   threadsafe_float_init(&ortho_p, 0.0f);

   config_apply("navi", options);
   
   param_add("navi_speed_p", &speed_p);
   param_add("navi_speed_min", &speed_min);
   param_add("navi_speed_max", &speed_max);
   param_add("navi_speed_std", &speed_std);
   param_add("navi_sqrt_shift", &sqrt_shift);
   param_add("navi_sqrt_scale", &sqrt_scale);
   param_add("navi_square_shift", &square_shift);
   param_add("navi_pos_i", &pos_i);
   param_add("navi_pos_i_max", &pos_i_max);
   param_add("navi_ortho_p", &ortho_p);

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

   navigator_reset_travel_speed();
}


void navigator_reset(void)
{
   vector2d_set(&pos_err_sum, 0.0f, 0.0f);
}


void navigator_reset_travel_speed(void)
{
   threadsafe_float_set(&travel_speed, threadsafe_float_get(&speed_std));
}


int navigator_set_travel_speed(float speed)
{
   if (speed > threadsafe_float_get(&speed_max) || speed < threadsafe_float_get(&speed_min))
   {
      return -1;
   }
   threadsafe_float_set(&travel_speed, speed);
   return 0;
}


/*
 * executes navigation control subsystem
 */
void navigator_run(navi_output_t *output, const navi_input_t *input)
{
   ASSERT_NOT_NULL(output);
   ASSERT_NOT_NULL(input);

   /* set-up input vectors: */
   vector2d_set(&curr_speed, input->speed_x, input->speed_y);
   
   if (vector2d_get_x(&dest_pos) != input->dest_x ||
       vector2d_get_y(&dest_pos) != input->dest_y)
   {
      vector2d_copy(&prev_dest_pos, &dest_pos);
      vector2d_set(&dest_pos, input->dest_x, input->dest_y);
   }
   vector2d_set(&curr_pos, input->pos_x, input->pos_y);
   vector2d_sub(&pos_err, &dest_pos, &curr_pos);
 
   /* add correction for inter-setpoint trajectory */
   vector2d_sub(&setpoints_dir, &dest_pos, &prev_dest_pos);
   vector2d_orthogonal_right(&ortho_vec, &setpoints_dir);
   vector2d_project(&ortho_pos_err, &pos_err, &ortho_vec);
   vector2d_scalar_multiply(&ortho_thrust, threadsafe_float_get(&ortho_p), &ortho_pos_err);
 
   /* calculate speed setpoint vector: */
   vector2d_copy(&virt_dest_pos, &dest_pos);
   vector2d_add(&virt_dest_pos, &dest_pos, &pos_err_sum);
   vector2d_add(&virt_dest_pos, &virt_dest_pos, &ortho_thrust);
   vector2d_sub(&virt_pos_err, &virt_dest_pos, &curr_pos);
   vector2d_sub(&speed_setpoint, &virt_dest_pos, &curr_pos);
   float target_dist = vector2d_length(&speed_setpoint);

   /* caluclate error sum for "i-part" of controller,
      if sum is below pos_i_max: */
   if (vector2d_length(&pos_err_sum) < threadsafe_float_get(&pos_i_max))
   {
      float _speed_max = threadsafe_float_get(&speed_max);
      float i_weight = (_speed_max - desired_speed(target_dist)) / _speed_max;
      vector2d_scalar_multiply(&pos_err_addend, input->dt * threadsafe_float_get(&pos_i) * i_weight, &pos_err);
      vector2d_add(&pos_err_sum, &pos_err_sum, &pos_err_addend);
   }

   float speed_val = desired_speed(target_dist) * threadsafe_float_get(&travel_speed);
   vector2d_normalize(&dest_dir, &virt_pos_err);
   vector2d_scalar_multiply(&speed_setpoint, speed_val, &dest_dir);

   /* calculate controller thrust vector: */
   vector2d_sub(&speed_err, &speed_setpoint, &curr_speed);
   vector2d_scalar_multiply(&world_thrust, threadsafe_float_get(&speed_p), &speed_err);

   /* rotate according to yaw angle: */
   float in[2] = {vector2d_get_x(&world_thrust), vector2d_get_y(&world_thrust)};
   float out[2] = {0.0, 0.0};
   rot2d_calc(rot2d_context, out, in, input->yaw);

   /* fill output structure: */
   output->roll = out[0];
   output->pitch = out[1];

   //EVERY_N_TIMES(300, printf("pos err: %f\n", target_dist));
}

