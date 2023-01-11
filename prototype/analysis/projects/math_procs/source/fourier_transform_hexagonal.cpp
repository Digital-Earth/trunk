/******************************************************************************
fourier_transform_hexagonal.cpp

begin		: 2006-05-31
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "fourier_transform_hexagonal.h"

// local includes
//#include "const_coverage.h"
#//include "binary_coverage.h"
#include "pyxis/derm/fourier_hexagonal_iterator.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <math.h>

//! Constants

//! The unit test class
TesterUnit<PYXFourierTransformHexagon> gTester;

/*!
The unit test method for the class.
*/
void PYXFourierTransformHexagon::test()
{
//	boost::shared_ptr<PYXConstCoverage> spConstCoverage(new PYXConstCoverage(PYXValue(100)));
//	PYXIcosIndex pentagonIndex = "5-20000000"; //Pentagon Index.
//	PYXIcosIndex pentRootIndex = "5-2";
//	PYXIcosIndex classOneIndex = "C-0200003000000"; //Class 1 Index.
//	PYXIcosIndex hexRootIndex = "C-0";
//	PYXIcosIndex classTwoIndex = "C-03000000"; // Class 2 Index. 
//	int nRes = 2; 
//
//	PYXFourierTransformHexagon transform(pentagonIndex,nRes); 
//
//	transform.setInput(spConstCoverage); 
//	transform.getLatticePts(); 
//	transform.spatialToFreq(); 
//	
//	boost::shared_ptr<PYXValueTile> frequencyTile = 
//		transform.createFreqData();
//
//	TEST_ASSERT(frequencyTile.get() != 0);
//	TEST_ASSERT(frequencyTile->getDataChannelCount(0) == 2); 
//	TEST_ASSERT(frequencyTile->getDataChannelType(0) == PYXValue::eType::knFloat); 
//
//	PYXFourierTransformHexagon transformOne(classOneIndex, nRes); 
//
//	transformOne.setInput(spConstCoverage); 
//	transformOne.getLatticePts(); 
//	transformOne.spatialToFreq(); 
//	
//	boost::shared_ptr<PYXValueTile> frequencyTile1 = 
//		transformOne.createFreqData();
//
//	TEST_ASSERT(frequencyTile1.get() != 0) ;
//	TEST_ASSERT(frequencyTile1->getDataChannelCount(0) == 2); 
//	TEST_ASSERT(frequencyTile1->getDataChannelType(0) == PYXValue::eType::knFloat); 
//
//	PYXFourierTransformHexagon transformTwo(classTwoIndex, nRes);
//
//	transformTwo.setInput(spConstCoverage); 
//	transformTwo.getLatticePts(); 
//	transformTwo.spatialToFreq(); 
//	
//	boost::shared_ptr<PYXValueTile> frequencyTile2 = 
//		transformTwo.createFreqData();
//
//	TEST_ASSERT(frequencyTile2.get() != 0);
//	TEST_ASSERT(frequencyTile2->getDataChannelCount(0) == 2);
//	TEST_ASSERT(frequencyTile2->getDataChannelType(0) == PYXValue::eType::knFloat);
}

/*!
Default Constructor.
*/
PYXFourierTransformHexagon::PYXFourierTransformHexagon(
	const PYXIcosIndex& index, int nRadius): 
	PYXFourierTransform(index, nRadius)	
{
}

/*!
Destructor.
*/
PYXFourierTransformHexagon::~PYXFourierTransformHexagon()
{
}

/*!
Since the lattice points for this fourier transform are to appear as a hexagon. 
A value tile containing the lattice points in the spatial domain is created, by 
iterating over the entire hexagon and taking the points out of the orginal 
image in iterative order. A shared pointer to the value tile is 
returned to enable the value tile to act as a data source, when 
passed through a filter. The values are also streamed out to a file to enable 
viewing the original image and the transformed image in an independant viewer. The 
file is a text formated, space delimited file in the following format. 

K, J(base vectors), The value at J,K as a double. 

*/
void PYXFourierTransformHexagon::createSpatialData() const
{
	assert(getInput().get() != 0 && "No input coverage set.");

	//TODO: When visualize plugin written remove! 
	
	std::ofstream out;
	out.open(getRawOutputFilePath().c_str(), std::ios_base::out);
	
	PYXHexagonalIterator itHex(getIndex(), getRadius());
	PYXValue value;
	while (!itHex.end())
	{
		PYXIcosIndex myIndex = itHex.getIndex();
		double fVal = (getInput()->getCoverageValue(myIndex).getDouble(0));
		value.setDouble(fVal);
		
		out << itHex.getK() << " " << itHex.getJ() << " " << fVal << "\n";
		itHex.next();
	}
	out.close();
}

/*!
Gets the lattice points from the orignal image to perform the fourier on. The 
lattice points are retrieved in iterative order via the hexagon iterator. 
The values are stored in a vector. The order in which the values are retrieved 
form a hexagon shape. 
*/
void PYXFourierTransformHexagon::getLatticePts()
{
	assert(getInput().get() != 0 && "No Input Coverage set.");

	PYXHexagonalIterator itHex(getIndex(), getRadius());

	std::vector<double>::size_type nSize =
		static_cast<unsigned int>(itHex.getTotalCells());
	
	m_vecLatticePts.resize(nSize);
	int nIndex = 0;
	PYXValue value;

	while (!itHex.end())
	{
		value = getInput()->getCoverageValue(itHex.getIndex());
		m_vecLatticePts[nIndex] = getInput()->
			getCoverageValue(itHex.getIndex()).getDouble();
		++nIndex;
		itHex.next();
	}
}

/*!
Creates a ValueTile containing the values of the lattice after the fourier 
Transform. The values are taken out a vector in the form of a complex number. 
A PYXValue which is an array of 2 stores the complex value. The first value 
of the PYXValue array is the real part of the data following the fourier 
transform and the second value is the imaginary part of the data. These values 
are then placed back onto a tile according to the hexagon iterative order. A 
shared pointer to the value tile is returned to enable the tile to act 
as a datasource through a view in the TVT when it is created. 
These values are also streamed out to a text file, space delimited to enable 
viewing of the data in the frequency domain via an independant viewer. 
The format of this file is. 

K,J(Base Vectors), Double(Real Part), Double(Imaginary Part) 

*/
PYXPointer<PYXValueTile> PYXFourierTransformHexagon::createFreqData() const
{
	assert(!m_vecSpatialVect.empty() &&
		"No frequency data to create data source from");
	
	std::string strIndex = getIndex().toString(); 
	std::string::size_type zeroOccur = strIndex.find_last_not_of("0");
	strIndex = strIndex.substr(0,zeroOccur + 2);
	PYXIcosIndex rootIndex(strIndex);

	PYXTile rootTile(rootIndex, getIndex().getResolution());
	std::vector<PYXValue::eType> vecTypes;
	std::vector<int> vecCounts;

	vecTypes.push_back(PYXValue::eType::knFloat);
	vecCounts.push_back(2);

	PYXHexagonalIterator itHex(getIndex(), getRadius());

	PYXPointer<PYXValueTile> frequencyDataSource = 
		PYXValueTile::create(rootTile, vecTypes, vecCounts);
	
	std::ofstream out;
	int nValue = 0;
	PYXValue complexValue =
		PYXValue::create(PYXValue::knFloat, &nValue, 2, false);
	out.open(getProcessedOutputFilePath().c_str(), std::ios_base::out);

	int nCount = 0;
	while (!itHex.end())
	{
		complexValue.setDouble(0, m_vecSpatialVect[nCount].re);
		complexValue.setDouble(1, m_vecSpatialVect[nCount].im);
		frequencyDataSource->setValue(itHex.getIndex(), 0, complexValue);

		out << itHex.getK() << " " << itHex.getJ() << " "
			<< complexValue.getDouble(0) << " " << complexValue.getDouble(1) << "\n";
		itHex.next();
		++nCount;
	}
	out.close();
	return frequencyDataSource;
}
