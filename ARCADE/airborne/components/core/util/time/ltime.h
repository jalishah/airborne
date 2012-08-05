
#ifndef LTIME_H
#define LTIME_H


#include <time.h>
#include <sys/time.h>


#define NSEC_PER_MSEC 1000000L
#define USEC_PER_SEC  1000000L
#define NSEC_PER_SEC  1000000000L


struct timespec timespec_add_s(struct timespec ts, unsigned int s);

struct timespec timespec_add_ms(struct timespec ts, unsigned int ms);

struct timespec timespec_add(struct timespec a, struct timespec b);

#define TIMESPEC_ADD(dst, src, val) \
   do \
   { \
      (dst).tv_sec = (src).tv_sec + (val).tv_sec; \
      (dst).tv_nsec = (src).tv_nsec + (val).tv_nsec; \
      if ((dst).tv_nsec >= 1000000000) \
      { \
         (dst).tv_sec++; \
         (dst).tv_nsec -= 1000000000; \
      } \
   } \
   while (0)


#define TIMESPEC_SUB(dst, src, val) \
   do \
   { \
      (dst).tv_sec = (src).tv_sec - (val).tv_sec; \
      (dst).tv_nsec = (src).tv_nsec - (val).tv_nsec; \
      if ((dst).tv_nsec < 0) { \
         (dst).tv_sec--; \
         (dst).tv_nsec += 1000000000; \
      } \
   } \
   while (0)

int timespec_cmp(struct timespec *a, struct timespec *b);

#endif /* LTIME_H */

