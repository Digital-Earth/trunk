/******************************************************************************
icosahedron.js

begin		: 2015-12-14
copyright	: derived from DGEllipsoidRF by Kevin Sahr
web			: www.pyxisinnovation.com
******************************************************************************/

var MathUtils = require('./math_utils');
var PYXIcosMath = require('./index_math');
var CoordLatLon = require('./coord_lat_lon');
var PYXCoord3D = require('./coord_3d');
var SphereMath = require('./sphere_math');



/*!
Moved from coord_lat_lon to improve module structure

Set the point and calculate the precomputed values.

\param  point   The point in lat/lon coordinates.
*/
function PreCompLatLon(point) {
    this.point = point;
    this.sinLat = Math.sin(this.point.lat);
    this.sinLon = Math.sin(this.point.lon);
    this.cosLat = Math.cos(this.point.lat);
    this.cosLon = Math.cos(this.point.lon);
};



/*
Elevation to vertices 1-5 when vertex 0 is at the north pole. Approximately
equal to 26.565051177077989 degrees.
*/
Icosahedron.VERTEX_ELEVATION = Math.asin(1.0 / Math.sqrt(5.0));

//! The number of vertices for the icosahedron.
Icosahedron.NUM_VERTICES = 12;

//! The number of faces for the icosahedron.
Icosahedron.NUM_FACES = 20;

/*! 
Constructs an Icosahedron that is
symmetrical with the equator and has only one vertex falling on land.
See http://www.sou.edu/cs/sahr/dgg/orientation/stdorient.html for details.
*/
function Icosahedron() {

    //! Get the length of a side of a unit icosahedron.
    this.SIDE_LENGTH = 2.0 / Math.sqrt(MathUtils.PHI * MathUtils.PHI + 1.0); 

    //! The inradius of a unit icosahedron.
    this.IN_RADIUS = (3.0 * MathUtils.SQRT_3 + Math.sqrt(15.0)) / 12.0 * this.SIDE_LENGTH;;

    //! The central angle of the icosahedron.
    this.CENTRAL_ANGLE = 2.0 * Math.asin(this.SIDE_LENGTH / 2.0);

    //! Location of the vertices in lat/lon coordinates.
    this.icosVertices = new Array(Icosahedron.NUM_VERTICES);

    //! Information about each of the faces.
    this.faces = new Array(Icosahedron.NUM_FACES);

    this.vertex0 = new CoordLatLon();
    this.vertex0.setInDegrees(58.28252559, 11.25);

    this.orientToSphere(this.vertex0, 0);
};

/*!
Fill in the icosahedron particulars given one point and one edge's azimuth.

\param vertex0	Vertex 0 specified in lat/lon coordinates.
\param fAzimuth	The azimuth from vertex 0 to vertex 1 in degrees.	
*/
Icosahedron.prototype.orientToSphere = function(vertex0, fAzimuth) {
    /*
    Calculate the lat/lon coordinates for each vertex assuming vertex 0 is at
    the North Pole.
    */
    var vertices = new Array(Icosahedron.NUM_VERTICES);

    for (var nVertex = 1; nVertex <= 5; nVertex++) {
        var coord1 = new CoordLatLon();
        coord1.setLat(Icosahedron.VERTEX_ELEVATION);
        coord1.setLonInDegrees(fAzimuth + 72.0 * (nVertex - 1));
        vertices[nVertex] = coord1;

        var coord2 = new CoordLatLon();
        coord2.setLat(-Icosahedron.VERTEX_ELEVATION);
        coord2.setLonInDegrees(fAzimuth + 36.0 + 72.0 * (nVertex - 1));
        vertices[nVertex + 5] = coord2;
    }

    var coord3 = new CoordLatLon();
    coord3.setLatInDegrees(-90.0);
    coord3.setLon(0.0);
    vertices[11] = coord3;

    // transform the vertex coordinates for actual vertex 0 location
    var newNPold = new CoordLatLon(vertex0.lat, 0.0);

    this.icosVertices[0] = vertex0;
    for (nVertex = 1; nVertex < Icosahedron.NUM_VERTICES; nVertex++) {
        this.icosVertices[nVertex] = this.coordTrans( newNPold,
            vertices[nVertex],
            vertex0.lon );
    }

    // set the vertices for each face
    for (var nFace = 0; nFace < Icosahedron.NUM_FACES; nFace++) {
        var v = PYXIcosMath.getFaceVertices(String.fromCharCode(nFace + "A".charCodeAt(0)));

        this.faces[nFace] = new Icosahedron.Face(
            this.icosVertices[v[0] - 1],
            this.icosVertices[v[1] - 1],
            this.icosVertices[v[2] - 1] );
    }
};

/*!
Calculate the new coordinates of any point defined in the original coordinate
system. Define a point (newNPold) in the original coordinate system as the
North Pole in the new coordinate system, and the great circle connecting the
original and new North Pole as the fLon0 longitude in new coordinate system.

\param	newNPold	Point in the old coordinate system defined as North Pole in
					the new coordinate system.
\param	ptOld		The point to convert.
\param	fLon0		The longitude of the great circle connecting the original
					and new North Poles.

\return	The point in the new coordinate system.
*/
Icosahedron.prototype.coordTrans = function(newNPold, ptOld, fLon0) {
    var fLat;
    var fLon;

    if (newNPold.isNorthPole()) {
        fLat = ptOld.lat;
        fLon = ptOld.lon;
    } else {
        // calculate the latitude of the new point in the range [0, 180]
        var fCosLat =
            Math.sin(newNPold.lat) * Math.sin(ptOld.lat) +
            Math.cos(newNPold.lat) * Math.cos(ptOld.lat) *
            Math.cos(newNPold.lon - ptOld.lon);

        fCosLat = Math.min(fCosLat, 1.0);
        fCosLat = Math.max(fCosLat, -1.0);

        fLat = Math.acos(fCosLat);

        // calculate the longitude of the new point in the range [-180, 180)
        if (MathUtils.equal(fLat, 0.0) || MathUtils.equal(fLat, Math.PI)) {
            fLon = 0;
        } else {
            var fCosLon =
                (Math.sin(ptOld.lat) * Math.cos(newNPold.lat) -
                Math.cos(ptOld.lat) * Math.sin(newNPold.lat) *
                Math.cos(newNPold.lon - ptOld.lon)) / Math.sin(fLat);

            fCosLon = Math.min(fCosLon, 1.0);
            fCosLon = Math.max(fCosLon, -1.0);

            if (
                ((ptOld.lon - newNPold.lon) >= 0.0) &&
                ((ptOld.lon - newNPold.lon) < Math.PI)) {
                fLon = fLon0 - Math.acos(fCosLon);
            } else {
                fLon = fLon0 + Math.acos(fCosLon);
            }
        }

        // convert latitude to a value in the range [-90, 90]
        fLat = Math.PI / 2.0 - fLat;
    }

    return new CoordLatLon(fLat, fLon);
};
        
/*!
Determine the icosahedron face in which the point lies.

\param	ll	The point in lat/lon coordinates

\return	The face ["A"-"T"]
*/
Icosahedron.prototype.findFace = function(ll) {
    // convert the point to xyz coordinates
    var xyz = SphereMath.llxyz(ll);

    for (var nFace = 0; nFace < Icosahedron.NUM_FACES; ++nFace) {
        if (this.faces[nFace].inside(xyz)) {
            return String.fromCharCode(nFace + "A".charCodeAt(0));
        }
    }

    throw "No face found for lat: " + ll.latInDegrees() + " lon: " + ll.lonInDegrees();
};

/*!
Get the information for a given face of the icosahedron.

\param	faceID	The face identifier ["A"-"T"]

\return	The face information.
*/
Icosahedron.prototype.getFace = function (faceID) {
    if (typeof faceID !== "string" || faceID.length !== 1 || faceID < "A" || faceID > "T") {
        throw "Invalid face: " + faceID;
    }

    return this.faces[faceID.charCodeAt(0) - "A".charCodeAt(0)];
};

/*!
Create a face. Vertices must be specified in counter-clockwise
order. This method also initializes the precomputed fields.

\param	vertex0	The first vertex in lat/lon coordinates.
\param	vertex1	The second vertex in lat/lon coordinates.
\param	vertex2	The third vertex in lat/lon coordinates.
*/
Icosahedron.Face = function(vertex0, vertex1, vertex2) {
    this.vertices = [vertex0, vertex1, vertex2];

    /*
	Calculate the centre of the spherical triangle formed by the vertices.
	*/

    // convert vertices to xyz coordinates
    var xyzCoords = new Array();
    for (var nVertex = 0; nVertex < 3; nVertex++) {
        xyzCoords.push(SphereMath.llxyz(this.vertices[nVertex]));
    }

    // calculate the centroid in xyz coordinates
    var xyz = new PYXCoord3D();
    xyz.set((xyzCoords[0].x + xyzCoords[1].x + xyzCoords[2].x) / 3.0,
    (xyzCoords[0].y + xyzCoords[1].y + xyzCoords[2].y) / 3.0,
    (xyzCoords[0].z + xyzCoords[1].z + xyzCoords[2].z) / 3.0 );

    // put centroid on the unit sphere
    xyz.normalize();

    // convert back to lat/lon coordinates
    this.sphTriCentre = new PreCompLatLon(SphereMath.xyzll(xyz));

    /*
	Pre-calculate the azimuth of the vector from the triangle's centre point to
	vertex 0. See http://williams.best.vwh.net/avform.htm, and search for
	"Course between points" for a reference to the formula.
	*/
    this.azimuth = Math.atan2( Math.cos(this.vertices[0].lat) * Math.sin(this.vertices[0].lon -
        this.sphTriCentre.point.lon),
        this.sphTriCentre.cosLat * Math.sin(this.vertices[0].lat) -
        this.sphTriCentre.sinLat * Math.cos(this.vertices[0].lat) *
        Math.cos(this.vertices[0].lon - this.sphTriCentre.point.lon) );

    /*
	Calculate three vectors pointing from each side of the triangle to the
	opposite vertex. These are used in the calculation for determining if
	a point lies inside the triangle.
	*/
    this.v0 = xyzCoords[1].cross(xyzCoords[2]);
    this.v1 = xyzCoords[2].cross(xyzCoords[0]);
    this.v2 = xyzCoords[0].cross(xyzCoords[1]);
};

/*!
Determine if a point lies inside this face (inclusive).

\param	xyz	The point in xyz coordinates.

\return	true if the point lies inside this face, otherwise false
*/
Icosahedron.Face.prototype.inside = function(xyz) {
    /*
	In a triangle, the line containing each edge divides space into two halves
	- the half containing the triangle and the half without the triangle. If a
	point lies on the "triangle" side of all edges then the point lies inside
	the triangle.
	
	To check whether or not a point is on the "triangle" side of an edge,
	calculate the projection of the point's vector onto a vector that points
	from the edge to the opposite vertex. If the projection is positive then
	the point lies on the "triangle" side of the edge.
	*/

    var bInside = false;
    if (0.0 <= xyz.dot(this.v0)) {
        if (0.0 <= xyz.dot(this.v1)) {
            if (0.0 <= xyz.dot(this.v2)) {
                bInside = true;
            }
        }
    }

    return bInside;
};

module.exports = Icosahedron;
