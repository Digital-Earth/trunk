/******************************************************************************
pyx_ogr_data_source.cpp

begin		: 2004-10-19
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_GDAL_SOURCE
#include "pyx_ogr_data_source.h"

#include "exceptions.h"
#include "pyx_shared_gdal_data_set.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/geometry/cell.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/sampling/xy_bounds_geometry.h"
#include "pyxis/utility/coord_2d.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/great_circle_arc.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/checksum_calculator.h"
#include "pyxis/pipe/process_local_storage.h"
#include "pyxis/procs/user_credentials_provider.h"

// ogr includes
#include "ogrsf_frmts.h"

// standard includes
#include <cassert>
#include <fstream>
#include <limits>
#include <boost/algorithm/string.hpp>
#include "pyxis/data/feature_iterator_linq.h"

// Properties Tag
const std::string PYXOGRDataSource::kstrScope = "PYXOGRDatasource";

// Default resolution.
static const int knDefaultResolution = -1;


OGRFeatureObject::OGRFeatureObject(OGRFeature * feature)
{
	assert(feature != 0);
	m_feature = feature;
}

OGRFeatureObject::~OGRFeatureObject()
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);
	OGRFeature::DestroyFeature(m_feature);
}

//! Tester class
Tester<PYXOGRDataSource> gTester;

//! Test method
void PYXOGRDataSource::test()
{
	/*
	OGRSFDriver* driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("WFS");
	//Test opening a WFS service
	PYXPointer<PYXSharedGDALDataSet> dataSource = PYXSharedGDALDataSet::createVector("wfs:http://giswebservices.massgis.state.ma.us/geoserver/wfs?SERVICE=WFS");

	std::string strName = dataSource->get()->GetName();

	TRACE_INFO("Opening OGR data set '" << strName << "'.");
	*/
}

/*!
Constructor initializes member variables.
*/
PYXOGRDataSource::PYXOGRDataSource() :
	m_pOGRDataSource(0),
	m_pOGRLayer(0),
	m_spGeometry(0),
	m_internalCoordConverter(0),
	m_spCoordConverter(0),
	m_localStorage(0),
	m_spFeatDefn(0),
	m_addExtentToMetadata(true),
	m_axisFlip(false),
	m_bReadingLock(false)
{
}

/*!
Check min and max bounds for y to ensure they are correct.
	
*.mdb data sources have the origin at the bottom of the
area instead of the top.  Therefore Y values for max
and min are backwards.
	
This function corrects this.
	
\param pBounds	Min,Max X and Y bounds of the data source/feature.
*/
//! Check min and max y bounds to ensure they are correct.
void PYXOGRDataSource::correctOGREnvelope(OGREnvelope* pBounds)
{
	double fMinY;
	double fMaxY;
	
	fMinY = std::min(pBounds->MinY, pBounds->MaxY);
	fMaxY = std::max(pBounds->MinY, pBounds->MaxY);
	
	pBounds->MinY = fMinY;
	pBounds->MaxY = fMaxY;
}

/*!
Destructor cleans up memory.
*/
PYXOGRDataSource::~PYXOGRDataSource()
{
	// The OGRLayer created for an AutoCAD data source is temporary and must be
	// released before the data source is closed.
	if (m_pOGRDataSource != nullptr && m_pOGRDataSource->isAutoCAD())
	{
		if (m_pOGRLayer != nullptr)
		{
			m_pOGRDataSource->get()->ReleaseResultSet(m_pOGRLayer);
			m_pOGRLayer = nullptr;
		}
	}
}


std::string escapeFileName(const std::string & name)
{
	std::string result = name;
	size_t nLen = result.length();

	for (size_t x = 0; x < nLen; ++x)
	{
		if (result[x] == '/')
		{
			result[x] = '_';
		}
		if (result[x] == '\\')
		{
			result[x] = '_';
		}
		if (result[x] == '*')
		{
			result[x] = '_';
		}
		if (result[x] == '?')
		{
			result[x] = '_';
		}
		if (result[x] == '&')
		{
			result[x] = '_';
		}
		if (result[x] == '@')
		{
			result[x] = '_';
		}
		if (result[x] == '=')
		{
			result[x] = '_';
		}			
		if (result[x] == ':')
		{
			result[x] = '_';
		}
	}

	return result;
}

/*!
Open the data source. This must be done before the OGR data source is used. If an SRS is not 
handed in then the open() will try to get it from the data source.

\param pOGRDataSource	The OGRDataSource to associate with this data source (ownership shared).
\param spSRS			The spatial reference system.
\param flipAxis			Is it necessary to flip the x and y axes
\param nLayerIndex		The layer index or 0 for default
\param strLayerName		The layer name or "" for default
\param storage			Pointer to local storage
*/
void PYXOGRDataSource::open(	PYXPointer<PYXSharedGDALDataSet> pOGRDataSource,
								boost::intrusive_ptr<ISRS> spSRS,
								bool flipAxis,
								int nLayerIndex, 
								const std::string& strLayerName,
								PYXPointer<PYXLocalStorage> storage)
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	assert(nullptr != pOGRDataSource);
	assert(nullptr == m_pOGRDataSource);

	m_pOGRDataSource = pOGRDataSource;
	m_pOGRLayer = NULL;
	m_localStorage = storage;
	m_axisFlip = flipAxis;

	// save the name of the data source
	std::string strName = m_pOGRDataSource->get()->GetDescription();
	TRACE_INFO("Opening OGR data set '" << strName << "'.");

	int nLayers = m_pOGRDataSource->get()->GetLayerCount();
	TRACE_INFO("Opening OGR data set '" << strName << "' has " << nLayers  << " layers.");
	if (nLayers <= 0)
	{
		PYXTHROW(GDALOGRException, "No layers in the OGR data source.");
	}

	// AutoCAD drivers put all features in one GDAL layer called "entities"
	if (pOGRDataSource->isAutoCAD())
	{
		// save the AutoCAD layer name so it can be used in attribute filters
		if (!strLayerName.empty())
		{
			m_strAutoCADLayerName = strLayerName;
		}
		else
		{
			m_strAutoCADLayerName = StringUtils::toString(nLayerIndex);
		}

		// Create a temporary OGRLayer representing the AutoCAD entities from a given
		// AutoCAD layer. This OGRLayer must be released before the data source is closed.
		std::string strWhere = "SELECT * FROM entities WHERE Layer = '" + m_strAutoCADLayerName + "'";
		m_pOGRLayer = pOGRDataSource->get()->ExecuteSQL(strWhere.c_str(), NULL, NULL);
		if (m_pOGRLayer == NULL)
		{
			PYXTHROW(GDALOGRException, "Unable to create data set for AutoCAD file.");
		}

		TRACE_INFO("OGR Layer (AutoCAD) 'entities' was selected.");
	}

	if (!m_pOGRLayer)
	{
		// other file based formats prefer to access the layer by index
		if (!pOGRDataSource->isWFS() && nLayerIndex < nLayers)
		{
			m_pOGRLayer = m_pOGRDataSource->get()->GetLayer(nLayerIndex);
			if (m_pOGRLayer == 0)
			{
				PYXTHROW(GDALOGRException,"failed to open Datasource layer");
			}
			TRACE_INFO("OGR Layer '" << StringUtils::toString(nLayerIndex) << "' was selected.");
		}
		else if (!strLayerName.empty())
		{
			m_pOGRLayer = m_pOGRDataSource->get()->GetLayerByName(strLayerName.c_str()) ;
			if (m_pOGRLayer == 0)
			{
				PYXTHROW(GDALOGRException,"failed to open Datasource layer");
			}
			TRACE_INFO("OGR Layer '" << strLayerName << "' was selected.");
		}
		else
		{
			PYXTHROW(GDALOGRException, "Layer index is out of range.");
		}

		m_pOGRLayer->SetAttributeFilter(NULL); 
		m_pOGRLayer->SetSpatialFilter(NULL);
	}

	if (spSRS)
	{
		setSpatialReferenceSystem(spSRS->getSRS());
	}
	else
	{
		// if no SRS is specified in the pipeline, get the SRS, 
		// if any, specified in the layer itself
		OGRSpatialReference* pSourceRef = m_pOGRLayer->GetSpatialRef();
		if (pSourceRef)
		{
			// Create new coord converter.
			m_internalCoordConverter.reset(new CoordConverterImpl());
			assert(m_internalCoordConverter.get());

			// Get a pointer to the WKT export.
			char* pPtr = 0;
			pSourceRef->exportToWkt(&pPtr);
			if (pPtr)
			{
				// Initialize the coord converter from the WKT export.
				std::string strWKT(pPtr);
				m_internalCoordConverter->initialize(strWKT);
				OGRFree(pPtr); 
			}
			else
			{
				// Initialize the coord converter from the source spatial reference.
				m_internalCoordConverter->initialize(*pSourceRef);
			}
		}
		else
		{
			PYXTHROW(MissingSRSException, "Null SRS for OGR data source.");
		}

		if (m_axisFlip)
		{
			m_spCoordConverter = new PYXAxisFlipCoordConverter(m_internalCoordConverter);
		}
		else
		{
			m_spCoordConverter = m_internalCoordConverter;
		}
	}

	TRACE_INFO("SRS for OGR source was set.");

	initMetaData();
	TRACE_INFO("Metadata for OGR source was set.");

	// build an rTree for the layer
	buildRTree(strName, strLayerName, nLayerIndex);

	TRACE_INFO("Opening OGR data set '" << strName << "' completed.");
}

/*!
OGR is notoriously slow for accessing feature in a random manner or with a spatial qualification. 
We build an rTree and implement our own spatial indexing to improve performance. We also use the
rTree for iterating through features in a layer. This is required for AutoCAD files as all
features are put in a single GDAL layer with the AutoCAD layer being specified by each feature's
"Layer" attribute.

\param strName	The name of the data source.
*/
void PYXOGRDataSource::buildRTree(	const std::string& strName,
									const std::string& strLayerName,
									int nLayerIndex	)
{
	std::string strFileName;
	
	// Special handling for VMAP
	static const std::string strPrefix("gltp:/vrf/");
	if (strName.find(strPrefix) == 0)
	{
		strFileName = strName.substr(strPrefix.length(), std::string::npos);

		// Change all forwards slashes to back slashes.
		size_t nLen = strFileName.length();

		for (size_t x = 0; x < nLen; ++x)
		{
			if (strFileName[x] == '/')
			{
				strFileName[x] = '\\';
			}
			if (strFileName[x] == '*')
			{
				strFileName[x] = '_';
			}
			if (strFileName[x] == '@')
			{
				strFileName[x] = '_';
			}
			// Change all full colons (:) to underscores, except the one at position 1 (ie. c:\)
			if (x > 1 && strFileName[x] == ':')
			{
				strFileName[x] = '_';
			}
		}
	}
	// handle WFS URL's
	else if (strName.find("wfs:") != std::string::npos)
	{
		boost::filesystem::path root = AppServices::getCacheDir("rtree");

		strFileName = "wfs_" + escapeFileName(strName);
		
		strFileName = FileUtils::pathToString((root / strFileName));
	}
	// handle GeoServices FeatureServer URL's
	else if (boost::starts_with(strName, "http:") || boost::starts_with(strName, "https:"))
	{
		boost::filesystem::path root = AppServices::getCacheDir("rtree");

		strFileName = "http_" + escapeFileName(strName);
		
		strFileName = FileUtils::pathToString((root / strFileName));
	}
	// handle directories of files
	else if (FileUtils::isDirectory(FileUtils::stringToPath(strName)))
	{
		boost::filesystem::path root = AppServices::getCacheDir("rtree");

		// change directory name so it can be used as a filename
		strFileName = strName;
		boost::replace_all(strFileName, ":", "_");
		boost::replace_all(strFileName, "/", "_");
		boost::replace_all(strFileName, "\\", "_");
		strFileName = FileUtils::pathToString((root / strFileName));
	}
	// handle single files
	else
	{
		strFileName = strName;

		std::string checkSum = ChecksumCalculator::getChecksumCalculator()->calculateFileCheckSum(strFileName);

		//TODO: we need a better way to detect checksum calculation error, this is an infinite loop if there is some error
		while(checkSum.empty())
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
			checkSum = ChecksumCalculator::getChecksumCalculator()->calculateFileCheckSum(strFileName);
		}

		boost::filesystem::path root = AppServices::getCacheDir("rtree");
		boost::replace_all(checkSum,"/","_");
		strFileName = FileUtils::pathToString((root / checkSum));
	}

	//if we have many layers. we need to create different RTree for each layer...
	if (!strLayerName.empty())
	{
		strFileName += ".layer_" + escapeFileName(strLayerName);
	}
	else if (nLayerIndex > 0)
	{
		strFileName += ".layer_" + StringUtils::toString(nLayerIndex);
	}

	/*
	// initialize the rTree
	bool bNewEmptyFile = m_rTree.initialize(strFileName);

	// If the tree is empty it is new and we need to add data to it.
	if (bNewEmptyFile)
	{
		// iterate over each feature in the data source.
		m_pOGRLayer->SetAttributeFilter(NULL);
		m_pOGRLayer->SetSpatialFilter(NULL);
		m_pOGRLayer->ResetReading();

		for (	OGRFeature* pOGRFeature = m_pOGRLayer->GetNextFeature();
				pOGRFeature != 0;
				pOGRFeature = m_pOGRLayer->GetNextFeature()	)
		{
			auto safePointer = OGRFeatureObject::create(pOGRFeature);

			OGRGeometry* pOGRGeometry = pOGRFeature->GetGeometryRef();

			//check if feature has a geometry
			if (pOGRGeometry == nullptr) 
			{
				continue;
			}

			OGREnvelope envelope;
			pOGRGeometry->getEnvelope(&envelope);

			// insert rectangle into rTree
			PYXRect2DDouble r(envelope.MinX, envelope.MinY, envelope.MaxX, envelope.MaxY);

			auto nFID = pOGRFeature->GetFID();

			if (sizeof(nFID) < sizeof(void*))
			{
				// this will be a problem when we move to 64-bit
				PYXTHROW(GDALOGRException, "Incompatible sizes - to be resolved.");
			}

			m_rTree.insert(r, reinterpret_cast<void*>(nFID));
		}

		// flush the tree
		m_rTree.flush();
	}	
	*/
}

/*!
Set the resolution of the data source and create the geometry.

\param	nResolution	The resolution.
*/
void PYXOGRDataSource::setResolution(int nResolution)
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	// if the resolution is invalid, try to create it automatically
	if (nResolution < 0)
	{
		std::string strFileName(m_pOGRDataSource->get()->GetDescription());
		nResolution = determineResolution(strFileName);
	}

	if (nResolution >= 0)
	{
		createGeometry(nResolution);
	}
}

/*!
Calculate the minimum distance between any two successive points in a line
geometry.

\return	The minimum distance in radians.
*/
double PYXOGRDataSource::calcMinDistance(const OGRLineString& ogrLineString)
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	double fMinDist = std::numeric_limits<double>::max();

	const int nNumPoints = ogrLineString.getNumPoints();
	if (0 < nNumPoints)
	{
		if (!m_spCoordConverter)
		{
			PYXTHROW(GDALOGRException, "There is no coordinate converter in the OGR data source.");
		}

		OGRPoint ptPrev;
		OGRPoint ptCur;

		ogrLineString.getPoint(0, &ptPrev);

		for (int n = 1; n < nNumPoints; ++n)
		{
			ogrLineString.getPoint(n, &ptCur);

			PYXCoord2DDouble nativePrev(ptPrev.getX(), ptPrev.getY());
			CoordLatLon coordPrev;
			m_spCoordConverter->nativeToLatLon(nativePrev, &coordPrev);

			PYXCoord2DDouble nativeCur(ptCur.getX(), ptCur.getY());
			CoordLatLon coordCur;
			m_spCoordConverter->nativeToLatLon(nativeCur, &coordCur);

			double fDist = SphereMath::distanceBetween(coordPrev, coordCur);
			if (0 < fDist && fDist < fMinDist)
			{
				fMinDist = fDist;
			}

			ptPrev = ptCur;
		}
	}

	return fMinDist;
}

/*!
Calculate the minimum distance between any two successive points in a polygon
geometry.

\return	The minimum distance in radians.
*/
double PYXOGRDataSource::calcMinDistance(const OGRPolygon& ogrPolygon)
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	double fMinDist = std::numeric_limits<double>::max();

	const int nNumPoints = ogrPolygon.getExteriorRing()->getNumPoints();
	if (0 < nNumPoints)
	{
		if (!m_spCoordConverter)
		{
			PYXTHROW(GDALOGRException, "There is no coordinate converter in the OGR data source.");
		}

		OGRPoint ptPrev;
		OGRPoint ptCur;

		ogrPolygon.getExteriorRing()->getPoint(0, &ptPrev);

		for (int n = 1; n < nNumPoints; ++n)
		{
			ogrPolygon.getExteriorRing()->getPoint(n, &ptCur);

			PYXCoord2DDouble nativePrev(ptPrev.getX(), ptPrev.getY());
			CoordLatLon coordPrev;
			m_spCoordConverter->nativeToLatLon(nativePrev, &coordPrev);

			PYXCoord2DDouble nativeCur(ptCur.getX(), ptCur.getY());
			CoordLatLon coordCur;
			m_spCoordConverter->nativeToLatLon(nativeCur, &coordCur);

			double fDist = SphereMath::distanceBetween(coordPrev, coordCur);
			if (0 < fDist && fDist < fMinDist)
			{
				fMinDist = fDist;
			}

			ptPrev = ptCur;
		}
	}

	return fMinDist;
}

/*!
\return	A reasonable resolution for the data.

\param	strFileName	Optional name of file to read resolution from (if file exists)
*/
int PYXOGRDataSource::determineResolution(std::string& strFileName)
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	std::string strFile(strFileName);
	strFile.append(".resolution");

	// If we have a file name - see if it contains the resolution.
	if (!strFileName.empty())
	{
		std::ifstream ifs;

		ifs.open(strFile.c_str());

		if (ifs)
		{
			int nResolution;

			ifs >> nResolution;

			if (nResolution >=0)
			{
				ifs.close();
				return nResolution;
			}

			ifs.close();
		}
	}

	double fMinDistance = std::numeric_limits<double>::max();

	// Iterate over each feature in the data source.
	m_pOGRLayer->SetAttributeFilter(NULL);
	m_pOGRLayer->SetSpatialFilter(NULL);
	m_pOGRLayer->ResetReading();

	for (	OGRFeature * pOGRFeature = m_pOGRLayer->GetNextFeature();
			pOGRFeature != 0 ;
			pOGRFeature = m_pOGRLayer->GetNextFeature()	)
	{
		auto safePointer = OGRFeatureObject::create(pOGRFeature);

		OGRGeometry* pOGRGeometry = pOGRFeature->GetGeometryRef();
		
		//make sure feature has a geometry
		if (pOGRGeometry == nullptr)
		{
			continue;
		}

		OGRwkbGeometryType nOGRGeometryType = pOGRGeometry->getGeometryType();

		double fDistance = std::numeric_limits<double>::max();

		switch (nOGRGeometryType)
		{
			case wkbLineString:
			case wkbLineString25D:
				fDistance = calcMinDistance(reinterpret_cast<OGRLineString&>(*pOGRGeometry));
				break;

			case wkbPolygon:
			case wkbPolygon25D:
				fDistance = calcMinDistance(reinterpret_cast<OGRPolygon&>(*pOGRGeometry));
				break;

			default:
				// if the geometry type is wkbMultiPolygon, we don't want to 
				// return -1
				// TODO[kabiraman]: do we need some logic for handling 
				// Multi Polygons?
				continue;
		}

		if (0 < fDistance && fDistance < fMinDistance)
		{
			fMinDistance = fDistance;
		}
	}

	if (fMinDistance == std::numeric_limits<double>::max())
	{
		// We don't have a minimum distance.
		return knDefaultResolution;
	}

	int nResolution = SnyderProjection::getInstance()->precisionToResolution(fMinDistance);

	TRACE_INFO("Determine OGR data source resolution: " << nResolution);

	// Write resolution to supplied file.

	// If we have a file name - write resolution to it.
	if (!strFileName.empty())
	{
		std::ofstream ofs;

		ofs.open(strFile.c_str());

		if (ofs)
		{
			ofs << nResolution << std::endl;

			ofs.close();
		}
	}

	return nResolution;
}

/*!
Call this method after the data source is opened to determine if the data
source contains a spatial reference system. If not, a spatial reference system
must be supplied by calling setSpatialReference() before getFeatureIterator()
is called.

\return	true if the data source has a spatial reference system, otherwise false
*/
bool PYXOGRDataSource::hasSpatialReferenceSystem() const
{
	return m_spCoordConverter;
}

/*!
Specify the spatial reference system for the data source. Call this method to
set the spatial reference system if after the data source is ed
hasSpatialReference() returns false.

\param	spSRS	The spatial reference system.
*/
void PYXOGRDataSource::setSpatialReferenceSystem(PYXPointer<PYXSpatialReferenceSystem> spSRS)
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	// As it turns out, creating a coordinate converter from Datum may sometimes produce unexpected result,
	// so we first try to initialize it from the SRS well known text
	if (spSRS->getWKT().empty() ||
		!(safeSetSpatialReferenceSystemFromWkt(m_internalCoordConverter, spSRS->getWKT()) ||
		// Spatial references by ArcGis sometimes come without 'EPSG:' prefix, which may be required
		(StringUtils::isNumeric(spSRS->getWKT()) && safeSetSpatialReferenceSystemFromWkt(m_internalCoordConverter, "EPSG:" + spSRS->getWKT())))
		)
	{
		m_internalCoordConverter.reset(new CoordConverterImpl());
		m_internalCoordConverter->initialize(*spSRS);
	}

	// defer geometry creation until the resolution is set
	if (m_axisFlip)
	{
		m_spCoordConverter = new PYXAxisFlipCoordConverter(m_internalCoordConverter);
	}
	else
	{
		m_spCoordConverter = m_internalCoordConverter;
	}
}

/*!
Try to set a spatial reference system for a coordinate converter from a well known text,
without throwing an exception.

\param	coordConverter	The coordinate converter (gets reset first).
\param	strWkt			The well known text.

\return	Sucees of the operation.
*/
bool PYXOGRDataSource::safeSetSpatialReferenceSystemFromWkt(boost::intrusive_ptr<CoordConverterImpl>& coordConverter, const std::string& srsWkt) const
{
	coordConverter.reset(new CoordConverterImpl());
	try
	{
		coordConverter->initialize(srsWkt);
		return true;
	}
	catch(const PYXException& e)
	{
		coordConverter.reset();
		TRACE_ERROR("Failed to initialize a coordinate converter from WKT: " << e.getFullErrorString());
		return false;
	}
}

/*!
Get an iterator to all features in the data source.

\return	The iterator.
*/
PYXPointer<FeatureIterator> PYXOGRDataSource::getFeatureIterator() const
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	if (!m_spGeometry || (0 == m_pOGRLayer) || !hasSpatialReferenceSystem())
	{
		assert(false && "Requesting feature iterator for uninitialized PYXOGRDataSource.");
		PYXTHROW(	GDALProcessException,
					"Requesting feature iterator for uninitialized PYXOGRDataSource."	);
	}

	if (m_bReadingLock)
	{
		PYXTHROW(	GDALProcessException,
					"Can't create new feature iterator while reading lock is set."	);
	}



	return PYXOGRAllFeaturesNativeIterator::create(const_cast<PYXOGRDataSource*>(this),getFeatureDefinition(),getResolution());
}
	


/*!
Get an iterator to the features in the data source that meet the specified
spatial qualification.

\param	geometry	The spatial qualification.

\return	The iterator.
*/
PYXPointer<FeatureIterator> PYXOGRDataSource::getFeatureIterator(
	const PYXGeometry& geometry	) const
{
	return PYXFeatureIteratorLinq(getFeatureIterator()).filter(geometry);
}

/*!
Get an iterator to the features in the data source that meet the specified
attribute qualification.

\param	strWhere	The attribute qualification.

\return	The iterator.
*/
PYXPointer<FeatureIterator> PYXOGRDataSource::getFeatureIterator(
	const std::string& strWhere	) const
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	if (!m_spGeometry || (0 == m_pOGRLayer) || !hasSpatialReferenceSystem())
	{
		assert(false && "Requesting feature iterator for uninitialized PYXOGRDataSource.");
		PYXTHROW(	GDALProcessException,
					"Requesting feature iterator for uninitialized PYXOGRDataSource."	);
	}

	if (m_bReadingLock)
	{
		PYXTHROW(	GDALProcessException,
					"Can't create new attribute feature iterator while reading lock is set."	);
	}

	std::set<GIntBig> setFID;
	{
		m_pOGRLayer->SetAttributeFilter(strWhere.c_str());
		m_pOGRLayer->SetSpatialFilter(NULL);
		m_pOGRLayer->ResetReading();

		for (	OGRFeature* pOGRFeature = m_pOGRLayer->GetNextFeature();
				pOGRFeature != 0;
				pOGRFeature = m_pOGRLayer->GetNextFeature()	)
		{
			auto safePointer = OGRFeatureObject::create(pOGRFeature);
			OGRGeometry* pOGRGeometry = pOGRFeature->GetGeometryRef();
			if (pOGRGeometry != nullptr) 
			{
				TRACE_INFO("Warning, skipping feature with no geometry. FeatureID=" << pOGRFeature->GetFID() );
				setFID.insert(pOGRFeature->GetFID());
			}
		}
	}

	boost::shared_ptr<PYXrTree::KeyList> filteredListKey(new PYXrTree::KeyList());

	for(auto id : setFID)
	{
		long nID = static_cast<long>(id);
		filteredListKey->insert(reinterpret_cast<void*>(nID));
	}
	
	return PYXOGRFeatureLazyIterator::create(	filteredListKey,
												this,
											    getFeatureDefinition(),
												getResolution()	);
}

/*!
Get an iterator to the features in the data source that meet the specified
spatial and attribute qualification.

\param	geometry	The spatial qualification.
\param	strWhere	The attribute qualification.

\return	The iterator.
*/
PYXPointer<FeatureIterator> PYXOGRDataSource::getFeatureIterator(
	const PYXGeometry& geometry,
	const std::string& strWhere	) const
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	if (!m_spGeometry || (0 == m_pOGRLayer) || !hasSpatialReferenceSystem())
	{
		assert(false && "Requesting feature iterator for uninitialized PYXOGRDataSource.");
		PYXTHROW(	GDALProcessException,
					"Requesting feature iterator for uninitialized PYXOGRDataSource."	);
	}

	if (m_bReadingLock)
	{
		PYXTHROW(	GDALProcessException,
					"Can't create new attribute and spatial feature iterator while reading lock is set."	);
	}

	boost::shared_ptr<PYXrTree::KeyList> featureListKey(new PYXrTree::KeyList());

	// calculate the bounding rectangle(s) from the geomtry
	PYXRect2DDouble r1;
	PYXRect2DDouble r2;

	if (dynamic_cast<const PYXGlobalGeometry*>(&geometry))
	{
		r1 = m_bounds;
	}
	else
	{
		calcBoundingRects(&geometry, &r1, &r2);
	}

	if (!r1.empty())
	{
		std::set<GIntBig> setFID;
		{
			m_pOGRLayer->SetAttributeFilter(strWhere.c_str());
			m_pOGRLayer->SetSpatialFilterRect(r1.xMin(), r1.yMin(), r1.xMax(), r1.yMax());
			m_pOGRLayer->ResetReading();

			for (	OGRFeature* pOGRFeature = m_pOGRLayer->GetNextFeature();
					pOGRFeature != 0;
					pOGRFeature = m_pOGRLayer->GetNextFeature()	)
			{
				auto safePointer = OGRFeatureObject::create(pOGRFeature);
				OGRGeometry* pOGRGeometry = pOGRFeature->GetGeometryRef();
				if (pOGRGeometry != nullptr) 
				{
					TRACE_INFO("Warning, skipping feature with no geometry. FeatureID=" << pOGRFeature->GetFID() );
					setFID.insert(pOGRFeature->GetFID());
				}
			}
		}

		for(auto id : setFID)
		{
			long nID = static_cast<long>(id);
			featureListKey->insert(reinterpret_cast<void*>(nID));
		}
	}

	if (!r2.empty())
	{
		std::set<GIntBig> setFID;
		{
			m_pOGRLayer->SetAttributeFilter(strWhere.c_str());
			m_pOGRLayer->SetSpatialFilterRect(r2.xMin(), r2.yMin(), r2.xMax(), r2.yMax());
			m_pOGRLayer->ResetReading();

			for (	OGRFeature* pOGRFeature = m_pOGRLayer->GetNextFeature();
					pOGRFeature != 0;
					pOGRFeature = m_pOGRLayer->GetNextFeature()	)
			{
				auto safePointer = OGRFeatureObject::create(pOGRFeature);
				OGRGeometry* pOGRGeometry = pOGRFeature->GetGeometryRef();
				if (pOGRGeometry != nullptr) 
				{
					TRACE_INFO("Warning, skipping feature with no geometry. FeatureID=" << pOGRFeature->GetFID() );
					setFID.insert(pOGRFeature->GetFID());
				}
			}
		}

		for(auto id : setFID)
		{
			long nID = static_cast<long>(id);
			featureListKey->insert(reinterpret_cast<void*>(nID));
		}
	}

	return PYXOGRFeatureLazyIterator::create(	featureListKey,
												this,
											    getFeatureDefinition(),
												geometry.getCellResolution() );	
}

/*!
Calculate the bounding rectangles that cover the specified PYXIS geometry. Most
of the time only one rectangle will be returned (the other will be empty). For
global data sets, where the geometry spans the international dateline, both
rectangles are returned.

\param	pGeometry	The geometry (ownership retained by caller)
\param	pRect1		The first rectangle (out)
\param	pRect2		The second rectangle (out);
*/
void PYXOGRDataSource::calcBoundingRects(	const PYXGeometry* pGeometry,
											PYXRect2DDouble* pRect1,
											PYXRect2DDouble* pRect2	) const
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	assert((pRect1 != 0) && "Invalid argument.");
	assert((pRect2 != 0) && "Invalid argument.");

	pRect1->setEmpty();
	pRect2->setEmpty();

	if (pGeometry)
	{
		const PYXCell* pC = dynamic_cast<const PYXCell*>(pGeometry);
		if (pC)
		{
			calcCellBoundingRects(*pC, pRect1, pRect2);
		}
		else
		{
			const PYXTile* pT = dynamic_cast<const PYXTile*>(pGeometry);
			if (pT)
			{
				calcTileBoundingRects(*pT, pRect1, pRect2);
			}
			else
			{
				assert(false && "Unhandled PYXIS geometry.");
			}
		}
	}
}

/*!
Calculate the bounding rectangles that cover the specified PYXIS cell. Most
of the time only one rectangle will be returned (the other will be empty). For
global data sets, where the cell spans the international dateline, both
rectangles are returned.

\param	cell	The cell.
\param	pRect1	The first rectangle (out)
\param	pRect2	The second rectangle (out);
*/
void PYXOGRDataSource::calcCellBoundingRects(	const PYXCell& cell,
												PYXRect2DDouble* pRect1,
												PYXRect2DDouble* pRect2	) const
{ 
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	assert((pRect1 != 0) && "Invalid argument.");
	assert((pRect2 != 0) && "Invalid argument.");
	assert(m_spCoordConverter && "Coordinate converter not available.");

	// Clear incoming rectangles.
	pRect1->setEmpty();
	pRect2->setEmpty();

	if (cell.isEmpty())
	{
		// No work required.
		return;
	}

	/*
		ALGORITHM: Project the center and each vertex into the native
		coordinate space. Compared to the center, each vertex will either be
		in the vicinity, or wrap in the coordinate space so it is not in the
		vicinity. Detect this by comparing the vertex's distance (sum of
		squares is sufficient) from the center to its distance from the
		opposite vertex. If the center is closer, the vertex is in the
		vicinity (i.e. hasn't wrapped). Once the vertices are sorted this
		way, bounding rectangles are easily constructed.
	*/

	// Get the center index.
	PYXCoord2DDouble xyCenter;
	m_spCoordConverter->pyxisToNative(cell.getIndex(), &xyCenter);

	// Get the vertex indices.
	int nVertexCount = 0;
	PYXCoord2DDouble xyVertexArray[6];
	PYXVertexIterator it(cell.getIndex());
	for (; !it.end(); it.next())
	{
		// convert this index into native coordinates
		m_spCoordConverter->pyxisToNative(it.getIndex(), &xyVertexArray[nVertexCount++]);
	}

	// Try to detect polar cells.
	// (Will this work if all coordinates are positive? e.g. 0..360 instead of -180..180)
	bool bSpansMeridian = false;
	bool bSpansDateline = false;
	const double kfHalfWidth = (m_bounds.xMax() - m_bounds.xMin()) / 2.0;
	for (int n = 0; n != nVertexCount; ++n)
	{
		int nPrev = (n + nVertexCount - 1) % nVertexCount;
		if (	((xyVertexArray[n].x() > 0) && (xyVertexArray[nPrev].x() < 0)) ||
				((xyVertexArray[n].x() < 0) && (xyVertexArray[nPrev].x() > 0))	)
		{
			if (fabs(xyVertexArray[n].x() - xyVertexArray[nPrev].x()) > kfHalfWidth)
			{
				bSpansDateline = true;
			}
			else
			{
				bSpansMeridian = true;
			}
		}
	}

	// Sum of squares from center to each vertex.
	double ssCenterArray[6];
	for (int n = 0; n != nVertexCount; ++n)
	{
		double xd = xyVertexArray[n].x() - xyCenter.x();
		double yd = xyVertexArray[n].y() - xyCenter.y();
		ssCenterArray[n] = xd * xd + yd * yd;
	}

	// Sum of squares from each vertex to its opposite vertex.
	// (For pentagons, arbitrarily use the second vertex of the opposite pair.)
	double ssOppArray[3];
	for (int n = 0; n != 3; ++n)
	{
		double xd = xyVertexArray[n].x() - xyVertexArray[(n + 3) % nVertexCount].x();
		double yd = xyVertexArray[n].y() - xyVertexArray[(n + 3) % nVertexCount].y();
		ssOppArray[n] = xd * xd + yd * yd;
	}

	// Decide whether each vertex is in the vicinity of the center.
	// A cohesive polygon has all vertices in the vicinity of the center.
	bool bCohesive = true;
	bool bVicinityArray[6];
	for (int n = 0; n != nVertexCount; ++n)
	{
		if (ssCenterArray[n] <= ssOppArray[n % 3])
		{
			bVicinityArray[n] = true;
		}
		else
		{
			bVicinityArray[n] = false;
			bCohesive = false;
		}
	}

	if (bSpansMeridian && bSpansDateline)
	{
		// Looks like we're at one of the poles.

		// Create rect.
		for (int n = 0; n != nVertexCount; ++n)
		{
			pRect1->expand(xyVertexArray[n]);
		}

		// Adjust x bounds.
		pRect1->setXMin(m_bounds.xMin());
		pRect1->setXMax(m_bounds.xMax());

		// Adjust y bounds.
		if (pRect1->yMin() > 0)
		{
			// North Pole.
			pRect1->setYMax(m_bounds.yMax());
		}
		else
		{
			// South Pole.
			pRect1->setYMin(m_bounds.yMin());
		}
	}
	else
	{
		// Create rect(s).
		for (int n = 0; n != nVertexCount; ++n)
		{
			if (bVicinityArray[n])
			{
				pRect1->expand(xyVertexArray[n]);
			}
			else
			{
				pRect2->expand(xyVertexArray[n]);
			}
		}

		if (!bCohesive)
		{
			// Adjust x bounds.
			if (xyCenter.x() < m_bounds.xMin() + kfHalfWidth)
			{
				// Center is at low x.
				pRect1->expandX(m_bounds.xMin());
				pRect2->expandX(m_bounds.xMax());
			}
			else
			{
				// Center is at high x.
				pRect2->expandX(m_bounds.xMin());
				pRect1->expandX(m_bounds.xMax());
			}

			// Adjust y bounds.
			pRect1->expandY(pRect2->yMin());
			pRect1->expandY(pRect2->yMax());
			pRect2->expandY(pRect1->yMin());
			pRect2->expandY(pRect1->yMax());
		}
	}

	// Clip rects to data source bounds.
	pRect1->clip(m_bounds);
	pRect2->clip(m_bounds);
}

/*!
Calculate the bounding rectangles that cover the specified PYXIS tile. Most
of the time only one rectangle will be returned (the other will be empty). For
global data sets, where the tile spans the international dateline, both
rectangles are returned.

\param	tile	The cell.
\param	pRect1	The first rectangle (out)
\param	pRect2	The second rectangle (out);
*/
void PYXOGRDataSource::calcTileBoundingRects(	const PYXTile& tile,
												PYXRect2DDouble* pRect1,
												PYXRect2DDouble* pRect2	) const
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	if (!tile.isEmpty())
	{
		calcCellBoundingRects(tile.getBoundingCell(), pRect1, pRect2);
	}
}

// Initialize the metadata for the data source.
void PYXOGRDataSource::initMetaData()
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	m_spDefn = PYXTableDefinition::create();
	m_spFeatDefn = PYXTableDefinition::create();

	// add the metadata for the data set
	addDataSetMetadata(this, m_pOGRDataSource, m_pOGRLayer, m_pOGRLayer->GetFeatureCount(FALSE));

	// clear any existing spatial filter.
	m_pOGRLayer->SetSpatialFilter(NULL);

	if (m_addExtentToMetadata)
	{
		OGREnvelope fExt;
		OGRErr ogrERR = m_pOGRLayer->GetExtent(&fExt, TRUE);
		correctOGREnvelope(&fExt);

		if (ogrERR != OGRERR_NONE)
		{
			fExt.MinX = -180.0;
			fExt.MinY = -90.0;
			fExt.MaxX = 180.0;
			fExt.MaxY = 90.0;
		}

		OGRFeatureDefn* poDefn = m_pOGRLayer->GetLayerDefn();

		addField(
			"layer name",
			PYXFieldDefinition::knContextNone,
			PYXValue::knString,
			1,
			PYXValue(std::string(poDefn->GetName()))	);

		addField(
			"geometry",
			PYXFieldDefinition::knContextNone,
			PYXValue::knString,
			1,
			PYXValue(std::string(OGRGeometryTypeToName(poDefn->GetGeomType())))	);

		addField(
			"extent min x",
			PYXFieldDefinition::knContextNone,
			PYXValue::knDouble,
			1,
			PYXValue(fExt.MinX)	);

		addField(
			"extent min y",
			PYXFieldDefinition::knContextNone,
			PYXValue::knDouble,
			1,
			PYXValue(fExt.MinY)	);

		addField(
			"extent max x",
			PYXFieldDefinition::knContextNone,
			PYXValue::knDouble,
			1,
			PYXValue(fExt.MaxX)	);

		addField(
			"extent max y",
			PYXFieldDefinition::knContextNone,
			PYXValue::knDouble,
			1,
			PYXValue(fExt.MaxY)	);
	}

	// get feature description (ie. names and types of attributes for the features).
	// DOES NOT CONTAIN VALUES.  GOTO THE FEATURE FOR THIS.
	addContentDefinition(m_pOGRLayer, getFeatureDefinition());
}

/*!
Add the metadata for the data set.

\param pRecord		The record associated with the data set
\param pGDALDataSet	The GDAL data set
\param pLayer		The OGR layer (ownership retained by caller)
\param nFeatures	The number of features in the layer or zero if not known
*/
void PYXOGRDataSource::addDataSetMetadata(
	IRecord* pRecord,
	PYXPointer<PYXSharedGDALDataSet> pGDALDataSet,
	OGRLayer* pLayer,
	GIntBig nFeatures)
{
	char** papszMetadata;

	// add data set metadata
	papszMetadata = pGDALDataSet->get()->GetMetadata(NULL);
	if (CSLCount(papszMetadata) > 0)
	{
		for (auto i = 0; papszMetadata[i] != NULL; i++)
		{
			char* pszKey;
			const char* pszValue = CPLParseNameValue(papszMetadata[i], &pszKey);

			// add name value pair to data set
			pRecord->addField(
				pszKey,
				PYXFieldDefinition::knContextNone,
				PYXValue::knString,
				1,
				PYXValue(pszValue));
		}
	}

	// add layer metadata
	if (nullptr != pLayer)
	{
		papszMetadata = pLayer->GetMetadata(NULL);
		if (CSLCount(papszMetadata) > 0)
		{
			for (auto i = 0; papszMetadata[i] != NULL; i++)
			{
				char* pszKey;
				const char* pszValue = CPLParseNameValue(papszMetadata[i], &pszKey);

				// add name value pair to data set
				pRecord->addField(
					pszKey,
					PYXFieldDefinition::knContextNone,
					PYXValue::knString,
					1,
					PYXValue(pszValue));

				CPLFree(pszKey);
			}
		}

		// record the number of features in the layer
		if (nFeatures > 0)
		{
			pRecord->addField(
				PYXDataSet::s_strPyxisFeatureCount,
				PYXFieldDefinition::knContextNone,
				PYXValue::knDouble,
				1,
				PYXValue((double) nFeatures));
		}
	}
}

/*!
Add the definition for the data set's content.

\param pOGRLayer	The OGR layer.
\param pTableDefn	The table definition.
*/
void PYXOGRDataSource::addContentDefinition(OGRLayer* pOGRLayer, PYXPointer<PYXTableDefinition> pTableDefn)
{
	assert(nullptr != pOGRLayer);
	assert(nullptr != pTableDefn);

	int nFieldCount = pOGRLayer->GetLayerDefn()->GetFieldCount();
	for (int nField = 0; nField < nFieldCount; ++nField)
	{
		std::string strName = pOGRLayer->GetLayerDefn()->GetFieldDefn(nField)->GetNameRef();

		// ensure that the field names are unique
		if (pTableDefn->getFieldIndex(strName) != -1)
		{
			int nFieldVersion = 2;
			std::string strNewName;
			do
			{
				strNewName = strName + " " + StringUtils::toString(nFieldVersion);
				++nFieldVersion;
			}
			while (pTableDefn->getFieldIndex(strNewName) != -1);
			strName = strNewName;
		}

		OGRFieldType nOGRType = pOGRLayer->GetLayerDefn()->GetFieldDefn(nField)->GetType();
		PYXValue::eType nType = PYXOGRFeature::convertOGRToPYXDataType(nOGRType);
		pTableDefn->addFieldDefinition(
			strName,
			PYXFieldDefinition::knContextNone,
			nType,
			1	);
	}
}

/*!
Constructor initializes member variables.

\param	spsetFeature	The set of features to iterate over (ownership shared)
\param	spDefn		The feature definition (ownership shared with caller)
\param	nResolution	Resolution at which to generate the feature's geometry.
*/
PYXOGRDataSource::PYXOGRFeatureIterator::PYXOGRFeatureIterator(
	boost::shared_ptr<FeatureSet> spsetFeature,
	PYXPointer<const PYXTableDefinition> spDefn,
	int nResolution	) :
	m_spsetFeature(spsetFeature),
	m_spDefn(spDefn),
	m_nResolution(nResolution)
{
	assert(0 != spsetFeature.get());

	m_it = m_spsetFeature->begin();
	m_itEnd = m_spsetFeature->end();

	if (!end())
	{
		m_spFeature = *m_it;
	}
}

/*!
Destructor cleans up memory.  (if necessary)
*/
PYXOGRDataSource::PYXOGRFeatureIterator::~PYXOGRFeatureIterator()
{
}

/*!
Move to the next feature.
*/
void PYXOGRDataSource::PYXOGRFeatureIterator::next()
{
	m_spFeature = 0;

	if (!end())
	{
		++m_it;

		if (!end())
		{
			m_spFeature = *m_it;
		}
	}
}

/*!
See if we have covered all the feature.

\return	true if all the features have been covered, otherwise false.
*/
bool PYXOGRDataSource::PYXOGRFeatureIterator::end() const
{
	return (m_it == m_itEnd);
}

/*!
Get the current PYXIS feature.

\return	The current feature (ownership retained) or 0 if iteration is complete.
*/
boost::intrusive_ptr<IFeature>
PYXOGRDataSource::PYXOGRFeatureIterator::getFeature() const
{
	return m_spFeature;
}

/*!
Create the PYXIS geometry for the data source.

\param	nResolution	The resolution for the geometry.
*/
void PYXOGRDataSource::createGeometry(int nResolution)
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	OGREnvelope fExt;

	std::auto_ptr<PYXConstWireBuffer> buffer;

	if (m_localStorage)
	{
		buffer = m_localStorage->get("ogr:extent");
	}

	if (buffer.get() != 0)
	{
		*buffer >> fExt.MinX >> fExt.MinY >> fExt.MaxX >> fExt.MaxY;
	}
	else
	{
		OGRErr m = m_pOGRLayer->GetExtent(&fExt, TRUE);
		correctOGREnvelope(&fExt);

		PYXStringWireBuffer newBuffer;
		newBuffer << fExt.MinX << fExt.MinY << fExt.MaxX << fExt.MaxY;

		m_localStorage->set("ogr:extent", newBuffer);
	}

	m_bounds.setXMin(fExt.MinX);
	m_bounds.setXMax(fExt.MaxX);
	m_bounds.setYMin(fExt.MinY);
	m_bounds.setYMax(fExt.MaxY);

	if (m_bounds.degenerate())
	{
		PYXTHROW(MissingGeometryException, "Geometry is empty.");
	}

	assert(m_spCoordConverter);

	m_spGeometry = PYXXYBoundsGeometry::create(m_bounds, *m_spCoordConverter, nResolution);
}

/*!
Get a particular feature.

\param strID	The id of the feature.

\return Shared pointer to feature or empty shared pointer if not found.
*/
boost::intrusive_ptr<IFeature>
PYXOGRDataSource::getFeature(const std::string& strID) const
{
	return getFeature(atoi(strID.c_str()));
}

/*!
Get a particular feature.

\param nID	The id of the feature.

\return Shared pointer to feature or empty shared pointer if not found.
*/
boost::intrusive_ptr<OGRFeatureObject>
PYXOGRDataSource::getOGRFeature(int nID) const
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	if (m_bReadingLock)
	{
		PYXTHROW(PYXException,"Iterator is using the OGCLayer at the moment - can't use GetFeature while reading the file");
	}

	return OGRFeatureObject::create(m_pOGRLayer->GetFeature(nID));
}

boost::intrusive_ptr<IFeature>
PYXOGRDataSource::getFeature(int nID) const
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

	PYXPointer<OGRFeatureObject> pOGRFeature = getOGRFeature(nID);

	boost::intrusive_ptr<IFeature> spFeature;

	if (pOGRFeature)
	{
		spFeature = new PYXOGRFeature(	pOGRFeature,
										this,
										getFeatureDefinition(),
										*m_spCoordConverter,
										getResolution(),	
										m_strStyle);
	}

	return spFeature;
}

/*!
Constructor initializes member variables.

\param	spsetFeature	The set of features to iterate over (ownership shared)
\param	spDefn		The feature definition (ownership shared with caller)
\param	nResolution	Resolution at which to generate the feature's geometry.
*/
PYXOGRDataSource::PYXOGRFeatureLazyIterator::PYXOGRFeatureLazyIterator(
	boost::shared_ptr<PYXrTree::KeyList> psetFeature,
	boost::intrusive_ptr<const PYXOGRDataSource> spDataSource,
	PYXPointer<const PYXTableDefinition> spDefn,
	int nResolution	) :
	m_spsetFeature(psetFeature),
	m_spDataSource(spDataSource),
	m_spDefn(spDefn),
	m_nResolution(nResolution)
{
	assert(0 != psetFeature.get());

	m_it = m_spsetFeature->begin();
	m_itEnd = m_spsetFeature->end();

	if (!end())
	{
		createFeature();
	}
}

/*!
Destructor cleans up memory.  (if necessary)
*/
PYXOGRDataSource::PYXOGRFeatureLazyIterator::~PYXOGRFeatureLazyIterator()
{
}

/*!
Move to the next feature.
*/
void PYXOGRDataSource::PYXOGRFeatureLazyIterator::next()
{
	m_spFeature = 0;

	if (!end())
	{
		++m_it;

		if (!end())
		{
			createFeature();
		}		
	}
}

/*!
See if we have covered all the feature.

\return	true if all the features have been covered, otherwise false.
*/
bool PYXOGRDataSource::PYXOGRFeatureLazyIterator::end() const
{
	return (m_it == m_itEnd);
}

/*!
Get the current PYXIS feature.

\return	The current feature (ownership retained) or 0 if iteration is complete.
*/
boost::intrusive_ptr<IFeature>
PYXOGRDataSource::PYXOGRFeatureLazyIterator::getFeature() const
{
	return m_spFeature;
}


void PYXOGRDataSource::PYXOGRFeatureLazyIterator::createFeature() 
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);	

	long nID = reinterpret_cast<long>(*m_it);
	
	m_spFeature	= new PYXOGRFeature(m_spDataSource->getOGRFeature(nID),
									m_spDataSource,
									m_spDefn.get(),
									*(m_spDataSource->m_spCoordConverter),
									m_nResolution,
									m_spDataSource->m_strStyle);	
}

/*!
Constructor initializes member variables.

\param	spsetFeature	The set of features to iterate over (ownership shared)
\param	spDefn		The feature definition (ownership shared with caller)
\param	nResolution	Resolution at which to generate the feature's geometry.
*/
PYXOGRDataSource::PYXOGRAllFeaturesNativeIterator::PYXOGRAllFeaturesNativeIterator(	
	boost::intrusive_ptr<PYXOGRDataSource> spDataSource,
	PYXPointer<const PYXTableDefinition> spDefn,
	int nResolution	) :	
	m_spDataSource(spDataSource),
	m_spDefn(spDefn),
	m_nResolution(nResolution)
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);	

	m_spDataSource->setReadingLock(true);
	m_spDataSource->m_pOGRLayer->ResetReading();	

	createFeature();	
}

/*!
Destructor cleans up memory.  (if necessary)
*/
PYXOGRDataSource::PYXOGRAllFeaturesNativeIterator::~PYXOGRAllFeaturesNativeIterator()
{
	m_spDataSource->setReadingLock(false);
}

/*!
Move to the next feature.
*/
void PYXOGRDataSource::PYXOGRAllFeaturesNativeIterator::next()
{
	if (!end())
	{
		createFeature();	
	}
}

/*!
See if we have covered all the feature.

\return	true if all the features have been covered, otherwise false.
*/
bool PYXOGRDataSource::PYXOGRAllFeaturesNativeIterator::end() const
{
	return !m_spFeature;
}

/*!
Get the current PYXIS feature.

\return	The current feature (ownership retained) or 0 if iteration is complete.
*/
boost::intrusive_ptr<IFeature>
PYXOGRDataSource::PYXOGRAllFeaturesNativeIterator::getFeature() const
{
	return m_spFeature;
}


void PYXOGRDataSource::PYXOGRAllFeaturesNativeIterator::createFeature() 
{
	boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);	

	m_spFeature.reset();

	OGRFeature* pOGRFeature = nullptr;

	while(pOGRFeature == nullptr)
	{
		pOGRFeature = m_spDataSource->m_pOGRLayer->GetNextFeature();
		if (pOGRFeature == 0)
		{
			return;
		}

		OGRGeometry* pOGRGeometry = pOGRFeature->GetGeometryRef();

		//skip feature if no geometry was found
		if (pOGRGeometry == nullptr) 
		{
			TRACE_INFO("Warning, skipping feature with no geometry. FeatureID=" << pOGRFeature->GetFID() );
			OGRFeature::DestroyFeature(pOGRFeature);
			pOGRFeature = nullptr;
		}
	}

	m_spFeature	= new PYXOGRFeature(OGRFeatureObject::create(pOGRFeature),
									m_spDataSource,
									m_spDefn.get(),
									*(m_spDataSource->m_spCoordConverter),
									m_nResolution,
									m_spDataSource->m_strStyle);	
}
