/******************************************************************************
rhombus_math.cpp

begin		: 2016-01-17
copyright	: (C) 2016 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

/*!
The PYXRhombusMath class defines various mathematical operations on rhombuses. 
Any function that directly alters or manipulates a PYXRhombusIndex will
be done here. The current areas covered by the PYXRhombusMath class include:

\verbatim
    Conversion
        To/from polar coordinates
\endverbatim

The main rhombuses correspond to pairs of icosahedron faces and are numbered
as follows when the icosahedron is unfolded:

\verbatim
   /\  /\  /\  /\  /\
  / 0\/ 1\/ 2\/ 3\/ 4\
  \  /\  /\  /\  /\  /\
   \/  \/  \/  \/  \/  \
    \ 5/\ 6/\ 7/\ 8/\ 9/
     \/  \/  \/  \/  \/
\endverbatim

Points within the rhombus are indexed by u, v coordinates in units that
correspond to the length of a side of a rhombus. For rhombuses 0-4, the origin
is located at the top of the rhombus with u pointing down along the left side
and v pointing down along the right. For rhombuses 5-9 the origin is located at
the bottom of the rhombus with u pointing up along the right side and v
pointing up along the left.

\verbatim
     o
   // \\          / \
  //   \\      v /   \ u
 u \   / v      \\   //
    \ /          \\ //
                   o
Rhombuses 0-4  Rhombuses 5-9
\endverbatim

*/

'use strict';

var MathUtils = require('./math_utils');
var PYXCoordPolar = require('./coord_polar');
var SnyderProjection = require('./snyder_projection');
var CoordLatLon = require('./coord_lat_lon');
var WGS84 = require('./wgs84');

function definePYXRhombusMath() {

    //! Singleton instance
    var PYXRhombusMath = {};

    // The distance from the nearest icosahedron face's centre to the rhombus uv origin
    var d1 = 2 / 3 * MathUtils.COS_30;

    // The distance from the furthest icosahedron face's centre to the rhombus uv origin
    var d2 = 4 / 3 * MathUtils.COS_30;

    // The distance between the centre of

    // Converts an icosahedron face to a main rhombus index.
    /*
    This connectivity chart associates icosahedron faces with rhombuses.
    For each icosahedron face, the first value is the main rhombus for the
    face. (Icosahedron faces are documented in index_math.js). The second
    value is the distance required to translate the face origin to the
    rhombus uv origin.
    */
    PYXRhombusMath.FACE_TO_RHOMBUS = [
        // Face A (65)
        ['0', d1],
        // Face B (66)
        ['1', d1],
        // Face C (67)
        ['2', d1],
        // Face D (68)
        ['3', d1],
        // Face E (69)
        ['4', d1],
        // Face F (70)
        ['0', -d2],
        // Face G (71)
        ['1', -d2],
        // Face H (72)
        ['2', -d2],
        // Face I (73)
        ['3', -d2],
        // Face J (74)
        ['4', -d2],
        // Face K (75)
        ['6', -d2],
        // Face L (76)
        ['5', -d2],
        // Face M (77)
        ['9', -d2],
        // Face N (78)
        ['8', -d2],
        // Face O (79)
        ['7', -d2],
        // Face P (80)
        ['6', d1],
        // Face Q (81)
        ['5', d1],
        // Face R (82)
        ['9', d1],
        // Face S (83)
        ['8', d1],
        // Face T (84)
        ['7', d1]
    ];

    // Converts a rhombus index to an icosahedron face.
    /*
    This connectivity chart associates rhombuses with icosahedron faces.
    For each rhombus, the first value is the icosahedron face nearest to
    the uv origin and the second value is the icosahedron face furthest
    from the uv origin. (Icosahedron faces are documented in
    index_math.js).
    */
    PYXRhombusMath.RHOMBUS_TO_FACE = [
        // Rhombus 0
        ["A", "F"],
        // Rhombus 1
        ["B", "G"],
        // Rhombus 2
        ["C", "H"],
        // Rhombus 3
        ["D", "I"],
        // Rhombus 4
        ["E", "J"],
        // Rhombus 5
        ["Q", "L"],
        // Rhombus 6
        ["P", "K"],
        // Rhombus 7
        ["T", "O"],
        // Rhombus 8
        ["S", "N"],
        // Rhombus 9
        ["R", "M"]
    ];

    PYXRhombusMath.testResolution = function() 
    {
        var r = {rhombus: "0123456781234", u: 0.5, v: 0.5};
        var r2 = PYXRhombusMath.decreaseRhombusResolution(r);
        while(r2.rhombus.length>1) {
            r2 = PYXRhombusMath.decreaseRhombusResolution(r2);
        }
        while(r2.rhombus.length<r.rhombus.length) {
            r2 = PYXRhombusMath.increaseRhombusResolution(r2);
        }
        return r.rhombus == r2.rhombus;
    }

    function TEST_ASSERT(isTure) {
        if (!isTure) {
            throw "Test Failed"
        }
    } 

    //! Test method
    PYXRhombusMath.test = function()
    {
        // the best precision we can expect through a forward and reverse conversion
        var kfDefaultPrecision = 5.0e-8;

        // test conversion both ways
        for (var rhombus = 0; rhombus < 10; ++rhombus) {
            for (var u = 0; u <= 1; u += 0.1) {
                for (var v = 0; v <= 1; v += 0.1) {
                    var result = PYXRhombusMath.rhombusUVToFacePolar(''+rhombus, u, v);
                    var result2 = PYXRhombusMath.facePolarToRhombusUV(result.face, result.pt);

                    TEST_ASSERT(result2.rhombus === ''+rhombus);
                    TEST_ASSERT(MathUtils.equal(result2.u, u, kfDefaultPrecision));
                    TEST_ASSERT(MathUtils.equal(result2.v, v, kfDefaultPrecision));
                }
            }
        }
        
        // test conversion both ways
        var ll = new CoordLatLon();
        for (var fLat = -80.0; fLat <= 80.0; fLat += 10) {
            for (var fLon = -180.0; fLon < 180.0; fLon += 10) {
                ll.setInDegrees(fLat, fLon);
                var result = PYXRhombusMath.wgs84LatLonToRhombusUV(ll);
                var result2 = PYXRhombusMath.rhombusUVToWgs84LatLon(result.rhombus, result.u, result.v);

                TEST_ASSERT(result2.equal(ll, kfDefaultPrecision));
            }
        }
    };

    /*!
    Convert WGS84 lat/lon coordinates to rhombus and uv coordinates.

    \param ll   The WGS84 lat/lon coordinates

    \return	The rhombus and uv coordinates.
    */
    PYXRhombusMath.wgs84LatLonToRhombusUV = function (ll) {
        var geocentricll = WGS84.toGeocentric(ll);
        var result = SnyderProjection.projectToFace(geocentricll);
        return PYXRhombusMath.facePolarToRhombusUV(result.face, result.polar);
    };

    /*!
    Convert geocentric lat/lon coordinates to rhombus and uv coordinates.

    \param ll   The geocentric lat/lon coordinates

    \return	The rhombus and uv coordinates.
    */
    PYXRhombusMath.geocentricLatLonToRhombusUV = function(ll) {
        var result = SnyderProjection.projectToFace(ll);
        return PYXRhombusMath.facePolarToRhombusUV(result.face, result.polar);
    };

    /*!
    Convert a point represented as a rhombus and uv coordinates to
    WGS84 lat/lon coordinates. The uv coordinate are in the range
    0..1 where 1 represents the length of a side of the icosahedron.

    \param rhombus	The rhombus ['0'-'9']
    \param u        The u coordinate
    \param v        The v coordinate

    \return The WGS84 lat/lon coordinates
    */
    PYXRhombusMath.rhombusUVToWgs84LatLon = function (rhombus, u, v) {
        var result = PYXRhombusMath.rhombusUVToFacePolar(rhombus, u, v);
        var geocentricll = SnyderProjection.projectToSphere(result.pt, result.face);
        return WGS84.toDatum(geocentricll);
    };

    /*!
    Convert a point represented as a rhombus and uv coordinates to
    geocentric lat/lon coordinates. The uv coordinate are in the range
    0..1 where 1 represents the length of a side of the icosahedron.

    \param rhombus	The rhombus ['0'-'9']
    \param u        The u coordinate
    \param v        The v coordinate

    \return The geocentric lat/lon coordinates
    */
    PYXRhombusMath.rhombusUVToGeocentricLatLon = function (rhombus, u, v) {
        var result = PYXRhombusMath.rhombusUVToFacePolar(rhombus, u, v);
        return SnyderProjection.projectToSphere(result.pt, result.face);
    };

    /*!
    Convert a point represented by an icosahedron face and polar coordinates
    to a rhombus and UV coordinates. For the polar coordinates, the angle
    is measured counter-clockwise relative to the base of the face and the
    radius is specified in inter-cell units at resolution zero, which also
    corresponds to the length of a side of the icosahedron.

    \param face The icosahedron face ["A"-"T"]
    \param pt   The polar coordinate

    \return	The rhombus and uv coordinates.
    */
    PYXRhombusMath.facePolarToRhombusUV = function(face, pt) {

        if (typeof face !== "string" || face.length !== 1 || face < "A" || face > "T") {
            throw "Invalid face: " + face;
        }

        // get the main rhombus corresponding to the icosahedron face
        var idx = face.charCodeAt(0) - "A".charCodeAt(0);
        var rhombus = PYXRhombusMath.FACE_TO_RHOMBUS[idx][0];

        // convert the polar coordinates to xy coordinates
        // with origin at the centre of the icosahedron face
        var x = pt.radius * Math.cos(pt.angle);
        var y = pt.radius * Math.sin(pt.angle);

        // translate the xy origin to the UV origin
        var translate = PYXRhombusMath.FACE_TO_RHOMBUS[idx][1];
        y -= translate;

        // calculate the uv coordinates
        // derived from: http://www.geom.uiuc.edu/docs/reference/CRC-formulas/node7.html
        x /= MathUtils.SIN_30;
        y /= MathUtils.SIN_60;

        var u, v;
        if (translate > 0) {
            // face is closest to the uv origin
            u = (-x - y) / 2;
            v = (x - y) / 2;
        } else {
            // face is furthest from the uv origin
            u = (x + y) / 2;
            v = (-x + y) / 2;
        }

        return {
            rhombus: rhombus,
            u: u,
            v: v
        };
    };
    
    /*!
    Convert a point represented as a rhombus and uv coordinates to an
    icosahedron and polar coordinates. The uv coordinate are in the range
    0..1 where 1 represents the length of a side of the icosahedron.

    \param rhombus	The rhombus ['0'-'9']
    \param u        The u coordinate
    \param v        The v coordinate

    \return The icosahedron face and polar coordinates.
    */
    PYXRhombusMath.rhombusUVToFacePolar = function(rhombus, u, v) {
        if (typeof rhombus !== "string" || rhombus < '0' || rhombus > '9') {
            throw "Invalid rhombus: " + rhombus;
        }

        if (typeof u !== "number" || u < 0 || u > 1) {
            throw "Invalid u coordinate: " + u;
        }

        if (typeof v !== "number" || v < 0 || v > 1) {
            throw "Invalid v coordinate: " + v;
        }

        // get the icosahedron face corresponding to this rhombus uv
        var face;
        if (u + v <= 1) {
            // face is closest to the uv origin
            face = PYXRhombusMath.RHOMBUS_TO_FACE[rhombus][0];
        } else {
            // face is furthest from the uv origin
            face = PYXRhombusMath.RHOMBUS_TO_FACE[rhombus][1];
        }

        // get the main rhombus corresponding to the icosahedron face
        var idx = face.charCodeAt(0) - "A".charCodeAt(0);

        // calculate the xy components
        // derived from: http://www.geom.uiuc.edu/docs/reference/CRC-formulas/node7.html
        var translate = PYXRhombusMath.FACE_TO_RHOMBUS[idx][1];

        var x, y;
        if (translate > 0) {
            // face is closest to the uv origin
            x = (-u + v) * MathUtils.SIN_30;
            y = (-u - v) * MathUtils.SIN_60;
        } else {
            // face is furthest from the uv origin
            x = (u - v) * MathUtils.SIN_30;
            y = (u + v) * MathUtils.SIN_60;
        }

        // translate the origin to the centre of the face
        y += translate;

        // convert the xy coordinates to polar coordinates
        var angle = Math.atan2(y, x);
        var radius = Math.sqrt((x * x) + (y * y));

        return {
            face: face,
            pt: new PYXCoordPolar(radius, angle)
        };
    };
    
    PYXRhombusMath.rhombusKeyToUvTransform = function(key)
    {
        var uvTransform = {
            uOffset: 0.0,
            vOffset: 0.0,
            uScale: 1.0,
            vScale: 1.0,
            u: function(value) { return value*this.uScale + this.uOffset; },
            v: function(value) { return value*this.vScale + this.vOffset; },
            inverseU: function(value) { return (value-this.uOffset)/this.uScale; },
            inverseV: function(value) { return (value-this.vOffset)/this.vScale; },
        };

        for(var i=0;i<key.length;i++) {
            var digit = parseInt(key[i]);

            uvTransform.uScale /= 3;
            uvTransform.vScale /= 3;
            uvTransform.uOffset += Math.floor(digit % 3) * uvTransform.uScale;
            uvTransform.vOffset += Math.floor(digit / 3) * uvTransform.vScale;
        }

        return uvTransform;
    }
    
    /*!
    increase the resolution of a rhombus UV coordinate.
    
    \param ruv          The rhombusUv coordinate { rhombus: key, u: float, v: float}
    \param resolutions  (optional) number of resolution to increase

    \return rhombusUv coordinates of the same location as higher rhombus index.
    
    exampe:
    increaseRhombusResolution({ rhombus:"02",u:0.1,v:0.5}) -> { rhombus:"023",u:0.3,v:0.5}
    */
    PYXRhombusMath.increaseRhombusResolution = function(ruv,resolutions)
    {
        resolutions = resolutions || 1;
        
        var result = {
            rhombus: ruv.rhombus + '',
            u: ruv.u,
            v: ruv.v
        };
        
        while(resolutions>0) {
            var intU = Math.min(Math.floor(result.u*3),2);
            var intV = Math.min(Math.floor(result.v*3),2);
            
            result.rhombus += '' + (intV*3+intU);
            result.u = result.u*3-intU;
            result.v = result.v*3-intV;
            
            resolutions--;
        }
        
        return result;
    }

    /*!
    decrease the resolution of a rhombus UV coordinate.
    
    \param ruv      The rhombusUv coordinate { rhombus: key, u: float, v: float}

    \return rhombusUv coordinates of the same location as lower rhombus index.
    
    exampe:
    decreaseRhombusResolution({ rhombus:"023",u:0.3,v:0.5}) -> { rhombus:"02",u:0.1,v:0.5}
    */
    PYXRhombusMath.decreaseRhombusResolution = function(ruv)
    {
        if (ruv.rhombus.length < 2) {
            throw "can decrease resolution of to level rhombus: " + ruv.rhombus;
        }
        var lastDigit = parseInt(ruv.rhombus[ruv.rhombus.length-1]);
        
        return {
            rhombus: ruv.rhombus.substr(0,ruv.rhombus.length-1),
            u: ( (lastDigit % 3) + ruv.u) / 3,
            v: ( Math.floor(lastDigit / 3) + ruv.v) / 3
        };
    }

    return PYXRhombusMath;
};

module.exports = definePYXRhombusMath();