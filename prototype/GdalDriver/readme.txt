PYXIS GDAL Driver

This folder contains the code and makefiles for a PYXIS GDAL Driver.
This implementation has been compiled against GDAL 1.5.
 
There are two ways to realize the GDAL driver:

1. Plugin 
Edit make.opt to reflect your system/build environment. 
Run “nmake -f makefile.vc plugin” in /frmts/pyxis. 

2. Integrated
Edit make.opt to reflect your system/build environment; 
and comment out the line “PYXIS_PLUGIN = YES”
Copy all the files into the gdal source folder and make.

