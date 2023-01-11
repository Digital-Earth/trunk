#ifndef HISTOGRAM_FILTER_H
#define HISTOGRAM_FILTER_H
/******************************************************************************
histogram_filter.h

begin		: 2006-05-09
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "binner.h"
#include "filter_impl.h"

// standard includes

// forward declarations

/*!
PYXHistogramFilter "bins" PYXValues, at specified indices appropirately into bins, 
for generation of histograms. 
*/
//! PYXHistogramFilter collects and stores data to generate Histogram. 
class PYXHistogramFilter : public FilterImpl
{
public:

	//! Constants 
	static const std::string kstrScope;

	//! Unit test method
	static void test();

	//! Default constructor.
	PYXHistogramFilter();

	//! Destructor.
	virtual ~PYXHistogramFilter();

	//! Returns the Binning object used. 
	boost::shared_ptr<PYXBinner> getBinner() {return m_spBinner;}
	
	//! Sets the binning object to you use. 
	void setBinner(boost::shared_ptr<PYXBinner> spNewBinner)
	{
		m_spBinner = spNewBinner; 
		m_vecBins.resize(m_spBinner->getBinCount());
	}
	
	//! Sets a specific geometry to generate a histogram of. 
	void setGeometry(boost::shared_ptr<PYXGeometry> spGeometry);

	//! Get the geometry of the coverage.
	boost::shared_ptr<const PYXGeometry> getGeometry() const;

	//! Sets the resolution to run the histogram on.
	void setHistogramResolution(int nResoultion);

	//! Sets the Input coverage. 
	void setInput(boost::shared_ptr<PYXCoverage> spInput);

	//! Start the binning calculation deciding which bin to place data in. 
	void startBinning();

	//! Get the vector containing the binned data.
	std::vector<int> getHistogramData() const {return m_vecBins;}

	//! Get flag to determine if data has been recalucated. 
	bool getDataRecalculated() {return m_bDataRecalculated;} 

	//! Static pointer to histogram. Used to provide reference self. 
	static boost::shared_ptr<PYXHistogramFilter> spHistogramFilter;
 
    //! Enable or disable histogram filter. 
	static void enable(bool bEnabled) {m_bEnabled = bEnabled;}

	//! Get whether filter is enabled or not. 
	static bool getEnabled() {return m_bEnabled;}

	//! Get the currently set resolution for the histogram.
	int getHistogramResolution(){return m_nResolution;}
	
protected:

	//! Disable copy constructor
	PYXHistogramFilter(const PYXHistogramFilter&);

	//! Disable copy assignment
	void operator =(const PYXHistogramFilter&);

private:

	//! Enable/Disable filter flag. 
	static bool m_bEnabled;

	//! Reset vector containing bins. 
	void reset(); 

	//! Default Binner object. Provides access to binning functions. 
	boost::shared_ptr<PYXBinner> m_spBinner; 

	//! Geometry to generate histogram on. 
	boost::shared_ptr<PYXGeometry> m_spGeometry; 

	//! Determine if data needs to be relcalculated flag. 
	bool m_bRecalculateDataFlag; 

	//! Determine if data has been recalculated. 
	bool m_bDataRecalculated;  

	//! Stores bins. 
	mutable std::vector<int> m_vecBins; 

	//! The resolution to run the histogram at.
	int m_nResolution;
};

#endif	// end if
