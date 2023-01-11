#ifndef PYXIS__GEOMETRY__ICOS_TRAVERSER_H
#define PYXIS__GEOMETRY__ICOS_TRAVERSER_H
/******************************************************************************
icos_traverser.h

begin		: 2005-06-20
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/utility/pointer.h"

// forward declarations
class PYXGeometry;

//! Traverses cells on the icosahedron.
/*!
Does a pre-order traversal on all cells of the icosahedron for the specified
resolutions. Calls overridable functions to do work. The traversal is controlled
by the results of these functions.  The area that is examined can be limited
by using the setGeometry() method.

Potential for future expansion:
- support for post-order traversal (in addition to pre-order)
- support for user data passed during each processIndex
  (though this is easily put into subclass objects instead)
*/
class PYXLIB_DECL PYXIcosTraverser
{
public:

	//! The traversal result
	enum eTraversalResult
	{
		//! Continue traversal
		knContinue = 0,
		//! Abort traversal
		knAbort = 1,
		//! Don't traverse children but continue traversal with siblings
		knPrune = 2
	};

	//! Constructor
	PYXIcosTraverser() {}

	//! Destructor
	virtual ~PYXIcosTraverser() {}

	//! Traverse the specified resolutions
	bool traverse(	int nStartResolution = 1,
					int nMaxResolution = PYXMath::knMaxAbsResolution	);

	//! Traverse the specified index
	bool traverse(	const PYXIcosIndex& startIndex,
					int nMaxResolution = PYXMath::knMaxAbsResolution	);

	//! sets the iterator of cells to traverse.  Will default to a global goemetry.
	void setGeometry (PYXPointer<PYXGeometry> spGeom) {m_spGeometry = spGeom;}

protected:

	//! Traverse implementation
	eTraversalResult traverseImpl(const PYXIcosIndex& index);

private:

	//! Disable copy constructor
	PYXIcosTraverser(const PYXIcosTraverser&);

	//! Disable copy assignment
	void operator=(const PYXIcosTraverser&);

	/*!
	Override to do work when a traversal is begun.

	\return		True if the traversal should be aborted; false otherwise.
	*/
	virtual bool doBeginTraversal() { return false; }

	/*!
	Override to do work when a traversal is ended.

	\param bAborted	true if the traversal was aborted; false otherwise.
	*/
	virtual void doEndTraversal(bool bAborted) {}

	/*!
	Override to do work during the traversal.

	\param index	The index to process.

	\return The traversal result.
	*/
	virtual eTraversalResult doProcessIndex(const PYXIcosIndex& index) = 0;

	//! The maximum resolution to traverse
	int m_nMaxResolution;

	//! geometry used to get cells to test -- will default to the world.
	PYXPointer<PYXGeometry> m_spGeometry;
};

#endif // guard
