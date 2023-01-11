/******************************************************************************
icosahedron.cpp

begin		: 2004-02-11
copyright	: derived from DGEllipsoidRF by Kevin Sahr
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/icosahedron.h"

// pyxlib includes
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/exceptions.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/math_utils.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/string_utils.h"

// standard includes
#include <algorithm>
#include <cassert>
#include <cmath>

/*
Elevation to vertices 1-5 when vertex 0 is at the north pole. Approximately
equal to 26.565051177077989 degrees.
*/
static const double kfVertexElevation = asin(1.0 / sqrt(5.0));

//! The number of vertices for the icosahedron.
const int Icosahedron::knNumVertices = 12;

//! The number of faces for the icosahedron.
const int Icosahedron::knNumFaces = 20;

//! Get the length of a side of a unit icosahedron.
double Icosahedron::kfSideLength; 

//! The inradius of a unit icosahedron.
double Icosahedron::fInRadius;

//! The central angle of the icosahedron.
double Icosahedron::kfCentralAngle;

/*! 
Constructs an Icosahedron that is
symmetrical with the equator and has only one vertex falling on land.
See http://www.sou.edu/cs/sahr/dgg/orientation/stdorient.html for details.
*/
Icosahedron::Icosahedron() :
	m_vecIcoVertices(),
	m_vecFaces(knNumFaces)
{
	static CoordLatLon vertex0;
	vertex0.setInDegrees(58.28252559, 11.25);
	orientToSphere(vertex0, 0.0);
}

/*!
Constructor fills in the icosahedron particulars.

\param	vertex0		Location of vertex 0 in lat/lon coordinates.
\param	fAzimuth	The azimuth from vertex 0 to vertex 1 in degrees.
*/
Icosahedron::Icosahedron(const CoordLatLon& vertex0, double fAzimuth) :
	m_vecIcoVertices(),
	m_vecFaces(knNumFaces)
{
	orientToSphere(vertex0, fAzimuth);
}

/*!
Fill in the icosahedron particulars given one point and one edge's azimuth.

\param	vertex0		Vertex 0 specified in lat/lon coordinates.
\param	fAzimuth	The azimuth from vertex 0 to vertex 1 in degrees.	
*/
void Icosahedron::orientToSphere(const CoordLatLon& vertex0, double fAzimuth)
{
	/*
	Calculate the lat/lon coordinates for each vertex assuming vertex 0 is at
	the North Pole.
	*/
	CoordLatLonVector vecVertex(knNumVertices);

	int nVertex;
	for (nVertex = 1; nVertex <= 5; nVertex++) 
	{
		vecVertex[nVertex].setLat(kfVertexElevation);
		vecVertex[nVertex].setLonInDegrees(fAzimuth + 72.0 * (nVertex - 1));
		
		vecVertex[nVertex + 5].setLat(-kfVertexElevation);
		vecVertex[nVertex + 5].setLonInDegrees(	fAzimuth + 36.0 +
												72.0 * (nVertex - 1)	);
	}

	vecVertex[11].setLatInDegrees(-90.0);
	vecVertex[11].setLon(0.0);

	// transform the vertex coordinates for actual vertex 0 location
	CoordLatLon newNPold(vertex0.lat(), 0.0);

	m_vecIcoVertices.push_back(vertex0);
	for (nVertex = 1; nVertex < knNumVertices; nVertex++) 
	{
		m_vecIcoVertices.push_back(coordTrans(	newNPold,
												vecVertex[nVertex],
												vertex0.lon()	));
	}

	// set the vertices for each face
	for (int nFace = 0; nFace < knNumFaces; nFace++)
	{
		int nV1;
		int nV2;
		int nV3;
		PYXIcosMath::getFaceVertices(nFace + 'A', &nV1, &nV2, &nV3);

		m_vecFaces[nFace].setVertices(	m_vecIcoVertices[nV1 - 1],
										m_vecIcoVertices[nV2 - 1],
										m_vecIcoVertices[nV3 - 1]	);
	}
}

/*!
This method is called at application startup to initialize static data
*/
void Icosahedron::initStaticData()
{
	//! Get the length of a side of a unit icosahedron.
	kfSideLength  = 2.0 / sqrt(MathUtils::kfPHI * MathUtils::kfPHI + 1.0);

	//! The inradius of a unit icosahedron.
	fInRadius  = (3.0 * MathUtils::kfSqrt3 + sqrt(15.0)) / 12.0 * kfSideLength;

	//! The central angle of the icosahedron.
	kfCentralAngle = 2.0 * asin(kfSideLength / 2.0);
}

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
CoordLatLon Icosahedron::coordTrans(	const CoordLatLon& newNPold,
										const CoordLatLon& ptOld, 
										double fLon0	)

{
	double fLat;
	double fLon;

	if (newNPold.isNorthPole())
	{
		fLat = ptOld.lat();
		fLon = ptOld.lon();
	}
	else
	{
		// calculate the latitude of the new point in the range [0, 180]
		double fCosLat =	sin(newNPold.lat()) * sin(ptOld.lat()) + 
							cos(newNPold.lat()) * cos(ptOld.lat()) *
							cos(newNPold.lon() - ptOld.lon());

		fCosLat = std::min(fCosLat, 1.0);
		fCosLat = std::max(fCosLat, -1.0);

		fLat = acos(fCosLat);

		// calculate the longitude of the new point in the range [-180, 180)
		fLon;
		if (MathUtils::equal(fLat, 0.0) || MathUtils::equal(fLat, MathUtils::kfPI))
		{
			fLon = 0;
		}
		else
		{
			double fCosLon =	(sin(ptOld.lat()) * cos(newNPold.lat()) -
								cos(ptOld.lat()) * sin(newNPold.lat()) *
								cos(newNPold.lon() - ptOld.lon())) / sin(fLat);

			fCosLon = std::min(fCosLon, 1.0);
			fCosLon = std::max(fCosLon, -1.0);

			if (	((ptOld.lon() - newNPold.lon()) >= 0.0) &&
					((ptOld.lon() - newNPold.lon()) < MathUtils::kfPI)	)
			{
				fLon = fLon0 - acos(fCosLon);
			}
			else
			{
				fLon = fLon0 + acos(fCosLon);
			}
		}

		// convert latitude to a value in the range [-90, 90]
		fLat = MathUtils::kfPI / 2.0 - fLat;
	}

	return CoordLatLon(fLat, fLon);
}

/*!
Determine the icosahedron face in which the point lies.

\param	ll	The point in lat/lon coordinates

\return	The index of the face in the range [0, 19]
*/
int Icosahedron::findFace(const CoordLatLon& ll) const
{
	// convert the point to xyz coordinates
	PYXCoord3DDouble xyz = SphereMath::llxyz(ll);

	int nFoundFace = -1;

	for (int nFace = 0; nFace < knNumFaces; ++nFace)
	{
		if (m_vecFaces[nFace].inside(xyz))
		{
			nFoundFace = nFace;
			break;
		}
	}

	// no triangle found
	assert(-1 != nFoundFace);

	return nFoundFace;
}

/*!
Get the information for a given face of the icosahedron.

\param	nIndex	The index of the face.

\return	The face information.
*/
const Icosahedron::Face& Icosahedron::getFace(int nIndex) const
{
	if ((0 > nIndex) || (20 <= nIndex))
	{
		PYXTHROW(PYXSnyderException, "Invalid face: '" << nIndex << "'.");
	}

	return m_vecFaces[nIndex];
}

/*!
Set the vertices for a face. Vertices must be specified in counter-clockwise
order. This method also initializes the precomputed fields.

\param	vertex0	The first vertex.
\param	vertex1	The second vertex.
\param	vertex2	The third vertex.
*/
void Icosahedron::Face::setVertices(	const CoordLatLon& vertex0,
										const CoordLatLon& vertex1,
										const CoordLatLon& vertex2	)
{
	m_vecVertex[0] = vertex0;
	m_vecVertex[1] = vertex1;
	m_vecVertex[2] = vertex2;

	/*
	Calculate the centre of the spherical triangle formed by the vertices.
	*/

	// convert vertices to xyz coordinates
	PYXCoord3DDouble xyz;
	std::vector<PYXCoord3DDouble> vecXYZ;
	for (int nVertex = 0; nVertex < 3; nVertex++)
	{
		SphereMath::llxyz(m_vecVertex[nVertex], &xyz);
		vecXYZ.push_back(xyz);
	}

	// calculate the centroid in xyz coordinates
	xyz.set((vecXYZ[0].x() + vecXYZ[1].x() + vecXYZ[2].x()) / 3.0,
  		    (vecXYZ[0].y() + vecXYZ[1].y() + vecXYZ[2].y()) / 3.0,
		    (vecXYZ[0].z() + vecXYZ[1].z() + vecXYZ[2].z()) / 3.0	);

	// put centroid on the unit sphere
	xyz.normalize();

	// convert back to lat/lon coordinates
	m_sphTriCentre.setPoint(SphereMath::xyzll(xyz));

	/*
	Pre-calculate the azimuth of the vector from the triangle's centre point to
	vertex 0. See http://williams.best.vwh.net/avform.htm, and search for
	"Course between points" for a reference to the formula.
	*/
	m_fAzimuth = atan2(	cos(m_vecVertex[0].lat()) * sin(m_vecVertex[0].lon() -
						m_sphTriCentre.point().lon()),
						m_sphTriCentre.cosLat() * sin(m_vecVertex[0].lat()) -
						m_sphTriCentre.sinLat() * cos(m_vecVertex[0].lat()) *
						cos(m_vecVertex[0].lon() - m_sphTriCentre.point().lon())	);

	/*
	Calculate three vectors pointing from each side of the triangle to the
	opposite vertex. These are used in the calculation for determining if
	a point lies inside the triangle.
	*/
	m_v0 = vecXYZ[1].cross(vecXYZ[2]);
	m_v1 = vecXYZ[2].cross(vecXYZ[0]);
	m_v2 = vecXYZ[0].cross(vecXYZ[1]);
}

/*!
Determine if a point lies inside this face (inclusive).

\param	xyz	The point in xyz coordinates.

\return	true if the point lies inside this face, otherwise false
*/
bool Icosahedron::Face::inside(const PYXCoord3DDouble& xyz) const
{
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

	bool bInside = false;
	if (0.0 <= xyz.dot(m_v0))
	{
		if (0.0 <= xyz.dot(m_v1))
		{
			if (0.0 <= xyz.dot(m_v2))
			{
				bInside = true;
			}
		}
	}

	return bInside;
}
