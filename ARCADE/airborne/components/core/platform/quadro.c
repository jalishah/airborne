
/*
 * file: quadro.c
 *
 * author: Alexander Barth
 *         Benjamin Jahn
 *         Tobias Simon, Ilmenau University of Technology
 */


#include <malloc.h>

#include <util.h>
#include "coupling.h"
#include "platform.h"
#include "../util/logger/logger.h"

/* interface includes: */
#include "../hardware/interfaces/rc.h"
#include "../hardware/interfaces/gps.h"

/* hardware includes: */
#include "../hardware/drivers/scl_gps/scl_gps.h"
#include "../hardware/drivers/rc_dsl/rc_dsl_driver.h"
#include "../hardware/drivers/holger_blmc/holger_blmc_driver.h"
#include "../hardware/bus/i2c/omap_i2c_bus.h"

/* arm length */
#define CTRL_L (0.2025f)

/* F = c * rpm ^ 2 */
#define F_C (1.5866e-007f)

/* tau = d * rpm ^ 2 */
#define F_D (4.0174e-009f)

/* number of motors we can control: */
#define N_MOTORS (4)

/* number of forces/moments we can control */
#define N_FORCES (4)


#define IMTX1 (1.0f / (4.0f * F_C))
#define IMTX2 (1.0f / (2.0f * F_C * CTRL_L))
#define IMTX3 (1.0f / (4.0f * F_D))


static const float coupling_matrix[N_FORCES * N_MOTORS] =
{          /* gas   pitch    roll     yaw */
   /* m0 */ IMTX1,  IMTX2,    0.0,  IMTX3,
   /* m1 */ IMTX1, -IMTX2,    0.0,  IMTX3,
   /* m2 */ IMTX1,    0.0, -IMTX2, -IMTX3,
   /* m3 */ IMTX1,    0.0,  IMTX2, -IMTX3
};

                                           /* m0    m1    m2    m3 */
static const uint8_t motor_addrs[N_MOTORS] = {0x29, 0x2a, 0x2b, 0x2c};


platform_t *quadro_create(void)
{
   /* create plain platform structure: */
   platform_t *plat = platform_create();
#if 0
   /* initialize buses: */
   int status = omap_i2c_bus_init();
   if (status != 0)
   {
      LOG(LL_ERROR, "could not open OMAP bus" );
      exit(1);
   }
   /* initialize actuator subsystems: */
   coupling_t *coupling = coupling_create(N_MOTORS, coupling_matrix);
   //holger_blmc_driver_init(omap_i2c_bus_get(), motor_addrs, coupling, N_MOTORS);
   /*float forces[4] = {0.0, 0.0, 0.0, 0.4};
   holger_blmc_driver_start_motors();
   while (1)
   {
      holger_blmc_driver_write_forces(forces, 16.0);
      msleep(100);
   }*/
   /* initialize sensor subsystems: */
   //plat->gps = gps_interface_create(scl_gps_init, scl_gps_read);
#endif
   rc_dsl_driver_init();
   if (rc_dsl_driver_calibrate() != 0)
   {
      printf("could not calibrate dsl\n");   
   }
   while (1)
   {
      rc_data_t data;
      rc_dsl_driver_read(&data);
      //printf("%f %f %f %f %f\n", data.gas, data.pitch, data.roll, data.yaw, data.rssi);
      sleep(1);
   }
   //plat->rc = rc_interface_create(rc_dsl_driver_init, rc_dsl_driver_read);
   return plat;
}

