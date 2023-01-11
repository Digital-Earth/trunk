/******************************************************************************
icos_traverser.cpp

begin		: 2005-06-20
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/icos_traverser.h"

// pyxlib includes
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/derm/icos_iterator.h"

#include "pyxis/geometry/exceptions.h"
#include "pyxis/geometry/geometry.h"
/*!
\param nStartResolution		The resolution at which to begin the traversal.
							The default is 1.
\param nMaxResolution		The resolution at which to end the traversal.
							The default is PYXMath::knMaxAbsResolution.
\return True if the traversal was aborted; false otherwise.
*/
bool PYXIcosTraverser::traverse(	int nStartResolution,
									int nMaxResolution	)
{
	// Check arguments
	if (nStartResolution < 1 ||
		nMaxResolution < nStartResolution ||
		PYXMath::knMaxAbsResolution < nMaxResolution)
	{
		PYXTHROW(	PYXGeometryException,
					"Invalid traversal resolution: '" << nMaxResolution << "'."	);
	}

	// Save arguments
	m_nMaxResolution = nMaxResolution;

	bool bAborted = doBeginTraversal();

	try
	{
		if (!bAborted)
		{
			PYXPointer<PYXIterator> spIterator;
			if (m_spGeometry == 0)
			{
				spIterator = PYXIcosIterator::create(nStartResolution);
			}
			else
			{
				spIterator = m_spGeometry->getIterator();
			}
			// Traverse children
			for (; !spIterator->end(); spIterator->next())
			{
				eTraversalResult nChildResult = traverseImpl(spIterator->getIndex());
				if (nChildResult == knAbort)
				{
					bAborted = true;
					break;
				}
			}
		}
	}
	catch (PYXException& e)
	{
		PYXRETHROW(	e, PYXGeometryException,
					"An error occurred during icosahedron traversal."	);			
	}

	doEndTraversal(bAborted);
	return bAborted;
}

/*!
\param startIndex			The index at which to begin the traversal.
\param nMaxResolution		The resolution at which to end the traversal.
							The default is PYXMath::knMaxAbsResolution.
\return true if the traversal was aborted; false otherwise.
*/
bool PYXIcosTraverser::traverse(	const PYXIcosIndex& startIndex,
									int nMaxResolution	)
{
	// Check arguments
	if (nMaxResolution < startIndex.getResolution() ||
		PYXMath::knMaxAbsResolution < nMaxResolution)
	{
		PYXTHROW(	PYXModelException,
					"Invalid resolution: '" << nMaxResolution << "'."	);
	}

	// Save arguments
	m_nMaxResolution = nMaxResolution;

	bool bAborted = doBeginTraversal();

	if (!bAborted)
	{
		eTraversalResult nResult = traverseImpl(startIndex);
		if (nResult == knAbort)
		{
			bAborted = true;
		}
	}

	doEndTraversal(bAborted);

	return bAborted;
}

/*!
This function implements the traversal using recursion.

\param index	The index to process.
\return The traversal result.
*/
PYXIcosTraverser::eTraversalResult PYXIcosTraverser::traverseImpl(const PYXIcosIndex& index)
{
	// Process index (pre-order)
	eTraversalResult nResult = doProcessIndex(index);

	if (nResult == knContinue &&
		index.getResolution() < m_nMaxResolution)
	{
		// Traverse children
		for (	PYXExhaustiveIterator it(index, index.getResolution() + 1);
				!it.end();
				it.next()	)
		{
			eTraversalResult nChildResult = traverseImpl(it.getIndex());
			if (nChildResult == knAbort)
			{
				nResult = knAbort;
				break;
			}
		}
	}

	return nResult;
}
