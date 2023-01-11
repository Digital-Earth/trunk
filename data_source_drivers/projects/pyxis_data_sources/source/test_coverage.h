#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H
/******************************************************************************
test_coverage.h

begin		: 2004-01-08
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxis/data_source/coverage.h"
//#include "pyx_data_item.h"

// boost includes

// standard includes
#include <string>

// local forward declarations
class PYXGeometry;
class PYXIcosIndex;
class PYXLibDatabase;

/*!
TestDataSet returns an integer value corresponding to the sub-index of the
PYXIS index being requested.
*/
//! Coverage to be used for test purposes.
class TestCoverage : public PYXCoverage
{
public:

	//! Unit test method
	static void test();

	//! Constants
	static const std::string kstrScope;
	static const std::string kstrFileExtension;
	static const std::string kstrIndexTag;
	static const std::string kstrResolutionTag;
	static const std::string kstrMultiValue;
	static const std::string kstrOwner;
	static const std::string kstrSize;
	static const double kpfDoubleArray[];
	static const double kpfDoubleArrayNullValue[];
	static const int knSize;
	static const std::string kstrOwnerValue;

	//! Constructor initializes member variables.
	TestCoverage();

	//! Destructor
	virtual ~TestCoverage();

	//! Open the data source.
	void open(const PYXIcosIndex& rootIndex, int nResolution);

	//! Open the data source for read using a file.
	bool openForRead(const std::string& strFileName);

	//! Return the class name of this observer class.
	virtual std::string getObserverDescription() const {return kstrScope;}

	//! Return the name of the notification class.
	virtual std::string getNotifierDescription() const {return kstrScope;}

	/*!
	Get the geometry of the feature. Note: must be non-null.

	\return	The geometry of the feature.
	*/
	virtual PYXPointer<const PYXGeometry> getGeometry() const
	{
		return m_spGeometry;
	}

	//! Get the coverage value the specified PYXIS index.
	virtual PYXValue getCoverageValue(
		const PYXIcosIndex& index,
		int nFieldIndex = 0	) const;

	//! Convert a PYX icos index to its numeric equivalent.
	static unsigned int indexToValue(const PYXIcosIndex& index);

	//! Return a pointer to a new test coverage encapsulated in a data item
	static PYXDataItem::SPtr getNewDataItem(
		const PYXTile& tile,
		PYXDataItem::eDSType = PYXDataItem::knRaster	);

	//! Creates a test file.  
	static std::string createTestFile
							(	const PYXIcosIndex& rootIndex,
								const int nResolution,
								const std::string strExt = std::string("")	);

protected:

private:

	//! Disable copy constructor
	TestCoverage(const TestCoverage&);

	//! Disable copy assignment
	void operator =(const TestCoverage&);

	//! Read the file.  
	void readFile(
		const std::string& strFileName,
		PYXIcosIndex* pIndex,
		int* pnResolution	);

	//! The geometry of the test coverage
	PYXPointer<PYXGeometry> m_spGeometry;

	//! A unique identifier for this coverage data source (built into the name).
	static int m_nCoverageID;
};

#endif	// TEST_COVERAGE_H
