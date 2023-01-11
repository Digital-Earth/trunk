/******************************************************************************
pyx_shared_gdal_data_set.cpp

begin      : 2015-12-01
copyright  : (c) 2015 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_GDAL_SOURCE
#include "pyx_shared_gdal_data_set.h"

// local includes

// pyxlib includes
#include "pyxis/procs/user_credentials.h"

// GDAL includes
#include "gdal_priv.h"

// boost includes
#include <boost/algorithm/string.hpp>

//! Mutex for GDAL and OGR thread safety
AppExecutionScope PYXSharedGDALDataSet::s_gdalScope("gdal");

//! PYXSharedGDALDataSets mapped by their uri's and open type
std::map<std::pair<std::string, unsigned int>, PYXSharedGDALDataSet*> PYXSharedGDALDataSet::s_mapGDALDataSet;

/*!
Get or create a GDALDataset in an PYXSharedGDALDataSet. If an existing PYXSharedGDALDataSet for the
GDALDataset exists, it is returned, otherwise a new wrapper with a newly opened GDALDataset is
returned. The PYXSharedGDALDataSet contains a null GDALDataset if the GDALDataset could not be
opened.

\param uri			The uri corresponding to the file, folder or url of the data source.
\param nOpenFlags	The GDAL open flags (GDAL_OF_RASTER or GDAL_OF_VECTOR)

\return The PYXSharedGDALDataSet.
*/
PYXPointer<PYXSharedGDALDataSet> PYXSharedGDALDataSet::create(const std::string & uri, unsigned int nOpenFlags)
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	PYXPointer<PYXSharedGDALDataSet> pObject;
	if (s_mapGDALDataSet.find(std::make_pair(uri, nOpenFlags)) != s_mapGDALDataSet.end())
	{
		// shared data set was found - use it
		pObject.reset(s_mapGDALDataSet[std::make_pair(uri, nOpenFlags)]);
	}
	else
	{
		// create new shared data set
		boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

		// http://www.gdal.org/ogr_apitut.html
		CPLErrorReset();
		auto pGDALDataset = (GDALDataset*) GDALOpenEx(uri.c_str(), nOpenFlags, NULL, NULL, NULL);
		if (nullptr == pGDALDataset)
		{
			std::string errorMsg = CPLGetLastErrorMsg();
			TRACE_INFO("Failed to open GDAL datasource " << uri << " with the following error: " << errorMsg << "[TYPE:" << CPLGetLastErrorType() << ", NUMBER:" << CPLGetLastErrorNo() << "]");
			CPLErrorReset();

			if (boost::algorithm::find_first(errorMsg ,"HTTP error code : 401"))
			{
				PYXTHROW(MissingUserCredentialsException, errorMsg);
			}
			else
			{
				PYXTHROW(GDALProcessException, errorMsg);
			}
		}

		// store the dataset for re-use
		pObject = PYXNEW(PYXSharedGDALDataSet, uri, nOpenFlags, pGDALDataset);
		s_mapGDALDataSet[std::make_pair(uri, nOpenFlags)] = pObject.get();
	}

	return pObject;
}

/*!
Deletes the GDALDataset once it is no longer in use.

\return The new reference count.
*/
long PYXSharedGDALDataSet::release() const
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	// if object is going to be destroyed...
	long nNewRef = PYXObject::release();
	if (0 == nNewRef)
	{
		// remove from map
		s_mapGDALDataSet.erase(std::make_pair(m_uri, m_nOpenFlags));

		// close the dataset
		if (m_pGDALDataset != 0)
		{
			// http://www.gdal.org/ogr_apitut.html
			GDALClose(m_pGDALDataset);
			m_pGDALDataset = NULL;
		}
	}

	return nNewRef;
}

/*!
Check the GDAL driver to see if this is an elevation file.

\return	true if an elevation file, otherwise false.
*/
bool PYXSharedGDALDataSet::isElevation() const
{
	if (!m_pGDALDataset)
	{
		return false;
	}

	// .dem files can be loaded by the GeoTIFF driver
	if (boost::iequals(m_pGDALDataset->GetDriverName(), "gtiff"))
	{
		// check if the filename ends with dem
		std::string strPath(m_pGDALDataset->GetDescription());
		auto path = FileUtils::stringToPath(strPath);
		if (boost::iequals(FileUtils::getExtensionNoDot(path), "dem"))
		{
			return true;
		}
	}

	return	boost::iequals(m_pGDALDataset->GetDriverName(), "dted") ||
			boost::iequals(m_pGDALDataset->GetDriverName(), "sdts") ||
			boost::iequals(m_pGDALDataset->GetDriverName(), "usgsdem");
}

/*!
Check the GDAL driver to see if this is an AutoCAD file.

\return	true if an AutoCAD file, otherwise false.
*/
bool PYXSharedGDALDataSet::isAutoCAD() const
{
	if (!m_pGDALDataset)
	{
		return false;
	}

	return	boost::iequals(m_pGDALDataset->GetDriverName(), "dwg") ||
			boost::iequals(m_pGDALDataset->GetDriverName(), "dxf");
}

/*!
Check the GDAL driver to see if this is a netCDF file.

\return	true if a netCDF file, otherwise false.
*/
bool PYXSharedGDALDataSet::isNetCDF() const
{
	if (!m_pGDALDataset)
	{
		return false;
	}

	return	boost::iequals(m_pGDALDataset->GetDriverName(), "netcdf");
}

/*!
Check the GDAL driver to see if this is a GRIB file.

\return	true if a GRIB file, otherwise false.
*/
bool PYXSharedGDALDataSet::isGRIB() const
{
	if (!m_pGDALDataset)
	{
		return false;
	}

	return	boost::iequals(m_pGDALDataset->GetDriverName(), "grib");
}

/*!
Check the GDAL driver to see if this is a WFS data source.

\return true if a WFS data source, otherwise false.
*/
bool PYXSharedGDALDataSet::isWFS() const
{
	if (!m_pGDALDataset)
	{
		return false;
	}

	return boost::iequals(m_pGDALDataset->GetDriverName(), "wfs");
}

#if UNUSED
/*!
Debugging method for dumping the structure of a GDALDataSet to a file.
The structure of the file is dumped to logfile.txt in the current directory.
	
\param pDataSet	The data set.
*/
void PYXSharedGDALDataSet::dumpDataSet(GDALDataset* pDataset)
{
	std::ofstream log("logfile.txt", std::ios_base::out | std::ios_base::trunc );

	log << "----- CONTENTS OF DWG FILE -----" << std::endl;
	const char *pcDescription = pDataset->GetDescription();
	log << "GetDescription: " << pcDescription << std::endl;
	int nRasterXSize = pDataset->GetRasterXSize();
	log << "GetRasterXSize: " << nRasterXSize << std::endl;
	int nRasterYSize = pDataset->GetRasterYSize();
	log << "GetRasterYSize: " << nRasterYSize << std::endl;
	int nRasterCount = pDataset->GetRasterCount();
	log << "GetRasterCount: " << nRasterCount << std::endl;
	const char* pcProjectionRef = pDataset->GetProjectionRef();
	log << "GetProjectionRef: " << pcProjectionRef << std::endl;
	char** pcFileList = pDataset->GetFileList();
	const char* pcDriverName = pDataset->GetDriverName();
	log << "GetDriverName: " << pcDriverName << std::endl;
	int nGCPCount = pDataset->GetGCPCount();
	log << "GetGCPCount: " << nGCPCount << std::endl;
	int nLayerCount = pDataset->GetLayerCount();
	log << "GetLayerCount: " << nLayerCount << std::endl;
	for (int i = 0; i < nLayerCount; ++i)
	{
		log << "- LAYER: " << i << " -" << std::endl;
		OGRLayer* pLayer = pDataset->GetLayer(i);
		const char* pcName = pLayer->GetName();
		log << "-GetName: " << pcName << std::endl;
		const char *pcLayerDescription = pLayer->GetDescription();
		log << "GetDescription: " << pcLayerDescription << std::endl;
		OGREnvelope envelope;
		pLayer->GetExtent(&envelope);
		log << "-GetExtent: " << envelope.MinX << ", " << envelope.MinY << ", " << envelope.MaxX << ", " << envelope.MaxY << std::endl;
		const char* pcFIDColumn = pLayer->GetFIDColumn();
		log << "-GetFIDColumn: " << pcFIDColumn << std::endl;
		const char* pcGeometryColumn = pLayer->GetGeometryColumn();
		log << "-GetGeometryColumn: " << pcGeometryColumn << std::endl;

		OGRFeatureDefn* pFeatureDefn = pLayer->GetLayerDefn();
		int nFieldCount = pFeatureDefn->GetFieldCount();
		log << "-GetFieldCount: " << nFieldCount << std::endl;
		for (int j = 0; j < nFieldCount; ++j)
		{
			log << "-- FIELD DEFINITION: " << j << " --" << std::endl;
			OGRFieldDefn* pFieldDefn = pFeatureDefn->GetFieldDefn(j);
			const char* pcNameRef = pFieldDefn->GetNameRef();
			log << "--GetNameRef: " << pcNameRef << std::endl;
			OGRFieldType fieldType = pFieldDefn->GetType();
			log << "--GetType: " << fieldType << std::endl;
			OGRFieldSubType fieldSubType = pFieldDefn->GetSubType();
			log << "--GetSubType: " << fieldSubType << std::endl;
			int nWidth = pFieldDefn->GetWidth();
			log << "--GetWidth: " << nWidth << std::endl;
			int nPrecision = pFieldDefn->GetPrecision();
			log << "--GetPrecision: " << nPrecision << std::endl;
			const char* pcDefault = pFieldDefn->GetDefault();
			if (pcDefault != NULL)
			{
				log << "--GetDefault: " << pcDefault << std::endl;
			}
		}

		GIntBig nFeatureCount = pLayer->GetFeatureCount();
		log << "-GetFeatureCount: " << nFeatureCount << std::endl;
		pLayer->SetAttributeFilter(NULL);
		pLayer->SetSpatialFilter(NULL);
		pLayer->ResetReading();
		int k = 0;
		OGRFeature* pFeature = pLayer->GetNextFeature();
		while (pFeature != NULL)
		{
			log << "---------------------------------------" << std::endl;
			log << "-- FEATURE: " << k << " --" << std::endl;
			int nFieldCount = pFeature->GetFieldCount();
			log << "--GetFieldCount: " << nFieldCount << std::endl;
			for (int m = 0; m < nFieldCount; ++m)
			{
				log << "--- FIELD: " << m << " ---" << std::endl;
				const char* pcFieldValue = pFeature->GetFieldAsString(m);
				log << "---GetFieldAsString: " << pcFieldValue << std::endl;
			}

			OGRFeature::DestroyFeature(pFeature);
			pFeature = pLayer->GetNextFeature();
			++k;
		}
	}
	log.flush();
	log.close();
}
#endif
