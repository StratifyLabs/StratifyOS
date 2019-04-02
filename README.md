# StratifyOS

Stratify OS is an microcontroller RTOS for the ARM Cortex M architecture. It is designed to make it dead simple to use the [powerful hardware features of the ARM Cortex M](https://docs.stratifylabs.co/Guide-ARM-Cortex-M/):

- MPU
- SysTick and PendSV
- SVCall
- Thread and Handler Mode
- NVIC
- FPU
- Cache (Cortex M7)

These hardware features are integrated into the OS in such a way that applications take full advantage of them without even knowing they exist!

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


