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

#endif
