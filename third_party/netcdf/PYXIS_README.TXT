This README describes how to build netcdf for PYXIS.
Last updated by: Mark Young
Date: 2015-09-09

1. Download the latest netcdf source from https://github.com/Unidata/netcdf-c/releases
2. Unzip to \third_party\netcdf-c-4.3.3.1
3. In CMakeLists.txt comment out the line: FIND_PACKAGE(SZIP NO_MODULE REQUIRED)
4. Run VS2012 x86 Native Tools Command Prompt from the Windows Start Menu
5. cd to c:\Trunk\third_party\netcdf-c-4.3.3.1
6. Run the following command to generate the project files:
cmake -G"Visual Studio 11" -D"CMAKE_CONFIGURATION_TYPES=Release;Debug" -D"ENABLE_NETCDF_4=ON" -D"CURL_INCLUDE_DIR=C:\Trunk\third_party\libcurl\include" -D"CURL_LIBRARY=C:\Trunk\third_party\libcurl\libcurl.lib" -D"HDF5_LIB=C:\Trunk\third_party\hdf5\hdf5.lib" -D"HDF5_HL_LIB=C:\Trunk\third_party\hdf5\hdf5_hl.lib" -D"HDF5_INCLUDE_DIR=C:\Trunk\third_party\hdf5\include" -D"USE_SZIP=ON" -D"SZIP_DIR=C:\Trunk\third_party\szip" -D"SZIP_LIBRARY=C:\Trunk\third_party\szip\szip_dynamic.lib" -D"ZLIB_INCLUDE_DIR=C:\Trunk\third_party\zlib\include" -D"ZLIB_LIBRARY=C:\Trunk\third_party\zlib\zlib_pyxis.lib"
7. Open netCDF.sln in Visual Studio 2012
8. In the netcdf project Debug configuration, open Properties->Linker->All Options->Additional Dependencies and change:
	1. hdf5.lib to hdf5_D.lib
	2. hdf5_hl.lib to hdf5_hl_D.lib
	3. zlib_pyxis.lib to zlib_pyxisd.lib
	4. szip_dynamic.lib to szip_dynamicd.lib
	5. libcurl.lib to libcurld.lib
9. Change GenerateProgramDatabaseFile to netcdfd.pdb and ImportLibrary to netcdfd.lib
10. Change Configuration Properties->General->Target Name to netcdfd
11. Build Release and Debug
12. Copy the include folder to \third_party\netcdf. Delete everything but the *.h files
13. Copy the contents of liblib\Release and liblib\Debug to \third_party\netcdf	

\third_party\zips\netcdf-c-4.3.3.1.zip contains all the changes described in step 3. It can be unzipped and rebuilt if needed.

See http://www.unidata.ucar.edu/software/netcdf/docs/getting_and_building_netcdf.html#netCDF-CMake for reference.