# StratifyOS

Stratify OS is a fully featured microcontroller operating system designed specifically for the ARM Cortex M architecture. It features:

- [Decoupled System and Application Binaries](guides/Stratify-OS.md)
- [Deep integration with Cortex M hardware](guides/ARM-Cortex-M.md) such as the MPU, SysTick/PendSV, SVCall, Dual Stacks, NVIC, BPU, and Cache
- [POSIX Style Filesystems](guides/Filesystems.md)
- [Complete Hardware Abstraction](guides/Device-Drivers.md)


## Getting Started

Getting started on Stratify OS is easy with the Stratify Labs cloud/command line tool called `sl`.

**Install on Mac OS X**

```
mkdir -p /Applications/StratifyLabs-SDK/Tools/gcc/bin
curl -L -o /Applications/StratifyLabs-SDK/Tools/gcc/bin/sl 'https://stratifylabs.page.link/slmac'
chmod 755 /Applications/StratifyLabs-SDK/Tools/gcc/bin/sl
echo 'export PATH=/Applications/StratifyLabs-SDK/Tools/gcc/bin:$PATH' >> ~/.bash_profile
source ~/.bash_profile
```

**Install on Windows (Git Bash)**

```
mkdir -p /C/StratifyLabs-SDK/Tools/gcc/bin
curl -L -o /C/StratifyLabs-SDK/Tools/gcc/bin/sl.exe 'https://stratifylabs.page.link/slwin'
chmod 755 /C/StratifyLabs-SDK/Tools/gcc/bin/sl.exe
echo 'export PATH=/C/StratifyLabs-SDK/Tools/gcc/bin:$PATH' >> ~/.bash_profile
source ~/.bash_profile
```

Once you install `sl`, visit the [Stratify Labs web application](https://app.stratifylabs.co) to login and start the tutorials.

## License

The Stratify OS source code is available the GPLv3 license.  We provide an exception to the license that allows you to distribute binary copies under the license of your choosing (similar to the old FreeRTOS license).  Read more about licensing here: http://stratifylabs.co/download/


