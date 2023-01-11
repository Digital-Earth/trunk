#ifndef PYXIS__DERM__COMPACT_INDEX_H
#define PYXIS__DERM__COMPACT_INDEX_H
/******************************************************************************
compact_index.h

begin		: 2007-05-25
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "index.h"

// standard includes
#include <string>
#include <vector>

//! The number of digits that can be stored in one byte
const int knDigitsPerByte = 4;

/*!
The CompactIndex class is a memory efficient implementation of a PYXIcosIndex.
It is useful when large numbers of indices must be stored in memory and uses
the pair-wise encoding scheme for the subindex. This class provides methods for
converting to/from a PYXIcosIndex and string as well as comparison methods for
depth-first sorting.

\verbatim
The index is encoded into 12 bytes as follows:

Byte
0:	  The primary index [0 | 1-12 | 'A' - 'T']
1-10: Each byte stores two pairs of digits as per the table below.
11:   The resolution of the index: "null"=-1, "A"=1, "A-0"=2...

If the last byte is -1 and all other bytes are 0, then the index is null.

The digits are encoded in pairs as follows. The second digit in the pair is
ignored if it exceeds the number of digits in the index. Any digit bits not
participating in the index are cleared.

Pair  Binary  Hex
 00    0000    0
 01    0001    1
 02    0010    2
 03    0011    3
 04    0100    4
 05    0101    5
 06    0110    6
 XX    0111    7    UNUSED
 XX    1000    8    UNUSED
 10    1001    9
 20    1010    A
 30    1011    B
 40    1100    C
 50    1101    D
 60    1110    E
 XX    1111    F    UNUSED
\endverbatim
*/
//! Identifies a unique cell in a global grid.
class PYXLIB_DECL CompactIndex
{
public:

	//! Test method
	static void test();

	//! Number of bits per digit
	static const int knBitsPerDigit;

public:

	//! Constructor
	CompactIndex();

	// Default copy constructor is sufficient

	//! Construct from a string
	CompactIndex(const std::string& strIndex);

	//! Construct from a PYXIS index
	CompactIndex(const PYXIcosIndex& pyxIndex);

	// Default destructor is sufficient
	
	//! Copy assignment.
//	CompactIndex& operator =(const CompactIndex &rhs);

	//! String assignment.
	CompactIndex& operator =(const std::string& strIndex);

	//! PYXIS index assignment.
	CompactIndex& operator =(const PYXIcosIndex& pyxIndex);

	//! Set the index to null
	void reset();

	/*!
	Get the resolution of the index

	\return	The resolution. (-1 if index is null.)
	*/
	inline int getResolution() const {return m_pcBuffer[sizeof(m_pcBuffer) - 1];}

	//! Convert an index to a string.
	std::string toString() const;

	//! Convert a compact index to a PYXIS index.
	operator PYXIcosIndex() const;

	//! Equality operator.
	bool operator ==(const CompactIndex& rhs) const;

	//! Inequality operator
	bool operator !=(const CompactIndex& rhs) const;

	//! Less than operator.
	bool operator <(const CompactIndex& index) const;

	//! Less than or equals operator.
	bool operator <=(const CompactIndex& index) const;

	//! Greater than operator.
	bool operator >(const CompactIndex& index) const;

	//! Greater than or equals operator.
	bool operator >=(const CompactIndex& index) const;

	/*!
	Determine if this is a null index.

	\return	true if this is a null index, otherwise false
	*/
	//! Is this a null index
	inline bool isNull() const {return (getResolution() < 0);}

	//! Does this index represent a vertex at resolution 1.
	bool isVertex() const;

	//! Determine if this index is on a PYXIS pole.
	bool isPolar() const;

	//! Does this index represent a face at resolution 1.
	bool isFace() const;

	//! Check to see if this index is a centroid child of its parent.
	bool hasVertexChildren() const;

	//! Check to see if this cell has 5 sides.
	bool isPentagon() const;

	//! Determine if this index is an ancestor of the specified index.
	bool isAncestorOf(const CompactIndex& index) const;

	//! Determine if this index is a descendant of the specified index.
	bool isDescendantOf(const CompactIndex& index) const;

protected:

private:

	//! Pack 4 digits into a byte.
	signed char packDigits(unsigned int pnDigits[]) const;

	//! Unpack 4 digits from a byte.
	void unpackDigits(signed char cByte, unsigned int pnDigits[]) const;

private:

	//! The bytes where the index is stored. See class header for details.
	signed char m_pcBuffer[knMaxDigits / knDigitsPerByte + 2];
};

//! Allows compact indices to be written to streams.
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const CompactIndex& index);

//! Allows compact indices to be read from streams.
PYXLIB_DECL std::istream& operator >>(std::istream& input, CompactIndex& index);

#endif // guard
