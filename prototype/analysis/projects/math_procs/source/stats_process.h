#ifndef STATS_FILTER_H
#define STATS_FILTER_H
/******************************************************************************
stats_filter.h

begin		: 2006-08-04
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "filter_impl.h"
// standard includes

// forward declarations

/*!
PYXStatsFilter, performs a statistical analysis on a coverage. 
The max, min, mean and root mean square are calculated on a per
channel basis for the coverage.  The nulber of cells, non-zero cells,
and non-null cells are also calculated.
*/
//! PYXStatsFilter, performs statistical analysis on a coverage.
class PYXStatsFilter : public PYXFilter
{
public:

	//! Constants
	static const std::string kstrTotal;
	static const std::string kstrTotalNotNull;
	static const std::string kstrTotalNotZero;
	static const std::string kstrSums;
	static const std::string kstrMax;
	static const std::string kstrMin;
	static const std::string kstrMean;
	static const std::string kstrRMS;
	static const std::string kstrCompTime;

	//! Unit test method
	static void test();

	//! Default constructor.
	PYXStatsFilter();

	//! Destructor.
	virtual ~PYXStatsFilter();

	//! Set the input coverage.
	void setInput(boost::shared_ptr<PYXCoverage> spInput); 

	//! Start the process to calculate the statistics.
	void calculateStatistics();

protected:

	//! Disable copy constructor
	PYXStatsFilter(const PYXStatsFilter&);

	//! Disable copy assignment
	void operator =(const PYXStatsFilter&);

	//! Set up the member varialbes to be ready to calculate stats
	void InitializeStats();

	//! Add the meta data to report the stats into.
	void AddStatsTableDefinitions(int nChannels = 1);

	//! Put the stats results into the meta data.
	void PopulateStatsTableDefinitions();

private:
	
	//! maximum values.
	std::vector<double>  m_vecMaxValues;
	
	//! minimum values.
	std::vector<double>  m_vecMinValues;

	//! mean values.
	std::vector<double>  m_vecMeanValues;

	//! root mean square values.
	std::vector<double>  m_vecRMSValues;

	//! The sum of each channel.
	std::vector<double> m_vecSums;

	//! the number of non zero cells in each channel.
	std::vector<int> m_vecNonZeroCellCount;

	//! Flag indicating whether the data is dirty or not.
	bool m_bIsDirty;

	//! Total number of cells used in computation.
	long m_nCellCount;

	//! Total number of non-null cells used in computation.
	long m_nNonNullCellCount;

};
#endif	// end if
