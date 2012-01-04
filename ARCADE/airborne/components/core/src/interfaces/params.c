
#include <glib.h>
#include <unistd.h>

#include "params.h"
#include "param.pb-c.h"
#include "util.h"
#include "../util/logger/logger.h"
#include "../util/threads/simple_thread.h"
#include "../../../../../../common/scl/src/sclhelper.h"


static GHashTable *params_ht;
static char *desc = NULL;


void params_init(void)
{
   ASSERT_ONCE();
   params_ht = g_hash_table_new(g_str_hash, g_str_equal);
}


int param_add(char *key, threadsafe_float_t *data)
{
   if (g_hash_table_lookup(params_ht, key) != NULL)
   {
      LOG(LL_ERROR, "ERROR: parameter already in table: %s", key);
      return -1;
   }
   g_hash_table_insert(params_ht, key, data);
   return 0;
}



static int param_set(char *key, float val)
{
   threadsafe_float_t *data = (threadsafe_float_t *)g_hash_table_lookup(params_ht, key);
   if (data == NULL)
   {
      LOG(LL_ERROR, "ERROR: could not set parameter: %s", key);
      return -1;
   }
   threadsafe_float_set(data, val);
   return 0;
}


static int param_get(char *key, float *out)
{ 
   threadsafe_float_t *data = (threadsafe_float_t *)g_hash_table_lookup(params_ht, key);
   if (data == NULL)
   {
      LOG(LL_ERROR, "ERROR: could not get parameter: %s", key);
      return -1;
   }
   *out = threadsafe_float_get(data);
   return 0;
}


static char *params_get_list(void)
{
   GHashTableIter iter;
   char *key = NULL;
   
   /* first pass: calculate buffer size */
   size_t count = 0;
   size_t max_key_len = 0;
   g_hash_table_iter_init(&iter, params_ht);
   while (g_hash_table_iter_next(&iter, (gpointer)&key, NULL))
   {
      size_t key_len = strlen(key);
      if (key_len > max_key_len)
      {
         max_key_len = key_len;   
      }
      count++;
   }
   const size_t space = 2;
   const size_t float_space = 128;
   const size_t buf_size = count * (max_key_len + space + float_space + 2 /* '\n' + '\0' */);
   
   /* allocate memory: */
   char *list = malloc(buf_size);
   list[0] = '\0';
   g_hash_table_iter_init(&iter, params_ht);
   while (g_hash_table_iter_next(&iter, (gpointer)&key, NULL))
   {
      strcat(list, key);
      for (size_t i = 0; i < max_key_len - strlen(key) + space; i++)
      {
         strcat(list, " ");
      }
      char buffer[float_space];
      float val = 0;
      param_get(key, &val);
      snprintf(buffer, float_space, "%f", val);
      strcat(list, buffer);
      strcat(list, "\n");
   }
   return list;
}


#define THREAD_NAME     "cmd_interface"
#define THREAD_PRIORITY 1


static void *socket = NULL;
static simple_thread_t thread;


SIMPLE_THREAD_BEGIN(thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
      ParamReply reply = PARAM_REPLY__INIT;
      unsigned char raw_data[1024];
      int raw_data_size = scl_recv_static(socket, raw_data, sizeof(raw_data));
      if (raw_data_size < 0)
      {
         LOG(LL_ERROR, "could not receive data");
         sleep(1);
      }
      ParamRequest *request = param_request__unpack(NULL, raw_data_size, raw_data);
      if (request == NULL)
      {
         LOG(LL_ERROR, "could not parse request");
         sleep(1);
      }
      else
      {
         reply.status = PARAM_REPLY__STATUS__OK;
         
         switch (request->type)
         {
            case PARAM_REQUEST__TYPE__GET:
               if (strlen(request->key) == 0)
               {
                  if (desc == NULL)
                  {
                     desc = params_get_list();
                  }
                  reply.desc = desc;
               }
               else
               {
                  float val;
                  if (param_get(request->key, &val) == 0)
                  {
                     reply.val = val;
                     reply.has_val = 1;
                  }
                  else
                  {
                     reply.status = PARAM_REPLY__STATUS__ERR;
                  }
               }
               break;

            case PARAM_REQUEST__TYPE__SET:
               if (param_set(request->key, request->val) != 0)
               {
                  reply.status = PARAM_REPLY__STATUS__ERR;
               }
               break;
         }
         param_request__free_unpacked(request, NULL);
      }
      SCL_PACK_AND_SEND_DYNAMIC(socket, param_reply, reply);
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END



void params_thread_start(void)
{
   ASSERT_ONCE();
   socket = scl_get_socket("params");
   simple_thread_start(&thread, thread_func, THREAD_NAME, THREAD_PRIORITY, NULL);
}

