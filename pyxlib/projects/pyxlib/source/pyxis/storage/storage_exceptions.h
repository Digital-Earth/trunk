#ifndef PYXIS__STORAGE__EXCEPTIONS_H
#define PYXIS__STORAGE__EXCEPTIONS_H
/******************************************************************************
storage_exceptions.h

begin		: 2016-03-10
copyright	: (C) 2016 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/exceptions.h"

// standard includes
#include <string>

//! PYXGeometry exception.
class PYXLIB_DECL PYXStorageException : public PYXException
{
public:
	//! Constructor
	PYXStorageException(const std::string& strError) : 
	  PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "A storage error has occurred.";}	
};

#endif // guard
