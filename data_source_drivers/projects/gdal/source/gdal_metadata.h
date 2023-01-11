#ifndef GDAL_METADATA_H
#define GDAL_METADATA_H
/******************************************************************************
gdal_metadata.h

begin		: 2004-10-15
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_gdal.h"

// pyxlib includes
#include "pyxis/utility/coord_2d.h"
#include "pyxis/utility/coord_lat_lon.h"

// local forward declarations
class GDALDataset;

//! Stores the Geospatial Metadata for the File.
/*!
Stores the Geospatial Metadata for the File.  It determines such things as the 
height and width and the limits of the file in its native coordinates.
*/
class MODULE_GDAL_DECL GDALMetaData 
{

public:
	
	//! Test method
	static void test();

	//! the tags for the GeoTransform structure.
	enum eGeoTransform
	{
		knGTUpperLeftX = 0,
		knGTPixelWidth,
		knGTSkewX,
		knGTUpperLeftY,
		knGTSkewY,
		knGTPixelHeight
	};

	//! Constructor.
	GDALMetaData();

	//! Destructor.
	virtual ~GDALMetaData();
	
	//!	Get the south west limit of the data source.
	virtual const PYXCoord2DDouble& getSouthWest() const {return m_southWest;}

	//!	Get the north west limit of the data source.
	virtual const PYXCoord2DDouble& getNorthWest() const {return m_northWest;}

	//!	Get the north east limit of the data source.
	virtual const PYXCoord2DDouble& getNorthEast() const {return m_northEast;}

	//!	Get the south east limit of the data source.
	virtual const PYXCoord2DDouble& getSouthEast() const {return m_southEast;}

	//! Initialize the metadata object.
	virtual void initialize(GDALDataset& dataset, int nOverview);
	
	//! Transforms the raster coordinates to native coordinates.
	void rasterToNative(	const PYXCoord2DInt& raster,
							PYXCoord2DDouble* pNative) const;

	//! Transforms the native coordinates to raster coordinates
	void nativeToRaster(	const PYXCoord2DDouble& native,
							PYXCoord2DInt* pRaster) const;

	//! Transforms the native coordinates to raster coordinates
	void nativeToRasterSubPixel(	const PYXCoord2DDouble& native,
									PYXCoord2DDouble* pRaster) const;

	//! return pixel width
	double getPixelWidth() const;

	//! return pixel height
	double getPixelHeight() const;

	//! Get the coordinate transform matrix.
	const double* getGeoTransform() const {return m_pfGeoTransform;}

protected:

private:

	//! Disable copy constructor.
	GDALMetaData(const GDALMetaData&);

	//! Disable copy assignment.
	void operator=(const GDALMetaData&);
	
	//! Stores a reference to a dataset.
	double m_pfGeoTransform[6];
	
	//! Origin Position.
	PYXCoord2DDouble m_northWest;

	//! northEast.
	PYXCoord2DDouble m_northEast;

	//! southWest.
	PYXCoord2DDouble m_southWest;

	//! The south east coordinate.
	PYXCoord2DDouble m_southEast;

	//!	The number of points in a line
	int m_nRasterXSize;

	//!	The number of lines
	int m_nRasterYSize;

	//! When geoTransform flip the order of X,Y
	bool m_bNativeFlipped;
};

#endif	// GDAL_METADATA_H
