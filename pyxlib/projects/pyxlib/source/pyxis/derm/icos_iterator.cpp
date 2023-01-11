/******************************************************************************
icos_iterator.cpp

begin		: 2004-03-24
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/icos_iterator.h"

// local includes
#include "pyxis/derm/icosahedron.h"
#include "pyxis/derm/exhaustive_iterator.h"

/*!
Constructor creates an iterator for all the vertices on the globe at a given
resolution.

\param nResolution	The resolution over which to iterate. The default is
					resolution 1.
*/
PYXIcosVertexIterator::PYXIcosVertexIterator(int nResolution) :
	m_nResolution(nResolution),
	m_nVertex(0),
	m_spIt()
{
	if (1 > m_nResolution)
	{
		m_nResolution = 1;
	}

	setupNextIterator();
}

/*!
Destructor cleans up memory.
*/
PYXIcosVertexIterator::~PYXIcosVertexIterator()
{
}

/*!
Move to the next cell.
*/
void PYXIcosVertexIterator::next()
{
	m_spIt->next();

	if (m_spIt->end())
	{
		setupNextIterator();
	}
}

/*!
See if we have covered all the cells.

\return	true if all cells have been covered, otherwise false
*/
bool PYXIcosVertexIterator::end() const
{
	return m_spIt->end();	
}

/*!
Get the PYXIS index for the current cell.

\return	The index.
*/
const PYXIcosIndex& PYXIcosVertexIterator::getIndex() const
{
	return m_spIt->getIndex();
}

/*!
Sets up the iterator for the next vertex once the previous one has been
completed.
*/
void PYXIcosVertexIterator::setupNextIterator()
{
	if (Icosahedron::knNumVertices > m_nVertex)
	{
		m_nVertex++;

		PYXIcosIndex root;
		root.setPrimaryResolution(m_nVertex);

		m_spIt = PYXExhaustiveIterator::create(root, m_nResolution);
	}
}

/*!
Constructor creates an iterator for all the faces on the globe at a given
resolution.

\param nResolution	The resolution over which to iterate. The default is
					resolution 1. If the resolution is set to 0 then the
					iterator is initialized to the end condition.
*/
PYXIcosFaceIterator::PYXIcosFaceIterator(int nResolution) :
	m_nResolution(nResolution),
	m_nFace(0),
	m_spIt()
{
	if (m_nResolution < 0)
	{
		m_nResolution = 1;
	}

	// create an iterator object.
	setupNextIterator();

	// if the resolution is 0, there are no faces to iterate over.
	if (m_nResolution == 0)
	{
		m_spIt->setEnd();
		m_nFace = Icosahedron::knNumFaces;
	}
}

/*!
Destructor cleans up memory.
*/
PYXIcosFaceIterator::~PYXIcosFaceIterator()
{
}

/*!
Move to the next cell.
*/
void PYXIcosFaceIterator::next()
{
	m_spIt->next();

	if (m_spIt->end())
	{
		setupNextIterator();
	}
}

/*!
See if we have covered all the cells.

\return	true if all cells have been covered, otherwise false
*/
bool PYXIcosFaceIterator::end() const
{
	return m_spIt->end();	
}

/*!
Get the PYXIS index for the current cell.

\return	The index.
*/
const PYXIcosIndex& PYXIcosFaceIterator::getIndex() const
{
	return m_spIt->getIndex();
}

/*!
Sets up the iterator for the next face once the previous one has been
completed.
*/
void PYXIcosFaceIterator::setupNextIterator()
{
	if (Icosahedron::knNumFaces > m_nFace)
	{
		m_nFace++;

		PYXIcosIndex root;
		root.setPrimaryResolution('A' + m_nFace - 1);

		m_spIt = PYXExhaustiveIterator::create(root, m_nResolution);
	}
}

/*!
Constructor creates an iterator for all the cells on the globe at a given
resolution.

\param nResolution	The resolution over which to iterate. The default is
					resolution 1.
*/
PYXIcosIterator::PYXIcosIterator(int nResolution) :
	m_itVertex(nResolution),
	m_itFace(nResolution),
	m_pIt(&m_itVertex)
{
}

/*!
Move to the next cell.
*/
void PYXIcosIterator::next()
{
	m_pIt->next();
	if (m_pIt->end() && (m_pIt == &m_itVertex))
	{
		m_pIt = &m_itFace;
	}	
}

/*!
See if we have covered all the cells.

\return	true if all cells have been covered, otherwise false.
*/
bool PYXIcosIterator::end() const
{
	return (	m_pIt->end() &&
				(m_pIt == dynamic_cast<const PYXIterator*>(&m_itFace))	);

}

/*!
Get the PYXIS index for the current cell.

\return	The PYXIS index for the current cell.
*/
const PYXIcosIndex& PYXIcosIterator::getIndex() const
{
	return m_pIt->getIndex();
}
