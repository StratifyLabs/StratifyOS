// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include "mcu/spi.h"

#define mcu_spi_open mcu_spi_dma_open
#define mcu_spi_close mcu_spi_dma_close
#define mcu_spi_setaction mcu_spi_dma_setaction
#define mcu_spi_setduplex mcu_spi_dma_setduplex
#define mcu_spi_swap mcu_spi_dma_swap
#define mcu_spi_getinfo mcu_spi_dma_getinfo
#define mcu_spi_setattr mcu_spi_dma_setattr
#define mcu_spi_write mcu_spi_dma_write
#define mcu_spi_read mcu_spi_dma_read
#define mcu_spi_ioctl mcu_spi_dma_ioctl


#define drive_sdspi_open drive_sdspi_dma_open
#define drive_sdspi_ioctl drive_sdspi_dma_ioctl
#define drive_sdspi_read drive_sdspi_dma_read
#define drive_sdspi_write drive_sdspi_dma_write
#define drive_sdspi_close drive_sdspi_dma_close


#include "drive_sdspi.c"


