#ifndef PYXIS__DERM__CURSOR_H
#define PYXIS__DERM__CURSOR_H
/******************************************************************************
cursor.h

begin		: 2006-12-14
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/derm/index.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <iosfwd>

/*!
The cursor moves over PYXIS indices while keeping track of rotation relative
to its initial direction. It can be thought of as akin to a Logo turtle, in
that it follows cell connectivity relative to itself via simple commands such
as forward, left, zoomIn, etc.

When a cursor is created, or its index or direction are set, the intial
direction is reset. Rotation relative to that initial direction is 0. As the
cursor moves, its nominal direction may change due to the orientation of the
indices, but it will remember its rotation relative to this inital direction.

Pentagons are handled as follows. When a pentagon is entered, one of two
potential directions must be chosen. For class I we choose to wind left (i.e.
towards class II), and for class II we choose to wind right (i.e. towards
class I).
*/
//! PYXIS DERM cursor.
class PYXLIB_DECL PYXCursor
{
public:

	//! Test method
	static void test();

public:

	//! Create a new cursor.
	explicit PYXCursor()
	{
		reset(PYXIcosIndex(PYXIcosIndex("1-0")), PYXMath::knDirectionOne);
	}

	//! Create a new cursor.
	explicit PYXCursor(const PYXIcosIndex& index)
	{
		reset(index, PYXMath::knDirectionOne);
	}

	//! Create a new cursor.
	explicit PYXCursor(const PYXIcosIndex& index, PYXMath::eHexDirection nDir)
	{
		reset(index, nDir);
	}

	//! Move the cursor to the next cell, in the direction the cursor is moving.
	void forward();

	//! repeat forward operation nCount times
	void forward(int nCount);

	//! Move the cursor to the next cell, in the opposite direction the cursor is moving.
	void backward();

	//! repeat forward operation nCount times
	void backward(int nCount);

	//! Rotate the cursor's current direction by one, to the left
	void left(int nCount = 1);

	//! Rotate the cursor's current direction by one, to the right
	void right(int nCount = 1);

	//! Decrement, by one, the resolution of the cell the cursor is on.
	void zoomOut();

	//! Increment, by one, the resolution of the cell the cursor is on.
	void zoomIn();

	//! Get the PYXIS index for the current cell.
	const PYXIcosIndex& getIndex() const
	{
		return m_index;
	}

	//! Get the current forward direction.
	PYXMath::eHexDirection getDir() const
	{
		return m_nDir;
	}

	//! Get the current backward direction.
	PYXMath::eHexDirection getBackDir() const;

	//! Get the direction of the cursor relative to the starting direction.
	PYXMath::eHexDirection getInitialDir() const
	{
		return PYXMath::rotateDirection(m_nDir, -m_nRot);
	}

	//! Get the rotation amount.
	int getRotation() const
	{
		return m_nRot;
	}

	//! Reset the current state.
	void reset(const PYXIcosIndex& index, PYXMath::eHexDirection nDir);

	//! Set the current index the cursor is on.
	void setIndex(const PYXIcosIndex& index);

	//! Set the direction the cursor is moving in.
	void setDir(PYXMath::eHexDirection nDir);

	//! Returns a string representation.
	std::string toString() const;

private:

	//! Resolves the direction.
	void resolveDirection();

	//! Debugging aid.
	void checkInvariant()
	{
		assert(getInitialDir() == m_nInitialDir);
	}

	//! Sets the initial direction to the current direction.
	void setInitialDir()
	{
		m_nRot = 0;
#ifndef NDEBUG
		m_nInitialDir = m_nDir;
#endif
	}

#ifndef NDEBUG
	//! Debugging aid.
	PYXMath::eHexDirection m_nInitialDir;
#endif

private:

	//! The index of the cursor.
	PYXIcosIndex m_index;

	//! The direction of the cursor.
	PYXMath::eHexDirection m_nDir;

	//! The rotation.
	int m_nRot;
};

//! Stream insertion operator.
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const PYXCursor& cursor);

#endif // guard
