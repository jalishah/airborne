

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../../../util/types.h"
#include "gpio.h"


static const int MAX_GPIO_SYSFS_PATH_LEN = 128;


int gpio_init(int id)
{
   FILE *f;
   char path_buf[MAX_GPIO_SYSFS_PATH_LEN];

   bool_t gpio_dir_out = FALSE;
   bool_t gpio_exported = FALSE;
   sprintf(path_buf, "/sys/class/gpio/gpio%d/direction", id);
   f = fopen(path_buf, "r");
   if (f != NULL)
   {
      gpio_exported = TRUE;
      const int DIR_BUF_SIZE = 3;
      char dir_buf[DIR_BUF_SIZE];
      char *s = fgets(dir_buf, DIR_BUF_SIZE, f);
      (void)s;
      fclose(f);
      gpio_dir_out = strstr("out", dir_buf) != NULL;
   }
   else
   {
      gpio_exported = FALSE;
   }

   if (!gpio_exported)
   {
      f = fopen("/sys/class/gpio/export", "w");
      int status = fprintf(f, "%d\n", id);
      fclose(f);
      if (status < 0)
      {
         return status;
      }
      sprintf(path_buf, "/sys/class/gpio/gpio%d/direction", id);
   }

   if (!gpio_dir_out)
   {
      f = fopen(path_buf, "w");
      int status = fprintf(f, "out\n");
      fclose(f);
      if (status < 0)
      {
         return status;
      }
   }

   return 0;
}


int gpio_write(int id, int val)
{
   char buf[MAX_GPIO_SYSFS_PATH_LEN];
   sprintf(buf, "/sys/class/gpio/gpio%d/value", id);
   FILE *f = fopen(buf, "w");
   int status = fprintf(f, "%d\n", val);
   fclose(f);
   if (status < 0)
   {
      return status;
   }
   return 0;
}

