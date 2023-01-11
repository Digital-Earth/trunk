/******************************************************************************
bicubic_sampler.cpp

begin		: 2006-03-24
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define MODULE_SAMPLING_SOURCE
#include "stdafx.h"
#include "bicubic_sampler.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_utils.h"
#include "pyxis/utility/value.h"

// standard includes
#include <cassert>

// {175AB4E6-BDE1-4c30-9C80-BC9A31E45058}
PYXCOM_DEFINE_CLSID(BicubicSampler, 
0x175ab4e6, 0xbde1, 0x4c30, 0x9c, 0x80, 0xbc, 0x9a, 0x31, 0xe4, 0x50, 0x58);
PYXCOM_CLASS_INTERFACES(BicubicSampler, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(BicubicSampler, "Bicubic Sampler", "Interpolates the matrix of 16 cells closest to the requested cell", "Sampling",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IXYCoverage::iid, 1, 1, "Input XY Coverage", "The input xy coverage to sample.")
IPROCESS_SPEC_END

#define SAMPLER_MATRIX_WIDTH 5
#define SAMPLER_MATRIX_SIZE SAMPLER_MATRIX_WIDTH * SAMPLER_MATRIX_WIDTH

namespace
{

//! Helper function for the function weight.
double s(double a) 
{
	if (a > 0)
	{
		return a;
	}
	else
	{
		return 0;
	}
}

/*!
Helper function to get the weight to use to factor in a particular point.

In the documentation this function is called R(a).

\param	distance	the distance from the actual point.

\return	  The weight to assign to this point.
*/
double weight(double distance) 
{
    return (1.0 / 6.0) * (pow(s(distance + 2), 3) - 4 * pow(s(distance + 1), 3) + 6 * pow(s(distance), 3) - 4 * pow(s(distance - 1), 3));
}

}

/*!
Get the coverage value at the specified native coordinates.
This call MUST be thread-safe.

\param	native		The native coordinates.
\param  pValue      an pointer to the PYXValue to be filled in
\param	nFieldIndex	The field index.

\return	True if a data value was retrieved, false if there was no value to return.
*/
bool BicubicSampler::getCoverageValue(	const PYXIcosIndex& index,
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
	bool	 hasValues[SAMPLER_MATRIX_SIZE];

	spXYCov->getMatrixOfValues(xy,&(values[0]),SAMPLER_MATRIX_WIDTH,SAMPLER_MATRIX_WIDTH);

	//the default getMatrixOfValues doesn't return if we realy get a value but set the values to null
	for(auto i=0;i<SAMPLER_MATRIX_SIZE;i++)
	{
		hasValues[i] = !values[i].isNull();
	}

	return generateCoverageValue(index,xy,hasValues,values,SAMPLER_MATRIX_WIDTH,SAMPLER_MATRIX_WIDTH,pValue);
}


bool BicubicSampler::generateCoverageValue(const PYXIcosIndex & index,
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

	double dx = raster.x() - nX;
	double dy = raster.y() - nY;
	
	int offsetX = 0;
	int offsetY = 0;

	if (dx > 0.5)
	{
		dx -= 0.5;
		offsetX = 1;
	} 
	else
	{
		dx += 0.5;
	}

	if (dy > 0.5)
	{
		dy -= 0.5;
		offsetY = 1;
	} 
	else
	{
		dy += 0.5;
	}

	int offset = offsetX * SAMPLER_MATRIX_WIDTH + offsetY;

	// if one of the values coming back are NULL return false.
	for (int x=0;x<4;x++)
	{
		for (int y=0;y<4;y++)
		{
			if (values[x*SAMPLER_MATRIX_WIDTH+y+offset].isNull())
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
		double ff = 0.0;
		for (int m = -1; m <= 2; m++)
		{
			for (int n = -1; n <= 2; n++)  
			{
				// add in the weighted factor for each point
				ff += values[(m + 1)*SAMPLER_MATRIX_WIDTH + n + 1 + offset].getDouble(elementIndex) * weight(m - dx) * weight(dy - n);
			}
		}
		pValue->setDouble(elementIndex, ff);
	}

	return true;
}

int BicubicSampler::getSamplingMatrixSize() const
{
	return SAMPLER_MATRIX_WIDTH;
}

IProcess::eInitStatus BicubicSampler::initImpl()
{
	m_spXYCov = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IXYCoverage>();

	return knInitialized;
}