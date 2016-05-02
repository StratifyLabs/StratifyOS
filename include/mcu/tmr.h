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

/*! \addtogroup TMR_DEV TMR
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_TMR_H_
#define _MCU_TMR_H_

#include "../iface/dev/tmr.h"
#include "iface/device_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! \details
 * \sa periph_open()
 *
 */
int mcu_tmr_open(const device_cfg_t * cfg) MCU_PRIV_CODE;
/*! \details The timer value is read using the ioctl request: \ref I_TMR_GET.  This
 * function always returns an error.
 * \sa periph_read()
 *
 */
int mcu_tmr_read(const device_cfg_t * cfg, device_transfer_t * rop) MCU_PRIV_CODE;
/*! \details The timer value is written using the ioctl request: \ref I_TMR_SET.  This
 * function always returns an error.
 * \sa periph_write()
 */
int mcu_tmr_write(const device_cfg_t * cfg, device_transfer_t * wop) MCU_PRIV_CODE;
/*! \details
 * \sa periph_ioctl()
 *
 */
int mcu_tmr_ioctl(const device_cfg_t * cfg, int request, void * ctl) MCU_PRIV_CODE;
/*! \details
 * \sa periph_close()
 */
int mcu_tmr_close(const device_cfg_t * cfg) MCU_PRIV_CODE;


int mcu_tmr_getattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_tmr_setattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_tmr_setaction(int port, void * ctl) MCU_PRIV_CODE;

int mcu_tmrsimple_getattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_tmrsimple_setattr(int port, void * ctl) MCU_PRIV_CODE;
int mcu_tmrsimple_setaction(int port, void * ctl) MCU_PRIV_CODE;

int mcu_tmr_on(int port, void * ctl) MCU_PRIV_CODE;
int mcu_tmr_off(int port, void * ctl) MCU_PRIV_CODE;
int mcu_tmr_setoc(int port, void * ctl) MCU_PRIV_CODE;
int mcu_tmr_getoc(int port, void * ctl) MCU_PRIV_CODE;
int mcu_tmr_setic(int port, void * ctl) MCU_PRIV_CODE;
int mcu_tmr_getic(int port, void * ctl) MCU_PRIV_CODE;
int mcu_tmr_set(int port, void * ctl) MCU_PRIV_CODE;
int mcu_tmr_get(int port, void * ctl) MCU_PRIV_CODE;



#ifdef __cplusplus
}
#endif

#endif // _MCU_TMR_H_

/*! @} */

