

#include <errno.h>
#include "iface/dev/ioctl.h"
#include "device/sys.h"
#include "iface/device_config.h"



static inline int mcu_open(const device_cfg_t * cfg,
		int num_ports,
		int (*powered_on)(int),
		void (*power_on)(int)
) MCU_ALWAYS_INLINE MCU_PRIV_CODE;
int mcu_open(const device_cfg_t * cfg,
		int num_ports,
		int (*powered_on)(int),
		void (*power_on)(int)
){
	//Check to see if the port is value
	if ( DEVICE_GET_PORT(cfg) >= num_ports ){
		errno = EIO;
		return -1 - num_ports;
	}

	//Turn the device on
	power_on(DEVICE_GET_PORT(cfg));
	return 0;
}

static inline int mcu_ioctl(const device_cfg_t * cfg,
		int request,
		void * ctl,
		int num_ports,
		int (*powered_on)(int),
		int (* const ioctl_func_table[])(int, void*),
		const int ioctl_func_table_size) MCU_ALWAYS_INLINE MCU_PRIV_CODE;
int mcu_ioctl(const device_cfg_t * cfg,
		int request,
		void * ctl,
		int num_ports,
		int (*powered_on)(int),
		int (* const ioctl_func_table[])(int, void*),
		const int ioctl_func_table_size){
	uint32_t periph_request;

	//Check to see if the port is value
	if ( DEVICE_GET_PORT(cfg) >= num_ports ){
		errno = EINVAL;
		return -1;
	}

	if ( !powered_on(DEVICE_GET_PORT(cfg)) ){
		errno = EIO;
		return -1;
	}

	periph_request = _IOCTL_NUM(request);

	if ( periph_request < ioctl_func_table_size ) {
		return ioctl_func_table[periph_request](DEVICE_GET_PORT(cfg), ctl);
	}

	errno = EINVAL;
	return -200;
}

static inline int mcu_read(const device_cfg_t * cfg,
		device_transfer_t * rop,
		int num_ports,
		int (*powered_on)(int),
		int (*read)(const device_cfg_t * cfg, device_transfer_t*) ) MCU_ALWAYS_INLINE MCU_PRIV_CODE;
int mcu_read(const device_cfg_t * cfg,
		device_transfer_t * rop,
		int num_ports,
		int (*powered_on)(int),
		int (*read)(const device_cfg_t * cfg, device_transfer_t*) ){
	//Check to see if the port is value
	if ( DEVICE_GET_PORT(cfg) >= num_ports ){
		errno = EINVAL;
		return -1;
	}

	//Make sure the device is powered up
	if ( !powered_on(DEVICE_GET_PORT(cfg)) ){
		errno = EIO;
		return -1;
	}

	return read(cfg, rop);
}

static inline int mcu_write(const device_cfg_t * cfg,
		device_transfer_t * wop,
		int num_ports,
		int (*powered_on)(int),
		int (*write)(const device_cfg_t * cfg, device_transfer_t*) ) MCU_ALWAYS_INLINE MCU_PRIV_CODE;
int mcu_write(const device_cfg_t * cfg,
		device_transfer_t * wop,
		int num_ports,
		int (*powered_on)(int),
		int (*write)(const device_cfg_t * cfg, device_transfer_t*) ){
	//Check to see if the port is value
	if ( DEVICE_GET_PORT(cfg) >= num_ports ){
		errno = EINVAL;
		return -1;
	}

	//Make sure the device is powered up
	if ( !powered_on(DEVICE_GET_PORT(cfg)) ){
		errno = EIO;
		return -1;
	}

	return write(cfg, wop);
}



static inline int mcu_close(const device_cfg_t * cfg,
		int num_ports,
		int (*powered_on)(int),
		void (*power_off)(int) ) MCU_ALWAYS_INLINE MCU_PRIV_CODE;
int mcu_close(const device_cfg_t * cfg,
		int num_ports,
		int (*powered_on)(int),
		void (*power_off)(int) ){
	//Check to see if the port is value
	if ( DEVICE_GET_PORT(cfg) >= num_ports ){
		errno = EINVAL;
		return -1;
	}

	power_off(DEVICE_GET_PORT(cfg));
	return 0;
}
