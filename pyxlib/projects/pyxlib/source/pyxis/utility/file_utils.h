#ifndef PYXIS__UTILITY__FILE_UTILS_H
#define PYXIS__UTILITY__FILE_UTILS_H
/******************************************************************************
file_utils.h

begin		: 2004-07-08
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

// standard includes
#include <string>

// boost includes
#include <boost/filesystem/operations.hpp>

/*!
This class contains various utilities for managing files and directories in a
cross platform manner. 
*/
//! Various file utilities
class PYXLIB_DECL FileUtils
{
public:

	//! Test method
	static void test();

	//! Delete all contents of a directory.
	static bool removeContents(boost::filesystem::path& path);

	//! Compute the size of all contents of a directory.
	static boost::intmax_t calcSize(const boost::filesystem::path& path);

	//! Copy 1 file or a directory full of them (recursively).
	static bool recursiveCopy(	boost::filesystem::path& src,
								boost::filesystem::path& dst);

	//! Sanitize the file or folder name for cross-platform usage (eg. replace spaces with underscores).
	static std::string sanitizeName(const std::string& strName);

	//! Get the extension of a file with the '.'
	static std::string getExtensionWithDot(const std::string& strPath);

	//! Get the extension of a file without the '.'
	static std::string getExtensionNoDot(const std::string& strPath);

	//! Update the last accessed time of a file with the current system time
	static bool touchFile(boost::filesystem::path file);

	//! Wraps calls to boost:filesystem::exists
	static bool exists(const boost::filesystem::path &file);

	//! Wraps calls to boost:filesystem::remove
	static bool remove(const boost::filesystem::path &file);

	//! Convert a path type into a string - thread safe
	static std::string pathToString(const boost::filesystem::path & src);

	//! Convert a string into a path type - thread safe
	static boost::filesystem::path stringToPath(const std::string & strPath);

	//! Determine if a path is a directory - thread safe
	static bool isDirectory(const boost::filesystem::path & src);

	//! Get the extension from a path with the '.' - thread safe
	static std::string getExtensionWithDot(const boost::filesystem::path & src);

	//! Get the extension from a path without the '.' - thread safe
	static std::string getExtensionNoDot(const boost::filesystem::path & src);

	//! For a given file path and extensions, get the files with the same name that are missing from the directory.
	static std::auto_ptr< std::vector<std::string> > FileUtils::findMissingFiles(
		const std::string& strPath,
		const std::vector<std::string>& vecExtensions	);
};

#endif
