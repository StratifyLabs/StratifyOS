/* Copyright 2011; 
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
 * Please see http://www.coactionos.com/license.html for
 * licensing information.
 */


#ifndef LINK_LOCAL_H_
#define LINK_LOCAL_H_

//#include "config.h"
#include <stddef.h>

#include "stratify/usb_dev.h"

int link_phyopen(void);
int link_phyread(void *buf, int nbyte);
int link_phywrite(void *buf, int nbyte);
int link_phyclose(void);


#endif /* LINK_LOCAL_H_ */
