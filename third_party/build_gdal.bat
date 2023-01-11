@echo off
rem Gene's build.bat for GDAL adapted for GDAL 2.0.2 by Mark
rem build_gdal.bat debugonly - to build only debug version
rem HDF4, HDF5 and netCDF libraries must be pre-built

set THIRD_PARTY=%CD%
set ZIPFILE=gdal202.zip
set TARGETDIR=GDAL-2.0.2
set PYXISDIR=GDAL.PYXIS

rem Enable this if you want to use USETEIGHA
rem set USETEIGHA=1

if exist %TARGETDIR% goto NO_UNZIP
if exist zips\%ZIPFILE% goto OKAY
echo Unable to find the zip file zips\%ZIPFILE%
goto :EOF

:OKAY
rem Unzip GDAL
echo A|..\Tools\unzip zips\%ZIPFILE%

:NO_UNZIP
rem Anything in GDAL.PYXIS gets copied over the contents of GDAL-2.0.2
rem This lets us make changes to GDAL, and track them.
robocopy /S %PYXISDIR% %TARGETDIR% *.*

rem Copy out some third party support libraries.
if exist Geo_DSDK\README.txt goto FOUND_MR_SID
if exist zips\MrSID.zip goto EXTRACT_MR_SID
echo ERROR! MrSID SDK has not been extracted, and cannot be found in the zip directory.
goto FAIL_MR_SID

:EXTRACT_MR_SID
..\Tools\unzip.exe zips\MrSID.zip 
if exist Geo_DSDK\README.txt goto FOUND_MR_SID
goto FAIL_MR_SID
:FAIL_MR_SID
echo Unable to find MrSID.zip.
goto :EOF

:FOUND_MR_SID

if NOT "%USETEIGHA%"=="1" goto START_BUILD

echo Extracting TIEGHA (DWG SUPPORT)
:EXTRACT_TEIGHA
if exist Teigha_vc11dll-4.01.00\Core\odalogo.ico goto FOUND_TEIGHA
..\Tools\unzip.exe zips\Teigha_vc11dll-4.01.00.zip -d Teigha_vc11dll-4.01.00
if exist Teigha_vc11dll-4.01.00\Core\odalogo.ico goto FOUND_TEIGHA
goto FAIL_TEIGHA
:FAIL_TEIGHA
echo Unable to find Teigha_vc11dll-4.01.00.zip
goto :EOF

:FOUND_TEIGHA
robocopy Teigha_vc11dll-4.01.00\exe\vc11dll teigha\exe\vc11dll *.dll *.tx *.txv

:START_BUILD

cd %TARGETDIR%

rem Release Build
if /i "%1"=="debugonly" GOTO :BUILD_DEBUG
echo Building GDAL release
echo y|del /s *.obj
nmake -f makefile.vc EXT_NMAKE_OPT=pyxis.nmake.opt MSVC_VER=1700 WITH_PDB=1

if NOT "%USETEIGHA%"=="1" goto DONE_BUILD_RELEASE

rem specific build DWG step
echo Building DWG plugin release
cd ogr\ogrsf_frmts\dwg
nmake -f makefile.vc EXT_NMAKE_OPT=pyxis.nmake.opt MSVC_VER=1700 WITH_PDB=1 plugin
cd ..\..\..\
move /y ogr\ogrsf_frmts\dwg\ogr_DWG.dll ..\gdal\bin\release\ogr_DWG.dll

:DONE_BUILD_RELEASE

call :MOVE_BINARIES release

rem Debug Build
:BUILD_DEBUG 
echo Building GDAL debug
echo y|del /s *.obj
nmake -f makefile.vc EXT_NMAKE_OPT=pyxis.nmake.opt MSVC_VER=1700 DEBUG=1

if NOT "%USETEIGHA%"=="1" goto DONE_BUILD_DEBUG

rem specific build DWG step
echo Building DWG plugin debug
cd ogr\ogrsf_frmts\dwg
nmake -f makefile.vc EXT_NMAKE_OPT=pyxis.nmake.opt MSVC_VER=1700 DEBUG=1 plugin
cd ..\..\..\
move /y ogr\ogrsf_frmts\dwg\ogr_DWG.dll ..\gdal\bin\debug\ogr_DWG.dll
move /y ogr\ogrsf_frmts\dwg\ogr_DWG.pdb ..\gdal\bin\debug\ogr_DWG.pdb

:DONE_BUILD_DEBUG

call :MOVE_BINARIES debug

robocopy gcore ..\gdal\include *.h
robocopy ogr ..\gdal\include *.h
robocopy ogr\ogrsf_frmts ..\gdal\include *.h
robocopy port ..\gdal\include *.h
robocopy alg ..\gdal\include *.h
robocopy data ..\gdal\data *.h

cd ..
set THIRD_PARTY=
set ZIPFILE=
set TARGETDIR=
set PYXISDIR=
goto :EOF

:MOVE_BINARIES
@echo on
rem Move GDAL binaries
move /y gdal200.dll ..\gdal\bin\%1\gdal200.dll
move /y gdal200.pdb ..\gdal\bin\%1\gdal200.pdb
move /y gdal_i.lib ..\gdal\lib\%1\gdal_i.lib
move /y gdal.lib ..\gdal\lib\%1\gdal.lib
@echo off
goto :EOF