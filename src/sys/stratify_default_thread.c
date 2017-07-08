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

/*! \addtogroup STFY
 * @{
 */

/*! \file */

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "sos/stratify.h"
#include "sos/fs/sysfs.h"
#include "config.h"
#include "mcu/debug.h"
#include "mcu/pio.h"
#include "sos/link/link.h"

extern void * link_update(void * args);

static int init_fs();
static int startup_fs();
static void start_filesystem(void);

static void priv_check_reset_source(void * args) MCU_PRIV_EXEC_CODE;

void priv_check_reset_source(void * args){
	u32 * src = args;
	core_info_t info;
	mcu_core_getinfo(0, &info);

	if( info.o_flags & CORE_FLAG_IS_RESET_SOFTWARE ){
		mcu_priv_debug("soft rst\n");
		*src = CORE_FLAG_IS_RESET_SOFTWARE;
	} else if( info.o_flags & CORE_FLAG_IS_RESET_POR ){
		mcu_priv_debug("por rst\n");
		*src = CORE_FLAG_IS_RESET_POR;
	} else if( info.o_flags & CORE_FLAG_IS_RESET_EXTERNAL ){
		mcu_priv_debug("ext rst\n");
		*src = CORE_FLAG_IS_RESET_EXTERNAL;
	} else if( info.o_flags & CORE_FLAG_IS_RESET_WDT ){
		mcu_priv_debug("wdt rst\n");
		*src = CORE_FLAG_IS_RESET_WDT;
	} else if( info.o_flags & CORE_FLAG_IS_RESET_BOR ){
		mcu_priv_debug("bor rst\n");
		*src = CORE_FLAG_IS_RESET_BOR;
	} else if( info.o_flags & CORE_FLAG_IS_RESET_SYSTEM ){
		mcu_priv_debug("sys rst\n");
		*src = CORE_FLAG_IS_RESET_SYSTEM;
	}
}

void check_reset_source(void){
	u32 src;
	mcu_core_privcall(priv_check_reset_source, &src);
	mcu_board_event(MCU_BOARD_CONFIG_EVENT_START_INIT, 0);
}

void start_filesystem(void){
	u32 started;
	started = startup_fs();
	mcu_debug("Started Filesystem Apps %ld\n", started);
	mcu_board_event(MCU_BOARD_CONFIG_EVENT_START_FILESYSTEM, &started);
}

void * stratify_default_thread(void * arg){

	check_reset_source();

	//Initialize the file systems
	if ( init_fs() < 0 ){ mcu_board_event(MCU_BOARD_CONFIG_EVENT_CRITICAL, (void*)"init_fs"); }

	start_filesystem();

	open("/dev/rtc", O_RDWR);

	mcu_board_event(MCU_BOARD_CONFIG_EVENT_START_LINK, 0);

	link_update(arg); 	//Run the link update thread--never returns

	return NULL;
}


int init_fs(){
	int i;
	i = 0;
	while( sysfs_isterminator(&sysfs_list[i]) == false ){
		mcu_debug("init %s\n", sysfs_list[i].mount_path);
		if ( sysfs_list[i].mount( sysfs_list[i].cfg ) < 0 ){
			mcu_debug("failed to init\n");
		}
		i++;
	}
	return 0;
}

int startup_fs(){
	int i;
	i = 0;
	int started;
	int ret;
	started = 0;
	while( sysfs_isterminator(&sysfs_list[i]) == false ){
		ret = sysfs_list[i].startup( sysfs_list[i].cfg );
		if ( ret > 0){
			started += ret;
		}
		i++;
	}
	//the number of processes that have been started
	return started;
}



