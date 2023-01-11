@echo off
rem Batch file name: third_party\build_ogdi.bat

if exist zips\ogdi-3.1.zip goto OKAY
echo Unable to find zips\ogdi-3.1.zip
goto :EOF

:OKAY
echo A|..\Tools\unzip zips\ogdi-3.1.zip

REM Start of build.  
REM Note that the OGDI solution is broken, and doesn't define dependencies correctly.
REM Therefore we build it multiple times.
cd ogdi-3.1.5
devenv OGDI_Build_All.sln /Build Release
devenv OGDI_Build_All.sln /Build Release
devenv OGDI_Build_All.sln /Build Release
devenv OGDI_Build_All.sln /Build Release
devenv OGDI_Build_All.sln /Build Release
cd ..
