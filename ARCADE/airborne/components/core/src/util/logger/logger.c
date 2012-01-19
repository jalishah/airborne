
#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <zmq.h>
#include <syslog.h>

#include <sclhelper.h>
#include <threadsafe_types.h>
#include <opcd_params.h>
#include <log_data.pb-c.h>

#include "logger.h"
#include "util.h"


static void *socket = NULL;
static tsint_t loglevel;
static tsint_t details;


int logger_open(void)
{
   ASSERT_ONCE();
   
   opcd_param_t params[] =
   {
      {"level", &loglevel},
      {"details", &details},
      OPCD_PARAMS_END
   };
   opcd_params_apply("logger.", params);
   
   socket = scl_get_socket("log");
   if (socket == NULL)
   {
      return -1;
   }
   return 0;
}


void logger_write(char *file, loglevel_t level, unsigned int line, char *format, ...)
{
   ASSERT_NOT_NULL(socket);
   ASSERT_NOT_NULL(file);

   if (level <= (unsigned int)tsint_get(&loglevel))
   {
      LogData log_data = LOG_DATA__INIT;

      /* fill log_data scalars #1: */
      log_data.level = level;
      log_data.file = file;
      log_data.line = line;
      log_data.details = (unsigned int)tsint_get(&details);

      /* set-up buffer for varg-message: */
      char message_buffer[1024];
      log_data.message = message_buffer;

      /* fill log_data scalars #2: */
      va_list ap;
      va_start(ap, format);
      vsnprintf(message_buffer, sizeof(message_buffer), format, ap);
      va_end(ap);

      /* publish: */
      unsigned int log_data_len = (unsigned int)log_data__get_packed_size(&log_data);
      void *buffer = malloc(log_data_len);
      if (buffer != NULL)
      {
         log_data__pack(&log_data, buffer);
         scl_send_dynamic(socket, buffer, log_data_len, ZMQ_NOBLOCK);
      }
      else
      {
         syslog(LOG_CRIT, "malloc() failed in module logger");
      }
   }
}


int logger_close(void)
{
   ASSERT_NOT_NULL(socket);
   return zmq_close(socket);
}

