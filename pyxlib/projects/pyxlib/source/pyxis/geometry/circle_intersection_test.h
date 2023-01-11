#ifndef CIRCLE_INTERSECTION_TEST_H
#define CIRCLE_INTERSECTION_TEST_H
/******************************************************************************
circle_intersection_test.h

begin		: 2006-05-10
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxlib.h"
#include "test.h"

// pyxlib includes
#include "pyxis/derm/index.h"
#include "pyxis/derm/projection_method.h"
#include "pyxis/utility/coord_lat_lon.h"


// standard includes
#include <string>

/*!
The test determines if all or part of a PYXIS cell falls within an area
defined by an index and a radius.  All calculations are performed on the
unit sphere.
*/
//! The test that determines if a PYXIS cell intersects circular area.
class PYXLIB_DECL CircleIntersectionTest : public PYXTest
{
public:

	//! The test result.
	enum eVertexResult
	{
		//! No vertices.
		knNoVertices,

		//! Not all but some vertices.
		knSomeVertices,

		//! All vertices.
		knAllVertices,

		//! The hexagon entirely contains the defined area.
		knAreaContained
	}; 

	//! Constructor
	explicit CircleIntersectionTest(const ProjectionMethod* pProjection);

	//! Destructor
	virtual ~CircleIntersectionTest() {;}

	//! Set the centre index of the circular area.
	void setCentre(const PYXIcosIndex& pyxIndex);

	//! Set the radius of the circle defined in metres?
	void setRadiusInMetres(double fRadius);

	//! Set the radius in radians for the great circle arc.
	void setRadiusInRadians(double fAngle);

	//! Return the radius of the test in radians.
	double getRadiusInRadians() {return m_fRadius;}

	//! Verify that the size of the area is appropriate for the resolution.
	bool verifySize(double fAngle);

protected:

private:

	//! Disable default constructor.
	CircleIntersectionTest();

	//! Disable copy constructor
	CircleIntersectionTest(const CircleIntersectionTest&);

	//! Disable copy assignment
	void operator=(const CircleIntersectionTest&);

	//! Perform the test.
	virtual eTestResult doTestIndex(	const PYXIcosIndex& index,
										bool* pbAbort	);

	//! Calculate the number of vertices that fall within the radius.
	eVertexResult calcIncludedVertices(const PYXIcosIndex& pyxIndex);

	//! The index at the centre of the circular area.
	PYXIcosIndex m_pyxIndex;

	//! The Lat Lon coord at the centre of the defining index.
	CoordLatLon m_centreCoord;

	//! The radius of the circular area stored in radians.
	double m_fRadius;

	//! The method of conversion from PYXIS cells to 3D points.
	const ProjectionMethod* m_pProjection;
};

#endif	// end guard
