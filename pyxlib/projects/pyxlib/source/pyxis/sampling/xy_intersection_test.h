#ifndef PYXIS__SAMPLING__XY_INTERSECTION_TEST_H
#define PYXIS__SAMPLING__XY_INTERSECTION_TEST_H
/******************************************************************************
xy_intersection_test.h

begin		: 2005-06-22
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/geometry/test.h"
#include "pyxis/utility/rect_2d.h"

// forward declarations
class ICoordConverter;

/*!
The test uses methods from ICoordConverter to determine if a PYXIS
cell intersects the geometry of a coverage exprssed in xy coordinates.
*/
//! The test that determines if a PYXIS cell intersects a non-PYXIS coverage.
class PYXLIB_DECL PYXXYIntersectionTest : public PYXTest
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
	}; 

	//! Constructor
	PYXXYIntersectionTest();

	//! Destructor
	virtual ~PYXXYIntersectionTest() {}

	//! Set the native coverage to be tested.
	void setConverter(const ICoordConverter* pConverter);

	//! Set the bounds of the area to be tested.
	void setBounds(const PYXRect2DDouble& bounds);

private:

	//! Disable copy constructor
	PYXXYIntersectionTest(const PYXXYIntersectionTest&);

	//! Disable copy assignment
	void operator=(const PYXXYIntersectionTest&);

	//! Perform the test.
	virtual eTestResult doTestIndex(	const PYXIcosIndex& index,
										bool* pbAbort	);

	//! Check if any of the hexagon vertices are in the area.
	bool hasHexSingleVertexInArea(const PYXIcosIndex& index) const;

	//! Check if all vertices of the hexagon are in the area.
	eVertexResult hasHexAllVerticesInArea(const PYXIcosIndex& index) const;
	
	//! Check if the cell is one of the areas corners.
	bool isCellAreaCorner(const PYXIcosIndex& index) const;

	//! Check if all of the children of a centroid hexagon are covered.
	bool allChildrenCovered(const PYXIcosIndex& index) const;

private:

	//! The coverage to be tested
	const ICoordConverter* m_pConverter;

	//! The bounds of the coverage in native coordinates.
	PYXRect2DDouble m_bounds;

#ifdef _DEBUG
	//! metrics which can be checked in a debug run.
	int	m_nTestsDone;
	int m_nResultsYes;
	int m_nResultsNo;
	int m_nResultsYesComplete;
	int m_nResultsMaybe;
#endif
};

#endif // guard
