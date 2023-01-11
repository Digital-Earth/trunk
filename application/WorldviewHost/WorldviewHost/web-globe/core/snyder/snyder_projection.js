/******************************************************************************
snyder_projection.js

-  modified to use commonjs module system (require etc.)

begin		: 2015-12-23
copyright	: derived from DgProjSnyder by Kevin Sahr
web			: www.pyxisinnovation.com
******************************************************************************/


var MathUtils = require('./math_utils');
var Icosahedron = require('./icosahedron');
var ReferenceSphere = require('./reference_sphere');
var CoordLatLon = require('./coord_lat_lon');
var PYXCoordPolar = require('./coord_polar');

var TEST_ASSERT = function(v)
{
    if (!v)
        throw new 10;
}



function defineSnyderProjection() {

    //! Singleton instance
    var SnyderProjection = {};

    //! The icosahedron
    SnyderProjection.sphIcosa = new Icosahedron();

    //! The best precision we can get from the Snyder Projection
    SnyderProjection.PRECISION = 5.0e-8;

    //! Precalculated value
    SnyderProjection.COT_30 = 1.0 / Math.tan(MathUtils.RAD_30);

    /*
    The spherical angle in degrees between a radius vector to the centre and the
    adjacent edge of a spherical polygon on the globe.
    */
    SnyderProjection.GH = MathUtils.degreesToRadians(36.0);
    SnyderProjection.SIN_GH = Math.sin(SnyderProjection.GH);
    SnyderProjection.COS_GH = Math.cos(SnyderProjection.GH);

    /*
    The spherical distance in radians from the centre of the polygon face to any of
    its vertices on the globe. Approximately 37.3773681406498 degrees.
    */
    SnyderProjection.DH = Math.asin(SnyderProjection.sphIcosa.SIDE_LENGTH / MathUtils.SQRT_3);
    SnyderProjection.COS_DH = Math.cos(SnyderProjection.DH);
    SnyderProjection.TAN_DH = Math.tan(SnyderProjection.DH);
    SnyderProjection.TAN_DH_SQR = SnyderProjection.TAN_DH * SnyderProjection.TAN_DH;

    SnyderProjection.SIN_GH_COS_DH = SnyderProjection.SIN_GH * SnyderProjection.COS_DH;

    /*
    The radius of an icosahedron relative to its enclosing sphere for equal area
    projection. Approximately 0.9103832815095.
    */
    SnyderProjection.R1 = Math.sqrt(
        Math.PI / (	15 * 
        SnyderProjection.TAN_DH_SQR * 
        MathUtils.SIN_30 * 
        MathUtils.COS_30		));

    SnyderProjection.R1_SQR = SnyderProjection.R1 * SnyderProjection.R1;

    /*
    The formula for kfOriginXOffset was derived from the Snyder forward projection
    method by specifying a lat/lon that results in the maximum x value.
    */
    SnyderProjection.ORIGIN_X_OFFSET = SnyderProjection.R1 * SnyderProjection.TAN_DH * Math.sin(MathUtils.RAD_120);
    SnyderProjection.ORIGIN_Y_OFFSET = SnyderProjection.ORIGIN_X_OFFSET * Math.tan(MathUtils.RAD_30);
    SnyderProjection.EDGE_SCALE = 2.0 * SnyderProjection.ORIGIN_X_OFFSET;

    /*
    This section determines the distance between resolution 0 PYXIS cells in both
    metres and radians.
    */
    var sqrt5 = Math.sqrt(5.0);
    var phi = (1 + sqrt5) / 2;              // golden ratio
    var fR = Math.sqrt(phi * sqrt5) / 2;    // circumradius of unit icosahedron
    SnyderProjection.UNIT_RADIAN_DISTANCE = Math.asin(0.5 / fR) * 2; // radians between icosahedron vertices
    SnyderProjection.UNIT_METRE_DISTANCE = ReferenceSphere.RADIUS * SnyderProjection.UNIT_RADIAN_DISTANCE;

    //! Test method
    SnyderProjection.test = function () {

        // test forward and reverse projection with lat/lon for precision
        for (var fLat = -90.0; fLat <= 90.0; fLat += 2.5) {
            for (var fLon = -180.0; fLon < 180.0; fLon += 2.5) {
                var ll = new CoordLatLon();
                ll.setInDegrees(fLat, fLon);

                var result = SnyderProjection.projectToFace(ll);
                var llResult = SnyderProjection.projectToSphere(result.polar, result.face);

                // the best precision we can achieve through a forward and reverse projection
                var kfTestPrecision = 1.0e-10;
                TEST_ASSERT(ll.equal(llResult, kfTestPrecision));
            }
        }
    };

    /*!
    Project a point on a sphere to a face on the icosahedron.

    \param	ll		The point on the sphere in lat/lon coordinates.

    \return polar	The polar coordinates relative to the centre of the triangle. The angle
			        is measured counter-clockwise from the base of the triangle towards the
                    opposite vertex and the radius is specified such that each side of the
                    face has a length of one unit.
            face	The face of the icosahedron ["A"-"T"]
    */
    SnyderProjection.projectToFace = function(ll) {
        var polar = new PYXCoordPolar();

        // find the face (triangle) in which the point lies
        var face = this.sphIcosa.findFace(ll);

        /*
		Perform the Snyder forward projection to determine the point in the
		triangle.
		*/
        polar = SnyderProjection.sllra(ll, face);

        return {
            polar: polar,
            face: face
        };
    };

    /*!
    Using the Snyder projection, project a point specified in lat/lon coordinates
    onto a face on the icosahedron. Return the distance and angle from the centre
    of the face where the angle is measured counter-clockwise from the base of the
    triangle toward the opposite vertex and each side of the face has a length of
    one unit.

    \param	ll		The point in lat/lon coordinates
    \param	faceID	The face in which the point lies ["A"-"T"]

    \return	The polar coordinate relative to the centre of the face.
    */
    SnyderProjection.sllra = function(ll, faceID) {
        // get the precomputed values for the face from the icosahedron
        var face = this.sphIcosa.getFace(faceID);
        var centre = face.sphTriCentre;

        // precompute some values
        var fCosLat = Math.cos(ll.lat);
        var fSinLat = Math.sin(ll.lat);
        var fDeltaLon = ll.lon - centre.point.lon;

        /*
        Calculate the great circle distance between the point and the triangle's
        centre point. Note for longitude differences < 90 degrees, it is not
        necessary to take the absolute value of the longitude difference.
        */
        // formula 13 from Snyder paper
        var fZ = Math.acos( (centre.sinLat * fSinLat) +
        (centre.cosLat * fCosLat * Math.cos(fDeltaLon)) );

        // sanity check, must be less than distance from centre to any vertex
        if (fZ > this.DH + this.PRECISION) {
            throw "Point is located on another face.";
        }

        /*
        Calculate the azimuth of the vector from the centre to the point relative
        to the triangle's azimuth.
        */
        // formula 14 from Snyder paper
        var fAZH = Math.atan2( fCosLat * Math.sin(ll.lon - centre.point.lon),
                centre.cosLat * fSinLat -
                centre.sinLat * fCosLat * Math.cos(fDeltaLon)) -
            face.azimuth;

        // convert to a value in the range [0, 360)
        if (fAZH < 0.0) {
            fAZH = fAZH + 2.0 * Math.PI;
        }

        /*
        Divide the triangle into three sub-triangles each formed by two vertices
        and the triangle's centre point. Rotate the azimuth so it falls in the
        range [0, 120).
        */
        var fRotate;
        if (fAZH < MathUtils.RAD_120) {
            fRotate = 0.0;
        } else if (fAZH <= MathUtils.RAD_240) {
            fRotate = MathUtils.RAD_120;
        } else if (fAZH > MathUtils.RAD_240) {
            fRotate = MathUtils.RAD_240;
        }

        fAZH -= fRotate;

        // precompute some more values
        var fSinAZH = Math.sin(fAZH);
        var fCosAZH = Math.cos(fAZH);

        /*
        Calculate the great circle distance from the centre of the face to the edge
        of the sub-triangle along the rotated azimuth.
        */
        var fDZ = Math.atan2(this.TAN_DH, fCosAZH + this.COT_30 * fSinAZH);

        // sanity check, distance from point to centre must be less than this
        if (fZ > fDZ + SnyderProjection.PRECISION) {
            throw "Point is located on another face.";
        }

        // formula 6 from Snyder paper
        var fH = Math.acos(fSinAZH * this.SIN_GH_COS_DH - fCosAZH * this.COS_GH);

        // calculate spherical excess
        var fAG = fAZH + this.GH + fH - MathUtils.RAD_180;

        // formula 8 from Snyder paper
        var fAZH1 = Math.atan2( 2.0 * fAG,
            this.R1_SQR * this.TAN_DH_SQR - 2.0 * fAG * this.COT_30 );

        // formulae 10, 11 from Snyder paper
        var fFH = this.TAN_DH / (2.0 * (Math.cos(fAZH1) + this.COT_30 *
            Math.sin(fAZH1)) * Math.sin(fDZ / 2.0));

        // formula 12 from Snyder paper
        var fPH = 2.0 * this.R1 * fFH * Math.sin(fZ / 2.0);

        // un-rotate the azimuth
        fAZH1 += fRotate;

        // measure angle CCW from base of triangle
        var fAngle = MathUtils.RAD_90 - fAZH1;

        return new PYXCoordPolar(fPH / this.EDGE_SCALE, fAngle);
    };

    /*!
    Project a point on an icosahedron face to a sphere. The point must be specified
    in polar coordinates relative to the centre of the face where each side of the
    face has a length of one unit and the angle is measured counter-clockwise from
    the base of the triangle.

    \param	ra		The point on the icosahedron face in polar coordinates.
    \param	faceID	The icosahedron face ["A"-"T"]

    \return The point on the sphere in lat/lon coordinates
    */
    SnyderProjection.projectToSphere = function(ra, faceID) {

        var ll = new CoordLatLon();

        var face = this.sphIcosa.getFace(faceID);
        var centre = face.sphTriCentre;

        if (MathUtils.equal(ra.radius, 0.0)) {
            // we are at the centre of the triangle, use the centre point
            ll = centre.point.copy();
        } else {
            /*
		    Formula 17 from Snyder paper. Convert to angle measured CW from top of
		    triangle.
		    */
            var fAZH1 = MathUtils.RAD_90 - ra.angle;

            // convert to value in the range [0, 360)
            if (fAZH1 < 0.0) {
                fAZH1 = fAZH1 + 2 * Math.PI;
            }

            /*
		    Divide the triangle into three sub-triangles each formed by two vertices
		    and the triangle's centre point. Rotate the azimuth so it falls in the
		    range [0, 120).
		    */
            var fRotate;
            if (fAZH1 <= MathUtils.RAD_120) {
                fRotate = 0.0;
            } else if (fAZH1 <= MathUtils.RAD_240) {
                fRotate = MathUtils.RAD_120;
            } else if (fAZH1 > MathUtils.RAD_240) {
                fRotate = MathUtils.RAD_240;
            }

            fAZH1 -= fRotate;
            var fAZH = fAZH1;

            // for a non-zero azimuth
            if (!MathUtils.equal(fAZH1, 0.0)) {
                // formula 19 from Snyder paper
                var agh = this.R1_SQR * this.TAN_DH_SQR /
                (2.0 * (1.0 / Math.tan(fAZH1) + this.COT_30));

                // iterate to determine azimuth
                var fDAZH = 1.0;
                while (!MathUtils.equal(fDAZH, 0.0)) {
                    var fH = Math.acos(Math.sin(fAZH) * this.SIN_GH_COS_DH - Math.cos(fAZH) * this.COS_GH);

                    // formula 20 from Snyder paper
                    var fFAZH = agh - fAZH - this.GH - fH + Math.PI;

                    // formula 21 from Snyder paper
                    var fFLAZH = ((Math.cos(fAZH) * this.SIN_GH_COS_DH + Math.sin(fAZH) * this.COS_GH) / Math.sin(fH)) - 1.0;

                    // formula 22 from Snyder paper
                    fDAZH = -fFAZH / fFLAZH;
                    fAZH = fAZH + fDAZH;
                }
            } else {
                fAZH = 0.0;
                fAZH1 = 0.0;
            }

            // formula 9 from Snyder paper
            var fDZ = Math.atan2(this.TAN_DH, Math.cos(fAZH) + this.COT_30 * Math.sin(fAZH));

            // formulae 10, 11 from Snyder paper
            var fFH = this.TAN_DH / (2.0 *
            (Math.cos(fAZH1) + this.COT_30 *
                Math.sin(fAZH1)) * Math.sin(fDZ / 2.0));

            // formula 18 from Snyder paper
            var fPH = ra.radius * this.EDGE_SCALE;

            // formula 23 from Snyder paper
            var fZ = 2.0 * Math.asin(fPH / (2.0 * this.R1 * fFH));

            // un-rotate the azimuth
            fAZH += fRotate;
            fAZH += face.azimuth;

            // convert to value in the range (-180, 180]
            while (fAZH <= -MathUtils.RAD_180) {
                fAZH += MathUtils.RAD_360;
            }

            while (fAZH > MathUtils.RAD_180) {
                fAZH -= MathUtils.RAD_360;
            }

            // calculate latitude
            var fSinLat = centre.sinLat * Math.cos(fZ) + centre.cosLat * Math.sin(fZ) * Math.cos(fAZH);

            fSinLat = Math.min(fSinLat, 1.0);
            fSinLat = Math.max(fSinLat, -1.0);

            ll.setLat(Math.asin(fSinLat));

            if (ll.isNorthPole() || ll.isSouthPole()) {
                ll.setLon(0.0);
            } else {
                // calculate longitude
                var fSinLon = Math.sin(fAZH) * Math.sin(fZ) / Math.cos(ll.lat);
                fSinLon = Math.min(fSinLon, 1.0);
                fSinLon = Math.max(fSinLon, -1.0);

                var fCosLon = (Math.cos(fZ) - centre.sinLat * Math.sin(ll.lat)) /
                    centre.cosLat / Math.cos(ll.lat);
                fCosLon = Math.min(fCosLon, 1.0);
                fCosLon = Math.max(fCosLon, -1.0);

                ll.setLon(centre.point.lon + Math.atan2(fSinLon, fCosLon));
            }
        }

        return ll;
    };

    /*!
    Convert a precision specified in arc radians to a resolution. This method
    returns the resolution with cell areas most closely fits the area of influence
    without exceeding it.

    \param	fPrecision	The precision in arc radians. Must be between 0 (exclusive)
				        and Icosahedron::CENTRAL_ANGLE (inclusive).

    \return	The resolution.
    */
    SnyderProjection.precisionToResolution = function(fPrecision) {
        // convert precision from a radius to a diameter
        fPrecision *= 2.0;

        if (!(0 < fPrecision && fPrecision <= Icosahedron.CENTRAL_ANGLE)) {
            throw "Invalid precision: " + fPrecision;
        }

        var nResolution = 0;
        var fAngle = Icosahedron.CENTRAL_ANGLE;

        while (fPrecision < fAngle) {
            fAngle /= MathUtils.SQRT_3;
            ++nResolution;
        }

        return nResolution;
    };

    /*!
    Convert a resolution to a precision specified in arc radians.

    \param	nResolution	The PYXIS resolution. Must be between 0 (inclusive)
	        and PYXMath::knMaxAbsResolution (inclusive).

    \return	The precision in arc radians.
    */
    SnyderProjection.resolutionToPrecision = function(nResolution) {

        if (!(0 <= nResolution && nResolution <= PYXMath.knMaxAbsResolution)) {
            throw "Invalid resolution: " + nResolution;
        }

        var fAngle = Icosahedron.CENTRAL_ANGLE;

        while (0 < nResolution) {
            fAngle /= MathUtils.SQRT_3;
            --nResolution;
        }

        // convert precision from a diameter to a radius
        fAngle /= 2.0;

        return fAngle;
    };

    return SnyderProjection;
};

module.exports = defineSnyderProjection();
