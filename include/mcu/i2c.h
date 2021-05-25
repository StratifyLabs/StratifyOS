// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup I2C_DEV
 * @{
 *
 * \ingroup DEV
 */

#ifndef _MCU_I2C_H_
#define _MCU_I2C_H_

#include "sos/dev/i2c.h"

#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  u32 value;
} i2c_event_t;

typedef struct MCU_PACK {
  i2c_attr_t attr; // default attributes
  u32 port;
} i2c_config_t;

int mcu_i2c_open(const devfs_handle_t *handle) MCU_ROOT_CODE;
int mcu_i2c_read(const devfs_handle_t *handle, devfs_async_t *async) MCU_ROOT_CODE;
int mcu_i2c_write(const devfs_handle_t *handle, devfs_async_t *async) MCU_ROOT_CODE;
int mcu_i2c_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_CODE;
int mcu_i2c_close(const devfs_handle_t *handle) MCU_ROOT_CODE;

int mcu_i2c_getinfo(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_i2c_setattr(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_i2c_setaction(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

#define I2C_DEFINE_ATTR_MASTER(                                                          \
  i2c_attr_flags, i2c_attr_freq, i2c_attr_sda_port, i2c_attr_sda_pin, i2c_attr_scl_port, \
  i2c_attr_scl_pin)                                                                      \
  .o_flags = i2c_attr_flags, .freq = i2c_attr_freq,                                      \
  .pin_assignment.scl = {i2c_attr_scl_port, i2c_attr_scl_pin},                           \
  .pin_assignment.sda = {i2c_attr_sda_port, i2c_attr_sda_pin}

#define I2C_DECLARE_CONFIG_MASTER(                                                       \
  i2c_name, i2c_attr_flags, i2c_attr_freq, i2c_attr_sda_port, i2c_attr_sda_pin,          \
  i2c_attr_scl_port, i2c_attr_scl_pin)                                                   \
  const i2c_config_t i2c_name##_config = {                                               \
    .attr = {I2C_DEFINE_ATTR_MASTER(                                                     \
      i2c_attr_flags, i2c_attr_freq, i2c_attr_sda_port, i2c_attr_sda_pin,                \
      i2c_attr_scl_port, i2c_attr_scl_pin)}}

#ifdef __cplusplus
}
#endif

#endif // _MCU_I2C_H_

/*! @} */
