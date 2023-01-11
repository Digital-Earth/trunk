This README describes how to build libcurl for PYXIS.
Last updated by: Mark Young
Date: 2015-08-25

1. Download the latest curl source from http://curl.haxx.se/download.html
2. Unzip to \third_party\curl-7.44.0
3. Open C:\Trunk\third_party\curl-7.44.0\projects\Windows\VC11\curl-all.sln
4. To enable zlib support, apply the following changes to these configurations: DLL Debug - DLL OpenSSL and DLL Release - DLL OpenSSL.
	1. Add preprocessor definitions: HAVE_ZLIB_H, HAVE_ZLIB, HAVE_LIBZ
	2. Add ..\..\..\..\..\zlib\include to Additional Include Directories
	3. Add ..\..\..\..\..\zlib\ to Additional Library Directories
	4. Add zlib_pyxis.lib (for release) and zlib_pyxisd.lib (for debug) to Additional Dependencies
5. Build the release and debug configurations
6. Copy the .dll, .exp, lib and .pdb files to \third_party\libcurl.
7. Copy \third_party\curl-7.44.0\include to \third_party\libcurl. Delete everything except for the .h and README files.

\third_party\zips\curl-7.44.0.zip contains all the changes described in step 4 above. It can be unzipped and rebuilt if needed.
