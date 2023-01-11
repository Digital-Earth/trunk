/******************************************************************************
index.cpp

begin		: 2004-01-28
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/index.h"

// pyxlib includes
#include "pyxis/derm/exceptions.h"
#include "pyxis/derm/hexagon.h"
#include "pyxis/derm/icosahedron.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/trace.h"

// standard includes
#include <cassert>
#include <iomanip>
#include <set>

//! The string used to represent a null label.
const std::string PYXIcosIndex::kstrNullIndexLabel = "NULL";

//! Tester class.
Tester<PYXIcosIndex> gTester;

//! Test method.
void PYXIcosIndex::test()
{
	const char* indexData[] =
	{
		"A-0000","A-0001","A-0002","A-0003","A-0004","A-0005","A-0006",
		"A-0010","A-0020","A-0030","A-0040","A-0050","A-0060",
		"A-0100","A-0101","A-0102","A-0103","A-0104","A-0105","A-0106",
		"A-0200","A-0201","A-0202","A-0203","A-0204","A-0205","A-0206",
		"A-0300","A-0301","A-0302","A-0303","A-0304","A-0305","A-0306",
		"A-0400","A-0401","A-0402","A-0403","A-0404","A-0405","A-0406",
		"A-0500","A-0501","A-0502","A-0503","A-0504","A-0505","A-0506",
		"A-0600","A-0601","A-0602","A-0603","A-0604","A-0605","A-0606"
	};
	const int knIndexDataSize = sizeof(indexData) / sizeof(indexData[0]);

	const std::string kstrTestDigits("1-20103040506010");
	const int knTestResolution = 15;

	// test that constructor creates null index
	PYXIcosIndex index1;
	TEST_ASSERT(index1.isNull());

	// test copy constructor
	PYXIcosIndex index2(index1);
	TEST_ASSERT(index1 == index2);
	TEST_ASSERT(index2.isNull());

	// test construction from string
	PYXIcosIndex index3(kstrTestDigits);
	TEST_ASSERT(index3.toString() == kstrTestDigits);

	{
		// test construction with invalid strings.
		PYXIcosIndex indexFail;
		TEST_ASSERT_EXCEPTION(indexFail = "X", PYXIndexException);
		TEST_ASSERT_EXCEPTION(indexFail = "15", PYXIndexException);
		
		// invalid face with a vertex child.
		TEST_ASSERT_EXCEPTION(indexFail = "A-20", PYXIndexException);
	}

	// test copy assignment
	index2 = index3;
	TEST_ASSERT(index2 == index3);
	TEST_ASSERT(index2.toString() == kstrTestDigits);

	// test resolution
	TEST_ASSERT(knTestResolution == index3.getResolution());

	int nResolution = knTestResolution - 2;
	const std::string kstrNewDigits(kstrTestDigits.substr(0, kstrTestDigits.length() - 2));
	index3.setResolution(nResolution);
	TEST_ASSERT(index3.getResolution() == nResolution);
	TEST_ASSERT(kstrNewDigits == index3.toString());

	// test equality and less than operators
	index1.reset();
	index2.reset();
	TEST_ASSERT(index1 == index2);
	TEST_ASSERT(!(index1 < index2));

	index1 = "1";
	TEST_ASSERT(index1 != index2);
	TEST_ASSERT(!(index1 < index2));

	index2 = "1";
	TEST_ASSERT(index1 == index2);
	TEST_ASSERT(!(index1 < index2));

	index2 = "A";
	TEST_ASSERT(index1 != index2);
	TEST_ASSERT(index1 < index2);

	// test reset()
	index3.reset();
	TEST_ASSERT(index3.isNull());
	TEST_ASSERT(0 > index3.getResolution());

	// test isVertex() and isFace() and isPentagon()
	index1.reset();
	TEST_ASSERT(!index1.isVertex());
	TEST_ASSERT(!index1.isFace());

	index1 = "1";
	TEST_ASSERT(index1.isVertex());
	TEST_ASSERT(!index1.isFace());

	index1 = "A";
	TEST_ASSERT(!index1.isVertex());
	TEST_ASSERT(index1.isFace());

	index1 = "1-000000000000000000";
	TEST_ASSERT(index1.isPentagon());

	index1 = "A-00000";
	TEST_ASSERT(!index1.isPentagon());

	// test isPentagon(unsigned int)
	{
		PYXIcosIndex index("1-0005");
		const int nDigitCount = index.getSubIndex().getDigitCount();
		TEST_ASSERT(!index.isPentagon(nDigitCount));
		TEST_ASSERT(!index.isPentagon(nDigitCount + 1));
		TEST_ASSERT(index.isPentagon(nDigitCount - 1));
		TEST_ASSERT(index.isPentagon(0));
	}
	{
		PYXIcosIndex index("A-01");
		const int nDigitCount = index.getSubIndex().getDigitCount();
		TEST_ASSERT(!index.isPentagon(nDigitCount));
		TEST_ASSERT(!index.isPentagon(nDigitCount + 1));
		TEST_ASSERT(!index.isPentagon(nDigitCount - 1));
		TEST_ASSERT(!index.isPentagon(0));
	}

	// test hasVertexChildren()
	index1.reset();
	TEST_ASSERT(!index1.hasVertexChildren());

	index1 = "01";
	TEST_ASSERT(index1.hasVertexChildren());

	index1 = "01-02";
	TEST_ASSERT(!index1.hasVertexChildren());

	index1 = "1-20";
	TEST_ASSERT(index1.hasVertexChildren());

	// test attribute access methods
	index1 = "4-204006";
	TEST_ASSERT(index1.getPrimaryResolution() == 4);

	index1 = "A-000";
	TEST_ASSERT(index1.getPrimaryResolution() == 'A');

	PYXIndex pyxIndex1 = "000";
	TEST_ASSERT(index1.getSubIndex() == pyxIndex1);

	// test isAncestorOf
	index1.reset();
	index2 = "01-03";
	TEST_ASSERT(!index1.isAncestorOf(index1));
	TEST_ASSERT(!index1.isAncestorOf(index2));
	TEST_ASSERT(!index2.isAncestorOf(index1));

	index1 = "A";
	index2 = "A-01";
	TEST_ASSERT(index1.isAncestorOf(index1));
	TEST_ASSERT(index1.isAncestorOf(index2));
	TEST_ASSERT(!index2.isAncestorOf(index1));

	// test streaming operators
	PYXIcosIndex writeIndex;
	PYXIcosIndex readIndex;

	// verify null index streaming
	std::stringstream testStream;
	testStream << writeIndex;
	std::string tempString = testStream.str();
	testStream >> readIndex;
	TEST_ASSERT(writeIndex == readIndex);

	// verify a series of indices
	for (int n = 0; n != knIndexDataSize; ++n)
	{
		std::stringstream testStream;
		writeIndex = PYXIcosIndex(indexData[n]);
		testStream << writeIndex;
		testStream >> readIndex;
		TEST_ASSERT(writeIndex == readIndex);
	}

	// test the addition operator; can't grow
	{
		index1 = "1-030";
		index2 = "1-201";

		TEST_ASSERT(index1 + index2 == PYXIcosIndex("1-203"));
		index1 += index2;
		TEST_ASSERT(index1 == PYXIcosIndex("1-203"));
	}

	// test increment and decrement resolution
	{
		index1 = "R-02003";
		index2 = "R-020030";
		index3 = "R-02003000";
		PYXIcosIndex index(index1);
		index.incrementResolution();
		TEST_ASSERT(index == index2);
		index.decrementResolution();
		TEST_ASSERT(index == index1);
		index.incrementResolution();
		index.incrementResolution();
		index.incrementResolution();
		TEST_ASSERT(index == index3);
		index.decrementResolution();
		index.decrementResolution();
		index.decrementResolution();
		TEST_ASSERT(index == index1);
		index.decrementResolution();
		index.decrementResolution();
		index.decrementResolution();
		index.decrementResolution();
		index.decrementResolution();
		TEST_ASSERT_EXCEPTION(index.decrementResolution(), PYXIndexException);
	}

	// TODO sort out max resolution
	// test randomization
	for (int nResolution = 2; nResolution <= PYXMath::knMaxAbsResolution - 1; ++nResolution)
	{
		PYXIcosIndex index;
		index.randomize(nResolution);
		TEST_ASSERT(index.getResolution() == nResolution);
	}

	// test getMaximumChildCount(unsigned int)
	{
		{
			PYXIcosIndex index("A-0");
			TEST_ASSERT(1 == index.getMaximumChildCount(0));
			TEST_ASSERT(7 == index.getMaximumChildCount(1));
			TEST_ASSERT(7 == index.getMaximumChildCount(2));
		}

		{
			PYXIcosIndex index("1-05");
			TEST_ASSERT(6 == index.getMaximumChildCount(0));
			TEST_ASSERT(6 == index.getMaximumChildCount(1));
			TEST_ASSERT(1 == index.getMaximumChildCount(2));
			TEST_ASSERT(1 == index.getMaximumChildCount(3));
		}
	}

	//[shatzi]: we no longer use the jason Tree index scheme - there is no need to run the unit test
	// Test the tree conversion.
	//testTreeConversion();
}

/*!
Constructor creates a null index.
*/
PYXIcosIndex::PYXIcosIndex()
{
	reset();
}

/*!
Copy constructor.

\param	rhs	The PYXIS index to copy.
*/
PYXIcosIndex::PYXIcosIndex(const PYXIcosIndex& rhs)
{
	// call copy assignment operator
	*this = rhs;
}

/*!
Construct the index from a string.

\param strIndex	The string.
*/
PYXIcosIndex::PYXIcosIndex(const std::string& strIndex)
{
	// call string assignment operator
	*this = strIndex;
}

/*!
Destructor does nothing
*/
PYXIcosIndex::~PYXIcosIndex()
{
}

/*!
Copy assignment.

\param	rhs	The index to copy.
*/
PYXIcosIndex& PYXIcosIndex::operator=(const PYXIcosIndex &rhs)
{
	reset();

	// copy the primary resolution
	m_nPrimaryResolution = rhs.m_nPrimaryResolution;

	// copy the sub-index
	m_pyxIndex = rhs.m_pyxIndex;

	return *this;
}

/*!
String assignment. The string must be of the following forms:
	P
	P-####

	PP
	PP-####

Where P is the vertex [00-12] or the face [A-T] and #### is the sub-index.

\param	strIndex	The index in the proper format or the word 'NULL'.
*/
PYXIcosIndex& PYXIcosIndex::operator=(const std::string& strIndex)
{
	bool bCreateSubIndex = false;
	int nStringVal = 0;

	// check the format of the string
	switch (strIndex.length())
	{
		// 0 characters will produce a null index
		case 0:
			m_nPrimaryResolution = knNullPrimaryRes;
			m_pyxIndex.reset();
			break;

		// 1 character must be a valid face or vertex value
		case 1:
			// check to see if the index is a letter (face identifier
			if (isalpha(strIndex.at(0)))
			{
				nStringVal = strIndex.at(0);
			}

			// Fall through to case 2

		// 2 characters must be a 2 digit vertex 
		case 2:
			if (0 == nStringVal)
			{
				nStringVal = atoi(strIndex.substr(0, strIndex.length()).c_str());
			}

			// verify the validity of the value and assign
			if (isValidPrimary(nStringVal))
			{
				m_nPrimaryResolution = nStringVal;

				// reset the sub index
				m_pyxIndex = PYXIndex();
			}
			else
			{
				PYXTHROW(PYXIndexException, "Index '" << strIndex << "' is invalid.");
			}
			break;

		// can be the string NULL 
		case 4:
			// if a null string then reset the index otherwise fall through
			if (strIndex == kstrNullIndexLabel)
			{
				reset();
				break;
			}

			// fall through to default

		// can contain a sub index
		default:

			// find the dash '-' character
			size_t nDashPos = strIndex.find("-", 0);
			if (0 >= nDashPos)
			{
				PYXTHROW(PYXIndexException, "Index '" << strIndex << "' is invalid.");
			}
			else if (1 == nDashPos)
			{
				if (isalpha(strIndex.at(0)))
				{
					nStringVal = strIndex.at(0);
				}
			}

			// parse out the primary resoltion
			if (0 == nStringVal)
			{
				nStringVal = atoi(strIndex.substr(0, nDashPos).c_str());
			}

			if (isValidPrimary(nStringVal))
			{
				m_nPrimaryResolution = nStringVal;
			}
			else
			{
				PYXTHROW(PYXIndexException, "Index '" << strIndex << "' is invalid.");
			}
		
			// parse out the sub resolution
			try
			{
				nDashPos++;
				m_pyxIndex = strIndex.substr((nDashPos), 
											 strIndex.length() - (nDashPos));
			}
			catch (PYXException& e)
			{
				PYXRETHROW(e, PYXIndexException, "Index '" << strIndex << "' is invalid.");
			}
	}

	if (!PYXIcosMath::isValidIndex(*this))
	{
		PYXTHROW(PYXIndexException, "Index '" << strIndex << "' is invalid.");
	}

	return *this;
}

/*!
Set the PYXIcosIndex to null.
*/
void PYXIcosIndex::reset()
{
	m_nPrimaryResolution = knNullPrimaryRes;
	m_pyxIndex.reset();
}

/*!
Convert an index to a string excluding null values.

\return	The string.
*/
std::string PYXIcosIndex::toString() const
{
	std::ostringstream ost;

	// resolution 0
	if (isNull())
	{
		ost << kstrNullIndexLabel;
	}
	else
	{
		// Add the face or vertex value
		if (0 != m_nPrimaryResolution)
		{
			if (isFace())
			{
				ost << static_cast<char>(m_nPrimaryResolution);
			}
			else
			{
				ost << m_nPrimaryResolution;
			}
		}

		// add the sub-index
		if (!m_pyxIndex.isNull())
		{
			ost << '-' << m_pyxIndex.toString();
		}
	}
	return ost.str();
}

/*! 
Allows PYXIS indices to be written to streams.

\param out		The stream to write to.
\param pyxIndex	The index to write to the stream.

\return The stream after the operation.
*/
std::ostream& operator<< (std::ostream& out, const PYXIcosIndex& pyxIndex)
{
	out << pyxIndex.getPrimaryResolution();
	if (!pyxIndex.isNull())
	{
		out << " ";
		out << pyxIndex.getSubIndex();
	}
	return out;
}

/*!
Allows PYXIS indices to be read from streams.

\param input	The stream to read from.
\param pyxIndex	The index read from the stream.

\return The stream after the operation.
*/
std::istream& operator>> (std::istream& input, PYXIcosIndex& pyxIndex)
{
	int nPrimary;
	input >> nPrimary;
	if (nPrimary == PYXIcosIndex::knNullPrimaryRes)
	{
		pyxIndex.reset();
	}
	else
	{
		pyxIndex.setPrimaryResolution(nPrimary);
		input >> pyxIndex.getSubIndex();
	}
	return input;
}

/*!
Addition operator.

\param lhs	The first index to add.
\param rhs	The second index to add.

\return The addition of two indexes.
*/
PYXIcosIndex operator+(PYXIcosIndex lhs, const PYXIcosIndex& rhs)
{
	return lhs += rhs;
}

/*!
Equality operator.

\return	true if the indices are equal, otherwise false.
*/
bool PYXIcosIndex::operator==(const PYXIcosIndex& rhs) const
{
	return	((m_nPrimaryResolution == rhs.m_nPrimaryResolution) &&
			(m_pyxIndex == rhs.m_pyxIndex)	);
}

/*!
Less than operator. Returns true if this index has a lower resolution than the
specified index or, if the resolutions are the same, this index comes before
the specified index in an exhaustive iteration.

\param	rhs	The index to compare.

\return true if this index is less than the index passed in.
*/
bool PYXIcosIndex::operator<(const PYXIcosIndex& rhs) const
{
	if (m_nPrimaryResolution < rhs.m_nPrimaryResolution)
	{
		return true;
	}
	else if (m_nPrimaryResolution > rhs.m_nPrimaryResolution)
	{
		return false;
	}
	
	// primary resolutions are the same, look at sub-index
	return (m_pyxIndex < rhs.m_pyxIndex);
}

/*!
Addition and return operator. Returns the addition of the two indexs.

Will use the resolution of the index where the addition is performed.

\param	rhs	The index to be added.

\return The sum index after add..

\return true if this index is less than the index passed in.
*/
PYXIcosIndex& PYXIcosIndex::operator +=(const PYXIcosIndex& rhs)
{
	*this = PYXIcosMath::add(*this, rhs, getResolution());

	return *this;
}

/*!
Get the resolution of the index.

\return	The resolution or -1 if the index is null.
*/
int PYXIcosIndex::getResolution() const
{
	int nResolution = -1;

	if (!isNull())
	{
		// get the resolution of the subindex
		nResolution = m_pyxIndex.getResolution();

		// check to see if the subindex is null
		if (nResolution < 0)
		{	
			// the primary resolution will always be represented as 1
			return knResolution1;
		}
		else
		{
			// the subindex total plus the first two resolutions is the total
			nResolution += PYXIcosIndex::knMinSubRes;
		}
	}

	return nResolution;
}

/*!
Set the resolution of the PYXIS index. This method will not change the 
resolution of a null index. This method will not allow the index to be set to
resolution zero.

\param nResolution	The new resolution.
*/
void PYXIcosIndex::setResolution(int nResolution)
{
	if (!isNull())
	{
		// perform the resolution change on the remainder
		switch (nResolution)
		{
		// can't set the resolution to 0 with this method
		case knResolution0:
			PYXTHROW(	PYXIndexException,
						"Invalid index: '" << knResolution0 << "'."	);
			break;

		// set the index to resolution 1
		case knResolution1:
			m_pyxIndex.reset();
			break;

		// set the index to all other resolutions
		default:
			if (nResolution > PYXMath::knMaxAbsResolution)
			{
				PYXTHROW(	PYXIndexException,
							"Invalid resolution: '" << nResolution << "'."	);
			}
			else
			{
				m_pyxIndex.setResolution(nResolution - PYXIcosIndex::knMinSubRes);
			}
			break;
		}
	}
	else
	{	
		PYXTHROW(	PYXIndexException,
					"Unable to set resolution on null index."	);
	}
}

/*! 
Increase the the resolution by 1 by adding a '0' onto the right hand side of
the index.
*/
void PYXIcosIndex::incrementResolution()
{
	if (!isNull())
	{
		if (	(m_pyxIndex.m_nDigitCount + PYXIcosIndex::knResolution1) 
				< PYXMath::knMaxAbsResolution	)
		{
			m_pyxIndex.m_pcDigits[m_pyxIndex.m_nDigitCount++] = '0';
			m_pyxIndex.m_pcDigits[m_pyxIndex.m_nDigitCount] = 0;
		}
		else
		{
			PYXTHROW(	PYXIndexException,
						"Invalid resolution: '" << (getResolution() + 1) << "'."	);
		}
	}
	else
	{
		PYXTHROW(	PYXIndexException,
					"Unable to set resolution of null index."	);
	}
}

/*! 
Decrease the the resolution by 1 by removing the right hand digit of
the index.
*/
void PYXIcosIndex::decrementResolution()
{
	if (!isNull())
	{
		if (0 < m_pyxIndex.m_nDigitCount)
		{
			m_pyxIndex.m_pcDigits[--m_pyxIndex.m_nDigitCount] = 0;
		}
		else
		{
			PYXTHROW(	PYXIndexException,
						"Invalid resolution: '" << (getResolution() - 1) << "'."	);
		}
	}
	else
	{
		PYXTHROW(	PYXIndexException,
					"Unable to set resolution of null index."	);
	}
}

/*!
Set the values for resolution 0 and 1.  This method does not reset the
sub-resolution.

\param	nPrimaryValue	The new value of the primary resolution.
*/
void PYXIcosIndex::setPrimaryResolution(int nPrimaryValue)
{
	if (!isValidPrimary(nPrimaryValue))
	{
		PYXTHROW(	PYXIndexException,
					"Invalid primary resolution: '" << nPrimaryValue << "'."	);
	}
	m_nPrimaryResolution = nPrimaryValue;
}

/*!
Does this index represent a vertex at resolution 1.

\return	True if this index is a vertex or false not.
*/
bool PYXIcosIndex::isVertex() const
{
	return ((m_nPrimaryResolution >= knFirstVertex) &&
			(m_nPrimaryResolution <= knLastVertex));
}

/*!
Determine if this index is on a PYXIS pole.

\return true if this index is on a PYXIS pole
*/
bool PYXIcosIndex::isPolar() const
{
	if (isNull())
	{
		PYXTHROW(PYXIndexException, "Null index.");
	}

	return((m_nPrimaryResolution == knVertexPole1) ||
		   (m_nPrimaryResolution == knVertexPole2));

}

/*!
Does this index represent a face at resolution 1.

\return True if this index is a face or false if the index is a vertex or if
		the index is null.
*/
bool PYXIcosIndex::isFace() const
{
	return ((m_nPrimaryResolution >= kcFaceFirstChar) &&
			(m_nPrimaryResolution <= kcFaceLastChar));
}

/*!
Determine if this PYXIS icos index is a centroid child of its parent.
The exception to this method is the resolution 0 hexagon which returns
true when it is the highest resolution.

\return	true if this index is its parent's centroid child otherwise false.
*/
bool PYXIcosIndex::hasVertexChildren() const
{
	bool bIsCentroidChild = true;

	if (!isNull())
	{
		// if the subindex is non-null then defer to it
		if (!getSubIndex().isNull())
		{
			bIsCentroidChild = getSubIndex().hasVertexChildren();
		}
		else if (isFace())
		{
			// the only false for the first 2 resolutions is on a face
			bIsCentroidChild = false;
		}
	}
	else
	{
		bIsCentroidChild = false;
	}

	return bIsCentroidChild;
}

/*!
Check for valid primary resolution value.  This method will not allow
the index to be set to null, use the reset method.

\param	nResValue	The vertex or face identifier to be validated

\return	true if it is a valid combination, otherwise false.
*/
bool PYXIcosIndex::isValidPrimary(int nResValue)
{
	// check for a valid face range 
	if ((nResValue >= kcFaceFirstChar) &&
		(nResValue <= kcFaceLastChar))
	{
		return true;
	}
	
	// check to see if it is a valid vertex
	if ((nResValue >= knFirstVertex) &&
		(nResValue <= knLastVertex))
	{
		return true;
	}

	return false;	
}

/*
This method is used to determine if an index is the centroid child of an
icosahedron vertex at any resolution.

\return True if the index is a pentagon otherwise false
*/
bool PYXIcosIndex::isPentagon() const
{
	bool bIsPentagon = false;

	if (!isNull())
	{
		// if the parent of this index is centred on an icosahedron vertex
		if (isVertex())
		{
			if (getSubIndex().isNull())
			{
				// we are on a pentagon on the primary resolution
				bIsPentagon = true;
			}
			else
			{
				// if the subindex is all 0's then we are a direct descendant
				if (getSubIndex().isAtOrigin())
				{
					bIsPentagon = true;
				}
			}
		}
	}
	
	return bIsPentagon;
}

/*
This method is used to determine if an index segment is the centroid 
child of an icosahedron vertex at any resolution.

\param nSubIndexDigitCount	The size of the subindex segment.
\return	True if the index segment is a pentagon; otherwise false.
*/
bool PYXIcosIndex::isPentagon(unsigned int nSubIndexDigitCount) const
{
	if (!isNull())
	{
		// if the parent of this index is centred on an icosahedron vertex
		if (isVertex())
		{
			if (getSubIndex().isNull() || nSubIndexDigitCount == 0)
			{
				// we are on a pentagon on the primary resolution
				return true;
			}

			// if the subindex is all 0's then we are a direct descendant
			if (getSubIndex().isAtOrigin(nSubIndexDigitCount))
			{
				return true;
			}
		}
	}
	
	return false;
}

/*!
Determine if the index is an ancestor of the specified index. A is an ancestor
of B if A and B are the same index or if B is generated by a tesselation of A
at some given resolution. If either A or B or both are the null index then
there is no ancestor relationship.

\param	index	The specified index.

\return	true if this index is an ancestor of the specified index, otherwise
		false.
*/
bool PYXIcosIndex::isAncestorOf(const PYXIcosIndex& index) const
{
	bool bIsAncestor = false;

	// the null index is a parent of everything
	if (!isNull() && !index.isNull())
	{
		if (m_nPrimaryResolution == index.m_nPrimaryResolution)
		{
			if (getSubIndex().isNull())
			{
				bIsAncestor = true;
			}
			else
			{
				bIsAncestor = getSubIndex().isAncestorOf(index.getSubIndex());
			}
		}
	}

	return bIsAncestor;
}

/*!
Determine if this index is a descendant of the specified index. A is a
descendant of B if B is an ancestor of A.

\param	index	The specified index.

\return	true if this index is a descendant of the specified index, otherwise
		false.
*/
bool PYXIcosIndex::isDescendantOf(const PYXIcosIndex& index) const
{
	// call the ancestor method with the parameters switched
	return index.isAncestorOf(*this);
}

/*!
This method determines if the index represents a pentagon or a hexagon and 
returns the appropriate number of sides.

\return The number of sides on the shape represented by this index
*/
int PYXIcosIndex::determineNumSides() const
{
	if (isPentagon())
	{
		return Hexagon::knNumSides - 1;
	}

	return Hexagon::knNumSides;
}

/*!
Normalizes the resolution

\param pnResolution The resolution to normalize.
*/
void PYXIcosIndex::normalizeResolution(int* pnResolution)
{
	if (*pnResolution < knMinSubRes) 
	{
		*pnResolution = knMinSubRes;
	}
	else if (*pnResolution > PYXMath::knMaxAbsResolution)
	{
		*pnResolution = PYXMath::knMaxAbsResolution;
	}
}

/*!
Randomizes this index at the specified resolution.

Note that the current implementation randomizes a lat/lon, then converts to a
PYXIcosIndex. This will therefore skew results away from the equator toward
the poles, and slightly away from the 12 pentagons (due to smaller area).

\param nResolution	The resolution.
*/
void PYXIcosIndex::randomize(int nResolution)
{
	// TODO mlepage should do a proper randomize method not using SnyderProjection.
	CoordLatLon coord;
	coord.randomize();
	SnyderProjection::getInstance()->nativeToPYXIS(coord, this, nResolution);
}

/*!
Get the maximum child count for an index segment.

\param nSubIndexDigitCount	The size of the index segment.
\return	The maximum number of children of the cell described by the index segment.
*/
unsigned int PYXIcosIndex::getMaximumChildCount(unsigned int nSubIndexDigitCount) const
{
	// Get the subindex.
	const PYXIndex& index = getSubIndex();

	// If subindex digit count is too high, bring it down to the subindex size.
	{
		assert(0 <= index.getDigitCount());
		const unsigned int nDigitCount = index.getDigitCount();
		if (nSubIndexDigitCount > nDigitCount)
		{
			nSubIndexDigitCount = nDigitCount;
		}
	}

	// If the segment size is 0, return the child count for the primary index.
	if (nSubIndexDigitCount == 0)
	{
		return isVertex() ? 6 : 1;
	}

	// Get the subindex digit.  If it is a vertex, return 1.
	if (index.getDigit(nSubIndexDigitCount - 1) != 0)
	{
		return 1;
	}

	// If it's a pentagon, return 6.
	if (isPentagon(nSubIndexDigitCount))
	{
		return 6;
	}

	// It's a hexagon centroid.  Return 7.
	return 7;
}
