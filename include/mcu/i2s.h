// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup I2S Inter-Integated Sound (I2S) Master/Slave Driver
 * @{
 *
 * \ingroup IFACE_DEV
 * \details This module implements an I2S driver for hardware which supports I2S.
 *
 *
 */

/*! \file
 * \brief I2S Header File
 *
 */

#ifndef I2S_H_
#define I2S_H_

#include "sos/dev/i2s.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
  i2s_attr_t attr; // default attributes
  u32 port;
} i2s_config_t;

typedef struct MCU_PACK {
  u32 value;
} i2s_event_t;

typedef i2s_config_t sai_config_t;

/*
 * These macros will declare the following functions with the specified variants
 *
 * int mcu_i2s_open(const devfs_handle_t * handle);
 * int mcu_i2s_close(const devfs_handle_t * handle);
 * int mcu_i2s_read(const devfs_handle_t * handle);
 * int mcu_i2s_write(const devfs_handle_t * handle);
 * int mcu_i2s_ioctl(const devfs_handle_t * handle, int request, ...);
 *
 * int mcu_i2s_getinfo(const devfs_handle_t * handle, void * ctl); //plus getversion,
 * setattr, setaction, mute, unmute
 *
 * In the above case mcu_i2s is the variant. A variant on the driver might be mcu_i2s_dma
 * where DMA is used rather than interrupts.
 *
 *
 */

#define MCU_I2S_IOCTL_REQUEST_DECLARATION(driver_name)                                   \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getinfo);                           \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setattr);                           \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setaction);                         \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, mute);                              \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, unmute)

#define MCU_I2S_DRIVER_DECLARATION(variant)                                              \
  DEVFS_DRIVER_DECLARTION(variant);                                                      \
  MCU_I2S_IOCTL_REQUEST_DECLARATION(variant)

MCU_I2S_DRIVER_DECLARATION(mcu_i2s);
MCU_I2S_DRIVER_DECLARATION(mcu_i2s_spi);
MCU_I2S_DRIVER_DECLARATION(mcu_i2s_spi_dma);
MCU_I2S_DRIVER_DECLARATION(mcu_sai);
MCU_I2S_DRIVER_DECLARATION(mcu_sai_dma);

#define I2S_DEFINE_ATTR(                                                                 \
  attr_flags, attr_freq, attr_mck_mult, attr_ws_port, attr_ws_pin, attr_sck_port,        \
  attr_sck_pin, attr_sdout_port, attr_sdout_pin, attr_sdin_port, attr_sdin_pin,          \
  attr_mck_port, attr_mck_pin)                                                           \
  .o_flags = attr_flags, .freq = attr_freq, .mck_mult = attr_mck_mult,                   \
  .pin_assignment.ws = {attr_ws_port, attr_ws_pin},                                      \
  .pin_assignment.sck = {attr_sck_port, attr_sck_pin},                                   \
  .pin_assignment.sdout = {attr_sdout_port, attr_sdout_pin},                             \
  .pin_assignment.sdin = {attr_sdin_port, attr_sdin_pin},                                \
  .pin_assignment.mck = {attr_mck_port, attr_mck_pin}

#define I2S_DECLARE_CONFIG(                                                              \
  name, attr_flags, attr_freq, attr_mck_mult, attr_ws_port, attr_ws_pin, attr_sck_port,  \
  attr_sck_pin, attr_sdout_port, attr_sdout_pin, attr_sdin_port, attr_sdin_pin,          \
  attr_mck_port, attr_mck_pin)                                                           \
  const i2s_config_t name##_config = {                                                   \
    .attr = {I2S_DEFINE_ATTR(                                                            \
      attr_flags, attr_freq, attr_mck_mult, attr_ws_port, attr_ws_pin, attr_sck_port,    \
      attr_sck_pin, attr_sdout_port, attr_sdout_pin, attr_sdin_port, attr_sdin_pin,      \
      attr_mck_port, attr_mck_pin)}}

#ifdef __cplusplus
}
#endif

#endif /* I2S_H_ */

/*! @} */
