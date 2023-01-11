This README describes how to build hdf5 for PYXIS.
Last updated by: Mark Young
Date: 2015-09-08

1. Download the hdf5 windows build package from https://www.hdfgroup.org/HDF5/release/cmakebuild.html
2. Unzip to \third_party\HDF518CMake
3. Make the following changes to HDF518config.cmake to disable packaging
	Uncomment the line with the LOCAL_NO_PACKAGE variable.
	Uncomment the line with -DHDF5_NO_PACKAGES:BOOL=ON.
	Comment out the line with -DHDF4_PACKAGE_EXTLIBS:BOOL=ON.
4. Make the following change to HDF518config.cmake to build shared libraries
	Comment out the line with -DBUILD_SHARED_LIBS:BOOL=OFF"
5. Change the following lines to use our zlib, szip and jpeg libraries
	####      ext libraries       ####
	### ext libs from tgz
	#set(ADD_BUILD_OPTIONS "${ADD_BUILD_OPTIONS} -DHDF4_ALLOW_EXTERNAL_SUPPORT:STRING=TGZ -DTGZPATH:PATH=	${CTEST_SCRIPT_DIRECTORY}")
	### ext libs from svn
	#set(ADD_BUILD_OPTIONS "${ADD_BUILD_OPTIONS} -DHDF4_ALLOW_EXTERNAL_SUPPORT:STRING=SVN")
	### ext libs on system
	set(ADD_BUILD_OPTIONS "${ADD_BUILD_OPTIONS} -DZLIB_LIBRARY:FILEPATH=${CTEST_DASHBOARD_ROOT}/../zlib/zlib_pyxis.lib -DZLIB_INCLUDE_DIR:PATH=${CTEST_DASHBOARD_ROOT}/../zlib/include")
	set(ADD_BUILD_OPTIONS "${ADD_BUILD_OPTIONS} -DSZIP_LIBRARY:FILEPATH=${CTEST_DASHBOARD_ROOT}/../szip/szip_dynamic.lib -DSZIP_INCLUDE_DIR:PATH=${CTEST_DASHBOARD_ROOT}/../szip/include")
6. Insert the following line at the beginning of build-VS2012-32.bat to use our dll's when running the build tests
	SET PATH=%PATH%;C:\Trunk\third_party\zlib\;C:\Trunk\third_party\szip\
7. To build release, delete the \CMake\build folder if it exists, run VS2012 Native Tools Command Prompt from the start menu and run build-VS2012-32.bat
8. To build debug, make the following changes:
	In build_VS2012-32.bat: change -C Release to -C Debug
	In HDF518config.cmake change:
		set(CTEST_BUILD_CONFIGURATION "Release") to set(CTEST_BUILD_CONFIGURATION "Debug")
		set(ADD_BUILD_OPTIONS "${ADD_BUILD_OPTIONS} -DZLIB_LIBRARY:FILEPATH=${CTEST_DASHBOARD_ROOT}/../zlib/zlib_pyxisd.lib -DZLIB_INCLUDE_DIR:PATH=${CTEST_DASHBOARD_ROOT}/../zlib/include")
		set(ADD_BUILD_OPTIONS "${ADD_BUILD_OPTIONS} -DSZIP_LIBRARY:FILEPATH=${CTEST_DASHBOARD_ROOT}/../szip/szip_dynamicd.lib -DSZIP_INCLUDE_DIR:PATH=${CTEST_DASHBOARD_ROOT}/../szip/include")
9. Copy \third_party\HDF518CMake\hdf5-1.8.15-patch1\src *.h to \third_party\hdf5\include
10. Copy \third_party\HDF518CMake\build\H5pubconf.h to \third_party\hdf5\include
10. Copy hdf5.* and hdf5_hl.* and _D variants from \third_party\HDF518CMake\build\bin to \third_party\hdf5.

\third_party\zips\hdf518CMake.zip contains all the changes described in steps 3-6 above. It can be unzipped and rebuilt if needed.

Note: A few of the build tests fail. The same tests fail for the default build of HDF5 and the build modified as above. These may need to be investigated further if our hdf5 testing uncovers problems.