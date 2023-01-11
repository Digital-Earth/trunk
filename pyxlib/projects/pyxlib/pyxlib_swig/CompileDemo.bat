@echo off
rem
rem CompileDemo.bat
rem
set JAVABIN=
if exist "c:\Program Files (x86)\java\jdk1.6.0_20\bin\javac.exe" set JAVABIN=c:\Program Files (x86)\java\jdk1.6.0_20\bin
if exist "c:\Program Files\java\jdk1.6.0_20\bin\javac.exe" set JAVABIN=c:\Program Files\java\jdk1.6.0_20\bin
if "%JAVABIN%"=="" goto NOTFOUND

rem
rem Figure out debug or release.  debug is the default.
rem

rem release is default for testing!  
goto RELEASE_BUILD
  
if "%1"=="release" goto RELEASE_BUILD

echo Building debug
set BUILD_VER=debug
goto START_COMPILE

:RELEASE_BUILD
echo Building release
set BUILD_VER=release

:START_COMPILE
set TARGETDIR=..\..\..\..\%BUILD_VER%
set TARGETDIR=..\..\..\..\application\worldview\%BUILD_VER%

if exist %TARGETDIR% goto BUILD
echo Output directory %TARGETDIR% does not exist.  You can only run the demo after you
echo compile the SDK.  Try running Build.bat, or CompileJave.bat

:BUILD
copy SDKSample.java %TARGETDIR% > nul
copy _CompileDemo.bat %TARGETDIR%\CompileDemo.bat > nul
if not exist %TARGETDIR%\com mkdir %TARGETDIR%\com\pyxisinnovation
copy java\pyxlib.jar %TARGETDIR%\com\pyxisinnovation\pyxlib.jar > nul
rem copy java\pyxlib.jar %TARGETDIR%\com.pyxisinnovation.pyxlib.jar > nul
robocopy java %TARGETDIR%\Java /s
pushd %TARGETDIR%

"%JAVABIN%\javac" -classpath "java;com.pyxisinnovation.pyxlib.jar" SDKSample.java
"%JAVABIN%\java" -classpath "java;." SDKSample
rem "C:\Users\egirard\Desktop\Downloads\Depends\depends.exe"  "%JAVABIN%\java.exe" -classpath "java;." SDKSample
popd

goto END

:NOTFOUND
echo Warning: Java SDK 1.6 does not appear to ber installed on this machine.  Skipping compilation.
echo You can install it from http://java.sun.com/javase/downloads/widget/jdk6.jsp
:END
set JAVABIN=