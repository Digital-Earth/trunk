/******************************************************************************
bicubic_sampler_with_null.cpp

begin		: 2015-11-20
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define MODULE_SAMPLING_SOURCE
#include "stdafx.h"
#include "bicubic_sampler_with_null.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_utils.h"
#include "pyxis/utility/value.h"

// standard includes
#include <cassert>

// {80B2DD06-1A5F-40EE-A9C7-FEB3B658A80C}
PYXCOM_DEFINE_CLSID(BicubicSamplerWithNull, 
					0x80b2dd06, 0x1a5f, 0x40ee, 0xa9, 0xc7, 0xfe, 0xb3, 0xb6, 0x58, 0xa8, 0xc);
PYXCOM_CLASS_INTERFACES(BicubicSamplerWithNull, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(BicubicSamplerWithNull, "Bicubic Sampler With Null", "Interpolates the matrix of 16 cells closest to the requested cell supporting null values within the sampling points", "Sampling",
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

\param	index		The pyxis index.
\param  pValue      an pointer to the PYXValue to be filled in
\param	nFieldIndex	The field index.

\return	True if a data value was retrieved, false if there was no value to return.
*/
bool BicubicSamplerWithNull::getCoverageValue(	const PYXIcosIndex& index,
											  PYXValue* pValue,
											  int nFieldIndex	) const
{
	assert(pValue!=nullptr);
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

// Trys to fill the values of the 4x4 window if some of them are null
bool BicubicSamplerWithNull::tryFillNulls(  PYXValue * values, int offsetX, int offsetY	) const
{

	//   |  0|  1|  2|  3|
	//   |  4|  5|  6|  7|
	//   |  8|  9| 10| 11|
	//   | 12| 13| 14| 15|

	auto offset = offsetX * SAMPLER_MATRIX_WIDTH + offsetY;

	const auto index5 = 1*SAMPLER_MATRIX_WIDTH + 1 + offset;
	const auto index6 = index5 + 1;
	const auto index9 = index5 + SAMPLER_MATRIX_WIDTH;
	const auto index10 = index9 + 1;

	const auto originCellIndex = index5 + (1-offsetX) * SAMPLER_MATRIX_WIDTH + ( 1 - offsetY);

	// the nearest value to the requested point is on [5] if it is null return false.
	if(values[originCellIndex].isNull())
	{
		return false;
	}

	//If the 2x2 core has null values replace them with the nearest (origin cell)
	for (int x=1;x<3;x++)
	{
		for (int y=1;y<3;y++)
		{
			if (values[x*SAMPLER_MATRIX_WIDTH+y+offset].isNull())
			{
				values[x*SAMPLER_MATRIX_WIDTH+y+offset] = values[originCellIndex];
			}
		}
	}

	// if corners have null value replace them with the nearest element from the core.
	for (int x=0;x<2;x++)
	{
		for (int y=0;y<2;y++)
		{
			if (values[x*SAMPLER_MATRIX_WIDTH+y+offset].isNull())
			{
				values[x*SAMPLER_MATRIX_WIDTH+y+offset] = values[index5];
			}
		}
		for (int y=2;y<4;y++)
		{
			if (values[x*SAMPLER_MATRIX_WIDTH+y+offset].isNull())
			{
				values[x*SAMPLER_MATRIX_WIDTH+y+offset] = values[index6];
			}
		}
	}
	for (int x=2;x<4;x++)
	{
		for (int y=0;y<2;y++)
		{
			if (values[x*SAMPLER_MATRIX_WIDTH+y+offset].isNull())
			{
				values[x*SAMPLER_MATRIX_WIDTH+y+offset] = values[index9];
			}
		}
		for (int y=2;y<4;y++)
		{
			if (values[x*SAMPLER_MATRIX_WIDTH+y+offset].isNull())
			{
				values[x*SAMPLER_MATRIX_WIDTH+y+offset] = values[index10];
			}
		}
	}
	return true;
}

bool BicubicSamplerWithNull::generateCoverageValue(const PYXIcosIndex & index,
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

	if(!tryFillNulls(values, offsetX, offsetY))
	{
		return false;
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

int BicubicSamplerWithNull::getSamplingMatrixSize() const
{
	return SAMPLER_MATRIX_WIDTH;
}

IProcess::eInitStatus BicubicSamplerWithNull::initImpl()
{
	m_spXYCov = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IXYCoverage>();

	return knInitialized;
}