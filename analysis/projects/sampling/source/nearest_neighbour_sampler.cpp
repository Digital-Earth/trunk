/******************************************************************************
nearest_neighbour_sampler.cpp

begin		: 2006-03-21
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define MODULE_SAMPLING_SOURCE
#include "stdafx.h"
#include "nearest_neighbour_sampler.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/geometry/vector_geometry.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/utility/joblist.h"
#include "pyxis/utility/value.h"

// standard includes
#include <cassert>

// {D612004E-FC51-449a-B0D6-1860E59F3B0D}
PYXCOM_DEFINE_CLSID(NearestNeighbourSampler, 
0xd612004e, 0xfc51, 0x449a, 0xb0, 0xd6, 0x18, 0x60, 0xe5, 0x9f, 0x3b, 0xd);
PYXCOM_CLASS_INTERFACES(NearestNeighbourSampler, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(NearestNeighbourSampler, "Nearest Neighbour Sampler", "Acquires the input value that is closest to the centre of cell being requested.", "Sampling",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IXYCoverage::iid, 1, 1, "Input Coverage", "The XY coverage that is being sampled.")
IPROCESS_SPEC_END

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

/*!
Get the coverage value at the specified native coordinates.
This call MUST be thread-safe.

\param	native		The native coordinates.
\param  pValue      an pointer to the PYXValue to be filled in
\param	nFieldIndex	The field index.

\return	True if a data value was retrieved, false if there was no value to return.
*/
bool NearestNeighbourSampler::getCoverageValue(	const PYXIcosIndex& index,
												PYXValue* pValue,
												int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	if (getXYRegion()->intersects(index) == PYXRegion::knNone)
	{
		return false;
	}

	// convert the PYXIS index to native coordinate
	PYXCoord2DDouble xy;
	if (!getCoordConverter()->tryPyxisToNative(index, &xy))
	{
		return false;
	}	

	// get the value
	return getXYCoverage()->getCoverageValue(xy, pValue);
}

bool NearestNeighbourSampler::generateCoverageValue(const PYXIcosIndex & index,
													const PYXCoord2DDouble & xy,
													bool * hasValues,
													PYXValue * values,
													int width,int height,
													PYXValue * pValue) const
{
	if (*hasValues)
	{
		*pValue = *values;
		return true;
	}
	return false;
}

int NearestNeighbourSampler::getSamplingMatrixSize() const
{
	return 1;
}

IProcess::eInitStatus NearestNeighbourSampler::initImpl()
{
	m_spXYCov = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IXYCoverage>();

	return knInitialized;
}

//IProcess::eInitStatus STDMETHODCALLTYPE initProc(bool bRecursive = false)
