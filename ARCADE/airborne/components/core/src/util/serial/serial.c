/*
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "serial.h"


static pthread_mutex_t write_mutex = PTHREAD_MUTEX_INITIALIZER;


int serial_open2(serialport_t *port, const char *path, int baudrate, unsigned int iflag, unsigned int lflag, unsigned int cflag)
{
   struct termios new_options;
   port->path = path;
   port->handle = open(path, O_RDWR | O_EXCL | O_NOCTTY /*| O_NONBLOCK*/);
   if (port->handle == -1)
   {
      return -1;
   }
   else
   {
      (void)tcgetattr(port->handle, &port->orig_options);

      new_options.c_cflag = CREAD | CLOCAL | CS8 | cflag;
      cfsetospeed(&new_options, baudrate);
      cfsetispeed(&new_options, baudrate);
      new_options.c_oflag = 0;
      new_options.c_iflag = iflag;
      new_options.c_lflag = lflag;

      new_options.c_cc[VINTR]    = 0;     /* Ctrl-c */
      new_options.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
      new_options.c_cc[VERASE]   = 0;     /* del */
      new_options.c_cc[VKILL]    = 0;     /* @ */
      new_options.c_cc[VEOF]     = (iflag == 0) ? 0 : 4;     /* Ctrl-d */
      new_options.c_cc[VTIME]    = 0;     /* inter-character timer unused */
      new_options.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
      new_options.c_cc[VSWTC]    = 0;     /* '\0' */
      new_options.c_cc[VSTART]   = 0;     /* Ctrl-q */
      new_options.c_cc[VSTOP]    = 0;     /* Ctrl-s */
      new_options.c_cc[VSUSP]    = 0;     /* Ctrl-z */
      new_options.c_cc[VEOL]     = 0;     /* '\0' */
      new_options.c_cc[VREPRINT] = 0;     /* Ctrl-r */
      new_options.c_cc[VDISCARD] = 0;     /* Ctrl-u */
      new_options.c_cc[VWERASE]  = 0;     /* Ctrl-w */
      new_options.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
      new_options.c_cc[VEOL2]    = 0;     /* '\0' */

      (void)tcsetattr(port->handle, TCSANOW, &new_options);
      return 0;
   }
}



int serial_open(serialport_t *port, const char *path, int baudrate, unsigned int iflag, unsigned int lflag, unsigned int cflag)
{
   struct termios new_options;
   port->path = path;
   port->handle = open(path, O_RDWR | O_EXCL | O_NOCTTY);
   if (port->handle == -1)
   {
      return -1;
   }
   else
   {
      (void)tcgetattr(port->handle, &port->orig_options);

      new_options.c_cflag = CREAD | CLOCAL | CS8 | cflag;
      cfsetospeed(&new_options, baudrate);
      cfsetispeed(&new_options, baudrate);
      new_options.c_oflag = 0;
      new_options.c_iflag = iflag;
      new_options.c_lflag = lflag;

      new_options.c_cc[VINTR]    = 0;     /* Ctrl-c */
      new_options.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
      new_options.c_cc[VERASE]   = 0;     /* del */
      new_options.c_cc[VKILL]    = 0;     /* @ */
      new_options.c_cc[VEOF]     = (iflag == 0) ? 0 : 4;     /* Ctrl-d */
      new_options.c_cc[VTIME]    = 0;     /* inter-character timer unused */
      new_options.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
      new_options.c_cc[VSWTC]    = 0;     /* '\0' */
      new_options.c_cc[VSTART]   = 0;     /* Ctrl-q */
      new_options.c_cc[VSTOP]    = 0;     /* Ctrl-s */
      new_options.c_cc[VSUSP]    = 0;     /* Ctrl-z */
      new_options.c_cc[VEOL]     = 0;     /* '\0' */
      new_options.c_cc[VREPRINT] = 0;     /* Ctrl-r */
      new_options.c_cc[VDISCARD] = 0;     /* Ctrl-u */
      new_options.c_cc[VWERASE]  = 0;     /* Ctrl-w */
      new_options.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
      new_options.c_cc[VEOL2]    = 0;     /* '\0' */

      (void)tcsetattr(port->handle, TCSANOW, &new_options);
      return 0;
   }
}


int serial_read_char(const serialport_t *port)
{
   unsigned char buffer;
   int ret;
   if ((ret = read(port->handle, &buffer, 1)) <= 0)
   {
      return -1;
   }
   return buffer;
}


int serial_read_buffer(char *buffer, int buf_size, const serialport_t *port)
{
   return read(port->handle, buffer, buf_size);
}



int serial_read_line(char buffer[256], const serialport_t *port)
{
   return read(port->handle, buffer, 256);
}


int serial_write(const serialport_t *port, const char *buffer, unsigned int len)
{
   int err;
   pthread_mutex_lock(&write_mutex);
   err = write(port->handle, buffer, len);
   pthread_mutex_unlock(&write_mutex);
   return err;
}


int serial_write_line(const serialport_t *port, const char *buffer)
{
   return serial_write(port, buffer, strlen(buffer));
}


int serial_close(serialport_t *port)
{
   (void)tcsetattr(port->handle, TCSANOW, &port->orig_options);
   return close(port->handle);
}

