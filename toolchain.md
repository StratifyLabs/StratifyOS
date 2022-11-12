# Porting a new Toolchain (GCC)


Porting Stratify OS to a new version of GCC is not an exact process. Here, I try to document the steps that have worked in the past, but each new version introduces new issues to address.

> These instructions are tailored for running on MacOS.

- You first need to download the binaries from [ARM](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads).
- Use the regular process to extract and install the toolchain on your computer.
- Clone a project that uses Stratify OS and bootstrap it with an existing toolchain that works properly.
  - In this case, I am using [StratifyOS-Nucleo-144](https://github.com/StratifyLabs/StratifyOS-Nucleo144)
  - I cloned to `~/gitv4/StratifyOS-Nucleo144`
- Replace the contents of `~/gitv4/StratifyOS-Nucleo-144/SDK/local` with the contents of the new ARM toolchain
  - For example, `/Applications/ArmGNUToolchain/11.3.rel1/arm-none-eabi`
- Build and install newlib using:
  - `ninja SDK/StratifyOS-newlib/install`
  - This will copy the StratifyOS specific newlib header files where they can be used to re-build libstdc++
- Download the source code for the GCC version (also from ARM)
- Use the commands below (which assume `arm-gnu-toolchain-src-snapshot-11.3.rel1`) to rebuild libstdc++
  - The key here is we need to build with `-mlong-calls -D__StratifyOS__`


```sh
export ARM_NONE_EABI_PATH=~/gitv4/StratifyOS-Nucleo-144/SDK/local
export CXX_FOR_TARGET=${ARM_NONE_EABI_PATH}/bin/arm-none-eabi-g++
export CC_FOR_TARGET=${ARM_NONE_EABI_PATH}/bin/arm-none-eabi-gcc
export LD_FOR_TARGET=${ARM_NONE_EABI_PATH}/bin/arm-none-eabi-ld
export AS_FOR_TARGET=${ARM_NONE_EABI_PATH}/bin/arm-none-eabi-as
export RANLIB_FOR_TARGET=${ARM_NONE_EABI_PATH}/bin/arm-none-eabi-ranlib
export AR_FOR_TARGET=${ARM_NONE_EABI_PATH}/bin/arm-none-eabi-ar
export NM_FOR_TARGET=${ARM_NONE_EABI_PATH}/bin/arm-none-eabi-nm
export CFLAGS=-static 
export CPPFLAGS=-static 
export LDFLAGS=-static

export PATH=${ARM_NONE_EABI_PATH}/bin:$PATH

../arm-gnu-toolchain-src-snapshot-11.3.rel1/arm-gnu-toolchain-src-snapshot-11.3.rel1/libstdc++-v3/configure \
    --host=arm-none-eabi \
    --prefix=/Users/tgil/gitv4/StratifyOS-Nucleo144/SDK/local/arm-none-eabi \
    --disable-shared \
    --enable-static \
    --disable-libstdcxx-pch \
    --disable-libstdcxx-verbose \
    --with-gnu-ld \
    --with-newlib \
    --with-multilib-list=../arm-gnu-toolchain-src-snapshot-11.3.rel1/arm-gnu-toolchain-src-snapshot-11.3.rel1/gcc/config/arm/t-rmprofile

    
make -j18 CXXFLAGS="-Os -fno-exceptions -ffunction-sections -fdata-sections -mlong-calls -D__StratifyOS__" CFLAGS="-Os -ffunction-sections -fdata-sections -mlong-calls -D__StratifyOS__"
```

- Use `sl` to create an compiler bundle
  - This command will create an `sblob` file. This is a non-secure encrypted compressed blob containing the toolchain.
  - The command will also output an `sha256` value.

```sh
sl os.pack:name=compiler-gcc11-3,path=SDK/local,destination=tmp
```

- Upload the `sblob` file to somewhere on the internet
- Use the following `sl` command to install the toolchain within a local Stratify OS project

```sh
export COMPILER_LINK=https://github.com/StratifyLabs/StratifyOS/releases/download/compilerv11/compiler-gcc-11-3.macosx_x86_64_sblob
export COMPILER_HASH=834B57C59B653BF27365A161DBB1EA88B327D8C2BA0DE9AC091D261F5CE7F114
sl cloud.install:compiler,url=${COMPILER_LINK},hash=${COMPILER_HASH},path=SDK/local
```

