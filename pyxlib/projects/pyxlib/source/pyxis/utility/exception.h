#ifndef PYXIS__UTILITY__EXCEPTION_H
#define PYXIS__UTILITY__EXCEPTION_H
/******************************************************************************
exception.h

begin		: 2003-12-08
copyright	: derived from igo_exception.h (C) 2000 by iGO Technologies Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/trace.h"

// standard includes
#include <string>
#include <sstream>

/*
Use this macro when throwing a PYXException or derived exception. The macro
appends file and line number information to the error string. The error message
is traced to the log if tracing is enabled.

\param	TYPE		The type of exception to throw (i.e. PYXException)
\param	EXPRESSION	A streamable expression (i.e. "Unable to open file: " << strFileName)
*/
#define PYXTHROW(TYPE, EXPRESSION) \
	do { \
		if (PYXException::isTracing()) \
		{ \
			TRACE_ERROR(EXPRESSION); \
		} \
		std::ostringstream stream; \
		stream << __FILE__ << "(" << __LINE__ << ") : " << #TYPE << ": " << EXPRESSION << std::ends; \
		throw TYPE(stream.str()); \
	} while (false)

#define PYXTHROW_NOT_IMPLEMENTED() PYXTHROW(PYXException,__FUNCTION__  << " is not implemented.")

#define PYXTHROW_NOT_SUPPORTED() PYXTHROW(PYXException,__FUNCTION__  << " is not supported.")

#define PYXTHROW_NULL_ARGUMENT(ARG) PYXTHROW(PYXException,__FUNCTION__  << "error" << #ARG << " is null.")

#define VALIDATE_ARGUMENT_NOT_NULL(ARG) \
	assert(ARG); \
	if (!ARG) \
	{ \
		PYXTHROW_NULL_ARGUMENT(ARG); \
	} \

/*
Use this macro when rethrowing a PYXException or derived exception. The macro
appends file and line number information to the error string.  The error message
is traced to the log if tracing is enabled.

\param	CHILD		The exception that caused this exception.
\param	TYPE		The type of exception to rethrow (i.e. PYXException)
\param	EXPRESSION	A streamable expression (i.e. "Unable to open file: " << strFileName)
*/
#define PYXRETHROW(CHILD, TYPE, EXPRESSION) \
	do { \
		if (PYXException::isTracing()) \
		{ \
			TRACE_ERROR(EXPRESSION); \
		} \
		std::ostringstream stream; \
		stream << __FILE__ << "(" << __LINE__ << ") : " << #TYPE << ": " << EXPRESSION << std::endl; \
		stream << CHILD.getFullErrorString() << std::ends; \
		throw TYPE(stream.str()); \
	} while (false)

/*!
PYXException is a simple exception that contains an error string. Developers
should use the PYXTHROW and PYXRETHROW macros when throwing a PYXException or
derived exception.
*/
//! PYXIS PYXException class
class PYXLIB_DECL PYXException
{
public:

	/*!
	Constructor. There is no default constructor in order to ensure that
	exceptions always contain meaningful information. Since we only have
	one type of exception, an empty one would convey almost no meaning.
	*/
	PYXException(const std::string& strError) : 
		m_strError(strError) {}
					
	//! Destructor
	virtual ~PYXException() {;}
					
	//! Get a localized error string.
	/*!
	Get a string suitable for display to the user.

	\return	The string.
	*/
	virtual const std::string getLocalizedErrorString() const {return "A generic error has occurred.";}
	
	//! Get the full error string in English.
	/*!
	Get the full error string. This is the string formed by concatenation
	of all exception strings that led up to this exception.

	\return	The error string.
	*/
	const std::string& getFullErrorString() const {return m_strError;}

	//! Determine if exception tracing is enabled
	static bool isTracing() {return m_bTrace;}

	//! Disable exception tracing
	static void disableTrace();

	//! Enable exception tracing
	static void enableTrace();

protected:

private:

	//! A global setting to trace out exceptions strings.
	static bool m_bTrace;

	//! The error string
	std::string m_strError;
};

#define CATCH_AND_RETHROW(MESSAGE) \
	catch(PYXException & __pyx_exp__) \
	{ \
		PYXTHROW(PYXException,MESSAGE << " due to error : " << __pyx_exp__.getFullErrorString()); \
	} \
	catch(std::exception & __std_exp__) \
	{ \
		PYXTHROW(PYXException,MESSAGE << " due to error : " << __std_exp__.what()); \
	} \
	catch(...) \
	{ \
		PYXTHROW(PYXException,MESSAGE << " due to error : unknown" ); \
	} \

#endif // guard
