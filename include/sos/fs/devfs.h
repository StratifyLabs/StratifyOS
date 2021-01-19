// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#ifndef SOS_FS_DEVFS_H_
#define SOS_FS_DEVFS_H_

#include <errno.h>
#include <sdk/types.h>

#include <sys/dirent.h>

#include "sysfs.h"

#define DEVFS_GET_PORT(x) (x->port)

#if defined __cplusplus
extern "C" {
#endif

/*! \details This structure is for executing a function as a result of an interrupt
 * in non-privileged mode.
 */
typedef struct {
  int tid;
  int si_signo;
  int si_sigcode;
  union {
    int sig_value;
    void *sig_ptr;
  };
  int keep;
} devfs_signal_callback_t;

int devfs_signal_callback(void *context, const mcu_event_t *data) MCU_ROOT_EXEC_CODE;

const devfs_handle_t *devfs_lookup_handle(const devfs_device_t *list, const char *name);
const devfs_device_t *
devfs_lookup_device(const devfs_device_t *list, const char *device_name);
int devfs_lookup_name(
  const devfs_device_t *list,
  const devfs_device_t *device,
  devfs_get_name_t name);

#define DEVFS_MODE(mode_value, uid_value, type)                                          \
  .mode = mode_value | type, .uid = uid_value

#define DEVFS_DRIVER(driver_name)                                                        \
  .driver.open = driver_name##_open, .driver.close = driver_name##_close,                \
  .driver.ioctl = driver_name##_ioctl, .driver.read = driver_name##_read,                \
  .driver.write = driver_name##_write

#define DEVFS_DRIVER_DECLARTION_OPEN(driver_name)                                        \
  int driver_name##_open(const devfs_handle_t *) MCU_ROOT_CODE
#define DEVFS_DRIVER_DECLARTION_CLOSE(driver_name)                                       \
  int driver_name##_close(const devfs_handle_t *) MCU_ROOT_CODE
#define DEVFS_DRIVER_DECLARTION_IOCTL(driver_name)                                       \
  int driver_name##_ioctl(const devfs_handle_t *, int, void *) MCU_ROOT_CODE
#define DEVFS_DRIVER_DECLARTION_READ(driver_name)                                        \
  int driver_name##_read(const devfs_handle_t *, devfs_async_t *) MCU_ROOT_CODE
#define DEVFS_DRIVER_DECLARTION_WRITE(driver_name)                                       \
  int driver_name##_write(const devfs_handle_t *, devfs_async_t *) MCU_ROOT_CODE

#define DEVFS_DRIVER_DECLARTION(driver_name)                                             \
  DEVFS_DRIVER_DECLARTION_OPEN(driver_name);                                             \
  DEVFS_DRIVER_DECLARTION_CLOSE(driver_name);                                            \
  DEVFS_DRIVER_DECLARTION_IOCTL(driver_name);                                            \
  DEVFS_DRIVER_DECLARTION_READ(driver_name);                                             \
  DEVFS_DRIVER_DECLARTION_WRITE(driver_name)

#define DEVFS_DRIVER_DECLARTION_IOCTL_REQUEST(driver_name, request)                      \
  int driver_name##_##request(const devfs_handle_t *, void *) MCU_ROOT_CODE

static inline int devfs_mcu_ioctl(
  const devfs_handle_t *handle,
  int request,
  void *ctl,
  char ident,
  int (*const ioctl_func_table[])(const devfs_handle_t *, void *),
  const int ioctl_func_table_size) MCU_ALWAYS_INLINE MCU_ROOT_CODE;
int devfs_mcu_ioctl(
  const devfs_handle_t *handle,
  int request,
  void *ctl,
  char ident,
  int (*const ioctl_func_table[])(const devfs_handle_t *, void *),
  const int ioctl_func_table_size) {
  u32 periph_request;

  periph_request = _IOCTL_NUM(request);

  // check the MCU IDENT request
  if ((ident == _IOCTL_IDENT(request)) || (request == I_MCU_SETACTION)) {
    if (periph_request < ioctl_func_table_size) {
      return ioctl_func_table[periph_request](handle, ctl);
    }
  }
  return SYSFS_SET_RETURN(EINVAL);
}

#define DEVFS_DRIVER_IS_BUSY(transfer, async)                                            \
  if (transfer) {                                                                        \
    return SYSFS_SET_RETURN(EBUSY);                                                      \
  }                                                                                      \
  if (async->nbyte == 0) {                                                               \
    return 0;                                                                            \
  }                                                                                      \
  transfer = async

#define DEVFS_DRIVER_PORT(object) (((const object##_config_t *)handle->config)->port)

#define DEVFS_DRIVER_DECLARE_LOCAL(object, port_count)                                   \
  const u32 port = config->port;                                                         \
  if (port >= port_count) {                                                              \
    return SYSFS_SET_RETURN(EBUSY);                                                      \
  }                                                                                      \
  object##_local_t *local = m_##object##_local + config->port

#define DEVFS_DRIVER_DECLARE_STATE_LOCAL(object, port_count)                             \
  const u32 port = config->port;                                                         \
  if (port >= port_count) {                                                              \
    return SYSFS_SET_RETURN(EBUSY);                                                      \
  }                                                                                      \
  object##_local_t *local = (object##_local_t *)handle->state

#define DEVFS_DRIVER_DECLARE_CONFIG_STATE(object)                                        \
  const object##_config_t *config = handle->config;                                      \
  object##_state_t *state = (object##_state_t *)handle->state

#define DEVFS_DRIVER_OPEN_STATE_LOCAL_V4(object)                                         \
  m_##object##_state_list[config->port] = handle->state;

#define DEVFS_DRIVER_CLOSE_STATE_LOCAL_V4(object)                                        \
  m_##object##_state_list[config->port] = NULL;

#define DEVFS_DRIVER_ASSIGN_STATE_LOCAL(object)                                          \
  (m_##object##_state_list[config->port] = handle->state)

#define DEVFS_ASSIGN_ATTRIBUTES(object, ctl)                                             \
  (ctl == NULL) ? &(((const object##_config_t *)handle->config)->attr) : ctl;

#define DEVFS_DRIVER_DECLARE_STATE_LOCAL_ARRAY(object, count)                            \
  object##_local_t *m_##object##_state_state_list[count] MCU_SYS_MEM

#define DEVFS_MCU_DRIVER_IOCTL_FUNCTION(                                                 \
  driver_name, version, ident_char, ioctl_total, ...)                                    \
  int mcu_##driver_name##_getversion(const devfs_handle_t *handle, void *ctl) {          \
    return version;                                                                      \
  }                                                                                      \
  int (*const mcu_##driver_name##_ioctl_func_table[ioctl_total])(                        \
    const devfs_handle_t *, void *) = {                                                  \
    mcu_##driver_name##_getversion, mcu_##driver_name##_getinfo,                         \
    mcu_##driver_name##_setattr, mcu_##driver_name##_setaction, __VA_ARGS__};            \
  int mcu_##driver_name##_ioctl(const devfs_handle_t *handle, int request, void *ctl) {  \
    return devfs_mcu_ioctl(                                                              \
      handle, request, ctl, ident_char, mcu_##driver_name##_ioctl_func_table,            \
      ioctl_total);                                                                      \
  }

#define DEVFS_MCU_DRIVER_IOCTL_FUNCTION_MIN(driver_name, version, ident_char)            \
  int mcu_##driver_name##_getversion(const devfs_handle_t *handle, void *ctl) {          \
    return version;                                                                      \
  }                                                                                      \
  int (*const mcu_##driver_name##_ioctl_func_table[I_MCU_TOTAL])(                        \
    const devfs_handle_t *, void *) = {                                                  \
    mcu_##driver_name##_getversion, mcu_##driver_name##_getinfo,                         \
    mcu_##driver_name##_setattr, mcu_##driver_name##_setaction};                         \
  int mcu_##driver_name##_ioctl(const devfs_handle_t *handle, int request, void *ctl) {  \
    return devfs_mcu_ioctl(                                                              \
      handle, request, ctl, ident_char, mcu_##driver_name##_ioctl_func_table,            \
      I_MCU_TOTAL);                                                                      \
  }

#define DEVFS_DEVICE(                                                                    \
  device_name, periph_name, handle_port, handle_config, handle_state, mode_value,        \
  uid_value, device_type)                                                                \
  {                                                                                      \
    .name = device_name, DEVFS_MODE(mode_value, uid_value, device_type),                 \
    DEVFS_DRIVER(periph_name), .handle.port = handle_port, .handle.state = handle_state, \
    .handle.config = handle_config                                                       \
  }

#define DEVFS_TERMINATOR                                                                 \
  { .driver.open = NULL }

static inline bool devfs_is_terminator(const devfs_device_t *dev);
bool devfs_is_terminator(const devfs_device_t *dev) {
  if (dev->driver.open == NULL) {
    return true;
  }
  return false;
}

int devfs_execute_event_handler(mcu_event_handler_t *handler, u32 o_events, void *data)
  MCU_ROOT_EXEC_CODE;
// executes the read handler (if it exists) and nulls the read async object so it can be
// assigned again
int devfs_execute_read_handler(
  devfs_transfer_handler_t *transfer_handler,
  void *data,
  int nbyte,
  u32 o_flags) MCU_ROOT_EXEC_CODE;
// executes the write handler (if it exists) and nulls the write async object so it can be
// assigned again
int devfs_execute_write_handler(
  devfs_transfer_handler_t *transfer_handler,
  void *data,
  int nbyte,
  u32 o_flags) MCU_ROOT_EXEC_CODE;

// executes read and write handlers (if they exist) with MCU_EVENT_FLAG_CANCELED set
void devfs_execute_cancel_handler(
  devfs_transfer_handler_t *transfer_handler,
  void *data,
  int nbyte,
  u32 o_flags) MCU_ROOT_EXEC_CODE;

int devfs_init(const void *cfg);
int devfs_open(const void *cfg, void **handle, const char *path, int flags, int mode);
int devfs_read(const void *cfg, void *handle, int flags, int loc, void *buf, int nbyte);
int devfs_write(
  const void *cfg,
  void *handle,
  int flags,
  int loc,
  const void *buf,
  int nbyte);
int devfs_aio(const void *cfg, void *handle, struct aiocb *aio);
int devfs_ioctl(const void *cfg, void *handle, int request, void *ctl);
int devfs_close(const void *cfg, void **handle);
int devfs_fstat(const void *cfg, void *handle, struct stat *st);
int devfs_stat(const void *cfg, const char *path, struct stat *st);
int devfs_opendir(const void *cfg, void **handle, const char *path);
int devfs_readdir_r(const void *cfg, void *handle, int loc, struct dirent *entry);
int devfs_closedir(const void *cfg, void **handle);

#define DEVFS_MOUNT(mount_loc_name, cfgp, permissions_value, owner_value)                \
  {                                                                                      \
    .mount_path = mount_loc_name, .permissions = permissions_value,                      \
    .owner = owner_value, .mount = devfs_init, .unmount = SYSFS_NOTSUP,                  \
    .ismounted = sysfs_always_mounted, .startup = SYSFS_NOTSUP, .mkfs = SYSFS_NOTSUP,    \
    .open = devfs_open, .aio = devfs_aio, .ioctl = devfs_ioctl, .fsync = SYSFS_NOTSUP,   \
    .read = devfs_read, .write = devfs_write, .close = devfs_close,                      \
    .rename = SYSFS_NOTSUP, .unlink = SYSFS_NOTSUP, .mkdir = SYSFS_NOTSUP,               \
    .rmdir = SYSFS_NOTSUP, .remove = SYSFS_NOTSUP, .opendir = devfs_opendir,             \
    .closedir = devfs_closedir, .readdir_r = devfs_readdir_r, .link = SYSFS_NOTSUP,      \
    .symlink = SYSFS_NOTSUP, .stat = devfs_stat, .lstat = SYSFS_NOTSUP,                  \
    .fstat = devfs_fstat, .chmod = SYSFS_NOTSUP, .chown = SYSFS_NOTSUP,                  \
    .unlock = SYSFS_NOTSUP_VOID, .config = cfgp,                                         \
  }

#if defined __cplusplus
}
#endif

#endif /* SOS_FS_DEVFS_H_ */
