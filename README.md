# StratifyOS

## What's New

Welcome to Stratify OS 4. Version 3 is available as [master](https://github.com/StratifyLabs/StratifyOS/tree/master). The main change from Stratify OS 3 to 4 is how to port and integrate with the board support package. See the [porting guide](guides/Porting.md) for details.

## Getting Started

The fastest way to get started is to use one of the following board support packages:

- [Nucleo144](https://github.com/StratifyLabs/StratifyOS-Nucleo144)
- [STM32H735G-DK](https://github.com/StratifyLabs/STM32H735G-DK)

The above projects have instructions to quickly and easily clone and build the required code.

## About

Stratify OS is a fully featured microcontroller operating system designed specifically for the ARM Cortex M architecture. It features:

- [Decoupled System and Application Binaries](guides/Overview.md)
- [Deep integration with Cortex M hardware](guides/ARM-Cortex-M.md) such as the MPU, SysTick/PendSV, SVCall, Dual Stacks, NVIC, BPU, and Cache
- [POSIX Style Filesystems](guides/Filesystems.md)
- [Complete Hardware Abstraction](guides/Device-Drivers.md)
- [Threads](guides/Threads.md)

## License

The Stratify OS source code is available under a commercial source available license. The license requires a hardware ID be issued by Stratify Labs, Inc and used to identify unique hardware configurations. Hardware IDs are available free of charge and allow the licensee to use Stratify OS without any charges or fees on the hardware design associated with the ID.


