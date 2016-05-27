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

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "link_flags.h"
#include "iface/dev/sys.h"

#define MAX_TRIES 3

int link_phy_lock(link_transport_phy_t phy){
	link_transport_mdriver_t * d = (link_transport_mdriver_t*)phy;
	return d->lock(d->dev.handle);
}

int link_phy_unlock(link_transport_phy_t phy){
	link_transport_mdriver_t * d = (link_transport_mdriver_t*)phy;
	return d->unlock(d->dev.handle);
}

const link_transport_mdriver_t link_default_driver = {
		.getname = link_phy_getname,
		.lock = link_phy_lock,
		.unlock = link_phy_unlock,
		.status = link_phy_status,
		.dev.handle = LINK_PHY_OPEN_ERROR,
		.dev.open = link_phy_open,
		.dev.write = link_phy_write,
		.dev.read = link_phy_read,
		.dev.close = link_phy_close,
		.dev.flush = link_phy_flush,
		.dev.wait = link_phy_wait,
		.dev.timeout = 100
};

int link_errno;

void link_load_default_driver(link_transport_mdriver_t * driver){
	link_debug(LINK_DEBUG_MESSAGE, "Load default read driver");
	memcpy(driver, &link_default_driver, sizeof(link_transport_mdriver_t));
}


int link_init(){
	return 0;
}


void link_exit(){}

int link_disconnect(link_transport_mdriver_t * driver){
	int ret;
	ret = driver->dev.close(driver->dev.handle);
	driver->dev.handle = LINK_PHY_OPEN_ERROR;
	return ret;
}

int link_connect(link_transport_mdriver_t * driver, const char * sn){
	char name[LINK_PHY_NAME_MAX];
	char serialno[256];
	char last[LINK_PHY_NAME_MAX];
	int err;
	int len;

	memset(last, 0, LINK_PHY_NAME_MAX);

	if( sn ){
		strcpy(serialno, sn);
	} else {
		strcpy(serialno, "NONE");
	}

	link_debug(LINK_DEBUG_MESSAGE, "Connect to %s", serialno);


	while( (err = driver->getname(name, last, LINK_PHY_NAME_MAX)) == 0 ){
		//success in getting new name
		driver->dev.handle = driver->dev.open(name, 0);
		if( driver->dev.handle != LINK_PHY_OPEN_ERROR ){
			link_debug(LINK_DEBUG_MESSAGE, "Read serial number for %s", name);
			if( link_readserialno(driver, serialno, 256) == 0 ){
				//check for NULL sn, zero length sn or matching sn
				if( (sn == NULL) || (strlen(sn) == 0) || (strcmp(sn, serialno) == 0) ){
					link_debug(LINK_DEBUG_MESSAGE, "Open Anon at 0x%llX", (uint64_t)driver->dev.handle);
					return 0;
				}

				//check for half the serial number for compatibility to old serial number format
				len = strlen(sn);
				if( strcmp(&(sn[len/2]), serialno) == 0 ){
					link_debug(LINK_DEBUG_MESSAGE, "Open SN at 0x%llX", (uint64_t)driver->dev.handle);
					return 0;
				}

				len = strlen(serialno);
				if( strcmp(sn, &(serialno[len/2])) == 0 ){
					link_debug(LINK_DEBUG_MESSAGE, "Open SN at 0x%llX", (uint64_t)driver->dev.handle);
					return 0;
				}
			}
			link_debug(LINK_DEBUG_MESSAGE, "Close Handle");
			driver->dev.close(driver->dev.handle);
			driver->dev.handle = LINK_PHY_OPEN_ERROR;

		} else {
			link_error("Failed to open %s\n", name);
		}
		strcpy(last, name);
	}


	//no device was found
	link_error("Device not found");
	return -1;
}

int link_readserialno(link_transport_mdriver_t * driver, char * serialno, int len){
	link_op_t op;
	link_reply_t reply;
	int err;

	op.cmd = LINK_CMD_READSERIALNO;

	link_debug(LINK_DEBUG_MESSAGE, "Send command (%d) to read serial number on 0x%llX", op.cmd, (u64)driver->dev.handle);
	err = link_transport_masterwrite(driver, &op, sizeof(link_cmd_t));
	if ( err < 0 ){
		link_error("failed to write op");
		return link_handle_err(driver, err);
	}

	link_debug(LINK_DEBUG_MESSAGE, "Read the reply");
	//Get the reply
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		link_error("failed to read reply");
		return link_handle_err(driver, err);
	}

	link_debug(LINK_DEBUG_MESSAGE, "Read the serial number (%d bytes)", reply.err);
	if ( reply.err > 0 ){
		//Read bulk in as the size of the the new data
		if( reply.err <= len ){
			memset(serialno, 0, len);
			err = link_transport_masterread(driver, serialno, reply.err);
		} else {
			return -1;
		}
		if ( err < 0 ){
			return err;
		}

	} else {
		link_errno = reply.err_number;
	}
	return 0;
}


int link_mkfs(link_transport_mdriver_t * driver, const char * path){
	link_op_t op;
	link_reply_t reply;
	int len;
	int err;

	link_debug(LINK_DEBUG_MESSAGE, "format %s", path);

	op.mkfs.cmd = LINK_CMD_MKFS;
	op.mkfs.path_size = strlen(path) + 1;


	link_debug(LINK_DEBUG_MESSAGE, "Write op (0x%d)", op.mkfs.cmd);
	err = link_transport_masterwrite(driver, &op, sizeof(link_open_t));
	if ( err < 0 ){
		return err;
	}

	//Send the path on the bulk out endpoint
	link_debug(LINK_DEBUG_MESSAGE, "Write mkfs path (%d bytes)", op.exec.path_size);
	len = link_transport_masterwrite(driver, path, op.exec.path_size);
	if ( len < 0 ){
		link_debug(1, "Failed to write bulk output");
		return LINK_TRANSFER_ERR;
	}

	driver->dev.wait(100);

	//read the reply to see if the file opened correctly
	err = link_transport_masterread(driver, &reply, sizeof(reply));
	if ( err < 0 ){
		link_debug(1, "Failed to read the reply");
		return err;
	}

	link_errno = reply.err_number;
	if ( reply.err < 0 ){
		link_debug(1, "Failed to exec file (%d)", link_errno);
	}
	return reply.err;

	return 0;
}

char * link_new_device_list(link_transport_mdriver_t * driver, int max){
	char name[LINK_PHY_NAME_MAX];
	char serialno[LINK_MAX_SN_SIZE];
	char last[LINK_PHY_NAME_MAX];
	char * sn_list;
	char * entry;
	int cnt;
	int err;
	int sys_fd;
	sys_attr_t sys_attr;

	sn_list = malloc(max*LINK_MAX_SN_SIZE);
	if( sn_list == NULL ){
		return NULL;
	}

	memset(sn_list, 0, max*LINK_MAX_SN_SIZE);

	//count the devices
	cnt = 0;
	memset(last, 0, LINK_PHY_NAME_MAX);

	//set timeout to account for devices that don't respond
	link_transport_mastersettimeout(50);

	while ( (err = driver->getname(name, last, LINK_PHY_NAME_MAX)) == 0 ){
		//success in getting new name
		driver->dev.handle = driver->dev.open(name, 0);
		if( driver->dev.handle != LINK_PHY_OPEN_ERROR ){
			if( link_readserialno(driver, serialno, LINK_MAX_SN_SIZE) == 0 ){
				entry = &(sn_list[LINK_MAX_SN_SIZE*cnt]);
				strcpy(entry, serialno);

				link_debug(LINK_DEBUG_MESSAGE, "Open dev/sys on 0x%llX", (u64)driver->dev.handle);

				//check to see if device is a bootloader?
				if( link_isbootloader(driver) ){
					link_debug(LINK_DEBUG_MESSAGE, "Device is a bootloader\n");
				} else {
					sys_fd = link_open(driver, "/dev/sys", LINK_O_RDWR);
					if( sys_fd >= 0 ){
						if( link_ioctl(driver, sys_fd, I_SYS_GETATTR, &sys_attr) == 0 ){
							//now add the sys_attr to the string list
							sys_attr.name[NAME_MAX-1] = 0;  //make sure these are zero terminated
							sys_attr.version[7] = 0;
							sprintf(entry, "%s:%s:%s",
									sys_attr.name,
									sys_attr.version,
									serialno);
						}
						link_close(driver, sys_fd);
					}
				}

				cnt++;
				if( cnt == max ){
					return sn_list;
				}
			}
			driver->dev.close(driver->dev.handle);
			driver->dev.handle = LINK_PHY_OPEN_ERROR;
		}
		strcpy(last, name);
	}

	//set timeout back to the default
	link_transport_mastersettimeout(0);

	return sn_list;
}


/*! \details This function deletes a device list previously
 * created with usbtools_new_device_list().
 */
void link_del_device_list(char * sn_list){
	free(sn_list);
}

char * link_device_list_entry(char * list, int entry){
	return &(list[entry*LINK_MAX_SN_SIZE]);
}

int link_handle_err(link_transport_mdriver_t * driver, int err){
	int tries;
	int err2;
	driver->dev.flush(driver->dev.handle);
	switch(err){
	case LINK_PHY_ERROR:
		break;
	case LINK_PROT_ERROR:
		//send zero length packets until device is ready
		tries = 0;
		link_debug(LINK_DEBUG_MESSAGE, "Try to overcome PROT error");
		do {
			err2 = link_transport_masterwrite(driver, NULL, 0);
			if( err2 == LINK_PHY_ERROR ){
				return LINK_PHY_ERROR; //failure
			}

			if( err2 == 0 ){
				link_debug(LINK_DEBUG_MESSAGE, "Successfully overcame PROT error");
				driver->dev.wait(10);
				driver->dev.flush(driver->dev.handle);
				return LINK_PROT_ERROR; //try the operation again
			}

			tries++;
		} while( tries < MAX_TRIES );
		break;
	case LINK_DEV_ERROR:
		return err;
	default:
		break;
	}

	link_error("PHY error");
	return LINK_PHY_ERROR;  //failure
}



/*! @} */
