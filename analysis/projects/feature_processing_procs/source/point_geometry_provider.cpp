/******************************************************************************
point_geometry_provider.cpp

begin		: 2013-6-23
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE

#include "stdafx.h"
#include "point_geometry_provider.h"
#include "pyxis/pipe/process.h"
#include "pyxis/region/vector_point_region.h"
#include "pyxis/derm/point_location.h"
#include "pyxis/geometry/vector_geometry2.h"
#include "pyxis/procs/srs.h"
#include "pyxis/derm/coord_converter.h"


// {5EFB4E9F-7D13-4808-A9A4-058AD38A5DC2}
PYXCOM_DEFINE_CLSID(PointGeometryProvider, 
0x5efb4e9f, 0x7d13, 0x4808, 0xa9, 0xa4, 0x5, 0x8a, 0xd3, 0x8a, 0x5d, 0xc2);



PYXCOM_CLASS_INTERFACES(PointGeometryProvider, IGeometryProvider::iid, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(PointGeometryProvider, "Point Location Geometry Provider", "Provides a geometry for a given IRecord based on the location information on the record (e.g. lat lon, etc)", "Analysis/Features/Geotagging",
					IGeometryProvider::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_PARAMETER(ISRS::iid, 1, 1, "Input SRS", "SRS of the geospatial data in the record")

IPROCESS_SPEC_END

IProcess::eInitStatus PointGeometryProvider::initImpl()
{
	auto srsProc = getParameter(0)->getValue(0)->QueryInterface<ISRS>();
	if(!srsProc)
	{
		setInitProcError<GenericProcInitError>("Invalid SRS");
		return knFailedToInit;
	}	

	auto srs = srsProc->getSRS();

	for(auto & factory : PYXCOM::DI::getAll<ICoordConverterFromSrsFactory>())
	{
		m_coordConverter = factory->createFromSRS(srs);

		if (m_coordConverter)
		{
			break;
		}
	}
	
	if(!m_coordConverter)
	{
		setInitProcError<GenericProcInitError>("Failed to generate coordConvertor for specificed SRS");
		return knFailedToInit;
	}

	return knInitialized;
}

std::map<std::string, std::string> STDMETHODCALLTYPE PointGeometryProvider::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	
	mapAttr["latFieldName"] = m_latFieldName;

	mapAttr["lonFieldName"] = m_lonFieldName;

	mapAttr["Resolution"] = StringUtils::toString(m_resolution);

	return mapAttr;
}

std::string STDMETHODCALLTYPE PointGeometryProvider::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"PointGeometryProvider\">"
		  "<xs:complexType>"
			"<xs:sequence>"

			   "<xs:element name=\"latFieldName\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Latitude Field Name</friendlyName>"
						"<description>Field Name reperesenting latitude</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

				  "<xs:element name=\"lonFieldName\" type=\"xs:string\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Longitude Field Name</friendlyName>"
						"<description>Field Name reperesenting Longitude</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"


				  "<xs:element name=\"Resolution\" type=\"xs:int\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Resolution</friendlyName>"
						"<description>Resolution of the output geometry</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

void STDMETHODCALLTYPE PointGeometryProvider::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	m_initState = knNeedsInit;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"latFieldName",m_latFieldName);
	
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"lonFieldName",m_lonFieldName);

	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"Resolution",int,m_resolution);

}

PYXPointer<PYXGeometry> STDMETHODCALLTYPE PointGeometryProvider::getGeometry(boost::intrusive_ptr<IRecord> & record) const
{
	auto latitude = record->getFieldValueByName(m_latFieldName);
	auto longitude = record->getFieldValueByName(m_lonFieldName);

	//X = long, Y = lat !!!!
	PYXCoord2DDouble native(longitude.getDouble(),latitude.getDouble());
	CoordLatLon latLon;
	
	m_coordConverter->nativeToLatLon(native,&latLon);

	auto region = PYXVectorPointRegion::create(SphereMath::llxyz(latLon));
	
	return PYXVectorGeometry2::create(region, m_resolution);
}