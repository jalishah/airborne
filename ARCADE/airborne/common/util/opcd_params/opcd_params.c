
/*
 * opcd params implementation
 * author: Tobias Simon, Ilmenau University of Technology
 */


#include <glib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <util.h>
#include <opcd.pb-c.h>
#include <simple_thread.h>
#include <sclhelper.h>
#include <threadsafe_types.h>

#include "opcd_params.h"


#define THREAD_NAME     "opcd_event_handler"
#define THREAD_PRIORITY 1


static char *prefix = NULL;
static void *ctrl_socket = NULL;
static void *event_socket = NULL;
static GHashTable *params_ht = NULL;
static simple_thread_t thread;


typedef enum
{
   TYPE_STR,
   TYPE_INT,
   TYPE_FLOAT,
   TYPE_BOOL
}
var_type_t;


typedef struct
{
   var_type_t type;
   void *data;
}
ht_entry_t;


static char *append_str(char *prefix, char *suffix)
{
   char *str = malloc(strlen(prefix) + strlen(suffix) + 1);
   strcpy(str, prefix);
   strcat(str, suffix);
   return str;
}


static var_type_t get_data_type(Value *val)
{
   var_type_t type;

   if (val->str_val)
   {
      type = TYPE_STR;
   }
   else if (val->has_int_val)
   {
      type = TYPE_INT;
   }
   else if (val->has_dbl_val)
   {
      type = TYPE_FLOAT;
   }
   else 
   {
      assert(val->has_bool_val);
      type = TYPE_BOOL;
   }

   return type;
}


static void init_param(Value *val, char *id, void *data)
{
   var_type_t type = get_data_type(val);
   
   /* copy initial data: */
   switch (type)
   {
      case TYPE_STR:
      {
         /* NOTE: strings are not updated online */
         char *mem = malloc(strlen(val->str_val) + 1);
         strcpy(mem, val->str_val);
         *(char **)data = mem;
         break;
      }
      case TYPE_INT:
      {
         tsint_init((tsint_t *)data, val->int_val);
         break;
      }
      case TYPE_FLOAT:
      {
         tsfloat_init((tsfloat_t *)data, val->dbl_val);
         break;
      }
      case TYPE_BOOL:
      {
         tsint_init((tsint_t *)data, val->bool_val);
         break;
      }
   }

   /* insert param into hash table for later use in event handler thread: */
   if (g_hash_table_lookup(params_ht, id) != NULL)
   {
      printf("parameter already registered: %s", id);
      exit(1);
   }
   ht_entry_t *entry = malloc(sizeof(ht_entry_t));
   entry->type = type;
   entry->data = data;
   g_hash_table_insert(params_ht, id, entry);
}


static void update_param(void *data, Pair *pair)
{
   Value *val = pair->val;
   var_type_t type = get_data_type(val);

   switch (type)
   {
      case TYPE_STR:
      {
         printf("not going to update string parameter %s", pair->id);
         break;
      }
      case TYPE_INT:
      {
         tsint_set((tsint_t *)data, val->int_val);
         break;
      }
      case TYPE_FLOAT:
      {
         tsfloat_set((tsfloat_t *)data, val->dbl_val);
         break;
      }
      case TYPE_BOOL:
      {
         tsint_set((tsint_t *)data, val->int_val);
         break;
      }
   }
}


void opcd_params_apply(char *_prefix, opcd_param_t *params)
{
   for (opcd_param_t *param = params;
        param->id != NULL;
        param++)
   {
      /* build and send request: */
      CtrlReq req = CTRL_REQ__INIT;
      req.type = CTRL_REQ__TYPE__GET;
      char *prefixes = append_str(prefix, _prefix);
      char *full_id = append_str(prefixes, param->id);
      req.id = full_id;
      free(prefixes);
      SCL_PACK_AND_SEND_DYNAMIC(ctrl_socket, ctrl_req, req);

      /* receive and parse reply: */
      CtrlRep *rep;
      SCL_RECV_AND_UNPACK_DYNAMIC(rep, ctrl_socket, ctrl_rep);
      if (rep != NULL)
      {
         if (rep->status == CTRL_REP__STATUS__OK)
         {
            /* NOTE: hash table takes care of full_id memory */
            init_param(rep->pairs[0]->val, full_id, param->data);
         }
         else
         {
            printf("could not find parameter: %s", req.id);   
            exit(1);
         }
         SCL_FREE(ctrl_rep, rep);
      }
      else
      {
         printf("could not communicate with opcd"); 
         exit(1);
      }
   }
}


void opcd_float_param_set(char *id, float val)
{
   /* build and send request: */
   CtrlReq req = CTRL_REQ__INIT;
   req.type = CTRL_REQ__TYPE__SET;
   req.id = id;
   req.val = malloc(sizeof(Value));
   value__init(req.val);
   req.val->has_dbl_val = 1;
   req.val->dbl_val = val;
   SCL_PACK_AND_SEND_DYNAMIC(ctrl_socket, ctrl_req, req);
   free(req.val);

   /* receive and parse reply: */
   CtrlRep *rep;
   SCL_RECV_AND_UNPACK_DYNAMIC(rep, ctrl_socket, ctrl_rep);
   if (rep != NULL)
   {
      if (rep->status != CTRL_REP__STATUS__OK)
      {
         printf("could not override or find float parameter: %s", req.id);   
      }
      SCL_FREE(ctrl_rep, rep);
   }
   else
   {
      printf("could not communicate with opcd"); 
   }
}


SIMPLE_THREAD_BEGIN(thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
      Pair *pair;
      SCL_RECV_AND_UNPACK_DYNAMIC(pair, event_socket, pair);
      if (pair != NULL)
      {
         ht_entry_t *entry = (ht_entry_t *)g_hash_table_lookup(params_ht, pair->id);
         if (entry != NULL)
         {
            update_param(entry->data, pair);
         }
         SCL_FREE(pair, pair);
      }
      else
      {
         printf("could not receive event from opcd");  
         sleep(1);
      }
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END


void opcd_params_init(char *_prefix, int enable_events)
{
   ASSERT_ONCE();
   ASSERT_NOT_NULL(_prefix);
   prefix = _prefix;
   ctrl_socket = scl_get_socket("opcd_ctrl");
   ASSERT_NOT_NULL(ctrl_socket);
   params_ht = g_hash_table_new(g_str_hash, g_str_equal);
   ASSERT_NOT_NULL(params_ht);
   if (enable_events)
   {
      event_socket = scl_get_socket("opcd_event");
      ASSERT_NOT_NULL(event_socket);
      simple_thread_start(&thread, thread_func, THREAD_NAME, THREAD_PRIORITY, NULL);
   }
}

