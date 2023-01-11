/******************************************************************************
sphere_math.js

begin		: 2015-12-16
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

var MathUtils = require('./math_utils');
var PYXCoord3D = require('./coord_3d');
var CoordLatLon = require('./coord_lat_lon');



function defineSphereMath() {

    //! Singleton instance
    var SphereMath = {};

    //! 100th of a millimeter on the earth's surface
    SphereMath.kfNumericEpsilon = 1e-12;

    //! Any distance smaller than this will return acos = 0
    SphereMath.kfDotNumericEpsilon = 1e-7;

    ///! Test method
    SphereMath.test = function ()
    {
        // test conversions
        var ll = new CoordLatLon();
        var xyz = new PYXCoord3D();

        ll.setLatInDegrees(-90.0);
        ll.setLonInDegrees(0.0);

        xyz = SphereMath.llxyz(ll);
        TEST_ASSERT(xyz.equal(new PYXCoord3D(0.0, 0.0, -1.0)));
        TEST_ASSERT(ll.equal(SphereMath.xyzll(xyz)));

        ll.setLatInDegrees(0.0);
        ll.setLonInDegrees(0.0);

        xyz = SphereMath.llxyz(ll);
        TEST_ASSERT(xyz.equal(new PYXCoord3D(1.0, 0.0, 0.0)));
        TEST_ASSERT(ll.equal(SphereMath.xyzll(xyz)));

        ll.setLatInDegrees(0.0);
        ll.setLonInDegrees(90.0);

        xyz = SphereMath.llxyz(ll);
        TEST_ASSERT(xyz.equal(new PYXCoord3D(0.0, 1.0, 0.0)));
        TEST_ASSERT(ll.equal(SphereMath.xyzll(xyz)));

        ll.setLatInDegrees(0.0);
        ll.setLonInDegrees(180.0);

        xyz = SphereMath.llxyz(ll);
        TEST_ASSERT(xyz.equal(new PYXCoord3D(-1.0, 0.0, 0.0)));
        TEST_ASSERT(ll.equal(SphereMath.xyzll(xyz)));

        ll.setLatInDegrees(0.0);
        ll.setLonInDegrees(-90.0);

        xyz = SphereMath.llxyz(ll);
        TEST_ASSERT(xyz.equal(new PYXCoord3D(0.0, -1.0, 0.0)));
        TEST_ASSERT(ll.equal(SphereMath.xyzll(xyz)));

        ll.setLatInDegrees(90.0);
        ll.setLonInDegrees(0.0);

        xyz = SphereMath.llxyz(ll);
        TEST_ASSERT(xyz.equal(new PYXCoord3D(0.0, 0.0, 1.0)));
        TEST_ASSERT(ll.equal(SphereMath.xyzll(xyz)));

        // test polygon surface area calculations
        var vertices = [
            new CoordLatLon(0.0, 0.0),
            new CoordLatLon(0.0, MathUtils.RAD_90),
            new CoordLatLon(MathUtils.RAD_90, 0.0)
        ];
        TEST_ASSERT(MathUtils.equal(	MathUtils.RAD_90,
								        SphereMath.calcPolygonArea(vertices)	));
    };

    /*!
    Transform a point on a unit sphere from xyz coordinates to a latitude and
    longitude. The xyz point is assumed to be normalized.

    \param	xyz	The point in xyz coordinates

    \return	The point in lat/lon coordinates.
    */
    SphereMath.xyzll = function(xyz) {
        var ll = new CoordLatLon();

        var fZ = xyz.z;
        if ((Math.abs(fZ) - 1) < MathUtils.DEFAULT_FLOAT_PRECISION) {
            fZ = MathUtils.constrain(fZ, -1.0, 1.0);

            ll.setLat(Math.asin(fZ));

            if (ll.isNorthPole() || ll.isSouthPole()) {
                ll.setLon(0.0);
            } else {
                ll.setLon(Math.atan2(xyz.y, xyz.x));
            }
        } else {
            throw "Sphere radius is not one unit.";
        }

        return ll;
    };

    /*!
    Transform a lat/lon to a point on a unit sphere in xyz coordinate.

    \param	ll	The point in lat/lon coordinates

    \return	The point in xyz coordinates.
    */
    SphereMath.llxyz = function(ll) {
        var fX = Math.cos(ll.lat) * Math.cos(ll.lon);
        var fY = Math.cos(ll.lat) * Math.sin(ll.lon);
        var fZ = Math.sin(ll.lat);

        if (MathUtils.equal(fX, 0.0)) {
            fX = 0.0;
        }

        if (MathUtils.equal(fY, 0.0)) {
            fY = 0.0;
        }

        if (MathUtils.equal(fZ, 0.0)) {
            fZ = 0.0;
        }

        return new PYXCoord3D(fX, fY, fZ);
    };

    /*!
    Calculate the area of a closed spherical polygon on the surface of a sphere.
    The vertices form the ends of great circle arcs and are assumed to be in order.
    See "Computing the Area of a Spherical Polygon" by Robert D. Miller in
    "Graphics Gems IV", Academic Press, 1994

    \param	vertices	Array of vertices in lat/lon coordinates.
    \param	fRadius		The radius of the sphere.

    \return	The area.
    */
    SphereMath.calcPolygonArea = function(vertices, fRadius) {
        fRadius = typeof fRadius !== 'undefined' ? fRadius : 1.0;
        var knCount = vertices.length;
        var fSum = 0.0;

        for (var nIndex = 0; nIndex < knCount; ++nIndex) {
            var pt1 = SphereMath.llxyz(vertices[(nIndex + knCount - 1) % knCount]);
            var pt2 = SphereMath.llxyz(vertices[nIndex]);
            var pt3 = SphereMath.llxyz(vertices[(nIndex + 1) % knCount]);

            var cross12 = pt1.cross(pt2);
            cross12.normalize();

            var cross32 = pt3.cross(pt2);
            cross32.normalize();

            var fAngle = Math.acos(cross12.dot(cross32));
            fSum += fAngle;
        }

        return ((fSum - Math.PI * (knCount - 2)) * fRadius * fRadius);
    };

    /*
    Calculate the distance in (radians) between two 3D points on the globe.

    \param pointA   The first point (must be normalized)
    \param pointB   The second point (must be normalized)

    \return The distance in radians.
    */
    SphereMath.distanceBetween3D = function(pointA, pointB) {
        var distance;

        // calculate euclid distance, which is numeric safe (for our needs right now)
        var fEuclidDistance = pointA.distance(pointB);
        if (fEuclidDistance <= SphereMath.kfNumericEpsilon) {
            // this is considered to be the same point
            return 0;
        } else if (fEuclidDistance < SphereMath.kfDotNumericEpsilon) {
            // cross operation is more stable when points are close to each other
            var cross = pointA.cross(pointB).length();
            return Math.asin(cross);
        } else {
            // it is safe to do a dot operation which is faster than cross
            var dot = pointA.dot(pointB);

            // make sure that acos will work - dot sometimes returns values larger than 1.0 and then acos returns #NAN.
            if (dot > 1.0) {
                dot = 1.0;
            } else if (dot < -1.0) {
                dot = -1.0;
            }

            return Math.acos(dot);
        }
    };

    /*!
    Calculate the distance in (radians) between two lat/lon points on the globe.

    \param pointA   The first point.
    \param pointB   The second point.

    \return The distance in radians
    */
    SphereMath.distanceBetweenLatLon = function(pointA, pointB) {
        var fA = Math.sin((pointA.lat - pointB.lat) / 2.0);
        var fB = Math.sin((pointA.lon - pointB.lon) / 2.0);

        var fRadians = 2 * Math.asin( Math.sqrt(fA * fA +
            Math.cos(pointA.lat) *
            Math.cos(pointB.lat) *
            fB * fB) );
        return fRadians;
    };

    /*!
    Calculate the heading (0...360) from a location to a different location.

    \param from The starting point
    \param to   The ending point

    \return The heading in degrees
    */
    SphereMath.headingInDegrees = function(from, to) {
        // Taken from: http://www.yourhomenow.com/house/haversine.html
        var y = Math.sin(to.lon - from.lon) * Math.cos(from.lat);
        var x = Math.cos(to.lat) * Math.sin(from.lat) -
            Math.sin(to.lat) * Math.cos(to.lat) * Math.cos(to.lon - from.lon);
        var bearing = Math.atan2(y, x) / MathUtils.DEGREES_TO_RADIANS;

        if (bearing < 0) {
            bearing += 360;
        }
        return bearing;
    };

    return SphereMath;
};

module.exports = defineSphereMath();
