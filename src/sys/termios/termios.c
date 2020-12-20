// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md




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
