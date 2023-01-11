#ifndef PYXIS__UTILITY__BBOX_LAT_LON_H
#define PYXIS__UTILITY__BBOX_LAT_LON_H
/******************************************************************************
bbox_lat_lon.h

begin		: 2016-06-09
copyright	: (C) 2016 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/coord_lat_lon.h"
#include "pyxis/utility/rect_2d.h"

// standard includes
#include <string>

/*!
Calculates a latitude and longitute coordinates specified in radians that can deals with international date line
*/
//! Calculates a latitude and longitute coordinates specified in radians that can deals with international date line
class PYXLIB_DECL BBoxLatLon
{
private:

	PYXRect2DDouble m_westBBox;
	PYXRect2DDouble m_eastBBox;

public:
	//! unit tests
	static void test();


	//! Default constructor initializes empty bbox
	BBoxLatLon()
	{
	}

	//! copy constructor 
	BBoxLatLon(const BBoxLatLon & other) : m_westBBox(other.m_westBBox), m_eastBBox(other.m_eastBBox)
	{
	}

	//! pin a coordiante to be inside the given bbox
	CoordLatLon pin(const CoordLatLon & coord) const;

	//! convert a latlon into PYXCoord2DDouble coordiantes. x=longitude y=latitude
	PYXCoord2DDouble toXY(const CoordLatLon & coord) const;

	//! returns true if bbox is empty
	bool empty() const;

	//! returns true if coord is inside the bbox
	bool inside(const CoordLatLon & coord) const;


	//! expand bbox so this coord would be inside bbox.
	void expand(const CoordLatLon & coord);

	//! expand the bbox in all direction in given amount of spherical radians.
	void expandInRadians(double rads);

	//! expand the bbox in given amount of spherical radians. different amount for latitude and longitude.
	void expandInRadians(double latitudeRads, double longitudeRads);

	//! enforce that the bbox has valid bounds (usally been called automaticly by the class)
	void enforceGlobalBounds();

	//! sanitize bbox to be correct around poles, zero longitute and internaional date line with a given threhsold.
	void sanitize(double thresholdInRadians);

	//! sanitize bbox to be correct around poles, zero longitute and internaional date line with a different given threhsold for latitude and longitudes.
	void sanitize(double latitudeThresholdInRadians,double longitudeThresholdInRadians);

	//! sanitize bbox to be correct around poles with a given threhsold.
	void sanitizeLatitudes(double thresholdInRadians);

	//! sanitize bbox to be correct around zero longitute and internaional date line with a given threhsold.
	void sanitizeLongitudes(double thresholdInRadians);

	//! returns true if it looks like the bbox crossing international date line
	bool looksLikeItCrossesInternationalDateLine(double thresholdInRadians) const;

	//! returns true if it looks like the bbox crossing zero longitude line
	bool looksLikeItCrossesZeroLongitude(double thresholdInRadians) const;

	//! returns width of bbox in spherical radians
	double widthInRadians() const;

	//! returns height of bbox in spherical radians
	double heightInRadians() const;

	//! set bbox to cover the entire globe
	void setToGlobal();

	//! set bbox to be empty
	void setToEmpty();

	//! return a human readable string of the latlon box (in degrees)
	std::string toReadableString() const;

};

#endif // guard
