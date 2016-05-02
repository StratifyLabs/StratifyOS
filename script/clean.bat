@echo off
:LOOP
set a=%1
set a=%a:/=\%
echo Delete %a%
if exist %a% del /f /Q %a%
SHIFT
IF NOT "%1" == "" GOTO LOOP
del /f *.a
exit /b 1
