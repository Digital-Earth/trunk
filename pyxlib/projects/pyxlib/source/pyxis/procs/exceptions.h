#ifndef PROCS__EXCEPTIONS_H
#define PROCS__EXCEPTIONS_H
/******************************************************************************
exceptions.h

begin		: 2007-04-27
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxis/utility/exception.h"

// standard includes
#include <string>

/*
Include all the exceptions for the Procs module in this file. Exceptions
should be sorted in alphabetical order for easier maintenance.
*/

//! Thrown for pyxis process error
class PYXLIB_DECL PYXProcsException : public PYXException
{
public:
	//! Constructor
	PYXProcsException(const std::string& strError) : 
	  PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "An error occurred with a PYXIS process.";}	
};

//! An exception specific to constant coverages
class PYXLIB_DECL ConstCoverageException : public PYXProcsException
{
public:
	//! Constructor
	ConstCoverageException(const std::string& strError) : 
	  PYXProcsException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "An error occurred in a PYXIS Constant Coverage.";}	
};

#endif
