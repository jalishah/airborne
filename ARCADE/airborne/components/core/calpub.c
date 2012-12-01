

#include <opcd_params.h>
#include <sclhelper.h>
#include <threadsafe_types.h>
#include <cal.pb-c.h>
#include <util.h>

#include "calpub.h"


/* calibration data: */
static tsfloat_t acc_bias[3];
static tsfloat_t acc_scale[3];
static tsfloat_t mag_bias[3];
static tsfloat_t mag_scale[3];

/* mon data emitter thread: */
static void *cal_socket = NULL;


void calpub_init(void)
{
   ASSERT_ONCE();

   /* load calibration: */
   opcd_param_t params[] =
   {
      /* acc bias: */
      {"acc_bias_x", &acc_bias[0]},
      {"acc_bias_y", &acc_bias[1]},
      {"acc_bias_z", &acc_bias[2]},
      /* acc scale: */
      {"acc_scale_x", &acc_scale[0]},
      {"acc_scale_y", &acc_scale[1]},
      {"acc_scale_z", &acc_scale[2]},
      /* mag bias: */
      {"mag_bias_x", &mag_bias[0]},
      {"mag_bias_y", &mag_bias[1]},
      {"mag_bias_z", &mag_bias[2]},
      /* mag scale: */
      {"mag_scale_x", &mag_scale[0]},
      {"mag_scale_y", &mag_scale[1]},
      {"mag_scale_z", &mag_scale[2]},
      OPCD_PARAMS_END
   };
   opcd_params_apply("cal.", params);

   cal_socket = scl_get_socket("cal");
   ASSERT_NOT_NULL(cal_socket);
}


void acc_mag_apply_cal(vec3_t *acc, vec3_t *mag)
{
   FOR_N(i, 3)
   {
      acc->vec[i] = (acc->vec[i] - tsfloat_get(&acc_bias[i]) * tsfloat_get(&acc_scale[i]));
      mag->vec[i] = (mag->vec[i] - tsfloat_get(&mag_bias[i]) * tsfloat_get(&mag_scale[i]));
   }
}


void calpub_send(marg_data_t *marg_data)
{
   CalData cal_data = CAL_DATA__INIT;
   cal_data.ax = marg_data->acc.x;
   cal_data.ay = marg_data->acc.y;
   cal_data.az = marg_data->acc.z;
   cal_data.mx = marg_data->mag.x;
   cal_data.my = marg_data->mag.y;
   cal_data.mz = marg_data->mag.z;
   SCL_PACK_AND_SEND_DYNAMIC(cal_socket, cal_data, cal_data);
}

