# Getting Started Guide

You can start developing with Stratify OS by install the `sl` command line tool. With `sl` you can install the SDK and other tools as well as build and install Stratify OS kernel images and applications.

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

**Install on Linux x86_64 (experimental)**

```
mkdir -p -m 777 /StratifyLabs-SDK/Tools/gcc/bin # create /StratifyLabs-SDK as publicly accessible
curl -L -o /StratifyLabs-SDK/Tools/gcc/bin/sl 'https://stratifylabs.page.link/sllinuxx86_64'
chmod 755 /StratifyLabs-SDK/Tools/gcc/bin/sl
echo 'export PATH=/StratifyLabs-SDK/Tools/gcc/bin:$PATH' >> ~/.profile
source ~/.profile
```

**Install on Linux arm32 (experimental)**

```
mkdir -p -m 777 /StratifyLabs-SDK/Tools/gcc/bin # create /StratifyLabs-SDK as publicly accessible
curl -L -o /StratifyLabs-SDK/Tools/gcc/bin/sl 'https://stratifylabs.page.link/sllinuxarm32'
chmod 755 /StratifyLabs-SDK/Tools/gcc/bin/sl
echo 'export PATH=/StratifyLabs-SDK/Tools/gcc/bin:$PATH' >> ~/.profile
source ~/.profile
```

**Linux USB Permissions**

Stratify OS devices need to be readable/writeable on Linux and are not by default. The following commands will make Stratify OS devices

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
