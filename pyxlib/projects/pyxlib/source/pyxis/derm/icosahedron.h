#ifndef PYXIS__DERM__ICOSAHEDRON_H
#define PYXIS__DERM__ICOSAHEDRON_H
/******************************************************************************
icosahedron.h

begin		: 2004-02-11
copyright	: derived from DGEllipsoidRF by Kevin Sahr
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/coord_lat_lon.h"
#include "pyxis/utility/coord_3d.h"

// standard includes
#include <vector>

/*!
Icosahedron represents an icosahedron oriented to a sphere. The orientation is
defined by specifying the location of vertex 0 in lat/lon coordinates and the
longitude of vertex 0 to vertex 1.
*/
//! Represents an icosahedron oriented on a sphere.
class PYXLIB_DECL Icosahedron
{
public:

	//! Test method
	static void test();

	//! The number of vertices for the icosahedron.
	static const int knNumVertices;

	//! The number of faces for the icosahedron.
	static const int knNumFaces;

	//! Get the length of a side of a unit icosahedron.
	static double kfSideLength;

	//! The inradius of a unit icosahedron.
	static double fInRadius;

	/*!
	The central angle of the icosahedron. The central angle is the angle
	formed at the centre of the icosahedron between two lines going from the
	centre to adjacent vertices.
	*/
	static double kfCentralAngle;

	/*! 
	Constructs an Icosahedron that is
	symmetrical with the equator and has only one vertex falling on land.
	See http://www.sou.edu/cs/sahr/dgg/orientation/stdorient.html for details.
	*/
	Icosahedron();

	//! Constructs the icosahedron
	Icosahedron(const CoordLatLon& vertex0, double fAzimuth);

	//! Destructor
	virtual ~Icosahedron() {}

	//! Vector of lat/lon coordinates
	typedef std::vector<CoordLatLon> CoordLatLonVector;

	//! Get the vector of vertices
	const CoordLatLonVector& getVertices() const {return m_vecIcoVertices;}

	//! Determine the icosahedron face in which the point lies.
	int findFace(const CoordLatLon& ll) const;

	//! Represents a face on the icosahedron
	class Face
	{
	public:

		//! Default constructor
		Face() : m_vecVertex(3), m_fAzimuth(0.0) {;}

		//! Set the vertices
		void setVertices(	const CoordLatLon& vertex0,
							const CoordLatLon& vertex1,
							const CoordLatLon& vertex2	);

		//! Get the centre of the spherical triangle formed by the vertices
		inline const PreCompLatLon& sphTriCentre() const {return m_sphTriCentre;}

		//! Set the azimuth of the face in radians (direction from centre to vertex 0)
		void setAzimuth(double fAzimuth) {m_fAzimuth = fAzimuth;}

		//! Get the azimuth of the face in radians (direction from centre to vertex 0)
		inline double azimuth() const {return m_fAzimuth;}

		//! Determine if a point lies inside this face
		bool inside(const PYXCoord3DDouble& xyz) const;

	private:
		
		//! Location of the vertices in lat/lon coordinates.
		CoordLatLonVector m_vecVertex;

		//! The centre of the spherical triangle formed by the vertices
		PreCompLatLon m_sphTriCentre;

		//! The azimuth of the face in radians (direction from centre to vertex 0)
		double m_fAzimuth;

		//! Vector from opposite edge to vertex 0
		PYXCoord3DDouble m_v0;

		//! Vector from opposite edge to vertex 1
		PYXCoord3DDouble m_v1;

		//! Vector from opposite edge to vertex 2
		PYXCoord3DDouble m_v2;
	};

	//! Vector of faces
	typedef std::vector<Face> FaceVector;

	//! Get the vector of faces
	const FaceVector& getFaces() const {return m_vecFaces;}

	//! Get the information for a given face
	const Face& getFace(int nIndex) const;

private:

	//! Disable copy constructor
	Icosahedron(const Icosahedron&);

	//! Disable copy assignment
	void operator=(const Icosahedron&);

	//! Orient the icosahedron.
	void orientToSphere(const CoordLatLon& vertex0, double fAzimuth);

	//! Calculate the new coordinates of any point in the original coordinate system.
	CoordLatLon coordTrans(	const CoordLatLon& newNPold,
							const CoordLatLon& ptOld, 
							double fLon0	);
	
	//! Initializes the Static Data.
	static void initStaticData();

	//! Frees the Static Data.
	static void freeStaticData() {}

	//! Location of the vertices in lat/lon coordinates.
	CoordLatLonVector m_vecIcoVertices;

	//! Information about each face
	FaceVector m_vecFaces;

	//! Allows PYXLibInstance to initialize the static data.
	friend class PYXLibInstance;
};

#endif // guard
