

#include <errno.h>
#include "mcu/debug.h"
#include "iface/dev/ioctl.h"
#include "dev/sys.h"
#include "iface/device_config.h"



static inline int mcu_open(const device_cfg_t * cfg,
		int (*powered_on)(int),
		void (*power_on)(int)
) MCU_ALWAYS_INLINE MCU_PRIV_CODE;
int mcu_open(const device_cfg_t * cfg,
		int (*powered_on)(int),
		void (*power_on)(int)
){

	//Turn the device on
	power_on(DEVICE_GET_PORT(cfg));
	return 0;
}

static inline int mcu_ioctl(const device_cfg_t * cfg,
		int request,
		void * ctl,
		int (*powered_on)(int),
		int (* const ioctl_func_table[])(int, void*),
		const int ioctl_func_table_size) MCU_ALWAYS_INLINE MCU_PRIV_CODE;
int mcu_ioctl(const device_cfg_t * cfg,
		int request,
		void * ctl,
		int (*powered_on)(int),
		int (* const ioctl_func_table[])(int, void*),
		const int ioctl_func_table_size){
	uint32_t periph_request;


	if ( !powered_on(DEVICE_GET_PORT(cfg)) ){
		errno = EIO;
		return -200;
	}

	periph_request = _IOCTL_NUM(request);

	if ( periph_request < ioctl_func_table_size ) {
		return ioctl_func_table[periph_request](DEVICE_GET_PORT(cfg), ctl);
	}

	errno = EINVAL;
	return -300;
}

static inline int mcu_read(const device_cfg_t * cfg,
		device_transfer_t * rop,
		int (*powered_on)(int),
		int (*read)(const device_cfg_t * cfg, device_transfer_t*) ) MCU_ALWAYS_INLINE MCU_PRIV_CODE;
int mcu_read(const device_cfg_t * cfg,
		device_transfer_t * rop,
		int (*powered_on)(int),
		int (*read)(const device_cfg_t * cfg, device_transfer_t*) ){

	//Make sure the device is powered up
	if ( !powered_on(DEVICE_GET_PORT(cfg)) ){
		errno = EIO;
		return -1;
	}

	return read(cfg, rop);
}

static inline int mcu_write(const device_cfg_t * cfg,
		device_transfer_t * wop,
		int (*powered_on)(int),
		int (*write)(const device_cfg_t * cfg, device_transfer_t*) ) MCU_ALWAYS_INLINE MCU_PRIV_CODE;
int mcu_write(const device_cfg_t * cfg,
		device_transfer_t * wop,
		int (*powered_on)(int),
		int (*write)(const device_cfg_t * cfg, device_transfer_t*) ){

	//Make sure the device is powered up
	if ( !powered_on(DEVICE_GET_PORT(cfg)) ){
		errno = EIO;
		return -1;
	}

	return write(cfg, wop);
}



static inline int mcu_close(const device_cfg_t * cfg,
		int (*powered_on)(int),
		void (*power_off)(int) ) MCU_ALWAYS_INLINE MCU_PRIV_CODE;
int mcu_close(const device_cfg_t * cfg,
		int (*powered_on)(int),
		void (*power_off)(int) ){
	power_off(DEVICE_GET_PORT(cfg));
	return 0;
}
