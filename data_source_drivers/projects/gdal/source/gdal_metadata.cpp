/******************************************************************************
gdal_metadata.cpp

begin		: 2004-10-15
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_GDAL_SOURCE
#include "gdal_metadata.h"

// local includes
#include "exceptions.h"

// GDAL includes
#include "cpl_conv.h"
#include "gdal_alg.h"
#include "gdal_priv.h"

// standard includes
#include <cassert>

/*!
Constructor.
*/
GDALMetaData::GDALMetaData()
{
}

/*!
Destructor. Delete the GeospatialCoordTransform
*/
GDALMetaData::~GDALMetaData()
{

}

/*!
Initializes the GDALMetaData object.  

\param	dataset The GDALDataset object.
\param	nOverview Number of the overview to use, or -1 for default raster size.
*/
void GDALMetaData::initialize(GDALDataset& dataset, int nOverview)
{
	m_bNativeFlipped = false;

	int nFullRasterXSize = dataset.GetRasterXSize();
	int nFullRasterYSize = dataset.GetRasterYSize();

	if (nOverview != -1)
	{
		// Overviews are the same for all bands.  Just as band 1 for its overview size.
		m_nRasterXSize = dataset.GetRasterBand(1)->GetOverview(nOverview)->GetXSize();
		m_nRasterYSize = dataset.GetRasterBand(1)->GetOverview(nOverview)->GetYSize();
	}
	else
	{
		m_nRasterXSize = nFullRasterXSize;
		m_nRasterYSize = nFullRasterYSize;
	}

	if (dataset.GetGCPCount() != 0 )
	{
		const GDAL_GCP* pGCP = dataset.GetGCPs();
		int nCount = dataset.GetGCPCount();
		int nResult = GDALGCPsToGeoTransform(nCount, pGCP, m_pfGeoTransform, true );
		if (nResult == 1)
		{
			std::string strTemp = dataset.GetGCPProjection();
			dataset.SetGeoTransform(m_pfGeoTransform);
			dataset.SetProjection(strTemp.c_str());
		}
		else
		{
 			PYXTHROW(GDALProcessException, "Invalid GeoTransform for defined GCP's.");
		}
	}
	else
	{
		/*
		Get the GeoTransform object.  This object is used to convert the 
		indexes into the file into the native geospatial coordinates of the 
		dataset.
		*/
		int nError = dataset.GetGeoTransform(m_pfGeoTransform);
		if (nError != CE_None)
		{
			PYXTHROW(MissingWorldFileException, "The data set is missing a geotransform. Possibly due to a missing world file.");
		}
	}

	if (m_pfGeoTransform[knGTSkewX] != 0.0 || m_pfGeoTransform[knGTSkewY] != 0.0)
	{
		if (m_pfGeoTransform[knGTPixelWidth] != 0.0 || m_pfGeoTransform[knGTPixelHeight] != 0.0)
		{
			PYXTHROW(GDALProcessException, "Invalid GeoTransform: we don't support skewed data sets.");
		}	
		else
		{
			m_bNativeFlipped = true;
		}
	}
	

	// Scale Pixel width (index 1) and height (index 5) in geotransform to size of any overview we may be using.
	m_pfGeoTransform[knGTPixelWidth] *= (static_cast<double>(nFullRasterXSize) / static_cast<double>(m_nRasterXSize));
	m_pfGeoTransform[knGTSkewY] *= (static_cast<double>(nFullRasterXSize) / static_cast<double>(m_nRasterXSize));
	
	m_pfGeoTransform[knGTPixelHeight] *= (static_cast<double>(nFullRasterYSize) / static_cast<double>(m_nRasterYSize));
	m_pfGeoTransform[knGTSkewX] *= (static_cast<double>(nFullRasterYSize) / static_cast<double>(m_nRasterYSize));

	// Load the limits of the file in native coordinates.
	rasterToNative(PYXCoord2DInt(0,0), &m_northWest);
	if (m_bNativeFlipped)
	{
		rasterToNative(PYXCoord2DInt(0,m_nRasterYSize), &m_northEast);
		rasterToNative(PYXCoord2DInt(m_nRasterXSize,0), &m_southWest);	
	}
	else
	{
		rasterToNative(PYXCoord2DInt(m_nRasterXSize,0), &m_northEast);
		rasterToNative(PYXCoord2DInt(0,m_nRasterYSize), &m_southWest);
	}
	rasterToNative(PYXCoord2DInt(m_nRasterXSize, m_nRasterYSize), &m_southEast);
}

/*!
Convert from raster coordinates to native coordinates.

\param	raster	The raster coordinates
\param	pNative	The native coordinates (out)
*/
void GDALMetaData::rasterToNative(	const PYXCoord2DInt& raster,
									PYXCoord2DDouble* pNative	) const
{
	assert (0 != pNative && "Null value passed as parameter.");

	auto transform = getGeoTransform();

	// calculate the X position
	double fX = transform[knGTUpperLeftX] + (transform[knGTPixelWidth] * raster.x()) + (transform[knGTSkewX] * raster.y());

	// calculate the Y position
	double fY = transform[knGTUpperLeftY] + (transform[knGTSkewY] * raster.x()) + (transform[knGTPixelHeight] * raster.y());

	pNative->setX(fX);
	pNative->setY(fY);
}

//! return pixel width
double GDALMetaData::getPixelWidth() const
{
	return m_bNativeFlipped ? getGeoTransform()[knGTSkewY] : getGeoTransform()[knGTPixelWidth];
}

//! return pixel height
double GDALMetaData::getPixelHeight() const
{
	return m_bNativeFlipped ? getGeoTransform()[knGTSkewX] : getGeoTransform()[knGTPixelHeight];
}


/*!
Converts from native coordinates to raster coordinates.

\param	native	The native coordinates
\param	pRaster The raster coordinates (out)
*/
void GDALMetaData::nativeToRaster(	const PYXCoord2DDouble& native,
									PYXCoord2DInt* pRaster) const
{
	assert(pRaster != 0 && "Null value passed as parameter.");
	
	PYXCoord2DDouble subPixel;
	nativeToRasterSubPixel(native,&subPixel);
	pRaster->setX(static_cast<int>(floor(subPixel.x())));
	pRaster->setY(static_cast<int>(floor(subPixel.y())));
}

/*!
Converts from native coordinates to raster coordinates.

\param	native	The native coordinates
\param	pRaster The raster subpixer coordinates (out)
*/
void GDALMetaData::nativeToRasterSubPixel(	const PYXCoord2DDouble& native,
											PYXCoord2DDouble* pRaster) const
{
	assert(pRaster != 0 && "Null value passed as parameter.");
	
	if (m_bNativeFlipped)
	{
		pRaster->setX(
			(native.x() - getGeoTransform()[knGTUpperLeftX]) /
			getGeoTransform()[knGTSkewY]);	

		pRaster->setY(
			(native.y() - getGeoTransform()[knGTUpperLeftY]) /
			getGeoTransform()[knGTSkewX]);
	} 
	else
	{
		pRaster->setX(
			(native.x() - getGeoTransform()[knGTUpperLeftX]) /
			getGeoTransform()[knGTPixelWidth]);

		pRaster->setY(
			(native.y() - getGeoTransform()[knGTUpperLeftY]) /
			getGeoTransform()[knGTPixelHeight]);
	}
}
