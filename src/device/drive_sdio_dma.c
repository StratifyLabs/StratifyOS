// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "mcu/sdio.h"

#define mcu_sdio_open mcu_sdio_dma_open
#define mcu_sdio_close mcu_sdio_dma_close
#define mcu_sdio_setaction mcu_sdio_dma_setaction
#define mcu_sdio_getinfo mcu_sdio_dma_getinfo
#define mcu_sdio_setattr mcu_sdio_dma_setattr
#define mcu_sdio_write mcu_sdio_dma_write
#define mcu_sdio_read mcu_sdio_dma_read
#define mcu_sdio_ioctl mcu_sdio_dma_ioctl
#define mcu_sdio_getcsd mcu_sdio_dma_getcsd
#define mcu_sdio_getcid mcu_sdio_dma_getcid
#define mcu_sdio_getstatus mcu_sdio_dma_getstatus


#define drive_sdio_open drive_sdio_dma_open
#define drive_sdio_ioctl drive_sdio_dma_ioctl
#define drive_sdio_read drive_sdio_dma_read
#define drive_sdio_write drive_sdio_dma_write
#define drive_sdio_close drive_sdio_dma_close


#include "drive_sdio.c"


