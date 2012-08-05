
/*
 * file: quad.c
 *
 * author: Alexander Barth
 *         Benjamin Jahn
 *         Tobias Simon, Ilmenau University of Technology
 */


#include <malloc.h>

#include "coupling.h"
#include "platform.h"
#include "../hardware/drivers/scl_gps/scl_gps.h"


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


coupling_t *quad_coupling(void)
{
   const float imtx1 = 1.0f / (4.0f * F_C);
   const float imtx2 = 1.0f / (2.0f * F_C * CTRL_L);
   const float imtx3 = 1.0f / (4.0f * F_D);

   float matrix[N_FORCES * N_MOTORS] =
   {          /* gas   pitch    roll     yaw */
      /* m0 */ imtx1,  imtx2,    0.0,  imtx3,
      /* m1 */ imtx1, -imtx2,    0.0,  imtx3,
      /* m2 */ imtx1,    0.0, -imtx2, -imtx3,
      /* m3 */ imtx1,    0.0,  imtx2, -imtx3
   };

   return coupling_create(N_MOTORS, matrix);
}


platform_t *quadro_create(void)
{
   platform_t *plat = platform_create();
   plat->gps = malloc(sizeof(gps_t));
   plat->gps->init = scl_gps_init;
   plat->gps->read = scl_gps_read;
   return plat;
}

