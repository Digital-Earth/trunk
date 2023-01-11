How GDAL get compiled in VS2012

wiki page:
   https://www.pyxisinnovation.com/pyxinternalwiki/index.php?title=How_to_compile_GDAL

1) you need to extract the zips for OGDI and PROJ.
2) compile PROJ and OGDI
3) run VS2012 x86 Native Tools Command Prompt from the Windows Start Menu
4) run build_gdal.bat > log.txt from the Command Prompt - then you have a log file plus all errors will be send to the console so you can deal with the problems. Note that pyxis.nmake.opt is used to specify the compile options.