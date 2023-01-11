#ifndef PYXIS__UTILITY__COORD_LAT_LON_H
#define PYXIS__UTILITY__COORD_LAT_LON_H
/******************************************************************************
coord_lat_lon.h

begin		: 2004-01-13
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/math_utils.h"

// standard includes
#include <string>

//! Earth radius in metres
const double earthRadius = 6370997;

//! Radius of icosahedron enclosing the earth 
static double icosahedronRadius = sqrt(earthRadius*earthRadius*3/2*sqrt(3.0)/15939499735869.30);

//! Ration of the circumference to the diameter 
#define PI 3.14159265358979323846264338327

/*!
CoordLatLon contains latitude and longitude coordinates specified in radians.
*/
//! A coordinate on a spherical surface specified by a latitude and longitude.
class PYXLIB_DECL CoordLatLon
{
public:

	//! Maximum absolute latitude value in degrees.
	static const double kfLatitudeAbsMax;
	
	//! Maximum absolute longitude value in degrees.
	static const double kfLongitudeAbsMax;

	//! Test method
	static void test();

	//! Default constructor initializes to the equator at the prime meridian.
	CoordLatLon() : m_fLat(0.0), m_fLon(0.0) {;}

	//! Constructor initializes member variables.
	CoordLatLon(double fLat, double fLon);

	/*!
	Constructor for a destination point 
	   see http://williams.best.vwh.net/avform.htm#LL
	\param from intial location
	\param bearing initial bearing (radian)
	\param distance distance at the given bearing
	*/
	CoordLatLon(const CoordLatLon from, double bearing, double distance);

	//! Destructor
	virtual ~CoordLatLon() {;}

	//! Get the latitude in radians
	double lat() const {return m_fLat;}

	//! Set the latitude in radians
	void setLat(double fLat);

	//! Get the latitude in degrees
	double latInDegrees() const;

	//! Set the latitude in degrees
	void setLatInDegrees(double fLatInDegrees);

	//! Get the longitude in radians
	double lon() const {return m_fLon;}

	//! Set the longitude in radians
	void setLon(double fLon);

	//! Get the longitude in degrees
	double lonInDegrees() const;

	//! Set the longitude in degrees
	void setLonInDegrees(double fLonInDegrees);

	//! Set the latitude and longitude in radians
	void set(double fLat, double fLon);

	//! Set the latitude and longitude in degrees
	void setInDegrees(double fLatInDegrees, double fLonInDegrees);

	//! Randomizes this coordinate.
	void randomize();

	/*!
	Perturbs (uniformly) the latitude in the range [-fMaxDeg, fMaxDeg] and the
	longitude in the range [-fMaxDeg, fMaxDeg].

	\param fMaxDeg The maximum (non-negative) degrees to perturb this coordinate.
	*/
	//! Perturbs this coordinate.
	inline void perturb(double fMaxDeg);

	//! Perturbs this coordinate.
	void perturb(double fMaxDegLat, double fMaxDegLon);

	//! Does this coordinate represent the north pole
	bool isNorthPole(double fEpsilon = MathUtils::kfDefaultDoublePrecision) const;

	//! Does this coordinate represent the south pole
	bool isSouthPole(double fEpsilon = MathUtils::kfDefaultDoublePrecision) const;

	//! Is the latitude on the equator.
	bool isOnEquator(double fEpsilon = MathUtils::kfDefaultDoublePrecision) const;

	//! Is this coordinate on the prime meridian
	bool isOnPrimeMeridian(double fEpsilon = MathUtils::kfDefaultDoublePrecision) const;

	//! Is this coordinate on the international date line
	bool isOnIntlDateLine(double fEpsilon = MathUtils::kfDefaultDoublePrecision) const;

	//! Are the coordinates equal within a given precision
	bool equal(	const CoordLatLon& latLon,
				double fPrecision = MathUtils::kfDefaultDoublePrecision	) const;

	//! Determine if a coordinate falls inside a rectangle.
	bool insideSWNE(	const CoordLatLon& southWest,
						const CoordLatLon& northEast	) const;

	bool insideNWSE(	const CoordLatLon& nouthWest,
						const CoordLatLon& southEast	) const;

	//! Determine if an arc crosses the international date line.
	static bool crossesIntlDateLine(	const CoordLatLon& pt1,
										const CoordLatLon& pt2	);

	//! Equality operator provided to allow ordering in containers.
	bool operator==(const CoordLatLon& pt) const;

	//! Less than operator provided to allow ordering in containers
	bool operator<(const CoordLatLon& pt) const;

	//! Calculates the resolution of a lat lon coordinate in radians.
	static double calculateResolutionRadians
							(	const std::string& strLatitude,
								const std::string& strLongitude	);
	
	//! Check if the latitude value in degrees is valid.
	static bool isLatitudeValid(double fLatitude);

	//! Check if the longitude value in degrees is valid.
	static bool isLongitudeValid(double fLongitude);

	//! Get the north pole.
	static const CoordLatLon& northPole();

	//! Get the south pole.
	static const CoordLatLon& southPole();

	//!	Calculate the shortest distance from 'this' point to the line specified by L1, L2
	double distanceToLine(CoordLatLon& L1, CoordLatLon& L2);

	/*!
	Length of the sides of the polygon at a given resolution	
	\param resolution
	\return (double)
	*/
	static double sideN(double resolution); 

	/*!
	Circumradius of an hexagon at the given resolution
	\param resolution
	\return (double)
	*/
	static double hexagonCircumradius(double resolution);

	/*!
	Inside radius of an hexagon at the given resolution
	\param resolution
	\return (double)
	*/
	static double hexagonInradius(double resolution);

	/*!
	Circumradius of an pentagon at the given resolution
	\param resolution
	\return (double)
	*/
	static double pentagonCircumradius(double resolution); 

	/*!
	Inside radius of an pentagon at the given resolution	
	\param resolution
	\return (double)
	*/
	static double pentagonInradius(double resolution);

	/*!
	Calculates the Haversine distance between two polar coordinates (on earth).

	\returns The distance between the two points in metres.
	*/
	//! Distance between two coordinate (result is positive)
	double operator-(const CoordLatLon& pt) const;

protected:

private:

	//! Normalize the latitude
	void normalizeLatitude();

	//! Normalize the longitude
	void normalizeLongitude();

	//! The latitude in radians [-pi/2, pi/2]
	double m_fLat;

	//! The longitude in radians [-pi, pi)
	double m_fLon;
};

//! Allows CoordLatLons to be written to streams.
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const CoordLatLon& latLon);

//! Pre-computed information for a lat/lon coordinate
class PYXLIB_DECL PreCompLatLon
{
public:

	//! Default constructor.
	PreCompLatLon() {;}

	//! Constructor
	PreCompLatLon(const CoordLatLon& point)
	{
		setPoint(point);
	}

	//! Set the point in lat/lon coordinates
	void setPoint(const CoordLatLon& point);

	//! Get the point in lat/lon coordinates
	const CoordLatLon& point() const {return m_point;}

	//! Get the sin of the latitude
	double sinLat() const {return m_fSinLat;}

	//! Get the sin of the longitude
	double sinLon() const {return m_fSinLon;}

	//! Get the cos of the latitude
	double cosLat() const {return m_fCosLat;}

	//! Get the cose of the longitude
	double cosLon() const {return m_fCosLon;}

protected:

private:

	//! The point in lat/lon coordinates
	CoordLatLon m_point;

	//! The sin of the latitude
	double m_fSinLat;

	//! The sin of the longitude
	double m_fSinLon;

	//! The cos of the latitude
	double m_fCosLat;

	//! The cos of the longitude
	double m_fCosLon;
};

#endif // guard
