/******************************************************************************
math_utils.js

now exporting as commonjs module

begin		: 2015-12-14
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/


function defineMathUtils() {

    //! Singleton instance
    var MathUtils = {};

    //! Conversion factor from degrees to radians
    MathUtils.DEGREES_TO_RADIANS = Math.PI / 180.0;

    //! Pre-calculated radian values.
    MathUtils.RAD_30 = 30.0 * MathUtils.DEGREES_TO_RADIANS;
    MathUtils.RAD_60 = 60.0 * MathUtils.DEGREES_TO_RADIANS;
    MathUtils.RAD_90 = 90.0 * MathUtils.DEGREES_TO_RADIANS;
    MathUtils.RAD_120 = 120.0 * MathUtils.DEGREES_TO_RADIANS;
    MathUtils.RAD_150 = 150.0 * MathUtils.DEGREES_TO_RADIANS;
    MathUtils.RAD_180 = 180.0 * MathUtils.DEGREES_TO_RADIANS;
    MathUtils.RAD_240 = 240.0 * MathUtils.DEGREES_TO_RADIANS;
    MathUtils.RAD_300 = 300.0 * MathUtils.DEGREES_TO_RADIANS;
    MathUtils.RAD_360 = 360.0 * MathUtils.DEGREES_TO_RADIANS;

    //! Pre-calculated trig values.
    MathUtils.SIN_30 = Math.sin(MathUtils.RAD_30);
    MathUtils.COS_30 = Math.cos(MathUtils.RAD_30);
    MathUtils.SIN_60 = Math.sin(MathUtils.RAD_60);
    MathUtils.COS_60 = Math.cos(MathUtils.RAD_60);
    MathUtils.TAN_60 = Math.tan(MathUtils.RAD_60);

    //! The default float precision
    MathUtils.DEFAULT_FLOAT_PRECISION = 1.0e-10;

    //! The golden ratio
    MathUtils.PHI = (Math.sqrt(5.0) + 1.0) / 2.0;

    //! The square root of three
    MathUtils.SQRT_3 = Math.sqrt(3.0);

    /*!
        Check to see if two floating point numbers are equal to each other within
        a given precision.

        \param lhs		The left hand side.	
        \param rhs		The right hand side.
        \param epsilon	The precision.

        \return	true if the numbers are equal, false if not
    */
    MathUtils.equal = function (lhs, rhs, epsilon) {
        epsilon = typeof epsilon !== 'undefined' ? epsilon : MathUtils.DEFAULT_FLOAT_PRECISION;
        return (Math.abs(lhs - rhs) < epsilon);
    };

    /*!
    Determine if a value is between two others (inclusive)

    \param	a	The value to be tested for containment.
    \param	b	An end point.
    \param	c	The other end point.

    \return	true if the value is between, otherwise false.
    */
    MathUtils.between = function(a, b, c) {
        return (((a <= b) && (a >= c)) || ((a >= b) && (a <= c)));
    };

    /*!
    Constrain a value to be between a minimum and maximum (inclusive)

    \param	a	The value
    \param	b	The minimum value
    \param	c	The maximum value

    \return	The constrained value.
    */
    MathUtils.constrain = function(a, b, c) {
        var d = Math.max(a, b);
        d = Math.min(d, c);

        return d;
    };

    /*!
    Convert an angle from radians to degrees.

    \param angle	The angle in radians.

    \return	The angle in degrees.
    */
    MathUtils.radiansToDegrees = function(angle) {
        return (angle / MathUtils.DEGREES_TO_RADIANS);
    };

    /*!
    Convert an angle from degrees to radians.

    \param angle	The angle in degrees.

    \return	The angle in radians.
    */
    MathUtils.degreesToRadians = function(angle) {
        return (angle * MathUtils.DEGREES_TO_RADIANS);
    };

    /*!
    Calculate the haversine function.

    \param	angle	The angle in radians.

    \return	The haversine of the angle.
    */
    MathUtils.hav = function(angle) {
        return (1.0 - Math.cos(angle)) / 2.0;
    };

    return MathUtils;
};

// export as entire
module.exports = defineMathUtils();
