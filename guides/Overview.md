# Stratify OS Overview

Stratify OS is not like any other microcontroller operating system you have ever used. [It was built from the ground up for the ARM Cortex M architecture](ARM-Cortex-M.md) to solve the challenge of increasingly complex hardware designs and expanding software requirements. 

It does this by:

- Building and Installing the OS independent of applications
- Providing portable applications that can be run on any hardware running Stratify OS
- Providing a cloud platform for board firmware updates, sharing pre-built applications, as well as storing and retrieving data

> **Note.** 
>
> Stratify OS is perfect for building hardware modules (like bluetooth, wifi, etc). Most module manufacturers use serialization or complex SDKs based on source code integration or scripting.
>
> With Stratify OS, module manufacturers can define the BSP and root applications while allowing customers to build and run applications that run native C/C++ applications on the MCU using an easy-to-use POSIX API.

## OS Built and Installed Independent of Applications

The first thing to understand is that the OS is built and installed independent of applications. A project for building Stratify OS for a particular board is called an OS package. The OS package is built using:

- Board specific configuration tables
- Any board specific code provided by the board manufacturer (such as custom drivers or kernel requests)
- System level routines (if applicable)

To get started on Stratify OS, [you can install a pre-built bootloader and kernel image in a matter of minutes](https://app.stratifylabs.co).

For a custom OS package, please see the [porting guide](Porting.md).

### Stratify OS Library

The Stratify OS library includes code that is developed for the ARM Cortex M architecture to accomplish context switching, filesystem abstraction, mutexes, threads, etc. The OS functionality is accessible via a POSIX API including:

- unistd: e.g., `open()`, `close()`, `read()`, `write()`, `ioctl()`, and `usleep()`
- pthreads: e.g., `pthread_create()`
- Semaphores
- Mutexes
- Message queues
- Signals: e.g., `kill()`

The Stratify OS library is built per instruction set architecture and ABI:

- armv7m (Cortex M3)
- armv7e-m (Cortex M4)
  - Hard Float v4 (Cortex M4F)
  - Hard Float v5 single precision (Cortex M7)
  - Hard Float v5 double precision (Cortex M7)

**[Learn More About Device Drivers](../Guide-Device-Drivers/)**

## Applications Built Independent of the OS

Stratify OS applications are built independently of the OS. The Stratify OS "Hello World" program is simply:

```c
#include <cstdio>

int main(int argc, char * argv[](){
   printf("Hello World\n");
   return 0;
}
```
When the application is built, it is linked to a CRT library (libsos_crt) that will call the printf() code that is already installed in the kernel. 

### Memory Protected Processes

Stratify OS uses the [ARM Cortex M memory protection unit (MPU)](../Guide-ARM-Cortex-M/#thread-and-handler-mode-with-the-mpu) to prevent processes from clobbering memory in other applications. Because applications run in thread mode (unprivileged mode), they cannot directly access hardware resources. ([They must use device drivers](../Guide-Device-Drivers/)).

### Relocatable Programs

The code is built in a re-locatable fashion such that it is translated when it is installed to its execution location in either flash or RAM (including external RAM). The relocatable copy of the program can be installed in a data filesystem (e.g., on an external SD card) and then be loaded dynamically.

The code below uses the [Stratify API](../StratifyAPI/#namespace-sys) to launch a program stored on a data filesystem.

```c
#include <sapi/sys.hpp>

int main(int argc, char * argv[](){
   printf("I am about to launch Hello World in a new process\n");
   Sys::launch("/home/HelloWorld");
   return 0;
}
```

The program above launches "HelloWorld" that is installed in the local folder `/home`. If "HelloWorld" exists in RAM or flash, it will be executed. If not, it will be dynamically installed to flash and executed. 

`Sys::launch()` calls a Stratify OS function called `launch()`. Most system calls are based on the standard C library or POSIX API. However, because the Cortex M architecture [does not have an MMU](https://blog.stratifylabs.co/device/2014-05-03-Applications-without-MMU/), `launch()` is used in lieu of the `exec()` and `fork()` function families.

## What to do Now

Now that you understand the basics you can head over to the [Stratify Dashboard](https://app.stratifylabs.co) and run through the tutorials.


