
/*
 * body_to_world.c
 *
 *  Created on: 22.09.2010
 *      Author: tobi
 */


#include <meschach/matrix.h>
#include <meschach/matrix2.h>
#include <math.h>

#include "body_to_world.h"
#include "util.h"


struct body_to_world
{
   VEC *body_acc_vec;
   VEC *world_acc_vec;
   MAT *intermediate_matrix;
   MAT *result_matrix;
};


void body_to_world_transform(body_to_world_t *btw, euler_angles_t *angles, body_vector_t *in, world_vector_t *out)
{
   ASSERT_NOT_NULL(btw);
   ASSERT_NOT_NULL(angles);
   ASSERT_NOT_NULL(in);
   ASSERT_NOT_NULL(out);

   /*
    * convert input to meschach vector:
    */
   btw->body_acc_vec->ve[0] = in->pitch_dir;
   btw->body_acc_vec->ve[1] = in->roll_dir;
   btw->body_acc_vec->ve[2] = in->yaw_dir;

   float theta = angles->pitch;
   float phi = angles->roll;
   float psi = angles->yaw;

   float cos_phi = cosf(phi);
   float cos_theta = cosf(theta);
   float cos_psi = cosf(psi);

   float sin_phi = sinf(phi);
   float sin_theta = sinf(theta);
   float sin_psi = sinf(psi);

   /*
       Rotation matrix from inertial frame to body frame (for computing expected sensor outputs given yaw, pitch, and roll angles)

       [ cos(psi) * cos(theta),                                   cos(theta) * sin(psi),                                  -sin(theta)            ]
       [ cos(psi) * sin(phi) * sin(theta) - cos(phi) * sin(psi),  cos(phi) * cos(psi) + sin(phi) * sin(psi) * sin(theta),  cos(theta) * sin(phi) ]
       [ sin(phi) * sin(psi) + cos(phi) * cos(psi) * sin(theta),  cos(phi) * sin(psi) * sin(theta) - cos(psi) * sin(phi),  cos(phi) * cos(theta) ]
   */

   MAT *R = btw->intermediate_matrix;
   R->me[0][0] = cos_psi * cos_theta;
   R->me[0][1] = cos_theta * sin_psi;
   R->me[0][2] = -sin_theta;

   R->me[1][0] = cos_psi * sin_phi * sin_theta - cos_phi * sin_psi;
   R->me[1][1] = cos_phi * cos_psi + sin_phi * sin_psi * sin_theta;
   R->me[1][2] = cos_theta * sin_phi;

   R->me[2][0] = sin_phi * sin_psi + cos_phi * cos_psi * sin_theta;
   R->me[2][1] = cos_phi * sin_psi * sin_theta - cos_psi * sin_phi;
   R->me[2][2] = cos_phi * cos_theta;

   m_inverse(btw->intermediate_matrix, btw->result_matrix); // TODO: optimize me out!!!

   /*
    * multiply resulting matrix with input vector:
    */
   mv_mlt(btw->result_matrix, btw->body_acc_vec, btw->world_acc_vec);

   /*
    * convert meschach vector to output:
    */
   out->x_dir = btw->world_acc_vec->ve[1];
   out->y_dir = btw->world_acc_vec->ve[0];
   out->z_dir = btw->world_acc_vec->ve[2];
}


body_to_world_t *body_to_world_create(void)
{
   body_to_world_t *btw = (body_to_world_t *)malloc(sizeof(body_to_world_t));
   btw->body_acc_vec = v_get(3);
   btw->world_acc_vec = v_get(3);
   btw->intermediate_matrix = m_get(3, 3);
   btw->result_matrix = m_get(3, 3);
   return btw;
}

