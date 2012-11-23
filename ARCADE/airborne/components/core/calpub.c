

#include <sclhelper.h>
#include <cal.pb-c.h>
#include <util.h>

#include "calpub.h"


/* mon data emitter thread: */
static void *cal_socket = NULL;

void calpub_init(void)
{
   ASSERT_ONCE();
   cal_socket = scl_get_socket("cal");
   ASSERT_NOT_NULL(cal_socket);
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

