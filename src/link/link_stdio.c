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


#include <mcu/mcu.h>
#include <stdbool.h>

#include "link_flags.h"



int link_open_stdio(link_transport_phy_t handle /*, const char * name */){
	return 0;
}


int link_close_stdio(link_transport_phy_t handle){
	return 0;
}

int link_read_stdout(link_transport_phy_t handle, void * buf, int nbyte){
	return link_driver()->read(handle, buf, nbyte);
}


int link_write_stdin(link_transport_phy_t handle, const void * buf, int nbyte){
	return link_driver()->write(handle, buf, nbyte);
}
