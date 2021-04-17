# Getting Started Guide

You can start developing with Stratify OS by installing the `sl` command line tool. With `sl`, you can install the SDK and other tools as well as build and install Stratify OS kernel images and applications.

**Installing sl**

First set your OS specific variables:

Windows x86_64

```
export SL_LINK=https://stratifylabs.page.link/sl_windows_x86_64
```

Linux x86_64

```
export SL_LINK=https://stratifylabs.page.link/sl_Linux_x86_64
```

Linux arm32

```
export SL_LINK=https://stratifylabs.page.link/sl_Linux_arm32
```

MacOS x86_64

```
export SL_LINK=https://stratifylabs.page.link/sl_Darwin_x86_64
```

Now, use the following commands to download and install `sl`.

```
export INSTALL_DIRECTORY=~/StratifyLabs-SDK # or another directory of your choosing
export PROFILE=~/.bash_profile # or another shell profile

mkdir -p $INSTALL_DIRECTORY
chmod 777 -R $INSTALL_DIRECTORY # if needed to make public
mkdir -p $INSTALL_DIRECTORY/Tools/gcc/bin
curl -L -o $INSTALL_DIRECTORY/Tools/gcc/bin/sl $SL_LINK
chmod 755 $INSTALL_DIRECTORY/Tools/gcc/bin/sl
echo 'export PATH='$INSTALL_DIRECTORY'/Tools/gcc/bin:$PATH' >> $PROFILE
echo 'export SOS_SDK_PATH='$INSTALL_DIRECTORY >> $PROFILE
source $PROFILE
```

**Linux USB Permissions**

Stratify OS devices need to be readable/writeable on Linux and are not by default. The following commands will do the trick.

```
export TARGET='SUBSYSTEM=="usb", ATTRS{idVendor}=="20a0", MODE="0666"'
echo "$TARGET" | sudo tee --append /etc/udev/rules.d/00-sos-usb-permissions.rules
sudo udevadm control --reload-rules && udevadm trigger
```

## Choose a Workspace

After you install `sl`, use the following commands to set up a workspace in a directory of your choosing.

```
sl --initialize
sl cloud.login
```

Now, copy your credentials from the website to the command line.

**Install the SDK**

Use `sl` to install the GCC cross-compiler on your machine and build the latest libraries.

```
sl cloud.install:compiler # download and install the SDK (arm-none-eabi-gcc)
```

**Install Stratify OS**

To install Stratify OS, you will need a compatible board and a way to program it.

**Run Blinky**

Blinky can be installed using a pre-compiled binary.

```
sl cloud.install:id=<TBD>
```

To build, install, and run Blinky use:

```
git clone https://github.com/StratifyLabs/Blinky.git
sl app.build:path=Blinky app.install:path=Blinky,run,terminal
```


