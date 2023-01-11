#ifndef EXCEL__EXCEPTIONS_H
#define EXCEL__EXCEPTIONS_H
/******************************************************************************
exceptions.h

begin      : 12/11/2007 4:20:24 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "excel.h"
#include "pyxis/utility/exception.h"

// standard includes
#include <string>

//! Thrown for xls process error
class EXCEL_DECL ExcelException : public PYXException
{
public:
	//! Constructor
	ExcelException(const std::string& strError) : 
	  PYXException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "An error occurred with a PYXIS Excel (Excel) process.";}	
};

class EXCEL_DECL LoadFailException : public ExcelException
{
public:
	//! Constructor
	LoadFailException(const std::string& strError) : 
	  ExcelException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "Can't load the specified excel file.";}	
};

class EXCEL_DECL AttributeException : public ExcelException
{
public:
	//! Constructor
	AttributeException(const std::string& strError) : 
	  ExcelException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "Error processing attribute data.";}	
};

class EXCEL_DECL DefinitionException : public ExcelException
{
public:
	//! Constructor
	DefinitionException(const std::string& strError) : 
	  ExcelException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "Error with the feild definitions for the process.";}	
};

class EXCEL_DECL DataException : public ExcelException
{
public:
	//! Constructor
	DataException(const std::string& strError) : 
	  ExcelException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "An xls data error occurred.";}	
};

class EXCEL_DECL FeatureException : public ExcelException
{
public:
	//! Constructor
	FeatureException(const std::string& strError) : 
	  ExcelException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "An xls feature error occurred.";}	
};

#endif
