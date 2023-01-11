#ifndef APPLICATION_EXCEPTIONS_H
#define APPLICATION_EXCEPTIONS_H
/******************************************************************************
exceptions.h

begin      : 3/19/2007 3:39:29 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "pyxis/utility/exception.h"


class PYXApplicationException : public PYXException
{
	public:
	//! Constructor
	PYXApplicationException(const std::string& strError) : 
	  PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "A application process exception occured.";}
};

//! Thrown errors related to visualizing symbols.
class DocumentException : public PYXApplicationException
{
public:
	//! Constructor
	DocumentException(const std::string& strError) : 
	  PYXApplicationException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "An error occurred in the document.";}	
};

#endif
