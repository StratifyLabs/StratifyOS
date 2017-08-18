@echo off
set TOOLSPATH=%1
set HOST=%2
set PROJ=%3
set NAME=%4
set ARCH=%5
set FLOAT=%6
set TYPE=%7

set LIB=lib%PROJ%_%NAME%%TYPE%

echo "Installing include files in %TOOLSPATH%\%HOST%\include"
xcopy /S /Y ..\include\* %TOOLSPATH%\%HOST%\include

if NOT "%NAME%"=="link" (
	echo "Installing linker files in %TOOLSPATH%\%HOST%\lib\ldscripts"
	xcopy /S /Y ..\ldscript\* %TOOLSPATH%\%HOST%\lib\ldscripts
	xcopy /S /Y ..\include\posix\* %TOOLSPATH%\%HOST%\include
)

if "%NAME%"=="link" (
	mkdir %TOOLSPATH%\%HOST%\include
	mkdir %TOOLSPATH%\%HOST%\include\posix
	mkdir %TOOLSPATH%\%HOST%\lib\ldscripts
	copy /y ..\src\link\Link.h %TOOLSPATH%\%HOST%\include\Link.h
	copy /y ..\ldscript\*.bat %TOOLSPATH%\%HOST%\lib\ldscripts
	xcopy /S /Y ..\include\posix\* %TOOLSPATH%\%HOST%\include\posix
)

echo "Installing library file: %LIB%.a in %TOOLSPATH%\%HOST%\lib\%ARCH%\%FLOAT%"
copy /y %LIB%.a %TOOLSPATH%\%HOST%\lib\%ARCH%\%FLOAT%

:NO_CRT

exit /b 1

