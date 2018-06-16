/* Copyright 2011-2018 Tyler Gilbert; 
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

#include <errno.h>
#include <stddef.h>
#include "mcu/pio.h"
#include "device/led_pio.h"

#include "mcu/debug.h"

static void led_setattr(const devfs_handle_t * handle, const led_attr_t * attr);

int led_pio_open(const devfs_handle_t * handle){
    return 0;
}

int led_pio_ioctl(const devfs_handle_t * handle, int request, void * ctl){
    led_info_t * info = ctl;
    switch(request){
    case I_LED_GETINFO:
        info->o_flags = LED_FLAG_ENABLE | LED_FLAG_DISABLE | LED_FLAG_IS_HIGH_IMPEDANCE;
        info->o_events = 0;
        break;

    case I_LED_SETATTR:
        led_setattr(handle, ctl);
        break;

    case I_LED_SETACTION:
        return SYSFS_SET_RETURN(ENOTSUP);

    default:
        return SYSFS_SET_RETURN(EINVAL);

    }

    return 0;
}

int led_pio_read(const devfs_handle_t * handle, devfs_async_t * rop){
    return SYSFS_SET_RETURN(ENOTSUP);
}

int led_pio_write(const devfs_handle_t * handle, devfs_async_t * wop){
    return SYSFS_SET_RETURN(ENOTSUP);
}

int led_pio_close(const devfs_handle_t * handle){
    return 0;
}

void led_setattr(const devfs_handle_t * handle, const led_attr_t * attr){
    const led_pio_config_t * config = handle->config;
    u32 o_flags = attr->o_flags;
    pio_attr_t pio_attr;
    pio_attr.o_pinmask = (1<<config->pin);

    if( o_flags & LED_FLAG_ENABLE ){

        pio_attr.o_flags = PIO_FLAG_SET_OUTPUT;
        mcu_pio_setattr(handle, &pio_attr);

        if( config->o_flags & LED_PIO_CONFIG_FLAG_IS_ACTIVE_HIGH ){
            mcu_pio_setmask(handle, (void*)(1<<config->pin));
        } else {
            mcu_pio_clrmask(handle, (void*)(1<<config->pin));
        }

    } else if( o_flags & LED_FLAG_DISABLE ){

        if( o_flags & LED_FLAG_IS_HIGH_IMPEDANCE ){
            pio_attr.o_flags = PIO_FLAG_SET_INPUT | PIO_FLAG_IS_FLOAT;
        } else {
            pio_attr.o_flags = PIO_FLAG_SET_OUTPUT;
        }
        mcu_pio_setattr(handle, &pio_attr);
        if( config->o_flags & LED_PIO_CONFIG_FLAG_IS_ACTIVE_HIGH ){
            mcu_pio_clrmask(handle, (void*)(1<<config->pin));
        } else {
            mcu_pio_setmask(handle, (void*)(1<<config->pin));
        }
    }
}


