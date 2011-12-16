
/*
 * global logger implementation
 */

#ifndef LOGGER_H
#define LOGGER_H


#include <string.h>
#include <stdio.h>


typedef enum
{
   LL_ERROR,
   LL_WARNING,
   LL_INFO,
   LL_DEBUG,

   __LL_COUNT /* don't use me! */
}
loglevel_t;


/*
 * log message "printf"-like using a cetain debug level
 */
#define LOG(level, format, ...) logger_write(__FILE__, level, __LINE__, format, ## __VA_ARGS__)


int logger_open(void);

void logger_write(char *file, loglevel_t level, unsigned int line, char *format, ...);

int logger_close(void);


#endif /* LOGGER_H */

