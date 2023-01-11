/******************************************************************************
histogram_filter.cpp

begin		: 2006-05-09
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "histogram_filter.h"

// local includes
#include "binner.h"
#include "const_coverage.h"
#include "pyxis/derm/index.h"
#include "pyxis/utility/tester.h"



//! The Name of the class. 
const std::string PYXHistogramFilter::kstrScope = "PYXHistogramFilter";
boost::shared_ptr<PYXHistogramFilter> PYXHistogramFilter::spHistogramFilter;

//! Resolution for histogram. 
const int knHistogramResolution = 6; 

bool PYXHistogramFilter::m_bEnabled = true; 

// standard includes

//! The unit test class
TesterUnit<PYXHistogramFilter> gTester;

/*!
The unit test method for the class.
*/
void PYXHistogramFilter::test()
{
	PYXIcosIndex  refIndex = "A-0500"; 
	m_bEnabled = true; 
	boost::shared_ptr<PYXConstCoverage> spConstCov (new PYXConstCoverage(PYXValue(100))); 
	boost::shared_ptr<PYXHistogramFilter> spHistogram(new PYXHistogramFilter()); 
	spHistogram->setInput(spConstCov);  

	spHistogram->startBinning();

	std::vector<int> testVect = spHistogram->getHistogramData();
	std::vector<int>::iterator vectorIterator; 
	
	int itemCount = 0; 

	for (vectorIterator = testVect.begin(); vectorIterator != testVect.end(); vectorIterator++)
	{
		itemCount += *(vectorIterator);
	}

	//TEST_ASSERT(1 == itemCount); 
//	m_bEnabled = false; 

}


/*!
Default Constructor.
*/
PYXHistogramFilter::PYXHistogramFilter():
	m_bRecalculateDataFlag(true),
	m_bDataRecalculated(false)
{

	m_spBinner = boost::shared_ptr<PYXBinner> (new PYXRgbBinner()); 
	m_vecBins.resize(m_spBinner->getBinCount()); 
}


/*!
Destructor.
*/
PYXHistogramFilter::~PYXHistogramFilter()
{
	
}

/*!
SetInput Sets the input coverage to generate a histogram of. 

\param spInput	Input coverage. 

*/
void PYXHistogramFilter::setInput(boost::shared_ptr<PYXCoverage> spInput) 
{
	assert(spInput.get() != 0 && "Invalid parameter."); 

	PYXFilter::setInput(spInput);
	
	boost::shared_ptr<PYXGeometry> spGeometry(getInput()->getGeometry()->clone());
	setGeometry(spGeometry);
	
	m_bRecalculateDataFlag = true; 
	m_bDataRecalculated = false; 
}

/*!
SetGeometry Sets a specific geometry to generate a histogram of. 

\param spGeometry  The geometry to use in generating a histogram. 

*/
void PYXHistogramFilter::setGeometry(boost::shared_ptr<PYXGeometry> spGeometry)
 {
	assert(spGeometry.get() !=0 && "Invalid parameter."); 
	m_spGeometry = spGeometry; 

	if (m_spGeometry->getCellResolution() > knHistogramResolution) 
	{
		// Change the resolution to one that doesn't kill the TVT.
		m_nResolution = knHistogramResolution;
		m_spGeometry->setCellResolution(m_nResolution);
	}

	m_bRecalculateDataFlag = true;
	m_bDataRecalculated = false; 

}

/*!
GetGeometry Gets a geometry to use in generating histogram. Returns set geometry if 
one has been set otherwise returns the geometry of the input. 

\return A shared pointer, if set, to the geometry otherwise bald pointer to input 
	geometry. 
*/
boost::shared_ptr<const PYXGeometry> PYXHistogramFilter::getGeometry() const
{
	return m_spGeometry; 
}

/*!
GetBins calculates which bin a PYXvalue is to be placed in. Places the value 
into the designated bin. Storing all bins in a vector. If the input or geometry 
is changed then all whole new calculation begins. 

*/
void PYXHistogramFilter::startBinning()
{
	assert(getInput().get() != 0 && "No input set."); 
	
	if (m_bEnabled)
	{
		if (m_bRecalculateDataFlag) 
		{
			reset(); 
			int nIndex = -1; 

			PYXPointer<PYXIterator> spPyxIterator(getGeometry()->getIterator());

			while (!spPyxIterator->end()) 
			{
				// Don't bin a Null value! 
				PYXValue v = getInput()->getCoverageValue(spPyxIterator->getIndex());
				if (!v.isNull())
				{
					nIndex = m_spBinner->bin(v);
					++m_vecBins[nIndex];
				}
				spPyxIterator->next(); 
			}

			m_bDataRecalculated  = true; 
			m_bRecalculateDataFlag = false; 
		}	
	} 
}

/*
Reset Resets the container containing the bins. 
*/
void PYXHistogramFilter::reset()
{
	std::vector<int>::iterator vectorIterator; 

	for (vectorIterator = m_vecBins.begin(); vectorIterator != m_vecBins.end(); vectorIterator++)
	{
		*(vectorIterator) = 0; 
	}
}

void PYXHistogramFilter::setHistogramResolution(int nResolution)
{
	assert((nResolution >= 1 &&  nResolution < 40) && "Invalid geometry resolution.");
	m_nResolution = nResolution;
	m_spGeometry->setCellResolution(nResolution);
}