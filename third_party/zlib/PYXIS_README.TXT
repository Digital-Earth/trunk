This README describes how to build zlib for PYXIS.
Last updated by: Mark Young
Date: 2015-09-08

1. Download the latest zlib source from http://www.zlib.net
2. Unzip to \third_party\zlib-1.2.8
3. Follow the instructions in http://www.tannerhelland.com/5076/compile-zlib-winapi-wapi-stdcall/ (copied below for reference)
	1. Extract the entire zLib file and navigate to the /contrib/masmx86 folder. Open the “bld_ml32.bat” file in a text editor.
	2. Add the “/safeseh” switch to both lines in that file (e.g. “ml /safeseh /coff /Zi /c /Flmatch686.lst match686.asm”). Then save and exit.
	3. Navigate to the /contrib/vstudio/vc11/ folder. Open the zlibvc.sln file in your newly installed Visual Studio 2012 Express.
	4. In the Solution Explorer (top-right by default), right-click “zlibstat” then select “Properties” at the bottom.
	5. Go to Configuration Properties -> C/C++ -> Preprocessor, and in the Preprocessor Definitions line remove “ZLIB_WINAPI;” (don’t forget to remove the trailing semicolon).
	6. Now, we need to fix a recently introduced problem that relies on Win8 functionality. In the Solution Explorer, navigate to zlibvc -> iowin32.c. Double-click to open the file.
	7. Find the line of text that reads “#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)”. Change this line to “#if WINAPI_FAMILY_ONE_PARTITION(WINAPI_FAMILY_DESKTOP_APP, WINAPI_PARTITION_APP)”. (Thanks to this link for this fix.)
	8. zLib uses a Version number declaration that can cause the build process to fail. To fix this, go back to the Solution Explorer, then navigate to zlibvc -> zlibvc.def. Double-click to open.
	9. Change the line that reads “VERSION 1.2.8” to read “VERSION 1.28”.
	10. Finally, go to the Build -> Configuration Manager menu and change the Active Solution Configuration to “Release”.
	11. Exit that window and press F7 (or click the Build -> Build Solution menu). The project should successfully build.
	12. You can find your newly compiled zlibwapi.dll file in the /contrib/vstudio/vc11/x86/ZlibDllRelease/ folder. (but see below)
4. In the project files remove the ZLIB_WINAPI preproccesor definition.
5. In the project files, retarget the output for the vcdll project to zlib_pyxis (release) and zlib_pyxisd (debug) for dll, lib and map files by editing the project properties and do a clean build for release and debug.
6. Move \third_party\zlib-1.2.8\zlib.h and zconf.h to \third_party\zlib\include
7. Move all the *.dll, *.exp, *.lib, *.map and *.pdb files to \third_party\zlib\

A number of third party libraries use zlib. The _pyxis extension is used to avoid conflict with third party libraries that supply their own zlib (for example Teigha).

\third_party\zips\zlib-1.2.8.zip contains all the changes described in steps 3 to 5 above. It can be unzipped and rebuilt if needed.
