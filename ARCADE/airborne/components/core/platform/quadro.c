
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
#include "../hardware/interfaces/voltage.h"

/* hardware includes: */
#include "../hardware/drivers/scl_gps/scl_gps.h"
#include "../hardware/drivers/rc_dsl/rc_dsl_driver.h"
#include "../hardware/drivers/holger_blmc/holger_blmc_driver.h"
#include "../hardware/bus/i2c/i2c.h"
#include "../hardware/drivers/scl_voltage/scl_voltage.h"

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


static float coupling_matrix[N_FORCES * N_MOTORS] =
{          /* gas   pitch    roll     yaw */
   /* m0 */ IMTX1,  IMTX2,    0.0,  IMTX3,
   /* m1 */ IMTX1, -IMTX2,    0.0,  IMTX3,
   /* m2 */ IMTX1,    0.0, -IMTX2, -IMTX3,
   /* m3 */ IMTX1,    0.0,  IMTX2, -IMTX3
};

                                     /* m0    m1    m2    m3 */
static uint8_t motor_addrs[N_MOTORS] = {0x29, 0x2a, 0x2b, 0x2c};
static i2c_bus_t i2c_3;




#include <stddef.h>
#include <unistd.h>


platform_t *quadro_create(void)
{
   /* create plain platform structure: */
   platform_t *plat = platform_create();

   /* initialize buses: */
   int ret = i2c_bus_open(&i2c_3, "/dev/i2c-3");
   if (ret < 0)
   {
      LOG(LL_ERROR, "could not open OMAP bus" );
      exit(1);
   }

   /* set-up motors driver: */
   coupling_t *coupling = coupling_create(N_MOTORS, coupling_matrix);
   holger_blmc_driver_init(&i2c_3, motor_addrs, coupling, N_MOTORS);
   plat->motors = motors_interface_create(N_MOTORS, holger_blmc_driver_start_motors, holger_blmc_driver_stop_motors, holger_blmc_driver_write_forces);
 
   /* set-up gps driver: */
   scl_gps_init();
   plat->gps = gps_interface_create(scl_gps_read);

   /* set-up dsl driver: */
   if (rc_dsl_driver_init() < 0)
   {
      LOG(LL_ERROR, "could not initialize dsl driver");
      exit(1);
   }
   plat->rc = rc_interface_create(rc_dsl_driver_calibrate, rc_dsl_driver_read);
   LOG(LL_INFO, "hardware initialized");

   if (scl_voltage_init() < 0)
   {
      LOG(LL_ERROR, "could not initialize voltage reader");
      exit(1);
   }
   plat->voltage = voltage_interface_create(scl_voltage_read);
   return plat;
}
