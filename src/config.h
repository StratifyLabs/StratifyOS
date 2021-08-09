/*! \file */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "sos/debug.h"

#define BASE_VERSION (MCU_STRINGIFY(__PROJECT_VERSION_MAJOR) "." MCU_STRINGIFY(                \
  __PROJECT_VERSION_MINOR) "." MCU_STRINGIFY(__PROJECT_VERSION_PATCH))

#if MCU_DEBUG
#define VERSION (BASE_VERSION "d")
#else
#define VERSION (BASE_VERSION)
#endif

#include <stdint.h>

#include "sos/arch.h"

//this file is the user provided configuration file for making
//a few compile time tweaks
#include "sos_config.h"


//If the chip has double precision floating point and only 8 sections
//this needs to be set to zero
#if !defined CONFIG_TASK_MPU_REGION_OFFSET
#define CONFIG_TASK_MPU_REGION_OFFSET 8
#endif


#if defined __v7em_f5dh
#define TASK_MPU_REGION_OFFSET CONFIG_TASK_MPU_REGION_OFFSET
#else
#define TASK_MPU_REGION_OFFSET 0
#endif

// higher numbers take precedence over lower numbers
#define TASK_SYSTEM_SECRET_KEY_REGION (7+TASK_MPU_REGION_OFFSET)
#define TASK_SYSTEM_STACK_MPU_REGION (6+TASK_MPU_REGION_OFFSET)
#define TASK_SYSTEM_CODE_MPU_REGION (5+TASK_MPU_REGION_OFFSET)
#define TASK_SYSTEM_DATA_MPU_REGION (4+TASK_MPU_REGION_OFFSET)
// stack guard must have higher precedence than application data because they overlap
#define TASK_APPLICATION_STACK_GUARD_REGION (3+TASK_MPU_REGION_OFFSET)
#define TASK_APPLICATION_CODE_MPU_REGION (2+TASK_MPU_REGION_OFFSET)
#define TASK_APPLICATION_DATA_MPU_REGION (1+TASK_MPU_REGION_OFFSET)
#define TASK_APPLICATION_DATA_USER_REGION (0+TASK_MPU_REGION_OFFSET)

#endif
