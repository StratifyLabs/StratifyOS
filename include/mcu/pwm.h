// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

/*! \addtogroup PWM_DEV PWM
 * @{
 * \ingroup DEV
 *
 */

#ifndef _MCU_PWM_H_
#define _MCU_PWM_H_

#include "sos/dev/pwm.h"
#include "sos/fs/devfs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MCU_PACK {
  u32 value;
} pwm_event_data_t;

typedef struct MCU_PACK {
  u32 port;
  pwm_attr_t attr; // default attributes
} pwm_config_t;

#define MCU_PWM_IOCTL_REQUEST_DECLARATION(driver_name)                                   \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getinfo);                           \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setattr);                           \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setaction);                         \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, setchannel);                        \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, getchannel);                        \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, get);                               \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, set);                               \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, enable);                            \
  DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, disable)

#define MCU_PWM_DRIVER_DECLARATION(variant)                                              \
  DEVFS_DRIVER_DECLARTION(variant);                                                      \
  MCU_PWM_IOCTL_REQUEST_DECLARATION(variant)

MCU_PWM_DRIVER_DECLARATION(mcu_pwm_tmr); // running a tmr in PWM mode

// native pwm unit
int mcu_pwm_open(const devfs_handle_t *handle) MCU_ROOT_CODE;
int mcu_pwm_read(const devfs_handle_t *handle, devfs_async_t *rop) MCU_ROOT_CODE;
int mcu_pwm_write(const devfs_handle_t *handle, devfs_async_t *wop) MCU_ROOT_CODE;
int mcu_pwm_ioctl(const devfs_handle_t *handle, int request, void *ctl) MCU_ROOT_CODE;
int mcu_pwm_close(const devfs_handle_t *handle) MCU_ROOT_CODE;

int mcu_pwm_getinfo(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_pwm_setattr(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_pwm_setaction(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_pwm_setchannel(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_pwm_getchannel(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_pwm_set(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_pwm_get(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_pwm_enable(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;
int mcu_pwm_disable(const devfs_handle_t *handle, void *ctl) MCU_ROOT_CODE;

#define PWM_DEFINE_ATTR(                                                                 \
  attr_flags, attr_freq, attr_period, attr_channel0_port, attr_channel0_pin,             \
  attr_channel1_port, attr_channel1_pin, attr_channel2_port, attr_channel2_pin,          \
  attr_channel3_port, attr_channel3_pin)                                                 \
  .o_flags = attr_flags, .freq = attr_freq, .period = attr_period, .channel = {0, 0},    \
  .pin_assignment.channel[0] = {attr_channel0_port, attr_channel0_pin},                  \
  .pin_assignment.channel[1] = {attr_channel1_port, attr_channel1_pin},                  \
  .pin_assignment.channel[2] = {attr_channel2_port, attr_channel2_pin},                  \
  .pin_assignment.channel[3] = {attr_channel3_port, attr_channel3_pin}

#define PWM_DECLARE_CONFIG(                                                              \
  name, attr_flags, attr_freq, attr_period, attr_channel0_port, attr_channel0_pin,       \
  attr_channel1_port, attr_channel1_pin, attr_channel2_port, attr_channel2_pin,          \
  attr_channel3_port, attr_channel3_pin)                                                 \
  pwm_config_t name##_config = {                                                         \
    .attr = {PWM_DEFINE_ATTR(                                                            \
      attr_flags, attr_freq, attr_period, attr_channel0_port, attr_channel0_pin,         \
      attr_channel1_port, attr_channel1_pin, attr_channel2_port, attr_channel2_pin,      \
      attr_channel3_port, attr_channel3_pin)}}

#ifdef __cplusplus
}
#endif

#endif /* _MCU_PWM_H_ */

/*! @} */
