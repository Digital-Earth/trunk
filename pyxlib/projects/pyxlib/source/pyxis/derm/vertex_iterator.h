#ifndef PYXIS__DERM__VERTEX_ITERATOR_H
#define PYXIS__DERM__VERTEX_ITERATOR_H
/******************************************************************************
vertex_iterator.h

begin		: 2004-05-27
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/derm/sub_index_math.h"

/*!
PYXVertexIterator iterates over the vertices of a PYXIS cell in counter-
clockwise order. The iterator will iterate over the vertices regardless of
whether or not the cell has vertex children.
*/
//! Iterates over the vertices of a PYXIS cell.
class PYXLIB_DECL PYXVertexIterator : public PYXIterator
{
public:

	//! Constructor
	PYXVertexIterator(const PYXIcosIndex& rootIndex);

	//! Destructor
	virtual ~PYXVertexIterator() {;}

	//! Reset the iterator
	void reset(const PYXIcosIndex& rootIndex);

	//! Determine if all of the vertices have been iterated over.
	bool end() const;

	//! Advance to the next vertex
	void next();

	//! Access the index at the current iterator position.
	const PYXIcosIndex& getIndex() const;

	//! Return the current direction of the iterator.
	inline PYXMath::eHexDirection getDirection() 
	{
		return static_cast<PYXMath::eHexDirection>(m_nDirection);
	};

	/*!
	Get the root index.

	\return	The root index.
	*/
	inline const PYXIcosIndex& getRootIndex() const {return m_rootIndex;};

	/*!
	Get the centroid child of the root index.

	\return	The centroid child.
	*/
	inline const PYXIcosIndex& getCentroidChild() const {return m_centroidIndex;}

	//! Get the number of vertices over which the iterator iterates.
	int numVertices() const;

protected:

private:

	//! Disable default constructor
	PYXVertexIterator();

	//! Disable copy constructor
	PYXVertexIterator(const PYXVertexIterator&);

	//! Disable copy assignment
	void operator =(const PYXVertexIterator&);

	//! The index whose vertices are being iterated
	PYXIcosIndex m_rootIndex;

	//! The centroid child index of the root
	PYXIcosIndex m_centroidIndex;

	//! Index for temporary use.
	mutable PYXIcosIndex m_index;

	//! The current direction
	int m_nDirection;

	//! Indicates if the cell is a pentagon or a hexagon
	bool m_bIsPentagon;
};

#endif // guard
