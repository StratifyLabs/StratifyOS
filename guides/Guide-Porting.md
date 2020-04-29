---
date: "2019-06-27"
layout: post
title: Porting Stratify OS to a new MCU Family
katex: true
slug: Guide-Porting
menu:
  sidebar:
    name: Porting
    parent: Guides
---

## Introduction

Stratify OS is an operating system specifically designed for the ARM Cortex-M architecture. It supports the following architectures:

- ARM Cortex M3 (armv7m)
- ARM Cortex M4 (armv7em)
- ARM Cortex M4F (armv7em_f4sh)
- ARM Cortex M7 (armv7em_f5sh and armv7em_f5dh)

There is planned support for the ARM Cortex M23 and M33 architectures though no chips are currently supported.

There are two major libraries that are included when building Stratify OS.

- Stratify OS
- Stratify MCU

> The Stratify API library is strictly for building applications. Applications are not built with the OS like most microcontroller operating systems so there is nothing that needs to be done with Stratify API when porting.

The Stratify OS library requires minor modifications to the `mcu/arch.h` file to add the appropriate preprocessor definitions for the new chip family. Other than that, no changes to the Stratify OS codebase are required when porting to a new MCU family.

The Stratify MCU library is contains the vast majority of MCU specific code. There is a library for each chip family that is supported. Here are two examples:

- [StratifyOS-mcu-stm32](https://github.com/StratifyLabs/StratifyOS-mcu-stm32)
- [StratifyOS-mcu-lpc](https://github.com/StratifyLabs/StratifyOS-lpc-stm32)

The Stratify MCU library contains all the MCU specific startup code and peripheral drivers that are required to run Stratify OS on the chip.

## Minimal Implementation

Stratify OS can run on an MCU if the following modules are implemented in the MCU library.

- [startup](https://github.com/StratifyLabs/StratifyOS-mcu-stm32/blob/master/src/stm32f417xx/core_startup.c): defines MCU startup vectors
- [core](https://github.com/StratifyLabs/StratifyOS-mcu-stm32/blob/master/src/core_dev.c): defines driver to access core MCU functionality
- [pio](https://github.com/StratifyLabs/StratifyOS-mcu-stm32/blob/master/src/pio_dev.c): defines driver to control a single LED
- [tmr](https://github.com/StratifyLabs/StratifyOS-mcu-stm32/blob/master/src/tmr_dev.c): defines a driver with a 32-bit hardware timers used for usleep() and other system timing.
- [uart](https://github.com/StratifyLabs/StratifyOS-mcu-stm32/blob/master/src/uart_dev.c): defines a driver that can be used for debugging and to run the link protocol if USB is not ready

## Standard Implementation

An MCU family is considered fully supported if it implements the following driver interfaces.

- [adc](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/adc.h)
- [dac](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/dac.h)
- [core](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/core.h)
- [eint](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/eint.h)
- [flash](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/flash.h)
- [i2c](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/i2c.h)
- [i2s](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/i2s.h)
- [pio](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/pio.h)
- [mem](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/mem.h)
- [rtc](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/rtc.h)
- [spi](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/spi.h)
- [tmr](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/tmr.h)
- [uart](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/uart.h)
- [usb](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/usb.h)

With the above drivers, the root filesystem, device filesystem, and application filesystem will all function correctly.

## Extended Implementation

Support for an MCU family may be extended to support the following driver interfaces defined by Stratify OS.

- [can](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/can.h)
- [eth](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/eth.h)
- [crc](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/crc.h)
- [crypt](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/crypt.h)
- [hash](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/hash.h)
- [random](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/random.h)
- [mmc](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/mmc.h)
- [sdio](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/sdio.h)
- [qei](https://github.com/StratifyLabs/StratifyOS/blob/master/include/sos/dev/qei.h)

## Beyond Stratify MCU

Drivers whose functionality is fixed by the hardware should be implemented at the OS package level and do not need to be accessible by applications through the device filesystem. Examples of such drivers include external memory chips that are mapped to address space, memory mapped LCD drivers, etc.




