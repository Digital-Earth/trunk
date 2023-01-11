#ifndef SAMPLER_H
#define SAMPLER_H
/******************************************************************************
sampler.h

begin		: 2006-03-13
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxis/data/coverage.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/sampling/xy_coverage.h"

// boost includes
#include <boost/shared_ptr.hpp>

// standard includes

// local forward declarations
//class PYXGeometry;
//class PYXXYCoverage;

/*!
PYXSampler is the abstract base for classes that integrate two dimensional
(e.g. xy, lat/lon) data into PYXIS. Derived classes typically override the
getCoverageValue() method.
*/
//! Abstract base for classes that sample xy data into PYXIS.
class PYXSampler : public PYXCoverage
{
public:

	// Constants
	static const std::string kstrScope;

	//! Destructor
	virtual ~PYXSampler() {;}

	////! Check if the input coverage has a spatial reference system.
	//virtual bool hasSpatialReferenceSystem() const;

	////! Set the spatial reference system for the input coverage.
	//virtual void setSpatialReferenceSystem(
	//	PYXPointer<PYXSpatialReferenceSystem> spSRS	);

	//! Return a description of the observing class.
	virtual std::string getObserverDescription() const 
	{
		return kstrScope + " " + getName();
	}

	//! Return the name of the notification class.
	virtual std::string getNotifierDescription() const
	{
		return getObserverDescription();
	}

	//! Set the PYXIS resolution for the input coverage.
	virtual void setResolution(int nResolution);

	//! Set the input coverage
	void setInput(PYXPointer<PYXXYCoverage> spInput);

	//! Get the input coverage.
	PYXPointer<PYXXYCoverage> getInput() { return m_spInput; };

	//! Get the input coverage
	inline PYXPointer<PYXXYCoverage> getInput() const
	{
		return m_spInput;
	}

	//! Get the geometry of the coverage. (Ownership retained.)
	virtual PYXPointer<const PYXGeometry> getGeometry() const
	{
		return m_spGeometry;
	}

	//! Get the coverage value at the specified PYXIS index.
	virtual PYXValue getCoverageValue(
		const PYXIcosIndex& index,
		int nFieldIndex = 0) const = 0;

	//!	Give the minumum available resolution we can return.
	virtual int getMinimumAvailableResolution();

	//! Give the maximum available resolution we can return.
	virtual int getMaximumAvailableResolution();

	//! Set the resolution of data to be returned for a multi-resolution data source.
	virtual void setRequestedDataResolution(int nRes);

protected:

	//! Constructor
	PYXSampler() {;}

private:

	//! Create the geometry and field definitions for the sampler.
	void createGeometryAndDefinitions();

	//! Create the PYXIS geometry for the coverage.
	void createGeometry(int nResolution);

	//! Create the data source and coverage definitions.
	void createDefinitions();

	//! The input coverage
	PYXPointer<PYXXYCoverage> m_spInput;

	//! The geometry of the input coverage
	PYXPointer<PYXGeometry> m_spGeometry;
};

#endif	// end if
