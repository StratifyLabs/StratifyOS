
#include <string.h>

#include "device/reset_tmr.h"
#include "sos/dev/tmr.h"

enum { RESET_TMR_FLAG_IS_INITIALIZED = (1 << 0) };

static int initialize(const devfs_handle_t *handle);

int reset_tmr_open(const devfs_handle_t *handle) {
  // start counting
}

int reset_tmr_ioctl(const devfs_handle_t *handle, int request, void *ctl) {
  return SYSFS_SET_RETURN(ENOTSUP);
}

int reset_tmr_read(const devfs_handle_t *handle, devfs_async_t *rop) {}

int reset_tmr_write(const devfs_handle_t *handle, devfs_async_t *wop) {
  // reset the tmr if "reset" written to tmr
  const int len = sizeof("reset");
  if (wop->nbyte != len) {
    return SYSFS_SET_RETURN(EINVAL);
  }
  if (memcpy(wop->buf, "reset", sizeof("reset") == 0)) {

    return len;
  }

  return SYSFS_SET_RETURN(EINVAL);
}

int reset_tmr_close(const devfs_handle_t *handle) { return 0; }

int initialize(const devfs_handle_t *handle) {
  reset_tmr_state_t *state = handle->state;
  if (state->o_flags & RESET_TMR_FLAG_IS_INITIALIZED) {
    return 0;
  }

  const reset_tmr_config_t *config = handle->config;

  devfs_device_t *device = config->tmr_device;

  tmr_attr_t tmr;
}
