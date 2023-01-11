/******************************************************************************
stats_filter.cpp

begin		: 2006-08-04
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stats_filter.h"

// local includes
#include "pyxis/utility/tester.h"
#include "stats_test_coverage.h"

// standard includes
#include "atltime.h"

//! The unit test class
TesterUnit<PYXStatsFilter> gTester;

//! The names of the meta data fields
const std::string PYXStatsFilter::kstrTotal = "Total Cells"; 
const std::string PYXStatsFilter::kstrTotalNotNull = "Total Cells (not null)"; 
const std::string PYXStatsFilter::kstrTotalNotZero = "Total Values (not zero)"; 
const std::string PYXStatsFilter::kstrSums = "Cells Sums"; 
const std::string PYXStatsFilter::kstrMax = "Maximum Values"; 
const std::string PYXStatsFilter::kstrMin = "Minimum Values"; 
const std::string PYXStatsFilter::kstrMean = "Arithmetic Mean"; 
const std::string PYXStatsFilter::kstrRMS = "Root Mean Square"; 
const std::string PYXStatsFilter::kstrCompTime = "Time to Compute (sec)"; 

/*!
The unit test method for the class.
*/
void PYXStatsFilter::test()
{
	PYXIcosIndex openIndex; 
	boost::shared_ptr<StatsTestCoverage> spCoverage(new StatsTestCoverage()); 
	/*spCoverage->open(openIndex,5);
	PYXStatsFilter statsAnalysis;

	statsAnalysis.setInput(spCoverage);
	
	statsAnalysis.calculateStatistics();
	statsAnalysis.getCellCount();
	
	TEST_ASSERT(statsAnalysis.getMaxValues().getDouble(0) == 7);
	TEST_ASSERT(statsAnalysis.getMaxValues().getDouble(1) == 14);
	TEST_ASSERT(statsAnalysis.getMaxValues().getDouble(2) == 12);
	
	TEST_ASSERT(statsAnalysis.getMinValues().getDouble(0) == 0);
	TEST_ASSERT(statsAnalysis.getMinValues().getDouble(1) == 0);
	TEST_ASSERT(statsAnalysis.getMinValues().getDouble(2) == 0);

	TEST_ASSERT(statsAnalysis.getMeanValues().getDouble(0) == 3.5);
	TEST_ASSERT(static_cast<int>(statsAnalysis.getMeanValues().getDouble(1)) == 7);
	TEST_ASSERT(static_cast<int>(statsAnalysis.getMeanValues().getDouble(2)) == 5);

	TEST_ASSERT(static_cast<int>(statsAnalysis.getStdDevValues().getDouble(0)) == -2);
	TEST_ASSERT(static_cast<int>(statsAnalysis.getStdDevValues().getDouble(0)) == 1);
	TEST_ASSERT(static_cast<int>(statsAnalysis.getStdDevValues().getDouble(0)) == 0);

	TEST_ASSERT(static_cast<int>(statsAnalysis.getArithmeticMean()) == 5);
	TEST_ASSERT(statsAnalysis.getCellCount() == 2432);*/
}

/*!
Default Constructor.
*/
PYXStatsFilter::PYXStatsFilter():
	m_bIsDirty(true)
{
	InitializeStats();
}

/*!
Destructor.
*/
PYXStatsFilter::~PYXStatsFilter()
{
}

/*!
Initialize the statistics variables to starting state.
*/
void PYXStatsFilter::InitializeStats()
{
	m_nCellCount = 0;
	m_nNonNullCellCount = 0;
	m_vecMaxValues.clear();
	m_vecMinValues.clear();
	m_vecMeanValues.clear();
	m_vecRMSValues.clear();
	m_vecSums.clear();
	m_vecNonZeroCellCount.clear();
}

/*!
Add the table definitions to report the stats into.

\param nChannels  The number of data points for each PYXValue in the coverage.
*/
void PYXStatsFilter::AddStatsTableDefinitions(int nChannels)
{
	addField(kstrTotal, PYXFieldDefinition::knContextNone, PYXValue::knUInt32, 1);
	addField(kstrTotalNotNull, PYXFieldDefinition::knContextNone, PYXValue::knUInt32, 1);
	addField(kstrTotalNotZero, PYXFieldDefinition::knContextNone, PYXValue::knUInt32, nChannels);
	addField(kstrSums, PYXFieldDefinition::knContextNone, PYXValue::knDouble, nChannels);
	addField(kstrMax, PYXFieldDefinition::knContextNone, PYXValue::knDouble, nChannels);
	addField(kstrMin, PYXFieldDefinition::knContextNone, PYXValue::knDouble, nChannels);
	addField(kstrMean, PYXFieldDefinition::knContextNone, PYXValue::knDouble, nChannels);
	addField(kstrRMS, PYXFieldDefinition::knContextNone, PYXValue::knDouble, nChannels);
	addField(kstrCompTime, PYXFieldDefinition::knContextNone, PYXValue::knUInt32, 1);
}

//! Put the stats results into the table fields.
void PYXStatsFilter::PopulateStatsTableDefinitions()
{
	setFieldValueByName(PYXValue(m_nCellCount),kstrTotal);
	setFieldValueByName(PYXValue(m_nNonNullCellCount),kstrTotalNotNull);
	setFieldValueByName(PYXValue(&m_vecNonZeroCellCount[0], m_vecNonZeroCellCount.size()),kstrTotalNotZero);
	setFieldValueByName(PYXValue(&m_vecSums[0], m_vecSums.size()),kstrSums);
	setFieldValueByName(PYXValue(&m_vecMaxValues[0], m_vecMaxValues.size()),kstrMax);
	setFieldValueByName(PYXValue(&m_vecMinValues[0], m_vecMinValues.size()),kstrMin);
	setFieldValueByName(PYXValue(&m_vecMeanValues[0], m_vecMeanValues.size()),kstrMean);
	setFieldValueByName(PYXValue(&m_vecRMSValues[0], m_vecRMSValues.size()),kstrRMS);
}

/*!
Sets the input coverage to perform a statistical calculation on.
Once the input coverage is changed a is dirty flag is set, indicating
the resulting data needs to be recalculated.

\param spCoverage	The coverage to perform a statistical calculation on.
*/
void PYXStatsFilter::setInput(boost::shared_ptr<PYXCoverage> spCoverage)
{
	assert(spCoverage && "Invalid input parameter.");
	PYXFilter::setInput(spCoverage);
	m_bIsDirty = true; 
}

/*!
Performs the stastical calculation within the geometry of the input coverage.
The maximum, minimum, mean and RMS values are calculated on a channel 
by channel basis. The results of these calculations are stored in vectors 
of doubles. Since this is a time intensive operation based on the size of
the geometry the stats are only recalculated if the input coverage changes.
*/
void PYXStatsFilter::calculateStatistics()
{
	assert(getInput() && "No input set to perform statistical analysis on.");
	if (m_bIsDirty)
	{
		m_bIsDirty = false;
		CTime startTime = CTime::GetCurrentTime();
		PYXValue currentValue;
		InitializeStats();
		bool bArraySizeSet = false;

		//Iterated over all cells in geometry.
		for (	PYXPointer<PYXIterator> spIt(getInput()->getGeometry()->getIterator());
				!spIt->end();
				spIt->next()	)
		{
			currentValue = getInput()->getCoverageValue(spIt->getIndex());
			if (!currentValue.isNull())
			{
				int nArraySize = currentValue.getArraySize(); 

				if (!bArraySizeSet)
				{
					// set up the array sizes and initialize all the elements
					m_vecMaxValues.resize(nArraySize);
					m_vecMinValues.resize(nArraySize);
					m_vecMeanValues.resize(nArraySize);
					m_vecRMSValues.resize(nArraySize);
					m_vecNonZeroCellCount.resize(nArraySize);
					m_vecSums.resize(nArraySize);
					for (int nArrayElement = 0; nArrayElement < 
						nArraySize; ++nArrayElement)
					{
						m_vecMeanValues[nArrayElement] = 0.0;
						m_vecRMSValues[nArrayElement] = 0.0;
						m_vecSums[nArrayElement] = 0.0;
						m_vecNonZeroCellCount[nArrayElement] = 0;
						m_vecMaxValues[nArrayElement] = currentValue.getDouble(nArrayElement);
						m_vecMinValues[nArrayElement] = currentValue.getDouble(nArrayElement);
					}
					bArraySizeSet = true;
				}
				
				//loop through all the channels of the PYXValue.
				for (int nArrayElement = 0; nArrayElement < 
					nArraySize; ++nArrayElement)
				{
					if (currentValue.getDouble(nArrayElement) > 
							m_vecMaxValues[nArrayElement])
					{
						m_vecMaxValues[nArrayElement] = 
							currentValue.getDouble(nArrayElement);
					}
					if (currentValue.getDouble(nArrayElement) < 
								m_vecMinValues[nArrayElement])
					{
						m_vecMinValues[nArrayElement] =
							currentValue.getDouble(nArrayElement);
					}

					if (currentValue.getDouble(nArrayElement) != 0.0)
					{
						++m_vecNonZeroCellCount[nArrayElement];
					}

					//Sum the data for each channel in the input coverage.
					m_vecSums[nArrayElement] +=
						currentValue.getDouble(nArrayElement);
					
					//Sum of the squares for each channel in the input coverage.
					m_vecRMSValues[nArrayElement] +=
						pow(currentValue.getDouble(nArrayElement), 2.0);
				}
				++m_nNonNullCellCount;
			}
			++m_nCellCount;	
		}

		if (m_nNonNullCellCount > 0)
		{
			//Calculate the mean on per channel.
			for (int nArrayElement = 0; nArrayElement < 
				static_cast<int>(m_vecSums.size()); ++nArrayElement )
			{
				m_vecMeanValues[nArrayElement] = 
					m_vecSums[nArrayElement] /
					static_cast<double>(m_nNonNullCellCount);
			}

			//Calculate the standard deviations.
			for (int nArrayElement = 0; nArrayElement < 
				static_cast<int>(m_vecRMSValues.size()); ++nArrayElement)
			{
				m_vecRMSValues[nArrayElement] =
					sqrt(m_vecRMSValues[nArrayElement] /
					static_cast<double>(m_nNonNullCellCount));
			}
		}
		CTime endTime = CTime::GetCurrentTime();

		AddStatsTableDefinitions(currentValue.getArraySize());
		PopulateStatsTableDefinitions();

		CTimeSpan computeTime = endTime - startTime;
		setFieldValueByName(PYXValue((long)computeTime.GetTotalSeconds()),kstrCompTime);
	}
}

