/******************************************************************************
app_services.cpp

begin		: 2004-10-05
copyright	: derived from app_services.cpp (C) 2000 by iGO Technologies Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h" 
#include "pyxis/utility/app_services.h"

// pyxlib includes
#include "pyxis/utility/trace.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/exceptions.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

// boost includes
#include <boost/filesystem/operations.hpp>

// standard includes
#include <cassert>

// static variables
std::string AppServices::m_strAppName = "";
boost::filesystem::path AppServices::m_appPath = "";
boost::filesystem::path AppServices::m_tempPath = "";
boost::filesystem::path AppServices::m_workingPath = "";
boost::filesystem::path AppServices::m_cachePath = "";
boost::filesystem::path AppServices::m_libraryPath = "";

boost::recursive_mutex AppServices::m_mutex;

std::auto_ptr<Properties> AppServices::m_spProps;
bool AppServices::m_bInitialized = false;
std::map<std::string,std::string> AppServices::m_appConfiguration;

//! The temporary directory name
static const std::string kstrTempDir = "PYXTemp";

//! The cache directory name
static const std::string kstrCacheDir = "PYXCache";

//! The library directory name
static const std::string kstrLibraryDir = "PYXLibrary";

//! The dot
static const std::string kstrDot = ".";

//! The PYXIS protocol name
static const std::string kstrPyxisProtocolName = "pyxis";

//! The PYXIS protocol string
static const std::string kstrPyxisProtocol = "pyxis://";

//! The number of times to try when creating a temporary file
static const int knMaxRetries = 500;

//! Tester class
Tester<AppServices> gTester;

//! Test method
void AppServices::test()
{
	// Test that the application path is initialized.
	boost::filesystem::path filePath = AppServices::getApplicationPath();
	TEST_ASSERT(!filePath.empty());

	// test that the working path is assigned.
	filePath = AppServices::getWorkingPath();
	TEST_ASSERT(!filePath.empty());

	// test that the library path is assigned.
	filePath = AppServices::getLibraryPath();
	TEST_ASSERT(!filePath.empty());

	// verify the internal temp directory is assigned
	TEST_ASSERT(!m_tempPath.empty());

	// verify a valid temporary directory is created.
	filePath = AppServices::makeTempDir();
	TEST_ASSERT(!filePath.empty());
	TEST_ASSERT(FileUtils::exists(filePath));
	{
		std::ofstream testOut;
		filePath /= "test.test";
		testOut.open(FileUtils::pathToString(filePath).c_str(), std::ios::out | std::ios::app);
		TEST_ASSERT(testOut.good());		
	}

	// verify files with or without extensions can be created
	filePath = AppServices::makeTempFile();
	TEST_ASSERT(!filePath.empty());
	TEST_ASSERT(FileUtils::exists(filePath));
	{
		std::ofstream testOut;
		testOut.open(FileUtils::pathToString(filePath).c_str(), std::ios::out | std::ios::app);
		TEST_ASSERT(testOut.good());		
	}

	filePath = AppServices::makeTempFile(".test");
	TEST_ASSERT(!filePath.empty());
	TEST_ASSERT(FileUtils::exists(filePath));
	{
		std::ofstream testOut;
		testOut.open(FileUtils::pathToString(filePath).c_str(), std::ios::out | std::ios::app);
		TEST_ASSERT(testOut.good());		
	}

	boost::filesystem::path old_WorkingPath = getWorkingPath(); // Get it and save it before testing.
	boost::filesystem::path tempPath = makeTempDir();
	TRACE_TEST("Creating temp working path directory for testing: " << FileUtils::pathToString(tempPath));
	setWorkingPath(FileUtils::pathToString(tempPath));
	filePath = getWorkingPath();
	TEST_ASSERT(!filePath.empty());
	setWorkingPath(FileUtils::pathToString(old_WorkingPath));
	filePath = getWorkingPath();
	TEST_ASSERT(filePath == old_WorkingPath);
}

/*!
Initialize the application services. Read the properties, setup trace. This method
must be called at the very start of program execution.

\param strAppName			The application name, used to name files and directories.
\param bClearCache			If true, any existing cache is cleared
\param strWorkingDir		The absolute path to the directory to store all 
							dynamically generated files. This includes temp files,
							trace files, and properties files. Leaving the path blank
							will co-locate the working directory to the 
							application directory.
\param strApplicationDir	The directory where the WorldView exe is located.
*/
void AppServices::initialize(	const std::string& strAppName,
								bool bClearCache,
								const std::string& strWorkingDir, 
								const std::string& strApplicationDir,
								const std::string& strCacheDir )
{
	assert(!strAppName.empty() && "Must provide a name for the application.");
	if (m_bInitialized)
	{
		PYXTHROW(PYXException, "AppServices already initialized."); 
	}

	// store the application directory
	if (strApplicationDir.empty())
	{
		m_appPath = boost::filesystem::initial_path();
	}
	else
	{
		m_appPath = FileUtils::stringToPath(strApplicationDir);
	}
	
	assert(FileUtils::exists(m_appPath) && 
		"Application path does not exist");

	// store the working directory of the application
	m_workingPath = FileUtils::stringToPath(strWorkingDir);
	if (!strWorkingDir.empty() && m_workingPath.is_complete())
	{
		if (!FileUtils::isDirectory(m_workingPath))
		{
			try 
			{
				boost::filesystem::create_directory(m_workingPath);
				TRACE_INFO("Created new directory for working path at '" <<
							FileUtils::pathToString(m_workingPath) << "'.");
			}
			catch (...)
			{
				m_workingPath = m_appPath;
				TRACE_ERROR("Could not create working directory '" <<
							FileUtils::pathToString(m_workingPath) << 
							"', defaulting to application path '" <<
							FileUtils::pathToString(m_appPath) << "'.");				
			}
		}
	}
	else
	{
		m_workingPath = m_appPath;
		TRACE_INFO(	
			"Could not use working path '" << strWorkingDir << 
			"', using application path '" << 
			FileUtils::pathToString(m_appPath) << "' for working directory");
	}

	// set the application name
	m_strAppName = strAppName;

	// initialize the application properties file and levels.
	m_spProps.reset(new Properties(m_strAppName));

	// seed the random number generator
	int nSeed = static_cast<int>(time(0));

#if 0
	// The seed can be set to a specific value for debugging, if necessary.
	nSeed = 1155884183;
#endif

	TRACE_INFO("Seeding the random number generator with '" << nSeed << "'.");
	srand(nSeed);

	// create the temporary directory
	try
	{
		// verify the temporary directory
		m_tempPath = m_workingPath / kstrTempDir;
		if (FileUtils::isDirectory(m_tempPath))
		{
			// empty the contents of the directory.
			if (!FileUtils::removeContents(m_tempPath))
			{
				TRACE_ERROR("Unable to remove all contents from the temporary directory: "
					<< FileUtils::pathToString(m_tempPath)	);
			}
		}
		else
		{
			// create the empty directory
			try
			{
				boost::filesystem::create_directory(m_tempPath);
			}
			catch (...)
			{
				PYXTHROW(PYXException, "Unable to create new temp directory.");
			}
		}

		// Initialize the cache root directory by looking for a custom setting
		m_cachePath = m_workingPath / kstrCacheDir;
		std::string strDefault = FileUtils::pathToString(m_cachePath);
		std::string strUserCacheDir = getAppProperty( 
						"App_Services", 
						"Cache_Directory", 
						strDefault,
						"The root directory in which to store all persistent data files.");
		
		//if cache is provided - use that
		if (!strCacheDir.empty())
		{
			m_cachePath = FileUtils::stringToPath(strCacheDir);
		}
		//if user provided a cached dir from configuration file
		else if (strUserCacheDir != strDefault)
		{
			// the user did not specify a directory, use the standard working path plus kstrCacheDir
			m_cachePath = FileUtils::stringToPath(strUserCacheDir);
		}

		// clear the cache directory
		if (FileUtils::isDirectory(m_cachePath) && bClearCache)
		{
			// delete the cache directory and its contents
			try
			{
				boost::filesystem::remove_all(m_cachePath);
			}
			catch(...)
			{
				PYXTHROW(PYXException, "Unable to clear cache directory: " << 
					FileUtils::pathToString(m_cachePath));
			}
		}

		if (!FileUtils::isDirectory(m_cachePath))
		{
			// create the empty directory
			try
			{
				boost::filesystem::create_directory(m_cachePath);
			}
			catch (...)
			{
				PYXTHROW(PYXException, "Unable to create new cache directory: " << 
					FileUtils::pathToString(m_cachePath));
			}
		}

		// set library path
		m_libraryPath = m_workingPath / kstrLibraryDir;
		if (!FileUtils::isDirectory(m_libraryPath))
		{
			// create the directory
			try
			{
				boost::filesystem::create_directory(m_libraryPath);
			}
			catch (...)
			{
				PYXTHROW(PYXException, "Unable to create new library directory: " << 
					FileUtils::pathToString(m_libraryPath));
			}
		}
	}
	catch (PYXException& e)
	{
		PYXRETHROW(e, PYXException, "Unable to initialize working directories.");
	}

	TRACE_INFO("Application services initialized with name '" << strAppName << "'.");
	m_bInitialized = true;
}


/*!
Set the working path of the application to that of a the boost path constructed from the 
parameter. If the path doesn't exist we'll try to create it otherwise default to the 
application path.

\param strWorkingPath The string representation of the working path we wish to set.
*/
void AppServices::setWorkingPath(const std::string &strWorkingPath)
{
	//--
	//-- TODO: This procedure does not reposition the directories that are suppose
	//--       to be positioned under the working path (eg. the library and cache directories).
	//--       Not really a huge issue because this procedure is only used by a test unit, 
	//--       that does not recheck the relative positions of the sub directories.
	//--
	// store the working directory of the application
	m_workingPath = FileUtils::stringToPath(strWorkingPath);
	if (!strWorkingPath.empty() && m_workingPath.is_complete())
	{
		if (!FileUtils::exists(m_workingPath))
		{
			try 
			{
				boost::filesystem::create_directory(m_workingPath);
				TRACE_INFO("Created new directory for working path at '" <<
							FileUtils::pathToString(m_workingPath) << "'.");
			}
			catch (...)
			{
				m_workingPath = m_appPath;
				TRACE_ERROR("Could not create working directory '" <<
							FileUtils::pathToString(m_workingPath) << 
							"', defaulting to application path '" <<
							FileUtils::pathToString(m_appPath) << "'.");				
			}
		}
	}
	else
	{
		m_workingPath = m_appPath;
			TRACE_INFO(	
				"Could not use working path '" << strWorkingPath << 
				"', using application path '" << 
				FileUtils::pathToString(m_appPath) << "' for working directory");
	}
}
	
/*!
Destroy the application services and append new properties to properties file.
*/
void AppServices::uninitialize()
{
	try
	{
		// delete the temporary directory and its contents

		// TODO: Fix this hack.  Consider delete on reboot.
		// Currently disabled because of autobuild failure.  The init routine also removes this dir, so it will get cleaned up.
		//boost::filesystem::remove_all(m_tempPath);
//		TRACE_INFO("Temporary directory deleted");
		TRACE_INFO("Temporary directory NOT deleted.");
	}
	catch(...)
	{
		TRACE_ERROR("Unable to remove temporary directory: " << FileUtils::pathToString(m_tempPath));
	}

	// write out the application properties in the destructor
	m_spProps.reset();

	TRACE_DEBUG("Application services destroyed.");
	Trace::getInstance()->destroy();
	m_bInitialized = false;
}

/*!
The application directory is the current working directory when the
AppServices module is initialized. When done properly, this is the
executable directory.

\return The path to the PYXIS application directory.
*/
const boost::filesystem::path& AppServices::getApplicationPath()
{
	if (FileUtils::pathToString(m_appPath) == "")
	{
		PYXTHROW(PYXException, "Can't get App path, AppServices not initialized.");
	}
	
	TRACE_INFO("The application path is: " + FileUtils::pathToString(m_appPath));
	return m_appPath;
}

/*!
The working directory is where all PYXIS generated files are 
placed. Files such as properties, trace and temporary files and
directories.

\return The path to the PYXIS working directory.
*/
const boost::filesystem::path& AppServices::getWorkingPath()
{
	if (FileUtils::pathToString(m_workingPath) == "")
	{
		PYXTHROW(PYXException, "Can't get working path, AppServices not initialized.");
	}
	return m_workingPath;
}

/*!
The library directory is where all PYXIS generated pipeline data is stored. 

\return The path to the PYXIS library directory.
*/
const boost::filesystem::path& AppServices::getLibraryPath()
{
	if (FileUtils::pathToString(m_libraryPath) == "")
	{
		PYXTHROW(PYXException, "Can't get library path, AppServices not initialized.");
	}
	return m_libraryPath;
}

/*!
The base cache directory is where root directory where all cache data is stored. 

\return The path to the base PYXIS cache directory.
*/
const boost::filesystem::path& AppServices::getBaseCachePath()
{
	if (FileUtils::pathToString(m_cachePath) == "")
	{
		PYXTHROW(PYXException, "Can't get base cache path, AppServices not initialized.");
	}
	return m_cachePath;
}

/*!
The PYXIS protocol string (a custom protocol similar to mailto:).

\return The PYXIS protocol string.
*/
const std::string& AppServices::getPyxisProtocol()
{
	return kstrPyxisProtocol;
}

/*!
The name of the PYXIS protocol (a custom protocol similar to mailto:).

\return The name of the PYXIS protocol.
*/
const std::string& AppServices::getPyxisProtocolName()
{
	return kstrPyxisProtocolName;
}

/*!
Create a temporary file in the "temp" folder of the application directory.
This file and the temp folder will be deleted when destroy() is called.

\param strExtension		The extension of the file to create. If you want a 
						'.' to separate the file name and the extension the
						caller must include it in the extension string.

\return	The full path and name of the temporary file.
*/
boost::filesystem::path AppServices::makeTempFile(const std::string& strExtension)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	assert(m_bInitialized && "App services not initialized.");
	static int fileNumber = 0;

	boost::filesystem::path filePath;
	for (int nRetry = 0; nRetry < knMaxRetries; ++nRetry)
	{
		fileNumber += rand() % 32;
		std::string strFileName = StringUtils::toString(fileNumber);

		if (!strExtension.empty())	
		{
			strFileName += strExtension;
		}

		filePath = m_tempPath / strFileName;

		if (!FileUtils::exists(filePath))
		{
			// create the file
			std::ofstream out;
			out.open(FileUtils::pathToString(filePath).c_str(), std::ios::out | std::ios::app);
			if (out.good())
			{
				return filePath;
			}
		}
	}

	PYXTHROW(PYXFileException, "Unable to create temporary file.");
}

/*!
Create a temporary directory in the "PYXTemp" folder of the application directory.
This directory and the temp folder will be deleted when destroy() is called.

This method call is not thread safe because the use of rand() is not a 
thread safe call.

\return	The full path and name of the directory.
*/
boost::filesystem::path AppServices::makeTempDir()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	assert(m_bInitialized && "App services not initialized.");

	for (int nRetry = 0; nRetry < knMaxRetries; ++nRetry)
	{
		int nRand = rand();
		boost::filesystem::path dirPath = m_tempPath / StringUtils::toString(nRand);
		if (!FileUtils::exists(dirPath))
		{
			try
			{
				boost::filesystem::create_directory(dirPath);
			}
			catch (...)
			{
				PYXTHROW(PYXFileException, 
					"Creation of temp directory '" <<
					FileUtils::pathToString(dirPath) << 
					"' failed.");
			}
			return dirPath;
		}
	}

	PYXTHROW(PYXFileException, "Unable to create unique temp directory name.");
}

/*!
Get the trace file path.
*/
boost::filesystem::path AppServices::getTraceFilePath()
{
	return getWorkingPath() / "trace.log";
}

/*!
Make a new, or retrieve an existing directory within the cache by name. If the
system is unable to create the directory an exception is thrown. The name that
is passed in will have spaces removed and replaced with '_' characters to
remain portable. As such, 

\param strDirName	The name of the specific cache directory that the caller
					wishes to retrieve (relative to the cache directory).

\return The full path to the desired directory.
*/
boost::filesystem::path AppServices::getCacheDir(const std::string& strDirName)
{
	// Create the full path.
	boost::filesystem::path path = m_cachePath / FileUtils::sanitizeName(strDirName);

	boost::recursive_mutex::scoped_lock lock(m_mutex);
	assert(m_bInitialized && "App services not initialized.");
	if (!FileUtils::exists(path))
	{
		try
		{
			boost::filesystem::create_directory(path);
		}
		catch (...)
		{
			PYXTHROW(PYXFileException, 
				"Creation of custom cache directory '" <<
				FileUtils::pathToString(path) << 
				"' failed.");
		}
	}
	return path;
}

//! Return a single mutex that allow modules in the application to sync their operations.
boost::recursive_mutex & AppServices::getExecutionScope(const std::string & scope)
{
	static boost::recursive_mutex scopesLock;	
	static std::map<std::string,boost::shared_ptr<boost::recursive_mutex>> scopes;

	boost::recursive_mutex::scoped_lock lock(scopesLock);
	
	auto mutexPtr = scopes.find(scope);
	if (mutexPtr != scopes.end())
	{
		return *(mutexPtr->second);
	}
	auto newMutex = boost::shared_ptr<boost::recursive_mutex>(new boost::recursive_mutex());
	scopes[scope] = newMutex;
	return *newMutex;
}

const std::string & AppServices::getConfiguration(const std::string & configKey)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	return m_appConfiguration[configKey];
}

void AppServices::setConfiguration(const std::string & configKey, const std::string & value)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	if (m_bInitialized)
	{
		PYXTHROW(PYXException,"Can't update configuration after app been initialized");
	}

	m_appConfiguration[configKey] = value;
}

const std::string AppServicesConfiguration::localStorageFormat = "localStorageFormat";
const std::string AppServicesConfiguration::localStorageFormat_files = "files";
const std::string AppServicesConfiguration::localStorageFormat_sqlite = "sqlite";

const std::string AppServicesConfiguration::importMemoryLimit = "importMemoryLimit";

/*!
This method will query the application properties file and determine if a
particular key exists within a scope.

\param strScope	The scope to search within.
\param strKey	The key to search for.

\return true if the value was found otherwise false.
*/
bool appPropertyExists(	const std::string& strScope,
						const std::string& strKey	)
{
	return AppServices::m_spProps->propertyExists(strScope, strKey);
}

/*!
Return a boolean value for an application property. If the value does not 
exist the passed default value is added to the properties file under
the key and scope and that same default value is returned.

\param strScope			The scope of the property (usually class name).
\param strKey			The key within the scope to retrieve a value for.
\param defaultValue		This value is returned if the properties file 
						has not been defined yet.

\return The value returned from the properties file.

/sa getAppProperty
*/
bool getAppProperty(	
	const std::string& strScope,
	const std::string& strKey,
	const bool& defaultValue	)
{
	bool bOutVal = defaultValue;

	if (0 != AppServices::m_spProps.get())
	{
		bOutVal = getProperty(	*(AppServices::m_spProps.get()), 
								strScope, 
								strKey, 
								("Boolean value for the key " + strKey),
								defaultValue	);
	}
	
	return bOutVal;
}

/*!
Return an unsigned long value for an application property. If the value does not 
exist the passed default value is added to the properties file under
the key and scope and that same default value is returned.

\param strScope			The scope of the property (usually class name).
\param strKey			The key within the scope to retrieve a value for.
\param defaultValue		This value is returned if the properties file 
						has not been defined yet.

\return The value returned from the properties file.

/sa getAppProperty
*/
unsigned long getAppProperty(	
	const std::string& strScope,
	const std::string& strKey,
	const unsigned long& defaultValue	)
{
	unsigned long nOutVal = defaultValue;

	if (0 != AppServices::m_spProps.get())
	{
		nOutVal = getProperty(	*(AppServices::m_spProps.get()), 
								strScope, 
								strKey, 
								("Unsigned long value for the key " + strKey),
								defaultValue	);
	}
	
	return nOutVal;
}
