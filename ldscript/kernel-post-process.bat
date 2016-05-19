@echo off
set TOOLSPATH=%1
set HOST=%2
set PROJ=%3
set NAME=%4
set ARCH=%5
set FLOAT=%6
set TYPE=%7

set OUT=%PROJ%

%TOOLSPATH%\bin\%HOST%-objcopy -j .text -j .priv_code -j .data -j .vectors -j .startup -O binary %OUT%.elf %OUT%.bin
%TOOLSPATH%\bin\%HOST%-objdump -S -j .text -j .priv_code -j .data -j .bss -j .sysmem -d %OUT%.elf > %OUT%.s
%TOOLSPATH%\bin\%HOST%-size %OUT%.elf 
