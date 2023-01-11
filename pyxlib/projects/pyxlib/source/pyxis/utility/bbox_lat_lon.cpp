/******************************************************************************
bbox_lat_lon.cpp

begin		: 2016-06-09
copyright	: (C) 2016 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/bbox_lat_lon.h"

// local includes
#include "pyxis/utility/math_utils.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cassert>
#include <cmath>
#include <cstdlib>

Tester<BBoxLatLon> gBBoxLatLonTester;

CoordLatLon latLonDegrees(double lat,double lon) {
	return CoordLatLon(MathUtils::degreesToRadians(lat),MathUtils::degreesToRadians(lon));
}

void BBoxLatLon::test()
{
	BBoxLatLon bbox;

	//testing empty and global states
	{
		TEST_ASSERT(bbox.empty());

		bbox.setToGlobal();
		TEST_ASSERT(bbox.inside(latLonDegrees(-89,-179)));
		TEST_ASSERT(bbox.inside(latLonDegrees(45,-45)));
		TEST_ASSERT(bbox.inside(latLonDegrees(0,0)));
		TEST_ASSERT(bbox.inside(latLonDegrees(-45,45)));
		TEST_ASSERT(bbox.inside(latLonDegrees(89,179)));
		TEST_ASSERT(!bbox.empty());

		bbox.setToEmpty();
		TEST_ASSERT(!bbox.inside(latLonDegrees(-89,-179)));
		TEST_ASSERT(!bbox.inside(latLonDegrees(45,-45)));
		TEST_ASSERT(!bbox.inside(latLonDegrees(0,0)));
		TEST_ASSERT(!bbox.inside(latLonDegrees(-45,45)));
		TEST_ASSERT(!bbox.inside(latLonDegrees(89,179)));
		TEST_ASSERT(bbox.empty());
	}

	//test single east bbox
	{
		bbox.expand(latLonDegrees(10,10));
		bbox.expand(latLonDegrees(60,90));

		TEST_ASSERT(!bbox.empty());	
	
		TEST_ASSERT(bbox.inside(latLonDegrees(10,10)));	
		TEST_ASSERT(bbox.inside(latLonDegrees(30,60)));	
		TEST_ASSERT(bbox.inside(latLonDegrees(60,90)));

		TEST_ASSERT(MathUtils::equal(bbox.widthInRadians(),MathUtils::degreesToRadians(80),MathUtils::degreesToRadians(1)));
		TEST_ASSERT(MathUtils::equal(bbox.heightInRadians(),MathUtils::degreesToRadians(50),MathUtils::degreesToRadians(1)));

		TEST_ASSERT(!bbox.inside(latLonDegrees(0,90)));	
		TEST_ASSERT(!bbox.inside(latLonDegrees(-30,60)));	
	}

	//testing zero longtitue sanitize
	{
		bbox.expand(latLonDegrees(-10,-10));
		bbox.expand(latLonDegrees(-20,-20));

		TEST_ASSERT(bbox.looksLikeItCrossesZeroLongitude(MathUtils::degreesToRadians(10)));
		TEST_ASSERT(!bbox.looksLikeItCrossesZeroLongitude(MathUtils::degreesToRadians(5)));
		TEST_ASSERT(!bbox.looksLikeItCrossesInternationalDateLine(MathUtils::degreesToRadians(10)));

		bbox.sanitize(MathUtils::degreesToRadians(10));

		TEST_ASSERT(bbox.inside(latLonDegrees(0,0)));	
		TEST_ASSERT(bbox.inside(latLonDegrees(-10,90)));
		TEST_ASSERT(bbox.inside(latLonDegrees(60,-20)));	
	}

	//testing expand
	{
		TEST_ASSERT(!bbox.inside(latLonDegrees(-30,0)));

		bbox.expandInRadians(MathUtils::degreesToRadians(10));

		TEST_ASSERT(bbox.inside(latLonDegrees(-29,0)));
		TEST_ASSERT(!bbox.inside(latLonDegrees(-31,0)));
	}

	//testing international date line sanitize
	{
		bbox.setToEmpty();

		//create east bbox: [-45..80] [100..170]
		bbox.expand(latLonDegrees(-45,100));	
		bbox.expand(latLonDegrees(0,150));	
		bbox.expand(latLonDegrees(80,170));

		//create east bbox [-40 70] [-170 -150]
		bbox.expand(latLonDegrees(-40,-150));	
		bbox.expand(latLonDegrees(10,-160));	
		bbox.expand(latLonDegrees(70,-170));

		TEST_ASSERT(!bbox.looksLikeItCrossesZeroLongitude(MathUtils::degreesToRadians(10)));
		TEST_ASSERT(bbox.looksLikeItCrossesInternationalDateLine(MathUtils::degreesToRadians(10)));

		//sanatize bbox
		bbox.sanitize(MathUtils::degreesToRadians(10));

		TEST_ASSERT(!bbox.inside(latLonDegrees(0,0)));
		TEST_ASSERT(bbox.inside(latLonDegrees(0,180)));
		TEST_ASSERT(bbox.inside(latLonDegrees(0,-180)));

		TEST_ASSERT(MathUtils::equal(bbox.widthInRadians(),MathUtils::degreesToRadians(110),MathUtils::degreesToRadians(1)));
		TEST_ASSERT(MathUtils::equal(bbox.heightInRadians(),MathUtils::degreesToRadians(125),MathUtils::degreesToRadians(1)));
	}

	//test pin go to right side
	{
		//pin [80,0] -> [-45,100]
		TEST_ASSERT(bbox.pin(latLonDegrees(-80,0)).equal(latLonDegrees(-45,100),MathUtils::degreesToRadians(1)));

		//pin [-40,-10] -> [-40,100] -> pin to east bbox which closer to [-40,-150] pin to west bbox
		TEST_ASSERT(bbox.pin(latLonDegrees(-40,-10)).equal(latLonDegrees(-40,100),MathUtils::degreesToRadians(1)));

		//ping [40,-50] -> [40,-150] -> pin to west bbox
		TEST_ASSERT(bbox.pin(latLonDegrees(40,-50)).equal(latLonDegrees(40,-150),MathUtils::degreesToRadians(1)));
	}
}

CoordLatLon BBoxLatLon::pin(const CoordLatLon & coord) const
{
	if (inside(coord)) 
	{
		return coord;
	}

	PYXCoord2DDouble xy = toXY(coord);

	auto west = m_westBBox.pin(xy);
	auto east = m_eastBBox.pin(xy);

	if (m_eastBBox.empty() || std::abs(west.x()-xy.x()) < std::abs(east.x()-xy.x()))
	{
		//we are closer to east
		return CoordLatLon(west.y(),west.x());
	}
	else
	{
		//we are closer to west
		return CoordLatLon(east.y(),east.x());
	}	
}

PYXCoord2DDouble BBoxLatLon::toXY(const CoordLatLon & coord) const
{
	return PYXCoord2DDouble(coord.lon(),coord.lat());
}

bool BBoxLatLon::empty() const 
{
	return m_westBBox.empty() && m_eastBBox.empty();
}

bool BBoxLatLon::inside(const CoordLatLon & coord) const
{
	PYXCoord2DDouble xy = toXY(coord);
	return m_westBBox.inside(xy) || m_eastBBox.inside(xy);
}

void BBoxLatLon::expand(const CoordLatLon & coord)
{
	PYXCoord2DDouble xy = toXY(coord);
	
	if (xy.x()<0) 
	{
		m_westBBox.expand(xy);
	}
	else 
	{
		m_eastBBox.expand(xy);
	}

	enforceGlobalBounds();
}

void BBoxLatLon::expandInRadians(double rads)
{
	expandInRadians(rads,rads);
}

void BBoxLatLon::expandInRadians(double latitudeRads, double longitudeRads)
{
	if (!m_westBBox.empty()) 
	{
		m_westBBox.expand(PYXCoord2DDouble( m_westBBox.xMin()-longitudeRads, m_westBBox.yMin()-latitudeRads ));
		m_westBBox.expand(PYXCoord2DDouble( m_westBBox.xMax()+longitudeRads, m_westBBox.yMax()+latitudeRads ));
	}

	if (!m_eastBBox.empty()) 
	{
		m_eastBBox.expand(PYXCoord2DDouble( m_eastBBox.xMin()-longitudeRads, m_eastBBox.yMin()-latitudeRads ));
		m_eastBBox.expand(PYXCoord2DDouble( m_eastBBox.xMax()+longitudeRads, m_eastBBox.yMax()+latitudeRads ));
	}

	enforceGlobalBounds();
}

void BBoxLatLon::enforceGlobalBounds()
{
	if (!m_eastBBox.empty())
	{
		m_eastBBox.clip(PYXRect2DDouble(0,-MathUtils::kf90Rad,MathUtils::kf180Rad,MathUtils::kf90Rad));
	}

	if (!m_westBBox.empty())
	{
		m_westBBox.clip(PYXRect2DDouble(-MathUtils::kf180Rad,-MathUtils::kf90Rad,0,MathUtils::kf90Rad));
	}
}


void BBoxLatLon::sanitize(double thresholdInRadians)
{
	sanitizeLatitudes(thresholdInRadians);
	sanitizeLongitudes(thresholdInRadians);
}

void BBoxLatLon::sanitize(double latitudeThresholdInRadians,double longitudeThresholdInRadians)
{
	sanitizeLatitudes(latitudeThresholdInRadians);
	sanitizeLongitudes(longitudeThresholdInRadians);
}


void BBoxLatLon::sanitizeLatitudes(double thresholdInRadians)
{
	if (m_westBBox.empty() || m_eastBBox.empty()) 
	{
		//nothing to sanitize
		return;
	}

	//align min/max on y axis (latitude)
	auto maxY = std::max(m_westBBox.yMax(),m_eastBBox.yMax());
	auto minY = std::min(m_westBBox.yMin(),m_eastBBox.yMin());
	m_westBBox.setYMax(maxY);
	m_westBBox.setYMin(minY);	
	m_eastBBox.setYMax(maxY);
	m_eastBBox.setYMin(minY);

	//pin to north and south pole if region is large enough
	if (widthInRadians() > MathUtils::kf180Rad) 
	{
		auto border = MathUtils::kf90Rad - thresholdInRadians;
		if (m_westBBox.yMax() >= border)
		{
			m_westBBox.setYMax(MathUtils::kf90Rad);
			m_eastBBox.setYMax(MathUtils::kf90Rad);
		}

		if (m_westBBox.yMin() <= -border)
		{
			m_westBBox.setYMin(-MathUtils::kf90Rad);
			m_eastBBox.setYMin(-MathUtils::kf90Rad);
		}
	}
}

void BBoxLatLon::sanitizeLongitudes(double thresholdInRadians)
{
	if (m_westBBox.empty() || m_eastBBox.empty()) 
	{
		//nothing to sanitize
		return;
	}

	if (looksLikeItCrossesInternationalDateLine(thresholdInRadians))
	{
		m_westBBox.setXMin(-MathUtils::kf180Rad);
		m_eastBBox.setXMax(MathUtils::kf180Rad);
	}

	if (looksLikeItCrossesZeroLongitude(thresholdInRadians))
	{
		m_westBBox.setXMax(0);
		m_eastBBox.setXMin(0);
	}
}

bool BBoxLatLon::looksLikeItCrossesInternationalDateLine(double thresholdInRadians) const
{
	if (m_westBBox.empty() || m_eastBBox.empty()) 
	{
		return false;
	}

	auto border =  MathUtils::kf180Rad - thresholdInRadians;
	return m_eastBBox.xMax() >= border && m_westBBox.xMin() <= -border;
}

bool BBoxLatLon::looksLikeItCrossesZeroLongitude(double thresholdInRadians) const
{
	if (m_westBBox.empty() || m_eastBBox.empty()) 
	{
		return false;
	}

	return m_eastBBox.xMin() <= thresholdInRadians && m_westBBox.xMax() >= -thresholdInRadians;
}


double BBoxLatLon::widthInRadians() const
{
	return (m_westBBox.empty() ? 0 : m_westBBox.width()) + (m_eastBBox.empty() ? 0 : m_eastBBox.width());
}

double BBoxLatLon::heightInRadians() const
{
	return std::max(m_westBBox.height(),m_eastBBox.height());
}

void BBoxLatLon::setToGlobal()
{
	m_eastBBox = PYXRect2DDouble(0,-MathUtils::kf90Rad,MathUtils::kf180Rad,MathUtils::kf90Rad);
	m_westBBox = PYXRect2DDouble(-MathUtils::kf180Rad,-MathUtils::kf90Rad,0,MathUtils::kf90Rad);
}

void BBoxLatLon::setToEmpty()
{
	m_eastBBox.setEmpty();
	m_westBBox.setEmpty();
}

std::string BBoxLatLon::toReadableString() const 
{
	std::string result;

	if (empty()) 
	{
		result += "empty bbox";
	}
	else if (m_westBBox.empty()) 
	{
		result += "longitudes:[" + StringUtils::toString(MathUtils::radiansToDegrees(m_eastBBox.xMin())) + ".." + StringUtils::toString(MathUtils::radiansToDegrees(m_eastBBox.xMax())) + "]";

		result += " latitudes:[" + StringUtils::toString(MathUtils::radiansToDegrees(m_eastBBox.yMin())) + ".." + StringUtils::toString(MathUtils::radiansToDegrees(m_eastBBox.yMax())) + "]";
	}
	else if (m_eastBBox.empty()) 
	{
		result += "longitudes:[" + StringUtils::toString(MathUtils::radiansToDegrees(m_westBBox.xMin())) + ".." + StringUtils::toString(MathUtils::radiansToDegrees(m_westBBox.xMax())) + "]";

		result += " latitudes:[" + StringUtils::toString(MathUtils::radiansToDegrees(m_westBBox.yMin())) + ".." + StringUtils::toString(MathUtils::radiansToDegrees(m_westBBox.yMax())) + "]";
	}
	else 
	{
		if (looksLikeItCrossesZeroLongitude(widthInRadians()/100)) 
		{
			//we using west bbox.xMin as min longitude and east bbox.xMax
			result += "longitudes:[" + StringUtils::toString(MathUtils::radiansToDegrees(m_westBBox.xMin())) + ".." + StringUtils::toString(MathUtils::radiansToDegrees(m_eastBBox.xMax())) + "]";

			//east and west latitudes bounds are the same
			result += " latitudes:[" + StringUtils::toString(MathUtils::radiansToDegrees(m_westBBox.yMin())) + ".." + StringUtils::toString(MathUtils::radiansToDegrees(m_westBBox.yMax())) + "]";
		}
		else 
		{
			//this is a complex bbox, show longitudes for both east and west
			result += "longitudes:[" + StringUtils::toString(MathUtils::radiansToDegrees(m_westBBox.xMin())) + ".." + StringUtils::toString(MathUtils::radiansToDegrees(m_westBBox.xMax())) + 
				" and " + StringUtils::toString(MathUtils::radiansToDegrees(m_eastBBox.xMin())) + ".." + StringUtils::toString(MathUtils::radiansToDegrees(m_eastBBox.xMax())) + "]";

			//east and west latitudes bounds are the same
			result += " latitudes:[" + StringUtils::toString(MathUtils::radiansToDegrees(m_westBBox.yMin())) + ".." + StringUtils::toString(MathUtils::radiansToDegrees(m_westBBox.yMax())) + "]";
		}
	}

	return result;
}