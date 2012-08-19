
/*
 * logger interface
 */

#ifndef __LOGGER_H__
#define __LOGGER_H__


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


/*
 * opens the logger, sets up connection using SCL
 */
int logger_open(void);


/*
 * writes format string to logger with given log level
 */
void logger_write(char *file, loglevel_t level, unsigned int line, char *format, ...);


/*
 * closes the SCL logger connection
 */
int logger_close(void);


#endif /* __LOGGER_H__ */

