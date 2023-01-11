/******************************************************************************
wgs84_coord_converter.cpp

begin		: 2005-09-28
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "wgs84_coord_converter.h"

// local includes
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/wgs84.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/file_utils.h"

// standard includes

// {615542A3-539B-485b-BBF8-B4E250BC090F}
PYXCOM_DEFINE_CLSID(WGS84CoordConverter, 
0x615542a3, 0x539b, 0x485b, 0xbb, 0xf8, 0xb4, 0xe2, 0x50, 0xbc, 0x09, 0x0f);

PYXCOM_CLASS_INTERFACES(WGS84CoordConverter, ICoordConverter::iid, PYXCOM_IUnknown::iid);

//! Tester class.
Tester<WGS84CoordConverter> gTester;

//! Test method.
void WGS84CoordConverter::test()
{
	// Test serialization.
	{
		// Create coord converter.
		WGS84CoordConverter conv(42);

		// Serialize coord converter.
		std::string strPath = FileUtils::pathToString(AppServices::makeTempFile(".test"));
		{
			std::basic_ofstream<char> out(strPath.c_str(), std::ios_base::binary);
			conv.serialize(out);
		}

		// Deserialize.
		WGS84CoordConverter conv2;
		{
			std::basic_ifstream<char> in(strPath.c_str(), std::ios_base::binary);
			conv2.deserialize(in);
		}

		// Compare.
		// Can't do equality on doubles.
		TEST_ASSERT(conv2.m_fUnitsPerDegree > 41 && conv2.m_fUnitsPerDegree < 43);
	}
}

/*!
Reads the object from the input stream.
\param in The input stream.
*/
WGS84CoordConverter::WGS84CoordConverter(std::basic_istream<char>& in)
{
	deserialize(in);
}

/*!
Convert native coordinates to a PYXIS index.

\param	native		The native coordinates.
\param	pIndex		The PYXIS index (out)
\param	nResolution	The resolution of the resulting index.
*/
void WGS84CoordConverter::nativeToPYXIS(	const PYXCoord2DDouble& native,
											PYXIcosIndex* pIndex,
											int nResolution	) const
{
	assert(pIndex != 0);
	assert(1 < nResolution);

	CoordLatLon geodeticLL;
	geodeticLL.setInDegrees(	native.y() / m_fUnitsPerDegree,
								native.x() / m_fUnitsPerDegree	);

	// convert to geocentric lat lon coordinates
	CoordLatLon geocentricLL = WGS84::getInstance()->toGeocentric(geodeticLL);

	// convert to a PYXIS index
	SnyderProjection::getInstance()->nativeToPYXIS(	geocentricLL,
													pIndex,
													nResolution	);
}

/*!
Convert a PYXIS index to native coordinates.

\param	index	The PYXIS index.
\param	pNative	The native coordinates (out)
*/
void WGS84CoordConverter::pyxisToNative(	const PYXIcosIndex& index,
											PYXCoord2DDouble* pNative	) const
{
	assert(pNative != 0);

	// calculate the geocentric lat lon coordinates for the index.
	CoordLatLon geocentricLL;
	SnyderProjection::getInstance()->pyxisToNative(	index,
													&geocentricLL	);

	// calculate the geodetic lat lon coordinates
	CoordLatLon geodeticLL = WGS84::getInstance()->toDatum(geocentricLL);

	pNative->setX(geodeticLL.lonInDegrees() * m_fUnitsPerDegree);
	pNative->setY(geodeticLL.latInDegrees() * m_fUnitsPerDegree);
}

/*!
Convert a PYXIS index to native coordinates.

\param	index	The PYXIS index.
\param	pNative	The native coordinates (out)
*/
bool WGS84CoordConverter::tryPyxisToNative(const PYXIcosIndex& index, PYXCoord2DDouble* pNative) const
{
	//this will always works
	pyxisToNative(index,pNative);
	return true;
}

//! Convert a native coordinate to a geocentric LatLon coordinates.
void WGS84CoordConverter::nativeToLatLon(	const PYXCoord2DDouble& native,
											CoordLatLon * pLatLon) const
{
	assert(pLatLon);

	CoordLatLon geodeticLL;
	geodeticLL.setInDegrees(	native.y() / m_fUnitsPerDegree,
								native.x() / m_fUnitsPerDegree	);

	// convert to geocentric lat lon coordinates
	*pLatLon = WGS84::getInstance()->toGeocentric(geodeticLL);
}

//! Convert a native coordinate to a geocentric LatLon coordinates.
void WGS84CoordConverter::latLonToNative(	const CoordLatLon & latLon,
											PYXCoord2DDouble * pNative) const
{
	// calculate the geodetic lat lon coordinates
	CoordLatLon geodeticLL = WGS84::getInstance()->toDatum(latLon);

	pNative->setX(geodeticLL.lonInDegrees() * m_fUnitsPerDegree);
	pNative->setY(geodeticLL.latInDegrees() * m_fUnitsPerDegree);
}

/*!
Writes the object to the output stream.
\param out The output stream.
\return The output stream.
*/
std::basic_ostream<char>& WGS84CoordConverter::serialize(std::basic_ostream<char>& out) const
{
	return out << m_fUnitsPerDegree;
}

//! Deserialize.
std::basic_istream<char>& WGS84CoordConverter::deserialize(std::basic_istream<char>& in)
{
	return in >> m_fUnitsPerDegree;
}

//! Serialize the COM object.
void WGS84CoordConverter::serializeCOM(std::basic_ostream<char>& out) const
{
	out << clsid;
	serialize(out);
}
