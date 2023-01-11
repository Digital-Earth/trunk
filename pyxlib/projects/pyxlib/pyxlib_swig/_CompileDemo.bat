@echo off
rem
rem CompileDemo.bat (This version runs in the WorldView directory, to compile and run the demo.)
rem

if exist plugins goto DIRECTORY_VERIFIED
echo This batch file should be run from the WorldView directory.
goto END

:DIRECTORY_VERIFIED
set JAVABIN=
if exist "c:\Program Files (x86)\java\jdk1.6.0_20\bin\javac.exe" set JAVABIN=c:\Program Files (x86)\java\jdk1.6.0_20\bin
if exist "c:\Program Files\java\jdk1.6.0_20\bin\javac.exe" set JAVABIN=c:\Program Files\java\jdk1.6.0_20\bin
if "%JAVABIN%"=="" goto NOTFOUND

:BUILD

"%JAVABIN%\javac" -classpath "java;com.pyxisinnovation.pyxlib.jar" SDKSample.java
"%JAVABIN%\java" -classpath "java;." SDKSample

goto END

:NOTFOUND
echo Warning: Java SDK 1.6 does not appear to ber installed on this machine.  Skipping compilation.
echo You can install it from http://java.sun.com/javase/downloads/widget/jdk6.jsp
:END
set JAVABIN=