
#include <fcntl.h>
#include <unistd.h>
#include <msgpack.h>

#include "main_loop.h"

static char *names[] = 
{
   "dt", "gyro_x", "gyro_y", "gyro_z", "acc_x", "acc_y", "acc_z", 
   "mag_x", "mag_y", "mag_z", "raw_e", "raw_n", "raw_ultra_u",
   "raw_baro_u", "yaw_sp", "pitch_sp", "roll_sp",
   "rc_valid", "rc_pitch", "rc_roll", "rc_yaw", "rc_gas", "rc_switch"
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
static float data[INPUT_VARIABLES];


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
                  data[idx] = p->via.dec;
                  break;
            
               case MSGPACK_OBJECT_POSITIVE_INTEGER:
                  data[idx] = p->via.i64;
                  break;
            
               case MSGPACK_OBJECT_NEGATIVE_INTEGER:
                  data[idx] = p->via.u64;
            }
         }
      }
      index++;
   }
}


/* a replay of previously recorded flight data */
void main_replay(char *file_name)
{
   main_init(0);
   
   msgpack_sbuffer *buffer = msgpack_sbuffer_new();
   int file = open(file_name, O_RDONLY);

   char buffer_data[1024];
   buffer->data = buffer_data;
   buffer->size = 1000;

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
         uint16_t sensor_status;
         main_step(dt, &marg_data, &gps_data, ultra_z, baro_z, voltage, channels, sensor_status, 1);
      }
   }
   close(file);
}

