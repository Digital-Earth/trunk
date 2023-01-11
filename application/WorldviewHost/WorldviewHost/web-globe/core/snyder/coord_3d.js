/******************************************************************************
coord_3d.js

begin		: 2015-12-16
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

var MathUtils = require('./math_utils');

/*!
Constructor initializes member variables.

\param	x	The x value (default = 0).
\param	y	The y value (default = 0).
\param	z	The z value (default = 0).
*/
function PYXCoord3D(x, y, z) {
    this.x = typeof x === "number" ? x : 0.0;
    this.y = typeof y === "number" ? y : 0.0;
    this.z = typeof z === "number" ? z : 0.0;
};

/*!
Make a copy.

\return	A copy of these coordinates.
*/
PYXCoord3D.prototype.copy = function () {
    return new PYXCoord3D(this.x, this.y, this.z);
};

/*!
Reset all of the coordinates to 0.
*/
PYXCoord3D.prototype.reset = function () {
    this.x = 0;
    this.y = 0;
    this.z = 0;
};

/*! 
Set all of the values in the in the coordinate

\param x	The value to be assigned to x.
\param y	The value to be assigned to y.
\param z	The value to be assigned to z.
*/
PYXCoord3D.prototype.set = function(x, y, z) {
    this.x = typeof x === "number" ? x : 0.0;
    this.y = typeof y === "number" ? y : 0.0;
    this.z = typeof z === "number" ? z : 0.0;
};

/*!
Check if two points are equal within a given precision. The default
implementation simply calls the equality operator.

\param	rhs		The coordinate to compare with this one.
\param	epsilon	The precision with which to compare.

\return	true if the points are equal, otherwise false.
*/
PYXCoord3D.prototype.equal = function(rhs, epsilon) {
    return (MathUtils.equal(this.x, rhs.x, epsilon) &&
        MathUtils.equal(this.y, rhs.y, epsilon) &&
        MathUtils.equal(this.z, rhs.z, epsilon));
};

/*!
Less than operator provided to allow ordering in containers. Sorting is done
first by x coordinate, then by the y coordinate, then by the z coordinate.

\param	rhs	The point to compare with this one.

\return	true if this point is less than the one passed in otherwise false.
*/
PYXCoord3D.prototype.lessThan = function (rhs) {
    var bLess = (this.x < rhs.x);

    if (this.x === rhs.x)
    {
        bLess = (this.y < rhs.y);

        if (this.y === rhs.y)
        {
            bLess = (this.z < rhs.z);
        }
    }

    return bLess;
}

/*!
Calculate the distance between two points.

\param	pt	The second point.

\return	The distance.
*/
PYXCoord3D.prototype.distance = function(pt) {
    var dX = this.x - pt.x;
    var dY = this.y - pt.y;
    var dZ = this.z - pt.z;

    return Math.sqrt((dX * dX) + (dY * dY) + (dZ * dZ));
};

/*!
Scale the point by the given value.

\param	value	The value by which to scale.
*/
PYXCoord3D.prototype.scale = function(value) {
    this.x *= value;
    this.y *= value;
    this.z *= value;
};

/*!
Calculate the length of the vector from {0,0,0} to this point.

\return The length.
*/
PYXCoord3D.prototype.length = function() {
    return Math.sqrt((this.x * this.x) + (this.y * this.y) + (this.z * this.z));
};

/*!
Calculate the square of the length of the vector from {0,0,0} to this point.

\return The square of the length.
*/
PYXCoord3D.prototype.squareLength = function() {
    return (this.x * this.x) + (this.y * this.y) + (this.z * this.z);
};

/*!
Treat the point as a vector from the origin and normalize so the vector has
a length of 1 unit.
*/
PYXCoord3D.prototype.normalize = function() {
    var fDivisor = this.length();

    // avoid divide by zero
    if (0.0 !== fDivisor) {
        this.x /= fDivisor;
        this.y /= fDivisor;
        this.z /= fDivisor;
    }
};

/*!
Invert each of the three components of the coordinate.
This operation places the coordinate on the opposite side of the origin for each
of the three axis.
*/
PYXCoord3D.prototype.negate = function() {
    this.x = -this.x;
    this.y = -this.y;
    this.z = -this.z;
};

/*!
Calculate the dot product of this vector and a specified vector.

\param	v	The vector

\return	this dot v
*/
PYXCoord3D.prototype.dot = function(v) {
    return (this.x * v.x) + (this.y * v.y) + (this.z * v.z);
};

/*!
Translate the point by an amount defined by another point

\param value	The value to translate by.
*/
PYXCoord3D.prototype.translate = function(value) {
    this.x += value.x;
    this.y += value.y;
    this.z += value.z;
};

/*!
Find the difference between two points.

\param value    The value to subtact from the point.
*/
PYXCoord3D.prototype.subtract = function(value) {
    this.x -= value.x;
    this.y -= value.y;
    this.z -= value.z;
};

/*!
Calculate the cross product of this vector and a specified vector.

\param	v	The vector

\return	this x v
*/
PYXCoord3D.prototype.cross = function(v) {
    var result = new PYXCoord3D();

    result.x = (this.y * v.z) - (this.z * v.y);
    result.y = (this.z * v.x) - (this.x * v.z);
    result.z = (this.x * v.y) - (this.y * v.x);

    return result;
};

/*!
Determine the point at the given parametric distance along the line defined by
two points.

\param	pt1			The first point.
\param	pt2			The second point.
\param	fDistance	The parametric distance in the range [0, 1].

\return	The point at the given parametric distance.
*/
PYXCoord3D.prototype.pointAlongLine = function(pt1, pt2, fDistance) {
    // get deltas
    var fDeltaX = pt2.x - pt1.x;
    var fDeltaY = pt2.y - pt1.y;
    var fDeltaZ = pt2.z - pt1.z;

    return new PYXCoord3D( pt1.x + fDeltaX * fDistance,
        pt1.y + fDeltaY * fDistance,
        pt1.z + fDeltaZ * fDistance );
};


module.exports = PYXCoord3D;
