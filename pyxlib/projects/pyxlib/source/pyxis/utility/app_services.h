#ifndef PYXIS__UTILITY__APP_SERVICES_H
#define PYXIS__UTILITY__APP_SERVICES_H
/******************************************************************************
app_services.h

begin		: 2004-10-05
copyright	: derived from app_services.h (C) 2000 by iGO Technologies Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/properties.h"

// boost includes
#include <boost/cstdint.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/thread/recursive_mutex.hpp>

// standard includes

/*!
This class manages a set of services provided to the application. It currently
manages a set of application properties, trace settings and temporary files. It
also seeds the random number generator used by rand().

\code
int main(int, char* [])
{
	AppServices::initialize(strAppName);

	// Application body here.

	AppServices::destroy();
	return 0;
}
\endcode
*/
//! Manages services used by the application.
class PYXLIB_DECL AppServices
{
public:

	//! Test method
	static void test();

	//! Initialize the application services
	static void initialize(	const std::string& strAppName,
							bool bClearCache,
							const std::string& strWorkingDir, 
							const std::string& strApplicationDir,
							const std::string& strCacheDir = "");
	
	//! Destroy the application services
	static void uninitialize();

	//! Return the application name.
	static const std::string& getAppName() {return m_strAppName;}

	//! Create a temporary file.
	static boost::filesystem::path makeTempFile(
		const std::string& strExtension = std::string("")	);

	//! Create a temporary directory.
	static boost::filesystem::path makeTempDir();

	//! Get the trace file path.
	static boost::filesystem::path getTraceFilePath();

	//! Create or fetch a cache directory by name.
	static boost::filesystem::path getCacheDir(const std::string& strDirName);

	//! Return the path to the executable directory.
	static const boost::filesystem::path& getApplicationPath();

	//! Return the path to the working directory.
	static const boost::filesystem::path& getWorkingPath();

	//! Return the path to the library directory.
	static const boost::filesystem::path& getLibraryPath();

	//! Return the path to the base cache directory.
	static const boost::filesystem::path& getBaseCachePath();

	//! Return the Pyxis protocol string.
	static const std::string& getPyxisProtocol();

	//! Return the name of the Pyxis protocol.
	static const std::string& getPyxisProtocolName();

	//! Return a single mutex that allow modules in the application to sync their operations.
	static boost::recursive_mutex & getExecutionScope(const std::string & scope);

	//! The application properties
	static std::auto_ptr<Properties> m_spProps;

	static const std::string & getConfiguration(const std::string & configKey);

	static void setConfiguration(const std::string & configKey, const std::string & value);

private:
	//
	// changed to private because there may be a fundemental flaw
	// with this route.  If someone wants to use this routine,
	// they need to resolve the side effects associated with using
	// this code.    --nle-- Aug 12, 2009
    //
	//! Set the working path of the application.
	static void setWorkingPath(const std::string& strWorkingPath);

protected:

private:

	//! Default constructor defined so compiler won't generate public one
	AppServices();

	//! Copy constructor defined so compiler won't generate public one
	AppServices(const AppServices&);

	//! Assignment operator defined so compiler won't generate public one
	void operator= (const AppServices&);

	//! The application name
	static std::string m_strAppName;

	//! The default working directory for the application. Should be the
	//! current working directory at all times.
	static boost::filesystem::path m_appPath;

	//! Path to temporary directory.
	static boost::filesystem::path m_tempPath;

	//! Path to the cache directory
	static boost::filesystem::path m_cachePath;

	//! Path to working directory. The place where all PYXIS generated files are stored.
	static boost::filesystem::path m_workingPath;

	//! Path to the library directory
	static boost::filesystem::path m_libraryPath;

	//! guard against multi thread access.
	static boost::recursive_mutex m_mutex;

	//! A guard against multiple initialization.
	static bool m_bInitialized;

	static std::map<std::string,std::string> m_appConfiguration;
};

class PYXLIB_DECL AppServicesConfiguration
{
public:

	//! what type of local storage to use
	static const std::string localStorageFormat;
	static const std::string localStorageFormat_files;
	static const std::string localStorageFormat_sqlite;

	//! import memory limit (in MB)
	static const std::string importMemoryLimit;
};

/*!
This method will query the application properties file and determine if a
particular key exists within a scope.

\param strScope	The scope to search within.
\param strKey	The key to search for.

\return true if the value was found otherwise false.
*/
//! Determine if an application property exists.
PYXLIB_DECL bool appPropertyExists(	const std::string& strScope,
									const std::string& strKey	);

/*!
Global function template for getting an application property.  This call is 
thread safe during regular program execution but not during application
initialization or destruction.

\param strScope			The scope of the property (usually class name).
\param strKey			The key within the scope to retrieve a value for.
\param defaultValue		The templated default value. This value is returned if
						the properties file has not been defined yet.
\param strDescription	The descriptive entry to place in the properties file if
						a new value is being created.

\return The value returned from the properties file, this is the same type as the
		default value (templated).

\sa Properties::getProperty
\relates AppServices
*/
template<class T> 
const T getAppProperty(	
	const std::string& strScope,
	const std::string& strKey,
	const T& defaultValue,
	const std::string& strDescription = "No description available."	)
{
	// use default value if not initialized
	T outValue = defaultValue;

	if (0 != AppServices::m_spProps.get())
	{
		outValue = getProperty(	*(AppServices::m_spProps.get()), 
								strScope, 
								strKey, 
								strDescription,
								defaultValue	);
	}
	
	return outValue;
};

/*!
AppProperty - Helper class to use app properties.

usage:
1. create a static/memeber AppProperty<T> prop;
2. use it as a const T:  T value = prop (using the T operator())

example:

	AppProperty<int> cacheLimit("WorldView","CacheLimit",100,"the Cache limit.");
	int chaceSize;

	if (cacheSize > cacheLimit) //the cacheLimit will automaticly load and covert to int
	{
  		limitCache(cacheLimit);
	}
*/
template<class T>
class AppProperty
{
protected:
	std::string m_scope;
	std::string m_key;
	std::string m_description;
	mutable T m_value;
	bool m_loaded;

public:
	AppProperty(
		const std::string& strScope,
		const std::string& strKey,
		const T& defaultValue,
		const std::string& strDescription) :
			m_scope(strScope),
			m_key(strKey),
			m_value(defaultValue),
			m_description(strDescription),
			m_loaded(false)
	{
	}

	//! convertions to value type
	operator const T & () { 
		if (!m_loaded)
		{
			load();
		}
		return m_value;
	}

	//! get the scope
	const std::string & getScope() const { return m_scope; }
	
	//! get the key name
	const std::string & getKey() const { return m_key; }
	
	//! get description
	const std::string & getDescription() const { return m_description; }
	
	//! get the property value
	const T & getValue() const { return *this; }
	
protected:
	//! load the data from ini file
	void load()
	{
		m_value = getAppProperty(m_scope,m_key,m_value,m_description);
		m_loaded = true;
	}


};

//! Return a single boolean application property
PYXLIB_DECL bool getAppProperty(	
	const std::string& strScope,
	const std::string& strKey,
	const bool& defaultValue	);

//! Return a single long application property
PYXLIB_DECL unsigned long getAppProperty(	
	const std::string& strScope,
	const std::string& strKey,
	const unsigned long& defaultValue	);


//! helper class to around AppServices::getExecutionScope.
class PYXLIB_DECL AppExecutionScope
{
private:
	boost::recursive_mutex & m_mutex;

public:
	AppExecutionScope(const std::string & scope) : m_mutex(AppServices::getExecutionScope(scope))
	{
	}

	boost::recursive_mutex & getMutex() { return m_mutex; }
	operator boost::recursive_mutex & () { return m_mutex; }
};

#endif // guard
