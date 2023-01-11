/******************************************************************************
bilinear_sampler.cpp

begin		: 2006-03-24
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define MODULE_SAMPLING_SOURCE
#include "stdafx.h"
#include "bilinear_sampler.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_utils.h"
#include "pyxis/utility/value.h"

// standard includes
#include <cassert>

#define SAMPLER_MATRIX_WIDTH 3
#define SAMPLER_MATRIX_SIZE SAMPLER_MATRIX_WIDTH * SAMPLER_MATRIX_WIDTH

// {46C8C829-9CFE-469e-93DF-F9688B83BB09}
PYXCOM_DEFINE_CLSID(BilinearSampler, 
0x46c8c829, 0x9cfe, 0x469e, 0x93, 0xdf, 0xf9, 0x68, 0x8b, 0x83, 0xbb, 0x9);
PYXCOM_CLASS_INTERFACES(BilinearSampler, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(BilinearSampler, "Bilinear Sampler", "Interpolates the matrix of 4 cells closest to the requested cell", "Sampling",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IXYCoverage::iid, 1, 1, "Input XY Coverage", "The input xy coverage to sample.")
IPROCESS_SPEC_END


/*!
Get the coverage value at the specified native coordinates.
This call MUST be thread-safe.

\param	native		The native coordinates.
\param  pValue      an pointer to the PYXValue to be filled in
\param	nFieldIndex	The field index.

\return	True if a data value was retrieved, false if there was no value to return.
*/
bool BilinearSampler::getCoverageValue(	const PYXIcosIndex& index,
												PYXValue* pValue,
												int nFieldIndex	) const
{
	boost::intrusive_ptr<IXYCoverage> spXYCov = getXYCoverage();
	assert(spXYCov && "input coverage must be set");

	if (getXYRegion()->intersects(index) == PYXRegion::knNone)
	{
		return false;
	}

	// convert the PYXIS index to native coordinate
	PYXCoord2DDouble xy;
	if (!spXYCov->getCoordConverter()->tryPyxisToNative(index, &xy))
	{
		return false;
	}

	PYXValue values[SAMPLER_MATRIX_SIZE];
	bool hasValues[SAMPLER_MATRIX_SIZE];

	spXYCov->getMatrixOfValues(xy, &(values[0]), SAMPLER_MATRIX_WIDTH, SAMPLER_MATRIX_WIDTH);

	for(auto i=0;i<SAMPLER_MATRIX_SIZE;i++)
	{
		hasValues[i] = !values[i].isNull();
	}

	return generateCoverageValue(index, xy, hasValues, &values[0], SAMPLER_MATRIX_SIZE, SAMPLER_MATRIX_SIZE, pValue);
}


bool BilinearSampler::generateCoverageValue(const PYXIcosIndex & index,
											const PYXCoord2DDouble & xy,
											bool * hasValues,
											PYXValue * values,
											int width,int height,
											PYXValue * pValue) const
{
	boost::intrusive_ptr<IXYCoverage> spXYCov = getXYCoverage();
	assert(spXYCov && "input coverage must be set");
	
	PYXCoord2DDouble raster = spXYCov->nativeToRasterSubPixel(xy);

	int nX = static_cast<int>(floor(raster.x()));
	int nY = static_cast<int>(floor(raster.y()));

	double distanceX1 = raster.x() - nX;
	double distanceY1 = raster.y() - nY;
	

	int dx = 0;
	int dy = 0;

	if (distanceX1 > 0.5)
	{
		distanceX1 -= 0.5;
		dx = 1;
	} 
	else
	{
		distanceX1 += 0.5;
	}

	if (distanceY1 > 0.5)
	{
		distanceY1 -= 0.5;
		dy = 1;
	} 
	else
	{
		distanceY1 += 0.5;
	}

	int offset = dx * SAMPLER_MATRIX_WIDTH + dy;
	double distanceX2 = 1.0 - distanceX1;		
	double distanceY2 = 1.0 - distanceY1;

	// if one of the values coming back are NULL return false.
	for (int x=0;x<2;x++)
	{
		for (int y=0;y<2;y++)
		{
			if (!hasValues[x*SAMPLER_MATRIX_WIDTH+y+offset] || values[x*SAMPLER_MATRIX_WIDTH+y+offset].isNull())
			{
				//we can make a bilieanr smapling if we don't have the data for it
				return false;
			}
		}
	}

	// assume that all values coming from the same field in the same data
	// source will have the same number of data elements.
	int numElements = values[offset].getArraySize();

	for (int elementIndex = 0; elementIndex < numElements; ++elementIndex)
	{
		double fAtXY1 = distanceX2 * values[0*SAMPLER_MATRIX_WIDTH+0+offset].getDouble(elementIndex) +
						distanceX1 * values[1*SAMPLER_MATRIX_WIDTH+0+offset].getDouble(elementIndex);
		double fAtXY2 = distanceX2 * values[0*SAMPLER_MATRIX_WIDTH+1+offset].getDouble(elementIndex) +
						distanceX1 * values[1*SAMPLER_MATRIX_WIDTH+1+offset].getDouble(elementIndex);
		double fAtXY = distanceY2 * fAtXY1 + distanceY1 * fAtXY2;
		pValue->setDouble(elementIndex, fAtXY);
	}

	return true;
}

int BilinearSampler::getSamplingMatrixSize() const
{
	return SAMPLER_MATRIX_WIDTH;
}

IProcess::eInitStatus BilinearSampler::initImpl()
{
	m_spXYCov = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IXYCoverage>();

	return knInitialized;
}