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
#include <stratify/stratify.h>
#include <stratify/sysfs.h>
#include "config.h"
#include "mcu/types.h"
#include "mcu/debug.h"
#include "mcu/pio.h"
#include "stratify/link.h"

extern void gled_priv_on(void * args);
extern void gled_priv_off(void * args);
void gled_priv_error(void * args);

static void led_startup(void);
static int init_fs(void);
static int startup_fs(void);

static void priv_check_reset_source(void * args) MCU_PRIV_EXEC_CODE;

void priv_check_reset_source(void * args){
	core_attr_t attr;
	mcu_core_getattr(0, &attr);
	switch(attr.reset_type){
	case CORE_RESET_SRC_WDT:
		mcu_priv_debug("wdt rst\n");
		break;
	case CORE_RESET_SRC_POR:
		mcu_priv_debug("por rst\n");
		break;
	case CORE_RESET_SRC_BOR:
		mcu_priv_debug("bor rst\n");
		break;
	case CORE_RESET_SRC_EXTERNAL:
		mcu_priv_debug("ext rst\n");
		break;
	}
}


void * initial_thread(void * arg){

	mcu_core_privcall(priv_check_reset_source, NULL);

	//Initialize the file systems
	if ( init_fs() < 0 ){
		;
	}

	startup_fs();

	open("/dev/rtc", O_RDWR);

	usleep(100*1000);
	led_startup();
	link_update(stfy_board_config.link_transport); 	//Run the link update thread--never returns

	return NULL;
}


void led_startup(void){
	int i;
	int duty;
	const int factor = 30;
	duty = 0;
	for(i=0; i < 100; i++){
		duty = i*factor;
		mcu_core_privcall(gled_priv_on, 0);
		usleep(duty);
		mcu_core_privcall(gled_priv_off, 0);
		usleep(100*factor - duty);
	}

	for(i=0; i < 100; i++){
		duty = i*factor;
		mcu_core_privcall(gled_priv_on, 0);
		usleep(100*factor - duty);
		mcu_core_privcall(gled_priv_off, 0);
		usleep(duty);
	}
}

int init_fs(void){
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

int startup_fs(void){
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

void gled_priv_on(void * args){
	pio_attr_t attr;
	attr.mask = (1<<mcu_board_config.led.pin);
	attr.mode = PIO_MODE_OUTPUT | PIO_MODE_DIRONLY;
	mcu_pio_setattr(mcu_board_config.led.port, &attr);
	mcu_pio_clrmask(mcu_board_config.led.port, (void*)(1<<mcu_board_config.led.pin));
}

/*! \details This function turns the green LED off by setting the line to high impedance.
 *
 */
void gled_priv_off(void * args){
	pio_attr_t attr;
	attr.mode = PIO_MODE_INPUT | PIO_MODE_DIRONLY;
	attr.mask = (1<<mcu_board_config.led.pin);
	if( mcu_pio_setattr(mcu_board_config.led.port, &attr) < 0 ){
		mcu_debug("failed to setattr\n");
	}
}

