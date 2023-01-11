#ifndef OWS_REFERENCE_INTERFACE_H
#define OWS_REFERENCE_INTERFACE_H

/******************************************************************************
ows_reference.h

begin      : 9/01/2011 9:57:18 AM
copyright  : (c) 2011 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "coord_converter_impl.h"
#include "gdal_metadata.h"
#include "gdal_xy_coverage.h"
#include "module_gdal.h"
#include "pyx_rtree.h"

// pyxlib includes
#include "pyxis/derm/horizontal_datum.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/path.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_iterator.h"


class MODULE_GDAL_DECL OWSFormat : public PYXObject
{
public:
	static PYXPointer<OWSFormat> create(const std::string & mimeType,const std::string & schema = "", const std::string & encoding = "")
	{
		return PYXNEW(OWSFormat,mimeType,schema,encoding);
	}

	static PYXPointer<OWSFormat> createFromWellKnownMimeType(const std::string & mimeType);

	OWSFormat(const std::string & mimeType,const std::string & schema = "", const std::string & encoding = "")
		: m_mimeType(mimeType), m_schema(schema), m_encoding(encoding)
	{
	}

	virtual ~OWSFormat()
	{
	}

public:
	bool operator == (const OWSFormat & other) const
	{
		return m_mimeType == other.m_mimeType && m_schema == other.m_schema && m_encoding == other.m_encoding;
	}

	bool operator != (const OWSFormat & other) const
	{
		return ! (*this == other);
	}

	const std::string & getMimeType() const { return m_mimeType; }
	const std::string & getSchema() const { return m_schema; }
	const std::string & getEncoding() const { return m_encoding; }

	PYXPointer<OWSFormat> clone() const
	{
		return create(m_mimeType,m_schema,m_encoding);
	}

public:
	bool supportMimeType(const std::string & mimeType) const; //do case-insensetive comparission
	bool supportSchema(const std::string & schema) const; //do case-insensetive comparission
	bool supportMimeTypeAndSchema(const std::string & mimeType,const std::string & schema) const; //do case-insensetive comparission

private:
	std::string m_mimeType;
	std::string m_schema;
	std::string m_encoding;
};

//! IOWSReference - interface to allow a process to expose an OWS reference. Used by WMS, WFS, WCS to expose their outputs to WPS processes
struct MODULE_GDAL_DECL IOWSReference : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:
	enum ReferenceType
	{
		WpsReference,
		OwsContextReference
	};

public:
	virtual PYXPointer<OWSFormat> getDefaultOutputFormat() const = 0;
	virtual bool supportOutputType(const OWSFormat & format) const = 0;
	virtual std::string getOWSReference(ReferenceType referenceType, const OWSFormat & format) const = 0;
};

//! IWMSReference - interface to allow a process to expose itself as a WMS reference
struct MODULE_GDAL_DECL IWMSReference : public IOWSReference
{
	PYXCOM_DECLARE_INTERFACE();
};

//! IWFSReference - interface to allow a process to expose itself as a WFS reference
struct MODULE_GDAL_DECL IWFSReference : public IOWSReference
{
	PYXCOM_DECLARE_INTERFACE();
};

//! IWCSReference - interface to allow a process to expose itself as a WCS reference
struct MODULE_GDAL_DECL IWCSReference : public IOWSReference
{
	PYXCOM_DECLARE_INTERFACE();
};

#endif