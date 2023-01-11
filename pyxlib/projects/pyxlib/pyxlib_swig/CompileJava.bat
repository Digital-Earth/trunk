@echo off
rem
rem CompileJava.bat
rem
set JAVABIN=

rem
rem jdk1.6.0_20
rem 
if exist "c:\Program Files (x86)\java\jdk1.6.0_20\bin\javac.exe" set JAVABIN=c:\Program Files (x86)\java\jdk1.6.0_20
if exist "c:\Program Files\java\jdk1.6.0_20\bin\javac.exe" set JAVABIN=c:\Program Files\java\jdk1.6.0_20

rem
rem jdk1.6.0_21
rem 
if exist "c:\Program Files (x86)\java\jdk1.6.0_21\bin\javac.exe" set JAVABIN=c:\Program Files (x86)\java\jdk1.6.0_21
if exist "c:\Program Files\java\jdk1.6.0_21\bin\javac.exe" set JAVABIN=c:\Program Files\java\jdk1.6.0_21

if "%JAVABIN%"=="" goto NOTFOUND

"%JAVABIN%\bin\javac" -d java java\com\pyxisinnovation\pyxlib\*.java 
"%JAVABIN%\bin\jar" cvf java/pyxlib.jar -C java/com/ . > java\jar.log
if not exist java\pyxlib.jar echo Error!  Failed to create java\pyxlib.jar

rem
rem Figure out debug or release.  debug is the default.
rem
  
if "%1"=="release" goto RELEASE_BUILD
if "%1"=="Release" goto RELEASE_BUILD

echo Building debug
set build_version=debug
set FLAGS= /D "_DEBUG" /MDd /Zi

goto START_COMPILE

:RELEASE_BUILD
echo Building release
set build_version=release
set FLAGS= /D "NDEBUG" /MD 
:START_COMPILE

set OUTPUTDIR=%2
if "%2"=="" set OUTPUTDIR="../../../../%build_version%"

rem
rem Now we compile the C++ code.
rem
pushd 
devenv /? > nul 2> nul
if errorlevel 1 (echo Not building from the devstudio command shell.) else goto FOUND_COMPILER
if exist "C:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" call "C:\Program Files\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
if exist "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
if exist "C:\Program Files (x64)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" call "C:\Program Files (x64)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
set LASTCOMMAND=pause
devenv /? > nul 2> nul
if errorlevel 1 (echo Dev studio not found in default location.) else goto FOUND_COMPILER
goto END

:FOUND_COMPILER
echo Attempting to add "%JAVABIN%\include" to path.
if not exist "%JAVABIN%\include\jni.h" echo JNI.h is not found!!!
set TMP_INCLUDE=%INCLUDE%
set INCLUDE=%INCLUDE%;"%JAVABIN%\include";"%JAVABIN%\include\win32";../source;"../../../../third_party/boost";"../../../../third_party/xerces-c-3.1.1/include"
set TMP_LIB=%LIB%
set LIB=%LIB%;../../../../third_party/boost/lib;%OUTPUTDIR%

set COMPILE_FLAGS=/Ox /GL /I "%JAVABIN%\include" /I "%JAVABIN%\include\win32" /I "../source" /I "../../../../third_party/boost" /I "../../../../third_party/xerces-c-3.1.1/include" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "_WINDLL" /D "_UNICODE" /D "UNICODE" /FD /EHa /Fo"%build_version%\\" /Fd"%build_version%\vc90.pdb" /W3 /nologo /c /TP /FI "../../../../config/windows/force_include.h" /errorReport:prompt
  
cl %COMPILE_FLAGS% %FLAGS% /EHa java\cppsource\pyxlib_swig_java.cxx /LD 
set INCLUDE=%TMP_INCLUDE%
echo Removing include extensions.
set LIB=%TMP_LIB%
set TMP_INCLUDE=
set TMP_LIB=
set COMPILE_FLAGS=

link /MACHINE:X86 /OUT:"%OUTPUTDIR%\pyxlib_swig_java.dll" /LTCG /NOLOGO /LIBPATH:"%OUTPUTDIR%" /LIBPATH:"../../../../third_party/boost/lib" /DLL /MANIFEST /DEBUG /PDB:"%OUTPUTDIR%\pyxlib_swig_java.pdb" /SUBSYSTEM:WINDOWS /DYNAMICBASE:NO /MACHINE:X86 /ERRORREPORT:PROMPT kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib "%OUTPUTDIR%\pyxlib.lib" %build_version%\pyxlib_swig_java.obj

mt -outputresource:%OUTPUTDIR%\pyxlib_swig_java.dll;#2 -manifest %OUTPUTDIR%\pyxlib_swig_java.dll.manifest

rem link /MACHINE:X86 /OUT:"..\..\..\..\%build_version%\pyxlib_swig_java.dll" /LTCG /NOLOGO /LIBPATH:"..\..\..\..\%build_version%" /LIBPATH:"../../../../third_party/boost/lib" /DLL /MANIFEST /MANIFESTFILE:"%build_version%\pyxlib_swig_java.dll.intermediate.manifest" /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /DEBUG /PDB:"..\..\..\..\%build_version%\pyxlib_swig_java.pdb" /SUBSYSTEM:WINDOWS /DYNAMICBASE:NO /MACHINE:X86 /ERRORREPORT:PROMPT kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib "..\..\..\..\%build_version%\pyxlib.lib" %build_version%\pyxlib_swig_java.obj

popd
goto END

:NOTFOUND
echo Warning: Java SDK 1.6 does not appear to be installed on this machine.  Skipping compilation.
echo You can install it from http://java.sun.com/javase/downloads/widget/jdk6.jsp
set JAVABIN=
%LASTCOMMAND%
rem We want to avoid returning an error code if we skipped compilation.
exit /B 0

:END
set JAVABIN=
%LASTCOMMAND%