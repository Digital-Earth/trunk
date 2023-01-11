/******************************************************************************
dilation_filter.cpp

begin		: 2006-05-23
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "dilation_filter.h"

// local includes
//#include "binary_coverage.h"
//#include "const_coverage.h"
//#include "null_coverage.h"
#include "pyxis/derm/neighbour_iterator.h"
#include "pyxis/utility/tester.h"

// standard includes

//! The name of the class 
const std::string PYXDilationFilter::kstrScope = "PYXDilationFilter"; 

//! The unit test class
TesterUnit<PYXDilationFilter> gTester;

/*!
The unit test method for the class.
*/
void PYXDilationFilter::test() 
{
	//int nNumIndices = 11; 
	//std::vector<PYXIcosIndex> testIndices(nNumIndices); 
	//
	//boost::shared_ptr<PYXBinaryCoverage> spCoverage(new PYXBinaryCoverage()); 
	//boost::shared_ptr<PYXDilationFilter> spDilation(new PYXDilationFilter()); 
	//spDilation->setInput(spCoverage); 
	//
	////Random Indices
	//testIndices[0] = "A-500";
	//testIndices[1] = "1-502"; //Pentagon
	//testIndices[2] = "C-504"; 
	//testIndices[3] = "D-503"; 
	//testIndices[4] = "E-506"; 
	//testIndices[5] = "F-502"; //Ok
	//testIndices[6] = "G-506"; 
	//testIndices[7] = "3-504"; //Pentagon
	//testIndices[8] = "C-502"; 
	//testIndices[9] = "B-300"; 
	//testIndices[10] = "K-501"; 

	//for (int nCount = 0; nCount < nNumIndices; ++nCount)
	//{
	//	TEST_ASSERT(1 == spDilation->getCoverageValue(testIndices[nCount]).getInt32());
	//}

	//PYXIcosIndex hexIndex = "A-500"; 
	//PYXIcosIndex pentIndex = "1-502";
	//
	//boost::shared_ptr<PYXDilationFilter> spDilat (new PYXDilationFilter()); 

	//boost::shared_ptr<PYXNullCoverage> spNullCoverage (new PYXNullCoverage()); 
	//spDilat->setInput(spNullCoverage); 

	//// Test  Null 
	//TEST_ASSERT(spDilat->getCoverageValue(hexIndex).isNull()); 
	//TEST_ASSERT(spDilat->getCoverageValue(pentIndex).isNull()); 
	
}


/*!
Default Constructor.
*/
PYXDilationFilter::PYXDilationFilter()
{
	int nValue = 1; 
	m_structElement = PYXValue::create(PYXValue::knInt32,&nValue,7,false); 

	for (int nElement =0; nElement < m_structElement.getArraySize(); ++nElement)
	{
		m_structElement.setInt(nElement,nValue); 
	}
}



/*!
Get the coverage value at the specified index. The cell at the index and it's 
direct neighbours are examined for the highest value. The highest value 
found among a cell's direct neighbours is then set as the value for the 
cell at this specified index. This occurs for each band in the dataset. 
Ex. If RGB, PYXValue returned would contain the highest red, green, blue 
values found among the cell's direct neighbours. 

\param index		The pyxis index. 
\param nFieldIndex	The field index. 

\return A PYXValue containing the maximum value found among the cells direct 
		neighbours
*/
PYXValue PYXDilationFilter::getCoverageValue(
	const PYXIcosIndex &index,
	int nFieldIndex	) const 
{
	assert(getInput() && "No input coverage set.");
	PYXValue rtnValue = getInput()->getCoverageValue(index, nFieldIndex); 
	int nElements = rtnValue.getArraySize(); 

	if(index.isNull())
	{
		return PYXValue(); 
	}

	for (int nElement = 0; nElement < nElements; ++nElement) 
	{
		PYXNeighbourIterator itNeighbour(index);
		double fMax = std::numeric_limits<double>::min(); 
		double fTMax = 0; 
		while (!itNeighbour.end())
		{
			fTMax = getInput()->getCoverageValue(itNeighbour.getIndex(), nFieldIndex).getDouble(nElement); 
		
			if (fTMax > fMax) 
			{
				fMax = fTMax; 
			}
			itNeighbour.next(); 
		}
		rtnValue.setDouble(nElement,fMax); 
	}
	return rtnValue; 
}