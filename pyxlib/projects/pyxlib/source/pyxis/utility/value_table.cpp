/******************************************************************************
value_table.cpp

begin		: 2006-05-10
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/value_table.h"

// pyxlib includes
#include "pyxis/utility/exceptions.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value_column.h"

// standard includes
#include <sstream>

//! The unit test class
Tester<PYXValueTable> gTester;

/*!
The unit test method for the class.
*/
void PYXValueTable::test()
{
	// allocate a PYXValueTable implementation object
	std::vector<PYXValue::eType> vecTypes;
	vecTypes.push_back(PYXValue::knInt16);
	vecTypes.push_back(PYXValue::knFloat);
	vecTypes.push_back(PYXValue::knFloat);
	vecTypes.push_back(PYXValue::knString);
	std::vector<int> vecCounts;
	vecCounts.push_back(1);
	vecCounts.push_back(1);
	vecCounts.push_back(3);
	vecCounts.push_back(1);
	PYXValueTable vt(2, vecTypes, vecCounts);

	// uncomment to verify that out-of-bounds accesses cause assert failure
	//vt.getColumnType(-1);
	//vt.getColumnType(10);

	// check the column types and counts
	TEST_ASSERT(vt.getColumnType(0) == PYXValue::knInt16);
	TEST_ASSERT(vt.getCount(0) == 1);
	TEST_ASSERT(vt.getColumnType(1) == PYXValue::knFloat);
	TEST_ASSERT(vt.getCount(1) == 1);
	TEST_ASSERT(vt.getColumnType(2) == PYXValue::knFloat);
	TEST_ASSERT(vt.getCount(2) == 3);
	TEST_ASSERT(vt.getColumnType(3) == PYXValue::knString);
	TEST_ASSERT(vt.getCount(3) == 1);

	// create some values
	PYXValue myInt16((int16_t)100);
	PYXValue myFloat((float)1000.0);
	float rgb[3]; rgb[0] = 100.; rgb[1] = 200.; rgb[2] = 300.;
	PYXValue myRGB(rgb, 3);
	std::string myStr("yahoo");
	PYXValue myString(myStr);

	// uncomment to verify that incorrect type in setValue() causes assert failure
	//vt.setValue(0,0,myFloat);
	// uncomment to verify that out-of-bounds accesses cause assert failure
	//vt.setValue(-1,0,myFloat);
	//vt.setValue(10,0,myFloat);
	//vt.setValue(0,-1,myFloat);
	//vt.setValue(0,10,myFloat);

	// populate just row 0 of the table; row 1 should remain null
	vt.setValue(0, 0, myInt16);
	vt.setValue(0, 1, myFloat);
	vt.setValue(0, 2, myRGB);
	vt.setValue(0, 3, myString);

	// retrieve values and check that they are correct (true value equivalence)
	PYXValue myOtherInt16((int16_t)100);
	PYXValue myOtherFloat((float)1000.0);
	PYXValue myOtherRGB(rgb, 3);
	PYXValue myOtherString(myStr);
	TEST_ASSERT(vt.getValue(0,0) == myOtherInt16);
	TEST_ASSERT(vt.getValue(0,1) == myOtherFloat);
	TEST_ASSERT(vt.getValue(0,2) == myOtherRGB);
	TEST_ASSERT(vt.getValue(0,3) == myOtherString);
	TEST_ASSERT(vt.getValue(1,0).isNull());
	TEST_ASSERT(vt.getValue(1,1).isNull());
	TEST_ASSERT(vt.getValue(1,2).isNull());
	TEST_ASSERT(vt.getValue(1,3).isNull());

	// uncomment to print this value table to trace output
	//TRACE_INFO("PYXValueTable diagnostic printout" << std::endl << *pvt);

	// serialize to test directory
	std::ostringstream out;
	vt.serialize(out);

	// try to read back
	PYXValueTable vt2;
	std::istringstream in(out.str());
	vt2.deserialize(in);

	// verify equivalence
	TEST_ASSERT(vt2.getNumberOfRows() == vt.getNumberOfRows());
	TEST_ASSERT(vt2.getNumberOfColumns() == vt.getNumberOfColumns());
	TEST_ASSERT(vt2.m_vecColumnType == vt.m_vecColumnType);
	TEST_ASSERT(vt2.getValue(0,0) == myOtherInt16);
	TEST_ASSERT(vt2.getValue(0,1) == myOtherFloat);
	TEST_ASSERT(vt2.getValue(0,2) == myOtherRGB);
	TEST_ASSERT(vt2.getValue(0,3) == myOtherString);
	TEST_ASSERT(vt2.getValue(1,0).isNull());
	TEST_ASSERT(vt2.getValue(1,1).isNull());
	TEST_ASSERT(vt2.getValue(1,2).isNull());
	TEST_ASSERT(vt2.getValue(1,3).isNull());
	
	// TODO: test the copy constructor.
}

/*!
Simple constructor: use when all columns have non-array type.

\param nRows	Number of rows to allocate for this table.
\param vecTypes	Vector of base types for the columns.
*/
PYXValueTable::PYXValueTable(	const int nRows,
								const std::vector<PYXValue::eType>& vecTypes	) :
	m_vecColumnType(vecTypes)
{
	int nColumns = static_cast<int>(vecTypes.size());
	for (int nCol = 0; nCol < nColumns; ++nCol)
	{
		assert(vecTypes[nCol] != PYXValue::knNull);
		assert(vecTypes[nCol] != PYXValue::knArray);
		assert(vecTypes[nCol] != PYXValue::knReservedForInt64);
		assert(vecTypes[nCol] != PYXValue::knReservedForUInt64);
		m_vecColumnData.push_back(new PYXValueColumn(vecTypes[nCol], nRows));
	}
}

/*!
Memory estimator to be used before simple constructor.

\param nRows	Number of rows to allocate for this table.
\param vecTypes	Vector of base types for the columns.

\return			Number of bytes expected to be allocated on heap.
*/
const int PYXValueTable::estimateHeapBytes(
	const int nRows,
	const std::vector<PYXValue::eType>& vecTypes	)
{
	int nBytes = sizeof(PYXValueTable);
	for (int nCol = 0; nCol < static_cast<int>(vecTypes.size()); nCol++)
	{
		nBytes = PYXValueColumn::estimateHeapBytes(vecTypes[nCol], nRows);
	}
	return nBytes;
}

/*!
General constructor: use when one or more columns have array type.

\param nRows		Number of rows to allocate for this table.
\param vecTypes		Vector of base types for the columns.
\param vecCounts	Vector of array counts for the columns.
*/
PYXValueTable::PYXValueTable(	const int nRows,
								const std::vector<PYXValue::eType>& vecTypes,
								const std::vector<int>& vecCounts
								) :
	m_vecColumnType(vecTypes)
{
	assert(vecTypes.size() == vecCounts.size());

	int nColumns = static_cast<int>(vecTypes.size());
	for (int nCol = 0; nCol < nColumns; ++nCol)
	{
		assert(vecTypes[nCol] != PYXValue::knNull);
		assert(vecTypes[nCol] != PYXValue::knArray);
		assert(vecTypes[nCol] != PYXValue::knReservedForInt64);
		assert(vecTypes[nCol] != PYXValue::knReservedForUInt64);
		m_vecColumnData.push_back(new PYXValueColumn(vecTypes[nCol], nRows, vecCounts[nCol]));
	}
}

/*!
Memory estimator to be used before general constructor.

\param nRows		Number of rows to allocate for this table.
\param vecTypes		Vector of base types for the columns.
\param vecCounts	Vector of array counts for the columns.

\return			Number of bytes expected to be allocated on heap.
*/
const int PYXValueTable::estimateHeapBytes(
	const int nRows,
	const std::vector<PYXValue::eType>& vecTypes,
	const std::vector<int>& vecCounts
	)
{
	int nBytes = sizeof(PYXValueTable);
	for (int nCol = 0; nCol < static_cast<int>(vecTypes.size()); nCol++)
	{
		nBytes = PYXValueColumn::estimateHeapBytes(vecTypes[nCol], nRows, vecCounts[nCol]);
	}
	return nBytes;
}

/*!
Copy constructor.
*/
PYXValueTable::PYXValueTable(const PYXValueTable& orig)
{
	m_vecColumnType = orig.m_vecColumnType;
	for (int nCol = 0; nCol < getNumberOfColumns(); ++nCol)
	{
		m_vecColumnData.push_back(new PYXValueColumn(*orig.m_vecColumnData[nCol]));
	}
}

/*!
Get a copy of a single channel of data.
*/
PYXValueTable::PYXValueTable(const PYXValueTable& orig, int nChannel)
{
	m_vecColumnType.push_back(orig.m_vecColumnType[nChannel]);
	m_vecColumnData.push_back(new PYXValueColumn(*orig.m_vecColumnData[nChannel]));
}

/*!
Destructor.
*/
PYXValueTable::~PYXValueTable()
{
	for (int nCol = 0; nCol < getNumberOfColumns(); ++nCol)
	{
		delete m_vecColumnData[nCol];
	}
}

/*!
Get an estimate of how much heap memory this PYXValueTable presently owns.
*/
int PYXValueTable::getHeapBytes() const
{
	int nHeapBytes = getNumberOfColumns() * sizeof(PYXValueColumn);
	for (int nCol = 0; nCol < getNumberOfColumns(); ++nCol)
	{
		nHeapBytes += m_vecColumnData[nCol]->getHeapBytes();
	}
	return nHeapBytes;
}

/*!
Get the number of table rows.
*/
inline int PYXValueTable::getNumberOfRows() const
{
	if (getNumberOfColumns() == 0)
	{
		return 0;
	}
	else
	{
		return m_vecColumnData[0]->getHeight();
	}
}

/*!
Get number of table columns.
*/
inline int PYXValueTable::getNumberOfColumns() const
{
	return static_cast<int>(m_vecColumnType.size());
}

/*!
Get the type for a specific column.
*/
PYXValue::eType PYXValueTable::getColumnType(const int nColumn) const
{
	assert ((nColumn >= 0) && (nColumn < getNumberOfColumns()));

	return m_vecColumnType[nColumn];
}

/*!
Get array count for given column.
*/
inline int PYXValueTable::getCount(const int nColumn) const
{
	return m_vecColumnData[nColumn]->getWidth();
}

/*!
Get value at given row and column.
*/
PYXValue PYXValueTable::getValue(	const int nRow,
									const int nColumn,
									bool* pbInitialized	) const
{
	assert((nRow >= 0) || (nRow < getNumberOfRows()));
	assert((nColumn >= 0) || (nColumn < getNumberOfColumns()));

	const PYXValue& value = m_vecColumnData[nColumn]->getValue(nRow, pbInitialized);

	assert(
		value.isNull() ||
		(!value.isArray() && (value.getType() == m_vecColumnType[nColumn])) ||
		(value.isArray() && (value.getArrayType() == m_vecColumnType[nColumn]) && 
			value.getArraySize() == getCount(nColumn))
		);

	return value;
}

/*!
Get value at given row and column.

/returns True if the data is not null data, and false if the data is null. 
         The data itself will not be set to null.
*/
bool PYXValueTable::getValue(	const int nRow,
								const int nColumn,
								PYXValue* pValue) const
{
	assert((nRow >= 0) || (nRow < getNumberOfRows()));
	assert((nColumn >= 0) || (nColumn < getNumberOfColumns()));

	bool bIsNotNull = m_vecColumnData[nColumn]->getValue(nRow, pValue);

	assert(
		pValue->isNull() ||
		(!pValue->isArray() && (pValue->getType() == m_vecColumnType[nColumn])) ||
		(pValue->isArray() && (pValue->getArrayType() == m_vecColumnType[nColumn]) && 
			pValue->getArraySize() == getCount(nColumn))
		);

	return bIsNotNull;
}

/*!
Set the value at a specific row and column.
*/
void PYXValueTable::setValue(	const int nRow,
								const int nColumn,
								const PYXValue& value	)
{
	assert((nRow >= 0) || (nRow < getNumberOfRows()));
	assert((nColumn >= 0) || (nColumn < getNumberOfColumns()));

	assert(
		value.isNull() ||
		(!value.isArray() && (value.getType() == m_vecColumnType[nColumn])) ||
		(value.isArray() && (value.getArrayType() == m_vecColumnType[nColumn]) && 
			value.getArraySize() == getCount(nColumn))
		);

	m_vecColumnData[nColumn]->setValue(nRow,value);
}

/*!
Current I/O format version: increment every time format changes.
*/
static int knIOFormatVersion = 1;

/*!
Serialize to stream.
*/
void PYXValueTable::serialize(std::ostream& out)
{
	out.write((char*)&knIOFormatVersion, sizeof(int));
	int nRows = getNumberOfRows();
	out.write((char*)&nRows, sizeof(int));
	int nColumns = getNumberOfColumns();
	out.write((char*)&nColumns, sizeof(int));

	for (int nCol = 0; nCol < getNumberOfColumns(); ++nCol)
	{
		int tc = m_vecColumnType[nCol];
		out.write((char*)&tc, sizeof(int));
	}

	for (int nCol = 0; nCol < getNumberOfColumns(); ++nCol)
	{
		m_vecColumnData[nCol]->serialize(out);
	}
}

/*!
De-serialize from stream.
*/
void PYXValueTable::deserialize(std::istream& in)
{
	int versionCode;
	in.read((char*)&versionCode, sizeof(int));
	switch (versionCode)
	{
	case 1:
		// read version-1 format
		int nRows;
		in.read((char*)&nRows, sizeof(int));
		int nColumns;
		in.read((char*)&nColumns, sizeof(int));

		m_vecColumnType.resize(nColumns);
		for (int nCol = 0; nCol < nColumns; ++nCol)
		{
			int tc;
			in.read((char*)&tc, sizeof(int));
			if (tc < 0 || tc >= PYXValue::knTypeCount)
			{
				throw PYXValueTableException("PYXValueTable: Invalid type code");
			}
			else m_vecColumnType[nCol] = static_cast<PYXValue::eType>(tc);
		}

		m_vecColumnData.resize(nColumns);
		for (int nCol =0; nCol < nColumns; ++nCol)
		{
			m_vecColumnData[nCol] = new PYXValueColumn();
			m_vecColumnData[nCol]->deserialize(in);
		}
		break;

	default:
		throw PYXValueTableException("PYXValueTable: unknown format version");
	}
}

/*!
Stream output operator: serialize to text stream.
*/
std::ostream& operator <<(std::ostream& out, PYXValueTable& pvt)
{
	out << "PYXValueTable: ";
	out << pvt.getNumberOfRows() << " rows, ";
	out << pvt.getNumberOfColumns() << " columns" << std::endl;

	for (int nCol = 0; nCol < pvt.getNumberOfColumns(); ++nCol)
	{
		out << "PYXValueTable column " << nCol << ":" << std::endl;
		out << *(pvt.m_vecColumnData[nCol]) << std::endl;
	}

	return out;
}

/*!
Stream input operator: de-serialize from text stream.
*/
std::istream& operator >>(std::istream& in, PYXValueTable& pvt)
{
	assert(false && "not yet implemented");
	return in;
}
