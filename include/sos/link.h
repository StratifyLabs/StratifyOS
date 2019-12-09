/* Copyright 2011-2017 Tyler Gilbert;
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

#ifndef DEV_LINK_H_
#define DEV_LINK_H_


#include <time.h>

#include "sos/dev/appfs.h"
#include "sos/dev/adc.h"
#include "sos/dev/bootloader.h"
#include "sos/dev/core.h"
#include "sos/dev/pio.h"
#include "sos/dev/dac.h"
#include "sos/dev/eint.h"
#include "sos/dev/flash.h"
#include "sos/dev/i2c.h"
#include "sos/dev/pwm.h"
#include "sos/dev/qei.h"
#include "sos/dev/rtc.h"
#include "sos/dev/spi.h"
#include "sos/dev/tmr.h"
#include "sos/dev/uart.h"
#include "sos/dev/usb.h"
#include "sos/dev/sys.h"

#include "link/transport.h"
#include "link/types.h"

#if defined( __cplusplus )
extern "C" {
#endif



extern int link_errno;


#include "link/commands.h"


#define LINK_INTERRUPT_ENDPOINT_SIZE 32
#define LINK_BULK_ENDPOINT_SIZE 64

#define LINK_ENDPOINT_DATA_SIZE (LINK_BULK_ENDPOINT_SIZE)


#define LINK_VID 0x20A0
#define LINK_PID 0x413B

void link_load_default_driver(link_transport_mdriver_t * driver);


int link_init();
void link_exit();

int link_connect(link_transport_mdriver_t * driver, const char * sn);
int link_ping(link_transport_mdriver_t * driver, const char * name, int is_keep_open, int is_legacy);
int link_disconnect(link_transport_mdriver_t * driver /*! The device to close */);
char * link_new_device_list(link_transport_mdriver_t * driver, int max);
void link_del_device_list(char * sn_list /*! The list to free */);
char * link_device_list_entry(char * list, int entry);
int link_get_err();
void link_set_debug(int debug_level);


enum {
	LINK_DEBUG_FATAL,
	LINK_DEBUG_CRITICAL,
	LINK_DEBUG_ERROR,
	LINK_DEBUG_WARNING,
	LINK_DEBUG_INFO,
	LINK_DEBUG_MESSAGE,
	LINK_DEBUG_DEBUG
};

typedef struct {
	char msg[1024];
	int type;
	char file[256];
	char function[256];
	int line;
} link_debug_context_t;

void link_set_debug_callback(void (*write_callback)(link_debug_context_t*));


int link_open(link_transport_mdriver_t * driver, const char * path, int flags, ...);
int link_ioctl(link_transport_mdriver_t * driver, int fildes, int request, ...);
int link_read(link_transport_mdriver_t * driver, int fildes, void * buf, int nbyte);
int link_write(link_transport_mdriver_t * driver, int fildes, const void * buf, int nbyte);
int link_close(link_transport_mdriver_t * driver, int fildes);
int link_unlink(link_transport_mdriver_t * driver /*! Device handle */, const char * path /*! The full path to the file to delete */);
int link_lseek(link_transport_mdriver_t * driver, int fildes, s32 offset, int whence);

//For files only
int link_stat(link_transport_mdriver_t * driver, const char * path, struct link_stat * buf);
int link_fstat(link_transport_mdriver_t * driver, int fildes, struct link_stat * buf);
int link_mkdir(link_transport_mdriver_t * driver, const char * path, link_mode_t mode);
int link_rmdir(link_transport_mdriver_t * driver, const char * path);
int link_opendir(link_transport_mdriver_t * driver, const char * dirname);
int link_closedir(link_transport_mdriver_t * driver, int dirp);
int link_readdir_r(link_transport_mdriver_t * driver, int dirp, struct link_dirent * entry, struct link_dirent ** result);
int link_mkfs(link_transport_mdriver_t * driver, const char * path);
int link_exec(link_transport_mdriver_t * driver, const char * file);
int link_symlink(link_transport_mdriver_t * driver, const char * old_path, const char * new_path);
int link_rename(link_transport_mdriver_t * driver, const char * old_path, const char * new_path);
int link_chown(link_transport_mdriver_t * driver, const char * path, int owner, int group);
int link_chmod(link_transport_mdriver_t * driver, const char * path, int mode);
int link_settime(link_transport_mdriver_t * driver, struct link_tm * t);
int link_gettime(link_transport_mdriver_t * driver, struct link_tm * t);

int link_kill_pid(link_transport_mdriver_t * driver, int pid, int signo);
int link_get_sys_info(link_transport_mdriver_t * driver, sys_info_t * sys_info);

int link_isbootloader(link_transport_mdriver_t * driver);
int link_bootloader_attr(link_transport_mdriver_t * driver, bootloader_attr_t * attr, u32 id);

int link_isbootloader_legacy(link_transport_mdriver_t * driver);
int link_bootloader_attr_legacy(link_transport_mdriver_t * driver, bootloader_attr_t * attr, u32 id);

int link_readserialno(link_transport_mdriver_t * driver, char * serialno, int len);
int link_reset(link_transport_mdriver_t * driver);

int link_resetbootloader(link_transport_mdriver_t * driver);
int link_readflash(link_transport_mdriver_t * driver, int addr, void * buf, int nbyte);
int link_writeflash(link_transport_mdriver_t * driver, int addr, const void * buf, int nbyte);
int link_eraseflash(link_transport_mdriver_t * driver);


#if defined( __cplusplus )
}
#endif


#endif /* DEV_LINK_H_ */

/*! @} */
