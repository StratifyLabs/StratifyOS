// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include <errno.h>
#include <limits.h>

#include "appfs_local.h"
#include "cortexm/util.h"
#include "sos/debug.h"
#include "sos/sos.h"

#include "../symbols.h"

#define APPFS_NAME_MAX_MINUS_1 (APPFS_NAME_MAX - 1)

#define APPFS_REWRITE_MASK 0xFE000000
#define APPFS_REWRITE_RAM_MASK (0x01000000)
#define APPFS_REWRITE_KERNEL_ADDR (0x01FF8000)
#define APPFS_REWRITE_KERNEL_ADDR_MASK (0x7FFF)

static int populate_file_header(
  const devfs_device_t *dev,
  appfs_file_t *file,
  const mem_pageinfo_t *page_info,
  int type);
static int
get_flash_page_type(const devfs_device_t *dev, u32 address, u32 size) MCU_ROOT_EXEC_CODE;
static int is_flash_blank(u32 address, u32 size) MCU_ROOT_EXEC_CODE;
static int
read_appfs_file_header(const devfs_device_t *dev, u32 address, appfs_file_t *dest)
  MCU_ROOT_EXEC_CODE;
static u32 find_protectable_addr(
  const devfs_device_t *dev,
  int size,
  int type,
  int *page,
  u32 *protectable_size,
  int skip_protection) MCU_ROOT_EXEC_CODE;
static u32 find_protectable_free(
  const devfs_device_t *dev,
  int type,
  int size,
  int *page,
  u32 *protectable_size,
  int skip_protection) MCU_ROOT_EXEC_CODE;

static int
check_for_free_space(const devfs_device_t *dev, int start_page, int type, int size)
  MCU_ROOT_EXEC_CODE;

static u8 calc_checksum(const char *name) MCU_ROOT_EXEC_CODE;

u8 calc_checksum(const char *name) {
  int i;
  u8 checksum;
  checksum = 0;
  for (i = 0; i < APPFS_NAME_MAX; i++) {
    checksum ^= name[i];
  }
  return checksum;
}

int is_flash_blank(u32 address, u32 size) {
  u32 *dest = (u32 *)address;
  for (u32 i = 0; i < size / sizeof(u32); i++) {
    if (*dest++ != 0xffffffff) {
      return 0;
    }
  }
  return 1;
}

int appfs_util_root_erase_pages(const devfs_device_t *dev, int start_page, int end_page) {
  int i;
  for (i = start_page; i <= end_page; i++) {
    sos_config.mcu.reset_watchdog_timer();
    int result = dev->driver.ioctl(&(dev->handle), I_MEM_ERASE_PAGE, (void *)i);
    if (result < 0) {
      return result;
    }
  }
  return 0;
}

int appfs_util_root_get_pageinfo(const devfs_device_t *dev, mem_pageinfo_t *pageinfo) {
  return dev->driver.ioctl(&(dev->handle), I_MEM_GETPAGEINFO, pageinfo);
}

int appfs_util_root_get_meminfo(const devfs_device_t *device, mem_info_t *mem_info) {
  return device->driver.ioctl(&(device->handle), I_MEM_GETINFO, mem_info);
}

void appfs_util_svcall_get_meminfo(void *args) {
  CORTEXM_SVCALL_ENTER();
  appfs_get_meminfo_t *p = args;
  p->result = appfs_util_root_get_meminfo(p->device, &p->mem_info);
}

void appfs_util_svcall_get_pageinfo(void *args) {
  CORTEXM_SVCALL_ENTER();
  appfs_get_pageinfo_t *p = args;
  p->result = appfs_util_root_get_pageinfo(p->device, &p->page_info);
}

void appfs_util_svcall_erase_pages(void *args) {
  CORTEXM_SVCALL_ENTER();
  appfs_erase_pages_t *p = args;
  p->result = appfs_util_root_erase_pages(p->device, p->start_page, p->end_page);
  sos_config.cache.invalidate_data_block((void *)p->start_address, p->size);
}

static u32
translate_value(u32 addr, u32 mask, u32 code_start, u32 data_start, u32 total, s32 *loc) {
  // check if the value is an address
  u32 ret = addr;
  *loc = 0;
  if ((addr & APPFS_REWRITE_MASK) == mask) { // matches Text or Data
    ret = addr & ~(APPFS_REWRITE_MASK | APPFS_REWRITE_RAM_MASK);
    if (
      ((addr & APPFS_REWRITE_KERNEL_ADDR) == APPFS_REWRITE_KERNEL_ADDR)
      && ((addr - 1) % 4 == 0) // if the value is not aligned, it shouldn't be translated
    ) {
      // This is a kernel value
      ret = (addr & APPFS_REWRITE_KERNEL_ADDR_MASK)
            >> 2; // convert the address to a table index value
      if (ret < total) {
        // get the symbol location from the symbols table
        if (symbols_table[ret] == 0) {
          sos_debug_log_error(
            SOS_DEBUG_APPFS, "symbol at offset %d (%p) is zero", ret,
            symbols_table + ret);
          *loc = ret; // this symbol isn't available -- it was removed to save space in
                      // the MCU flash
        }
        return symbols_table[ret];
      } else {
        sos_debug_log_error(
          SOS_DEBUG_APPFS, "location exceeds total for %p (%d)", addr, total);
        *loc = total;
        return 0;
      }
    } else if (addr & APPFS_REWRITE_RAM_MASK) {
      ret += data_start;
    } else {
      ret += code_start;
    }
  }
  return ret;
}

u32 find_protectable_addr(
  const devfs_device_t *dev,
  int size,
  int type,
  int *page,
  u32 *protectable_size,
  int skip_protection) {
  int i;
  u32 tmp_rbar;
  u32 tmp_rasr;
  mem_pageinfo_t pageinfo;
  int err;
  int mem_type;
  u32 region_size;

  if (type & MEM_FLAG_IS_FLASH) {
    mem_type = MPU_MEMORY_FLASH;
  } else {
    mem_type = MPU_MEMORY_SRAM;
  }

  i = *page;
  do {
    // go through each page
    pageinfo.num = i;
    pageinfo.o_flags = type;
    if (appfs_util_root_get_pageinfo(dev, &pageinfo) < 0) {
      return (u32)-1;
    }

    if (pageinfo.o_flags == (u32)type) { // type should be an exact match with the page
                                         // info
                                         // flags (external, tightly coupled or normal)

      if (skip_protection) {
        err = 0;
      } else {

        // this will return 0 if the address and size is actually protectable
        region_size = mpu_calc_region(
          TASK_APPLICATION_CODE_MPU_REGION, // doesn't matter what this is
          (void *)pageinfo.addr, size, MPU_ACCESS_PR_UR, mem_type, true, &tmp_rbar,
          &tmp_rasr);

        if (region_size == 0) {
          err = -1;
        } else {
          err = 0;
        }
      }

      if (err == 0) {
        *page = i;
        *protectable_size = region_size;
        return pageinfo.addr;
      }
    }

    i++;
  } while (1);

  return (u32)-1;
}

int check_for_free_space(const devfs_device_t *dev, int start_page, int type, int size) {
  mem_pageinfo_t page_info;
  int free_size;
  int last_addr;
  int last_size;
  int ret;

  page_info.o_flags = type;
  page_info.num = start_page;

  free_size = 0;
  last_addr = -1;
  last_size = -1;


  do {

    appfs_util_root_get_pageinfo(dev, &page_info);

    if (
      (last_addr != -1) && // make sure last_addr is initialized
      (last_addr + last_size != (int)page_info.addr)) {
      // memory is not contiguous
      if (size <= free_size) {
        return free_size;
      } else {
        return -1;
      }
    }

    last_addr = page_info.addr;
    last_size = page_info.size;
    if (type == MEM_FLAG_IS_FLASH) {
      // load the file info
      ret = get_flash_page_type(dev, page_info.addr, page_info.size);
    } else {
      // get the type from the ram usage table
      ret = appfs_ram_root_get(dev, page_info.num);
    }

    if (ret == APPFS_MEMPAGETYPE_FREE) {
      free_size += page_info.size;
    } else {
      if (size <= free_size) {
        return free_size;
      } else {
        return -1;
      }
    }
    page_info.num++;
  } while (ret >= 0);

  return free_size;
}

u32 find_protectable_free(
  const devfs_device_t *dev,
  int type,
  int size,
  int *page,
  u32 *protectable_size,
  int skip_protection) {
  u32 start_addr;
  int space_available;

  // find any area for the code
  *page = 0;

  do {
    start_addr =
      find_protectable_addr(dev, size, type, page, protectable_size, skip_protection);

    if (start_addr != (u32)-1) {
      // could not find any free space

      space_available = check_for_free_space(dev, *page, type, *protectable_size);
      if (space_available > 0) {
#if 0
        // there is room here -- find the smallest free space where the program fits
        space_available = tmp;
        smallest_space_addr = start_addr;
        smallest_space_page = *page;
        if (size == space_available) { // this is a perfect fit
          return smallest_space_addr;
        }
#else
        return start_addr;
#endif
      }

      (*page)++;
    }

  } while (start_addr != (u32)-1);

  *page = 0;

  // return an invalid address
  return (u32)-1;
}

const appfs_file_t *appfs_util_getfile(appfs_handle_t *h) {
  return (appfs_file_t *)h->type.reg.beg_addr;
}

int appfs_util_root_free_ram(const devfs_device_t *dev, appfs_handle_t *h) {
  const appfs_file_t *f;

  if (h->is_install != 0) {
    return SYSFS_SET_RETURN(EBADF);
  }

  // the RAM info is stored in flash
  f = appfs_util_getfile(h);

  mem_pageinfo_t page_info;
  page_info.addr = f->exec.ram_start;
  page_info.o_flags = MEM_FLAG_IS_QUERY;

  if (appfs_util_root_get_pageinfo(dev, &page_info) < 0) {
    return SYSFS_SET_RETURN(EINVAL);
  }

  appfs_ram_root_set(dev, page_info.num, f->exec.ram_size, APPFS_MEMPAGETYPE_FREE);

  return 0;
}

int appfs_util_root_reclaim_ram(const devfs_device_t *dev, appfs_handle_t *h) {
  const appfs_file_t *f;
  size_t s;
  size_t page_num;
  mem_pageinfo_t page_info;

  if (h->is_install) {
    return SYSFS_SET_RETURN(EBADF);
  }

  // the RAM info is store in flash
  f = appfs_util_getfile(h);

  page_info.addr = f->exec.ram_start;
  page_info.o_flags = MEM_FLAG_IS_QUERY;

  if (appfs_util_root_get_pageinfo(dev, &page_info)) {
    return SYSFS_SET_RETURN(EIO);
  }

  page_num = page_info.num;

  for (s = 0; s < f->exec.ram_size; s += MCU_RAM_PAGE_SIZE) {
    if (appfs_ram_root_get(dev, page_num++) != APPFS_MEMPAGETYPE_FREE) {
      return SYSFS_SET_RETURN(ENOMEM);
    }
  }

  appfs_ram_root_set(dev, page_info.num, f->exec.ram_size, APPFS_MEMPAGETYPE_SYS);

  return 0;
}

int appfs_util_root_mem_write_page(
  const devfs_device_t *dev,
  appfs_handle_t *h,
  appfs_installattr_t *attr) {
  // now write the buffer
  mem_writepage_t write_page;

  if (
    (attr->loc + attr->nbyte) > (h->type.install.code_size + h->type.install.data_size)) {
    return SYSFS_SET_RETURN(EINVAL);
  }

  write_page.addr = h->type.install.code_start + attr->loc;
  write_page.nbyte = attr->nbyte;
  memcpy(write_page.buf, attr->buffer, 256);
  int result = dev->driver.ioctl(&(dev->handle), I_MEM_WRITEPAGE, &write_page);
  if (result < 0) {
    sos_debug_log_error(
      SOS_DEBUG_APPFS, "failed to write page (%d, %d)", SYSFS_GET_RETURN(result),
      SYSFS_GET_RETURN_ERRNO(result));
  }
  return result;
}

int appfs_util_root_create(
  const devfs_device_t *dev,
  appfs_handle_t *h,
  appfs_installattr_t *attr) {
  u32 code_start_addr;
  int page;
  appfs_file_t *dest;
  dest = (appfs_file_t *)attr->buffer;

  if (h->is_install == false) {
    return SYSFS_SET_RETURN(EBADF);
  }

  if (attr->loc == 0) {

    // This is the header data -- make sure it is complete
    if (attr->nbyte < sizeof(appfs_file_t)) {
      return SYSFS_SET_RETURN(ENOTSUP);
    }

    if (dest->exec.signature != APPFS_CREATE_SIGNATURE) {
      return SYSFS_SET_RETURN(EINVAL);
    }

    // make sure the name is valid
    int len = strnlen(dest->hdr.name, APPFS_NAME_MAX_MINUS_1);
    if (len == (APPFS_NAME_MAX_MINUS_1)) {
      // truncate the name if it is too long
      dest->hdr.name[APPFS_NAME_MAX] = 0;
      dest->hdr.name[APPFS_NAME_MAX_MINUS_1] = 0;
    }

    // add a checksum to the name
    dest->hdr.name[APPFS_NAME_MAX] = calc_checksum(dest->hdr.name);

    // set the mode to read only
    dest->hdr.mode = 0444;

    // check the options
    dest->exec.o_flags = APPFS_FLAG_IS_FLASH;
    int type = MEM_FLAG_IS_FLASH;

    u32 protectable_size;

    // find space for the code -- this doesn't need to be protectable
    code_start_addr =
      find_protectable_free(dev, type, dest->exec.code_size, &page, &protectable_size, 0);
    if (code_start_addr == (u32)-1) {
      return SYSFS_SET_RETURN(ENOSPC);
    }

    // remove the header for read only flash files
    dest->exec.startup = 0;
    dest->exec.code_start = code_start_addr;
    dest->exec.ram_start = 0;
    dest->exec.ram_size = 0;
    dest->exec.data_size = 0;
    dest->exec.o_flags = 0;
    dest->exec.startup = 0;

    h->type.install.code_start = (u32)code_start_addr;
    h->type.install.code_size = dest->exec.code_size;
    h->type.install.data_start = 0;
    h->type.install.data_size = 0;

    h->type.install.rewrite_mask = 0;
    h->type.install.kernel_symbols_total = 0;

  } else {
    if ((attr->loc & 0x03)) {
      // this is not a word aligned write
      return SYSFS_SET_RETURN(EINVAL);
    }
  }

  // now write the buffer
  return appfs_util_root_mem_write_page(dev, h, attr);
}

int appfs_util_root_writeinstall(
  const devfs_device_t *dev,
  appfs_handle_t *h,
  appfs_installattr_t *attr) {
  union {
    const appfs_file_t *file;
    const u32 *ptr;
  } src;
  u32 code_start_addr;
  u32 data_start_addr;
  int code_page;
  int ram_page;
  s32 loc_err = 0;

  if (h->is_install == false) {
    return SYSFS_SET_RETURN(EBADF);
  }

  union {
    appfs_file_t file;
    u32 buf[APPFS_PAGE_SIZE / sizeof(u32)];
  } dest;

  src.ptr = (const u32 *)attr->buffer;

  if (attr->loc == 0) {
    // This is the header data -- make sure it is complete
    if (attr->nbyte < sizeof(appfs_file_t)) {
      sos_debug_log_error(SOS_DEBUG_APPFS, "Page size is less than min");
      return SYSFS_SET_RETURN(ENOTSUP);
    }

    // make sure the name is valid
    {
      dest.file = (appfs_file_t){};
      memcpy(&dest.file, src.file, attr->nbyte);
      const size_t name_len = strnlen(dest.file.hdr.name, APPFS_NAME_MAX_MINUS_1);
      if (name_len == (APPFS_NAME_MAX_MINUS_1)) {
        // truncate the name if it is too long
        dest.file.hdr.name[APPFS_NAME_MAX_MINUS_1] = 0;
      }
    }

    {
      const size_t file_len = strnlen(dest.file.hdr.id, APPFS_NAME_MAX_MINUS_1);
      if (file_len == (APPFS_NAME_MAX_MINUS_1)) {
        // truncate the name if it is too long
        dest.file.hdr.id[APPFS_NAME_MAX_MINUS_1] = 0;
      }
    }

    // add a checksum to the name
    dest.file.hdr.name[APPFS_NAME_MAX] = calc_checksum(dest.file.hdr.name);
    dest.file.hdr.id[APPFS_NAME_MAX] = calc_checksum(dest.file.hdr.id);

    // set mode to read/exec
    dest.file.hdr.mode = 0555;

    // is signature correct
    if (appfs_util_is_executable(&src.file->exec) == 0) {
      sos_debug_log_error(SOS_DEBUG_APPFS, "Not executable");
      return SYSFS_SET_RETURN(ENOEXEC);
    }

    // check the options
    const int type = ((src.file->exec.o_flags) & APPFS_FLAG_IS_FLASH) ? MEM_FLAG_IS_FLASH
                                                                      : MEM_FLAG_IS_RAM;

    const int code_size =
      src.file->exec.code_size + src.file->exec.data_size; // code plus initialized data
    int ram_size = src.file->exec.ram_size;
    u32 protectable_size;
    // find space for the code
    code_start_addr = (u32)-1;

    sos_debug_log_info(SOS_DEBUG_APPFS, "Install flags 0x%lX", src.file->exec.o_flags);

    // check for external or tightly coupled flags and see if memory is available in those
    // regions
    if (src.file->exec.o_flags & APPFS_FLAG_IS_DATA_EXTERNAL) {
      code_start_addr = find_protectable_free(
        dev, type | MEM_FLAG_IS_EXTERNAL, code_size, &code_page, &protectable_size, 0);
    } else if (src.file->exec.o_flags & APPFS_FLAG_IS_DATA_TIGHTLY_COUPLED) {
      code_start_addr = find_protectable_free(
        dev, type | MEM_FLAG_IS_TIGHTLY_COUPLED, code_size, &code_page, &protectable_size,
        0);
    }

    // if failed to find external or tightly coupled memory -- try just regular memory
    if (code_start_addr == (u32)-1) {
      code_start_addr =
        find_protectable_free(dev, type, code_size, &code_page, &protectable_size, 0);

      if (code_start_addr == (u32)-1) { // try other memory types if default is not
                                        // available
        code_start_addr = find_protectable_free(
          dev, type | MEM_FLAG_IS_EXTERNAL, code_size, &code_page, &protectable_size, 0);
        if (code_start_addr == (u32)-1) {
          code_start_addr = find_protectable_free(
            dev, type | MEM_FLAG_IS_TIGHTLY_COUPLED, code_size, &code_page,
            &protectable_size, 0);
        }
      }
    }

    if (code_start_addr == (u32)-1) {
      sos_debug_log_error(SOS_DEBUG_APPFS, "No exec region available");
      return SYSFS_SET_RETURN(ENOSPC);
    }
    sos_debug_log_info(
      SOS_DEBUG_APPFS, "Code start addr is %p:%d", code_start_addr, protectable_size);

    if (!((src.file->exec.o_flags)
          & APPFS_FLAG_IS_FLASH)) { // for RAM app's mark the RAM usage
      // mark the range as SYS
      appfs_ram_root_set(dev, code_page, protectable_size, APPFS_MEMPAGETYPE_SYS);

      // mark the first page as USER
      appfs_ram_root_set(
        dev, code_page,
        MCU_RAM_PAGE_SIZE, // mark the first page as USER
        APPFS_MEMPAGETYPE_USER);
    }

    ram_page = 0;
    data_start_addr = (u32)-1;
    // check to see if more specific memory is available (external or tightly coupled)
    if (src.file->exec.o_flags & APPFS_FLAG_IS_DATA_EXTERNAL) {
      data_start_addr = find_protectable_free(
        dev, MEM_FLAG_IS_RAM | MEM_FLAG_IS_EXTERNAL, ram_size, &ram_page,
        &protectable_size, 0);
    } else if (src.file->exec.o_flags & APPFS_FLAG_IS_DATA_TIGHTLY_COUPLED) {
      data_start_addr = find_protectable_free(
        dev, MEM_FLAG_IS_RAM | MEM_FLAG_IS_TIGHTLY_COUPLED, ram_size, &ram_page,
        &protectable_size, 0);
    }

    // if no external/tightly coupled memory available (or not specified) then just
    // install in regular RAM
    if (data_start_addr == (u32)-1) {
      data_start_addr = find_protectable_free(
        dev, MEM_FLAG_IS_RAM, ram_size, &ram_page, &protectable_size, 0);

      if (data_start_addr == (u32)-1) { // try other memory types if default is not
                                        // available
        data_start_addr = find_protectable_free(
          dev, MEM_FLAG_IS_RAM | MEM_FLAG_IS_EXTERNAL, ram_size, &ram_page,
          &protectable_size, 0);
        if (data_start_addr == (u32)-1) {
          data_start_addr = find_protectable_free(
            dev, MEM_FLAG_IS_RAM | MEM_FLAG_IS_TIGHTLY_COUPLED, ram_size, &ram_page,
            &protectable_size, 0);
        }
      }
    }

    if (data_start_addr == (u32)-1) {
      if (!((src.file->exec.o_flags)
            & APPFS_FLAG_IS_FLASH)) { // for RAM app's mark the RAM usage
        // free the code section
        appfs_ram_root_set(dev, code_page, protectable_size, APPFS_MEMPAGETYPE_FREE);
      }
      sos_debug_log_error(SOS_DEBUG_APPFS, "No RAM region available %d", ram_size);
      return SYSFS_SET_RETURN(ENOSPC);
    }
    sos_debug_log_info(
      SOS_DEBUG_APPFS, "Data start addr is %p:%d", data_start_addr, protectable_size);

    h->type.install.code_start = (u32)code_start_addr;
    h->type.install.code_size = code_size;
    h->type.install.data_start = (u32)data_start_addr;
    h->type.install.data_size = src.file->exec.data_size;

    h->type.install.rewrite_mask = (u32)(src.file->exec.code_start) & APPFS_REWRITE_MASK;
    h->type.install.kernel_symbols_total = symbols_total();

    appfs_ram_root_set(dev, ram_page, ram_size, APPFS_MEMPAGETYPE_SYS);

    dest.file.exec.code_start = code_start_addr;
    dest.file.exec.ram_start = data_start_addr;
    dest.file.exec.ram_size = protectable_size;

    sos_debug_log_info(
      SOS_DEBUG_APPFS, "code startup is at %p (%p %p 0x%X)", dest.file.exec.startup,
      h->type.install.code_start, h->type.install.data_start,
      h->type.install.rewrite_mask);
    dest.file.exec.startup = translate_value(
      (u32)dest.file.exec.startup, h->type.install.rewrite_mask,
      h->type.install.code_start, h->type.install.data_start,
      h->type.install.kernel_symbols_total, &loc_err);

    sos_debug_log_info(
      SOS_DEBUG_APPFS, "code startup is translated to at %p", dest.file.exec.startup);

    for (u32 i = sizeof(appfs_file_t) >> 2; i < (attr->nbyte >> 2); i++) {
      dest.buf[i] = translate_value(
        src.ptr[i], h->type.install.rewrite_mask, h->type.install.code_start,
        h->type.install.data_start, h->type.install.kernel_symbols_total, &loc_err);
      if (loc_err != 0) {
        sos_debug_log_error(SOS_DEBUG_APPFS, "Code relocation error: %d", loc_err);
        return SYSFS_SET_RETURN_WITH_VALUE(EIO, loc_err);
      }
    }

#if CONFIG_APPFS_IS_VERIFY_SIGNATURE
    // if verifying the signature, the first page is cached
    // until the signature is verified
    memcpy(attr->buffer, &dest, attr->nbyte);
    h->type.install.first_page = *attr;
    return attr->nbyte;
#endif

  } else {

    if ((attr->loc & 0x03)) {
      // this is not a word aligned write
      sos_debug_log_error(SOS_DEBUG_APPFS, "word alignment error 0x%X\n", attr->loc);
      return SYSFS_SET_RETURN(EINVAL);
    }
    for (u32 i = 0; i < (attr->nbyte >> 2); i++) {
      dest.buf[i] = translate_value(
        src.ptr[i], h->type.install.rewrite_mask, h->type.install.code_start,
        h->type.install.data_start, h->type.install.kernel_symbols_total, &loc_err);
      if (loc_err != 0) {
        sos_debug_log_error(SOS_DEBUG_APPFS, "Code relocation error %d", loc_err);
        return SYSFS_SET_RETURN_WITH_VALUE(EIO, loc_err);
      }
    }
  }

  memcpy(attr->buffer, &dest, attr->nbyte);

  // now write buffer
  return appfs_util_root_mem_write_page(dev, h, attr);
}

int get_flash_page_type(const devfs_device_t *dev, u32 address, u32 size) {
  appfs_file_t appfs_file;

  if (is_flash_blank(address, size)) {
    return APPFS_MEMPAGETYPE_FREE;
  }

  read_appfs_file_header(dev, address, &appfs_file);
  u32 len = strnlen(appfs_file.hdr.name, APPFS_NAME_MAX_MINUS_1);
  if (
    (len == APPFS_NAME_MAX_MINUS_1) || // check if the name is short enough
    (len < APPFS_NAME_MAX_MINUS_1 && (len != strspn(appfs_file.hdr.name, sysfs_validset)))
    || // check if only valid characters are present
    (appfs_file.hdr.name[APPFS_NAME_MAX] != calc_checksum(appfs_file.hdr.name))
    || // check for a valid checksum
    (len == 0)) {

    return APPFS_MEMPAGETYPE_SYS;
  }

  return APPFS_MEMPAGETYPE_USER;
}

int appfs_util_is_executable(const appfs_exec_t *exec) {
  // do a priv read of the signature
  const int device_arch = symbols_table[0] & 0x0f;
  const int app_arch = exec->signature & 0x0f;
  if (app_arch > device_arch) {
    return 0;
  }

  if (exec->signature > symbols_table[0]) {
    return 0;
  }
  return 1;
}

int appfs_util_get_data_mpu_type(const appfs_file_t *file) {
  const u32 flags = file->exec.o_flags;

  if (flags & APPFS_FLAG_IS_DATA_EXTERNAL) {
    return MPU_MEMORY_EXTERNAL_SRAM;
  }

  if (flags & APPFS_FLAG_IS_DATA_TIGHTLY_COUPLED) {
    return MPU_MEMORY_TIGHTLY_COUPLED_DATA;
  }

  return MPU_MEMORY_SRAM;
}

int appfs_util_get_code_mpu_type(const appfs_file_t *file) {
  const u32 flags = file->exec.o_flags;

  if (flags & APPFS_FLAG_IS_CODE_EXTERNAL) {
    return MPU_MEMORY_EXTERNAL_FLASH;
  }

  if (flags & APPFS_FLAG_IS_CODE_TIGHTLY_COUPLED) {
    return MPU_MEMORY_TIGHTLY_COUPLED_INSTRUCTION;
  }

  return MPU_MEMORY_FLASH;
}

int populate_file_header(
  const devfs_device_t *device,
  appfs_file_t *file,
  const mem_pageinfo_t *page_info,
  int memory_type) {
  char hex_num[9];
  int page_type;

  if (memory_type == MEM_FLAG_IS_FLASH) {
    page_type = get_flash_page_type(device, page_info->addr, page_info->size);
  } else {
    page_type = appfs_ram_root_get(device, page_info->num);
  }

  htoa(hex_num, page_info->num);
  switch (page_type) {
  case APPFS_MEMPAGETYPE_SYS:
    strcpy(file->hdr.name, ".sys");
    strncat(file->hdr.name, hex_num, APPFS_NAME_MAX_MINUS_1);
    file->hdr.mode = S_IFREG | 0444;
    file->exec = (appfs_exec_t){};
    break;
  case APPFS_MEMPAGETYPE_FREE:
    strcpy(file->hdr.name, ".free");
    strncat(file->hdr.name, hex_num, APPFS_NAME_MAX_MINUS_1);
    file->hdr.mode = S_IFREG | 0444;
    file->exec = (appfs_exec_t){};
    break;
  case APPFS_MEMPAGETYPE_USER:
    break;
  }

  return page_type;
}

void appfs_util_svcall_get_fileinfo(void *args) {
  CORTEXM_SVCALL_ENTER();
  appfs_get_fileinfo_t *p = args;
  p->result =
    appfs_util_root_get_fileinfo(p->device, &p->file_info, p->page, p->type, &p->size);
}

int read_appfs_file_header(const devfs_device_t *dev, u32 address, appfs_file_t *dest) {
  // now that addr is available -- read the address
  devfs_async_t async;

  async.buf = dest;
  async.nbyte = sizeof(appfs_file_t);
  async.handler.context = NULL;
  async.loc = (int)address;
  async.tid = task_get_current();

  // read the memory directly to get the file header
  return dev->driver.read(&dev->handle, &async);
}

int appfs_util_root_get_fileinfo(
  const devfs_device_t *dev,
  appfs_file_t *file_info,
  int page,
  int type,
  int *size) {
  int file_type;
  int result;
  mem_pageinfo_t page_info;
  page_info.num = page;
  page_info.o_flags = type;

  // this will get the size and address of the page
  result = appfs_util_root_get_pageinfo(dev, &page_info);
  if (result < 0) {
    return result;
  }

  result = read_appfs_file_header(dev, page_info.addr, file_info);
  if (result < 0) {
    return result;
  }

  // get the header info for free and sys files
  file_type = populate_file_header(dev, file_info, &page_info, type);

  if (size != NULL) {
    if (file_type == APPFS_MEMPAGETYPE_USER) {
      *size = file_info->exec.code_size + file_info->exec.data_size;
    } else {
      *size = page_info.size;
    }
  }

  return file_type;
}

int appfs_util_lookupname(
  const devfs_device_t *device,
  const char *path,
  appfs_file_t *file_info,
  mem_pageinfo_t *page_info,
  int type,
  int *size) {
  appfs_get_fileinfo_t get_fileinfo_args;
  appfs_get_pageinfo_t get_pageinfo_args;

  get_fileinfo_args.device = device;
  get_fileinfo_args.type = type;

  get_pageinfo_args.device = device;
  get_pageinfo_args.page_info.o_flags = type;

  if (strnlen(path, APPFS_NAME_MAX_MINUS_1) == APPFS_NAME_MAX_MINUS_1) {
    return -1;
  }

  get_fileinfo_args.page = 0;
  do {
    // go through each page

    cortexm_svcall(appfs_util_svcall_get_fileinfo, &get_fileinfo_args);

    if (get_fileinfo_args.result < 0) {
      return get_fileinfo_args.result;
    }

    if (size) {
      *size = get_fileinfo_args.size;
    }
    get_pageinfo_args.page_info.num = get_fileinfo_args.page;
    cortexm_svcall(appfs_util_svcall_get_pageinfo, &get_pageinfo_args);
    if (get_pageinfo_args.result < 0) {
      return get_fileinfo_args.result;
    }

    if (
      strncmp(
        path, get_fileinfo_args.file_info.hdr.name,
        sizeof(get_fileinfo_args.file_info.hdr.name) - 1)
      == 0) {
      *file_info = get_fileinfo_args.file_info;
      *page_info = get_pageinfo_args.page_info;
      return 0;
    }

    get_fileinfo_args.page++;

  } while (1);

  // name not found
  return -1;
}
