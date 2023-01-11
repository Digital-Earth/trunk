#ifndef PYXIS__UTILITY__EXCEPTIONS_H
#define PYXIS__UTILITY__EXCEPTIONS_H
/******************************************************************************
exceptions.h

begin		: 2005-10-20
copyright	: (C) 2005 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/exception.h"

/*
Include all the exceptions for the utilities module in this file. Exceptions
should be sorted in alphabetical order for easier maintenance.
*/
//! Thrown when command fails while doing some arbritary operation.
class PYXLIB_DECL PYXCommandException : public PYXException
{

public:

	//! Constructor
	PYXCommandException(const std::string& strError) : PYXException(strError) {;}

public:	

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const 
	{
		return "A command failure has occurred.";
	}
};
//! Thrown when command execution fails.
class PYXLIB_DECL PYXCommandExecuteException : public PYXCommandException
{

public:

	//! Constructor
	PYXCommandExecuteException(const std::string& strError) : PYXCommandException(strError) {;}

public:
	
	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const 
	{
		return "A command execution failure has occurred.";
	}
};

//! Thrown when a command undo fails.
class PYXLIB_DECL PYXCommandUndoException : public PYXCommandException
{

public:
	
	//! Constructor
	PYXCommandUndoException(const std::string& strError) : PYXCommandException(strError) {;}

public:	

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const 
	{
		return "A command undo failure has occurred.";
	}
};

//! Thrown when a command redo fails.
class PYXLIB_DECL PYXCommandRedoException : public PYXCommandException
{

public:

	//! Constructor
	PYXCommandRedoException(const std::string& strError) : PYXCommandException(strError) {;}

public:	

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const 
	{
		return "A command redo failure has occurred.";
	}
};

//! Thrown when a HTTP operation fails.
class PYXLIB_DECL PYXHttpException : public PYXException
{

public:

	//! Constructor
	PYXHttpException(const std::string& strError) : PYXException(strError) {;}

public:	

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const 
	{
		return "A HTTP communications error has occurred.";
	}
};

//! Thrown when a command fails.
class PYXLIB_DECL PYXThreadException : public PYXException
{
public:
	//! Constructor
	PYXThreadException(const std::string& strError) : PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "An error occurred in a thread.";}
};

//! Thrown when a directory operation fails.
class PYXLIB_DECL PYXDirException : public PYXException
{
public:
	//! Constructor
	PYXDirException(const std::string& strError) : PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "A directory error has occurred.";}
};

//! Thrown when a file operation fails.
class PYXLIB_DECL PYXFileException : public PYXException
{
public:
	//! Constructor
	PYXFileException(const std::string& strError) : PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "A file error has occurred.";}
};

//! Thrown when a lock operation fails.
class PYXLIB_DECL PYXLockException : public PYXException
{
public:
	//! Constructor
	PYXLockException(const std::string& strError) : PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "A synchronization error has occurred.";}
};

//! Thrown when a serialization operation fails.
class PYXLIB_DECL PYXSerializableException : public PYXException
{
public:
	//! Constructor
	PYXSerializableException(const std::string& strError) : PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "A serialization error has occurred.";}
};

//! Thrown when an attempt to access a PYXValueColumn entry fails.
class PYXLIB_DECL PYXValueColumnException : public PYXException
{
public:
	//! Constructor
	PYXValueColumnException(const std::string& strError) : PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "A PYXValueColumn access failed.";}
};

//! Thrown when an attempt to access a PYXValueTable entry fails.
class PYXLIB_DECL PYXValueTableException : public PYXException
{
public:
	//! Constructor
	PYXValueTableException(const std::string& strError) : PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "A PYXValueTable access failed.";}
};

#endif // guard
