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
 * along with Stratify OS.  If not, see <http://www.gnu.org/licenses/>. */


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "link_flags.h"


int link_settime(link_transport_phy_t handle, struct tm * t){
	int fd;
	int ret;
	struct link_tm ltm;

	ltm.tm_hour = t->tm_hour;
	ltm.tm_isdst = t->tm_isdst;
	ltm.tm_mday = t->tm_mday;
	ltm.tm_min = t->tm_min;
	ltm.tm_mon = t->tm_mon;
	ltm.tm_sec = t->tm_sec;
	ltm.tm_wday = t->tm_wday;
	ltm.tm_yday = t->tm_yday;
	ltm.tm_year = t->tm_year;

	link_debug(LINK_DEBUG_MESSAGE, "open rtc device");
	fd = link_open(handle, "/dev/rtc", LINK_O_RDWR);
	if( fd < 0 ){
		return -1;
	}

	link_debug(LINK_DEBUG_MESSAGE, "write time");
	ret = link_ioctl(handle,
			fd,
			I_RTC_SET,
			&ltm);

	link_debug(LINK_DEBUG_MESSAGE, "close");
	if( link_close(handle, fd) < 0 ){
		return -1;
	}

	return ret;
}


int link_gettime(link_transport_phy_t handle, struct tm * t){
	int fd;
	int ret;
	struct link_tm ltm;

	link_debug(LINK_DEBUG_MESSAGE, "Open RTC fildes");
	fd = link_open(handle, "/dev/rtc", LINK_O_RDWR);
	if( fd < 0 ){
		return -1;
	}

	ret = link_ioctl(handle,
			fd,
			I_RTC_GET,
			&ltm);
	if( ret < 0 ){
		link_error("Failed to I_RTC_GET");
		return -1;
	}

	link_debug(LINK_DEBUG_MESSAGE, "Close RTC fildes");
	if( link_close(handle, fd) < 0 ){
		link_error("failed to close");
		return -1;
	}

	link_debug(LINK_DEBUG_MESSAGE, "Translate time");
	if( ret == 0 ){
		t->tm_hour = ltm.tm_hour;
		t->tm_isdst = ltm.tm_isdst;
		t->tm_mday = ltm.tm_mday;
		t->tm_min = ltm.tm_min;
		t->tm_mon = ltm.tm_mon;
		t->tm_sec = ltm.tm_sec;
		t->tm_wday = ltm.tm_wday;
		t->tm_yday = ltm.tm_yday;
		t->tm_year = ltm.tm_year;
	}

	return ret;
}






/*! @} */
