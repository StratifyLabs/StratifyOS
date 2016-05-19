@echo off

set TOOLSPATH=%1
set HOST=%2
set PROJ=%3
set NAME=%4
set ARCH=%5
set FLOAT=%6
set FOLDER=%7
set TYPE=%8
set LIB=lib%NAME%%TYPE%
echo "Installing include files in%TOOLSPATH%\%HOST%\include\%FOLDER%"
mkdir %TOOLSPATH%\%HOST%\include\%FOLDER%
xcopy /S /Y ..\include\* %TOOLSPATH%\%HOST%\include\%FOLDER%
echo "Installing library file: %LIB%.a in %TOOLSPATH%\%HOST%\lib\%ARCH%\%FLOAT%"
copy /y %LIB%.a %TOOLSPATH%\%HOST%\lib\%ARCH%\%FLOAT%
exit /b 1
