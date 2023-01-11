/******************************************************************************
file_utils.cpp

begin		: 2004-07-08
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/file_utils.h"

// pyxlib includes
#include "pyxis/utility/exceptions.h"
#include "pyxis/utility/tester.h"

// boost includes
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/convenience.hpp>

// standard includes
#include <io.h>
#include <set>

//! Tester class
Tester<FileUtils> gTester;

//! Test method
void FileUtils::test()
{
	// test the recursive copy and remove all functionality
	{
		boost::filesystem::path testPath("\\file_utils_test", boost::filesystem::native);
		boost::filesystem::remove_all(testPath);
		TEST_ASSERT(!FileUtils::exists(testPath));

		// verify the method does not throw when called on a non existing path
		TEST_ASSERT(!removeContents(testPath));
		boost::filesystem::create_directory(testPath);
		TEST_ASSERT(boost::filesystem::is_empty(testPath));

		// fill with files and directories
		std::ofstream out;
		boost::filesystem::path testFilePath = testPath / "testFile1.tst";
		out.open(FileUtils::pathToString(testFilePath).c_str(), std::ios::out | std::ios::app);
		TEST_ASSERT(out.good());
		TEST_ASSERT(!removeContents(testPath));
		out << "This is a block of data to place in the first test file";
		out.close();

		boost::filesystem::create_directory(testPath / "testdir1");
		boost::filesystem::create_directory(testPath / "testdir2");
		testFilePath = testPath / "testFile2.tst";
		out.open(FileUtils::pathToString(testFilePath).c_str(), std::ios::out | std::ios::app);
		TEST_ASSERT(out.good());
		out << "This is a block of data to place in the second test file";
		out.close();
		boost::filesystem::create_directories(testPath / "testdir3/subdir1");
		testFilePath = testPath / "testdir3/subdir1" / "testFile3.tst";
		out.open(FileUtils::pathToString(testFilePath).c_str(), std::ios::out | std::ios::app);
		TEST_ASSERT(out.good());
		out << "This is a block of data to place in the third test file";
		out.close();
		boost::filesystem::create_directory(testPath / "testdir4");
		TEST_ASSERT(!boost::filesystem::is_empty(testPath));

#ifdef _WINDOWS
		// create a sub directory and files with spaces in the names
		boost::filesystem::path spacePath = testPath /
			boost::filesystem::path("a dir with spaces", boost::filesystem::native);
		boost::filesystem::create_directory(spacePath);
		spacePath /= boost::filesystem::path("file with spaces.tst", boost::filesystem::native);
		out.open(FileUtils::pathToString(spacePath).c_str(), std::ios::out | std::ios::app);
		TEST_ASSERT(out.good());
		out << "This is a block of data to place in the spaced test file";
		out.close();
#endif

		// test the copy method
		boost::filesystem::path testPathCopy("\\file_utils_test_copy", boost::filesystem::native);
		TEST_ASSERT(FileUtils::recursiveCopy(testPath, testPathCopy));
		TEST_ASSERT(FileUtils::calcSize(testPath) == FileUtils::calcSize(testPathCopy));
		boost::filesystem::remove_all(testPathCopy);
		
		// test the remove contents method
		FileUtils::removeContents(testPath);
		TEST_ASSERT(FileUtils::exists(testPath));
		TEST_ASSERT(boost::filesystem::is_empty(testPath));
		boost::filesystem::remove_all(testPath);
	}

	// TODO Write test for the calc size and remove oldest methods
}

/*!
Delete all files and directories from within a directory but leaving the 
path on disk. If the passed path is not a directory no action is taken.

\param path	The directory to remove all of the contents of.

\return true if the specified path is a directory that is empty at the end of 
		the operation.
*/
bool FileUtils::removeContents(boost::filesystem::path& path)
{
	bool bSuccess = true;
	if (FileUtils::exists(path) && FileUtils::isDirectory(path))
	{
		TRACE_INFO("About to remove path: " << FileUtils::pathToString(path));
		boost::filesystem::directory_iterator itEnd;
		for (	boost::filesystem::directory_iterator itDir(path); 
			itDir != itEnd; 
			++itDir)
		{
			try
			{
				boost::filesystem::remove_all(*itDir);
			}
			catch (...)
			{
				boost::filesystem::path path = (*itDir).path();
				TRACE_ERROR("Unable to remove all contents from '" << 
					FileUtils::pathToString(path) << 
					"'. Files possibly in use...");
				bSuccess = false;
			}
		}
		return bSuccess;
	}

	return false;

}

/*!
Recursively iterate over all contents of a directory and determine
the total size.

\param path	The directory or file to determine the size of.

\return The size of the directory in bytes as reported by the OS.
*/
boost::intmax_t FileUtils::calcSize(const boost::filesystem::path& path)
{
	boost::intmax_t nTotalSize = 0;
	if (!FileUtils::exists(path))
	{
		return 0;
	}

	// return the size of the only file
	if (!FileUtils::isDirectory(path))
	{
		try
		{
			nTotalSize = boost::filesystem::file_size(path);
		}
		catch (...)
		{
			TRACE_ERROR("Error occurred during size calculation.");
		}
	}
	else
	{
		// iterate over each of the files or directories
		boost::filesystem::directory_iterator itEnd;
		for (boost::filesystem::directory_iterator itDir(path); itDir != itEnd; ++itDir)
		{
			nTotalSize += calcSize(*itDir);
		}
	}

	return nTotalSize;
}


/*
This method updates the 'last write time' of a file. The input file path
must be that of an existing file (not directory).

\param file	The file to 'touch'
\return true if the file write time was changed, otherwise false.
*/
bool FileUtils::touchFile(boost::filesystem::path file)
{
	try
	{
		if (file.is_complete() && 
			FileUtils::exists(file) && 
			!FileUtils::isDirectory(file))
		{
			// get the current system time
			std::time_t time;
			std::time(&time);
			boost::filesystem::last_write_time(file, time);
			return true;
		}
	}
	catch (...)
	{
		TRACE_ERROR("An error occurred while trying to set the write time on file: " 
			<< FileUtils::pathToString(file));
	}
	return false;
}

/*!
Copy a file or a folder full of files (recursively).  Error handling is very minimal
in this implementation; the function simply returns false and DOES NOT clean up any
partial copy results.

\param src	Boost path to original (must be a valid path to an existing item)
\param dst	Boost path for copy (must not exist prior to copy)

\return true on success, false on any failure.
*/
bool FileUtils::recursiveCopy(boost::filesystem::path& src, boost::filesystem::path& dst)
{
	try
	{
		if (!FileUtils::isDirectory(src))
		{
			// not a directory: just copy 1 file
			boost::filesystem::copy_file(src, dst);
		}
		else
		{
			// create new destination directory
			boost::filesystem::create_directory(dst);

			if (!FileUtils::isDirectory(dst))
			{
				TRACE_ERROR("Unable to create directory '" << 
							FileUtils::pathToString(dst) << 
							"' during copy operation.");
				return false;
			}

			// recursively copy files/subdirectories
			boost::filesystem::directory_iterator itEnd;
			boost::filesystem::directory_iterator itDir(src);
			while (itDir != itEnd)
			{
				boost::filesystem::path srcItem = *itDir;
				if (FileUtils::exists(srcItem))
				{
					boost::filesystem::path dstItem = dst / srcItem.leaf();
					if (!recursiveCopy(srcItem, dstItem))
					{
						return false;
					}
				}
				++itDir;
			}
		}

		return true;
	}
	catch (std::exception& e)
	{
		TRACE_ERROR("File system error during recursive copy: \n\n" << e.what());
	}
	return false;
}

/*!
Sanitize the name for cross-platform usage (eg. replace spaces with underscores).

\param strName	The original file or folder name.

\return The new name.
*/
std::string FileUtils::sanitizeName(const std::string& strName)
{
	std::string strNewName = strName;
	std::replace(strNewName.begin(), strNewName.end(), ' ', '_');
	return strNewName;
}

/*!
Get the extension of a file or the extension of the file in the entire path.

\param strPath	The file or the path of the file to get the extension of.

\return The extension of the file with the '.' or the empty string if no extension found.
*/
std::string FileUtils::getExtensionWithDot(const std::string &strPath)
{
	size_t nPos = strPath.find_last_of(".");
	if (nPos != std::string::npos)
	{
		std::string strExt = strPath.substr(nPos, std::string::npos);
		return strExt;
	}

	return "";
}

/*!
Get the extension of a file or the extension of the file in the entire path.

\param strPath	The file or the path of the file to get the extension of.

\return The extension of the file without the '.' or the empty string if no extension found.
*/
std::string FileUtils::getExtensionNoDot(const std::string &strPath)
{
	size_t nPos = strPath.find_last_of(".");
	if (nPos != std::string::npos)
	{
		std::string strExt = strPath.substr(nPos + 1, std::string::npos);
		return strExt;
	}

	return "";
}

//! Wraps calls to boost:filesystem::exists.  There is a breaking change in boost - exists can throw!
bool FileUtils::exists(const boost::filesystem::path &file)
{
	try
	{
		boost::system::error_code ec;
		auto status = boost::filesystem::status(file, ec);
		return boost::filesystem::exists( status );
	}
	catch (...)
	{
		return false;
	}
}

//! Remove a file 
bool FileUtils::remove(const boost::filesystem::path &file)
{
	try
	{
		boost::system::error_code ec;
		return boost::filesystem::remove( file, ec);
	}
	catch (...)
	{
		return false;
	}
}

boost::recursive_mutex s_boost_path_mutex;

//! Convert a path type into a string
std::string FileUtils::pathToString(const boost::filesystem::path & src)
{
	//It seems boost::filesystem::path::string() on windows is not thread safe
	// https://svn.boost.org/trac/boost/ticket/6737
	// this might change in the next version of boost
	boost::recursive_mutex::scoped_lock lock(s_boost_path_mutex);
	return src.string();
}

//! Convert a string into a path type
boost::filesystem::path FileUtils::stringToPath(const std::string & strPath)
{
	// It seems boost::filesystem::path() on windows is not thread safe
	// https://svn.boost.org/trac/boost/ticket/6320
	boost::recursive_mutex::scoped_lock lock(s_boost_path_mutex);
	return boost::filesystem::path(strPath, boost::filesystem::native);
}

//! Determine if a path is a directory - thread safe
bool FileUtils::isDirectory(const boost::filesystem::path & src)
{
	// It seems boost::filesystem::is_directory() on windows is not thread safe
	boost::recursive_mutex::scoped_lock lock(s_boost_path_mutex);
	return boost::filesystem::is_directory(src);
}

//! Get the extension from a path with the '.' - thread safe
std::string FileUtils::getExtensionWithDot(const boost::filesystem::path & src)
{
	// It seems boost::filesystem::extension() on windows is not thread safe
	boost::recursive_mutex::scoped_lock lock(s_boost_path_mutex);
	return src.extension().string();
}

//! Get the extension from a path without the '.' - thread safe
std::string FileUtils::getExtensionNoDot(const boost::filesystem::path & src)
{
	std::string strExt(getExtensionWithDot(src));
	if (strExt.length() > 0)
	{
		return strExt.substr(1, std::string::npos);
	}

	return strExt;
}

/*!
For the given file path and extensions, determine if files are missing from the file's directory.

For example for file path "/mydirectory/roads.shp" and extensions "shx" and "dbf", the method will
return "roads.shx" and "roads.dbf" if the files are missing from /mydirectory.

\param strPath			The file path
\param vecExtensions	The file extensions without the "."

\return The missing files or an empty vector if no files are missing.
*/
std::auto_ptr< std::vector<std::string> > FileUtils::findMissingFiles(
	const std::string& strPath,
	const std::vector<std::string>& vecExtensions	)
{
	std::auto_ptr< std::vector<std::string> > pvecMissingFiles(new std::vector<std::string>());

	// protect folder deletion with try/catch
	try
	{
		auto path = stringToPath(strPath);
		if (exists(path) && !isDirectory(path))
		{
			for (auto& it : vecExtensions)
			{
				auto supportFile = path;
				supportFile.replace_extension(it);

				if (!exists(supportFile))
				{
					pvecMissingFiles->push_back(FileUtils::pathToString(supportFile.leaf()));
				}
			}
		}
	}
	catch (...)
	{
		// fall through
	}

	return pvecMissingFiles;
}