/******************************************************************************
ows_reference.cpp

begin      : 9/01/2011 9:57:18 AM
copyright  : (c) 2011 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_GDAL_SOURCE

// local includes
#include "ows_reference.h"


#include <boost/algorithm/string.hpp>  

#include <map>

// {CFB38CB7-586F-4895-BAA9-D2E1EBA810B3}
PYXCOM_DEFINE_IID(IOWSReference, 
0xcfb38cb7, 0x586f, 0x4895, 0xba, 0xa9, 0xd2, 0xe1, 0xeb, 0xa8, 0x10, 0xb3);

// {5AF71F31-995F-4ceb-BB54-79DE33E6C261}
PYXCOM_DEFINE_IID(IWFSReference, 
0x5af71f31, 0x995f, 0x4ceb, 0xbb, 0x54, 0x79, 0xde, 0x33, 0xe6, 0xc2, 0x61);

// {67D468B4-88FA-48cb-9889-138DAAA0B3CE}
PYXCOM_DEFINE_IID(IWMSReference, 
0x67d468b4, 0x88fa, 0x48cb, 0x98, 0x89, 0x13, 0x8d, 0xaa, 0xa0, 0xb3, 0xce);

// {148E850B-A3FD-4fb6-B01E-244236762FDB}
PYXCOM_DEFINE_IID(IWCSReference, 
0x148e850b, 0xa3fd, 0x4fb6, 0xb0, 0x1e, 0x24, 0x42, 0x36, 0x76, 0x2f, 0xdb);



bool OWSFormat::supportMimeType(const std::string & mimeType) const
{
	return boost::iequals(m_mimeType,mimeType);
}

bool OWSFormat::supportSchema(const std::string & schema) const
{
	return boost::iequals(m_schema,schema);
}

bool OWSFormat::supportMimeTypeAndSchema(const std::string & mimeType,const std::string & schema) const
{
	return boost::iequals(m_mimeType,mimeType) && boost::iequals(m_schema,schema);
}

std::list<PYXPointer<OWSFormat>> & getWellKnownFormats()
{
	static std::list<PYXPointer<OWSFormat>> wellKnownFormats;

	if (wellKnownFormats.size()==0)
	{
		wellKnownFormats.push_back(OWSFormat::create("text/xml; subtype=gml/2.1.2","http://schemas.opengis.net/gml/2.1.2/feature.xsd"));
		wellKnownFormats.push_back(OWSFormat::create("text/xml; subtype=gml/3.0.0","http://schemas.opengis.net/gml/3.0.0/base/feature.xsd"));
		wellKnownFormats.push_back(OWSFormat::create("text/xml; subtype=gml/3.0.1","http://schemas.opengis.net/gml/3.0.1/base/feature.xsd"));
		wellKnownFormats.push_back(OWSFormat::create("text/xml; subtype=gml/3.1","http://schemas.opengis.net/gml/3.1.0/base/feature.xsd"));
		wellKnownFormats.push_back(OWSFormat::create("text/xml; subtype=gml/3.1.0","http://schemas.opengis.net/gml/3.1.0/base/feature.xsd"));
		wellKnownFormats.push_back(OWSFormat::create("text/xml; subtype=gml/3.1.1","http://schemas.opengis.net/gml/3.1.1/base/feature.xsd"));
		wellKnownFormats.push_back(OWSFormat::create("text/xml; subtype=gml/3.2.1","http://schemas.opengis.net/gml/3.2.1/feature.xsd"));

		wellKnownFormats.push_back(OWSFormat::create("application/gml+xml; version=2.1.2","http://schemas.opengis.net/gml/2.1.2/feature.xsd"));
		wellKnownFormats.push_back(OWSFormat::create("application/gml+xml; version=3.1.1","http://schemas.opengis.net/gml/3.1.1/base/feature.xsd"));
		wellKnownFormats.push_back(OWSFormat::create("application/gml+xml; version=3.2.1","http://schemas.opengis.net/gml/3.2.1/feature.xsd"));

		wellKnownFormats.push_back(OWSFormat::create("GML2","http://schemas.opengis.net/gml/2.1.2/feature.xsd"));
		wellKnownFormats.push_back(OWSFormat::create("GML3","http://schemas.opengis.net/gml/3.1.1/base/feature.xsd"));
		wellKnownFormats.push_back(OWSFormat::create("GML32","http://schemas.opengis.net/gml/3.2.1/feature.xsd"));

		wellKnownFormats.push_back(OWSFormat::create("application/tiff"));
		wellKnownFormats.push_back(OWSFormat::create("application/geotiff"));
	}

	return wellKnownFormats;
}

PYXPointer<OWSFormat> OWSFormat::createFromWellKnownMimeType(const std::string & mimeType)
{
	std::list<PYXPointer<OWSFormat>> & formats = getWellKnownFormats();

	for(std::list<PYXPointer<OWSFormat>>::iterator it = formats.begin();it != formats.end();++it)
	{
		if ((**it).supportMimeType(mimeType))
		{
			return (**it).clone();
		}
	}

	return 0;
}