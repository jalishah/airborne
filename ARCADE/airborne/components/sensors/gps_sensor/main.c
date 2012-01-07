
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "../../../../airborne/common/util/serial/serial.h"

#include "nmea/nmea.h"


int main(void)
{
   char *dev_path = "/dev/ttyUSB0";
   /*opcd_param_t params[] =
   {
      {"serial_port", &dev_path},
      OPCD_PARAMS_END
   };
   opcd_params_apply("sensors.gps_sensor.", params);
   */
   serialport_t port;
   serial_open(&port, dev_path, B9600, 0, 0, 0);

   nmeaPARSER parser;
   nmea_parser_init(&parser);

   nmeaINFO info;
   nmea_zero_INFO(&info);

   while (1)
   {
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
      nmea_parse(&parser, buffer, pos, &info);
      if (info.sig != 0)
      {
         printf("lat: %.10f, lon: %.10f, alt: %f, sats: %d, sig: %d, fix: %d\n", 
                info.lat, info.lon, info.elv, info.satinfo.inuse, info.sig, info.fix);
      }
   }

   nmea_parser_destroy(&parser);
   return 0;   
}

