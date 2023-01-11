#ifndef PYXIS__GEOMETRY__EXCEPTIONS_H
#define PYXIS__GEOMETRY__EXCEPTIONS_H
/******************************************************************************
exceptions.h

begin		: 2005-11-01
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/exceptions.h"

// standard includes
#include <string>

//! PYXGeometry exception.
class PYXLIB_DECL PYXGeometryException : public PYXModelException
{
public:
	//! Constructor
	PYXGeometryException(const std::string& strError) : 
	  PYXModelException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "A geometry error has occurred.";}	
};


class PYXLIB_DECL CircularIntersectionException : public PYXGeometryException
{
public:
	
	//! Constructor
	CircularIntersectionException(const std::string& strError) : PYXGeometryException(strError){;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "An error occured in CircleIntersectionTest";}
};

#endif // guard
