

#include <string.h>
#include <math.h>

#include "orientation.h"


void quat_rot_vec(vec3_t *v_out, const vec3_t *v_in, const quat_t *quat)
{
   float r = quat->q0;
   float i = quat->q1;
   float j = quat->q2;
   float k = quat->q3;
   v_out->x = 2 * (r * v_in->z * j + i * v_in->z * k - r * v_in->y * k + i * v_in->y * j) + v_in->x * (r * r + i * i - j * j - k * k);
   v_out->y = 2 * (r * v_in->x * k + i * v_in->x * j - r * v_in->z * i + j * v_in->z * k) + v_in->y * (r * r - i * i + j * j - k * k);
   v_out->z = 2 * (r * v_in->y * i - r * v_in->x * j + i * v_in->x * k + j * v_in->y * k) + v_in->z * (r * r - i * i - j * j + k * k);
}


void quat_copy(quat_t *q_out, const quat_t *q_in)
{
   memcpy(q_out, q_in, sizeof(quat_t));   
}


static float quat_len(const quat_t *quat)
{
   float sum = 0.0f;
   int i;
   for (i = 0; i < 4; i++)
   {
      sum += quat->vec[i] * quat->vec[i];
   }
   return 1.0f / sum;
}


void quat_inv(quat_t *q_out, const quat_t *q_in)
{
   float len = quat_len(q_in);
   float table[4] = {len, -len, -len, -len};
   int i;
   for (i = 0; i < 4; i++)
   {
      q_out->vec[i] = q_in->vec[i] * table[i];
   }
}


void quat_to_euler(euler_t *euler, const quat_t *quat)
{
   float s = quat->q0;
   float x = quat->q1;
   float y = quat->q2;
   float z = quat->q3;

   float sqw = s * s;
   float sqx = x * x;
   float sqy = y * y;
   float sqz = z * z;

   euler->pitch = atan2f(2.f * (x * y + z * s), sqx - sqy - sqz + sqw);
   euler->roll = asinf(-2.f * (x * z - y * s));
   euler->yaw = atan2f(2.f * (y * z + x * s), -sqx - sqy + sqz + sqw);
}


