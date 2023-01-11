/******************************************************************************
vertex_iterator.cpp

begin		: 2004-04-27
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/vertex_iterator.h"

// pyxlib includes
#include "pyxis/derm/hexagon.h"
#include "pyxis/derm/index_math.h"

// standard includes
#include <cassert>

/*!
Constructor

\param	rootIndex	The index over which to iterate.
*/
PYXVertexIterator::PYXVertexIterator(const PYXIcosIndex& rootIndex)
{
	reset(rootIndex);
}

/*!
Reset the iteration.

\param	rootIndex	The index over which to iterate.
*/
void PYXVertexIterator::reset(const PYXIcosIndex& rootIndex)
{
	assert(!rootIndex.isNull() && "Invalid argument.");

	m_rootIndex = rootIndex;
	m_centroidIndex = rootIndex;
	PYXIcosMath::zoomIntoNeighbourhood(&m_centroidIndex);

	m_nDirection = PYXMath::knDirectionZero;

	m_bIsPentagon = m_rootIndex.isPentagon();
	next();
}

/*! 
Is the iterator complete.

\return	true if the iterator is complete, otherwise false.
*/
bool PYXVertexIterator::end() const
{
	return (m_nDirection > PYXMath::knDirectionSix);
}

/*!
Advance to the next vertex.
*/
void PYXVertexIterator::next()
{
	if (!end())
	{
		m_nDirection++;

		PYXMath::eHexDirection nHexDirection =
			static_cast<PYXMath::eHexDirection>(m_nDirection);

		if (	!end() &&
				m_bIsPentagon &&
				!PYXIcosMath::isValidDirection(m_rootIndex, nHexDirection)	)
		{
			next();
		}
	}
}

/*!
Get the PYXIcosIndex (if it exists) on the next higher resolution that shares
its origin with the current vertex.

\return true if a valid index was found, otherwise false
*/
const PYXIcosIndex& PYXVertexIterator::getIndex() const
{
	bool bSuccess = false;

	if (!end())
	{
		m_index = m_rootIndex;
		PYXMath::eHexDirection nHexDirection =
			static_cast<PYXMath::eHexDirection>(m_nDirection);

		PYXIcosMath::zoomIntoNeighbourhood(&m_index, nHexDirection);
	}

	return m_index;
}

/*!
Get the number of vertices over which the iterator iterates. This will be
six for hexagonal cells and five for pentagonal cells.

\return	The number of vertices.
*/
int PYXVertexIterator::numVertices() const
{
	if (m_bIsPentagon)
	{
		return Hexagon::knNumVertices - 1;
	}

	return Hexagon::knNumVertices;
}
