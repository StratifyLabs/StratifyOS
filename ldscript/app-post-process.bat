@echo off
set TOOLSPATH=%1
set HOST=%2
set PROJ=%3
set NAME=%4
set ARCH=%5
set FLOAT=%6
set TYPE=%7
%TOOLSPATH%\bin\%HOST%-objcopy -j .text -j .data -O binary %PROJ%.elf %PROJ%
%TOOLSPATH%\bin\%HOST%-objdump -j .text -j .data -j .bss -j .sysmem -d %PROJ%.elf > %PROJ%.s
%TOOLSPATH%\bin\%HOST%-size %PROJ%.elf 
exit /b 1