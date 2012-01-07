
/*
 * daemon.h - process daemonizer with pidfile support
 * Copyright (c) 2012  Tobias Simon <tobias.simon@tu-ilmenau.de>
 *
 * This file is part of the ARCADE UAV software system.
 * it was inspired by: http://www.netzmafia.de/skripten/unix/linux-daemon-howto.html
 * The original author of most of the code is Devin Watson, thanks!
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111, USA
 */

#ifndef __DAEMON_H__
#define __DAEMON_H__


typedef void (*main_func_t)(int argc, char *argv[]);
typedef void (*clean_func_t)(void);


void daemonize(char *pid_path, main_func_t main, clean_func_t clean, int argc, char *argv[]);


#endif /* __DAEMON_H__*/

