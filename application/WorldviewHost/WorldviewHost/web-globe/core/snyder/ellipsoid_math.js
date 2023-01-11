/******************************************************************************
ellipsoid_math.js

begin		: 2015-12-16
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/


var MathUtils = require('./math_utils');
var CoordLatLon = require('./coord_lat_lon');

function defineEllipsoidMath() {

    //! Singleton instance
    var EllipsoidMath = {};

    //! Test method
    EllipsoidMath.test = function() {
        // test great circle distance calculation
        var pt1 = new CoordLatLon;
        var pt2 = new CoordLatLon;

        pt1.setInDegrees(-90.0, 0.0);
        pt2.setInDegrees(90.0, 0.0);
        TEST_ASSERT(MathUtils.equal( Math.PI,
            EllipsoidMath.calcDistance(pt1, pt2, 1.0, 0.0)));

        pt1.setInDegrees(0.0, 0.0);
        pt2.setInDegrees(0.0, 180.0);
        TEST_ASSERT(MathUtils.equal( Math.PI,
            EllipsoidMath.calcDistance(pt1, pt2, 1.0, 0.0)));

        // test average radius calculation
        TEST_ASSERT(MathUtils.equal(1.0, EllipsoidMath.calcAverageRadius(1.0, 1.0, 0.0)));
        TEST_ASSERT(MathUtils.equal(
            1.0,
            EllipsoidMath.calcAverageRadius( 1.0,
                1.0,
                Math.PI / 2.0)));

        // test geodetic and geocentric latitude conversions
        var kfFlattening = 0.0034;
        var kf90Rad = MathUtils.degreesToRadians(90.0);

        TEST_ASSERT(MathUtils.equal(EllipsoidMath.geodeticToGeocentric(kf90Rad, kfFlattening), kf90Rad));
        TEST_ASSERT(MathUtils.equal(EllipsoidMath.geodeticToGeocentric(0.0, kfFlattening), 0.0));
        TEST_ASSERT(MathUtils.equal(EllipsoidMath.geodeticToGeocentric(-kf90Rad, kfFlattening), -kf90Rad));

        TEST_ASSERT(MathUtils.equal(EllipsoidMath.geocentricToGeodetic(kf90Rad, kfFlattening), kf90Rad));
        TEST_ASSERT(MathUtils.equal(EllipsoidMath.geocentricToGeodetic(0.0, kfFlattening), 0.0));
        TEST_ASSERT(MathUtils.equal(EllipsoidMath.geocentricToGeodetic(-kf90Rad, kfFlattening), -kf90Rad));

        var kf45Rad = MathUtils.degreesToRadians(45.0);
        var result = EllipsoidMath.geodeticToGeocentric(kf45Rad, kfFlattening);
        TEST_ASSERT(result < kf45Rad);
        TEST_ASSERT(MathUtils.equal(EllipsoidMath.geocentricToGeodetic(result, kfFlattening), kf45Rad));
    };

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
    EllipsoidMath.calcDistance = function(pt1, pt2, fA, fFlattening)
    {
//	        assert(0.0 < fA);
//	        assert(0.0 <= fFlattening);

        var fF = (pt1.lat + pt2.lat) / 2.0;
        var fG = (pt1.lat - pt2.lat) / 2.0;
        var fL = (pt1.lon - pt2.lon) / 2.0;

        var fSinF = Math.sin(fF);
        var fSinG = Math.sin(fG);
        var fSinL = Math.sin(fL);
        var fCosF = Math.cos(fF);
        var fCosG = Math.cos(fG);
        var fCosL = Math.cos(fL);

        var fS =    fSinG * fSinG * fCosL * fCosL +
			        fCosF * fCosF * fSinL * fSinL;

        var fC =    fCosG * fCosG * fCosL * fCosL +
			        fSinF * fSinF * fSinL * fSinL;

        var fW = Math.atan2(Math.sqrt(fS), Math.sqrt(fC));
        var fR = Math.sqrt((fS * fC)) / fW;
        var fH1 = (3.0 * fR - 1.0) / (2.0 * fC);
        var fH2 = (3.0 * fR + 1.0) / (2.0 * fS);
        var fD = 2.0 * fW * fA;

        return (fD * (	1.0 +
				        fFlattening * fH1 * fSinF * fSinF * fCosG * fCosG -
				        fFlattening * fH2 * fCosF * fCosF * fSinG * fSinG	));
    };

    /*!
    Calculate the average radius for an ellipsoid at a given latitude. See
    http://www.census.gov/cgi-bin/geo/gisfaq?Q5.1.

    \param	fA		The semi-major axis.
    \param	fB		The semi-minor axis.
    \param	fLat	The latitude at which to calculate average radius.

    \return	The average radius.
    */
    EllipsoidMath.calcAverageRadius = function(fA, fB, fLat) {
//	        assert(0.0 < fA);
//	        assert(0.0 < fB);

        var fE = Math.sqrt(1.0 - (fB * fB) / (fA * fA));
        var fE2 = fE * fE;

        var fSinLat = Math.sin(fLat);
        var fRadius = fA * Math.sqrt(1.0 - fE2) / (1.0 - fE2 * fSinLat * fSinLat);

        return fRadius;
    };

    /*!
    Convert a geodetic latitude to a geocentric latitude.

    \param	fLat		The geodetic latitude in radians
    \param	fFlattening	The flattening constant

    \return	The geocentric latitude in radians.
    */
    EllipsoidMath.geodeticToGeocentric = function(fLat, fFlattening) {
        // ensure latitude is in range
//	        assert((MathUtils::kfPI / 2.0) >= fabs(fLat));

        // calculate (1 - f)^2
        var fFactor = (1 - fFlattening) * (1 - fFlattening);

        return Math.atan(Math.tan(fLat) * fFactor);
    };

    /*!
    Convert a geocentric latitude to a geodetic latitude.

    \param	fLat		The geocentric latitude in radians
    \param	fFlattening	The flattening constant

    \return	The geodetic latitude in radians.
    */
    EllipsoidMath.geocentricToGeodetic = function(fLat, fFlattening) {
        // ensure latitude is in range
//	        assert((MathUtils::kfPI / 2.0) >= fabs(fLat));

        // calculate (1 - f)^2
        var fFactor = (1 - fFlattening) * (1 - fFlattening);

        return Math.atan(Math.tan(fLat) / fFactor);
    };
    return EllipsoidMath;
};

module.exports = defineEllipsoidMath();


