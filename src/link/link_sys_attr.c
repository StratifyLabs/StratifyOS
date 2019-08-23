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

#include <string.h>
#include "sos/dev/sys.h"
#include "link_local.h"

static sys_info_t convert_sys_26_info(const sys_26_info_t * sys_26_info);
static sys_info_t convert_sys_23_info(const sys_23_info_t * sys_23_info, const sys_id_t * id);

int link_get_sys_info(link_transport_mdriver_t * driver, sys_info_t * sys_info){
	int sys_fd;

	sys_26_info_t sys_26_info;
	sys_23_info_t sys_23_info;


	sys_fd = link_open(driver, "/dev/sys", LINK_O_RDWR);
	if( sys_fd >= 0 ){

		memset(sys_info, 0, sizeof(sys_info_t));
		if( link_ioctl(driver, sys_fd, I_SYS_GETINFO, sys_info) < 0 ){
			//this usually means there is a version mismatch between StratifyIO and StratifyOS
			if( link_ioctl(driver, sys_fd, I_SYS_26_GETINFO, &sys_26_info) < 0 ){
				//try previous version
				if( link_ioctl(driver, sys_fd, I_SYS_23_GETINFO, &sys_23_info) < 0 ){
					return -1;
				} else {
					//converting 2.3 to 3.0
					sys_id_t sys_id;
					memset(&sys_id, 0, LINK_PATH_MAX);
					link_ioctl(driver, sys_fd, I_SYS_GETID, &sys_id);
					*sys_info = convert_sys_23_info(&sys_23_info, &sys_id);
				}
			} else {
				*sys_info = convert_sys_26_info(&sys_26_info);
			}
		}

		link_close(driver, sys_fd);
	}

	return 0;
}


sys_info_t convert_sys_23_info(const sys_23_info_t * sys_23_info, const sys_id_t * id){
	sys_info_t sys_info;
	memset(&sys_info, 0, sizeof(sys_info_t));
	memcpy(sys_info.kernel_version, sys_23_info->version, 8);
	memcpy(sys_info.sys_version, sys_23_info->sys_version, 8);
	memcpy(sys_info.arch, sys_23_info->arch, 8);
	sys_info.signature =  sys_23_info->signature;
	sys_info.security =  sys_23_info->security;
	sys_info.cpu_freq =  sys_23_info->cpu_freq;
	sys_info.sys_mem_size =  sys_23_info->cpu_freq;
	memcpy(sys_info.stdout_name, sys_23_info->stdout_name, LINK_NAME_MAX);
	memcpy(sys_info.stdin_name, sys_23_info->stdin_name, LINK_NAME_MAX);
	memcpy(sys_info.name, sys_23_info->name, LINK_NAME_MAX);
	memcpy(sys_info.id, id, LINK_PATH_MAX);
	memcpy(&sys_info.serial, &sys_23_info->serial, sizeof(mcu_sn_t));
	sys_info.o_flags =  sys_23_info->o_flags;

	return sys_info;
}

sys_info_t convert_sys_26_info(const sys_26_info_t * sys_26_info){
	sys_info_t sys_info;
	memset(&sys_info, 0, sizeof(sys_info_t));
	memcpy(&sys_info, sys_26_info, sizeof(sys_26_info_t));
	return sys_info;
}

