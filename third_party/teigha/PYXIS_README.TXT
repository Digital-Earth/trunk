The Teigha libraries are supplied in pre-built form by the Open Design Alliance - https://www.opendesign.com/

The member site can be accessed with:

username: myoung@pyxisinnovation.com
password: 1nwq0mowda8xgvi

Both release and debug libraries are available; however, only the release libraries are currently used.

The Teigha libraries are used by the GDAL DWG plugin. To build, run build_gdal.bat.

The Teigha zlib.dll is modified slightly from the standard zlib.dll release. Because of DLL naming conflicts, the Teigha zlib.dll
is not used. This does not appear to cause problems loading .dwg files. Because the Teigha's zlib.dll cannot be rebuilt, it will
required us to support multiple GDAL plugin directories - a feature which is not supported at this time.