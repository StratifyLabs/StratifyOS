#!/bin/bash
TOOLSPATH=$1
HOST=$2
PROJ=$3
NAME=$4
ARCH=$5
FLOAT=$6
TYPE=$7
$TOOLSPATH/bin/$HOST-objcopy -j .text -j .data -O binary ./$PROJ.elf ./$PROJ
$TOOLSPATH/bin/$HOST-objdump -j .text -j .data -j .bss -j .sysmem -d $PROJ.elf > $PROJ.s
$TOOLSPATH/bin/$HOST-size ./$PROJ.elf 
