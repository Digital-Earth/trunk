#ifndef LIBRARY_EXCEPTIONS_H
#define LIBRARY_EXCEPTIONS_H
/******************************************************************************
exceptions.h

begin      : 08/03/2007 5:17:08 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "library_config.h"

// pyxlib includes
#include "pyxis/utility/exception.h"

//! Base class for library exceptions.
class LIBRARY_DECL LibraryException : public PYXException
{
public:
	//! Constructor
	LibraryException(const std::string& strError) : PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "A library exception occurred.";}
};

//! Thrown when the library fails to open.
class LIBRARY_DECL LibraryOpenException : public LibraryException
{
public:
	//! Constructor
	LibraryOpenException(const std::string& strError) : LibraryException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "The library failed to open.";}
};

//! Thrown when a library operation fails.
class LIBRARY_DECL LibraryOperationException : public LibraryException
{
public:
	//! Constructor
	LibraryOperationException(const std::string& strError) : LibraryException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "A library operation failed.";}
};


//! Thrown when PYXCOM cannot find any Pipeline builders to build Pipelines with.
class LIBRARY_DECL NoPipeBuildersFoundException : public LibraryException
{
public:
	
	//! Constructor
	NoPipeBuildersFoundException(const std::string& strError) : LibraryException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "PYXCOM did not find any Pipeline builders in the system.";}
};

//! Thrown when PYXCOM fails to instantiate a class id of an object that implements IPipelineBuilder
class LIBRARY_DECL PipeBuilderInstantiationFailedException : public LibraryException
{
public:
	
	//! Constructor
	PipeBuilderInstantiationFailedException(const std::string& strError) : LibraryException(strError) {;}
	
	//! Get localized error string.
	virtual const std::string getLocalizedErrorString() const {return "Failed to insantiate Pipeline Builder";}
};

class LIBRARY_DECL FailedToCreatePipelineException : public LibraryException
{
public:

	//! Constructor
	FailedToCreatePipelineException(const std::string& strError) : LibraryException(strError){;}

	//! Get localized error string.
	virtual const std::string getLocalizedErrorString() const {return "Failed to bulid a pipeline";}
};

class LIBRARY_DECL ProcessAlreadyExistsException : public LibraryException
{
public:

	//! Constructor
	ProcessAlreadyExistsException(const std::string& strError) : LibraryException(strError){;}

	//! Get localized error string.
	virtual const std::string getLocalizedErrorString() const {return "Process already exists in library";}
};

#endif