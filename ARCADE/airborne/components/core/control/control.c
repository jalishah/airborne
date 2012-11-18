
/*
 * File: control.c
 * Type: main controller interface
 * Purpose: runs multiple controllers (x,y,z,yaw) and executes attitude control
 * Design Pattern: Singleton using ctrl_init
 *
 * Responsibilities:
 *   - self-configuration through OPCD
 *   - sub-controller management
 *   - controller state management
 *
 * Author: Tobias Simon, Ilmenau University of Technology
 */


#include <stdint.h>
#include <periodic_thread.h>
#include <threadsafe_types.h>
#include <sclhelper.h>
#include <core.pb-c.h>
#include <util.h>

#include "control.h"
#include "position/navi.h"
#include "position/att_ctrl.h"
#include "position/z_ctrl.h"
#include "position/yaw_ctrl.h"
#include "xy_speed.h"
#include "../util/logger/logger.h"
#include "../util/time/ltime.h"


/* mon data emitter thread: */
static pthread_mutex_t mon_data_mutex = PTHREAD_MUTEX_INITIALIZER;
static void *mon_socket = NULL;
static MonData mon_data = MON_DATA__INIT;
static periodic_thread_t emitter_thread;


PERIODIC_THREAD_BEGIN(mon_emitter)
{
   PERIODIC_THREAD_LOOP_BEGIN
   {
      pthread_mutex_lock(&mon_data_mutex);
      SCL_PACK_AND_SEND_DYNAMIC(mon_socket, mon_data, mon_data);
      pthread_mutex_unlock(&mon_data_mutex);
   }
   PERIODIC_THREAD_LOOP_END
}
PERIODIC_THREAD_END


void ctrl_step(ctrl_out_t *out, float dt, pos_t *pos, euler_t *euler)
{
   /* run yaw controller: */
   float yaw_err;
   float yaw_ctrl_val = yaw_ctrl_step
   (
      &yaw_err, euler->yaw,
      0, dt
   );

   /* run z controller: */
   float z_err;
   float gas_ctrl_val = z_ctrl_step(&z_err, pos->ultra_z.pos,
                                    pos->baro_z.pos, pos->baro_z.speed, dt);

   /* run navi controller: */
   vec2_t speed_sp;
   int direct_speed_ctrl = 1;
   if (direct_speed_ctrl)
   {
      speed_sp.x = 0.0f;
      speed_sp.y = 0.0f;
   }
   else
   {
      vec2_t pos_vec = {pos->x.pos, pos->y.pos};
      navi_run(&speed_sp, &pos_vec, dt);
   }
   vec2_t speed_vec = {pos->x.speed, pos->y.speed};
   vec2_t pitch_roll_sp;
   xy_speed_ctrl_run(&pitch_roll_sp, &speed_sp, &speed_vec, euler->yaw);
   
   vec2_t pitch_roll_ctrl;
   vec2_t pitch_roll = {euler->pitch, euler->roll};
   att_ctrl_step(&pitch_roll_ctrl, dt, &pitch_roll, &pitch_roll_sp);

   /* set monitoring data: */
   if (pthread_mutex_trylock(&mon_data_mutex) == 0)
   {
      mon_data.pitch = euler->pitch;
      mon_data.roll = euler->roll;
      mon_data.yaw = euler->yaw;
      mon_data.x = pos->x.pos;
      mon_data.y = pos->y.pos;
      mon_data.z = pos->baro_z.pos;
      mon_data.z_ground = pos->ultra_z.pos;
      mon_data.x_speed = pos->x.speed;
      mon_data.y_speed = pos->y.speed;
      mon_data.z_speed = pos->baro_z.speed;
      mon_data.x_err = pos->x.pos - navi_get_dest_x();
      mon_data.y_err = pos->y.pos - navi_get_dest_y();
      mon_data.z_err = z_err;
      mon_data.yaw_err = yaw_err;
      mon_data.dt = dt;
      pthread_mutex_unlock(&mon_data_mutex);
   }

   out->pitch = pitch_roll_ctrl.x;
   out->roll = pitch_roll_ctrl.y;
   out->yaw = yaw_ctrl_val;
   out->gas = gas_ctrl_val;
}


void ctrl_reset(void)
{
   yaw_ctrl_reset();
   z_ctrl_reset();
   att_ctrl_reset();
   navi_reset(); // TODO: not threadsafe
}


void ctrl_init(void)
{
   ASSERT_ONCE();
   mon_socket = scl_get_socket("mon");
   ASSERT_NOT_NULL(mon_socket);

   /* create monitoring connection: */
   const struct timespec period = {0, 100 * NSEC_PER_MSEC};
   periodic_thread_start(&emitter_thread, mon_emitter, "mon_thread", 0, period, NULL);

   /* initialize controllers: */
   att_ctrl_init();
   yaw_ctrl_init();
   z_ctrl_init();
   navi_init();
}

