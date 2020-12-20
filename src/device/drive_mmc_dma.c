// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "mcu/mmc.h"

#define mcu_mmc_open mcu_mmc_dma_open
#define mcu_mmc_close mcu_mmc_dma_close
#define mcu_mmc_setaction mcu_mmc_dma_setaction
#define mcu_mmc_getinfo mcu_mmc_dma_getinfo
#define mcu_mmc_setattr mcu_mmc_dma_setattr
#define mcu_mmc_write mcu_mmc_dma_write
#define mcu_mmc_read mcu_mmc_dma_read
#define mcu_mmc_ioctl mcu_mmc_dma_ioctl
#define mcu_mmc_getcsd mcu_mmc_dma_getcsd
#define mcu_mmc_getcid mcu_mmc_dma_getcid
#define mcu_mmc_getstatus mcu_mmc_dma_getstatus


#define drive_mmc_open drive_mmc_dma_open
#define drive_mmc_ioctl drive_mmc_dma_ioctl
#define drive_mmc_read drive_mmc_dma_read
#define drive_mmc_write drive_mmc_dma_write
#define drive_mmc_close drive_mmc_dma_close


#include "drive_mmc.c"


