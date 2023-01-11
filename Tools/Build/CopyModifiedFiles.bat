@echo off
rem if %1*==* goto SYNTAX
if %2*==* goto SYNTAX
goto START

:SYNTAX
echo:
echo Gene's (minimal) copy utility.  Syntax:
echo:
echo     %0 source-dir destination-dir
echo:
echo Copies the contents of source-dir into destination-dir, removing files from 
echo destination if necessary. If any files are "unchanged", then their timestamps 
echo will be maintained in destination-dir.
echo:
goto END

:START

rem Figure out source-dir.
set sourcedir=%1
if exist %CD%\%1\nul set sourcedir=%CD%\%1
if not exist %sourcedir%\nul echo Unable to find source directory %1
if not exist %sourcedir%\nul goto SYNTAX

rem Figure out destination-dir.
if not exist %2\nul echo Creating destination directory %2
if not exist %2\nul md %2

set destinationdir=%2
if exist %CD%\%2\nul set destinationdir=%CD%\%2
if not exist %destinationdir%\nul echo Unable to find destination directory %2
if not exist %destinationdir%\nul goto SYNTAX

rem Some sanity checking...
if %sourcedir%==%destinationdir% echo Warning!  Copying a directory onto itself.
if %sourcedir%==%destinationdir% goto END

goto START_PROCESSING
rem this is a hack!
echo off
echo:
echo Source is %sourcedir%
echo Destination is %destinationdir%
echo:

goto END

:START_PROCESSING

pushd %sourcedir%
for %%a in (*.*) do call :COPY_IF_CHANGED %%a
popd
pushd %destinationdir%
for %%a in (*.*) do call :REMOVE_IF_DELETED %%a
popd
goto END

:COPY_IF_CHANGED
if not exist %destinationdir%\%1 echo Copying %1 (new)
if not exist %destinationdir%\%1 copy %1 %destinationdir%\%1 > nul
fc /b %1 %destinationdir%\%1  | find "FC: no differences encountered" > nul
if not errorlevel 1 goto :END
copy %1 %destinationdir%\%1 > nul
echo Copying %1
goto :END

:REMOVE_IF_DELETED
if not exist %sourcedir%\%1 echo deleting %1
if not exist %sourcedir%\%1 del %1
goto :END

:END

