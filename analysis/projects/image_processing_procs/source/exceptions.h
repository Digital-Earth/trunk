#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
/******************************************************************************
exceptions.h

begin      : 13/02/2008 2:58:22 PM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "module_image_processing_procs.h"
#include "pyxis/utility/exceptions.h"

//! Generic image processing exception.
class MODULE_IMAGE_PROCESSING_PROCS_DECL ImageProcessingException : public PYXException
{
public:
	//! Constructor
	ImageProcessingException(const std::string& strError) : 
	  PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const 
	{
		return "An image processing exception occurred";
	}
};

#endif