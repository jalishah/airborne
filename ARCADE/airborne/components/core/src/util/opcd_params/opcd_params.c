
/*
 * online parameter configuration client implementation
 */


#include "opcd_params.h"


#include "util.h"
#include "config.pb-c.h"
#include "../logger/logger.h"
#include "../threads/simple_thread.h"
#include "../../../../../common/scl/src/sclhelper.h"
#include "../threads/threadsafe_types.h"


static void *ctrl_socket = NULL;
static void *event_socket = NULL;


void opcd_params_init(void)
{
   ASSERT_ONCE();
   ctrl_socket = scl_get_socket("opcd_ctrl");
   ASSERT_NOT_NULL(ctrl_socket);
   event_socket = scl_get_socket("opcd_event");
   ASSERT_NOT_NULL(event_socket);
}


void opcd_params_apply(opcd_param_t *params)
{
   for (opcd_param_t *param = params;
        param->id != NULL;
        param++)
   {
      /* build and send request: */
      CtrlReq req = CTRL_REQ__INIT;
      req.type = CTRL_REQ__TYPE__GET;
      req.id = param->id;
      SCL_PACK_AND_SEND_DYNAMIC(ctrl_socket, ctrl_req, req);
      
      /* receive and parse reply: */
      CtrlRep *rep;
      SCL_RECV_AND_UNPACK_DYNAMIC(rep, ctrl_socket, ctrl_rep);
      if (rep != NULL)
      {
         if (rep->status == CTRL_REP__STATUS__OK)
         {
            Value *val = rep->pairs[0]->val;
            if (val->str_val)
            {
               threadsafe_string_init((threadsafe_string_t *)param->data, val->str_val);
            }
            else if (val->has_int_val)
            {
               threadsafe_int_init((threadsafe_int_t *)param->data, val->int_val);
            }
            else if (val->has_dbl_val)
            {
               threadsafe_float_init((threadsafe_float_t *)param->data, val->dbl_val);
            }
            else if (val->has_bool_val)
            {
               threadsafe_int_init((threadsafe_int_t *)param->data, val->int_val);
            }
            else
            {
               assert(0);   
            }
         }
         else
         {
            LOG(LL_ERROR, "could not find parameter: %s", param->id);   
            exit(1);
         }
         SCL_FREE(ctrl_rep, rep);
      }
      else
      {
         LOG(LL_ERROR, "could not communicate with opc daemon"); 
         exit(1);
      }
   }
}



#define THREAD_NAME     "opc_client"
#define THREAD_PRIORITY 1


static simple_thread_t thread;


SIMPLE_THREAD_BEGIN(thread_func)
{
   SIMPLE_THREAD_LOOP_BEGIN
   {
   }
   SIMPLE_THREAD_LOOP_END
}
SIMPLE_THREAD_END
