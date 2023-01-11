/******************************************************************************
wms_multi_data_source.cpp

begin		: 2004-10-15
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.comonver

******************************************************************************/

#define MODULE_GDAL_SOURCE
#include "wms_multi_data_source.h"

// local includes
#include "pyxis/data/exceptions.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/exception.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/geometry/curve.h"
#include "pyxis/utility/value.h"
#include "pyxis/sampling/xy_bounds_geometry.h"
#include "pyxis/sampling/xy_iterator.h"
#include "pyxis/derm/reference_sphere.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/utility/tester.h"
#include "pyxis/derm/wgs84.h"
#include "wms_data_source.h"

//#include "pyx_data_item.h"

#include <boost/shared_ptr.hpp>

// standard includes
#include <cassert>
#include <io.h>
#include <string>
#include <istream>

//! Tester class
TesterUnit<WMSMultiDataSource> gMultiReaderTester;

//! Counter incremented each time a WMSMultiDataSource is opened to keep pipe names unique.
int WMSMultiDataSource::m_nCounter = 0;

//! Height and width in pixels of images to download from the WMS server.
const int WMSMultiDataSource::knImagePixels = 512;

//! Number of resolutions to lower for the "low res" download.
const int knLowResLOD = 1;

//! Mutex to serialize concurrent access by multiple threads
boost::recursive_mutex WMSMultiDataSource::m_staticMutex;

std::map<std::string, std::vector<PYXPointer<PYXGeometry> > > WMSMultiDataSource::m_geometryMap;


double getWidthInDegrees (int nLod)
{
	double fWidthInDegrees = 360.0;

	double fFactor = 1.0; //1.0; //2.0;

	switch (nLod)
	{
	case 1:
		return 90.0/fFactor;
	case 2:
		return 60.0/fFactor;
	case 3:
		return 30.0/fFactor;
	case 4:
		return 15.0/fFactor;
	case 5:
		return 2.0/fFactor;
	case 6:
		return 0.5/fFactor;
	case 7:
		return 0.1/fFactor;
	case 8:
		return 0.05/fFactor;
	case 9:
		return 0.01/fFactor;
	case 10:
		return 0.005/fFactor;
	case 11:
		return 0.001/fFactor;
	default:
		return 0.001/fFactor;
	}
}

void addMessage(const char* pWhere, const char* pWhat, const char* pOther = 0)
{
	//! Mutex to serialize concurrent access by multiple threads
	static boost::recursive_mutex staticMutex;

	boost::recursive_mutex::scoped_lock lock(staticMutex);

	std::ofstream ofs;
	ofs.open("c:\\wmsmultidatasourcelog.txt", std::ios::app);

	if (pOther == 0)
	{
		ofs << GetTickCount() << " " << pWhere << " " << pWhat << std::endl;
	}
	else
	{
		ofs << GetTickCount() << " " << pWhere << " " << pWhat << " " << pOther << std::endl; 
	}

	ofs.close(); 

}

void WMSMultiDataSource::getInvalidGeometry(std::map<std::string, std::vector<PYXPointer<PYXGeometry> > >& mapGeometry)
{
	boost::recursive_mutex::scoped_lock lock(m_staticMutex);

	std::map<std::string, std::vector<PYXPointer<PYXGeometry> > >::iterator it = m_geometryMap.begin();

	while (it != m_geometryMap.end())
	{
		mapGeometry[(*it).first] = (*it).second;
		++it;
	}

	m_geometryMap.clear();
}

void WMSMultiDataSource::addInvalidGeometry(std::string& strURI, PYXPointer<PYXGeometry> spGeometry)
{
	boost::recursive_mutex::scoped_lock lock(m_staticMutex);

	m_geometryMap[strURI].push_back(spGeometry);
}



//! Test method
void WMSMultiDataSource::test()
{

#if 0

	PYXSpatialReferenceSystem spatialReference;

	spatialReference.setSystem(PYXSpatialReferenceSystem::knSystemProjected);
	spatialReference.setDatum(PYXSpatialReferenceSystem::knDatumWGS84);
	spatialReference.setProjection(PYXSpatialReferenceSystem::knProjectionUTM );
	spatialReference.setZone(19);
	
	GDALMultiDataSource reader;

	std::string strDirName = "c:\\data\\ortho\\quebec_32_colour";

	try
	{
		reader.open(strDirName, "tif", spatialReference);
	}
	catch (PYXException& e)
	{
		return;
	}

						
	PYXIcosIndex index;

	index = "3-20406050060010100000300204";

/*	const void* pValue = reader.getValue(index);

	int nCount = 0;
	
	for (PYXPointer<PYXIterator> spIt(reader.getIterator()); !spIt->end(); spIt->next())
	{
		pValue = spIt->getFieldValue();
		if (0 != pValue)
		{
			int nValue = *((int*) pValue);
		}

		nCount++;

		if (nCount > 100)
		{
			break;
		}
	}
*/
#endif

}

/*!
Constructor initializes member variables.
*/
WMSMultiDataSource::WMSMultiDataSource() :
	m_bounds(),
		m_nCurrentLOD(-1),
		m_nMinLOD(-1),
		m_nMaxLOD(-1),
		m_pThreadDataSource(0),
		m_bTerminate(false),
		m_fGridFactor (0.0),
		m_bElevationDataSource(false),
		m_nLod(1),
		m_strTerminator("|"),
		m_hPlugIn(0),
		m_bClearDownloads(false)
{
	setType(PYXDataSource::knRaster);

	m_bounds.setXMin(-180.0);
	m_bounds.setXMax(180.0);

	m_bounds.setYMin(-90.0);
	m_bounds.setYMax(90.0);

	// create the spatial reference system
	PYXPointer<PYXSpatialReferenceSystem> spSRS(PYXSpatialReferenceSystem::create());
	
	//TODO: Deal with the SRS issue later. CHOWELL
	spSRS->setSystem(PYXSpatialReferenceSystem::knSystemGeographical);
	spSRS->setDatum(PYXSpatialReferenceSystem::knDatumWGS84);
	spSRS->setProjection(PYXSpatialReferenceSystem::knProjectionNone);

	// Picked an arbitrary zone for initialization.
	spSRS->setZone(1);	

	m_coordConverter.initialize(*(spSRS.get()));

}

//! Constructor.
WMSMultiDataSource::WMSMultiDataSource(WMSMultiDataSource* pDataSource)
:	m_pThreadDataSource(pDataSource),
	m_hPlugIn(0)
{
}

/*!
Boost threads create a copy of this object when a new thread is spawned.
This requires a copy constructor.

\param wmsDataSource	The object to initialize the constructed copy with.
*/
WMSMultiDataSource::WMSMultiDataSource(const WMSMultiDataSource& wmsDataSource)
:	m_pThreadDataSource(wmsDataSource.m_pThreadDataSource),
	m_hPlugIn(0)
{
}


/*!
Get the geometry

\return shared pointer to the geometry.
*/
PYXPointer<const PYXGeometry> WMSMultiDataSource::getGeometry() const
{
	PYXPointer<PYXCurve> spCurve = PYXCurve::create();

	spCurve->addNode(PYXIcosIndex("3-200000000000000000000000000000000000000000000000"));

	return spCurve;
}

/*!
Destructor cleans up memory.
*/
WMSMultiDataSource::~WMSMultiDataSource()
{
	m_bTerminate = true;

	m_serverPipe.terminate();

	if (m_hPlugIn != 0)
	{
		// Remove any pending requests for download from server before closing.
		std::string strRequest("removerequests");
		strRequest.append(m_strTerminator);

		strRequest.append(m_strPipeName);
		strRequest.append(m_strTerminator);

		PYXNamedPipe serverPipe(m_strServerPipeName);

		std::string strResponse;

		serverPipe.callNamedPipe(strRequest, strResponse);

		m_pDLLDeleteInstance(m_nInstance);
		FreeLibrary(m_hPlugIn);
	}

	if (m_spPipeThread.get() != 0)
	{
		//m_processPipeMessagesSemaphore.signal();

		// Terminate our thread.
		m_spPipeThread->join();

		m_spPipeThread.reset();
	}
}

/*!
Open a WMS multi data set.  

There are two ways that failure can occur:
1. It is not a valid WMS multi data set, and
2. It is a valid WMS multi data set but it had some problem opening it.

If it is not a valid WMS multi data set, then the method will return false.
If it is valid but failed to open it, then an exception will be thrown.
If it is valid and was successfully opened, then the method will return true.

\param	strFileName			The file to open.
\param	bCreateGeometry		true to create the geometry, otherwise false

\returns false if it is not a valid WMS multi data set, 
			true if it opens successfully.
*/
bool WMSMultiDataSource::open(	const std::string& strDir,
								const std::string& strFileExt)
{

	if (strDir.rfind(".wms") == std::string::npos)
	{
		return false;
	}

	// save the name of the data source
	setName(strDir);

	m_nCurrentLOD = -1;

	std::ifstream ifs;

	ifs.open(strDir.c_str());

	/*
	Data in file is in following order:

	Server URL
	server path
	Layer Name
	Style
	Min Lon Value
	Min Lat Value
	Max Lon value
	Max Lat Value
	Min LOD
	Max LOD
	Number of Bands
	Interpretation
	*/

	if (ifs && !ifs.eof())
	{
		std::string strBuffer;

		std::getline(ifs, m_strHost);
		std::getline(ifs, m_strHostPath);
		std::getline(ifs, m_strLayer);
		std::getline(ifs, m_strStyle);
		std::getline(ifs, m_strFormat);

		std::getline(ifs, strBuffer);
		double fMinLon = atof(strBuffer.c_str());

		std::getline(ifs, strBuffer);
		double fMinLat = atof(strBuffer.c_str());

		std::getline(ifs, strBuffer);
		double fMaxLon = atof(strBuffer.c_str());

		std::getline(ifs, strBuffer);
		double fMaxLat = atof(strBuffer.c_str());

		std::getline(ifs, strBuffer);
		m_nMinLOD = atoi(strBuffer.c_str());

		std::getline(ifs, strBuffer);
		m_nMaxLOD = atoi(strBuffer.c_str());

		PYXValue::eType nFieldType = PYXValue::knInt32;

		std::getline(ifs, strBuffer);
		std::string strType(strBuffer);

		std::getline(ifs, strBuffer);

		int nNumBands = atoi(strBuffer.c_str());

		if (nNumBands == 2)
		{
			PYXValue::eType nFieldType = PYXValue::knInt16;
			nNumBands = 1;
		}

		if (strType == "raster")
		{
			setType(knRaster);
			getCoverageDefinition()->addFieldDefinition(
			"1",
			PYXFieldDefinition::knContextRGB,
			nFieldType,
			nNumBands );
		}

		if (strType == "elevation")
		{
			m_bElevationDataSource = true;
			setType(knDEM);
			getCoverageDefinition()->addFieldDefinition(
			"1",
			PYXFieldDefinition::knContextElevation,
			nFieldType,
			nNumBands );
		}

		m_bounds.setXMin(fMinLon);
		m_bounds.setXMax(fMaxLon);
	
		m_bounds.setYMin(fMinLat);
		m_bounds.setYMax(fMaxLat);

		ifs.close();
	}

	m_pLastDS = 0;

	initMetaData();

	// TODO we're building using UNICODE settings hence string literal
	m_hPlugIn = LoadLibrary(L"WMSClientDLL.dll");

	// Did the DLL load?
	if (m_hPlugIn != 0)
	{
		// Get create instance function.
		m_pDLLMakeNewInstance = (DLLMAKENEWINSTANCE)GetProcAddress(m_hPlugIn, "DLLMakeNewInstance");

		// Get delete instance function.
		m_pDLLDeleteInstance = (DLLDELETEINSTANCE)GetProcAddress(m_hPlugIn, "DLLDeleteInstance");

		// Get pipe name function.
		 m_pDLLGetPipeName = (DLLGETPIPENAME)GetProcAddress(m_hPlugIn, "DLLGetPipeName");

		if (m_pDLLMakeNewInstance != 0 && m_pDLLDeleteInstance != 0 && m_pDLLGetPipeName != 0)
		{
			m_nInstance = m_pDLLMakeNewInstance();

			char szServerPipeName[100];

			m_pDLLGetPipeName(m_nInstance, szServerPipeName, 99);

			m_strServerPipeName = szServerPipeName;

		}
		else
		{
			FreeLibrary(m_hPlugIn);
		}
	}

	m_strPipeName = "\\\\.\\pipe\\WMSDataSourceClientPipe";

	// Increment counter.
	++m_nCounter;

	char buffer[100];

	_itoa_s(m_nCounter, buffer, 99, 10);

	m_strPipeName.append(buffer);

	m_serverPipe.setPipeName(m_strPipeName);


	//std::string strMessage;
	//std::string strResponse;

	//strMessage = "0getcapabilities";
	//strMessage.append(m_strTerminator);
	//strMessage.append("UnusedPipeName");
	//strMessage.append(m_strTerminator);
	//strMessage.append(m_strHost);
	//strMessage.append(m_strTerminator);
	//strMessage.append(m_strHostPath);
	//strMessage.append(m_strTerminator);

	//PYXNamedPipe wmsClientPipe(m_strServerPipeName);

	//// Get capabilities of server
	//wmsClientPipe.callNamedPipe(strMessage, strResponse);


	// create a new thread to update the tiles
	WMSMultiDataSource  pipeThread(this);

	m_spPipeThread.reset(new boost::thread(pipeThread));

	return true;
}

/*!
Get the spatial precision of the data.

\return	The spatial precision in metres or -1.0 if not known.
*/
double WMSMultiDataSource::getSpatialPrecision() const
{
	return -1.0;
}
	

/*!
Initialize metadata
*/
void WMSMultiDataSource::initMetaData()
{
	setType(knRaster);

	addField(
		"longitude min",
		PYXFieldDefinition::knContextNone,
		PYXValue::knDouble,
		1,
		PYXValue(m_bounds.xMin())	);

	addField(
		"longitude max",
		PYXFieldDefinition::knContextNone,
		PYXValue::knDouble,
		1,
		PYXValue(m_bounds.xMax())	);

	addField(
		"latitude min",
		PYXFieldDefinition::knContextNone,
		PYXValue::knDouble,
		1,
		PYXValue(m_bounds.yMin())	);

	addField(
		"latitude max",
		PYXFieldDefinition::knContextNone,
		PYXValue::knDouble,
		1,
		PYXValue(m_bounds.yMax())	);

	addField(
		"description",
		PYXFieldDefinition::knContextNone,
		PYXValue::knString,
		1,
		PYXValue(m_strDescription));

	addField(
		"host",
		PYXFieldDefinition::knContextNone,
		PYXValue::knString,
		1,
		PYXValue(m_strHost));

	addField(
		"layer",
		PYXFieldDefinition::knContextNone,
		PYXValue::knString,
		1,
		PYXValue(m_strLayer));

		addField(
		"style",
		PYXFieldDefinition::knContextNone,
		PYXValue::knString,
		1,
		PYXValue(m_strStyle));

	addField(
		"format",
		PYXFieldDefinition::knContextNone,
		PYXValue::knString,
		1,
		PYXValue(m_strFormat));

}

/*!
Call this method after the data source is opened to determine if the data
source contains a spatial reference. If not, a spatial reference must be
supplied by calling setSpatialReference() before getFeatureIterator() is
called. Native PYXIS data sources always return true.

\return	true if the data source has a spatial reference, otherwise false
*/
bool WMSMultiDataSource::hasSpatialReferenceSystem() const
{
	return true;
}

/*!
Specify the spatial reference for the data source. Call this method to set the
spatial reference if after the data source is opened hasSpatialReference()
returns false.

\param	spSRS	The spatial reference system.
*/
void WMSMultiDataSource::setSpatialReferenceSystem(PYXPointer<PYXSpatialReferenceSystem> spSRS)
{
}

/*!
Create a GDAL data source but do not open it.

\param	strFileName			The name of the data source.  Empty if not to open it.
\param  nLod				Level of detail of this file.
\param  nLonMin				Minimum longitude for this entry.
\param  nLatMin				Minimum latitude for this entry.
\param  nLonMax				Maximum longitude for this entry.
\param  nLatMax				Maximum latitude for this entry.
\param bInvalidate		I don't know what this does.

\return	true if successfull, false otherwise.
*/
bool WMSMultiDataSource::openFile(const std::string& strFileName,
								  int nLod,
								  int nLonMin, int nLatMin,
								  int nLonMax, int nLatMax,
								  bool bInvalidate) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	if (nLod != m_nCurrentLOD)
	{
		return false;
	}

	// create the data source
	PYXPointer<WMSDataSource> spDataSource;

	// Coordinate of bottom left corner of grid square.
	PYXCoord2DInt pt(nLonMin, nLatMin); //fLonMin, fLatMin);

	if (strFileName.empty())
	{
		// Add empty entry to our map.
		if (nLod == m_nCurrentLOD)
		{
		//	spDataSource.reset(new WMSDataSource);
			spDataSource = WMSDataSource::create();		
			m_mapDataSource[pt] = spDataSource;
		}

		//if (nLod == m_nCurrentLOD - knLowResLOD)
		//{
		//	spDataSource.reset(new WMSDataSource);
		//	m_mapLowDataSource[pt] = spDataSource;
		//}
	}
	else
	{
		// If file is for the current LOD and not already in map, or in map as zero (new data source) - add it and open it.
		if (nLod == m_nCurrentLOD)
		{
			WMSDataSourceMap::iterator it = m_mapDataSource.find(pt);
	
			 if (it == m_mapDataSource.end())
			 {	
				 spDataSource = WMSDataSource::create();
				// Store the reader based on its geodetic southwest coordinate.
				m_mapDataSource[pt] = spDataSource;
			 }
			 else
			 {
				 spDataSource = m_mapDataSource[pt];
			 }
		}
		//else
		//{
		//	if (nLod == m_nCurrentLOD - knLowResLOD)
		//	{
		//		WMSDataSourceMap::iterator it = m_mapLowDataSource.find(pt);
		//
		//		 if (it == m_mapLowDataSource.end())
		//		 {	
		//			 spDataSource.reset(new WMSDataSource);
		//			// Store the reader based on its geodetic southwest coordinate.
		//			m_mapLowDataSource[pt] = spDataSource;
		//		 }
	 //			 else
		//		 {
		//			 spDataSource = m_mapLowDataSource[pt];
		//		 }
		//	}
		//	else
		//	{
		//		return false;
		//	}
		//}

		// open the data source.  
		if (!spDataSource->open(strFileName))
		{
			// if the data set is not a GDAL data set then false.
			spDataSource->setOpenState(3); // Error state.
			return false;
		}

		// If we downloaded a "low res large image", search all small images that overlap this one and are zero and invalidate them.
		//if (nLod == m_nCurrentLOD - knLowResLOD)
		//{
		//	PYXRect2DInt dsBounds;

		//	int nDSWidth = int(getWidthInDegrees(nLod) * 100000.0);

		//	int nWidth = int(getWidthInDegrees(m_nCurrentLOD) * 100000.0);

		//	dsBounds.setXMin(nLonMin);
		//	dsBounds.setXMax(nLonMin + nDSWidth);

		//	dsBounds.setYMin(nLatMin);
		//	dsBounds.setYMax(nLatMin + nDSWidth);

		//	WMSDataSourceMap::iterator it = m_mapDataSource.begin();

		//	while (it != m_mapDataSource.end())
		//	{
		//		if ((*it).second->getOpenState() == 0 )
		//		{
		//			PYXRect2DInt bounds;

		//			bounds.setXMin((*it).first.x());
		//			bounds.setXMax((*it).first.x() + nWidth);

		//			bounds.setYMin((*it).first.y());
		//			bounds.setYMax((*it).first.y() + nWidth);

		//			// Do we intersect the larger low res image?
		//			if (bounds.intersects(dsBounds))
		//			{
		//				PYXRect2DDouble fBounds;

		//				fBounds.setXMin(double(bounds.xMin()) / 100000.0);
		//				fBounds.setXMax(double(bounds.xMax()) / 100000.0);

		//				fBounds.setYMin(double(bounds.yMin()) / 100000.0);
		//				fBounds.setYMax(double(bounds.yMax()) / 100000.0);

		//				if (bInvalidate)
		//				{
		//					boost::shared_ptr<PYXGeometry> spBoundsGeometry(new PYXXYBoundsGeometry(fBounds, getCoordConverter(), m_nCurrentLOD*3));
		//					addInvalidGeometry(getName(), spBoundsGeometry);
		//				}
		//			}
		//		}
		//		++it;
		//	}
		//}
		//else
		{
			if (bInvalidate)
			{
				//PYXPoitner<PYXGeometry> spBoundsGeometry(new PYXXYBoundsGeometry(spDataSource->getBounds(), getCoordConverter(), nLod*3));
				PYXPointer<PYXGeometry> spBoundsGeometry = PYXXYBoundsGeometry::create(spDataSource->getBounds(),getCoordConverter(),nLod*3);
				addInvalidGeometry(getName(), spBoundsGeometry);
			}
		}
	}

	return true;
}


/*!
Get the data source which contains information for a particular native
coordinate.

\param	native	The native coordinate.

\return	the GDALDataSource.
*/
const WMSDataSource* WMSMultiDataSource::getDataSource(const PYXCoord2DDouble& native) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	// If current and requested LOD are different - notify server to cancel all previous downloads and reset our internal data.
	if (m_nLod != m_nCurrentLOD || m_bClearDownloads)
	{
		std::string strRequest("removerequests");
		strRequest.append(m_strTerminator);

		strRequest.append(m_strPipeName);
		strRequest.append(m_strTerminator);

		PYXNamedPipe serverPipe(m_strServerPipeName);

		std::string strResponse;

		serverPipe.callNamedPipe(strRequest, strResponse);

		m_nCurrentLOD = m_nLod;

		if (m_bClearDownloads)
		{
			// We panned - remove all data files not yet downloaded.
			WMSDataSourceMap::iterator it = m_mapDataSource.begin();
			WMSDataSourceMap::iterator itn = it;

			while(itn != m_mapDataSource.end())
			{
				++itn;
				if ((*it).second->getOpenState() != 1)
				{
					m_mapDataSource.erase(it);
				}
				it = itn;
			}
			
			// We panned - remove all data files not yet downloaded.
			//it = m_mapLowDataSource.begin();
			//itn = it;

			//while(it != m_mapLowDataSource.end())
			//{
			//	++itn;
			//	if ((*it).second->getOpenState() != 1)
			//	{
			//		m_mapLowDataSource.erase(it);
			//	}
			//	it = itn;
			//}

		}
		else
		{
			m_mapDataSource.clear();
//			m_mapLowDataSource.clear();
		}

		m_fGridFactor = getWidthInDegrees(m_nCurrentLOD);

		m_bClearDownloads = false;
	}

	double fLat = std::max(-90.0, (floor((native.y() / m_fGridFactor)) * m_fGridFactor));
	double fLon = std::max(-180.0,(floor((native.x() / m_fGridFactor)) * m_fGridFactor));

	fLat = std::min(90.0, fLat);
	fLon = std::min(180.0, fLon);

	int nNewLat = int(fLat * 100000.0);
	int nNewLon = int(fLon * 100000.0);

	// Coordinate of bottom left corner of grid square.
	PYXCoord2DInt pt(nNewLon, nNewLat); //fLonMin, fLatMin);

	WMSDataSourceMap::iterator it = m_mapDataSource.find(pt);

	WMSDataSource* pDS = 0;

	// If we don't have the specified file - request it from the server.
	if (it == m_mapDataSource.end() || (*it).second->getOpenState() == 0)
	{
		// Data source not available for current LOD.  Check for low res version if we have a lower LOD to load.
		//if (m_nCurrentLOD - knLowResLOD > 0)
		//{
		//	int nLod = m_nCurrentLOD - knLowResLOD;

		//	double fGridFactor = getWidthInDegrees(nLod);

		//	fLat = std::max(-90.0, (floor((native.y() / fGridFactor)) * fGridFactor));
		//	fLon = std::max(-180.0,(floor((native.x() / fGridFactor)) * fGridFactor));

		//	fLat = std::min(90.0, fLat);
		//	fLon = std::min(180.0, fLon);

		//	nNewLat = int(fLat * 100000.0);
		//	nNewLon = int(fLon * 100000.0);

		//	// Coordinate of bottom left corner of grid square.
		//	PYXCoord2DInt pt(nNewLon, nNewLat); //fLonMin, fLatMin);

		//	WMSDataSourceMap::iterator iter = m_mapLowDataSource.find(pt);

		//	if (iter != m_mapLowDataSource.end())
		//	{
		//		pDS = (*iter).second.get();
		//	}
		//	else
		//	{
		//		// Request low res version of file.
		//		requestDataSource(native, nLod, '5');
		//		it = m_mapLowDataSource.find(pt);
		//	}
		//}

		// Request current LOD of file.
		if (it == m_mapDataSource.end())
		{
			requestDataSource(native, m_nCurrentLOD, '0');
			it = m_mapDataSource.find(pt);
		}
	}

	if (it != m_mapDataSource.end()) 
	{
		// Get pointer to requested data (set to zero in requestDataSource for data being downloaded).
		if ((*it).second.get() != 0)
		{
			pDS = (*it).second.get();
		}
		//else
		//{
		//	// Get Low res version of data.
		//	if (m_nCurrentLOD - knLowResLOD > 0)
		//	{
		//		int nLod = m_nCurrentLOD - knLowResLOD;

		//		double fGridFactor = getWidthInDegrees(nLod);

		//		fLat = std::max(-90.0, (floor((native.y() / fGridFactor)) * fGridFactor));
		//		fLon = std::max(-180.0,(floor((native.x() / fGridFactor)) * fGridFactor));

		//		fLat = std::min(90.0, fLat);
		//		fLon = std::min(180.0, fLon);

		//		nNewLat = int(fLat * 100000.0);
		//		nNewLon = int(fLon * 100000.0);

		//		// Coordinate of bottom left corner of grid square.
		//		PYXCoord2DInt pt(nNewLon, nNewLat); //fLonMin, fLatMin);

		//		WMSDataSourceMap::iterator iter = m_mapLowDataSource.find(pt);

		//		if (iter != m_mapLowDataSource.end())
		//		{
		//			pDS = (*iter).second.get();
		//		}
		//	}
		//}
	}

	return pDS;
}

/*!
Get an iterator to all the points in the data source.

\return	The iterator (ownership transferred)
*/
PYXPointer<PYXXYIterator> WMSMultiDataSource::getXYIterator() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	return WMSMultiIterator::create(m_mapDataSource.begin(), m_mapDataSource.end());
}

/*!
\return The coordinate converter.
*/
const CoordConverterImpl* WMSMultiDataSource::getCoordConverter() const
{
	return &m_coordConverter;
}

/*!
Get value from data source.
Check size of elevation data sources - if more than 40000  return NULL value.

\param pDS	Pointer to data source to get value from.
\param native	Native coordinates of location to get value for.
*/
PYXValue WMSMultiDataSource::getDSValue(const WMSDataSource* pDS, const PYXCoord2DDouble& native) const
{
	if (pDS != 0)
	{
		PYXValue pyxValue = pDS->getCoverageValue(native);

		if (!m_bElevationDataSource)
		{
			return pyxValue;
		}

		if (pyxValue.getUInt16() < 40000)
		{
			return pyxValue;
		}
	}
	return PYXValue();
}

/*!
Request the data source that contained the specified point at the specified LOD.

\param native	Native coordinates (lat, lon) of point to be contained in the requested file.
\param nTheLod	Level of detail of image to be requested.
\param szPriority	Character representing download priority.  Larger values have priority over smaller ones - larger are downloaded first.
*/
void WMSMultiDataSource::requestDataSource(const PYXCoord2DDouble& native, int nTheLod, char szPriority) const
{
	// We do not have a file - ask the server for it.
	try
	{
		double fLon = native.x();
		double fLat = native.y();

		double fGridFactor = getWidthInDegrees(nTheLod);

		std::string strRequest;
		
		char szTemp[2];
		
		szTemp[0] = szPriority;
		szTemp[1] = 0;

		strRequest.append(szTemp);
		
		strRequest.append("getimage");
		strRequest.append(m_strTerminator);

		strRequest.append(m_strPipeName);
		strRequest.append(m_strTerminator);

		strRequest.append(m_strHost);
		strRequest.append(m_strTerminator);

		strRequest.append(m_strHostPath);
		strRequest.append(m_strTerminator);

		strRequest.append(m_strLayer);
		strRequest.append(m_strTerminator);

		strRequest.append(m_strStyle);
		strRequest.append(m_strTerminator);

		strRequest.append(m_strFormat);
		strRequest.append(m_strTerminator);

		std::stringstream strStreamPixels;
		strStreamPixels << knImagePixels << "\0";
		strRequest.append(strStreamPixels.str());
		strRequest.append(m_strTerminator);

		double fNewLat = std::max(-90.0, (floor((fLat / fGridFactor)) * fGridFactor));
		double fNewLon = std::max(-180.0,(floor((fLon / fGridFactor)) * fGridFactor));

		fNewLat = std::min(90.0, fNewLat);
		fNewLon = std::min(180.0, fNewLon);

		int nNewLat = int(fNewLat * 100000.0);
		int nNewLon = int(fNewLon * 100000.0);

		std::stringstream strStreamLat;
		strStreamLat << nNewLat << "\0";
		strRequest.append(strStreamLat.str());

		strRequest.append(m_strTerminator);

		std::stringstream strStreamLon;
		strStreamLon << nNewLon << "\0";
		strRequest.append(strStreamLon.str());

		strRequest.append(m_strTerminator);

		int nWidth = int(fGridFactor * 100000.0);

		std::stringstream strStreamWidth;
		strStreamWidth << nWidth << "\0";
		strRequest.append(strStreamWidth.str());

		strRequest.append(m_strTerminator);

		std::stringstream strStreamLod;
		strStreamLod << nTheLod << "\0";
		strRequest.append(strStreamLod.str());

		strRequest.append(m_strTerminator);

		PYXNamedPipe serverPipe(m_strServerPipeName);

		std::string strResponse;

		bool bResult = serverPipe.callNamedPipe(strRequest, strResponse);

		if (bResult)
		{
			if (strResponse.find("<download>") != std::string::npos)
			{
				std::stringstream strStream(strResponse);

				std::string strCmd;

				strStream >> strCmd;

				int nLatMin;
				int nLatMax;
				int nLonMin;
				int nLonMax;
				int nLod;

				strStream >> nLonMin;
				strStream >> nLatMin;
				strStream >> nLod;

				int nGridFactor = int(m_fGridFactor * 100000.0);

				nLonMax = nLonMin + nGridFactor;
				nLatMax = nLatMin + nGridFactor;

				// Create entry without opening data source.
				openFile(std::string(), nLod, nLonMin, nLatMin, nLonMax, nLatMax);
			}
			else
			{
				// If we have an error - create an empty data source for this item so we don't request it again.
				if (strResponse.find("<error>") != std::string::npos)
				{

					// Create entry without opening data source.
					openFile(std::string(), nTheLod, nNewLon, nNewLat, nNewLon + nWidth, nNewLat + nWidth);
				}
				else
				{
					if (strResponse.find("<file>") != std::string::npos)
					{
						std::stringstream strStream(strResponse.c_str());

						std::string strCmd;

						strStream >> strCmd;

						if (strCmd == "<file>")
						{
							int nLatMin;
							int nLonMin;
							int nLonMax;
							int nLatMax;
							int nLod;

							std::string strName;

							strStream >> nLonMin;
							strStream >> nLatMin;
							strStream >> nLod;
							strStream >> strName;

							int nGridFactor = int(m_fGridFactor * 100000.0);

							nLonMax = nLonMin + nGridFactor;
							nLatMax = nLatMin + nGridFactor;

							// Create entry and open data source now with no need for invalidating the geometry.
							openFile(strName, nLod, nLonMin, nLatMin, nLonMax, nLatMax, false);
						}
					}
				}
			}
		}
	}
	catch (...)
	{
	}
}

/*!
Get the coverage value at the specified native coordinate.

\param	native		The native coordinate.
\param	nFieldIndex	The field index (ignored).

\return	The value.
*/
PYXValue WMSMultiDataSource::getCoverageValue(
	const PYXCoord2DDouble& native,
	int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	const WMSDataSource* pDataSource;

	pDataSource = getDataSource(native);

	if (0 != pDataSource)
	{
		return getDSValue(pDataSource, native);
	}

	return PYXValue();
}

/*!
Get the field values around the specified native coordinate.
The origin of the matrix of returned values will be at the grid point 
in the mesh that is lower or equal to the nativeCentre point and adjusted 
left (or down) by trunc((size - 1)/2).  

If the values to be returned fall outside of the current data set, then 
the edges of the data set will be duplicated and returned.

\param	nativeCentre	The native coordinate.
\param	nFieldIndex		The field index.
\param  pValues         an array of PYXValue to be filled in with field values
						that are centered on the point requested.
\param  sizeX           width of PYXValue array
\param  sizeY           height of PYXValue array
*/
void WMSMultiDataSource::getMatrixOfAttrValues(	const PYXCoord2DDouble& nativeCentre,
										PYXValue* pValues,
										int sizeX,
										int sizeY,
										int nFieldIndex) const
{
	// this routine is a Critical Section in terms of thread safety
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	assert(pValues != 0 && "validate the pointer to the array.");
	assert(sizeX > 0 && "we want an array size that is positive");
	assert(sizeY > 0 && "we want an array size that is positive");
	assert(nFieldIndex == 0 && "Ignored, so should be 0.");
	assert(!m_mapDataSource.empty() && "Data source not open.");

	// calculate the lower left corner in raster coordinates.
	PYXCoord2DInt rasterBaseLL;
	nativeToLowerRaster(nativeCentre, &rasterBaseLL);
	rasterBaseLL.setX(rasterBaseLL.x()-static_cast<int>((sizeX - 1)/2.0));
	rasterBaseLL.setY(rasterBaseLL.y()-static_cast<int>((sizeY - 1)/2.0));

	// find the data source for the lower left corner
	PYXCoord2DDouble nativeLL;
	rasterToNative(rasterBaseLL, &nativeLL);
	const WMSDataSource* pDataSourceLL = getDataSource(nativeLL);

	// find the data source for the upper right corner
	PYXCoord2DInt rasterUR;
	rasterUR.setX(rasterBaseLL.x()+sizeX-1);
	rasterUR.setY(rasterBaseLL.y()+sizeY-1);
	PYXCoord2DDouble nativeUR;
	rasterToNative(rasterUR, &nativeUR);
	const WMSDataSource* pDataSourceUR = getDataSource(nativeUR);

	// if the matrix is covered completely by one data sorce, or one
	// of the corners falls outside the data.
	if (pDataSourceLL == pDataSourceUR && 0 != pDataSourceLL)
	{
		pDataSourceLL->getMatrixOfAttrValues(nativeCentre, pValues, sizeX, sizeY, nFieldIndex);
	}
	else if (0 != pDataSourceLL && 0 == pDataSourceUR)
	{
		pDataSourceLL->getMatrixOfAttrValues(nativeCentre, pValues, sizeX, sizeY, nFieldIndex);
	}
	else if (0 != pDataSourceUR && 0 == pDataSourceLL)
	{
		pDataSourceUR->getMatrixOfAttrValues(nativeCentre, pValues, sizeX, sizeY, nFieldIndex);
	}
	else
	{
		// default to get values one at a time if they span data sources.
	    for (int x = 0; x < sizeX; x++)
		{
			PYXCoord2DInt rasterGetAt;
			rasterGetAt.setX(rasterBaseLL.x()+x);
			for (int y = 0; y < sizeY; y++)
			{
				rasterGetAt.setY(rasterBaseLL.y()+y);
				PYXCoord2DDouble nativeGetAt;
				rasterToNative(rasterGetAt, &nativeGetAt);
				const WMSDataSource* pDataSource = getDataSource(nativeGetAt);
				if (0 != pDataSource)
				{
					pValues[x*sizeX+y] = pDataSource->getCoverageValue(nativeGetAt, nFieldIndex);
				}
			}
		}
	}
}

//! Get the distance between data points in this coverage
const PYXCoord2DDouble WMSMultiDataSource::getStepSize() const
{
	return m_mapDataSource.begin()->second->getStepSize();
}

/*! 
Constructor.

\param	begin	Start of map iteration
\param	end		End of map iteration
*/
WMSMultiDataSource::WMSMultiIterator::WMSMultiIterator(	WMSDataSourceMap::const_iterator begin,
														WMSDataSourceMap::const_iterator end	) :
	m_it(begin),
	m_end(end),
	m_spIterator(m_it->second->getXYIterator())
{
	// make sure we start with a valid iteration
	if (m_spIterator->end())
	{
		next();
	}
}

/*!
Destructor cleans up memory.
*/
WMSMultiDataSource::WMSMultiIterator::~WMSMultiIterator()
{
}

/*!
Move to the next point. Called recursively.
*/
void WMSMultiDataSource::WMSMultiIterator::next()
{
	if (!end())
	{
		m_spIterator->next();
		while (m_spIterator->end())
		{
			m_spIterator = 0;

			++m_it;
			if (end())
			{
				// we are done
				break;
			}

			// start iteration of next GDAL file
			m_spIterator = m_it->second->getXYIterator();
		}
	}
}

/*!
Get the coordinates for the current point.

\return	The coordinates.
*/
PYXCoord2DDouble WMSMultiDataSource::WMSMultiIterator::getPoint() const
{
	if (!end())
	{
		return m_spIterator->getPoint();
	}

	return getEmptyPoint();
}

/*!
Get the value of the current point.

\param	nFieldIndex	The field index (default = 0).

\return	The value.
*/
PYXValue WMSMultiDataSource::WMSMultiIterator::getFieldValue(int nFieldIndex) const
{
	if (!end())
	{
		return m_spIterator->getFieldValue(nFieldIndex);
	}

	return PYXValue();
}

/*!
Call this method on a data source that can supply data at multiple resolutions.

Call the function to set the resolution subsequent data is to be returned at.

The function can be called any number of times and the data source is to adjust
the resolution of any internal data returned to reflect the change requested.

Empty implementation except for data sources (such as WMS that downloads data
at various resolutions) that require the functionality.

\param	nResolution	Pyxis resolution of data to be returned by the data source.
*/
void WMSMultiDataSource::setRequestedDataResolution(int nResolution)
{
	// Did the screen pan or update in some manner - trigger removal of all currently downloading files.
	if (nResolution == -99)
	{
		m_bClearDownloads = true;
	}
	else
	{
		int nLod = nResolution / 3;

		if (nLod < 1)
		{
			nLod = 1;
		}

		// Make sure we are within the min and max allowable by the data source.
		if (nLod >= m_nMinLOD && nLod  <= m_nMaxLOD)
		{
			m_nLod = nLod;
		}
	}
}

/*!
The code that is run when this class is instantiated as a thread.
*/
void WMSMultiDataSource::operator()()
{
	m_pThreadDataSource->m_serverPipe.createServer();

   DWORD nBytesRead = 1;

   while (nBytesRead > 0 && !m_pThreadDataSource->m_bTerminate)
	{
		std::string strResponse;
		bool bResult = m_pThreadDataSource->m_serverPipe.waitForServerResponse(strResponse);

		if (bResult)
		{
			m_pThreadDataSource->m_serverPipe.sendResponse(std::string("OK"));
		}

		if (bResult && !strResponse.empty() && !m_pThreadDataSource->m_bTerminate)
		{
			std::stringstream strStream(strResponse.c_str());

			std::string strCmd;

			strStream >> strCmd;

			if (strCmd == "<file>")
			{
				int nLatMin;
				int nLonMin;
				int nLod;

				std::string strFileName;

				strStream >> nLonMin;
				strStream >> nLatMin;
				strStream >> nLod;
				strStream >> strFileName;

				m_pThreadDataSource->openFile(strFileName, nLod, nLonMin, nLatMin, 0, 0);
			}
		 }
	}
}
