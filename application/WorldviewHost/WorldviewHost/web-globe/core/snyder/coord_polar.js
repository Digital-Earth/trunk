/******************************************************************************
coord_polar.js

begin		: 2015-12-23
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

var MathUtils = require('./math_utils');


//! Test method
PYXCoordPolar.test = function() {
    var pt1 = new PYXCoordPolar();
    var pt2 = new PYXCoordPolar();

    // test equal method

    // test normal cases
    pt1.radius = 10.0;
    pt1.setAngleInDegrees(30.0);
    pt2 = pt1.copy();
    TEST_ASSERT(pt1.equal(pt2));

    pt2.radius = 15.0;
    TEST_ASSERT(!pt1.equal(pt2));

    pt2 = pt1.copy();
    pt2.setAngleInDegrees(45.0);
    TEST_ASSERT(!pt1.equal(pt2));

    // test special case - radius = 0
    pt1.radius = 0.0;
    pt1.setAngleInDegrees(0.0);
    pt2.radius = 0.0;
    pt2.setAngleInDegrees(30.0);
    TEST_ASSERT(pt1.equal(pt2));

    // test special case - angles around +/- 180
    pt1.radius = 10.0;
    pt1.setAngleInDegrees(179.9);
    pt2.radius = 10.0;
    pt2.setAngleInDegrees(-179.9);
    TEST_ASSERT(pt1.equal(pt2, 0.01));

    // test addition and subtraction
    for (var fDeg = 0; fDeg < 360 + Math.PI; fDeg += Math.PI) {
        var fRad = MathUtils.degreesToRadians(fDeg);
        var a = new PYXCoordPolar(1, MathUtils.RAD_30 + fRad);
        var b = new PYXCoordPolar(Math.sqrt(3.0), MathUtils.RAD_120 + fRad);
        var c = new PYXCoordPolar(2, MathUtils.RAD_90 + fRad);

        var d = a.copy();
        d.add(b);
        TEST_ASSERT(d.equal(c));

        var e = b.copy();
        e.add(a);
        TEST_ASSERT(e.equal(c));

        var f = c.copy();
        f.subtract(a);
        TEST_ASSERT(f.equal(b));

        var g = c.copy();
        g.subtract(b);
        TEST_ASSERT(g.equal(a));
    }
};

/*!
Constructor.

\param	radius	The radius.
\param	angle	The angle in radians.
*/
function PYXCoordPolar(radius, angle) {
    this.radius = typeof radius === 'number' ? radius : 0.0;
    this.angle = typeof angle === 'number' ? angle : 0.0;
    this.normalize();
};

/*!
Make a copy.

\return	A copy of these coordinates.
*/
PYXCoordPolar.prototype.copy = function() {
    return new PYXCoordPolar(this.radius, this.angle);
};

/*! Set the angle in radians

\param angle    The angle in radians
*/
PYXCoordPolar.prototype.setAngle = function(angle) {
    this.angle = angle;
    this.normalize();
};

/*! Get the angle in degrees

\return The angle in degrees
*/
PYXCoordPolar.prototype.angleInDegrees = function() {
    return MathUtils.radiansToDegrees(this.angle);
};

/*! Set the angle in degrees

\param angle    The angle in degrees
*/
PYXCoordPolar.prototype.setAngleInDegrees = function(angle) {
    this.angle = MathUtils.degreesToRadians(angle);
    this.normalize();
};

/*!
Normalize the angle. Convert to a value in the range [-pi, pi).
*/
PYXCoordPolar.prototype.normalize = function() {
    while (this.angle > MathUtils.RAD_180) {
        this.angle -= MathUtils.RAD_360;
    }

    while (this.angle < -MathUtils.RAD_180) {
        this.angle += MathUtils.RAD_360;
    }
};

/*!
Are the coordinates equal within a given precision. This method assumes the
angle has been normalized.

\param	pt			The coordinate to compare with this one.
\param	fPrecision	The precision with which to compare.

\return	true if the coordinates are equal within the given precision, otherwise
		false.
*/
PYXCoordPolar.prototype.equal = function(pt, fPrecision) {
    fPrecision = typeof fPrecision !== 'undefined' ? fPrecision : MathUtils.DEFAULT_FLOAT_PRECISION;

    var bEqual = false;

    // check for equal radius
    if (MathUtils.equal(this.radius, pt.radius, fPrecision)) {
        // if the radius is zero, angle doesn't matter
        if (MathUtils.equal(this.radius, 0.0, fPrecision)) {
            bEqual = true;
        } else // check angle
        {
            // if angle is near 180 degrees, must check +/- case
            if (MathUtils.equal(Math.abs(this.angle), MathUtils.RAD_180, fPrecision)) {
                if (MathUtils.equal(Math.abs(this.angle), Math.abs(pt.angle), fPrecision)) {
                    bEqual = true;
                }
            } else if (MathUtils.equal(this.angle, pt.angle, fPrecision)) {
                bEqual = true;
            }
        }
    }

    return bEqual;
};

/*!
Add a polar coordinate to this one.

\param rhs  The polar coordinate to add to this one.

\return this
*/
PYXCoordPolar.prototype.add = function(rhs) {

    if (rhs.radius) {
        // adapted from formulas at http://www.iancgbell.clara.net/maths/vectors.htm
        var x1 = this.radius * Math.cos(this.angle);
        var y1 = this.radius * Math.sin(this.angle);
        var x2 = rhs.radius * Math.cos(rhs.angle);
        var y2 = rhs.radius * Math.sin(rhs.angle);
        var x = x1 + x2;
        var y = y1 + y2;

        this.radius = Math.sqrt((x * x) + (y * y));
        if (x < 0) {
            this.angle = MathUtils.RAD_180 - Math.atan(-y / x);
        } else if (0 < x) {
            this.angle = Math.atan(y / x);
        } else if (y < 0) {
            this.angle = -MathUtils.RAD_90;
        } else if (0 < y) {
            this.angle = MathUtils.RAD_90;
        } else {
            this.angle = 0; // arbitrary
        }

        this.normalize();
    }

    return this;
};

/*!
Subtract a polar coordinate from this one.

\param rhs  The polar coordinate to subtract

\return this
*/
PYXCoordPolar.prototype.subtract = function(rhs) {
    var tmp = rhs.copy();
    tmp.negate();
    this.add(tmp);

    return this;
};

/*!
Restrict angle to range, assuming it's only out by at most two PI.

\param angle    The angle

\return The restricted angle.
*/
PYXCoordPolar.prototype.restrictSingle = function(angle) {
    var fRestrict = angle;
    if (fRestrict < -MathUtils.RAD_180) {
        fRestrict += MathUtils.RAD_360;
    } else if (MathUtils.RAD_180 <= fRestrict) {
        fRestrict -= MathUtils.RAD_360;
    }

    return fRestrict;
};

/*!
Negate the angle for this coordinate.
*/
PYXCoordPolar.prototype.negate = function() {
    this.angle = this.restrictSingle(this.angle + MathUtils.RAD_180);
};

module.exports = PYXCoordPolar;
