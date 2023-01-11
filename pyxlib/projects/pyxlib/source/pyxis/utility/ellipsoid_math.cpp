/******************************************************************************
ellipsoid_math.cpp

begin		: 2004-04-01
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "ellipsoid_math.h"

// local includes
#include "math_utils.h"
#include "tester.h"

// system includes
#include <cassert>

//! Tester class
Tester<EllipsoidMath> gTester;

//! Test method
void EllipsoidMath::test()
{
	// test great circle distance calculation
	CoordLatLon pt1;
	CoordLatLon pt2;

	pt1.setInDegrees(-90.0, 0.0);
	pt2.setInDegrees(90.0, 0.0);
	TEST_ASSERT(MathUtils::equal(	MathUtils::kfPI, 
									calcDistance(pt1, pt2, 1.0, 0.0)));

	pt1.setInDegrees(0.0, 0.0);
	pt2.setInDegrees(0.0, 180.0);
	TEST_ASSERT(MathUtils::equal(	MathUtils::kfPI, 
									calcDistance(pt1, pt2, 1.0, 0.0)));

	// test average radius calculation
	TEST_ASSERT(MathUtils::equal(1.0, calcAverageRadius(1.0, 1.0, 0.0)));
	TEST_ASSERT(MathUtils::equal(	1.0,
									calcAverageRadius(	1.0,
														1.0,
														MathUtils::kfPI / 2.0	)	));

	// test geodetic and geocentric latitude conversions
	const double kfFlattening = 0.0034;
	const double kf90Rad = MathUtils::degreesToRadians(90.0);

	TEST_ASSERT(MathUtils::equal(geodeticToGeocentric(kf90Rad, kfFlattening), kf90Rad));
	TEST_ASSERT(MathUtils::equal(geodeticToGeocentric(0.0, kfFlattening), 0.0));
	TEST_ASSERT(MathUtils::equal(geodeticToGeocentric(-kf90Rad, kfFlattening), -kf90Rad));

	TEST_ASSERT(MathUtils::equal(geocentricToGeodetic(kf90Rad, kfFlattening), kf90Rad));
	TEST_ASSERT(MathUtils::equal(geocentricToGeodetic(0.0, kfFlattening), 0.0));
	TEST_ASSERT(MathUtils::equal(geocentricToGeodetic(-kf90Rad, kfFlattening), -kf90Rad));

	const double kf45Rad = MathUtils::degreesToRadians(45.0);
	double fResult = geodeticToGeocentric(kf45Rad, kfFlattening);
	TEST_ASSERT(fResult < kf45Rad);
	TEST_ASSERT(MathUtils::equal(geocentricToGeodetic(fResult, kfFlattening), kf45Rad));
}

/*!
Calculate the distance along the ellipsoid surface between two points.
See http://www.codeguru.com/Cpp/Cpp/algorithms/article.php/c5115/ and
Astronomical Algorithms by Jean Meeus.

\param	pt1			The first point.
\param	pt2			The second point.
\param	fA			The radius of the semi-major axis (at equator).
\param	fFlattening	The flattening constant.

\return	The distance in the same units as the semi-major axis.
*/
double EllipsoidMath::calcDistance(	const CoordLatLon& pt1,
									const CoordLatLon& pt2,
									const double fA,
									const double fFlattening	)
{
	assert(0.0 < fA);
	assert(0.0 <= fFlattening);

	double fF = (pt1.lat() + pt2.lat()) / 2.0;
	double fG = (pt1.lat() - pt2.lat()) / 2.0;
	double fL = (pt1.lon() - pt2.lon()) / 2.0;

	double fSinF = sin(fF);
	double fSinG = sin(fG);
	double fSinL = sin(fL);
	double fCosF = cos(fF);
	double fCosG = cos(fG);
	double fCosL = cos(fL);

	double fS =	fSinG * fSinG * fCosL * fCosL +
				fCosF * fCosF * fSinL * fSinL;

	double fC = fCosG * fCosG * fCosL * fCosL +
				fSinF * fSinF * fSinL * fSinL;

	double fW = atan2(sqrt(fS), sqrt(fC));
	double fR = sqrt((fS * fC)) / fW;
	double fH1 = (3.0 * fR - 1.0) / (2.0 * fC);
	double fH2 = (3.0 * fR + 1.0) / (2.0 * fS);
	double fD = 2.0 * fW * fA;

	return (fD * (	1.0 +
					fFlattening * fH1 * fSinF * fSinF * fCosG * fCosG -
					fFlattening * fH2 * fCosF * fCosF * fSinG * fSinG	));
}

/*!
Calculate the average radius for an ellipsoid at a given latitude. See
http://www.census.gov/cgi-bin/geo/gisfaq?Q5.1.

\param	fA		The semi-major axis.
\param	fB		The semi-minor axis.
\param	fLat	The latitude at which to calculate average radius.

\return	The average radius.
*/
double EllipsoidMath::calcAverageRadius(	const double fA,
											const double fB,
											const double fLat	)
{
	assert(0.0 < fA);
	assert(0.0 < fB);

	double fE = sqrt(1.0 - (fB * fB) / (fA * fA));
	double fE2 = fE * fE;

	double fSinLat = sin(fLat);
	double fRadius = fA * sqrt(1.0 - fE2) / (1.0 - fE2 * fSinLat * fSinLat);

	return fRadius;
}

/*!
Convert a geodetic latitude to a geocentric latitude.

\param	fLat		The geodetic latitude in radians
\param	fFlattening	The flattening constant

\return	The geocentric latitude in radians.
*/
double EllipsoidMath::geodeticToGeocentric(	const double fLat,
											const double fFlattening	)
{
	// ensure latitude is in range
	assert((MathUtils::kfPI / 2.0) >= fabs(fLat));

	// calculate (1 - f)^2
	double fFactor = (1 - fFlattening) * (1 - fFlattening);

	return atan(tan(fLat) * fFactor);
}


/*!
Convert a geocentric latitude to a geodetic latitude.

\param	fLat		The geocentric latitude in radians
\param	fFlattening	The flattening constant

\return	The geodetic latitude in radians.
*/
double EllipsoidMath::geocentricToGeodetic(	const double fLat,
											const double fFlattening	)
{
	// ensure latitude is in range
	assert((MathUtils::kfPI / 2.0) >= fabs(fLat));

	// calculate (1 - f)^2
	double fFactor = (1 - fFlattening) * (1 - fFlattening);

	return atan(tan(fLat) / fFactor);
}


