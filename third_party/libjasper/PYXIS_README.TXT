This README describes how to build libjasper for PYXIS.
Last updated by: Mark Young
Date: 2015-10-16

1. Download the latest libjasper source from ftp://ftp.remotesensing.org/gdal/jasper-1.900.1.uuid.tar.gz
2. Unzip to \third_party\jasper-1.900.1.uuid
3. Open \src\msvc\jasper.sln
4. In the libjasper project for the release configuration change Properties->C/C++->All Options->Runtime Library to Multi-threaded DLL (/MD)
5. In the libjasper project for the debug configuration change Properties->C/C++->All Options->Runtime Library to Multi-threaded Debug DLL (/MDd)
6. In libjasper project for the debug configuration change Properties->Librarian->All Options->Output File to .\Win32_Debug\libjasperd.lib
7. Build the debug and release versions of the libjasper project
8. Copy the libjasper.lib and libjasperd.lib files to \third_party\libjasper
9. Copy the *.h files from \libjasper\include\jasper to \third_party\libjasper\include\jasper

\third_party\zips\jasper-1.900.1.uuid.zip contains all the changes described in steps 4-6 above. It can be unzipped and rebuilt if needed.
