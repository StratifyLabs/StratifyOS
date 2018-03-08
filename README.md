# StratifyOS

Stratify OS is an microcontroller RTOS for the ARM Cortex M architecture. It is designed to make it dead simple to use the [powerful hardware features of the ARM Cortex M](https://stratifylabs.co/user%20guides/2018/03/06/How-Stratify-OS-Levarages-ARM-Cortex-M/):

- MPU
- SysTick and PendSV
- SVCall
- Thread and Handler Mode
- NVIC
- FPU
- Cache (Cortex M7)

These hardware features are integrated into the OS in such a way that applications take full advantage of them without even knowing they exist!

You can find resources for getting started in the [Stratify SDK documentation](https://stratifylabs.co/docs/).

## License

The Stratify OS source code is available the GPLv3 license.  We provide an exception to the license that allows you to distribute binary copies under the license of your choosing (similar to the FreeRTOS license).  Read more about licensing and download the SDK here: http://stratifylabs.co/download/

Stratify OS can be installed on [many popular development boards](http://stratifylabs.co/hardware/) in just a few minutes.

## Building Stratify OS

Stratify OS is built and distributed with the [Stratify Labs SDK](https://stratifylabs.co/download/). You only need to build and install the source code if you want to debug, contribute new features, or equip your local SDK with a previous version.

Here are the steps to build and install Stratify OS. You need to have git, CMake and the Stratify Labs SDK installed before running these commands.

The API builds for running with Stratify OS on embedded platforms. Stratify OS includes a serialization (called link) feature that enables communications with Stratify OS on Windows and Mac OS X.  The cmake scripts check for the suffix "_arm" or "_link" to see which to build.

### Mac OS X

```
git clone https://github.com/StratifyLabs/StratifyOS.git
cd StratifyOS
mkdir cmake_arm; mkdir cmake_link
cd cmake_arm
cmake ..; make; make install
cd ../cmake_link
cmake ..; make; make install
```

### Windows

For windows, you need to specify some additional options because "make" typically isn't available in the PATH environment. If you are using CMD or powershell use "-G "MinGW Makefiles. If you are using msys, you can use -G "Unix Makefiles" and use the commands listed above for Mac OS X.

```
git clone https://github.com/StratifyLabs/StratifyOS.git
cd StratifyOS
mkdir cmake_arm; mkdir cmake_link
cd cmake_arm
cmake .. -G "MinGW Makefiles"
cmake --build . --target install
cd ../cmake_link
cmake .. -G "MinGW Makefiles"
cmake --build . --target install
```

