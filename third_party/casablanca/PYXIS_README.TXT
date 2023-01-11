This README describes how to build casablanca for PYXIS.
Last updated by: Mark Young
Date: 2016-03-10

1. Make sure git is installed.
2. Use the command "git clone https://git01.codeplex.com/casablanca" to get the source. This will clone the Casablanca source to a directory named "casablanca". Note: depending on the compiler you wish to support, you may need to get an earlier tag.
3. Copy to \third_party\casablanca-x.y.z where x.y.z is the version number.
4. Open cpprestsdk110.sln and build Release and Debug.
5. Copy the include folder to \third_party\casablanca
6. Copy the Release and Debug cpprest110 and cpprest110d lib, dll and pdb files to \third_party\casablanca
A number of third party libraries use zlib. The _pyxis extension is used to avoid conflict with third party libraries that supply their own zlib (for example Teigha).

\third_party\zips\casablanca-2.5.0.zip contains all the changes described in steps 1 and 2 above. It can be unzipped and rebuilt if needed.
