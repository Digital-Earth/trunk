#ifndef PYXIS__DERM__ICOS_ITERATOR_H
#define PYXIS__DERM__ICOS_ITERATOR_H
/******************************************************************************
icos_iterator.h

begin		: 2004-03-22
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/iterator.h"

/*!
The PYXIcosVertexIterator iterates through all the vertices on the icosahedron
at the specified resolution. It returns indices 01-0*, 02-0*, ..., 12-0*.
*/
//! Iterates through all the vertices on the icosahedron.
class PYXLIB_DECL PYXIcosVertexIterator : public PYXIterator
{
public:

	//! Constructor
	PYXIcosVertexIterator(int nResolution = -1);

	//! Destructor
	virtual ~PYXIcosVertexIterator();

	//! Move to the next cell
	virtual void next();

	//! See if we have covered all the cells
	bool end() const;

	//! Get the PYXIS index for the current cell.
	virtual const PYXIcosIndex& getIndex() const;

private:

	//! Disable copy constructor
	PYXIcosVertexIterator(const PYXIcosVertexIterator&);

	//! Disable copy assignment
	void operator=(const PYXIcosVertexIterator&);

	//! Sets up the iterator for the next vertex.
	void setupNextIterator();

private:

	//! The resolution
	int m_nResolution;

	//! The current vertex
	int m_nVertex;

	//! The exhaustive iterator for the current vertex
	PYXPointer<PYXExhaustiveIterator> m_spIt;
};

/*!
The PYXIcosFaceIterator iterates through all the faces on the icosahedron at
the specified resolution. It returns indices A-0*, B-0*, ..., T-0*.
*/
//! Iterates through all the faces on the icosahedron.
class PYXLIB_DECL PYXIcosFaceIterator : public PYXIterator
{
public:

	//! Constructor
	PYXIcosFaceIterator(int nResolution = -1);

	//! Destructor
	virtual ~PYXIcosFaceIterator();

	//! Move to the next cell
	virtual void next();

	//! See if we have covered all the cells
	bool end() const;

	//! Get the PYXIS index for the current cell.
	virtual const PYXIcosIndex& getIndex() const;

private:

	//! Disable copy constructor
	PYXIcosFaceIterator(const PYXIcosFaceIterator&);

	//! Disable copy assignment
	void operator=(const PYXIcosFaceIterator&);

	//! Sets up the iterator for the next face
	void setupNextIterator();

private:

	//! The resolution
	int m_nResolution;

	//! The current face
	char m_nFace;

	//! The exhaustive iterator for the current face
	PYXPointer<PYXExhaustiveIterator> m_spIt;
};

/*!
The PYXIcosIterator performs an exhaustive iteration over the icosahedron
covering the vertices then the faces at the specified resolution.
*/
//! Iterates through all the cells on the icosahedron.
class PYXLIB_DECL PYXIcosIterator : public PYXIterator
{
public:

	//! Dynamic creator
	static PYXPointer<PYXIcosIterator> create(int nMinResolution = -1)
	{
		return PYXNEW(PYXIcosIterator, nMinResolution);
	}

	//! Constructor
	PYXIcosIterator(int nMinResolution = -1);

	//! Destructor
	virtual ~PYXIcosIterator() {;}

	//! Move to the next cell
	virtual void next();

	//! See if we have covered all the cells
	bool end() const;

	//! Get the PYXIS index for the current cell.
	virtual const PYXIcosIndex& getIndex() const;

private:

	//! Disable copy constructor
	PYXIcosIterator(const PYXIcosIterator&);

	//! Disable copy assignment
	void operator=(const PYXIcosIterator&);

private:

	//! The vertex iterator
	PYXIcosVertexIterator m_itVertex;

	//! The face iterator
	PYXIcosFaceIterator m_itFace;

	//! The current iterator.
	//! Note that this pointer does not own the pointee.
	PYXIterator* m_pIt;
};

#endif // guard
