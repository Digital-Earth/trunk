#ifndef PYXIS__DERM__INDEX_H
#define PYXIS__DERM__INDEX_H
/******************************************************************************
index.h

begin		: 2004-01-28
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/sub_index.h"

#ifdef INSTANCE_COUNTING
#include "pyxis/utility/instance_counter.h"
#endif

// standard includes
#include <string>

/*!
The PYXIcosIndex class identifies a unique cell and grid resolution in a
discrete global grid system formed by tesselating an icosahedron with
hexagons. As a string, the index takes the form [1-12 | A-T] - # where the
first segment identifies primary resolution value in the form of an alpha-
numeric value indicating a face or vertex.  A dash separates the primary
resolution from the sub index.
*/
//! Identifies a unique cell on an icosahedraon tesselated by hexagons.
class PYXLIB_DECL PYXIcosIndex
#ifdef INSTANCE_COUNTING
	: protected InstanceCounter
#endif
{
public:

	//! Test method
	static void test();

	//! Numeric offset for resolution 0.
	static const int knResolution0 = 0;

	//! Numeric offset for resolution 1.
	static const int knResolution1 = 1;

	/*!
		This value is the first single character label for the face values on the 
		icosahedron.  The remaining 19 sides of the icosahedron will be labeled
		with the 19 values that follow this one in the ASCII character set.
	*/

	//! Offset for the face characters.
	static const char kcFaceFirstChar = 'A';

	//! The last face value on the icosahedron.
	static const char kcFaceLastChar = 'T';

	//! The first conventionally addressed resolution.
	static const int knMinSubRes = 2;

	//! The first vertex.
	static const int knFirstVertex = 1;

	//! The last vertex.
	static const int knLastVertex = 12;

	//! Primary resolution value indicating a null index.
	static const int knNullPrimaryRes = 0;

	//! Constructor.
	PYXIcosIndex();

	//! Copy constructor.
	PYXIcosIndex(const PYXIcosIndex& rhs);

	//! Construct from a string.
	PYXIcosIndex(const std::string& strIndex);

	//! Destructor.
	virtual ~PYXIcosIndex();
	
	//! Copy assignment.
	PYXIcosIndex& operator=(const PYXIcosIndex &rhs);

	//! String assignment.
	PYXIcosIndex& operator=(const std::string& strIndex);

	//! Set the PYXIcosIndex to null.
	void reset();

	//! Normalizes the resolution
	static void normalizeResolution(int* pnResolution);

	/*!
	Determine if this is a null index.

	\return	true if this is a null index, otherwise false.
	*/
	//! Is this a null index.
	inline bool isNull() const
	{
		return (knNullPrimaryRes == m_nPrimaryResolution);
	}

	//! Convert an index to a string.
	std::string toString() const;

	//! Equality operator.
	bool operator==(const PYXIcosIndex& rhs) const;

	//! Inequality operator.
	bool operator!=(const PYXIcosIndex& rhs) const {return !(*this == rhs);}

	//! Less than operator.
	bool operator<(const PYXIcosIndex& index) const;

	//! Add and return operator.
    PYXIcosIndex& PYXIcosIndex::operator +=(const PYXIcosIndex& rhs);

	/*!
	Get the subindex associated with this index.

	\return	The subindex.
	*/
	//! Return a constant reference to the sub index.
	const PYXIndex& getSubIndex() const {return m_pyxIndex;}

	/*!
	Get the subindex associated with this index.

	\return	The subindex.
	*/
	//! Return the sub index.
	PYXIndex& getSubIndex() {return m_pyxIndex;}

	//! Get the resolution of the PYXIS index.
	int getResolution() const;

	//! Set the resolution of the PYXIS index.
	void setResolution(int nResolution);

	//! Increase the the resolution by 1
	void incrementResolution();

	//! Decrease the the resolution by 1
	void decrementResolution();

	/*!
	Get the primary resolution value.

	\return	The primary resolution value.
	*/
	//! Return the combined primary resolution value.
	int getPrimaryResolution() const {return m_nPrimaryResolution;}

	/*!
	Set the primary resolution value.

	\param	nPrimaryValue	The primary resolution value.
	*/
	//! Set the combined primary resolution value.
	void setPrimaryResolution(int nPrimaryValue);

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

	//! Check to see if this cell has 5 sides.
	bool isPentagon(unsigned int nSubIndexDigitCount) const;

	//! Check to see if this cell has 6 sides.
	bool isHexagon() const
	{
		return !isPentagon();
	}

	//! Check to see if this is a Major index.
	bool isMajor() const
	{
		return hasVertexChildren();
	}

	//! Check to see if this is a Minor index.
	bool isMinor() const
	{
		return !hasVertexChildren();
	}

	//! Check to see if this is index is in the northern hemisphere.
	bool isNorthern() const
	{
		return getPrimaryResolution() <= 6
			|| ('A' <= getPrimaryResolution() && getPrimaryResolution() <= 'J');
	}

	//! Check to see if this is index is in the southern hemisphere.
	bool isSouthern() const
	{
		return 'K' <= getPrimaryResolution()
			|| (7 <= getPrimaryResolution() && getPrimaryResolution() <= 12);
	}

	//! Returns the class of this index (either 1 or 2).
	int getClass() const
	{
		return (getResolution() % 2) + 1;
	}

	//! Determine if this index is an ancestor of the specified index.
	bool isAncestorOf(const PYXIcosIndex& index) const;

	//! Determine if this index is a descendant of the specified index.
	bool isDescendantOf(const PYXIcosIndex& index) const;

	//! Determine the number of sides.
	int determineNumSides() const;

	//! Randomizes this index.
	void randomize(int nResolution);

	//! Returns the maximum child count for the index described by the first 'nSubIndexDigitCount' digits.
	unsigned int getMaximumChildCount(unsigned int nSubIndexDigitCount) const;

private:

	//! Check for valid primary resolution value.
	static bool isValidPrimary(int nResValue);

	//! Give PYXIcosMath access to private methods and data.
	friend class PYXIcosMath;

	//! Give CompactIndex access to private constants.
	friend class CompactIndex;

private:

	//! The string used to store the null index label.
	static const std::string kstrNullIndexLabel;

	//! Place holder for empty resolution 1 value when on a vertex
	static const int knRes1Vertex = '0';

	//! Vertex at the first PYXIS pole.
	static const int knVertexPole1 = 1;

	//! Vertex at the second PYXIS pole.
	static const int knVertexPole2 = 12;

	//! The combination of resolution 0 and 1
	int m_nPrimaryResolution;

	//! The sub-index within the face/vertex.
	PYXIndex m_pyxIndex;
};

//! Allows PYXIS indices to be written to streams.
PYXLIB_DECL std::ostream& operator<< (std::ostream& out, const PYXIcosIndex& pyxIndex);

//! Allows PYXIS indices to be read from streams.
PYXLIB_DECL std::istream& operator>> (std::istream& input, PYXIcosIndex& pyxIndex);

//! Addition operator.
PYXLIB_DECL PYXIcosIndex operator+(PYXIcosIndex lhs, const PYXIcosIndex& rhs);

#endif