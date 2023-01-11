/******************************************************************************
fourier_transform_diamond.cpp

begin		: 2006-05-31
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "fourier_transform_diamond.h"

// local includes
//#include "const_coverage.h"
//#include "binary_coverage.h"
#include "pyxis/derm/diamond_iterator.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/fourier_diamond_iterator.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <math.h>

//! Constants

//! The unit test class
TesterUnit<PYXFourierTransformDiamond> gTester;

/*!
The unit test method for the class.
*/
void PYXFourierTransformDiamond::test()
{
//	std::string testFile1 = AppServices::makeTempFile("txt");
//	std::string testFile2 = AppServices::makeTempFile("txt");
//
//	boost::shared_ptr<PYXConstCoverage> spConstCoverage(new PYXConstCoverage(PYXValue(100)));
//	PYXIcosIndex pentagonIndex = "5-20000000"; //Pentagon Index.
//	PYXIcosIndex pentRootIndex = "5-2";
//	PYXIcosIndex classOneIndex = "C-0200003000000"; //Class 1 Index.
//	PYXIcosIndex hexRootIndex = "C-0";
//	PYXIcosIndex classTwoIndex = "C-03000000"; // Class 2 Index. 
//	int nRes = 2; 
//
//	PYXFourierTransformDiamond transform(pentagonIndex, nRes); 
//
//	transform.setInput(spConstCoverage); 
//	transform.getLatticePts(); 
//	transform.spatialToFreq();  
//
//	boost::shared_ptr<PYXValueTile> frequencyTile = transform.createFreqDataSource();
//	TEST_ASSERT(frequencyTile.get() != 0);
//	TEST_ASSERT(frequencyTile->getDataChannelCount(0) == 2); 
//	TEST_ASSERT(frequencyTile->getDataChannelType(0) == PYXValue::eType::knFloat); 
//
//	PYXFourierTransformDiamond transformOne(classOneIndex,nRes); 
//
//	transformOne.setInput(spConstCoverage); 
//	transformOne.getLatticePts(); 
//	transformOne.spatialToFreq(); 
//
//	boost::shared_ptr<PYXValueTile> frequencyTile1 = transformOne.createFreqDataSource();
//	TEST_ASSERT(frequencyTile1.get() != 0) ;
//	TEST_ASSERT(frequencyTile1->getDataChannelCount(0) == 2); 
//	TEST_ASSERT(frequencyTile1->getDataChannelType(0) == PYXValue::eType::knFloat); 
//
//	PYXFourierTransformDiamond transformTwo(classTwoIndex,nRes);
//
//	transformTwo.setInput(spConstCoverage); 
//	transformTwo.getLatticePts(); 
//	transformTwo.spatialToFreq(); 
//
//	boost::shared_ptr<PYXValueTile> frequencyTile2 = transformTwo.createFreqDataSource();
//	TEST_ASSERT(frequencyTile2.get() != 0) ;
//	TEST_ASSERT(frequencyTile2->getDataChannelCount(0) == 2); 
//	TEST_ASSERT(frequencyTile2->getDataChannelType(0) == PYXValue::eType::knFloat); 
//	
//	PYXFourierTransformDiamond transformThree(classOneIndex,nRes); 
//
//	//Tests assertions will be thrown. Uncomment to test. 
//	/*
//	transformThree.getLatticePts(); 
//	transformThree.spatialToFreq(); 
//	transformThree.createFreqDataSource(hexRootIndex,classTwoIndex.getResolution()); 
//	transformThree.createSpatialDataSource(hexRootIndex,classTwoIndex.getResolution()); 
//	*/
}

/*!
Default Constructor.
*/
PYXFourierTransformDiamond::PYXFourierTransformDiamond(
	PYXIcosIndex& index, int nRadius): PYXFourierTransform(index, nRadius)
{
}

/*!
Destructor.
*/
PYXFourierTransformDiamond::~PYXFourierTransformDiamond()
{
}

/*!
Since the lattice points for this fourier transform are to appear as a diamond. 
A value tile containing the lattice points in the spatial domain is created, by 
iterating over the entire diamond and taking the points out of the orginal 
image in iterative order. A shared pointer to the value tile is 
returned to enable the value tile to act as a data source, when 
passed through a filter. The values are also streamed out to a file to enable 
viewing the original image and the transformed image in an independant viewer. The 
file is a text formated, space delimited file in the following format. 

K, J(base vectors), The value at J,K as a double. 

*/
void PYXFourierTransformDiamond::createSpatialData()const 
{
	assert(getInput().get() != 0 && "No input coverage set."); 

	PYXFourierDiamondIterator itJk(getIndex(), getRadius());
	PYXDiamondIterator itDiamond(getIndex(), getRadius());

	std::ofstream out; 
	out.open(getRawOutputFilePath().c_str(), std::ios_base::out); 

	while (!itDiamond.end())
	{
		double fVal = getInput()->getCoverageValue(itDiamond.getIndex()).getDouble();
		
		//TODO: Remove output once visualizer added. 
		out << itJk.getJK().nK << ' ' << itJk.getJK().nJ << ' ' << fVal << '\n';
		itJk.next();
		itDiamond.next();
	}
}

/*!
Gets the lattice points from the orignal image to perform the fourier on. The 
lattice points are retrieved in iterative order via the diamond iterator. 
The values are stored in a vector. The order in which the values are retrieved 
form a diamond shape. 
*/
void PYXFourierTransformDiamond::getLatticePts()
{
	assert(getInput().get() != 0 && "No Input Coverage set."); 

	PYXFourierDiamondIterator it(getIndex(), getRadius());
	PYXDiamondIterator itDiamond(getIndex(), getRadius());

	std::vector<double>::size_type nSize = static_cast<unsigned int>(it.getTotalCells());

	m_vecLatticePts.resize(nSize);
	int nIndex = 0; 
	PYXValue value;

	while (!itDiamond.end())
	{
		value = getInput()->getCoverageValue(itDiamond.getIndex()); 
		m_vecLatticePts[nIndex] = getInput()->getCoverageValue(itDiamond.getIndex()).getDouble(); 
		++nIndex;
		itDiamond.next(); 
	}
}

/*!
Creates a ValueTile containing the values of the lattice after the fourier 
Transform. The values are taken out a vector in the form of a complex number. 
A PYXValue which is an array of 2 stores the complex value. The first value 
of the PYXValue array is the real part of the data following the fourier 
transform and the second value is the imaginary part of the data. These values 
are then placed back onto a tile according to the dimaond iterative order. A 
shared pointer to the value tile is returned to enable the tile to act 
as a datasource through a view in the TVT when it is created. 
These values are also streamed out to a text file, space delimited to enable 
viewing of the data in the frequency domain via an independant viewer. 
The format of this file is. 

K,J(Base Vectors), Double(Real Part), Double(Imaginary Part) 

\return shared pointer to the value tile containing the transformed 
		data in the frequency domain. 
*/
PYXPointer<PYXValueTile> PYXFourierTransformDiamond::createFreqDataSource() const
{
	std::string strIndex = getIndex().toString(); 
	std::string::size_type zeroOccur = strIndex.find_last_not_of("0");
	strIndex = strIndex.substr(0,zeroOccur + 2);
	PYXIcosIndex rootIndex(strIndex);

	assert(!m_vecSpatialVect.empty() && "No frequency data to create data source from"); 

	PYXTile rootTile(rootIndex, getIndex().getResolution()); 
	std::vector<PYXValue::eType> vecTypes; 
	std::vector<int> vecCounts;

	vecTypes.push_back(PYXValue::eType::knFloat); 
	vecCounts.push_back(2); 

	PYXFourierDiamondIterator itJk(getIndex(), getRadius());
	PYXDiamondIterator itDiamond(getIndex(), getRadius());

	PYXPointer<PYXValueTile> frequencyDataSource = PYXValueTile::create(rootTile, vecTypes, vecCounts);

	std::ofstream out;
	int nValue = 0; 
	PYXValue complexValue = PYXValue::create(PYXValue::knFloat, &nValue, 2, false); 

	out.open(getProcessedOutputFilePath().c_str(), std::ios_base::out); 

	int nCount = 0; 
	while (!itDiamond.end())
	{
		complexValue.setDouble(0, m_vecSpatialVect[nCount].re); 
		complexValue.setDouble(1, m_vecSpatialVect[nCount].im); 
		if (itDiamond.getIndex().isDescendantOf(rootIndex))
		{
			frequencyDataSource->setValue(itDiamond.getIndex(), 0, complexValue);
		}
		out << itJk.getJK().nK << ' ' << itJk.getJK().nJ << ' '
			<< complexValue.getDouble(0) << ' '
			<< complexValue.getDouble(1) << '\n';

		itJk.next();
		itDiamond.next(); 
		++nCount; 
	}

	out.close(); 
	return frequencyDataSource; 
}
