Summary of changes made by the PYXIS TEAM:
------------------------------------------

please note that to get the complete list of changes you might want to download the older version of swig and diff it with the original 1.3.30 files.

Changes:

Lib/csharp/csharphead.swg
1) Using Utf8 as string conversion - we have changed the default ANSI string parsing to Utf8 text encoding.
2) solving a .net4 crash when marshaling an exception due to missing static constructor on pyxlibPINVOKE class (and others)
   based on:

   2010-05-23: wsfulton
           [C#] Fix #2957375 - SWIGStringHelper and SWIGExceptionHelper not always being
           initialized before use in .NET 4 as the classes were not marked beforefieldinit.
           A static constructor has been added to the intermediary class like this:

             %pragma(csharp) imclasscode=%{
               static $imclassname() {
               }
             %}

           If you had added your own custom static constructor to the intermediary class in
           the same way as above, you will have to modify your approach to use static variable
           initialization or define SWIG_CSHARP_NO_IMCLASS_STATIC_CONSTRUCTOR - See csharphead.swg.