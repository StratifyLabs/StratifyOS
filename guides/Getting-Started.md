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
mkdir -p /StratifyLabs-SDK/Tools/gcc/bin # create /StratifyLabs-SDK as publicly accessible
curl -L -o /StratifyLabs-SDK/Tools/gcc/bin/sl 'https://stratifylabs.page.link/sllinuxx86_64'
chmod 755 /StratifyLabs-SDK/Tools/gcc/bin/sl
echo 'export PATH=/StratifyLabs-SDK/Tools/gcc/bin:$PATH' >> ~/.profile
source ~/.profile
```

**Install on Linux**

Coming Soon!

## Choose a Workspace

After you install `sl`, use the following commands to set up a workspace in a directory of your choosing.

```
sl --initialize
sl cloud.login
```

Now, copy your credentials from the website to the command line.
