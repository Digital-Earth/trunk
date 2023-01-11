#ifndef PYXIS__DATA__EXCEPTIONS_H
#define PYXIS__DATA__EXCEPTIONS_H
/******************************************************************************
exceptions.h

begin		: 2005-11-01
copyright	: (C) 2005 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/exception.h"

/*
Include all the exceptions for the database module in this file. Exceptions
should be sorted in alphabetical order for easier maintenance.
*/

//! Thrown when a data source error occurs.
class PYXLIB_DECL PYXDataException : public PYXException
{
public:
	//! Constructor
	PYXDataException(const std::string& strError) : PYXException(strError) {}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const
	{
		return "A generic data error has occurred.";
	}
};

//! Thrown when a data source error occurs.
class PYXLIB_DECL PYXDataSourceException : public PYXDataException
{
public:
	//! Constructor
	PYXDataSourceException(const std::string& strError) :
		PYXDataException(strError) {}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const
	{
		return "A data source error has occurred.";
	}
};

//! Thrown when a data item error occurs.
class PYXLIB_DECL PYXDataItemException : public PYXDataException
{
public:
	//! Constructor
	PYXDataItemException(const std::string& strError) :
		PYXDataException(strError) {}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const
	{
		return "A data item error has occurred.";
	}
};

//! Thrown when a data tile error occurs.
class PYXLIB_DECL PYXDataTileException : public PYXDataException
{
public:
	//! Constructor
	PYXDataTileException(const std::string& strError) :
		PYXDataException(strError) {}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const
	{
		return "A data tile error has occurred.";
	}
};

//! Thrown when a data tile error occurs.
class PYXLIB_DECL PYXTileCacheException : public PYXDataException
{
public:
	//! Constructor
	PYXTileCacheException(const std::string& strError) :
		PYXDataException(strError) {}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const
	{
		return "A tile cache error has occurred.";
	}
};

//! Thrown for errors with mapping feature attributes to visual elements.
class PYXLIB_DECL PYXStyleMappingException : public PYXDataException
{
public:
	//! Constructor
	PYXStyleMappingException(const std::string& strError) :
		PYXDataException(strError) {}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const
	{
		return "A error occurred while mapping data attributes to style.";
	}
};

//! Thrown when an attempt to access a PYXValueTile entry fails.
class PYXLIB_DECL PYXValueTileException : public PYXException
{
public:
	//! Constructor
	PYXValueTileException(const std::string& strError) :
		PYXException(strError) {}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const
	{
		return "Unable to access PYXIS data.";
	}
};

//! Thrown when an error occurs in a table or field definition.
class PYXLIB_DECL PYXDefinitionException : public PYXException
{
public:
	//! Constructor
	PYXDefinitionException(const std::string& strError) :
		PYXException(strError) {}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const
	{
		return "A PYXIS definition error occurred.";
	}
};

#endif // guard
