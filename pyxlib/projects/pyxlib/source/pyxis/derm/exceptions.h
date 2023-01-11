#ifndef PYXIS__DERM__EXCEPTIONS_H
#define PYXIS__DERM__EXCEPTIONS_H
/******************************************************************************
exceptions.h

begin		: 2005-11-01
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/exception.h"

// standard includes
#include <string>

/*
Include all the exceptions for the derm module in this file. Exceptions
should be sorted in alphabetical order for easier maintenance.
*/

//! Thrown for any derm error.
class PYXLIB_DECL PYXModelException : public PYXException
{
public:
	//! Constructor
	PYXModelException(const std::string& strError) :	
	  PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "A generic PYXIS error has occurred.";}
};

//! An coordinate conversion error.
class PYXLIB_DECL PYXCoordConversionException : public PYXModelException
{
public:
	//! Constructor
	PYXCoordConversionException(const std::string& strError) : 
	  PYXModelException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "A coordinate conversion error has occurred.";}	
};

//! An error specific to PYXIndex.
class PYXLIB_DECL PYXIndexException : public PYXModelException
{
public:
	//! Constructor
	PYXIndexException(const std::string& strError) : 
	  PYXModelException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "An indexing error has occurred.";}	
};

//! Thrown when a PYXIS math exception occurs
class PYXLIB_DECL PYXMathException : public PYXModelException
{
public:
	//! Constructor
	PYXMathException(const std::string& strError) : PYXModelException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "A mathematical error has occurred.";}	
};

//! Thrown when a Snyder projection exception occurs
class PYXLIB_DECL PYXSnyderException : public PYXModelException
{
public:
	//! Constructor
	PYXSnyderException(const std::string& strError) : PYXModelException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "A projection error has occurred.";}	
};

#endif // guard
