/******************************************************************************
erosion_filter.cpp

begin		: 2006-05-23
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "erosion_filter.h"

// local includes
//#include "binary_coverage.h"
//#include "const_coverage.h"
#include "pyxis/derm/neighbour_iterator.h"
//#include "null_coverage.h"
#include "pyxis/utility/tester.h"

// standard includes

//! The name of the class 
const std::string PYXErosionFilter::kstrScope = "PYXErosionFilter"; 

//! The unit test class
TesterUnit<PYXErosionFilter> gTester;

/*!
The unit test method for the class.
*/
void PYXErosionFilter::test() 
{
	//int nNumIndices = 11; 
	//std::vector<PYXIcosIndex> testIndices(nNumIndices); 
	//
	//boost::shared_ptr<PYXBinaryCoverage> spCoverage(new PYXBinaryCoverage()); 
	//boost::shared_ptr<PYXErosionFilter> spErosion(new PYXErosionFilter()); 
	//spErosion->setInput(spCoverage); 
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
	//	TEST_ASSERT(0 == spErosion->getCoverageValue(testIndices[nCount]).getInt32());
	//}

	//PYXIcosIndex hexIndex = "A-500"; 
	//PYXIcosIndex pentIndex = "1-502";

	//boost::shared_ptr<PYXNullCoverage> spNullCov(new PYXNullCoverage()) ;
	//boost::shared_ptr<PYXErosionFilter> spErode(new PYXErosionFilter()); 
	//spErode->setInput(spNullCov); 

	//// Test  Null 
	//TEST_ASSERT(spErode->getCoverageValue(hexIndex).isNull()); 
	//TEST_ASSERT(spErode->getCoverageValue(pentIndex).isNull()); 	
}


/*!
Default Constructor.
*/
PYXErosionFilter::PYXErosionFilter()
{
	int nValue = 1; 
	m_structElement = PYXValue::create(PYXValue::knInt32,&nValue,7,false); 

	for (int nElement =0; nElement < m_structElement.getArraySize(); ++nElement)
	{
		m_structElement.setInt(nElement,nValue); 
	}
}

/*!
Get the value at the specified index. The cell at the index and it's 
direct neighbours are examined for the lowest value. The lowest value 
found among a cell's direct neighbours is then set as the value for the 
cell at this specified index. This occurs for each band in the dataset. 
Ex. If RGB, PYXValue returned would contain the lowest red, green, blue 
values found among the cell's direct neighbours. 

\param index		The pyxis index. 
\param nFieldIndex	The field index. 

\return A PYXValue containing the minmum value found among the cells direct neighbours
*/
PYXValue PYXErosionFilter::getCoverageValue(
	const PYXIcosIndex &index,
	int nFieldIndex	) const 
{
	assert(getInput() && "No input coverage set.");
	PYXValue rtnValue = getInput()->getCoverageValue(index, nFieldIndex); 
	
	int nElements = rtnValue.getArraySize(); 

	for (int nElement = 0; nElement < nElements; ++nElement) 
	{
		PYXNeighbourIterator itNeighbour(index);
		double fMin = std::numeric_limits<double>::max();  
		double fTMin = 0; 

		while (!itNeighbour.end())
		{
			fTMin = getInput()->getCoverageValue(itNeighbour.getIndex(), nFieldIndex).getDouble(nElement); 
		
			if(fTMin < fMin) 
			{
				fMin = fTMin; 
			}
			itNeighbour.next(); 
		}
		rtnValue.setDouble(nElement,fMin);
	}
	return rtnValue; 
}



