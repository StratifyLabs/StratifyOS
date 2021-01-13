// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <errno.h>
#include <unistd.h>

#include "sos/debug.h"
#include "mcu/pio.h"
#include "mcu/spi.h"
#include "sos/sos.h"
#include "sst25vf_local.h"

static int complete_spi_write(void *context, const mcu_event_t *event);
static int continue_spi_write(void *context, const mcu_event_t *event);
static int complete_spi_read(void *context, const mcu_event_t *event);

int sst25vf_open(const devfs_handle_t *handle) {
  int err;
  uint8_t status;
  pio_attr_t pio_attr;
  sst25vf_state_t *state = (sst25vf_state_t *)handle->state;
  const sst25vf_config_t *config = handle->config;
  devfs_handle_t pio_handle;
  pio_handle.port = config->cs.port;
  pio_handle.config = 0;
  pio_handle.state = 0;

  err = mcu_spi_open(handle);
  if (err < 0) {
    return err;
  }

  if ((err = mcu_spi_setattr(handle, (void *)&(config->spi.attr))) < 0) {
    return err;
  }

  sst25vf_share_deassert_cs(handle);
  pio_attr.o_pinmask = (1 << config->cs.pin);
  pio_attr.o_flags = PIO_FLAG_SET_OUTPUT | PIO_FLAG_IS_DIRONLY;
  mcu_pio_setattr(&pio_handle, &pio_attr);

  sst25vf_share_write_disable(handle);
  sst25vf_share_write_ebsy(handle); // enable SO

  // Now ping the device to see if it responds
  sst25vf_share_power_up(handle);
  sst25vf_share_global_protect(handle);
  status = sst25vf_share_read_status(handle);

  if (status != 0x9C) {
    // Global protect command failed
    return SYSFS_SET_RETURN(EIO);
  }

  if (sst25vf_share_global_unprotect(handle)) {
    return SYSFS_SET_RETURN(EIO);
  }
  status = sst25vf_share_read_status(handle);

  if (status != 0x80) {
    // global unprotect failed
    return SYSFS_SET_RETURN(EIO);
  }

  state->prot = 0;

  // The device is ready to use
  return 0;
}

int complete_spi_read(void *context, const mcu_event_t *event) {
  const devfs_handle_t *handle = context;
  sst25vf_state_t *state = handle->state;
  sst25vf_share_deassert_cs(handle);

  // use mcu_execute_handler
  devfs_execute_event_handler(&(state->handler), MCU_EVENT_FLAG_DATA_READY, 0);
  return 0;
}

int sst25vf_read(const devfs_handle_t *handle, devfs_async_t *rop) {
  sst25vf_state_t *state = (sst25vf_state_t *)handle->state;
  const sst25vf_config_t *dcfg = (const sst25vf_config_t *)(handle->config);

  if (state->handler.callback) {
    return SYSFS_SET_RETURN(EBUSY);
  }

  if (rop->loc >= dcfg->size) {
    return SYSFS_SET_RETURN(EINVAL);
  }

  state->handler.callback = rop->handler.callback;
  state->handler.context = rop->handler.context;
  rop->handler.context = (void *)handle;
  rop->handler.callback = (mcu_callback_t)complete_spi_read;

  if (rop->loc + rop->nbyte > dcfg->size) {
    rop->nbyte =
      dcfg->size - rop->loc; // update the bytes read to not go past the end of the disk
  }

  sst25vf_share_assert_cs(handle);
  sst25vf_share_write_opcode_addr(handle, SST25VF_INS_RD_HS, rop->loc);
  mcu_spi_swap(handle, NULL); // dummy byte output
  return mcu_spi_read(handle, rop);
}

static void assert_delay() {
  int loop_max;
  int tmp;

  if (sos_config.clock.frequency <= 25000000) {
    return;
  } else if (sos_config.clock.frequency > 100000000) {
    loop_max = 75;
  } else if (sos_config.clock.frequency > 50000000) {
    loop_max = 60;
  } else if (sos_config.clock.frequency > 25000000) {
    loop_max = 30;
  }

  // Must be high for either 100ns (25MHz model) or 50ns (50MHz and 80MHz) models
  // This loop is 4 instructions --
  for (tmp = 0; tmp < loop_max; tmp++) {
    asm volatile("nop\n");
  }
}

int continue_spi_write(void *context, const mcu_event_t *event) {
  const devfs_handle_t *handle = context;
  sst25vf_state_t *state = (sst25vf_state_t *)handle->state;
  // should be called 10 us after complete_spi_write() executes

  sst25vf_share_deassert_cs(handle);

  if (state->nbyte > 0) {
    int tmp;
    state->cmd[0] = SST25VF_INS_SEQ_PROGRAM;
    state->cmd[1] = state->buf[0];
    if (state->nbyte > 1) {
      state->cmd[2] = state->buf[1];
      tmp = 2;
    } else {
      state->cmd[2] = 0xFF;
      tmp = 1;
    }

    assert_delay();
    sst25vf_share_assert_cs(handle);
    state->op.nbyte = 3;

    mcu_spi_write(handle, &(state->op));
    state->buf += tmp;
    state->nbyte -= tmp;

  } else {

    sst25vf_share_write_disable(handle);

    sst25vf_share_read_status(handle);

    // call the event handler to show the operation is complete
    devfs_execute_event_handler(&(state->handler), MCU_EVENT_FLAG_WRITE_COMPLETE, 0);
  }
  return 0;
}

int complete_spi_write(void *context, const mcu_event_t *event) {
  const devfs_handle_t *handle = context;
  sst25vf_config_t *sst_cfg = (sst25vf_config_t *)handle->config;
  devfs_handle_t pio_handle;
  u32 pio_value;
  pio_handle.port = sst_cfg->spi.attr.pin_assignment.miso.port;
  pio_handle.config = 0;
  pio_handle.state = 0;

  sst25vf_share_deassert_cs(handle);
  assert_delay();
  sst25vf_share_assert_cs(handle);

  do {
    mcu_pio_get(&pio_handle, &pio_value);
  } while ((pio_value & (1 << sst_cfg->spi.attr.pin_assignment.miso.pin)) == 0);

  continue_spi_write(context, event);

  return 0;
}

int sst25vf_write(const devfs_handle_t *handle, devfs_async_t *wop) {
  int tmp;
  int err;
  uint8_t *addrp;
  sst25vf_state_t *state = (sst25vf_state_t *)handle->state;
  // sst25vf_config_t * sst_cfg = (sst25vf_config_t*)handle->config;

  if (state->prot == 1) {
    return SYSFS_SET_RETURN(EROFS);
  }

  if (state->handler.callback) {
    return SYSFS_SET_RETURN(EBUSY);
  }

  // This is the final callback and context when all the writing is done
  state->handler = wop->handler;
  state->buf = wop->buf;
  state->nbyte = wop->nbyte;

  sst25vf_share_write_enable(handle);

  // set Auto increment
  addrp = (uint8_t *)&(wop->loc);
  state->cmd[0] = SST25VF_INS_SEQ_PROGRAM;
  state->cmd[1] = addrp[2];
  state->cmd[2] = addrp[1];
  state->cmd[3] = addrp[0];

  if (wop->loc & 0x01) {
    // starts on an odd address
    state->cmd[4] = 0xFF;
    state->cmd[5] = state->buf[0];
    tmp = 1;
  } else {
    state->cmd[4] = state->buf[0];
    if (state->nbyte > 1) {
      state->cmd[5] = state->buf[1];
      tmp = 2;
    } else {
      state->cmd[5] = 0xFF;
      tmp = 1;
    }
  }

  state->op.flags = wop->flags;
  state->op.handler.callback = (mcu_callback_t)complete_spi_write;
  state->op.handler.context = (void *)handle;
  state->op.buf_const = state->cmd;
  state->op.nbyte = 6;
  state->op.loc = 0;

  sst25vf_share_assert_cs(handle);
  err = mcu_spi_write(handle, &state->op);
  state->buf = state->buf + tmp;
  state->nbyte = state->nbyte - tmp;

  return err;
}

int sst25vf_ioctl(const devfs_handle_t *handle, int request, void *ctl) {
  return sst25vf_share_ioctl(handle, request, ctl);
}

int sst25vf_close(const devfs_handle_t *handle) { return 0; }

/*! @} */
