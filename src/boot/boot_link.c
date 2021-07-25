/* Copyright 2011;
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
 * Please see http://www.coactionos.com/license.html for
 * licensing information.
 */

/*! \addtogroup LINK
 * @{
 *
 */

#include <errno.h>
#include <stdbool.h>
#include <sys/fcntl.h>

#include "sos_config.h"

#include "sos/symbols.h"

#include "boot_config.h"
#include "boot_link.h"
#include "cortexm/cortexm.h"
#include "cortexm/util.h"
#include "sos/arch.h"
#include "sos/debug.h"
#include "sos/led.h"
#include "sos/sos.h"

static bool is_erased = false;

static u8 first_page[256];

static u32 hash_size = 0;

#if CONFIG_BOOT_IS_VERIFY_SIGNATURE
static u8 ecc_context_buffer[256];
static void *ecc_context = ecc_context_buffer;
static u8 sha_context_buffer[256];
static void *sha_context = sha_context_buffer;
#endif

const devfs_handle_t flash_dev = {.port = 0};

static int read_flash(link_transport_driver_t *driver, int loc, int nbyte);
static int read_flash_callback(void *context, void *buf, int nbyte);

typedef struct {
  int err;
  int nbyte;
  link_op_t op;
  link_reply_t reply;
} link_data_t;

static void boot_link_cmd_none(link_transport_driver_t *driver, link_data_t *args);
static void
boot_link_cmd_readserialno(link_transport_driver_t *driver, link_data_t *args);
static void boot_link_cmd_ioctl(link_transport_driver_t *driver, link_data_t *args);
static void boot_link_cmd_read(link_transport_driver_t *driver, link_data_t *args);

static void
boot_link_cmd_reset_bootloader(link_transport_driver_t *driver, link_data_t *args);
static void erase_flash(link_transport_driver_t *driver);
static void boot_link_cmd_reset(link_transport_driver_t *driver, link_data_t *args);

static const u8 *get_public_key() {
  return (const u8 *)((u32)sos_config.sys.secret_key_address & ~0x01);
}

void (*const boot_link_cmd_func_table[LINK_BOOTLOADER_CMD_TOTAL])(
  link_transport_driver_t *,
  link_data_t *) = {
  boot_link_cmd_none, boot_link_cmd_readserialno, boot_link_cmd_ioctl,
  boot_link_cmd_read};

void *boot_link_update(void *arg) {

  link_transport_driver_t *driver = arg;
  link_data_t data;

  dstr("open driver\n");
  if ((driver->handle = driver->open(0, 0)) == LINK_PHY_ERROR) {
    return 0;
  }

  data.op.cmd = 0;

  dstr("Enter update loop\n");
  while (1) {
    // Wait for data to arrive on the USB
    int err;
    do {

      if (
        (err = link_transport_slaveread(driver, &data.op, sizeof(link_op_t), NULL, NULL))
        < 0) {
        dstr("e:");
        dint(err);
        dstr("\n");
        continue;
      }

    } while (err < 0);

    dstr("EXEC CMD: ");
    dint(data.op.cmd);
    dstr("\n");
    if (data.op.cmd < LINK_BOOTLOADER_CMD_TOTAL) {
      data.reply.err_number = 0;
      data.reply.err = 0;
      boot_link_cmd_func_table[data.op.cmd](driver, &data);
    } else {
      data.reply.err = -1;
      data.reply.err_number = EINVAL;
    }

    // send the reply
    if (data.op.cmd != 0) {
      link_transport_slavewrite(driver, &data.reply, sizeof(data.reply), NULL, NULL);
      data.op.cmd = 0;
    }
  }
  return NULL;
}

void boot_link_cmd_none(link_transport_driver_t *driver, link_data_t *args) {
  MCU_UNUSED_ARGUMENT(driver);
  MCU_UNUSED_ARGUMENT(args);
  return;
}

void boot_link_cmd_readserialno(link_transport_driver_t *driver, link_data_t *args) {
  char serialno[LINK_PACKET_DATA_SIZE] = {};
  mcu_sn_t tmp = {0};
  int i, j;
  char *p = serialno;

  if (sos_config.sys.get_serial_number) {
    sos_config.sys.get_serial_number(&tmp);
  }

  for (j = 3; j >= 0; j--) {
    for (i = 0; i < 8; i++) {
      *p++ = htoc((tmp.sn[j] >> 28) & 0x0F);
      tmp.sn[j] <<= 4;
    }
  }

  args->reply.err = strnlen(serialno, 256);
  args->reply.err_number = 0;

  if (
    link_transport_slavewrite(driver, &args->reply, sizeof(args->reply), NULL, NULL)
    < 0) {
    return;
  }

  if (link_transport_slavewrite(driver, serialno, args->reply.err, NULL, NULL) < 0) {
    return;
  }

  args->op.cmd = 0; // reply was already sent

  sos_handle_event(SOS_EVENT_BOOT_READ_SERIAL_NUMBER, 0);
}

void boot_link_cmd_ioctl(link_transport_driver_t *driver, link_data_t *args) {
  int err;
  const int size = _IOCTL_SIZE(args->op.ioctl.request);
  bootloader_attr_t attr;
  bootloader_writepage_t wattr;
  static boot_event_flash_t event_args;

#if CONFIG_BOOT_IS_VERIFY_SIGNATURE
  const crypt_ecc_api_t *ecc_api =
    sos_config.sys.kernel_request_api(CRYPT_ECC_ROOT_API_REQUEST);
  const crypt_hash_api_t *sha_api =
    sos_config.sys.kernel_request_api(CRYPT_SHA256_ROOT_API_REQUEST);
#endif

  dstr("IOCTL REQ: ");
  dhex(args->op.ioctl.request);
  dstr("\n");

  switch (args->op.ioctl.request) {
  case I_BOOTLOADER_ERASE:
    dstr("erase\n");
    // the erase takes awhile -- so send the reply a little early
    link_transport_slavewrite(driver, &args->reply, sizeof(args->reply), NULL, NULL);
    // set this to zero so caller doesn't execute the slavewrite again
    args->op.cmd = 0;

    erase_flash(driver);
    is_erased = true;

    event_args.abort = 0;
    event_args.bytes = 0;
    event_args.total = -1;

    dstr("erd\n");
    return;

  case I_BOOTLOADER_GETINFO:
    // write data to io_buf
    dstr("info\n");
    attr.version = BCDVERSION;
    sos_config.sys.get_serial_number((mcu_sn_t *)attr.serialno);
    // mcu_core_getserialno((mcu_sn_t *)(attr.serialno));

    // attr.startaddr = boot_board_config.program_start_addr;
    attr.startaddr = sos_config.boot.program_start_address;
    // attr.hardware_id = boot_board_config.id;
    attr.hardware_id = cortexm_get_hardware_id();

    err = link_transport_slavewrite(driver, &attr, size, NULL, NULL);
    if (err == -1) {
      args->op.cmd = 0;
      args->reply.err = -1;
    } else {
      args->reply.err = 0;
    }

    break;

  case I_BOOTLOADER_RESET:
    dstr("rst\n");
    if (args->op.ioctl.arg == 0) {
      sos_handle_event(SOS_EVENT_BOOT_RESET, 0);
      boot_link_cmd_reset(driver, args);
    } else {
      sos_handle_event(SOS_EVENT_BOOT_RESET_BOOTLOADER, 0);
      boot_link_cmd_reset_bootloader(driver, args);
    }
    break;

  case I_BOOTLOADER_IS_SIGNATURE_REQUIRED:
#if CONFIG_BOOT_IS_VERIFY_SIGNATURE
    dstr("signature is required\n");
    args->reply.err = 1;
#else
    dstr("signature NOT required\n");
    args->reply.err = 0;
#endif
    break;

  case I_BOOTLOADER_WRITEPAGE:

    err = link_transport_slaveread(driver, &wattr, size, NULL, NULL);
    if (err < 0) {
      dstr("failed to read data\n");
      break;
    }

    dstr("w:");
    dhex(wattr.addr);
    dstr(":");
    dint(wattr.nbyte);
    dstr("\n");

    if (wattr.addr == sos_config.boot.program_start_address) {

      if (wattr.nbyte < sizeof(first_page)) {
        dstr("first page too small\n");
        // this is an error
        args->reply.err = -1;
        args->reply.err_number = EINVAL;
        return;
      }

#if CONFIG_BOOT_IS_VERIFY_SIGNATURE
      ecc_api = sos_config.sys.kernel_request_api(CRYPT_ECC_ROOT_API_REQUEST);
      sha_api = sos_config.sys.kernel_request_api(CRYPT_SHA256_ROOT_API_REQUEST);

      ecc_api->init(&ecc_context);
      sha_api->init(&sha_context);

      sha_api->start(sha_context);
      sha_api->update(sha_context, wattr.buf, wattr.nbyte);
      hash_size = wattr.nbyte;
#endif

      memcpy(first_page, wattr.buf, sizeof(first_page));
      wattr.addr += sizeof(first_page);
      wattr.nbyte = wattr.nbyte - sizeof(first_page);
      memcpy(wattr.buf, wattr.buf + sizeof(first_page), wattr.nbyte);

      args->reply.err =
        sos_config.boot.flash_write_page(&sos_config.boot.flash_handle, &wattr);

    } else {
#if CONFIG_BOOT_IS_VERIFY_SIGNATURE
      sha_api->update(sha_context, wattr.buf, wattr.nbyte);
      hash_size += wattr.nbyte;
#endif

      args->reply.err =
        sos_config.boot.flash_write_page(&sos_config.boot.flash_handle, &wattr);

      if (args->reply.err < 0) {
        dstr("Failed to write flash:");
        dhex(args->reply.err);
        dstr("\n");
      }
    }

    event_args.increment = wattr.nbyte;
    event_args.bytes += event_args.increment;
    sos_handle_event(SOS_EVENT_BOOT_WRITE_FLASH, &event_args);
    break;

  case I_BOOTLOADER_GET_PUBLIC_KEY: {
    auth_public_key_t key;
#if CONFIG_BOOT_IS_VERIFY_SIGNATURE
    memcpy(key.data, get_public_key(), sizeof(key.data));
#else
    memset(key.data, 0, sizeof(key.data));
#endif
    link_transport_slavewrite(driver, key.data, size, NULL, NULL);

    break;
  }

  case I_BOOTLOADER_VERIFY_SIGNATURE: {
#if CONFIG_BOOT_IS_VERIFY_SIGNATURE
    auth_signature_t signature;
    err = link_transport_slaveread(driver, signature.data, size, NULL, NULL);
    if (err < 0) {
      dstr("failed to receive signature\n");
      return;
    }
    u8 hash[32];
    sha_api->finish(sha_context, hash, sizeof(hash));
    dstr("hash:"); dint(hash_size); dstr(":");
    for (u32 i = 0; i < 32; i++) {
      dhex(hash[i]);
    }
    dstr("\n");

    const u8 *public_key = get_public_key();
    dstr("key:");
    for (u32 i = 0; i < 64; i++) {
      dhex(public_key[i]);
    }
    dstr("\n");
    dstr("signature:");
    for (u32 i = 0; i < 64; i++) {
      dhex(signature.data[i]);
    }
    dstr("\n");

    ecc_api->dsa_set_key_pair(
      ecc_context, public_key, sos_config.sys.secret_key_size, NULL, 0);

    const int is_verified = ecc_api->dsa_verify(
      ecc_context, hash, sizeof(hash), signature.data, sizeof(signature));
#else
    const int is_verified = 1;
#endif

    if (is_verified == 1) {
      // write first page to flash memory

      dstr("signature verified correctly\n");
      wattr.addr = sos_config.boot.program_start_address;
      wattr.nbyte = sizeof(first_page);
      memcpy(wattr.buf, first_page, sizeof(first_page));

      args->reply.err =
        sos_config.boot.flash_write_page(&sos_config.boot.flash_handle, &wattr);
    } else {
      dstr("image not signed\n");
    }
  } break;

  default:
    args->reply.err_number = EINVAL;
    args->reply.err = -1;
    break;
  }

  if (args->reply.err < 0) {
    args->reply.err_number = errno;
  }
}

void boot_link_cmd_read(link_transport_driver_t *driver, link_data_t *args) {
  args->reply.err = read_flash(driver, args->op.read.addr, args->op.read.nbyte);
  dint(args->reply.err);
  dstr("\n");
  return;
}

void boot_link_cmd_write(link_transport_driver_t *driver, link_data_t *args) {
  MCU_UNUSED_ARGUMENT(driver);
  MCU_UNUSED_ARGUMENT(args);
  args->reply.err = -1;
  return;
}

void erase_flash(link_transport_driver_t *driver) {
  int page = 0;
  int result;
  boot_event_flash_t args = {.abort = 0, .total = -1, .increment = -1};

  while (sos_config.boot.flash_erase_page(&sos_config.boot.flash_handle, (void *)page++)
         != 0) {
    // while (mcu_flash_erasepage(FLASH_PORT, (void *)page++) != 0) {
    // these are the bootloader pages and won't be erased

    sos_led_root_enable();
    driver->wait(1);
    sos_led_root_disable();
    args.bytes = page;
    sos_handle_event(SOS_EVENT_BOOT_ERASE_FLASH, &args);
  }
  page--;

  dstr("erase:");
  dint(page);
  dstr("\n");

  // erase the flash pages -- ends when an erase on an invalid page is attempted
  while ((result = sos_config.boot.flash_erase_page(
            &sos_config.boot.flash_handle, (void *)page++))
         == 0) {
    // while ((result = mcu_flash_erasepage(FLASH_PORT, (void *)page++)) == 0) {
    sos_led_svcall_enable(0);
    driver->wait(1);
    sos_led_svcall_disable(0);
    args.bytes = page;
    sos_handle_event(SOS_EVENT_BOOT_ERASE_FLASH, &args);
  }

  dstr("result:");
  dint(SYSFS_GET_RETURN_ERRNO(result));
  dstr("\n");

  sos_led_svcall_enable(0);
}

void boot_link_cmd_reset(link_transport_driver_t *driver, link_data_t *args) {
  sos_led_svcall_disable(0);
  u32 *dfu_sw_req;
  driver->wait(500);
  driver->close(&(driver->handle));
  dfu_sw_req = (u32 *)sos_config.boot.software_bootloader_request_address;
  *dfu_sw_req = 0;
  cortexm_reset(NULL);
  // the program never arrives here
}

void boot_link_cmd_reset_bootloader(link_transport_driver_t *driver, link_data_t *args) {
  u32 *dfu_sw_req;
  driver->wait(500);
  driver->close(&(driver->handle));
  dfu_sw_req = (u32 *)sos_config.boot.software_bootloader_request_address;
  *dfu_sw_req = sos_config.boot.software_bootloader_request_value;
  cortexm_reset(NULL);
  // the program never arrives here
}

static int mcu_flash_read(const devfs_handle_t *cfg, devfs_async_t *async) {
#if CONFIG_BOOT_IS_VERIFY_SIGNATURE
  memset(async->buf, 0, async->nbyte);
#else
  memcpy(async->buf, (const void *)async->loc, async->nbyte);
#endif
  return async->nbyte;
}

int read_flash_callback(void *context, void *buf, int nbyte) {
  int *loc;
  int ret;
  loc = context;
  ret = mcu_sync_io(&flash_dev, mcu_flash_read, *loc, buf, nbyte, O_RDWR);
  if (ret > 0) {
    *loc += ret;
  }
  return ret;
}

int read_flash(link_transport_driver_t *driver, int loc, int nbyte) {
  return link_transport_slavewrite(driver, NULL, nbyte, read_flash_callback, &loc);
}

/*! @} */
