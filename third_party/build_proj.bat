@echo off

rem This is Gene's build.bat for Proj
if exist zips\proj-4.5.zip goto OKAY
echo Unable to find the zip file zips\proj-4.5.zip
goto :EOF

:OKAY
rem if exist proj-4.5.0\nul rmdir /q /s proj-4.5.0
echo A|..\Tools\unzip zips\proj-4.5.zip

REM Start of build
cd proj-4.5.0\src
call build.bat
cd ..\..
