#ifndef COVERAGES__EXCEPTIONS_H
#define COVERAGES__EXCEPTIONS_H
/******************************************************************************
exceptions.h

begin		: 2005-10-20
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "module_pyxis_coverages.h"
#include "pyxis/utility/exception.h"

// standard includes
#include <string>

/*
Include all the exceptions for the Pyxis Coverages module in this file. Exceptions
should be sorted in alphabetical order for easier maintenance.
*/

//! Thrown for pyxis coverage process error
class MODULE_PYXIS_COVERAGES_DECL PYXCoveragesException : public PYXException
{
public:
	//! Constructor
	PYXCoveragesException(const std::string& strError) : 
	  PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "An error occurred with a PYXIS coverage process.";}	
};

//! An exception specific to null coverages
class MODULE_PYXIS_COVERAGES_DECL NullCoverageException : public PYXCoveragesException
{
public:
	//! Constructor
	NullCoverageException(const std::string& strError) : 
	  PYXCoveragesException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "An error occurred in a PYXIS Null Coverage.";}	
};

//! An exception specific to checker coverages
class MODULE_PYXIS_COVERAGES_DECL CheckerCoverageException : public PYXCoveragesException
{
public:
	//! Constructor
	CheckerCoverageException(const std::string& strError) : 
	  PYXCoveragesException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "An error occurred in a PYXIS Checkerboard Coverage.";}	
};

/*! An exception to throw for coverages that want to specify that a 
PYXValueTile is currently unavailable but should be available later.
*/
class MODULE_PYXIS_COVERAGES_DECL TileUnavailableException : public PYXCoveragesException
{
public:
	//! Constructor
	TileUnavailableException(const std::string& strError) : 
	  PYXCoveragesException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const 
	{
		return "The tile you have requested for is currently unavailable but should be available later.";
	}	
};

#endif
