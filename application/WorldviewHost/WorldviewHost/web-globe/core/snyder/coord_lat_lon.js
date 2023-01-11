/******************************************************************************
coord_lat_lon.js

begin		: 2015-12-14
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

var MathUtils = require('./math_utils');

//! Maximum absolute latitude value in degrees.
CoordLatLon.LATITUDE_ABS_MAX = 90.0;
	
//! Maximum absolute longitude value in degrees.
CoordLatLon.LONGITUDE_ABS_MAX = 180.0;

//! Test method
CoordLatLon.test = function() {
    // test default constructor
    var latLon = new CoordLatLon();
    TEST_ASSERT(MathUtils.equal(0.0, latLon.lat));
    TEST_ASSERT(MathUtils.equal(0.0, latLon.lon));

    // test normalization
    latLon.setLatInDegrees(45.0);
    TEST_ASSERT(MathUtils.equal(45.0, latLon.latInDegrees()));

    latLon.setLatInDegrees(135.0);
    TEST_ASSERT(MathUtils.equal(45.0, latLon.latInDegrees()));

    latLon.setLatInDegrees(225.0);
    TEST_ASSERT(MathUtils.equal(-45.0, latLon.latInDegrees()));

    latLon.setLatInDegrees(315.0);
    TEST_ASSERT(MathUtils.equal(-45.0, latLon.latInDegrees()));

    latLon.setLatInDegrees(405.0);
    TEST_ASSERT(MathUtils.equal(45.0, latLon.latInDegrees()));

    latLon.setLatInDegrees(-45.0);
    TEST_ASSERT(MathUtils.equal(-45.0, latLon.latInDegrees()));

    latLon.setLatInDegrees(-135.0);
    TEST_ASSERT(MathUtils.equal(-45.0, latLon.latInDegrees()));

    latLon.setLatInDegrees(-225.0);
    TEST_ASSERT(MathUtils.equal(45.0, latLon.latInDegrees()));

    latLon.setLatInDegrees(-315.0);
    TEST_ASSERT(MathUtils.equal(45.0, latLon.latInDegrees()));

    latLon.setLatInDegrees(-405.0);
    TEST_ASSERT(MathUtils.equal(-45.0, latLon.latInDegrees()));

    latLon.setLonInDegrees(45.0);
    TEST_ASSERT(MathUtils.equal(45.0, latLon.lonInDegrees()));

    latLon.setLonInDegrees(135.0);
    TEST_ASSERT(MathUtils.equal(135.0, latLon.lonInDegrees()));

    latLon.setLonInDegrees(225.0);
    TEST_ASSERT(MathUtils.equal(-135.0, latLon.lonInDegrees()));

    latLon.setLonInDegrees(315.0);
    TEST_ASSERT(MathUtils.equal(-45.0, latLon.lonInDegrees()));

    latLon.setLonInDegrees(405.0);
    TEST_ASSERT(MathUtils.equal(45.0, latLon.lonInDegrees()));

    latLon.setLonInDegrees(-45.0);
    TEST_ASSERT(MathUtils.equal(-45.0, latLon.lonInDegrees()));

    latLon.setLonInDegrees(-135.0);
    TEST_ASSERT(MathUtils.equal(-135.0, latLon.lonInDegrees()));

    latLon.setLonInDegrees(-225.0);
    TEST_ASSERT(MathUtils.equal(135.0, latLon.lonInDegrees()));

    latLon.setLonInDegrees(-315.0);
    TEST_ASSERT(MathUtils.equal(45.0, latLon.lonInDegrees()));

    latLon.setLonInDegrees(-405.0);
    TEST_ASSERT(MathUtils.equal(-45.0, latLon.lonInDegrees()));

    // test inside method

    var southWest = new CoordLatLon();
    southWest.setInDegrees(-45.0, 90.0);

    var northEast = new CoordLatLon();
    northEast.setInDegrees(45.0, -90.0);

    latLon.setInDegrees(0.0, 180.0);
    TEST_ASSERT(latLon.insideSWNE(southWest, northEast));
    TEST_ASSERT(!latLon.insideSWNE(northEast, southWest));

    latLon.setInDegrees(75.0, 180.0);
    TEST_ASSERT(!latLon.insideSWNE(southWest, northEast));
    TEST_ASSERT(!latLon.insideSWNE(northEast, southWest));

    latLon.setInDegrees(0.0, 0.0);
    TEST_ASSERT(!latLon.insideSWNE(southWest, northEast));
    TEST_ASSERT(!latLon.insideSWNE(northEast, southWest));

    // test north pole, south pole, equator, prime meridian and int'l date line
    latLon.setInDegrees(90.0, 0.0);
    TEST_ASSERT(latLon.isNorthPole());
    TEST_ASSERT(!latLon.isSouthPole());
    TEST_ASSERT(!latLon.isOnEquator());
    TEST_ASSERT(latLon.isOnPrimeMeridian());
    TEST_ASSERT(!latLon.isOnIntlDateLine());

    latLon.setInDegrees(-90.0, 180.0);
    TEST_ASSERT(!latLon.isNorthPole());
    TEST_ASSERT(latLon.isSouthPole());
    TEST_ASSERT(!latLon.isOnEquator());
    TEST_ASSERT(!latLon.isOnPrimeMeridian());
    TEST_ASSERT(latLon.isOnIntlDateLine());

    latLon.setInDegrees(0.0, 180.0);
    TEST_ASSERT(!latLon.isNorthPole());
    TEST_ASSERT(!latLon.isSouthPole());
    TEST_ASSERT(latLon.isOnEquator());

    // test equal

    var test = new CoordLatLon();
    latLon.setInDegrees(45.0, 90.0);

    test.setInDegrees(45.0, 90.0);
    TEST_ASSERT(latLon.equal(test));

    test.setInDegrees(-45.0, -90.0);
    TEST_ASSERT(!latLon.equal(test));

    // test crossesIntlDateLine
    var ll1 = new CoordLatLon();
    var ll2 = new CoordLatLon();

    ll1.setInDegrees(0.0, 170.0);
    ll2.setInDegrees(0.0, -170.0);
    TEST_ASSERT(CoordLatLon.crossesIntlDateLine(ll1, ll2));

    ll1.setInDegrees(0.0, 10.0);
    ll2.setInDegrees(0.0, -10.0);
    TEST_ASSERT(!CoordLatLon.crossesIntlDateLine(ll1, ll2));
};

/*!
Constructor initializes member variables.

\param	lat	The latitude in radians.
\param	lon	The longitude in radians.
*/
function CoordLatLon(lat, lon) {
    this.lat = typeof lat === "number" ? lat : 0.0;
    this.lon = typeof lon === "number" ? lon : 0.0;
	this.normalizeLatitude();
	this.normalizeLongitude();
};

/*!
Make a copy.

\return	A copy of these coordinates.
*/
CoordLatLon.prototype.copy = function () {
    return new CoordLatLon(this.lat, this.lon);
};

/*!
Set the latitude in radians. This method normalizes the latitude to a value in
the range [-pi/2, pi/2].

\param lat	The latitude in radians.
*/
CoordLatLon.prototype.setLat = function(lat) {
    this.lat = lat;
    this.normalizeLatitude();
};

/*!
Get the latitude in degrees.

\return	The latitude in degrees.
*/
CoordLatLon.prototype.latInDegrees = function() {
    return MathUtils.radiansToDegrees(this.lat);
};

/*!
Set the latitude in degrees.

\param	latInDegrees	The latitude in degrees.
*/
CoordLatLon.prototype.setLatInDegrees = function(latInDegrees) {
    this.setLat(MathUtils.degreesToRadians(latInDegrees));
};

/*!
Set the longitude in radians. This method normalizes the longitude to a value
in the range [-pi, pi)

\param	lon	The longitude in radians.
*/
CoordLatLon.prototype.setLon = function(lon) {
    this.lon = lon;
    this.normalizeLongitude();
};

/*!
Get the longitude in degrees.

\return	The longitude in degrees.
*/
CoordLatLon.prototype.lonInDegrees = function() {
    return MathUtils.radiansToDegrees(this.lon);
};

/*!
Set the longitude in degrees.

\param	lonInDegrees	The longitude in degrees.
*/
CoordLatLon.prototype.setLonInDegrees = function(lonInDegrees) {
    this.setLon(MathUtils.degreesToRadians(lonInDegrees));
};

/*!
Set the latitude and longitude in radians.

\param	lat	The latitude in radians.
\param	lon	The longitude in radians.
*/
CoordLatLon.prototype.setLatLon = function(lat, lon) {
    this.setLat(lat);
    this.setLon(lon);
};

/*!
Set the latitude and longitude in degrees.

\param	latInDegrees	The latitude in degrees.
\param	lonInDegrees	The longitude in degrees.
*/
CoordLatLon.prototype.setInDegrees = function(latInDegrees, lonInDegrees) {
    this.setLatInDegrees(latInDegrees);
    this.setLonInDegrees(lonInDegrees);
};

/*!
Randomizes (uniformly) the latitude in the range [-pi/2, pi/2] and the longitude
in the range [-pi, pi).
*/
CoordLatLon.prototype.randomize = function() {
    // No need to normalize as we manage the range manually.
    this.lat = (Math.random() * Math.PI) - (Math.PI / 2.0);

    this.lon = (Math.random() * Math.PI * 2.0) - Math.PI;

    if (this.lon === Math.PI) {
        this.lon = -Math.PI;
    }
};

/*!
Perturbs (uniformly) the latitude in the range [-fMaxDegLat, fMaxDegLat] and
the longitude in the range [-fMaxDegLon, fMaxDegLon].

\param fMaxDegLat The maximum (non-negative) degrees to perturb the latitude.
\param fMaxDegLon The maximum (non-negative) degrees to perturb the longitude.
*/
CoordLatLon.prototype.perturb = function (fMaxDegLat, fMaxDegLon) {
    this.setLatInDegrees(this.latInDegrees() + (Math.random() * (2 * fMaxDegLat)) - fMaxDegLat);
    this.setLonInDegrees(this.lonInDegrees() + (Math.random() * (2 * fMaxDegLon)) - fMaxDegLon);
};

/*!
Perturbs (uniformly) the latitude in the range [-fMaxDeg, fMaxDeg] and the
longitude in the range [-fMaxDeg, fMaxDeg].

\param fMaxDeg The maximum (non-negative) degrees to perturb this coordinate.
*/
CoordLatLon.prototype.perturb = function(fMaxDeg) {
    this.perturb(fMaxDeg, fMaxDeg);
};

/*!
Does the latitude represent the north pole.

\param epsilon	The precision.

\return true if the latitude is the north pole, otherwise false.
*/
CoordLatLon.prototype.isNorthPole = function(epsilon) {
    return MathUtils.equal(this.lat, MathUtils.RAD_90, epsilon);
};

/*!
Does the latitude represent the south pole.

\param epsilon	The precision.

\return true if the latitude is the south pole, otherwise false.
*/
CoordLatLon.prototype.isSouthPole = function(epsilon) {
    return MathUtils.equal(this.lat, -MathUtils.RAD_90, epsilon);
};

/*!
Is the latitude on the equator.

\param epsilon	The precision.

\return true if the latitude is on the equator, otherwise false.
*/
CoordLatLon.prototype.isOnEquator = function(epsilon) {
    return MathUtils.equal(this.lat, 0.0, epsilon);
};

/*!
Is the longitude on the the prime meridian.

\param	epsilon	The precision.

\return	true if the longitude is on the prime meridian, otherwise false
*/
CoordLatLon.prototype.isOnPrimeMeridian = function(epsilon) {
    return MathUtils.equal(this.lon, 0.0, epsilon);
};

/*!
Is the longitude on the international date line.

\param	epsilon	The precision.

\return	true if the latitude is on the international date line, otherwise false
*/
CoordLatLon.prototype.isOnIntlDateLine = function(epsilon) {
    return (MathUtils.equal(this.lon, MathUtils.RAD_180, epsilon) ||
        MathUtils.equal(this.lon, -MathUtils.RAD_180, epsilon));
};

/*!
Are the coordinates equal within a given precision. This method handles the
special conditions at the poles and at the international date line. The
coordinates must be normalized before entry to this method.

\param latLon		The latitude / longitude to compare.
\param epsilon	The precision to use for comparison.

\return true if the coordinates are equal otherwise false.
*/
CoordLatLon.prototype.equal = function(latLon, epsilon) {
    var bEqual = false;

    // handle special cases at the poles, ignore longitude
    if (this.isNorthPole(epsilon)) {
        bEqual = latLon.isNorthPole(epsilon);
    } else if (this.isSouthPole(epsilon)) {
        bEqual = latLon.isSouthPole(epsilon);
    } else if (MathUtils.equal(this.lat, latLon.lat, epsilon)) {
        // latitudes are equal and not at poles, check longitude

        // handle special case on the international date line
        if (this.isOnIntlDateLine(epsilon)) {
            bEqual = latLon.isOnIntlDateLine(epsilon);
        } else {
            bEqual = MathUtils.equal(this.lon, latLon.lon, epsilon);
        }
    }

    return bEqual;
};

/*!
Determine if a coordinate falls inside or on the border of the rectangle
defined by a southwest and northeast coordinate.

\param southWest    The southwest coordinate
\param northEast    The northeast coordinate

\return	true if the coordinate is inside the rectangle, otherwise false.
*/
CoordLatLon.prototype.insideSWNE = function(southWest, northEast) {
    var bInside = false;

    // check latitudes first
    if ((southWest.lat <= this.lat) && (northEast.lat >= this.lat)) {
        /*
		Check for special case where the longitudes straddle the international
		date line.
		*/
        if (southWest.lon > northEast.lon) {
            bInside = ((southWest.lon <= this.lon) ||
            (northEast.lon >= this.lon));
        } else {
            // normal case
            bInside = ((southWest.lon <= this.lon) &&
            (northEast.lon >= this.lon));
        }
    }

    return bInside;
};

/*!
Determine if a coordinate falls inside or on the border of the rectangle
defined by a northwest and southeast coordinate.

\param northWest    The northwest coordinate
\param southEast    The southeast coordinate

\return	true if the coordinate is inside the rectangle, otherwise false.
*/
CoordLatLon.prototype.insideNWSE = function(northWest, southEast) {
    var bInside = false;

    // check latitudes first
    if ((southEast.lat <= this.lat) && (northWest.lat >= this.lat)) {
        /*
		Check for special case where the longitudes straddle the international
		date line.
		*/
        if (northWest.lon > southEast.lon) {
            bInside = ((northWest.lon <= this.lon) ||
            (southEast.lon >= this.lon));
        } else {
            // normal case
            bInside = ((northWest.lon <= this.lon) &&
            (southEast.lon >= this.lon));
        }
    }

    return bInside;
};

/*!
Normalize the latitude. Convert to a value in the range [-pi/2, pi/2].
*/
CoordLatLon.prototype.normalizeLatitude = function() {

    // convert latitude to value in the range [0, 2pi)
    var nMultiple = Math.floor(this.lat / (2.0 * Math.PI));
    this.lat -= 2.0 * Math.PI * nMultiple;

    if (0.0 > this.lat) {
        this.lat = 2.0 * Math.PI + this.lat;
    }

    // convert latitude to a value in the range [-pi/2, pi/2]
    if ((3.0 * Math.PI / 2.0) < this.lat) {
        this.lat = this.lat - 2.0 * Math.PI;
    } else if (Math.PI < this.lat) {
        this.lat = Math.PI - this.lat;
    } else if ((Math.PI / 2.0) < this.lat) {
        this.lat = Math.PI - this.lat;
    }
};

/*!
Normalize the longitude. Convert to a value in the range [-pi, pi).
*/
CoordLatLon.prototype.normalizeLongitude = function() {

    // convert longitude to value in the range [0, 2pi)
    var nMultiple = Math.floor(this.lon / (2.0 * Math.PI));
    this.lon -= 2.0 * Math.PI * nMultiple;

    // convert longitude to value in the range [-pi, pi)
    if (Math.PI <= this.lon) {
        this.lon -= 2.0 * Math.PI;
    } else if (-Math.PI > this.lon) {
        this.lon += 2.0 * Math.PI;
    }
};

/*!
Determine if the shortest great circle arc between two points crosses the
international date line.

\param	pt1	The first point.
\param	pt2	The second coordinate.

\return	true if the coordinates straddle the date line otherwise false.
*/
CoordLatLon.crossesIntlDateLine = function(pt1, pt2) {
    return (!pt1.isOnIntlDateLine() &&
        !pt2.isOnIntlDateLine() &&
        (Math.PI < Math.abs(pt2.lon - pt1.lon)));
};

/*!
Less than operator provided to allow ordering in containers. Sorting is done
first by longitude, then by latitude.

\param	rhs	The point to be compared with this one.

\return	true if this point is less than the one passed in otherwise false.
*/
CoordLatLon.prototype.lessThan = function(rhs) {
    var bLess = (this.lon < rhs.lon);

    if (MathUtils.equal(this.lon, rhs.lon)) {
        bLess = (this.lat < rhs.lat);
    }

    return bLess;
};

/* 
Check to see if the latitude in degrees is valid.

\param latitude The latitude in degrees.

\return true if valid, false otherwise.
*/
CoordLatLon.prototype.isLatitudeValid = function(latitude) {
    return (Math.abs(latitude) <= CoordLatLon.LATITUDE_ABS_MAX);
};

/* 
Check to see if the longitude in degrees is valid.

\param longitude The longitude in degrees.

\return true if valid, false otherwise.
*/
CoordLatLon.prototype.isLongitudeValid = function(longitude) {
    return (Math.abs(longitude) <= CoordLatLon.LONGITUDE_ABS_MAX);
};

/*!
Get the north pole.

\return	The north pole.
*/
CoordLatLon.prototype.northPole = function() {
    return new this.CoordLatLon(MathUtils.RAD_90, 0.0);
};

/*!
Get the south pole.

\return	The south pole.
*/
CoordLatLon.prototype.southPole = function() {
    return new this.CoordLatLon(-MathUtils.RAD_90, 0.0);
};

/*!
Calculate the shortest distance from 'this' point to the line specified by L1, L2

\param p1	Starting point of line to measure distance to.
\param p2	Ending point of line to measure distance to.

\return	Distance from point to line in degrees.
*/
CoordLatLon.prototype.distanceToLine = function(p1, p2) {
    var y0 = p1.latInDegrees();
    var x0 = p1.lonInDegrees();

    var y1 = p2.latInDegrees();
    var x1 = p2.lonInDegrees();

    var y = this.latInDegrees();
    var x = this.lonInDegrees();

    var x1x0 = x1 - x0;
    var y1y0 = y1 - y0;

    var fTop = (y1y0 * x) + (x1x0 * y) + ((x0 * y1) - (x1 * y0));

    var fBottom = Math.sqrt((x1x0 * x1x0) + (y1y0 * y1y0));

    return Math.abs(fTop / fBottom);
};

module.exports = CoordLatLon;

