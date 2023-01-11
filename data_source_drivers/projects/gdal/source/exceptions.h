#ifndef GDAL__EXCEPTIONS_H
#define GDAL__EXCEPTIONS_H
/******************************************************************************
exceptions.h

begin      : 28/11/2007 9:53:07 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "module_gdal.h"
#include "pyxis/data/exceptions.h"
#include "pyxis/pipe/process_init_error.h"
#include "pyxis/procs/exceptions.h"

// standard includes
#include <string>

class MODULE_GDAL_DECL GDALProcessException : public PYXDataSourceException
{
public:
	//! Constructor
	GDALProcessException(const std::string& strError) : 
	  PYXDataSourceException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "An error occurred with a GDAL process.";}	
};

class MODULE_GDAL_DECL GDALOGRException : public PYXDataSourceException
{
public:
	//! Constructor
	GDALOGRException(const std::string& strError) : 
	  PYXDataSourceException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "An OGR error occurred.";}
};

class MODULE_GDAL_DECL GDALFileConvertException : public GDALProcessException
{
public:
	//! Constructor
	GDALFileConvertException(const std::string& strError) : 
	  GDALProcessException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "An error occured in the GDALFileConverterProcess.";}	
};

class MODULE_GDAL_DECL MissingGeometryException : public PYXDataSourceException
{
public:
	//! Constructor
	MissingGeometryException(const std::string& strError) : 
	  PYXDataSourceException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "The data set has a missing or empty geometry.";}	
};

//! An error that indicates process could not be successfully initialized due to a missing or empty geometry.
class MODULE_GDAL_DECL MissingGeometryInitError : public ProcInitErrorBase
{
	PYXCOM_DECLARE_CLASS();

public:
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcessInitError)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();
	GET_ERRORID_IMPL(MissingGeometryInitError)

	MissingGeometryInitError()
	{
		m_strError = "The data set has a missing or empty geometry.";
	}
};

class MODULE_GDAL_DECL MissingWorldFileException : public PYXDataSourceException
{
public:
	//! Constructor
	MissingWorldFileException(const std::string& strError) : 
	  PYXDataSourceException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "The data set is missing a world file (file with wld extension or extension ending in 'w').";}	
};

//! An error that indicates process could not be successfully initialized due to a missing or invalid world file.
class MODULE_GDAL_DECL MissingWorldFileInitError : public ProcInitErrorBase
{
	PYXCOM_DECLARE_CLASS();

public:
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcessInitError)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();
	GET_ERRORID_IMPL(MissingWorldFileInitError)

	MissingWorldFileInitError()
	{
		m_strError = "The data set is missing a geotransform. Possibly missing a world file.";
	}
};

class MODULE_GDAL_DECL MissingSRSException : public PYXDataSourceException
{
public:
	//! Constructor
	MissingSRSException(const std::string& strError) :
	  PYXDataSourceException(strError) {;}

	  //! Get a localized error string.
	  virtual const std::string getLocalizedErrorString() const {return "The data set is missing a spatial reference system.";}
};
	
//! An error that indicates process could not be successfully initialized due to missing or invalid SRS.
class MODULE_GDAL_DECL GDALSRSInitError : public ProcInitErrorBase
{
	PYXCOM_DECLARE_CLASS();

public:
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcessInitError)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();
	GET_ERRORID_IMPL(GDALSRSInitError)

	GDALSRSInitError()
	{
		m_strError = "Missing or invalid SRS";
	}
};

class MODULE_GDAL_DECL MissingUserCredentialsException : public PYXDataSourceException
{
public:
	//! Constructor
	MissingUserCredentialsException (const std::string& strError) :
	  PYXDataSourceException(strError) {;}

	//! Get a localized error string.
	virtual const std::string getLocalizedErrorString() const {return "The data set requires valid user credentials.";}
};

#endif
