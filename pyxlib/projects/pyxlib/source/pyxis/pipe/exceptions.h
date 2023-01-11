#ifndef PYXIS__PIPE__EXCEPTIONS_H
#define PYXIS__PIPE__EXCEPTIONS_H
/******************************************************************************
exceptions.h

begin      : 30/11/2007 11:25:42 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxis/procs/exceptions.h"

// standard includes
#include <string>

class PYXLIB_DECL ProcessListException : public PYXException
{
public:
	//! Constructor
	ProcessListException(const std::string& strError) : 
	  PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const 
	{
		return "An error occurred while manipulating the process list.";
	}	
};

#endif
