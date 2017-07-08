/* Copyright 2011-2016 Tyler Gilbert; 
 * This file is part of Stratify OS.
 *
 * Stratify OS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Stratify OS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * 
 */



#include <errno.h>
#include <unistd.h>
#include "sos/dev/tty.h"
#include "sys/ioctl.h"


int tcgetattr(int fd, struct termios * termios_p){
	return ioctl(fd, I_TTY_GETINFO, termios_p);
}

int tcsetattr(int fd, int optional_actions, const struct termios * termios_p){
	if ( optional_actions == TCSANOW ){
		return ioctl(fd, I_TTY_SETATTR, termios_p);
	}

	errno = ENOTSUP;
	return -1;
}
