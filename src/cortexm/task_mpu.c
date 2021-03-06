// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md

#include "cortexm/mpu.h"
#include "sos/sos.h"
#include "sos/symbols.h"
#include "task_local.h"

// returns 1 if target and size fit in task_memory
static int is_part_of_memory(void *target, int size, volatile task_memory_t *task_memory);

static void svcall_setstackguard(void *arg) MCU_ROOT_CODE;
static int init_os_memory_protection(task_memories_t *os_mem);
int task_mpu_calc_protection(task_memories_t *mem);

int task_validate_memory(void *target, int size) {

  // most likely
  if (is_part_of_memory(target, size, &sos_task_table[task_get_current()].mem.data)) {
    return 0;
  }

  // next most likely
  if (is_part_of_memory(target, size, &sos_task_table[task_get_current()].mem.code)) {
    return 0;
  }

  // part of shared kernel memory?
  if (is_part_of_memory(target, size, &sos_task_table[0].mem.data)) {
    return 0;
  }

  // part of kernel code (const data may be read)
  if (is_part_of_memory(target, size, &sos_task_table[0].mem.code)) {
    return 0;
  }

  // target and size overflow the memory
  return -1;
}

int is_part_of_memory(void *target, int size, volatile task_memory_t *task_memory) {
  u32 task_address = (u32)task_memory->address;
  u32 task_size = task_memory->size;
  u32 target_address = (u32)target;
  u32 target_size = (u32)size;
  if (
    (target_address >= task_address)
    && (target_address + target_size <= task_address + task_size)) {
    return 1;
  }
  return 0;
}

int task_init_mpu(void *system_memory, int system_memory_size) {
  task_memories_t os_mem;
  int err;

  // Calculate the device specific memory protection regions
  // mpu_dev_init();

  // disable all regions
  for (u32 i = 0; i < 16; i++) {
    mpu_disable_region(i);
  }

  // Turn the MPU On
  mpu_enable();

  u32 text_start = (u32)&_text & ~sos_config.sys.os_mpu_text_mask;

  // Memory Protection
  os_mem.code.address = (void *)text_start;
  os_mem.code.size = (char *)&_etext - (char *)text_start;
  os_mem.data.address = system_memory;
  os_mem.data.size = system_memory_size;
  err = init_os_memory_protection(&os_mem);
  if (err < 0) {
    return err;
  }

  // Calculate the memory protection scheme for the OS memory
  err =
    task_mpu_calc_protection(&os_mem); // This has to be after init_os_memory_protection()
                                       // because it modifies os_mem
  if (err < 0) {
    return err;
  }
  memcpy(
    (void *)&(sos_task_table[0].mem), &os_mem,
    sizeof(os_mem)); // Copy the OS mem to the task table

  sos_handle_event(SOS_EVENT_ROOT_MPU_INITIALIZED, NULL);
  sos_config.cache.enable();
  return 0;
}

typedef struct {
  int tid;
  void *stackaddr;
  int stacksize;
} root_setstackguard_t;

void svcall_setstackguard(void *arg) {
  CORTEXM_SVCALL_ENTER();
  root_setstackguard_t *p = arg;
  p->tid = task_root_set_stackguard(p->tid, p->stackaddr, p->stacksize);
}

int task_setstackguard(int tid, void *stackaddr, int stacksize) {
  root_setstackguard_t arg;
  arg.tid = tid;
  arg.stackaddr = stackaddr;
  arg.stacksize = stacksize;
  // security? args is written
  cortexm_svcall(svcall_setstackguard, &arg);
  return arg.tid;
}

int task_root_set_stackguard(int tid, void *stackaddr, int stacksize) {
  u32 newaddr;
  u32 rbar;
  u32 rasr;

  if ((u32)tid < task_get_total()) {
    int err;

    newaddr = (u32)stackaddr;
    newaddr = (newaddr & ~(stacksize - 1)) + stacksize; // align to the size of the guard

    err = mpu_calc_region(
      TASK_APPLICATION_STACK_GUARD_REGION, (void *)newaddr, stacksize, MPU_ACCESS_PRW,
      MPU_MEMORY_SRAM, false, &rbar, &rasr);

    if (err == 0) {
      return -1;
    }

  } else {
    return -1;
  }

  sos_task_table[tid].mem.stackguard.address = (void *)newaddr;
  sos_task_table[tid].mem.stackguard.size = stacksize;
  sos_task_table[tid].mem.stackguard.rbar = rbar;
  sos_task_table[tid].mem.stackguard.rasr = rasr;

  if (tid == task_get_current()) {
    // make the settings effective now if the task is currently active
    MPU->RBAR = rbar;
    MPU->RASR = rasr;
  }

  return 0;
}

int init_os_memory_protection(task_memories_t *os_mem) {
  int err;

#if 0
	err = mpu_dev_init();
	if ( err < 0 ){
		return err;
	}
#endif

  if (sos_config.sys.secret_key_size > 0) {
    err = mpu_enable_region(
      TASK_SYSTEM_SECRET_KEY_REGION,
      (void *)((u32)sos_config.sys.secret_key_address & ~0x01),
      sos_config.sys.secret_key_size, MPU_ACCESS_PR, MPU_MEMORY_FLASH, 0);
    if (err < 0) {
      sos_debug_log_error(
        SOS_DEBUG_SYS, "Failed to init OS secret key region 0x%lX to 0x%lX (%d)",
        sos_config.sys.secret_key_address, sos_config.sys.secret_key_size, err);
      return err;
    }
  }

  // Make OS System memory read-only -- region 0 -- highest priority
  err = mpu_enable_region(
    TASK_SYSTEM_STACK_MPU_REGION, &_sys, (char *)&_esys - (char *)&_sys,
    MPU_ACCESS_PRW_UR, MPU_MEMORY_SRAM, 0);
  if (err < 0) {
    sos_debug_log_error(
      SOS_DEBUG_SYS, "Failed to init OS read-only 0x%lX to 0x%lX (%d)", (u32)&_sys,
      (u32)&_esys, err);
    return err;
  }

  // Make the OS flash executable and readable
  err = mpu_enable_region(
    TASK_SYSTEM_CODE_MPU_REGION, os_mem->code.address, os_mem->code.size,
    MPU_ACCESS_PR_UR, MPU_MEMORY_FLASH, 1);
  if (err < 0) {
    sos_debug_log_error(
      SOS_DEBUG_SYS, "Failed to init OS flash 0x%lX -> 0x%ld bytes (%d)",
      (u32)os_mem->code.address, (u32)os_mem->code.size, err);
    return err;
  }

  // Make the OS shared memory R/W
  err = mpu_enable_region(
    TASK_SYSTEM_DATA_MPU_REGION, os_mem->data.address, os_mem->data.size,
    MPU_ACCESS_PRW_URW, MPU_MEMORY_SRAM, 0);
  if (err < 0) {
    sos_debug_log_error(
      SOS_DEBUG_SYS, "Failed to init shared mem 0x%lX -> 0x%lX bytes (%d)",
      (u32)os_mem->data.address, (u32)os_mem->data.size, err);
    return err;
  }

  return 0;
}

int task_mpu_calc_protection(task_memories_t *mem) {
  int err;
  mpu_memory_t mem_type;
  uint32_t rasr;
  uint32_t rbar;

  if (mem->code.address < (void *)&_data) {
    mem_type = MPU_MEMORY_FLASH;
  } else {
    mem_type = MPU_MEMORY_SRAM;
  }

  // Region 6
  err = mpu_calc_region(
    TASK_APPLICATION_CODE_MPU_REGION, mem->code.address, mem->code.size, MPU_ACCESS_PR_UR,
    mem_type, true, &rbar, &rasr);

  if (err == 0) {
    return -1;
  }

  mem->code.rbar = rbar;
  mem->code.rasr = rasr;

  // Region 7
  err = mpu_calc_region(
    TASK_APPLICATION_DATA_MPU_REGION, mem->data.address, mem->data.size,
    MPU_ACCESS_PRW_URW, MPU_MEMORY_SRAM, false, &rbar, &rasr);

  if (err == 0) {
    return -1;
  }

  mem->data.rbar = rbar;
  mem->data.rasr = rasr;

  return 0;
}
