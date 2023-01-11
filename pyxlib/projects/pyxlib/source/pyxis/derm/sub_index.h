#ifndef PYXIS__DERM__SUB_INDEX_H
#define PYXIS__DERM__SUB_INDEX_H
/******************************************************************************
sub_index.h

begin		: 2003-12-03
copyright	: (C) 2003 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

// standard includes
#include <string>

// The maximum number of digits in a PYXindex. Must match knMaxAbsResolution in PYXMath.
const int knMaxDigits = 40;

/*!
The PYXIndex class identifies a unique cell and grid resolution in a tesselated
hexagon grid. The digits are implemented as an array of char instead of a
std::string for performance reasons.
*/
//! Identifies a unique cell in a hex grid.
class PYXLIB_DECL PYXIndex
{
public:

	//! Test method
	static void test();

	//! Bit mask for the digit
	static const unsigned int knDigitMask;

	//! Number of bits per digit
	static const int knBitsPerDigit;

public:

	//! Constructor
	PYXIndex();

	//! Copy constructor
	PYXIndex(const PYXIndex& rhs);

	//! Construct from a string
	PYXIndex(const std::string& strIndex);

	//! Destructor
	virtual ~PYXIndex();
	
	//! Copy assignment.
	PYXIndex& operator =(const PYXIndex &rhs);

	//! String assignment.
	PYXIndex& operator =(const std::string& strIndex);

	//! Set the PYXIndex to the null index
	void reset();

	/*!
	Get the resolution of the PYXIS index

	\return	The resolution. (-1 if index is null.)
	*/
	int getResolution() const {return getDigitCount() - 1;}

	/*!
	Set the resolution of the PYXIS index.

	\param nResolution	The resolution.
	*/
	void setResolution(int nResolution) {setDigitCount(nResolution + 1);}

	//! Convert an index to a string.
	std::string toString() const {return std::string(m_pcDigits);}

	//! Equality operator.
	bool operator ==(const PYXIndex& rhs) const;

	//! Inequality operator
	bool operator !=(const PYXIndex& rhs) const {return !(*this == rhs);}

	//! Less than operator.
	bool operator <(const PYXIndex& index) const;

	//! Add and return operator.
    PYXIndex& PYXIndex::operator +=(const PYXIndex& rhs);

	//! Subtract and return operator.
	PYXIndex& PYXIndex::operator -=(const PYXIndex& rhs);

	/*!
	Determine if this is a null index.

	\return	true if this is a null index, otherwise false
	*/
	//! Is this a null index
	bool isNull() const {return (0 == m_nDigitCount);}

	//! Is this hexagon centred on the origin
	bool isAtOrigin() const;

	//! Is the hexagon represented by the index segment centred on the origin
	bool isAtOrigin(unsigned int nDigitCount) const;

	//! Determine if this PYXIS index is a centroid child of its parent
	bool hasVertexChildren() const;

	//! Append a single digit to the right hand side of an index.
	void appendDigit(unsigned int nDigit);

	//! Append a pair of digits to the right hand side of the index.
	void appendPair(unsigned int nHexDigits);

	//! Prepend a single digit to the left hand side of the index.
	void prependDigit(unsigned int nDigit);

	//! Prepend a pair of digits to the left hand side of the index.
	void prependPair(unsigned int nHexDigits);

	//! Negate a PYXIS index.
	void negate();

	//! Determine if this index is an ancestor of the specified index
	bool isAncestorOf(const PYXIndex& index) const;

	//! Determine if this index is a descendant of the specified index
	bool isDescendantOf(const PYXIndex& index) const;

	//! Strip the least significant digit from the index.
	unsigned int stripRight();

	//! Strip the two least significant digits from the index.
	unsigned int stripRightPair();

	//! Strip the most significant digit from the index.
	unsigned int stripLeft();

	//! Strip the most significant pair of digits from the index. 
	unsigned int stripLeftPair(); 

	//! Randomizes this index.
	void randomize(int nResolution);

	/*!
	Get the number of digits in the PYXIS index.

	\return	The number of digits in the PYXIS index.
	*/
	int getDigitCount() const {return m_nDigitCount;}

	//! Append an index to the right side of this one.
	void append(const PYXIndex& index);

	//! Get the digit at a given position.
	unsigned int getDigit(int nPosition) const;

	//! Get the digit at the end of the index
	unsigned int getLastDigit() const;

	//! Returns a subsequence of digits.
	PYXIndex subseq(int nPos, int nLen = INT_MAX) const;

	//! Get the most significant non-zero digit in the index.
	unsigned int mostSignificant(int* pnPosition) const;

private:

	/*!
	Determine if a digit is valid for a PYXIS index.

	\param	nDigit	The digit.

	\return	true if the digit is valid, otherwise false.
	*/
	//! Is the digit a valid PYXIS digit?
	static bool isPYXDigit(unsigned int nDigit)
	{
		return ((0 <= nDigit) && (6 >= nDigit));
	};

private:

	/*!
	Set the number of digits in the PYXIS index. Unused digits are set to zero.
	*/
	void setDigitCount(int nDigitCount);

	//! Set the value of the PYXIS index
	void set(unsigned int nValue, int nBase = 10);

	//! Set the digit at a given position.
	void setDigit(int nPosition, unsigned int nDigit);

	//! Strip a number of digits from the left of the index
	void stripLeftCount(int nCount);

	//! Cut the index into parts at the designated point
	PYXIndex split(int nCount);

	//! Split the index into two new indexes.
	void split(PYXIndex* pTail);

	//! Prepend an index to the left side of this one.
	void prepend(const PYXIndex& index);

	//! Attempt to adjust the resolution by adding/removing digits on the left.
	void adjustResolutionLeft(int nTargetResolution);

private:

	//! Give PYXMath access to private methods and data
	friend class PYXMath;

	//! Give PYXIcosIndex access to private methods and data
	friend class PYXIcosIndex;

	//! Give PYXIcosMath access to private methods and data
	friend class PYXIcosMath;

	//! Give PYXExhaustiveIterator access to private methods and data.
	friend class PYXExhaustiveIterator;

private:

	//! The number of digits (not including null terminator). (TODO document valid range)
	int m_nDigitCount;

	//! Where the digits are stored.
	char m_pcDigits[knMaxDigits + 1];
};

//! Allows PYXIS indices to be written to streams.
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const PYXIndex& pyxIndex);

//! Allows PYXIS indices to be read from streams.
PYXLIB_DECL std::istream& operator >>(std::istream& input, PYXIndex& pyxIndex);

//! Addition operator.
PYXLIB_DECL PYXIndex operator +(PYXIndex lhs, const PYXIndex& rhs);

//! Subtraction operator.
PYXLIB_DECL PYXIndex operator -(PYXIndex lhs, const PYXIndex& rhs);

#endif // guard
