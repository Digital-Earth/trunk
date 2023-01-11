/******************************************************************************
index_math.cpp

begin		: 2015-12-17
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

/*!
The PYXIcosMath class defines various mathematical operations on PYXISIcosIndex 
objects. Any function that directly alters or manipulates a PYXIcosIndex will
be done here. The current areas covered by the PYXIcosMath class include:

/verbatim
	Tesseral Arithmetic
		Addition, Subtraction, move etc.
	Tesseral geometry
		Resolution 1 connectivity
		Resolution 2 connectivity
		Missing directions for pentagons

/endverbatim

Resolution 0 is represented by pentagons that are each centred on the 12 
vertices.  Resolution 1 is made up of hexagons and pentagons and
forms the 3D shape of a truncated icosahedron (soccer ball).  The remaining
resolutions are made up entirely of hexagons that are tesselated down from 
the first generation pentagons at the vertices.

When the icosahedron is assembled into its 3D shape it has 12 unique
vertices that are represented as follows when the icosahedron is unfolded:

\verbatim
    1   1   1   1   1
   /\  /\  /\  /\  /\
  /  \/  \/  \/  \/  \
 2----3---4---5---6---2
  \  /\  /\  /\  /\  /\
   \/  \/  \/  \/  \/  \
    7---8---9---10--11--7
    \  /\  /\  /\  /\  /
     \/  \/  \/  \/  \/
     12  12  12  12  12
\endverbatim

The names of the faces can be represented as follows when the icosahedron
is unfolded:
\verbatim

 /\  /\  /\  /\  /\
/A \/B \/C \/D \/E \
--------------------
\F /\G /\H /\I /\J /\
 \/K \/L \/M \/N \/O \
  --------------------
  \P /\Q /\R /\S /\T /
   \/  \/  \/  \/  \/
\endverbatim

*/

'use strict';

function definePYXIcosMath() {

    //! Singleton instance
    var PYXIcosMath = {};

    // Connectivity of resolution 1 hexagons on faces
    /*
    This connectivity chart provides the resolution 1 hexagon that is the result of
    moving from a resolution 1 face pentagon in a given direction. The first index
    into the array is the face value (A=0, B=1...), the second index is the
    direction number. The third index determines the type of information being
    requested:

        0	The destination resolution 1 hexagon face or pentagon vertex
        1	The counter-clockwise rotation required to correct the movement. 
    */
    PYXIcosMath.RES1_FACE_CONNECT = [
        // Face A (65)
        [[1, 4], [69, 1], [2, 0], [70, 0], [3, 0], [66, 5]],
        // Face B (66)
        [[1, 5], [65, 1], [3, 0], [71, 0], [4, 0], [67, 5]],
        // Face C (67)
        [[1, 0], [66, 1], [4, 0], [72, 0], [5, 0], [68, 5]],
        // Face D (68)
        [[1, 1], [67, 1], [5, 0], [73, 0], [6, 0], [69, 5]],
        // Face E (69)
        [[1, 2], [68, 1], [6, 0], [74, 0], [2, 0], [65, 5]],
        // Face F (70)
        [[65, 0], [2, 0], [79, 0], [7, 0], [75, 0], [3, 0]],
        // Face G (71)
        [[66, 0], [3, 0], [75, 0], [8, 0], [76, 0], [4, 0]],
        // Face H (72)
        [[67, 0], [4, 0], [76, 0], [9, 0], [77, 0], [5, 0]],
        // Face I (73)
        [[68, 0], [5, 0], [77, 0], [10, 0], [78, 0], [6, 0]],
        // Face J (74)
        [[69, 0], [6, 0], [78, 0], [11, 0], [79, 0], [2, 0]],
        // Face K (75)
        [[3, 0], [70, 0], [7, 0], [80, 0], [8, 0], [71, 0]],
        // Face L (76)
        [[4, 0], [71, 0], [8, 0], [81, 0], [9, 0], [72, 0]],
        // Face M (77)
        [[5, 0], [72, 0], [9, 0], [82, 0], [10, 0], [73, 0]],
        // Face N (78)
        [[6, 0], [73, 0], [10, 0], [83, 0], [11, 0], [74, 0]],
        // Face O (79)
        [[2, 0], [74, 0], [11, 0], [84, 0], [7, 0], [70, 0]],
        // Face P (80)
        [[75, 0], [7, 0], [84, 5], [12, 1], [81, 1], [8, 0]],
        // Face Q (81)
        [[76, 0], [8, 0], [80, 5], [12, 0], [82, 1], [9, 0]],
        // Face R (82)
        [[77, 0], [9, 0], [81, 5], [12, 5], [83, 1], [10, 0]],
        // Face S (83)
        [[78, 0], [10, 0], [82, 5], [12, 4], [84, 1], [11, 0]],
        // Face T (84)
        [[79, 0], [11, 0], [83, 5], [12, 2], [80, 1], [7, 0]]
    ];

    /*!
    Get the vertex indices for a given face. Faces are oriented so they point up
    (towards direction 1) or down (towards direction 4). The vertices are returned
    in counter-clockwise order starting with the vertex that points either up or
    down.

    \param face    The face ["A"-"T"]

    \return Array of 3 vertices.
    */
    PYXIcosMath.getFaceVertices = function (face) {
        if (typeof face !== "string" || face.length !== 1 || face < "A" || face > "T") {
            throw "Invalid face: " + face;
        }

        var idx = face.charCodeAt(0) - "A".charCodeAt(0);

        var vertices = [];
        if ("A".charCodeAt(0) > PYXIcosMath.RES1_FACE_CONNECT[idx][0][0]) {
            /*
	        Direction 1 is a vertex, so the triangle points up. Get the vertices at
	        directions 1, 3 and 5.
	        */
            vertices[0] = PYXIcosMath.RES1_FACE_CONNECT[idx][0][0];
            vertices[1] = PYXIcosMath.RES1_FACE_CONNECT[idx][2][0];
            vertices[2] = PYXIcosMath.RES1_FACE_CONNECT[idx][4][0];
        } else {
            /*
	        Direction 1 is a face, so the triangle points down. Get the vertices at
	        directions 4, 6 and 2.
	        */
            vertices[0] = PYXIcosMath.RES1_FACE_CONNECT[idx][3][0];
            vertices[1] = PYXIcosMath.RES1_FACE_CONNECT[idx][5][0];
            vertices[2] = PYXIcosMath.RES1_FACE_CONNECT[idx][1][0];
        }

        return vertices;
    };

    return PYXIcosMath;
};

module.exports = definePYXIcosMath();


