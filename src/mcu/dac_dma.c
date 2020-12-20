// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "sos/fs/devfs.h"
#include "mcu/dac.h"


DEVFS_MCU_DRIVER_IOCTL_FUNCTION(dac_dma, DAC_VERSION, I_MCU_TOTAL + I_DAC_TOTAL, mcu_dac_dma_get, mcu_dac_dma_set)


