
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
   /*
    * just some shortcut definitions:
    */
   float pos_x = pos->x.pos;
   float pos_y = pos->y.pos;
   float speed_x = pos->x.speed;
   float speed_y = pos->y.speed;
   float yaw = euler->yaw;

   /* run yaw controller: */
   float yaw_err;
   float yaw_ctrl_val = yaw_ctrl_step
   (
      &yaw_err, yaw,
      0, dt
   );

   /* run z controller: */
   float z_err;
   float gas_ctrl_val = z_ctrl_step
   (
      &z_err, pos->ultra_z.pos,
      pos->baro_z.pos,
      pos->baro_z.speed, dt
   );

   /* run navi controller: */
   float navi_output[2];
   navi_input_t navi_input = 
   {
      {pos_x, pos_y}, {speed_x, speed_y},
      dt, yaw
   };
   navi_run(navi_output, &navi_input);
   
   
   float att_ctrl[2];
   float att_pos[2] = {euler->pitch, euler->roll};
   att_ctrl_step(att_ctrl, dt, att_pos, navi_output);

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
      mon_data.x_err = pos_x - navi_get_dest_x();
      mon_data.y_err = pos_y - navi_get_dest_y();
      mon_data.z_err = z_err;
      mon_data.yaw_err = yaw_err;
      mon_data.dt = dt;
      pthread_mutex_unlock(&mon_data_mutex);
   }

   out->pitch = att_ctrl[0];
   out->roll = att_ctrl[1];
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
   int64_t hwm = 1;
   zmq_setsockopt(mon_socket, ZMQ_HWM, &hwm, sizeof(hwm));

   /* create monitoring connection: */
   const struct timespec period = {0, 100 * NSEC_PER_MSEC};
   periodic_thread_start(&emitter_thread, mon_emitter, "mon_thread", 0, period, NULL);

   /* initialize controllers: */
   att_ctrl_init();
   yaw_ctrl_init();
   z_ctrl_init();
   navi_init();
}

