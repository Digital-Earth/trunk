/******************************************************************************
wgs84.js

begin		: 2015-12-14
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/


var CoordLatLon = require('./coord_lat_lon');
var EllipsoidMath = require('./ellipsoid_math');

function defineWGS84() {

    //! Singleton instance
    var WGS84 = {};

    //! The radius of the semi-major axis (at equator) in metres
    WGS84.SEMI_MAJOR_AXIS = 6378137.0;

    //! The flattening constant = (semi-major - semi-minor) / semi-major
    WGS84.FLATTENING = 1.0 / 298.257223563;
    WGS84.ONE_MINUS_F_SQR = (1.0 - WGS84.FLATTENING) * (1.0 - WGS84.FLATTENING);

    //! The radius of the semi-minor axis (at poles in metres
    WGS84.SEMI_MINOR_AXIS = WGS84.SEMI_MAJOR_AXIS * (1.0 - WGS84.FLATTENING);

    //! Test method
    WGS84.test = function() {
        // test conversion from geocentric to datum coordinates and back
        var pt1 = new CoordLatLon();

        for (var lat = -90; lat < 90; lat += 5) {
            for (var lon = -180; lat < 180; lat += 5) {
                pt1.setInDegrees(lat, lon);
                var pt2 = WGS84.toDatum(pt1);
                pt2 = WGS84.toGeocentric(pt2);

                TEST_ASSERT(pt1.equal(pt2));
            }
        }
    };

    /*!
    Calculate the distance in metres between two points on the earth's
    surface.

    \param	pt1	The first point in datum lat/lon coordinates.
    \param	pt2	The second point in datum lat/lon coordinates.

    \return	The distance in metres.
    */
    WGS84.calcDistance = function(pt1, pt2) {
        return EllipsoidMath.calcDistance(
            pt1,
            pt2,
            WGS84.SEMI_MAJOR_AXIS,
            WGS84.FLATTENING );
    };

    /*!
    Convert a point in datum lat/lon coordinates to geocentric lat/lon
    coordinates.

    \param	pt	The point in datum lat/lon coordinates.

    \return	The point in geocentric lat/lon coordinates.
    */
    WGS84.toGeocentric = function(pt) {
        var lat = Math.atan(Math.tan(pt.lat) * WGS84.ONE_MINUS_F_SQR);

        return new CoordLatLon(lat, pt.lon);
    };

    /*!
    Convert a point in geocentric lat/lon coordinates to datum lat/lon
    coordinates.

    \param	pt	The point in geocentric lat/lon coordinates.

    \return	The point in datum lat/lon coordinates.
    */
    WGS84.toDatum = function(pt) {
        var lat = Math.atan(Math.tan(pt.lat) / WGS84.ONE_MINUS_F_SQR);

        return new CoordLatLon(lat, pt.lon);
    };
    
    return WGS84;
};

module.exports = defineWGS84();

