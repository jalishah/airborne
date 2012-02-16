
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include <math.h>

#include <threadsafe_types.h>
#include <opcd_params.h>
#include <serial.h>
#include <gps_data.pb-c.h>
#include <sclhelper.h>
#include <daemon.h>

#include "nmea/nmea.h"


#define TIME_STR_LEN 20


static char running = 1;


static void generate_time_str(char str[TIME_STR_LEN], nmeaTIME *time)
{
   struct tm tm_time;
   tm_time.tm_year = time->year;
   tm_time.tm_mon = time->mon;
   tm_time.tm_mday = time->day;
   tm_time.tm_hour = time->hour;
   tm_time.tm_min = time->min;
   tm_time.tm_sec = time->sec;
   tm_time.tm_isdst = -1;

   if (mktime(&tm_time) == -1)
   {
      str[0] = '\0';
   }
   else
   {
      strftime(str, TIME_STR_LEN, "%Y-%m-%d %H:%M:%S", &tm_time);
   }
}


static double convert(double val)
{
   double deg = ((int)(val / 100.0));
   val = deg + (val - deg * 100.0) / 60.0;
   return val;
}


void _main(int argc, char *argv[])
{
   (void)argc;
   (void)argv;

   if (scl_init("gps_sensor") != 0)
   {
      syslog(LOG_CRIT, "could not init scl module");
      exit(EXIT_FAILURE);
   }

   void *gps_socket = scl_get_socket("gps");
   if (gps_socket == NULL)
   {
      syslog(LOG_CRIT, "could not get scl gate");   
      exit(EXIT_FAILURE);
   }
   int64_t hwm = 1;
   zmq_setsockopt(gps_socket, ZMQ_HWM, &hwm, sizeof(hwm));

   char *serial_path = "/dev/ttyACM0";
   tsint_t serial_speed;
   tsint_t min_sats;
   
   opcd_param_t params[] =
   {
      {"serial_path", &serial_path},
      {"serial_speed", &serial_speed},
      {"min_sats", &min_sats},
      OPCD_PARAMS_END
   };
   
   opcd_params_init("sensors.gps.", 0);
   opcd_params_apply("", params);
   
   serialport_t port;
   serial_open(&port, serial_path, tsint_get(&serial_speed), 0, 0, 0);

   nmeaPARSER parser;
   nmea_parser_init(&parser);

   nmeaINFO info;
   nmea_zero_INFO(&info);
   int time_set = 0;
   int smask = 0; /* global smask collects all sentences and is never reset,
                     in contrast to info.smask */
   while (running)
   {
      /* assemble NMEA frame: */
      char buffer[1024];
      int pos = 0;
      int c;
      do
      {
         c = serial_read_char(&port);
         if (c > 0)
         {
            buffer[pos++] = c;
         }
      }
      while (c != '\n');
      
      /* parse NMEA frame: */
      info.smask = 0; /* reset mask in order to distinguish between received nmea sentences */
      if (nmea_parse(&parser, buffer, pos, &info) == 1)
      {
         smask |= info.smask;
         if (   (info.smask & GPGGA) /* check for new position update */
             && (smask & (GPGSA | GPRMC))) /* go sure that we collect all sentences for first output*/
         {
            GpsData gps_data = GPS_DATA__INIT;
            
            /* set general data: */
            char time_str[TIME_STR_LEN];
            generate_time_str(time_str, &info.utc);
            gps_data.fix = 0;
            gps_data.time = time_str;

            /* set system time to gps time once: */
            if (!time_set)
            {
               char shell_date_cmd[TIME_STR_LEN + 8];
               sprintf(shell_date_cmd, "date -s \"%s\"", time_str);
               system(shell_date_cmd);
               time_set = 1;   
            }
            
            /* set position data if a minimum of satellites is seen: */
            if (info.satinfo.inuse >= tsint_get(&min_sats))
            {
               /* set data for 2d fix: */
               if (info.fix >= 2)
               {
                  gps_data.fix = 2;
                  PB_SET(gps_data, hdop, info.HDOP);
                  PB_SET(gps_data, vdop, info.VDOP);
                  PB_SET(gps_data, lat, convert(info.lat));
                  PB_SET(gps_data, lon, convert(info.lon));
                  PB_SET(gps_data, sats, info.satinfo.inuse);
                  PB_SET(gps_data, course, info.direction);
                  PB_SET(gps_data, speed, info.speed);
               }
              
               /* set data for 3d fix: */
               if (info.fix == 3)
               {
                  gps_data.fix = 3;
                  PB_SET(gps_data, alt, info.elv);
               }
            }

            /* add satellit info: */
            int i;
            gps_data.n_satinfo = info.satinfo.inview;
            SatInfo **satinfo = malloc(gps_data.n_satinfo * sizeof(SatInfo *));
            for (i = 0; i < gps_data.n_satinfo; i++)
            {
               /* fill SatInfo structure: */
               nmeaSATELLITE *nmea_satinfo = &info.satinfo.sat[i];
               satinfo[i] = malloc(gps_data.n_satinfo * sizeof(SatInfo));
               sat_info__init(satinfo[i]);
               satinfo[i]->id = nmea_satinfo->id;
               satinfo[i]->in_use = nmea_satinfo->in_use;
               satinfo[i]->elv = nmea_satinfo->elv;
               satinfo[i]->azimuth = nmea_satinfo->azimuth;
               satinfo[i]->sig = nmea_satinfo->sig;
            }
            gps_data.satinfo = satinfo;

            /* send the data: */
            SCL_PACK_AND_SEND_DYNAMIC(gps_socket, gps_data, gps_data);
            
            /* free allocated memory: */
            for (i = 0; i < gps_data.n_satinfo; i++)
            {
               free(satinfo[i]);
            }
            free(satinfo);
         }
      }
   }

   nmea_parser_destroy(&parser);
   scl_close(gps_socket);
   scl_term();
}


void _cleanup(void)
{
   running = 0;
}


int main(int argc, char *argv[])
{
   daemonize("/var/run/gps_sensor.pid", _main, _cleanup, argc, argv);
   return 0;
}


