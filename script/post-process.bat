@echo off
set TOOLSPATH=%1
set HOST=%2
set ARCH=%3
echo "Installing include files in %TOOLSPATH%\%HOST%\include"
xcopy /S /Y ..\include\* %TOOLSPATH%\%HOST%\include
if NOT "%ARCH%"=="link" (
	echo "Installing linker files in %TOOLSPATH%\%HOST%\lib\ldscripts"
	xcopy /S /Y ..\ldscript\* %TOOLSPATH%\%HOST%\lib\ldscripts
	echo D | xcopy /S /Y ..\include\dev_sys\* %TOOLSPATH%\%HOST%\lib\include
)

if "%ARCH%"=="link" (
	copy /y ..\src\link\Link.h %TOOLSPATH%\%HOST%\include\hwpl\Link.h
)

if "%ARCH%"=="CRT" (
	copy /y .\libcrt.a %TOOLSPATH%\%HOST%\lib
	GOTO :NO_CRT
)

echo "Installing library file: libcaos_%ARCH%.a in %TOOLSPATH%\%HOST%\lib"
copy /y .\libcaos_%ARCH%.a %TOOLSPATH%\%HOST%\lib

:NO_CRT

exit /b 1

