/******************************************************************************
test_coverage.cpp

begin		: 2004-01-08
copyright	: (C) 2003 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "test_coverage.h"

// local includes
#include "pyxis/data_source/exceptions.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/value.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <algorithm>
#include <cassert>

//! The file name of the class
const std::string TestCoverage::kstrScope = "TestCoverage";

//! The file extenstion for a test coverage data source
const std::string TestCoverage::kstrFileExtension = "tst";

//! The connection string tag
const std::string TestCoverage::kstrIndexTag = "Index";

//! The resolution tag
const std::string TestCoverage::kstrResolutionTag = "Resolution";

//! Return a null when the return value matches this on
const int knNullValue = 0;

//! A static value to ensure each test coverage has a unique name
int TestCoverage::m_nCoverageID = 0;

//! Tester class
TesterUnit<TestCoverage> gTester;

//! Test method
void TestCoverage::test()
{
	const int knTestResolution = 6;

	// get the full directory path
	const std::string strTestDir = FileUtils::getFullDir("pyx_test", true);

	// clean up test files from a previous run
	FileUtils::recursiveDelete(strTestDir);

	// create a test coverage
	PYXIcosIndex rootIndex;
	TestCoverage testCoverage;
	testCoverage.open(rootIndex, knTestResolution);

	for (PYXPointer<PYXIterator> spIt(testCoverage.getIterator()); !spIt->end(); spIt->next())
	{
		TEST_ASSERT(
			spIt->getFieldValue().getInt() ==
			TestCoverage::indexToValue(spIt->getIndex())	);
	}
}

/*!
Constructor initializes member variables.
*/
TestCoverage::TestCoverage()
{
}

/*!
Destructor cleans up memory.
*/
TestCoverage::~TestCoverage()
{
}

/*!
Open the data source from a file.

\param	strFileName		The file name.

\return	true if the data source was successfully opened, otherwise false.
*/
bool TestCoverage::openForRead(const std::string& strFileName)
{

	// make sure it has the right extension
	std::string::size_type nIndex = strFileName.find(kstrFileExtension);
	std::string::size_type nPos = -1;
	if (nIndex == nPos)
	{
		TRACE_INFO("Invalid file extension.");
		return false;
	}

	TRACE_INFO("Opening Test Coverage '" << strFileName << "'.");

	PYXIcosIndex index;
	int nResolution = -1;

	readFile(strFileName, &index, &nResolution);

	if (nResolution < 0 || nResolution > PYXMath::knMaxAbsResolution)
	{
		PYXTHROW(PYXDataSourceException, "Invalid resolution.");
	}

	return true;
}
		
/*!
Read the TestCoverage info from the file. Extracts the root index and the 
resolution.
	
\param		strFileName			The name of the connection file. 
\param		pIndex	 			The index (out)
\param		pnResolution		The resolution (out)
*/
void TestCoverage::readFile(
	const std::string& strFileName,
	PYXIcosIndex* pIndex,
	int* pnResolution	)
{
	// open the file
	std::ifstream in;
	in.open(strFileName.c_str());
	assert((in.good()) && "Unable to open file.");

	std::string strLine;

	do
	{
		// read the next line
		std::getline(in, strLine);

		// ignore empty lines and comment lines
		if ((strLine.length() > 0) && (*(strLine.begin()) != '#'))
		{
			// extract the key and value
			size_t nOffset = strLine.find('=');
			if (nOffset > 0)
			{
				// add key and value to our map
				std::string strKey = StringUtils::trim(strLine.substr(0, nOffset));

				nOffset++;
				size_t nLength = strLine.length() - nOffset;
				std::string strValue = StringUtils::trim(strLine.substr(nOffset, nLength));

				if (strKey == kstrIndexTag)
				{
					if (pIndex != 0)
					{
						*pIndex = strValue;
					}
				}

				if (strKey == kstrResolutionTag)
				{
					if (pnResolution != 0)
					{
						*pnResolution = atoi(strValue.c_str());
					}
				}
			}
			// else ignore line
		}

	} while (!in.eof());
}

/*!
Open the data source.

\param	rootIndex		The root index.
\param	nResolution		The absolute data resolution.
*/
void TestCoverage::open(const PYXIcosIndex& rootIndex, int nResolution)
{
	// initialize the coverage definition
	getCoverageDefinition()->addFieldDefinition(
		"1",
		PYXFieldDefinition::knContextGreyScale,
		PYXValue::knInt32	);

	// assign an ID number to the end of the name
	setName("Test Coverage " + ::toString(m_nCoverageID));
	++m_nCoverageID;

	setType(PYXDataSource::knDEM);

	if (rootIndex.isNull())
	{
		nResolution = std::max(0, nResolution);
	}
	else
	{
		nResolution = std::max(nResolution, rootIndex.getResolution());
	}

	if (rootIndex.isNull())
	{
		boost::shared_ptr<PYXTileCollection> spTileCollection(new PYXTileCollection());

		// a null index means a global data set
		PYXIcosIterator it(2);
		for (; !it.end(); it.next())
		{
			spTileCollection->addTile(it.getIndex(), nResolution);
		}

		m_spGeometry = spTileCollection;
	}
	else
	{
		m_spGeometry.reset(new PYXTile(rootIndex, nResolution));
	}

	getCoverageDefinition()->addFieldDefinition(
		"2",
		PYXFieldDefinition::knContextNone,
		PYXValue::knInt32,
		1	);
}

/*!
Create a new test coverage data source, open it and associate it with a data
item.  The new data item is returned to the caller.

\param tile		The geometry of the test coverage.
\param nDSType	The type of data source to be emulated.

\return A new PYXDataItem that contains an open test coverage.
*/
PYXDataItem::SPtr TestCoverage::getNewDataItem(	
	const PYXTile& tile,
	PYXDataItem::eDSType nDSType	)
{
	// create and open a new test coverage
	boost::shared_ptr<TestCoverage> spTestCoverage(new TestCoverage());
	spTestCoverage->open(tile.getRootIndex(), tile.getCellResolution());

	// create a new data item
	PYXDataItem::SPtr spDataItem(PYXDataItem::create());
	spDataItem->setDSType(nDSType);
	spDataItem->setDS(spTestCoverage);
	spDataItem->setURI(spTestCoverage->getName());
	spDataItem->dataSourceOpen(PYXSpatialReferenceSystem::SPtr(), -1);

	return spDataItem;
}

/*!
Get the coverage value the specified PYXIS index. Returns the numeric
equivalent of the index. Returns null if the numeric equivalent matches
the null value.

\param	index			The PYXIS index.
\param	nFieldIndex		The field index (ignored).

\return	The value.
*/
PYXValue TestCoverage::getCoverageValue(
	const PYXIcosIndex& index,
	int nFieldIndex	) const
{
	assert(!index.isNull() && "Invalid argument.");
	assert(nFieldIndex == 0 && "Ignored, so should be 0.");

	int nValue = TestCoverage::indexToValue(index);

	if (nValue == knNullValue)
	{
		return PYXValue();
	}

	return PYXValue(nValue);
}

/*!
Convert a PYXIS index to its numeric equivalent for test purposes.

\param	index	The index.

\return	The numeric equivalent.
*/
unsigned int TestCoverage::indexToValue(const PYXIcosIndex& index)
{
	std::string str = index.getSubIndex().toString();

	unsigned int nValue = 0;
	std::string::const_iterator it = str.begin();
	for (; it != str.end(); ++it)
	{
		nValue = nValue * 10 + (*it - '0');
	}

	return nValue;
}

/*!
Creates a temporary connection file.  
	
\param		rootIndex 			The root index.
\param		nResolution 		The resolution.
\param		strExt				The file extension.

\return	 The name of the file created.
*/
std::string TestCoverage::createTestFile(
	const PYXIcosIndex& rootIndex,
	int nResolution,
	const std::string strExt	)
{
	// determine the extension
	std::string strExtension;
	if (strExt.empty())
	{
		strExtension = TestCoverage::kstrFileExtension;
	}
	else
	{
		strExtension = strExt;
	}

	// create a temporary file 
	std::string strFileName = AppServices::makeTempFile(strExtension);

	// open the temp file
	std::ofstream out;
	out.open
			(	strFileName.c_str(), 
				std::ios::out | std::ios::trunc	);
	assert((out.good()) && "Unable to create test file.");
	if (!out.good())
	{
		PYXTHROW(PYXDataSourceException, "Unable to create a test file.");
	}

	// output the description and the property
	out << TestCoverage::kstrIndexTag;
	out << "=" << rootIndex << std::endl;
	out << TestCoverage::kstrResolutionTag;
	out << "=" << ::toString(nResolution) << std::endl;
	
	return strFileName;
}
