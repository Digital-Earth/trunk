/******************************************************************************
pyx_shared_gdal_data_set.h

begin      : 2015-12-01
copyright  : (c) 2015 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#pragma once

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/utility/object.h"

// standard includes
#include <map>

// local forward declarations
class GDALDataset;

/*!
Wrapper class to ensure a given data source corresponding to a file, folder or url is only opened
once and shared across all instances that require it. Multiple bands, for example, should be
accessed through a common GDALDataset. Also ensures that the dataset is closed properly through
GDALClose() when no longer needed. It is not recommended to delete pointers to GDALDatasets.
*/
class PYXSharedGDALDataSet : public PYXObject
{
public:
	//! Mutex for GDAL and OGR thread safety
	static AppExecutionScope s_gdalScope;

	//! Get or create a raster PYXSharedGDALDataSet for a given uri.
	static PYXPointer<PYXSharedGDALDataSet> createRaster(const std::string & uri)
	{
		return create(uri, GDAL_OF_RASTER);
	}

	//! Get or create a vector PYXSharedGDALDataSet for a given uri.
	static PYXPointer<PYXSharedGDALDataSet> createVector(const std::string & uri)
	{
		return create(uri, GDAL_OF_VECTOR);
	}

	//! Destructor
	virtual ~PYXSharedGDALDataSet() {}
	
	//! Deletes the GDALDataset when no longer used.
	virtual long release() const;	

	//! Get a pointer to the GDALDataset
	GDALDataset* get() const { return m_pGDALDataset; }

	//! Check the GDAL driver to see if this is an elevation file.
	bool isElevation() const;

	//! Check the GDAL driver to see if this is an AutoCAD file.
	bool isAutoCAD() const;

	//! Check the GDAL driver to see if this is a NetCDF file.
	bool isNetCDF() const;

	//! Check the GDAL driver to see if this is an GRIB file.
	bool isGRIB() const;

	//! Check the GDAL driver to see if this is a WFS data source.
	bool isWFS() const;

#if UNUSED
	//! Convenience method for debugging
	static void dumpDataSet(GDALDataset* pDataSet);
#endif

private:

	//! Get or create a PYXSharedGDALDataSet for a given uri and open flags.
	static PYXPointer<PYXSharedGDALDataSet> create(const std::string & uri, unsigned int nOpenFlags);
	
	//! Disable default constructor
	PYXSharedGDALDataSet();

	/*!
	Constructor (hidden).

	\param uri	The uri corresponding to the GDALDataset to be opened.
	\param nOpenFlags	The GDAL open flags (GDAL_OF_RASTER or GDAL_OF_VECTOR)
	\param pGDALDataset
	*/
	PYXSharedGDALDataSet(const std::string& uri, unsigned int nOpenFlags, GDALDataset* pGDALDataset) :
		m_uri(uri),
		m_nOpenFlags(nOpenFlags),
		m_pGDALDataset(pGDALDataset) {}


	//! Pointer to the GDALDataset
	mutable GDALDataset* m_pGDALDataset;
	
	//! The uri corresponding to the file, folder or url for this data source
	std::string m_uri;

	//! The flags used to open the data set (GDAL_OF_RASTER or GDAL_OF_VECTOR)
	unsigned int m_nOpenFlags;
	
	//! Stores wrappers for all the GDALDatasets that are currently open
	static std::map<std::pair<std::string, unsigned int>, PYXSharedGDALDataSet*> s_mapGDALDataSet;
};
