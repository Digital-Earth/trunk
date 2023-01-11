#ifndef PYXIS__UTILITY__PROPERTIES_H
#define PYXIS__UTILITY__PROPERTIES_H
/******************************************************************************
properties.h

begin		: 2004-10-05
copyright	: derived from properties.h (C) 2000 by iGO Technologies Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/trace.h"

// boost includes
#include <boost/thread/recursive_mutex.hpp>

// standard includes
#include <map>
	
/*!
This class is loosely modelled from the Java Properties class. It manages a
file of key, value pairs that provide application configuration information.

In the file, blank lines and lines beginning with # are ignored. The syntax
of a key value pair is:

key=value

By convention, the key has the format Scope.SubKey where scope is the name of
the class that uses the value and subkey is a unique identifier for the
property. Leading and trailing spaces are removed from both key and value.

An example properties file looks like the following:

# Properties file 'FluoroGuide.properties' created: Mon Feb 21 12:56:34 2000

# OpticalTracker.Type: Description for property  
OpticalTracker.type=Polaris

# OpticalTracker.BaudRate: Description for property  
OpticalTracker.baudRate=57600

# OpticalTracker.Parity: Description for property 
OpticalTracker.parity=E

# New properties added: Mon Feb 21 16:21:02 2000

# DRB.ID: Description for property  
DRB.ID=8700223

# DRB.IsActive: Description for property 
DRB.IsActive=true

# DRB.IsDynamic: Description for property  
DRB.IsDynamic=false

# Drill.ID: Description for property  
Drill.ID=8700247

# Drill.IsActive: Description for property  
Drill.IsActive=false

# Drill.IsDynamic: Description for property  
Drill.IsDynamic=true

*/
//! Manages a set of serialized variables used by the application.
class PYXLIB_DECL Properties
{
public:

	//! The default description value
	static const std::string kstrDefaultDescription;

	//! Construct a properties object from a file, create the file if not present.
	explicit Properties(const std::string& strName);
	
	//! Destructor, write default values for any new properties to the file.
	virtual ~Properties();
	
	//! Get a property value, return the default if none found.
	const std::string getProperty(	const std::string& strScope,
									const std::string& strKey,
									const std::string& strDescription,
									const std::string& strDefaultValue	);

	//! Check if a property exists.
	bool propertyExists(	const std::string& strScope,
							const std::string& strKey	) const;											

	//! Make a property key value from a scope and key.
	const std::string makeKey(	const std::string& strScope,
								const std::string& strKey	) const;

protected:

private:

	//! The extension used for properties files
	static const std::string kstrFileExtension;

	//! The fully qualified name of the properties file
	std::string m_strFileName;
	
	//! A properties map
	typedef std::map<std::string, std::string> PropertiesMap;
	
	//! The key value pairs read from the file
	PropertiesMap m_mapFileProps;
	
	//! The key value pairs not found in the file
	PropertiesMap m_mapDefaultProps;

	//! The keys and descriptions not fount in the file
	PropertiesMap m_mapDefaultDescs;

 	//! Object to facilitate thread safe code.
	mutable boost::recursive_mutex m_mutex;
};

/*!
Get a property value, return the default if none found. Default properties are
added to the properties file when this object is destructed.

\param props			The properties object.
\param strScope			The scope of the key, typically the class name.
\param strKey			The key, a unique identifier within scope that 
						identifies the value.
\param strDescription	A 1 line description of the purpose of the property.
\param defaultValue		The default value to be returned if none is found.

\return	The value corresponding to the given scope and key.
*/
template<class T>
const T getProperty(	Properties& props,
									const std::string& strScope,
									const std::string& strKey,
									const std::string& strDescription,
									const T& defaultValue	)
{
	// convert default value to a string, << trims whitespace
	std::ostringstream strDefaultStream;
	strDefaultStream << defaultValue;
	
	// call specialized form of this template for strings
	const std::string strValue = props.getProperty(	strScope,
													strKey,
													strDescription,
													strDefaultStream.str()	);
	
	// convert the value from a string
	T outValue;
	std::istringstream strInStream(strValue);

	strInStream >> outValue;
	
	if (!strInStream.eof())
	{
		std::string strFullKey = props.makeKey(strScope, strKey);
		
		// conversion failed, use the default value
		TRACE_ERROR(	"Conversion failed, returning default: key = " +
						strFullKey +
						" default value = " +
						strDefaultStream.str()	);
		outValue = defaultValue;
	}
	
	return outValue;
}

//! Specialization for QString of getProperty() template method.
template<>
PYXLIB_DECL const std::string getProperty(	Properties& props,
											const std::string& strScope,
											const std::string& strKey,
											const std::string& strDescription,
											const std::string& strDefaultValue	);

//! Specialization for bool of getProperty() template method.
template<> 
PYXLIB_DECL const bool getProperty(	Properties& props,
									const std::string& strScope,
									const std::string& strKey,
									const std::string& strDescription,
									const bool& bDefaultValue	);

#endif // guard
