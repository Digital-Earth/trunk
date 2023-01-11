/******************************************************************************
sampler.cpp

begin		: 2006-03-13
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "sampler.h"

// local includes
#include "pyxis/sampling/xy_bounds_geometry.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/derm/reference_sphere.h"
#include "pyxis/derm/snyder_projection.h"

// boost includes

// standard includes
#include <cassert>

//! The name of the class
const std::string PYXSampler::kstrScope = "PYXSampler";

/*
Set the input coverage.

\param	spInput	The input coverage.
*/
void PYXSampler::setInput(PYXPointer<PYXXYCoverage> spInput)
{
	assert(spInput && "Invalid argument.");

	m_spInput = spInput;

	if (m_spInput->hasSpatialReferenceSystem())
	{
		createGeometryAndDefinitions();
	}
}

/*!
Call this method after the data source is opened to determine if the data
source contains a spatial reference. If not, a spatial reference must be
supplied by calling setSpatialReference() before getFeatureIterator() is
called.

\return	true if the data source has a spatial reference, otherwise false
*/
//bool PYXSampler::hasSpatialReferenceSystem() const
//{
//	assert(m_spInput && "No input coverage set.");
//
//	return m_spInput->hasSpatialReferenceSystem();
//}

///*!
//Specify the spatial reference for the data source. Call this method to set
//the spatial reference if after the data source is opened
//hasSpatialReference() returns false.
//
//\param	spSRS	Shared pointer to the spatial reference system.
//*/
//void PYXSampler::setSpatialReferenceSystem(PYXPointer<PYXSpatialReferenceSystem> spSRS)
//{
//	assert(m_spInput && "No input coverage set.");
//
//	m_spInput->setSpatialReferenceSystem(spSRS);
//
//	createGeometryAndDefinitions();
//}

/*!
Create the geometry and field definitions for the sampler.
*/
void PYXSampler::createGeometryAndDefinitions()
{
	double fPrecision = m_spInput->getSpatialPrecision();
	if (fPrecision >= 0)
	{
		// convert spatial precision in metres to a PYXIS resolution
		double fArcRadians = fPrecision / ReferenceSphere::kfRadius;
		int nResolution =
			SnyderProjection::getInstance()->precisionToResolution(fArcRadians);

		// create the geometry at that resolution
		createGeometry(nResolution);
	}

	createDefinitions();
}

/*!
Set the resolution. Recreate the geometry if sufficient information is
available.

\param	nResolution	The resolution.
*/
void PYXSampler::setResolution(int nResolution)
{
	assert((nResolution > 0) && "Invalid argument.");
	assert((m_spInput.get() != 0) && "No input coverage set.");

	// create the geometry at the specified resolution
	createGeometry(nResolution);

	createDefinitions();
}

/*!
Attempt to create the PYXIS geometry for the input coverage. Necessary
conditions for creating the PYXIS geometry are:

/verbatim
1. The input data source exists.
2. The input data source has a spatial reference system.
3. The resolution is valid.
/endverbatim

\param	nResolution	The resolution of the geometry.
*/
void PYXSampler::createGeometry(int nResolution)
{
	if (	m_spInput &&
			m_spInput->hasSpatialReferenceSystem() &&
			nResolution >= 0	)
	{
		//// convert the spatial precision in metres to radians
		//m_spGeometry.reset(new PYXXYBoundsGeometry(
		//	m_spInput->getBounds(),
		//	m_spInput->getCoordConverter(),
		//	nResolution	));
		m_spGeometry = PYXXYBoundsGeometry::create(m_spInput->getBounds(),
					   m_spInput->getCoordConverter(),
					   nResolution);
	}
}

/*!
Attempt to copy the data source and coverage definitions from the input
coverage. Necessary conditions for copying the definition are:

/verbatim
1. The input data source exists.
2. The input data set's definition has been loaded.
/endverbatim
*/
void PYXSampler::createDefinitions()
{
	if (	m_spInput &&
			(m_spInput->getDefinition()->getFieldCount() > 0) &&
			(m_spInput->getCoverageDefinition()->getFieldCount() > 0)	)
	{
		// copy definition from input
		PYXPointer<PYXTableDefinition> spDefn = m_spInput->getDefinition()->clone();
		setDefinition(spDefn);

		// copy values from input
		int nFieldCount = spDefn->getFieldCount();
		for (int nFieldIndex = 0; nFieldIndex < nFieldCount; ++nFieldIndex)
		{
			setFieldValue(m_spInput->getFieldValue(nFieldIndex), nFieldIndex);
		}

		// copy coverage definition from input
		setCoverageDefinition(m_spInput->getCoverageDefinition()->clone());
	}
}

/*!
For data sources that can have multiple resolutions of data (WMS for now)
give the minumum available resolution we can return.

\return minimum resolution this data source can return. -1 if not implemented.
*/
int PYXSampler::getMinimumAvailableResolution()
{
	return m_spInput->getMinimumAvailableResolution();
}

/*!
For data sources that can have multiple resolutions of data (WMS for now)
give the maximum available resolution we can return.

\return maximum resolution this data source can return. -1 if not implemented.
*/
int PYXSampler::getMaximumAvailableResolution()
{
	return m_spInput->getMaximumAvailableResolution();
}

/*!
For data sources that can have multiple resolutions of data (WMS for now)
set the resolution of data being requested from the data source.
*/
void PYXSampler::setRequestedDataResolution(int nRes)
{
	m_spInput->setRequestedDataResolution(nRes); 
}