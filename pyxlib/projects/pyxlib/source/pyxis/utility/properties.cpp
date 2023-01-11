/******************************************************************************
properties.cpp - Manages a properties file

begin		: 2004-10-05
copyright	: derived from properties.cpp (C) 2000 by iGO Technologies Inc.
web			: www.igotechnologies.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h" 
#include "pyxis/utility/properties.h"

// pyxlib includes
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/trace.h"

// standard includes
#include <fstream>

// file extension for properties files
const std::string Properties::kstrFileExtension = ".properties";

// the start of a description string
const std::string kstrComment = "# ";

/*!
Open the specified properties file. ".properties" is appended to the name.
If the file is not present, it is be created when the properties object is
destroyed. The application will append the default values for any properties
requested but not found, to the properties file.

\param strName	The name to assign to the properties file. Any spaces in the 
				file name are replaced with underscore ('_') characters.
*/
Properties::Properties(const std::string& strName) :
	m_mapFileProps(),
	m_mapDefaultProps(),
	m_mapDefaultDescs()
{
	assert(!strName.empty());

	// remove spaces from the proposed filename
	std::string strNewName = strName;
	std::replace(strNewName.begin(), strNewName.end(), ' ', '_');
	boost::filesystem::path propertyPath =  
		AppServices::getWorkingPath() /
		(strNewName + kstrFileExtension);

	m_strFileName = FileUtils::pathToString(propertyPath);
	std::ifstream in;
	in.open(m_strFileName.c_str(), std::ios::in);
	
	// open the file
	if (in.good())
	{
		// for each line in the file
		std::string strLine;
		do
		{
			// read the next line
			std::getline(in, strLine);

			// ignore empty lines and comment lines
			if ((strLine.length() > 0) && (*(strLine.begin()) != '#'))
			{
				// extract the key and value
				size_t nOffset = strLine.find('=');
				if (0 > nOffset)
				{
					// can't find separator, ignore the line
					// can't use Trace because the trace level is not set yet
				}
				else
				{
					// add key and value to our map
					std::string strKey = StringUtils::trim(strLine.substr(0, nOffset));

					nOffset++;
					size_t nLength = strLine.length() - nOffset;
					std::string strValue = StringUtils::trim(strLine.substr(nOffset, nLength));
						
					m_mapFileProps[strKey] = strValue;
				}
			}

		} while (!in.eof());

		// close the file
		in.close();
	}
	else
	{
		// unable to open properties file, probably because it doesn't exist
		// default values for properties will be returned
		// the properties file will be created in the destructor
		TRACE_INFO("Properties file does not exist, using default.");
	}
}

/*!
The destructor creates the properties file if not present and appends default
values for any properties requested but not found, to the file.
*/	
Properties::~Properties()
{
	// if we have some default properties, append them to the properties file
	if (m_mapDefaultProps.size() > 0)
	{
		boost::filesystem::path propertiesPath = FileUtils::stringToPath(m_strFileName);
		bool bExists = FileUtils::exists(propertiesPath);
		
		// open the properties file for appending, create if it doesn't exist
		std::ofstream out;
		out.open(m_strFileName.c_str(), std::ios::out | std::ios::app);
		if (out.good())
		{
			

			// add header info if the file is new
			if (!bExists)
			{
				out	<< "# Properties file '"
					<< m_strFileName
					<< "' created: "
					<< StringUtils::now()
					<< std::endl;
			}

			// append new properties to the file
			out << std::endl;
			out	<< "# New properties added: "
				<< StringUtils::now()
				<< std::endl;
				
			PropertiesMap::const_iterator itProps = m_mapDefaultProps.begin();
			PropertiesMap::const_iterator itDescs = m_mapDefaultDescs.begin();
			
			for (; itProps != m_mapDefaultProps.end(); itProps++)
			{
				// output the description and the property
				out << std::endl;
				out << kstrComment << itDescs->first << ": " << itDescs->second << std::endl;
				out << itProps->first << "=" << itProps->second << std::endl;

				// increment the description iterator
				itDescs++;
			}

			// close the file
			out.flush();
			out.close();
		}
		else
		{
			TRACE_ERROR(	"Failed to write properties file '" << 
							m_strFileName << "'."	);
		}
	}
}

/*!
Get a property value, return the default if none found. Default properties are
added to the properties file when this object is destructed.

\param strScope			The scope of the key, typically the class name.
\param strKey			The key, a unique identifier within scope that identifies the value.
\param strDefaultValue	The default value to be returned if none is found.
\param strDescription	A one line description of uses or options for the property. 

\return	The value corresponding to the given scope and key.
*/
const std::string Properties::getProperty(	const std::string& strScope,
											const std::string& strKey,
											const std::string& strDescription,
											const std::string& strDefaultValue	)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	// make the key
	const std::string strRealKey = makeKey(strScope, strKey);
	
	// check the properties we loaded from file
	PropertiesMap::const_iterator itProps = m_mapFileProps.find(strRealKey);
	if (itProps == m_mapFileProps.end())
	{
		// check properties added this session
		itProps = m_mapDefaultProps.find(strRealKey);
		if (itProps == m_mapDefaultProps.end())
		{
			// property was not found, add it to our list of default values
			m_mapDefaultProps[strRealKey] = strDefaultValue;
			m_mapDefaultDescs[strRealKey] = strDescription;
			itProps = m_mapDefaultProps.find(strRealKey);
		}
	}
	
	return itProps->second;
}

/*!
Check if a property exists.

\param strScope			The scope of the key, typically the class name.
\param strKey			The key, a unique identifier within scope that identifies the value.

\return	true if the property exists, otherwise false.
*/
bool Properties::propertyExists(	const std::string& strScope,
									const std::string& strKey	) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	bool bExists = true;
	
	// make the key
	const std::string strRealKey = makeKey(strScope, strKey);
	
	// check the properties we loaded from file
	PropertiesMap::const_iterator itProps = m_mapFileProps.find(strRealKey);
	if (itProps == m_mapFileProps.end())
	{
		// check properties added this session
		itProps = m_mapDefaultProps.find(strRealKey);
		if (itProps == m_mapDefaultProps.end())
		{
			// property was not found
			bExists = false;
		}
	}
	
	return bExists;
}
										
/*!
Make a property key value from a scope and key.

The scope and key should be CapitalizedLikeThis, and include the units if
appropriate (e.g. SpecialAngleDegrees vs. SpecialAngleRadians).

\param strScope	The scope of the key, typically the class name.
\param strKey	The key, a unique identifier within scope that identifies the value.

\return	The key for the properties maps.
*/
const std::string Properties::makeKey(	const std::string& strScope,
										const std::string& strKey	) const
{
	/*
	Ensure scope and key adhere to naming standards. Both the scope and key
	should use class naming conventions ie. no spaces, each word capitalized.
	For example MyClass.MyPropertyName. This code checks for spaces in the name
	which is the only check we can perform here.
	*/
	assert(strScope.find(' ') == std::string::npos && "No spaces allowed in properties scope.");
	assert(strKey.find(' ') == std::string::npos && "No spaces allowed in properties key.");

	return	StringUtils::trim(strScope) +
			"." +
			StringUtils::trim(strKey);
}

/*!
Get a property value, return the default if none found. Default properties are
added to the properties file when this object is destructed. Specialized for
strings.

\param props			The properties object.
\param strScope			The scope of the key, typically the class name.
\param strKey			The key, a unique identifier within scope that identifies the value.
\param strDescription	A one line description of uses or options for the property. 
\param strDefaultValue	The default value to be returned if none is found.

\return	The value corresponding to the given scope and key.
*/
template<> const std::string getProperty(	Properties& props,
											const std::string& strScope,
											const std::string& strKey,
											const std::string& strDescription,
											const std::string& strDefaultValue	)
{
	return props.getProperty(strScope, strKey, strDescription, strDefaultValue);
}

/*!
Get a property value, return the default if none found. Default properties are
added to the properties file when this object is destructed. Specialized for
bool.

\param props			The properties object.
\param strScope			The scope of the key, typically the class name.
\param strKey			The key, a unique identifier within scope that identifies the value.
\param strDescription	A one line description of uses or options for the property. 
\param bDefaultValue	The default value to be returned if none is found.

\return	The value corresponding to the given scope and key.
*/
template<> const bool getProperty(	Properties& props,
									const std::string& strScope,
									const std::string& strKey,
									const std::string& strDescription,
									const bool& bDefaultValue	)
{
	std::string strValue = props.getProperty(	strScope, 
												strKey, 
												strDescription,
												bDefaultValue ? "1" : "0"	);
	return (strValue == "1");
}
