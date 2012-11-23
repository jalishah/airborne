

#include <sclhelper.h>
#include <cal.pb-c.h>
#include <util.h>

#include "calpub.h"


/* mon data emitter thread: */
static void *cal_socket = NULL;

#define INT_SCALE (1000)
 

void calpub_init(void)
{
   ASSERT_ONCE();
   cal_socket = scl_get_socket("cal");
   ASSERT_NOT_NULL(cal_socket);
}


void calpub_send(marg_data_t *marg_data)
{
   CalData cal_data = CAL_DATA__INIT;
   cal_data.ax = (int)marg_data->acc.x * INT_SCALE;
   cal_data.ay = (int)marg_data->acc.y * INT_SCALE;
   cal_data.az = (int)marg_data->acc.z * INT_SCALE;
   cal_data.mx = (int)marg_data->mag.x * INT_SCALE;
   cal_data.my = (int)marg_data->mag.y * INT_SCALE;
   cal_data.mz = (int)marg_data->mag.z * INT_SCALE;
   SCL_PACK_AND_SEND_DYNAMIC(cal_socket, cal_data, cal_data);
}

