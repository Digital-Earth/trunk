This README describes how to build szip for PYXIS.
Last updated by: Mark Young
Date: 2015-09-08

1. Download the latest szip source from ftp://ftp.hdfgroup.org/lib-external/szip/2.1/src/
2. Unzip/untar to \third_party\szip-2.1
3. Open szip_dynamic.vcproj from \windows\dynamic in Visual Studio 2012.
4. In the Debug configuration, edit the following in Linker->AllOptions
	1. Generate Program Database File:  $(OutDir)$(TargetName)d.pdb
	2. Import Library: $(OutDir)$(TargetName)d.lib
5. Build Release and Debug
6. Move all *.h files to \third_party\szip\include
7. Move all *.dll, *.exp, *.lib and *.pdb files to \third_party\szip

\third_party\zips\szip-2.1.zip contains all the changes described in step 4 above. It can be unzipped and rebuilt if needed.
