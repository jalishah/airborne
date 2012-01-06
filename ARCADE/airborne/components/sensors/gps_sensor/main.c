
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
   serial_open(&port, dev_path, B57600, 0, 0, 0);

   nmeaPARSER parser;
   nmea_parser_init(&parser);

   nmeaINFO info;
   nmea_zero_INFO(&info);

   while (1)
   {
      nmeaPOS dpos;
      char buffer[1024];
      int c;
      int pos = 0;
      do
      {
         c = serial_read_char(&port);
         if (c > 0)
         {
            buffer[pos] = c;
         }
      }
      while (c != '\n');

      nmea_parse(&parser, buffer, pos, &info);
      nmea_info2pos(&info, &dpos);
      printf("lat: %f, lon: %f, sig: %d, fix: %d\n", dpos.lat, dpos.lon, info.sig, info.fix);
   }

   nmea_parser_destroy(&parser);
   return 0;   
}

