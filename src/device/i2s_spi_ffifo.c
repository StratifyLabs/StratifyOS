// Copyright 2011-2021 Tyler Gilbert and Stratify Labs, Inc; see LICENSE.md


#include <fcntl.h>
#include <errno.h>
#include <stddef.h>
#include "device/usbfifo.h"
#include "sos/debug.h"

#define i2s_ffifo_open i2s_spi_ffifo_open
#define i2s_ffifo_close i2s_spi_ffifo_close
#define i2s_ffifo_read i2s_spi_ffifo_read
#define i2s_ffifo_write i2s_spi_ffifo_write
#define i2s_ffifo_ioctl i2s_spi_ffifo_ioctl

#define mcu_i2s_open mcu_i2s_spi_open
#define mcu_i2s_close mcu_i2s_spi_close
#define mcu_i2s_read mcu_i2s_spi_read
#define mcu_i2s_write mcu_i2s_spi_write
#define mcu_i2s_ioctl mcu_i2s_spi_ioctl


#include "i2s_ffifo.c"
