/******************************************************************************
grib_coord_converter.cpp

begin		: 2006-03-29
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define GRIB_SOURCE
#include "grib_coord_converter.h"

// local includes

#include "pyxis/derm/horizontal_datum.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/derm/wgs84.h"
#include "pyxis/utility/math_utils.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/tester.h"
#include "pyxis/data/exceptions.h"
#include "pyxis/sampling/spatial_reference_system.h"

// standard includes
#include <cassert>
#include <memory>

// {1F82445A-7482-427d-8102-B22E6C4C72CB}
PYXCOM_DEFINE_CLSID(GRIBCoordConverter, 
0x1f82445a, 0x7482, 0x427d, 0x81, 0x02, 0xb2, 0x2e, 0x6c, 0x4c, 0x72, 0xcb);

PYXCOM_CLASS_INTERFACES(GRIBCoordConverter, ICoordConverter::iid, PYXCOM_IUnknown::iid);

/*!
Constructor.
*/
GRIBCoordConverter::GRIBCoordConverter()
{
}

/*!
Copy constructor.
*/
GRIBCoordConverter::GRIBCoordConverter(const GRIBCoordConverter& other) :
m_fLonStart(other.m_fLonStart),
m_fLatStart(other.m_fLatStart),
m_fLonStep(other.m_fLonStep),
m_fLatStep(other.m_fLatStep)
{
}

/*!
Reads the object from the input stream.
\param in The input stream.
*/
GRIBCoordConverter::GRIBCoordConverter(std::istream& in)
{
}

/*!
Destructor. Delete the GeospatialCoordTransforms
*/
GRIBCoordConverter::~GRIBCoordConverter()
{
}

/*!
Specify the native spatial reference system with a PYXSpatialReferenceSystem.

\param	fLonStart	Longitude of start of data for the data source.
\param	fLatStart	Latitude of start of data for the data source.
\param	fLonStep	Longitude increment for data.
\param	fLatStep	Latitude increment for data.
*/
void GRIBCoordConverter::initialize(double fLonStart, double fLatStart,
									double fLonStep, double fLatStep)
{
	m_fLonStart = fLonStart;
	m_fLatStart = fLatStart;
	m_fLonStep = fLonStep;
	m_fLatStep = fLatStep;
}

/*!
Convert native coordinates to WGS84 coordinates.

\param	native	The position in native coordinates.
\param	pll		The position in geodetic lat lon coordinates (out)
*/
void GRIBCoordConverter::nativeToWGS84(	const PYXCoord2DDouble& native,
										CoordLatLon* pll	) const
{
	assert(pll != 0);

	// Scale values and add offset.
	double fX = (native.x() * m_fLonStep) + m_fLonStart;
	double fY = (native.y() * m_fLatStep) + m_fLatStart;

	if (fX > 180.0)
	{
		fX -= 360.0;
	}

	pll->setLonInDegrees(fX);
	pll->setLatInDegrees(fY);
}

/*!
Convert WGS84 coordinates to native coordinates.

\param	ll		The position in geodetic lat lon coordinates.
\param	pNative	The position in native coordinates (out)
*/
void GRIBCoordConverter::wgs84ToNative(	const CoordLatLon& ll,
										PYXCoord2DDouble* pNative	) const
{
	assert(pNative != 0);

	double fX = ll.lonInDegrees();
	double fY = ll.latInDegrees();

	fX -= m_fLonStart;
	fY -= m_fLatStart;

	if (fX < 0.0 && m_fLonStart >= 0.0)
	{
		fX += 360.0;
	}

	// If we have a data source that wraps - go back to start.
	if (fX > 359.0)
	{
		fX -= 359.0;
	}

	// Scale values based on lat/lon increment.
	fX /= m_fLonStep;
	fY /= m_fLatStep;

	pNative->setX(fX);
	pNative->setY(fY);
}

/*!
Convert WGS84 coordinates to a PYXIS index.

\param	ll			The position in geodetic lat lon coordinates.
\param	pIndex		The PYXIS index (out)
\param	nResolution	The resolution of the resulting index.
*/
void GRIBCoordConverter::wgs84ToPYXIS(	const CoordLatLon& ll,
										PYXIcosIndex* pIndex,
										int nResolution	) const
{
	assert(pIndex != 0);
	assert(1 < nResolution);

	// convert to geocentric coordinates
	CoordLatLon llGeocentric;
	llGeocentric = WGS84::getInstance()->toGeocentric(ll);

	// convert to a PYXIS index
	SnyderProjection::getInstance()->nativeToPYXIS(llGeocentric, pIndex, nResolution);
}

/*!
Convert a PYXIS index to WGS84 coordinates.

\param	index	The PYXIS index
\param	pll		The position in geodetic lat lon coordinates (out)
*/
void GRIBCoordConverter::pyxisToWGS84(	const PYXIcosIndex& index,	
										CoordLatLon* pll	) const
{
	assert(pll != 0);

	CoordLatLon llGeocentric;
	SnyderProjection::getInstance()->pyxisToNative(index, &llGeocentric);

	// convert to WGS84 coordinates
	*pll = WGS84::getInstance()->toDatum(llGeocentric);
}


/*!
Convert native coordinates to a PYXIS index.

\param	native		The native coordinates.
\param	pIndex		The PYXIS index (out)
\param	nResolution	The resolution of the resulting index.
*/
void GRIBCoordConverter::nativeToPYXIS(	const PYXCoord2DDouble& native,
										PYXIcosIndex* pIndex,
										int nResolution	) const
{
	assert(pIndex != 0);
	assert(1 < nResolution);

	CoordLatLon ll;

	// convert to WGS84 coordinates
	nativeToWGS84(native, &ll);

	// convert to a PYXIS index
	wgs84ToPYXIS(ll, pIndex, nResolution);
}

/*!
Convert a PYXIS index to native coordinates.

\param	index	The PYXIS index.
\param	pNative	The native coordinates (out)
*/
void GRIBCoordConverter::pyxisToNative(	const PYXIcosIndex& index,
										PYXCoord2DDouble* pNative	) const
{
	assert(pNative != 0);

	CoordLatLon ll;

	// convert to WGS84 coordinates
	pyxisToWGS84(index, &ll);

	// convert to native coordinates
	wgs84ToNative(ll, pNative);
}

//! Convert a PYXIS index to native coordinates.
bool GRIBCoordConverter::tryPyxisToNative( const PYXIcosIndex& index, PYXCoord2DDouble* pNative	) const
{
	//this will always works
	pyxisToNative(index,pNative);
	return true;
}

//! Convert a native coordinate to a geocentric LatLon coordinates.
void GRIBCoordConverter::nativeToLatLon(const PYXCoord2DDouble& native,
								CoordLatLon * pLatLon) const
{
	assert(pLatLon);

	CoordLatLon wgs84latlon;

	nativeToWGS84(native,&wgs84latlon);

	*pLatLon = WGS84::getInstance()->toGeocentric(wgs84latlon);
}

//! Convert a native coordinate to a geocentric LatLon coordinates.
void GRIBCoordConverter::latLonToNative(const CoordLatLon & latLon,
								PYXCoord2DDouble * pNative) const
{
	assert(pNative != 0);

	CoordLatLon wgs84latlon;

	wgs84latlon = WGS84::getInstance()->toDatum(latLon);

	wgs84ToNative(wgs84latlon,pNative);
}


/*!
Get the Datum as a string

\param	nDatum	The datum.

\return	The Datum as a string.
*/
std::string GRIBCoordConverter::getDatumString(PYXSpatialReferenceSystem::eDatum nDatum) 
{
	std::string strGeogCS;

	switch (nDatum)
	{
		case PYXSpatialReferenceSystem::knDatumNAD27:
		{
			strGeogCS = "NAD27";		
			break;
		}

		case PYXSpatialReferenceSystem::knDatumNAD83:
		{
			strGeogCS = "NAD83";		
			break;
		}
	
		case PYXSpatialReferenceSystem::knDatumWGS72:
		{
			strGeogCS = "WGS72";
			break;
		}

		case PYXSpatialReferenceSystem::knDatumWGS84:
		{
			strGeogCS = "WGS84";		
			break;
		}

		default:
		{
			strGeogCS = "None";
		}
	}

	return strGeogCS;
	
}

/*!
Determine if two converter objects represent the same conversion.

\param	converter	The GRIBCoordConverter object to be compared with this one.

\return	true if they are the same, otherwise false.
*/
bool GRIBCoordConverter::operator==(const GRIBCoordConverter& converter) const
{
	return true;
}

/*!
Writes the object to the output stream.
\param out The output stream.
\return The output stream.
*/
std::basic_ostream</*unsigned*/ char>& GRIBCoordConverter::serialize(std::basic_ostream</*unsigned*/ char>& out) const
{
	return out;
}

//! Deserialize.
std::basic_istream</*unsigned*/ char>& GRIBCoordConverter::deserialize(std::basic_istream</*unsigned*/ char>& in)
{
	return in;
}

//! Serialize the COM object.
void GRIBCoordConverter::serializeCOM(std::basic_ostream</*unsigned*/ char>& out) const
{
	out << clsid;
	serialize(out);
}
