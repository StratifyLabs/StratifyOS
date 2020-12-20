// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "sos/fs/devfs.h"
#include "mcu/core.h"

DEVFS_MCU_DRIVER_IOCTL_FUNCTION(core, CORE_VERSION, I_MCU_TOTAL + I_CORE_TOTAL, mcu_core_setpinfunc, mcu_core_setclkout, mcu_core_setclkdivide, mcu_core_getmcuboardconfig)
