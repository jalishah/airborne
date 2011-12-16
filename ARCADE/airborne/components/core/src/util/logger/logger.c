
#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <zmq.h>

#include "logger.h"
#include "log_data.pb-c.h"
#include "util.h"
#include "../../../../../../../common/scl/src/sclhelper.h"
#include "../config/config.h"


static void *socket = NULL;
static int loglevel;
static int details;


static config_t options[] =
{
   {"level", &loglevel},
   {"details", &details},
   {NULL, NULL}
};


int logger_open(void)
{
   ASSERT_NULL(socket);

   config_apply("logger", options);
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
   ASSERT_NOT_NULL(format);

   if (level <= (unsigned int)loglevel)
   {
      LogData log_data = LOG_DATA__INIT;

      /* fill log_data scalars #1: */
      log_data.level = level;
      log_data.file = file;
      log_data.line = line;
      log_data.details = (unsigned int)details;

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
      ASSERT_NOT_NULL(buffer);

      log_data__pack(&log_data, buffer);
      scl_send_dynamic(socket, buffer, log_data_len, ZMQ_NOBLOCK);
   }
}


int logger_close(void)
{
   ASSERT_NOT_NULL(socket);
   return zmq_close(socket);
}

