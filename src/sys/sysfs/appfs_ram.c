// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "appfs_local.h"
#include "cortexm/cortexm.h"
#include "cortexm/mpu.h"
#include "sos/sos.h"

static void set_page_usage(u32 *usage, u32 page, int type);
static int get_page_usage(u32 *usage, u32 page);

int appfs_ram_root_get(const devfs_device_t *dev, u32 page) {
  appfs_ram_t ram;
  ram.device = dev;
  ram.page = page;
  appfs_ram_svcall_get(&ram);
  return ram.type;
}

void appfs_ram_root_set(const devfs_device_t *dev, u32 page, u32 size, int type) {
  appfs_ram_t ram;
  ram.device = dev;
  ram.page = page;
  ram.size = size;
  ram.type = type;
  appfs_ram_svcall_set(&ram);
}

void appfs_ram_root_init(const devfs_device_t *dev) {
  mem_info_t info;
  dev->driver.ioctl(&dev->handle, I_MEM_GETINFO, &info);
  memset(info.usage, 0, info.usage_size);
}

void appfs_ram_svcall_set(void *args) {
  CORTEXM_SVCALL_ENTER();
  u32 i;
  u32 pages;
  mem_info_t info;
  appfs_ram_t *p = args;
  p->device->driver.ioctl(&p->device->handle, I_MEM_GETINFO, &info);

  pages = (p->size + MCU_RAM_PAGE_SIZE - 1) / MCU_RAM_PAGE_SIZE;

  for (i = p->page; i < (p->page + pages); i++) {
    if (p->page < info.ram_pages) {
      set_page_usage(info.usage, i, p->type);
    }
  }
}

void appfs_ram_svcall_get(void *args) {
  mem_info_t info;
  appfs_ram_t *p = args;
  p->device->driver.ioctl(&p->device->handle, I_MEM_GETINFO, &info);
  if (p->page < info.ram_pages) {
    p->type = get_page_usage(info.usage, p->page);
  } else {
    p->type = APPFS_MEMPAGETYPE_INVALID;
  }
}

int get_page_usage(u32 *usage, u32 page) {
  u32 block;
  u32 shift;
  block = page >> 4;
  shift = (page & 0xF) * 2;
  return ((usage[block] >> (shift)) & APPFS_MEMPAGETYPE_MASK);
}

void set_page_usage(u32 *usage, u32 page, int type) {
  u32 block;
  u32 shift;
  block = page >> 4;
  shift = (page & 0xF) * 2;
  usage[block] &= (u32) ~(APPFS_MEMPAGETYPE_MASK << (shift));        // clear the bits
  usage[block] |= (u32)((type & APPFS_MEMPAGETYPE_MASK) << (shift)); // set the bits
}
