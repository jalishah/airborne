
#include <fcntl.h>
#include <unistd.h>
#include <msgpack.h>

#include "main_loop.h"


static char *names[] = 
{
   "dt",                         /* 0 */
   "gyro_x", "gyro_y", "gyro_z", /* 1 - 3 */
   "acc_x", "acc_y", "acc_z",    /* 4 - 6 */
   "mag_x", "mag_y", "mag_z"     /* 7 - 9 */
};



#define INPUT_VARIABLES (sizeof(names) / sizeof(char *))


static int get_index(char *name)
{
   int i;
   for (i = 0; i < INPUT_VARIABLES; i++)
   {
      if (strcmp(name, names[i]) == 0)
      {
         return i;
      }
   }
   return -1;
}


static int index_table[1024];
static float float_data[INPUT_VARIABLES];
static int int_data[INPUT_VARIABLES];


void handle_array(msgpack_object array, int header)
{
   int index = 0;
   msgpack_object *p = array.via.array.ptr;
   msgpack_object *const pend = array.via.array.ptr + array.via.array.size;
   for (; p < pend; ++p)
   {
      if (header)
      {
         char name[1024];
         memcpy(name, p->via.raw.ptr, p->via.raw.size);
         name[p->via.raw.size] = '\0';
         int idx = get_index(name);
         index_table[index] = idx;
      }
      else
      {
         int idx = index_table[index];
         if (idx >= 0)
         {
            switch (p->type)
            {
               case MSGPACK_OBJECT_DOUBLE:
                  float_data[idx] = p->via.dec;
                  break;
            
               case MSGPACK_OBJECT_POSITIVE_INTEGER:
                  int_data[idx] = p->via.i64;
                  break;
            
               case MSGPACK_OBJECT_NEGATIVE_INTEGER:
                  int_data[idx] = p->via.u64;
            }
         }
      }
      index++;
   }
}


/* a replay of previously recorded flight data */
void main_replay(char *file_name)
{
   main_init(1);
   
   msgpack_sbuffer *buffer = msgpack_sbuffer_new();
   int file = open(file_name, O_RDONLY);

   char buffer_data[1024];
   buffer->data = buffer_data;
   buffer->size = sizeof(buffer_data);

   msgpack_unpacker pac;
   msgpack_unpacker_init(&pac, MSGPACK_UNPACKER_INIT_BUFFER_SIZE);
   msgpack_unpacked result;
   msgpack_unpacked_init(&result);

   int c = 0;
   int len;
   DATA_DEFINITION();
   while ((len = read(file, buffer->data, buffer->size)))
   {
      msgpack_unpacker_reserve_buffer(&pac, len);
      memcpy(msgpack_unpacker_buffer(&pac), buffer->data, len);
      msgpack_unpacker_buffer_consumed(&pac, buffer->size);
      while (msgpack_unpacker_next(&pac, &result))
      {
         handle_array(result.data, c++ == 0);
         dt = float_data[0];
         memcpy(&marg_data.gyro.vec[0], &float_data[1], sizeof(float) * 3);
         memcpy(&marg_data.acc.vec[0],  &float_data[4], sizeof(float) * 3);
         memcpy(&marg_data.mag.vec[0],  &float_data[7], sizeof(float) * 3);
         uint16_t sensor_status = 0xFFFF & ~GPS_VALID;
         main_step(dt, &marg_data, &gps_data, ultra_z, baro_z, voltage, channels, sensor_status, 1);
      }
   }
   close(file);
}

