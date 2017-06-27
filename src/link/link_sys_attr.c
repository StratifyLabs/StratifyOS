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

#include "mcu/mcu.h"
#include "link_flags.h"

static sys_attr_t convert_sys_23_attr(const sys_23_attr_t * sys_23_attr, const sys_id_t * id);

int link_get_sys_attr(link_transport_mdriver_t * driver, sys_attr_t * sys_attr){
	int sys_fd;
	sys_fd = link_open(driver, "/dev/sys", LINK_O_RDWR);
	if( sys_fd >= 0 ){

		memset(sys_attr, 0, sizeof(sys_attr_t));
		if( link_ioctl(driver, sys_fd, I_SYS_GETATTR, sys_attr) < 0 ){
			//this usually means there is a version mismatch between StratifyIO and StratifyOS
			sys_23_attr_t sys_23_attr;
			if( link_ioctl(driver, sys_fd, I_SYS_23_GETATTR, &sys_23_attr) < 0 ){
				//unknown version
			} else {
				sys_id_t sys_id;

				memset(&sys_id, 0, LINK_PATH_MAX);
				link_ioctl(driver, sys_fd, I_SYS_GETID, &sys_id);
				*sys_attr = convert_sys_23_attr(&sys_23_attr, &sys_id);
			}
		}

		link_close(driver, sys_fd);
	}

	return 0;
}


sys_attr_t convert_sys_23_attr(const sys_23_attr_t * sys_23_attr, const sys_id_t * id){
    sys_attr_t sys_attr;
    memcpy(sys_attr.kernel_version, sys_23_attr->version, 8);
    memcpy(sys_attr.sys_version, sys_23_attr->sys_version, 8);
    memcpy(sys_attr.arch, sys_23_attr->arch, 8);
    sys_attr.signature =  sys_23_attr->signature;
    sys_attr.security =  sys_23_attr->security;
    sys_attr.cpu_freq =  sys_23_attr->cpu_freq;
    sys_attr.sys_mem_size =  sys_23_attr->cpu_freq;
    memcpy(sys_attr.stdout_name, sys_23_attr->stdout_name, LINK_NAME_MAX);
    memcpy(sys_attr.stdin_name, sys_23_attr->stdin_name, LINK_NAME_MAX);
    memcpy(sys_attr.name, sys_23_attr->name, LINK_NAME_MAX);
    memcpy(sys_attr.id, id, LINK_PATH_MAX);
    memcpy(&sys_attr.serial, &sys_23_attr->serial, sizeof(sn_t));
    sys_attr.flags =  sys_23_attr->flags;

    return sys_attr;
}
