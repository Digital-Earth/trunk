/******************************************************************************
value_tile.cpp

begin		: 2006-05-10
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/data/value_tile.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/data/record.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/memory_manager.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value_math.h"
#include "pyxis/utility/value_table.h"
#include "pyxis/utility/xml_utils.h"

// third party includes
#include "zlib.h"

// standard includes
#include <sstream>

//! The unit test class
Tester<PYXValueTile> gTester;

/*!
The unit test method for the class.
*/
void PYXValueTile::test()
{
	//TODO [shatzi]: add unit testing to make sure we have memory tokens for each PYXValueTile!
	//TODO [shatzi]: add unit testing to make sure that the memory token is in the right size!

	// create a value tile with 4 channels, one of which is has array type
	const PYXIcosIndex myRootIndex("A-0");
	const int myCellResolution = 6;
	PYXTile myTile(myRootIndex, myCellResolution);
	std::vector<PYXValue::eType> vecTypes;
	vecTypes.push_back(PYXValue::knInt16);
	vecTypes.push_back(PYXValue::knFloat);
	vecTypes.push_back(PYXValue::knFloat);	// array type: float[3]
	vecTypes.push_back(PYXValue::knString);
	std::vector<int> vecCounts;
	vecCounts.push_back(1);
	vecCounts.push_back(1);
	vecCounts.push_back(3);							// array type: float[3]
	vecCounts.push_back(1);

	// create a table definition that matches the above.
	PYXPointer<PYXTableDefinition> spDefn = PYXTableDefinition::create();
	spDefn->addFieldDefinition("first value", PYXFieldDefinition::knContextNone, PYXValue::knInt16, 1);
	spDefn->addFieldDefinition("second value", PYXFieldDefinition::knContextNone, PYXValue::knFloat, 1);
	spDefn->addFieldDefinition("third value", PYXFieldDefinition::knContextNone, PYXValue::knFloat, 3);
	spDefn->addFieldDefinition("fourth value", PYXFieldDefinition::knContextNone, PYXValue::knString, 1);

	PYXValueTile vt(myTile, vecTypes, vecCounts);

	// verify number of channels
	TEST_ASSERT(vt.getNumberOfDataChannels() == 4);

	// check the column types and counts
	TEST_ASSERT(vt.getDataChannelType(0) == PYXValue::knInt16);
	TEST_ASSERT(vt.getDataChannelCount(0) == 1);
	TEST_ASSERT(vt.getDataChannelType(1) == PYXValue::knFloat);
	TEST_ASSERT(vt.getDataChannelCount(1) == 1);
	TEST_ASSERT(vt.getDataChannelType(2) == PYXValue::knFloat);
	TEST_ASSERT(vt.getDataChannelCount(2) == 3);
	TEST_ASSERT(vt.getDataChannelType(3) == PYXValue::knString);
	TEST_ASSERT(vt.getDataChannelCount(3) == 1);
	
	// verify field definition
	TEST_ASSERT(vt.isValueTileCompatible(spDefn));
	PYXValueTile vtDefn(myTile, spDefn);
	TEST_ASSERT(vtDefn.isValueTileCompatible(spDefn));
	
	// uncomment to verify that out-of-bounds accesses cause assert failure
	//vt.getDataChannelType(-1);
	//vt.getDataChannelType(10);

	// define some indices
	const PYXIcosIndex aValidCellIndex("A-01010");
	const PYXIcosIndex anotherValidCellIndex("A-01000");
	const PYXIcosIndex anIndexBiggerThanOneCell("A-010");
	const PYXIcosIndex anIndexSmallerThanOneCell("A-0101000");
	const PYXIcosIndex anIndexOutsideThisTile("B-001");

	// create some values
	PYXValue myInt16((int16_t)100);
	PYXValue myFloat((float)1000.0);
	float rgb[3]; rgb[0] = 100.; rgb[1] = 200.; rgb[2] = 300.;
	PYXValue myRGB(rgb,3);
	std::string myStr("yahöo");
	PYXValue myString(myStr);

	// uncomment to verify that incorrect type in setValue() causes assert failure
	//vt.setValue(aValidCellIndex, 0, myFloat);
	// uncomment to verify that incorrect index resolution causes assert failure
	//vt.setValue(anIndexBiggerThanOneCell, 1, myFloat);
	//vt.setValue(anIndexSmallerThanOneCell, 1, myFloat);
	// uncomment to verify that out-of-bounds access causes assert failure
	//vt.setValue(anIndexOutsideThisTile, 1, myFloat);

	// populate just one cell's data
	vt.setValue(aValidCellIndex, 0, myInt16);
	vt.setValue(aValidCellIndex, 1, myFloat);
	vt.setValue(aValidCellIndex, 2, myRGB);
	vt.setValue(aValidCellIndex, 3, myString);

	// retrieve values and check that they are correct (true value equivalence)
	PYXValue myOtherInt16((int16_t)100);
	PYXValue myOtherFloat((float)1000.0);
	PYXValue myOtherRGB(rgb, 3);
	PYXValue myOtherString(myStr);
	TEST_ASSERT(vt.getValue(aValidCellIndex, 0) == myOtherInt16);
	TEST_ASSERT(vt.getValue(aValidCellIndex, 1) == myOtherFloat);
	TEST_ASSERT(vt.getValue(aValidCellIndex, 2) == myOtherRGB);
	TEST_ASSERT(vt.getValue(aValidCellIndex, 3) == myOtherString);
	TEST_ASSERT(vt.getValue(aValidCellIndex, 3).getString() == myStr);
	TEST_ASSERT(vt.getValue(anotherValidCellIndex, 0).isNull());
	TEST_ASSERT(vt.getValue(anotherValidCellIndex, 1).isNull());
	TEST_ASSERT(vt.getValue(anotherValidCellIndex, 2).isNull());
	TEST_ASSERT(vt.getValue(anotherValidCellIndex, 3).isNull());

	// let's do that again exhaustively
	for (PYXPointer<PYXIterator> spIt(vt.getIterator()); !spIt->end(); spIt->next())
	{
		PYXIcosIndex cellIndex = spIt->getIndex();
		if (cellIndex == aValidCellIndex)
		{
			TEST_ASSERT(vt.getValue(cellIndex, 0) == myOtherInt16);
			TEST_ASSERT(vt.getValue(cellIndex, 1) == myOtherFloat);
			TEST_ASSERT(vt.getValue(cellIndex, 2) == myOtherRGB);
			TEST_ASSERT(vt.getValue(cellIndex, 3) == myOtherString);
		}
		else
		{
			TEST_ASSERT(vt.getValue(cellIndex, 0).isNull());
			TEST_ASSERT(vt.getValue(cellIndex, 1).isNull());
			TEST_ASSERT(vt.getValue(cellIndex, 2).isNull());
			TEST_ASSERT(vt.getValue(cellIndex, 3).isNull());
		}
	}

	// uncomment to print this value tile to trace output
	// serialize to a memory stream
	std::ostringstream out;
	vt.serialize(out);

	// try to read back
	PYXValueTile vt2;
	std::istringstream in(out.str());
	vt2.deserialize(in);

	// verify equivalence
	TEST_ASSERT(vt2.m_tile == vt.m_tile);
	for (PYXPointer<PYXIterator> spIt(vt.getIterator()); !spIt->end(); spIt->next())
	{
		PYXIcosIndex cellIndex = spIt->getIndex();
		if (cellIndex == aValidCellIndex)
		{
			TEST_ASSERT(vt2.getValue(cellIndex, 0) == myOtherInt16);
			TEST_ASSERT(vt2.getValue(cellIndex, 1) == myOtherFloat);
			TEST_ASSERT(vt2.getValue(cellIndex, 2) == myOtherRGB);
			TEST_ASSERT(vt2.getValue(cellIndex, 3) == myOtherString);

			std::string vt2String = vt2.getValue(cellIndex, 3).getString();
			std::string vtOtherString = myOtherString.getString();
			TRACE_INFO(vtOtherString);


		}
		else
		{
			TEST_ASSERT(vt2.getValue(cellIndex, 0).isNull());
			TEST_ASSERT(vt2.getValue(cellIndex, 1).isNull());
			TEST_ASSERT(vt2.getValue(cellIndex, 2).isNull());
			TEST_ASSERT(vt2.getValue(cellIndex, 3).isNull());
		}
	}

	// TODO: unit test the copy constructor and the single channel copy consturctor.

	// Test the isComplete functionality.
	{
		// A new tile defaults to not complete.
		static boost::filesystem::path newFile = AppServices::makeTempFile("newTile.test");
		std::ofstream out1(FileUtils::pathToString(newFile).c_str(), std::ios_base::binary);
		vt.serialize(out1);
		out1.close();
		TEST_ASSERT(!PYXValueTile::isTileFileComplete(FileUtils::pathToString(newFile)));

		// set it to complete and try again
		vt.setIsComplete(true);
		static boost::filesystem::path completeFile = AppServices::makeTempFile("completeTile.test");
		std::ofstream out2(FileUtils::pathToString(completeFile).c_str(), std::ios_base::binary);
		vt.serialize(out2);
		out2.close();
		TEST_ASSERT(PYXValueTile::isTileFileComplete(FileUtils::pathToString(completeFile)));

		// set it back to incomplete and see if that works.
		vt.setIsComplete(false);
		static boost::filesystem::path incompleteFile = AppServices::makeTempFile("incompleteTile.test");
		std::ofstream out3(FileUtils::pathToString(incompleteFile).c_str(), std::ios_base::binary);
		vt.serialize(out3);
		out3.close();
		TEST_ASSERT(!PYXValueTile::isTileFileComplete(FileUtils::pathToString(incompleteFile)));
	}

	// Test zoom in functionality...
	{
		int cellCount = myTile.getCellCount();
		for (int index = 0; index < cellCount; index++)
		{
			vt.setValue(index, 0, myInt16);
		}

		PYXPointer<PYXValueTile> zoomTile = vt.zoomIn(0);
		int zoomCellCount = zoomTile->getNumberOfCells();
		TEST_ASSERT(zoomCellCount > cellCount);
		TEST_ASSERT(vt.getTile().getRootIndex() == zoomTile->getTile().getRootIndex());
		TEST_ASSERT((vt.getTile().getDepth()+1) == zoomTile->getTile().getDepth());

		for (int index = 0; index < zoomCellCount; index++)
		{
			TEST_ASSERT(zoomTile->getValue(index, 0) == myInt16);
		}
	}
}

/*!
Default constructor creates an empty tile: only used only before a call to
deserialize().
*/
PYXValueTile::PYXValueTile() :
	m_nTableRows(0)
{
	m_cacheStatus.initialize();
	m_bDirty = true;
	m_bComplete = false;
}

/*!
Create the value tile initializaed to a particular table definition.

\param index	The tile root.
\param nRes		The tile resolution.
\param spDefn	The table definition to model the tile after.
*/
PYXValueTile::PYXValueTile(	const PYXIcosIndex& index,
							int nRes,
							PYXPointer<const PYXTableDefinition> spDefn) :
	m_tile(PYXTile(index, nRes))
{
	assert(spDefn);

	// vectors to hold definition entries.
	std::vector<PYXValue::eType> vecTypes;
	std::vector<int> vecCounts;

	// cycle through each field in the definition
	int nCount = spDefn->getFieldCount();
	PYXFieldDefinition field;
	for (int nIndex = 0; nIndex < nCount; ++nIndex)
	{
		field = spDefn->getFieldDefinition(nIndex);
		vecTypes.push_back(field.getType());
		vecCounts.push_back(field.getCount());
	}

	m_nTableRows = m_tile.getCellCount();

	m_spValueTable.reset(new PYXValueTable(m_nTableRows, vecTypes, vecCounts));
	m_bDirty = true;
	m_bComplete = false;
}

/*!
Create the value tile initializaed to a particular table definition.

\param tile		The geometry for the value tile.
\param spDefn	The table definition to model the tile after.
*/
PYXValueTile::PYXValueTile(	const PYXTile& tile,
							PYXPointer<const PYXTableDefinition> spDefn) :
	m_tile(tile)
{
	assert(spDefn);

	// vectors to hold definition entries.
	std::vector<PYXValue::eType> vecTypes;
	std::vector<int> vecCounts;

	// cycle through each field in the definition
	int nCount = spDefn->getFieldCount();
	PYXFieldDefinition field;
	for (int nIndex = 0; nIndex < nCount; ++nIndex)
	{
		field = spDefn->getFieldDefinition(nIndex);
		vecTypes.push_back(field.getType());
		vecCounts.push_back(field.getCount());
	}

	m_nTableRows = PYXIcosMath::getCellCount(	m_tile.getRootIndex(),
												m_tile.getCellResolution()	);
	m_bDirty = true;
	m_spValueTable.reset(new PYXValueTable(m_nTableRows, vecTypes, vecCounts));
	m_bComplete = false;
}

/*!
Simple constructor: use when all data channels have non-array type.

\param tile		The tile geometry for this value tile.
\param vecTypes	Vector of base types for the data channels.
*/
PYXValueTile::PYXValueTile(	const PYXTile& tile,
							const std::vector<PYXValue::eType>& vecTypes	) :
	m_tile(tile)
{
	m_nTableRows = PYXIcosMath::getCellCount(	m_tile.getRootIndex(),
												m_tile.getCellResolution()	);

	m_bDirty = true;
	m_spValueTable.reset(new PYXValueTable(m_nTableRows, vecTypes));
	m_bComplete = false;
}

/*!
General constructor: use when one or more data channels have array type.

\param tile			The tile geometry for this value tile.
\param vecTypes		Vector of base types for the data channels.
\param vecCounts	Vector of array counts for the data channels.
*/
PYXValueTile::PYXValueTile(	const PYXTile& tile,
							const std::vector<PYXValue::eType>& vecTypes,
							const std::vector<int>& vecCounts	) :
	m_tile(tile)
{
	m_nTableRows = PYXIcosMath::getCellCount(	m_tile.getRootIndex(),
												m_tile.getCellResolution()	);

	m_bDirty = true;
	m_spValueTable.reset(new PYXValueTable(m_nTableRows, vecTypes, vecCounts));
	m_bComplete = false;
}

/*!
Copy constructor.
*/
PYXValueTile::PYXValueTile(const PYXValueTile& orig)
{
	m_tile = orig.m_tile;
	m_nTableRows = orig.m_nTableRows;

	m_spValueTable.reset(new PYXValueTable(*(orig.m_spValueTable.get())));
	m_cacheStatus.initialize();
	m_bDirty = true;
	m_bComplete = orig.m_bComplete;
}

/*!
Copy constructor that only copies one of the contained field tiles.
*/
PYXValueTile::PYXValueTile(const PYXValueTile& orig, int nFieldIndex)
{
	m_tile = orig.m_tile;
	m_nTableRows = orig.m_nTableRows;

	m_spValueTable.reset(new PYXValueTable(*(orig.m_spValueTable.get()), nFieldIndex));
	m_cacheStatus.initialize();
	m_bDirty = true;
	m_bComplete = orig.m_bComplete;
}

/*!
Destructor.
*/
PYXValueTile::~PYXValueTile()
{
}

//! Return a copy of the field tile that is at the given index.
PYXPointer<PYXValueTile> PYXValueTile::cloneFieldTile(int nFieldIndex) const
{
	PYXPointer<PYXValueTile> spFieldTile = PYXNEW(PYXValueTile, *this, nFieldIndex);
	return spFieldTile;
}

PYXPointer<PYXValueTile> PYXValueTile::clone() const
{
	return PYXValueTile::create(*this);
}

//! Assignment operator.
void PYXValueTile::operator =(const PYXValueTile&)
{
	// TODO: implement this and unhide declaration.
}


//! Verify that the coverage definition matches a value tile.
bool PYXValueTile::isValueTileCompatible(PYXPointer<const PYXTableDefinition> spDefn) const
{
	if (m_spValueTable->getNumberOfColumns() == spDefn->getFieldCount())
	{
		for (int n = 0; n < spDefn->getFieldCount(); ++n)
		{
			const PYXFieldDefinition& fieldDef = spDefn->getFieldDefinition(n);
			if (m_spValueTable->getColumnType(n) != fieldDef.getType() ||
				m_spValueTable->getCount(n) != fieldDef.getCount()	)
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

/*!
Get value for given cell index and channel, and optionally, an indication of
whether or not the field has ever been set.

\param cellIndex		The index to retreive a value for.
\param nChannelIndex	The offset of the index.
\param pbInitialized	Optionally, points to a bool to receive field's status.

\return The value that is held at the location and offset.
*/
PYXValue PYXValueTile::getValue(	const PYXIcosIndex& cellIndex,
									const int nChannelIndex,
									bool* pbInitialized	) const
{
	assert(cellIndex.getResolution() == m_tile.getCellResolution() && "Invalid cell resolution.");
	int nOffset = PYXIcosMath::calcCellPosition(m_tile.getRootIndex(), cellIndex);
	
	return getValue(nOffset, nChannelIndex, pbInitialized);
}

/*!
Get value for given cell index and channel, and an indication of
whether or not the field has ever been set.

\param cellIndex		The index to retreive a value for.
\param nChannelIndex	The offset of the index.
\param pValue			Points to a PYXValue to receive the data.

\return The status of the retrieved value, true if it has been set, false if it has never been initialized.
*/
bool PYXValueTile::getValue(	const PYXIcosIndex& cellIndex,
								const int nChannelIndex,
								PYXValue* pValue) const
{
	assert(cellIndex.getResolution() == m_tile.getCellResolution() && "Invalid cell resolution.");
	int nOffset = PYXIcosMath::calcCellPosition(m_tile.getRootIndex(), cellIndex);
	
	return getValue(nOffset, nChannelIndex, pValue);
}

/*!
Get value for given integer cell offset and channel, and optionally, an indication of
whether or not the field has ever been set.

\param nOffset			Integer offset to the desired cell.
\param nChannelIndex	The field index.
\param pbInitialized	Optionally, points to a bool to receive field's status.

\return The value that is held at the location and offset.
*/
PYXValue PYXValueTile::getValue(	const int nOffset,
									const int nChannelIndex,
									bool* pbInitialized	) const
{
	// ensure we have a valid offset
	if (	((nOffset >= 0) && (nOffset < m_nTableRows)) &&
			(nChannelIndex >= 0 && nChannelIndex < getNumberOfDataChannels())	)
	{
		return m_spValueTable->getValue(nOffset, nChannelIndex, pbInitialized);
	}

	assert(nOffset >= 0 && nOffset < m_nTableRows && "Invalid index offset.");
	assert(nChannelIndex >= 0 && nChannelIndex < getNumberOfDataChannels() && "Invalid data channel.");

	return PYXValue();
}

/*!
Get value for given integer cell offset and channel, an indication of
whether or not the field has ever been set.

\param nOffset			Integer offset to the desired cell.
\param nChannelIndex	The field index.
\param pValue			Points to a PYXValue to receive the data.

\return The status of the retrieved value, true if it has been set, false if it has never been initialized.
*/
bool PYXValueTile::getValue(	const int nOffset,
								const int nChannelIndex,
								PYXValue* pValue	) const
{
	// ensure we have a valid offset
	if (	((nOffset >= 0) && (nOffset < m_nTableRows)) &&
			(nChannelIndex >= 0 && nChannelIndex < getNumberOfDataChannels())	)
	{
		return m_spValueTable->getValue(nOffset, nChannelIndex, pValue);
	}

	assert(nOffset >= 0 && nOffset < m_nTableRows && "Invalid index offset.");
	assert(nChannelIndex >= 0 && nChannelIndex < getNumberOfDataChannels() && "Invalid data channel.");
	return false;
}

/*!
Set value for given cell index and channel.

\param cellIndex		The index to set value for.
\param nChannelIndex	The field index.
\param value			New value to set.
*/
void PYXValueTile::setValue(	const PYXIcosIndex& cellIndex,
								const int nChannelIndex,
								const PYXValue& value	)
{
	assert(cellIndex.getResolution() == m_tile.getCellResolution() && "Invalid cell resolution.");

	int nOffset = PYXIcosMath::calcCellPosition(m_tile.getRootIndex(), cellIndex);
	setValue(nOffset, nChannelIndex, value);
}

/*!
Set value for given integer cell offset and channel.

\param nOffset			Integer offset to the desired cell.
\param nChannelIndex	The field index.
\param value			New value to set.
*/
void PYXValueTile::setValue(	const int nOffset,
								const int nChannelIndex,
								const PYXValue& value	)
{
	// ensure we have a valid cell offset
	if (	((nOffset >= 0) && (nOffset < m_nTableRows)) &&
			(nChannelIndex >= 0 && nChannelIndex < getNumberOfDataChannels())	)
	{
#ifdef _DEBUG
		// help when debugging, to know why assert below may have failed
		PYXValue::eType valType = value.getArrayType();
		PYXValue::eType dchType = getDataChannelType(nChannelIndex);
		int valSize = value.getArraySize();
		int dchSize = getDataChannelCount(nChannelIndex);
		bool bValueIsNull = value.isNull();
		bool bSimpleTypesMatch = !value.isArray() && (value.getType() == getDataChannelType(nChannelIndex));
		bool bArrayTypesMatch = value.isArray() && (value.getArrayType() == getDataChannelType(nChannelIndex));
		bool bArraySizesMatch = value.getArraySize() == getDataChannelCount(nChannelIndex);
#endif
		assert(	(bValueIsNull || bSimpleTypesMatch || (bArrayTypesMatch && bArraySizesMatch)) &&
				"Data problem while setting a value."	);

		m_spValueTable->setValue(nOffset, nChannelIndex, value);
		m_bDirty = true;
	}
	else
	{
		assert(nOffset >= 0 && nOffset < m_nTableRows && "Invalid index offset.");
		assert(nChannelIndex >= 0 && nChannelIndex < getNumberOfDataChannels() && "Invalid data channel.");
		TRACE_INFO("Invalid data channel!");
	}
}

/*!
Current I/O format version: increment every time format changes.
*/
static int knIOFormatVersion = 3;

/*!
Serialize to binary stream.
NOTE: The data format will be architecture-dependent!

\param out	output stream
*/
void PYXValueTile::serialize(std::ostream& out)
{
	// version number as int
	out.write((char*)&knIOFormatVersion,sizeof(int));

	// write out the completness
	out.write ((char*)&m_bComplete, sizeof(bool));

	// create a memory stream for the tile so we can compress it before it goes on disk.
	std::ostringstream memOut;
	
	// serialize our tile, followed one blank char to ensure it reads back correctly
	memOut << m_tile << " ";

	// serialize our value table
	m_spValueTable->serialize(memOut);
	unsigned long nUnCompressedLength = static_cast<unsigned long>(memOut.str().length());
	unsigned long nCompressedBufferLength = compressBound(nUnCompressedLength);
	boost::scoped_array<char> pCompressedDataBuffer(new char[nCompressedBufferLength]);
	int returnCode = compress((Bytef *)pCompressedDataBuffer.get(), &nCompressedBufferLength,
                                (const Bytef *)memOut.str().c_str(), nUnCompressedLength);

	// Check to see if the compress was successful.
	if (returnCode != Z_OK)
	{
		PYXTHROW(PYXDataException, "Compression failed while saving a value tile.");
	}

	out.write((char*)&nUnCompressedLength,sizeof(unsigned long));
	out.write((char*)&nCompressedBufferLength,sizeof(unsigned long));
	out.write(pCompressedDataBuffer.get(), nCompressedBufferLength);

	// tile isn't dirty anymore
	m_bDirty = false;
}

/*!
De-serialize from binary stream.
NOTE: The data format will be architecture-dependent!

\param in	input stream
*/
void PYXValueTile::deserialize(std::istream& in)
{
	// get version number
	int nFormatVersion;
	in.read((char*)&nFormatVersion,sizeof(int));

	m_bComplete = false;
	if (nFormatVersion >= 2)
	{
		in.read((char*)&m_bComplete,sizeof(bool));
	}

	// reconstruct our value table
	m_spValueTable.reset(new PYXValueTable());

	switch (nFormatVersion)
	{
	case 1:
	case 2:
		// recover our tile
		in >> m_tile;
		in.get();	// eat the trailing blank
		m_nTableRows = PYXIcosMath::getCellCount(	m_tile.getRootIndex(),
													m_tile.getCellResolution()	);
		m_spValueTable->deserialize(in);
		break;
	case 3:
		{
			unsigned long nUnCompressedLength;
			unsigned long nCompressedBufferLength;
			in.read((char*)&nUnCompressedLength,sizeof(unsigned long));
			in.read((char*)&nCompressedBufferLength,sizeof(unsigned long));
			boost::scoped_array<char> pCompressedDataBuffer(new char[nCompressedBufferLength]);
			in.read(pCompressedDataBuffer.get(), nCompressedBufferLength);
			boost::scoped_array<char> pUnCompressedDataBuffer(new char[nUnCompressedLength]);
			int returnCode = uncompress((Bytef *)pUnCompressedDataBuffer.get(), &nUnCompressedLength,
                                   (const Bytef *)pCompressedDataBuffer.get(), nCompressedBufferLength);

			// Check to see if the compress was successful.
			if (returnCode != Z_OK)
			{
				PYXTHROW(PYXDataException, "Uncompression failed while loading a value tile.");
			}
			std::string stringIn(pUnCompressedDataBuffer.get(), nUnCompressedLength);
			std::istringstream memIn(stringIn);
			memIn >> m_tile;
			memIn.get();	// eat the trailing blank
			m_nTableRows = PYXIcosMath::getCellCount(	m_tile.getRootIndex(),
														m_tile.getCellResolution()	);
			m_spValueTable->deserialize(memIn);
		}
		break;
	default:
		char errorMessage[80];
		strerror_s(errorMessage, errno);
		PYXTHROW(PYXValueTileException,"Unknown format version -  Error: " << errorMessage);
	}


	// tile is not dirty yet
	m_bDirty = false;
}


/* static */
/*!
Static helper method to determine if a serialize tile file is complete 
on disk by reading the header information.

\param tileFilename		The name of the file to parse for the completness flag.
*/
bool PYXValueTile::isTileFileComplete (const std::string& tileFilename)
{
	bool bResult = false;
	std::ifstream in(tileFilename.c_str(), std::ios_base::binary );

	// get version number
	int nFormatVersion;
	in.read((char*)&nFormatVersion,sizeof(int));

	if (nFormatVersion >= 2)
	{
		in.read((char*)&bResult,sizeof(bool));
	}

	return bResult;
}

PYXPointer<PYXValueTile> PYXValueTile::createFromFile(const std::string& tileFilename)
{
	std::ifstream in(tileFilename.c_str(), std::ios_base::binary);
	PYXPointer<PYXValueTile> spTile = PYXValueTile::create(in);
	return spTile;
}

PYXPointer<PYXValueTile> PYXValueTile::createFromBase64String(const std::string& base64)
{
	auto string = XMLUtils::fromBase64(base64);
	std::istringstream in(string);
	PYXPointer<PYXValueTile> spTile = PYXValueTile::create(in);
	return spTile;
}

std::string PYXValueTile::toBase64String()
{
	std::ostringstream out;
	serialize(out);
	return XMLUtils::toBase64(out.str());
}

/*!
Get total memory allocated.
*/
inline int PYXValueTile::getHeapBytes() const
{
	return m_spValueTable->getHeapBytes();
}

/*!
Get number of data channels (columns).
*/
inline int PYXValueTile::getNumberOfDataChannels() const
{
	return m_spValueTable->getNumberOfColumns();
}

/*!
Get number of cells (rows),
*/
inline int PYXValueTile::getNumberOfCells() const
{
	return m_spValueTable->getNumberOfRows();
}

/*!
Get the type for a specific data channel.
*/
inline const PYXValue::eType PYXValueTile::getDataChannelType(
	const int nChannelIndex	) const
{
	return m_spValueTable->getColumnType(nChannelIndex);
}

/*!
Get the array count for a data channel.
*/
inline const int PYXValueTile::getDataChannelCount(
	const int nChannelIndex	) const
{
	return m_spValueTable->getCount(nChannelIndex);
}

/*!
Stream output operator: serialize to text stream.
*/
std::ostream& operator <<(std::ostream& out, PYXValueTile& pvt)
{
	out << "PYXValueTile: ";
	if (pvt.m_bDirty)
	{
		out << "[DIRTY] ";
	}
	out << "root index " << pvt.m_tile.getRootIndex().toString();
	out << ", cell resolution " << pvt.m_tile.getCellResolution() << std::endl;
	out << *(pvt.m_spValueTable);
	return out;
}

/*!
Stream input operator: de-serialize from text stream.
*/
std::istream& operator >>(std::istream& in, PYXValueTile& pvt)
{
	assert(false && "not yet implemented");
	return in;
}


// ****  Utility functions to support zoom in functions. **** //

// Load a Look Up Table from disk (LUT)
// Returns true if the file was successfully loaded.
bool loadLUT(boost::filesystem::path srcFilename, int* pOffsets, int nNumberOffsets)
{
	bool bLUTLoaded = false;

	if (FileUtils::exists(srcFilename))
	{
		// load the table
		std::ifstream file(FileUtils::pathToString(srcFilename).c_str(), std::ios::binary);

		const char* const magic = "zio"; // note this is 4 bytes not 3!
		char buf[8] = { 0 }; // init buffer to zero

		file.read(buf, 8);
		if (memcmp(magic, buf, 4) == 0)
		{
			int& nVersion = *reinterpret_cast<int*>(buf + 4);
			if (nVersion == 1)
			{
				unsigned long nUnCompressedLength;
				unsigned long nCompressedBufferLength;
				file.read((char*)&nUnCompressedLength,sizeof(unsigned long));
				file.read((char*)&nCompressedBufferLength,sizeof(unsigned long));
				boost::scoped_array<char> pCompressedDataBuffer(new char[nCompressedBufferLength]);
				file.read(pCompressedDataBuffer.get(), nCompressedBufferLength);
				file.close();
				boost::scoped_array<char> pUnCompressedDataBuffer(new char[nUnCompressedLength]);
				int returnCode = uncompress((Bytef *)pUnCompressedDataBuffer.get(), &nUnCompressedLength,
									   (const Bytef *)pCompressedDataBuffer.get(), nCompressedBufferLength);

				// Check to see if the compress was successful.
				if (returnCode != Z_OK)
				{
					PYXTHROW(PYXException, "Uncompression failed while loading a zoom in/out lookup table.");
				}

				// stream it in from the uncompressed buffer.
				std::string compressedLUT(pUnCompressedDataBuffer.get(), nUnCompressedLength);
				std::istringstream memIn(compressedLUT);
				memIn.read(reinterpret_cast<char*>(pOffsets), nNumberOffsets * sizeof(int));
				bLUTLoaded = memIn.good();
			}
		}
	}
	return bLUTLoaded;
}

// Save a Look Up Table (LUT) to disk.
void saveLUT(boost::filesystem::path dstFilename, int* pOffsets, int nNumberOffsets)
{
	// create a memory stream for the tile so we can compress it before it goes on disk.
	std::ostringstream memOut;
	
	memOut.write(reinterpret_cast<char*>(pOffsets), nNumberOffsets * sizeof(unsigned int));

	unsigned long nUnCompressedLength = static_cast<unsigned long>(memOut.str().length());
	unsigned long nCompressedBufferLength = compressBound(nUnCompressedLength);
	boost::scoped_array<char> pCompressedDataBuffer(new char[nCompressedBufferLength]);
	int returnCode = compress((Bytef *)pCompressedDataBuffer.get(), &nCompressedBufferLength,
                                (const Bytef *)memOut.str().c_str(), nUnCompressedLength);

	// Check to see if the compress was successful.
	if (returnCode != Z_OK)
	{
		PYXTHROW(PYXException, "Compression failed while saving a zoom in/out lookup table.");
	}

	// create the output file
	std::ofstream file(FileUtils::pathToString(dstFilename).c_str(), std::ios::binary);

	// Write magic number.
	file.write("zio", 4);

	// Write version.
	int nVersion = 1;
	file.write(reinterpret_cast<const char*>(&nVersion), 4);

	// write the compresed data
	file.write((char*)&nUnCompressedLength,sizeof(unsigned long));
	file.write((char*)&nCompressedBufferLength,sizeof(unsigned long));
	file.write(pCompressedDataBuffer.get(), nCompressedBufferLength);

	file.close();
}

/* Utility function for generating the name of the file that holds the look-up table.

This function must ensure when it generates a file name for the look-up table such that
all look-up tables that have the same file name will have the same contents.

For all tiles the tile depth and class are used to identify the file name.

For hexagonal tiles, the general case is enough to identify them.

For pentagonal tiles, the naming depends the north or south hemisphere which determines
where the "gap" is.
*/
boost::filesystem::path PYXValueTile::getZoomInFilename()
{
	boost::filesystem::path fname = AppServices::getCacheDir("zio");

	// put in the common naming traits.
	std::string name = StringUtils::toString(m_tile.getDepth()) + "_" + 
		StringUtils::toString(m_tile.getRootIndex().getClass());
	if (m_tile.getRootIndex().isHexagon())
	{
		name += "H";
	}
	else
	{
		name += "P";
		if (m_tile.getRootIndex().isNorthern())
		{
			name += "N";
		}
		else
		{
			name += "S";
		}
	}

	name += ".zin";

	fname /= name;
	return fname;
}

// Find the size of the array we need for doing zoom in.
int CalculateNumberOfOffsets(PYXTile targetTile)
{
	return (targetTile.getCellCount() * 3);
}

// Generate the correct lookup table for performing a zoom in operation on this tile.
void PYXValueTile::generateZoomInLUT(PYXTile targetTile, int* pOffsets)
{
	PYXPointer<PYXIterator> targetIter = targetTile.getIterator();
	int targetIndex = 0;
	int offsetsIndex = 0;

	PYXIcosIndex workingIndex;
	PYXIcosIndex parentIndex;
	PYXIcosIndex otherParentIndex;
	PYXMath::eHexClass parentType = PYXMath::getHexClass(this->getTile().getCellResolution());
	PYXMath::eHexDirection nCellDirection;
	PYXMath::eHexDirection nCellDirection2;

	for (; !targetIter->end(); targetIter->next())
	{
		workingIndex = targetIter->getIndex();
		parentIndex = PYXIcosMath::getParent(workingIndex);
		nCellDirection = PYXMath::knDirectionZero;
		PYXIcosMath::directionFromParent(workingIndex, &nCellDirection);

		if (nCellDirection != PYXMath::knDirectionZero)
		{
			// Normal case (3 parents)
			pOffsets[offsetsIndex] = PYXIcosMath::calcCellPosition(getTile().getRootIndex(), parentIndex);
			++offsetsIndex;

			otherParentIndex = PYXIcosMath::move(parentIndex, nCellDirection);
			if (getTile().hasIndex(otherParentIndex))
			{
				pOffsets[offsetsIndex] = PYXIcosMath::calcCellPosition(getTile().getRootIndex(), otherParentIndex);
			}
			else
			{
				pOffsets[offsetsIndex] = -1;
			}
			++offsetsIndex;

			nCellDirection2 =
				PYXMath::rotateDirection(nCellDirection, (parentType == PYXMath::knClassI) ? 1 : -1);
			if (parentIndex.isPentagon())
			{
				PYXMath::eHexDirection nGapDirection;
				// make sure that the direction is not in the gap.
				PYXIcosMath::getCellGap(parentIndex, &nGapDirection);
				if (nCellDirection2 == nGapDirection)
				{
					nCellDirection2 =
						PYXMath::rotateDirection(nCellDirection2, (parentType == PYXMath::knClassI) ? 1 : -1);
				}
			}

			otherParentIndex = PYXIcosMath::move(parentIndex, nCellDirection);
			if (getTile().hasIndex(otherParentIndex))
			{
				pOffsets[offsetsIndex] = PYXIcosMath::calcCellPosition(getTile().getRootIndex(), otherParentIndex);
			}
			else
			{
				pOffsets[offsetsIndex] = -1;
			}
			++offsetsIndex;
		}
		else
		{
			pOffsets[offsetsIndex] = PYXIcosMath::calcCellPosition(getTile().getRootIndex(), parentIndex);
			++offsetsIndex;
			pOffsets[offsetsIndex] = -1;
			++offsetsIndex;
			pOffsets[offsetsIndex] = -1;
			++offsetsIndex;
		}
	}
}

PYXPointer<PYXValueTile> PYXValueTile::zoomIn(int nFieldIndex)
{
	// Make a new tile that is one res high than this tile.
	std::vector<PYXValue::eType> vecTypes;
	std::vector<int> vecCounts;
	vecTypes.push_back(m_spValueTable->getColumnType(nFieldIndex));
	int nFieldComponents = m_spValueTable->getCount(nFieldIndex);
	vecCounts.push_back(nFieldComponents);
	PYXTile tile(getTile().getRootIndex(), getTile().getCellResolution() + 1);
	PYXPointer<PYXValueTile> ziTile = create(tile, vecTypes, vecCounts);

	// get the transformation information, either from cache, or by generating it.
	int nNumberOffsets = CalculateNumberOfOffsets(tile);
	boost::scoped_array<int> pOffsets(new int[nNumberOffsets]);

	if (!loadLUT(getZoomInFilename(), pOffsets.get(), nNumberOffsets))
	{
		generateZoomInLUT(tile, pOffsets.get());
		saveLUT(getZoomInFilename(), pOffsets.get(), nNumberOffsets);
	}

	// perform the zoom in

	// Create a temporary PYXValue that can hold doubles with the same number of elements
	// as our data source is returning.
	PYXValue sumValue = PYXValue::create(PYXValue::knDouble, 0, nFieldComponents, 0);
	PYXValue vertexValue = 
		PYXValue::create(m_spValueTable->getColumnType(nFieldIndex), 0, nFieldComponents, 0);

	int index = 0;
	int maxCount = tile.getCellCount();
	for (int count = 0; count < maxCount; ++count)
	{
		double fTotalWeight = 0.0;
		if (getValue(pOffsets.get()[index], nFieldIndex, &vertexValue))
		{
			PYXValueMath::assignInto(&sumValue, vertexValue);
			fTotalWeight = 1.0;
		}
		else
		{
			PYXValueMath::zero(&sumValue);
		}

		int lookUp = pOffsets.get()[index+1];
		if (lookUp != -1 && getValue(lookUp, nFieldIndex, &vertexValue))
		{
			PYXValueMath::addInto(&sumValue, vertexValue);
			fTotalWeight += 1.0;
		}

		lookUp = pOffsets.get()[index+2];
		if (lookUp != -1 && getValue(lookUp, nFieldIndex, &vertexValue))
		{
			PYXValueMath::addInto(&sumValue, vertexValue);
			fTotalWeight += 1.0;
		}

		if (fTotalWeight != 0.0)
		{
			// divide by total weight
			PYXValueMath::divideBy(&sumValue, fTotalWeight);
		}

		// put the result back into the correct type.
		PYXValueMath::assignInto(&vertexValue, sumValue);

		// store the result in the output tile.
		ziTile->setValue(count, 0, vertexValue);

		index += 3;
	}

	// return the new tile
	return ziTile;
}

/*!
Calculate some statistics on the tile.

\return The statistics.
*/
PYXValueTileStatistics PYXValueTile::calcStatistics() const
{
	PYXValueTileStatistics stats;

    // track some statistics
    stats.nValues = 0;
    stats.minValue = PYXValue();
    stats.maxValue = PYXValue();
	stats.avgValue = PYXValue();

    stats.nCells = getNumberOfCells();
	double fTotal = 0.0;
    for (int i = 0; i < stats.nCells; ++i)
    {
        auto value = getValue(i, 0);

        if (!value.isNull())
        {
			stats.nValues++;

            if (    stats.minValue.isNull() ||
                    (stats.minValue.isNumeric() && (value.getDouble() < stats.minValue.getDouble())) ||
                    (!stats.minValue.isNumeric() && (strcmp(value.getString().c_str(), stats.minValue.getString().c_str()) < 0))  )
            {
                stats.minValue = value;
            }

            if (   stats.maxValue.isNull() ||
                   (stats.maxValue.isNumeric() && (value.getDouble() > stats.maxValue.getDouble())) ||
                   (!stats.maxValue.isNumeric() && (strcmp(value.getString().c_str(), stats.maxValue.getString().c_str()) > 0))   )
            {
                stats.maxValue = value;
            }

			if (value.isNumeric())
			{
				fTotal += value.getDouble();
			}
        }
    }

	if (stats.nValues > 0)
	{
		stats.avgValue = PYXValue(fTotal / stats.nValues);
	}

	return stats;
}
