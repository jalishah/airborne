
/*
 * file: quadro.c
 *
 * author: Alexander Barth
 *         Benjamin Jahn
 *         Tobias Simon, Ilmenau University of Technology
 */


#include <malloc.h>

#include "coupling.h"
#include "platform.h"

/* interface includes: */
#include "../hardware/interfaces/rc.h"
#include "../hardware/interfaces/gps.h"

/* hardware includes: */
#include "../hardware/drivers/scl_gps/scl_gps.h"
#include "../hardware/drivers/rc_dsl/rc_dsl_driver.h"


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


float coupling_matrix[N_FORCES * N_MOTORS] =
{          /* gas   pitch    roll     yaw */
   /* m0 */ IMTX1,  IMTX2,    0.0,  IMTX3,
   /* m1 */ IMTX1, -IMTX2,    0.0,  IMTX3,
   /* m2 */ IMTX1,    0.0, -IMTX2, -IMTX3,
   /* m3 */ IMTX1,    0.0,  IMTX2, -IMTX3
};


coupling_t *quadro_coupling(void)
{
   return coupling_create(N_MOTORS, coupling_matrix);
}


platform_t *quadro_create(void)
{
   platform_t *plat = platform_create();
   plat->gps = gps_interface_create(scl_gps_init, scl_gps_read);
   plat->rc = rc_interface_create(rc_dsl_driver_init, rc_dsl_driver_read);
   return plat;
}

