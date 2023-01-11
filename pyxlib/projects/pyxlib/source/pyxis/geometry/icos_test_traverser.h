#ifndef PYXIS__GEOMETRY__ICOS_TEST_TRAVERSER_H
#define PYXIS__GEOMETRY__ICOS_TEST_TRAVERSER_H
/******************************************************************************
icos_test_traverser.h

begin		: 2005-06-21
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxis includes
#include "pyxlib.h"
#include "pyxis/geometry/icos_traverser.h"

// forward declarations
class PYXTest;
class PYXTileCollection;

/*!
Performs a test on each index. Depending on the test result, indices are
recorded in a result set.

Potential for future expansion:
- it would be trivial to make the class collect results across multiple
  traversals, if that's useful
*/
//! Traverses cells on the icosahedron.
class PYXLIB_DECL PYXIcosTestTraverser : public PYXIcosTraverser
{
public:

	//! Test method
	static void test();

	//! Constructor
	PYXIcosTestTraverser() : m_pTest(0) {}

	//! Returns the results of the traversal.
	PYXPointer<PYXTileCollection> getTileCollection();

	//! Sets the test
	void setTest(PYXTest& test);

private:

	//! Disable copy constructor
	PYXIcosTestTraverser(const PYXIcosTestTraverser&);

	//! Disable copy assignment
	void operator=(const PYXIcosTestTraverser&);

	//! Override to do work when a traversal is begun
	virtual bool doBeginTraversal();

	//! Override to do work during the traversal
	virtual eTraversalResult doProcessIndex(const PYXIcosIndex& index);

private:

	//! The test
	PYXTest* m_pTest;

	//! The tile collection
	PYXPointer<PYXTileCollection> m_spTileCollection;
};

#endif // guard
