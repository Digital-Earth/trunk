#ifndef MASS_CONSERVING_SAMPLER_H
#define MASS_CONSERVING_SAMPLER_H
/******************************************************************************
mass_conserving_sampler.h

begin		: 2006-08-22
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "sampler.h"

// boost includes

// standard includes

// local forward declarations
class PYXDefaultCoverage;

/*!
PYXMassConservingSampler implements mass conserving sampling for non-PYXIS
coverages.  This is uses full for population type coverages where the 
sum of all the data points needs to be the same in the cartesian space
and the Pyxis space.  This sample could also be aptly named the "Population
Sampler" or the "Summing Sampler".
*/
//! Implements mass conserving sampler for non-PYXIS coverages.
class PYXMassConservingSampler : public PYXSampler
{

public:

	//! Constructor
	PYXMassConservingSampler();

	//! Destructor
	virtual ~PYXMassConservingSampler();

	//! Get the coverage value at the specified PYXIS index.
	virtual PYXValue getCoverageValue(
		const PYXIcosIndex& index,
		int nFieldIndex = 0	) const;

	//! set sample resolution
	void setSampleResolution (int sampleResolution) { m_sampleResolution = sampleResolution; };

	//! get the sample resolution
	int getResolution () const;

	/*! 
	Run the calculation sum all the data values into Pyxis space.

	This is an expensive and blocking operation.
	*/
	void runCalculation() const;

protected:

private:

	//! Disable copy constructor
	PYXMassConservingSampler(const PYXMassConservingSampler&);

	//! Disable copy assignment
	void operator=(const PYXMassConservingSampler&);

	//! helper function to open the cache
	void openCache() const;

	//! the resolution that we are going to sample into
	mutable int m_sampleResolution;

	//! true if the source data has been processed into Pyxis space
	mutable bool m_bCalculated;

	//! the cache for the calculated data
	mutable PYXPointer<PYXDefaultCoverage> m_spCache;

};

#endif	// end guard
