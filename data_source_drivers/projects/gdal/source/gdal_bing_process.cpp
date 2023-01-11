/******************************************************************************
gdal_bing_process.cpp

begin      : 9/25/2012 9:57:00 AM
copyright  : (c) 2012 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_GDAL_SOURCE

//Code directives
//#define CHECK_WITH_METADATA
#define MULTI_PART_DOWNLOAD

// local includes
#include "gdal_bing_process.h"
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/neighbour_iterator.h"
#include "pyxis/derm/reference_sphere.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/derm/wgs84.h"
#include "pyxis/derm/wgs84_coord_converter.h"
#include "pyxis/geometry/bounding_rects_calculator.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/pipe/process.h"
#include "pyxis/pipe/process_identity_cache.h"
#include "pyxis/procs/srs.h"
#include "pyxis/sampling/xy_utils.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/http_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"
#include "pyxis/utility/thread_pool.h"

// GDAL includes
#include "cpl_string.h"
#include "gdal_priv.h"

// boost includes
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>
#include "boost/filesystem/convenience.hpp"

// standard includes
#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>
#include <cmath>

// {68EDBF04-72BD-4009-B388-834B8A6AE3C5}
PYXCOM_DEFINE_CLSID(GDALBingProcess,
					0x68edbf04, 0x72bd, 0x4009, 0xb3, 0x88, 0x83, 0x4b, 0x8a, 0x6a, 0xe3, 0xc5);
PYXCOM_CLASS_INTERFACES(GDALBingProcess, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(GDALBingProcess, "Bing map", "Bing base map", "Reader",
					IXYCoverage::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid)
					IPROCESS_SPEC_END

					// Tester class
					Tester<GDALBingProcess> gTester();

// Test method
void GDALBingProcess::test()
{
	PYXTile testTile= PYXTile(PYXIcosIndex("11-1"),10);
	TileInfo info = TileInfo(testTile,"Aerial","AjYCO1LdcgBJxGQ9iiVY0LoUwIY9y79I2xUBbdDGZduFw8deV1Ja93H5YFGTgrfc");
	PYXRect2DDouble rect1(0,-180,30,-170);
	PYXRect2DDouble rect2(0,170,30,180);
	info.m_imageSize=800;
	info.prepareTile();
	//info.createBingVRT(rect1,rect2);
}

// Bing Utilities
namespace
{
	const double EarthRadius = 6378137;
	const double MinLatitude = -85.05112878;
	const double MaxLatitude = 85.05112878;
	const double MinLongitude = -180;
	const double MaxLongitude = 180;

	//these are the size of the domain of epsg 3785 projection; more details : http://spatialreference.org/ref/epsg/3785/
	// The link above is the EPSG - bing map uses a different number which is :20037508.34 for both lat and long
	const double BingNativeDomainSize = 20037508.34;

	/// <summary>
	/// Clips a number to the specified minimum and maximum values.
	/// </summary>
	/// <param name="n">The number to clip.</param>
	/// <param name="minValue">Minimum allowable value.</param>
	/// <param name="maxValue">Maximum allowable value.</param>
	/// <returns>The clipped value.</returns>
	double clip(double n, double minValue, double maxValue)
	{
		return std::min(std::max(n, minValue), maxValue);
	}

	/// <summary>
	/// Determines the map width and height (in pixels) at a specified level
	/// of detail.
	/// </summary>
	/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
	/// to 23 (highest detail).</param>
	/// <returns>The map width and height in pixels.</returns>
	unsigned int mapSize(int levelOfDetail)
	{
		return (unsigned int) 256 << levelOfDetail;
	}

	/// <summary>
	/// Determines the ground resolution (in meters per pixel) at a specified
	/// latitude and level of detail.
	/// </summary>
	/// <param name="latitude">Latitude (in degrees) at which to measure the
	/// ground resolution.</param>
	/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
	/// to 23 (highest detail).</param>
	/// <returns>The ground resolution, in meters per pixel.</returns>
	double groundResolution(double latitude, int levelOfDetail)
	{
		latitude = clip(latitude, MinLatitude, MaxLatitude);
		return cos(latitude * PI / 180) * 2 * PI * EarthRadius / mapSize(levelOfDetail);
	}

	/// <summary>
	/// Determines the map scale at a specified latitude, level of detail,
	/// and screen resolution.
	/// </summary>
	/// <param name="latitude">Latitude (in degrees) at which to measure the
	/// map scale.</param>
	/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
	/// to 23 (highest detail).</param>
	/// <param name="screenDpi">Resolution of the screen, in dots per inch.</param>
	/// <returns>The map scale, expressed as the denominator N of the ratio 1 : N.</returns>
	double mapScale(double latitude, int levelOfDetail, int screenDpi)
	{
		return groundResolution(latitude, levelOfDetail) * screenDpi / 0.0254;
	}

	/// <summary>
	/// Converts a point from latitude/longitude WGS-84 coordinates (in degrees)
	/// into pixel XY coordinates at a specified level of detail.
	/// </summary>
	/// <param name="latitude">Latitude of the point, in degrees.</param>
	/// <param name="longitude">Longitude of the point, in degrees.</param>
	/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
	/// to 23 (highest detail).</param>
	/// <param name="pixelX">Output parameter receiving the X coordinate in pixels.</param>
	/// <param name="pixelY">Output parameter receiving the Y coordinate in pixels.</param>
	void latLongToPixelXY(double latitude, double longitude,  int levelOfDetail, unsigned int & pixelX,unsigned int & pixelY)
	{
		latitude = clip(latitude, MinLatitude, MaxLatitude);
		longitude = clip(longitude, MinLongitude, MaxLongitude);

		double x = (longitude + 180) / 360;
		double sinLatitude = sin(latitude * PI / 180);
		double y = 0.5 - log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * PI);

		unsigned int size = mapSize(levelOfDetail);
		pixelX = (unsigned int)clip((x * size + 0.5), 0, size);
		pixelY = (unsigned int)clip((y * size + 0.5), 0, size);
	}

	/// <summary>
	/// Converts a pixel from pixel XY coordinates at a specified level of detail
	/// into latitude/longitude WGS-84 coordinates (in degrees).
	/// </summary>
	/// <param name="pixelX">X coordinate of the point, in pixels.</param>
	/// <param name="pixelY">Y coordinates of the point, in pixels.</param>
	/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
	/// to 23 (highest detail).</param>
	/// <param name="latitude">Output parameter receiving the latitude in degrees.</param>
	/// <param name="longitude">Output parameter receiving the longitude in degrees.</param>
	void pixelXYToLatLong(int pixelX, int pixelY, int levelOfDetail, double & latitude, double & longitude)
	{
		double size = mapSize(levelOfDetail);
		double x = (clip(pixelX, 0, size - 1) / size) - 0.5;
		double y = 0.5 - (clip(pixelY, 0, size - 1) / size);

		latitude = 90 - 360 * atan(exp(-y * 2 * PI)) / PI;
		longitude = 360 * x;
	}

	/// <summary>
	/// Converts pixel XY coordinates into tile XY coordinates of the tile containing
	/// the specified pixel.
	/// </summary>
	/// <param name="pixelX">Pixel X coordinate.</param>
	/// <param name="pixelY">Pixel Y coordinate.</param>
	/// <param name="tileX">Output parameter receiving the tile X coordinate.</param>
	/// <param name="tileY">Output parameter receiving the tile Y coordinate.</param>
	void pixelXYToTileXY(int pixelX, int pixelY, int & tileX,  int & tileY)
	{
		tileX = pixelX / 256;
		tileY = pixelY / 256;
	}

	/// <summary>
	/// Converts tile XY coordinates into pixel XY coordinates of the upper-left pixel
	/// of the specified tile.
	/// </summary>
	/// <param name="tileX">Tile X coordinate.</param>
	/// <param name="tileY">Tile Y coordinate.</param>
	/// <param name="pixelX">Output parameter receiving the pixel X coordinate.</param>
	/// <param name="pixelY">Output parameter receiving the pixel Y coordinate.</param>
	void tileXYToPixelXY(int tileX, int tileY,  int& pixelX,  int& pixelY)
	{
		pixelX = tileX * 256;
		pixelY = tileY * 256;
	}

	/// <summary>
	/// Converts tile XY coordinates into a QuadKey at a specified level of detail.
	/// </summary>
	/// <param name="tileX">Tile X coordinate.</param>
	/// <param name="tileY">Tile Y coordinate.</param>
	/// <param name="levelOfDetail">Level of detail, from 1 (lowest detail)
	/// to 23 (highest detail).</param>
	/// <returns>A string containing the QuadKey.</returns>
	std::string tileXYToQuadKey(int tileX, int tileY, int levelOfDetail)
	{
		std::stringstream quadKey;
		for (int i = levelOfDetail; i > 0; i--)
		{
			char digit = '0';
			int mask = 1 << (i - 1);
			if ((tileX & mask) != 0)
			{
				digit++;
			}
			if ((tileY & mask) != 0)
			{
				digit++;
				digit++;
			}
			quadKey << digit;
		}
		return quadKey.str();
	}

	/// <summary>
	/// Converts a QuadKey into tile XY coordinates.
	/// </summary>
	/// <param name="quadKey">QuadKey of the tile.</param>
	/// <param name="tileX">Output parameter receiving the tile X coordinate.</param>
	/// <param name="tileY">Output parameter receiving the tile Y coordinate.</param>
	/// <param name="levelOfDetail">Output parameter receiving the level of detail.</param>
	void quadKeyToTileXY(std::string quadKey,  int& tileX,  int& tileY,  int& levelOfDetail)
	{
		tileX = tileY = 0;
		levelOfDetail = quadKey.length();
		for (int i = levelOfDetail; i > 0; i--)
		{
			int mask = 1 << (i - 1);
			switch (quadKey[levelOfDetail - i])
			{
			case '0':
				break;

			case '1':
				tileX |= mask;
				break;

			case '2':
				tileY |= mask;
				break;

			case '3':
				tileX |= mask;
				tileY |= mask;
				break;

			default:
				PYXTHROW(PYXException,"Invalid QuadKey digit sequence.");
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

/*!
Serialize the process to a map of strings.

\return the attribute schema as a string.
*/
std::string GDALBingProcess::getAttributeSchema() const
{
	return
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
		"elementFormDefault=\"qualified\" "
		"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"

		"<xs:simpleType name=\"MapType\">"
		"<xs:restriction base=\"xs:string\">"
		"<xs:enumeration value=\"Aerial\" />"
		"<xs:enumeration value=\"AerialWithLabels\" />"
		"<xs:enumeration value=\"Road\" />"
		//OrdnanceSurvey and CollinsBart are not available outside UK
		//"<xs:enumeration value=\"OrdnanceSurvey\" />"
		//"<xs:enumeration value=\"CollinsBart\" />"
		"</xs:restriction>"
		"</xs:simpleType>"

		"<xs:element name=\"GDALBingProcess\">"
		"<xs:complexType>"
		"<xs:sequence>"

		"<xs:element name=\"MapType\" type=\"MapType\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Type of Imagery</friendlyName>"
		"<description>Type of Imagery.</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"<xs:element name=\"Key\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Bing Map Key</friendlyName>"
		"<description>Key to access bing map.</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"</xs:sequence>"
		"</xs:complexType>"
		"</xs:element>"
		"</xs:schema>";
}

/*!
Set the attributes for this process.
*/
void STDMETHODCALLTYPE GDALBingProcess::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"MapType",m_mapType);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"Key",m_bingKey);
}

/*!
Get the attributes for this process.
*/
std::map<std::string, std::string> GDALBingProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr["MapType"] = m_mapType;
	mapAttr["Key"] = m_bingKey;
	return mapAttr;
}

/*!
Initialize the process so that it is able to provide data.
*/
IProcess::eInitStatus GDALBingProcess::initImpl()
{
	m_spParentXYCoverage.reset();
	m_spCurrentDataSrc.reset();
	m_downloadedTiles.clear();

	PYXCOMCreateInstance(GDALXYCoverage::clsid, 0, IXYCoverage::iid, (void**) &m_spParentXYCoverage);

	TileInfo info = TileInfo(PYXTile(),m_mapType,m_bingKey);
	info.m_imageSize=800;
	std::string vrt = info.createBingVRT(PYXRect2DDouble(MinLatitude, MinLongitude, MaxLatitude, MaxLongitude),0);

	if (vrt.length() > 0 && !m_spParentXYCoverage->openAsRGB(vrt,0))
	{
		m_spInitError->setError("GDAL is unable to open the Bing datasource");
		return knFailedToInit;
	}
	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// IXYCoverage
////////////////////////////////////////////////////////////////////////////////
/*!
Returns a constant coverage definition.

\return		The coverage definition, as a constant.
*/

PYXPointer<const PYXTableDefinition> GDALBingProcess::getCoverageDefinition() const
{
	if(!m_definition )
	{
		m_definition=m_spParentXYCoverage->getCoverageDefinition();
	}
	return m_definition;
}

/*!
Returns the coverage definition.

\return		The coverage definition.
*/
PYXPointer<PYXTableDefinition> GDALBingProcess::getCoverageDefinition()
{
	if(!m_definition )
	{
		m_definition=m_spParentXYCoverage->getCoverageDefinition();
	}
	return m_definition;
}

/*!
Get the coverage value at the specified native coordinate.

\param	native		The native coordinate.
\param	pValue		The value being retrieved.

\return	True if successful, false otherwise.
*/
bool GDALBingProcess::getCoverageValue(const PYXCoord2DDouble& native,
									   PYXValue* pValue) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	if(!m_spParentXYCoverage->getBounds().inside(native))
	{
		return false;
	}

	if(!m_spCurrentDataSrc)
	{
		return m_spParentXYCoverage->getCoverageValue(native, pValue);
	}

	return m_spParentXYCoverage->getCoverageValue(native, pValue);
}

//////////////////////////////////////////////////////////////////////
// BingXYAsyncValueGetterAndConsumer
//////////////////////////////////////////////////////////////////////
// Bing service some times return damaged images with the large gray area
// which is RGB=(233, 230, 211)
// Therefore, this utility class count how many cells got this collor.
// the heuristic is if we got more then 100 of this color - it a damaged
// tile. and we notify the sampler that something went worng
//////////////////////////////////////////////////////////////////////
class BingXYAsyncValueGetterAndConsumer : public XYAsyncValueGetter, XYAsyncValueConsumer
{
private:
	mutable boost::detail::atomic_count m_grayValuesCount;
	bool m_checkForFaults;
	const XYAsyncValueConsumer & m_consumer;
	PYXPointer<XYAsyncValueGetter> m_getter;

public: //XYAsyncValueConsumer
	virtual void onRequestCompleted(const PYXIcosIndex & index,
		const PYXCoord2DDouble & nativeCoord,
		bool * hasValues,
		PYXValue * values,
		int width,int height) const
	{
		//check if the value returned is a "damaged" gray.
		if(m_checkForFaults)
		{
			for(int x=0;x<width;x++)
			{
				for(int y=0;y<height;y++)
				{
					if (hasValues[x*height+y] && !values[x*height+y].isNull())
					{
						PYXValue & value = values[x*height+y];

						if ((value.getUInt8(0)==233 && value.getUInt8(1)==230 && value.getUInt8(2)==211) || //gray #1
							(value.getUInt8(0)==85 && value.getUInt8(1)==109 && value.getUInt8(2)==73)   || //gray #2
							(value.getUInt8(0)==172 && value.getUInt8(1)==199 && value.getUInt8(2)==241) || //gray #3
							(value.getUInt8(0)==172 && value.getUInt8(1)==199 && value.getUInt8(2)==242))   //gray #4
						{
							++m_grayValuesCount;
						}
					}
				}
			}
		}
		//forward the request...
		m_consumer.onRequestCompleted(index,nativeCoord,hasValues,values,width,height);
	}

public: //class XYAsyncValueGetter

	virtual void addAsyncRequests(const PYXTile & tile)
	{
		m_getter->addAsyncRequests(tile);
	}

	virtual void addAsyncRequest(const PYXIcosIndex & index)
	{
		m_getter->addAsyncRequest(index);
	}

	virtual bool join()
	{
		//Note: good tiles from some testing had around 1..4 gray values.
		//However, bad tiles have few thousands (4K~16K) of gray values.
		//So, for now I think the threshold of 100 is safe enough
		if (m_getter->join() && (!m_checkForFaults || m_grayValuesCount<100))
		{
			//we have very few gray values - the image should be great
			return true;
		}

		TRACE_INFO("Bing has too many gray values (Bing m_grayValuesCount = " << m_grayValuesCount << ") - consider this as an error image");
		return false;
	}

public:
	BingXYAsyncValueGetterAndConsumer(const XYAsyncValueConsumer & consumer, bool checkForFaults) : m_consumer(consumer), m_grayValuesCount(0),m_checkForFaults(checkForFaults)
	{
	}

	static PYXPointer<BingXYAsyncValueGetterAndConsumer> create(const XYAsyncValueConsumer & consumer,const boost::intrusive_ptr<GDALXYCoverage> & coverage,int width,int height, bool checkForFaults)
	{
		PYXPointer<BingXYAsyncValueGetterAndConsumer> getterAndConsumer = PYXNEW(BingXYAsyncValueGetterAndConsumer,consumer,checkForFaults);
		//create a new the real getter of the coverage - and attach it to our consumer - which will forward everything to the real consumer.
		getterAndConsumer->m_getter = coverage->getAsyncCoverageValueGetter(*getterAndConsumer,width,height);

		//return a pointer to us. we will free the real getter once the caller done with us.
		return getterAndConsumer;
	}
};

PYXPointer<XYAsyncValueGetter> GDALBingProcess::getAsyncCoverageValueGetter(
	const XYAsyncValueConsumer & consumer,
	int matrixWidth,
	int matrixHeight
	) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	bool shouldCheckforFaults = (m_mapType=="AerialWithLabels" || m_mapType=="Aerial");

	if(!m_spCurrentDataSrc)
	{
		return BingXYAsyncValueGetterAndConsumer::create(consumer,m_spParentXYCoverage,matrixWidth,matrixHeight, shouldCheckforFaults);
	}

	return BingXYAsyncValueGetterAndConsumer::create(consumer,m_spCurrentDataSrc,matrixWidth,matrixHeight, shouldCheckforFaults);
}

/*!
Indicates if the coverage has a spatial reference system.

\return True if the coverage has a spatial reference system. False otherwise.
*/
bool GDALBingProcess::hasSpatialReferenceSystem() const
{
	return m_spParentXYCoverage->hasSpatialReferenceSystem();
}

/*!
Sets the coverage's spatial reference system.

\param	The SRS we are setting to.
*/
void GDALBingProcess::setSpatialReferenceSystem(boost::intrusive_ptr<ISRS> spSRS)
{
	m_spParentXYCoverage->setSpatialReferenceSystem(spSRS);
}

/*!
Get the coordinate converter.

\return	The coordinate converter.
*/
const ICoordConverter* GDALBingProcess::getCoordConverter() const
{
	return m_spParentXYCoverage->getCoordConverter();
}

void GDALBingProcess::getMatrixOfValues(const PYXCoord2DDouble& nativeCentre,
										PYXValue* pValues,
										int nSizeX,
										int nSizeY) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	if(!m_spCurrentDataSrc)
	{
		return m_spParentXYCoverage->getMatrixOfValues(nativeCentre, pValues, nSizeX, nSizeY);
	}

	m_spCurrentDataSrc->getMatrixOfValues(nativeCentre, pValues, nSizeX, nSizeY);
}

/*!
Get the spatial precision.

\return	The spatial precision in metres or -1.0 if not known.
*/
double GDALBingProcess::getSpatialPrecision() const
{
	return m_spParentXYCoverage->getSpatialPrecision();
}

/*!
Get the bounds of the data set in native coordinate.

\return	The bounds of the coverage.
*/

const PYXRect2DDouble& GDALBingProcess::getBounds() const
{
	return m_spParentXYCoverage->getBounds();
}

/*!
Get the distance between data points in this coverage.

\return The step size.
*/
PYXCoord2DDouble GDALBingProcess::getStepSize() const
{
	return m_spParentXYCoverage->getStepSize();
}

PYXCoord2DDouble GDALBingProcess::nativeToRasterSubPixel(const PYXCoord2DDouble & native) const
{
	return m_spParentXYCoverage->nativeToRasterSubPixel(native);
}

/*!
Loads a hint for tile data access.
*/

void GDALBingProcess::downloadTile(PYXPointer<GDALBingProcess::TileInfo> tileInfo)
{
	tileInfo->prepareTile();
}

void GDALBingProcess::tileLoadHint(const PYXTile& tile) const
{
	PYXTile testTile=tile;
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	if (!m_downloadedTiles.exists(testTile))
	{
		m_downloadedTiles[testTile] = TileInfo::create(testTile,m_mapType,m_bingKey);
		//downloadTile(m_downloadedTiles[testTile]);
		PYXThreadPool::addSlowTask( boost::bind(downloadTile,m_downloadedTiles[testTile]));
	}
	PYXPointer<TileInfo> currentTile = m_downloadedTiles[testTile];

	//[shatzi]: it seems requesting other tiles slowed us down.
	//downloading other requiest just cause the system to wait more.
	//therefore, we currently only downloading the needed tile

	/*
	for (PYXNeighbourIterator it(testTile.getRootIndex());!it.end();it.next())
	{
	PYXTile otherTile(it.getIndex(),testTile.getCellResolution());
	if (!m_downloadedTiles.exists(otherTile))
	{
	m_downloadedTiles[otherTile] = TileInfo::create(otherTile,m_mapType,m_bingKey);
	//downloadTile(m_downloadedTiles[otherTile]);
	PYXThreadPool::addSlowTask( boost::bind(downloadTile,m_downloadedTiles[otherTile]));
	}
	}
	*/

	while(!currentTile->downloaded && !currentTile->failed)
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}

	if (currentTile->failed)
	{
		m_downloadedTiles.erase(currentTile->m_tile);
		PYXTHROW(PYXException,"Failed to download tile from bing");
	}

	std::string request=currentTile->vrt;

	if( request.length() > 0)
	{
		m_spCurrentDataSrc = 0;
		PYXCOMCreateInstance(GDALXYCoverage::clsid, 0, IXYCoverage::iid, (void**) &m_spCurrentDataSrc);
		if (!m_spCurrentDataSrc->openAsRGB(request, 0))
		{
			PYXTHROW(PYXException,"Can not read the bing request!");
			m_spInitError->setError("GDAL is unable to open the Bing datasource");
		}

		m_spCurrentDataSrc->m_coordConverterExternal = boost::intrusive_ptr<BingCoordConverter>(new BingCoordConverter());
	}

	return;
}

/*!
Indicates when tile hint is no longer needed.
*/
void GDALBingProcess::tileLoadDoneHint(const PYXTile& tile) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_spCurrentDataSrc.reset();
	m_downloadedTiles.erase(tile);
}

void GDALBingProcess::TileInfo::fixRectForBingRequest( PYXRect2DDouble &rect1 )
{
	rect1.scaleInPlace(1.1);
	rect1.flip();
	rect1.clip(PYXRect2DDouble(MinLatitude,MinLongitude,MaxLatitude,MaxLongitude));
}

GDALBingProcess::TileInfo::TileInfo(const PYXTile & aTile, const std::string & mapType,const std::string & bingKey)
	: m_tile(aTile), downloaded(false), failed(false),m_mapType(mapType),m_bingKey(bingKey)
{
}

void GDALBingProcess::TileInfo::prepareTile()
{
	try
	{
		//Calculating the rectangle of tile
		PYXRect2DDouble rect1;
		PYXRect2DDouble rect2;
		WGS84CoordConverter coordConverter;
		PYXPointer<PYXCell> spTestCell = PYXCell::create(m_tile.getRootIndex());
		PYXBoundingRectsCalculator calc(&coordConverter, *spTestCell);

		if(spTestCell->getIndex().hasVertexChildren())
		{
			for (PYXChildIterator it(spTestCell->getIndex());!it.end();it.next())
			{
				calc.addCell(PYXCell(it.getIndex()));
			}
			m_imageSize = 613;
		}
		else
		{
			m_imageSize = 400;
			calc.addCell(*(spTestCell.get()));
		}
		calc.getBoundingRects(&rect1, &rect2);
		fixRectForBingRequest(/*ref*/ rect1);
		fixRectForBingRequest(/*ref*/ rect2);
		// Creating the VRT
		vrt = createBingVRT(rect1,rect2);

		downloaded = true;
	}
	catch (...)
	{
		TRACE_ERROR("Unable to prepare the VRT for this tile" << m_tile.getRootIndex());
		failed = true;
	}
}
#if defined(MULTI_PART_DOWNLOAD)
const int GDALBingProcess::TileInfo::s_numberOfparts = 4;
#else
const int GDALBingProcess::TileInfo::s_numberOfparts = 1;
#endif
std::string GDALBingProcess::TileInfo::createBingVRT( PYXRect2DDouble rect, PYXRect2DDouble rect2 )
{
	std::stringstream str;
	SourceBandInfo info1[s_numberOfparts];
	SourceBandInfo info2[s_numberOfparts];
	InitSourceBands(rect, rect2, info2, info1);
	for (int i = 0 ; i < s_numberOfparts ; i++)
	{
		if(!rect2.empty())
		{
			filesToDelete.insert(info2[i].m_fileLocation);
		}
		filesToDelete.insert(info1[i].m_fileLocation);
	}

	unsigned int size = mapSize(info1[0].m_zoom);
	double scale = 2 * BingNativeDomainSize / size;

	str	 << std::setprecision(11)
		<< "<VRTDataset rasterXSize=\""<< size <<"\" rasterYSize=\""<< size <<"\">"
		// SRS definition: http://spatialreference.org/ref/sr-org/7483/proj4/
		// more info: http://alastaira.wordpress.com/2011/01/23/the-google-maps-bing-maps-spherical-mercator-projection/
		<< "<SRS>+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext  +no_defs</SRS>"
		<< "<GeoTransform>"<< -BingNativeDomainSize <<","<< scale <<","<< 0.0 <<","<< BingNativeDomainSize <<","<< 0.0 <<","<< -scale << "</GeoTransform>";

	for (int color = 1; color < 4 ; ++color)
	{
		str
			<< "<VRTRasterBand dataType=\"Byte\" band=\"" <<color<< "\">"
			<< "<ColorInterp>"<< m_colors[color - 1] <<"</ColorInterp>";
		for (int i = 0 ; i < s_numberOfparts ; i++)
		{
			if(!rect2.empty())
			{
				str << info2[i].createSourceBand(color);
			}
			str	<< info1[i].createSourceBand(color);
		}
		str << "</VRTRasterBand>";
	}
	str  << "</VRTDataset>";
	return str.str();
}

GDALBingProcess::TileInfo::~TileInfo()
{
	for(std::set<boost::filesystem::path>::iterator it = filesToDelete.begin();
		it != filesToDelete.end();
		++it)
	{
		//try to delete the files... if we can...
		try
		{
			boost::filesystem::remove(*it);
		}
		catch (...)
		{
		}
	}
}

void GDALBingProcess::TileInfo::findZoomlevelandImageSize( PYXRect2DDouble & rect,int & resolution,unsigned int & imageSize )
{
	unsigned int i=0;
	unsigned int j=0;

	unsigned int m=0;
	unsigned int n=0;

	unsigned int r=0;
	unsigned int s=0;

	PYXCoord2DDouble center = rect.center();
	latLongToPixelXY(rect.xMin(),rect.yMin(),23,i,j);
	latLongToPixelXY(rect.xMax(),rect.yMax(),23,m,n);
	latLongToPixelXY(center.x(),center.y(),23,r,s);

	unsigned int delta = std::max(std::max(j - s,s - n), std::max(m - r , r - i)) * 2;
	double scale = delta / m_imageSize;
	int zoom = 22 - (int)(log(scale) / log(2.0));

	imageSize = delta / (1 << (23-zoom)) + 10;
	resolution = std::max(1,zoom);
	if(resolution==1)
	{
		imageSize = std::max((unsigned int)513,imageSize);
	}
}

void GDALBingProcess::TileInfo::downloadParts( SourceBandInfo info[] ,PYXRect2DDouble &rect, int imageSize, int resolution, PYXTaskGroup &tasks )
{
#if defined (MULTI_PART_DOWNLOAD)
	PYXCoord2DDouble center = rect.center();
	resolution++;
	info[0] = SourceBandInfo(m_mapType,m_bingKey,imageSize,resolution, PYXRect2DDouble(rect.xMin(),rect.yMin(),center.x(),center.y()));
	info[1] = SourceBandInfo(m_mapType,m_bingKey,imageSize,resolution, PYXRect2DDouble(center.x(),center.y(),rect.xMax(),rect.yMax()));
	info[2] = SourceBandInfo(m_mapType,m_bingKey,imageSize,resolution, PYXRect2DDouble(rect.xMin(),center.y(),center.x(),rect.yMax()));
	info[3] = SourceBandInfo(m_mapType,m_bingKey,imageSize,resolution, PYXRect2DDouble(center.x(),rect.yMin(),rect.xMax(),center.y()));

	for (int i = 0 ; i < s_numberOfparts ; i++)
	{
		tasks.addSlowTask(boost::bind(&SourceBandInfo::getBingFile, &info[i]));
	}
#else
	info[0] = SourceBandInfo(m_mapType,m_bingKey,imageSize,resolution, rect);
	info[0].getBingFile();
#endif
}

void GDALBingProcess::TileInfo::InitSourceBands( PYXRect2DDouble &rect, PYXRect2DDouble &rect2, SourceBandInfo * info2, SourceBandInfo * info1 )
{
	PYXTaskGroup tasks;
	int resolution;
	int resolution2 = 24;
	unsigned int imageSize2 = 0;
	int resolution1;
	unsigned int imageSize1;
	findZoomlevelandImageSize(rect,resolution1,imageSize1);
	//resolution1;

	if(!rect2.empty())
	{
		findZoomlevelandImageSize(rect2,resolution2,imageSize2);
		//resolution2;
		m_imageSize = std::max (imageSize1, imageSize2);
		resolution = std::min (resolution1, resolution2);
		downloadParts(info2, rect2, m_imageSize, resolution, tasks);
	}

	m_imageSize = std::max (imageSize1, imageSize2);
	resolution = std::min (resolution1, resolution2);

	downloadParts(info1, rect, m_imageSize, resolution, tasks);
	tasks.joinAll();
}

const std::string GDALBingProcess::TileInfo::m_colors[] = {"Red","Blue","Green"};

void SourceBandInfo::getBingFile()
{
	m_fileLocation = getFileName();
	std::string request = createRequest();

#if defined(CHECK_WITH_METADATA)
	BingMetaData meta;
	meta.getMetaData(request);
	PYXRect2DDouble given = meta.getRectangle();
	if( given.yMax() + SphereMath::knDotNumericEpsilon < m_rect.yMax() || given.yMin() - SphereMath::knDotNumericEpsilon > m_rect.yMin() ||
		given.xMax() + SphereMath::knDotNumericEpsilon < m_rect.xMax() || given.xMin() - SphereMath::knDotNumericEpsilon > m_rect.xMin() )
	{
		TRACE_ERROR("Bing map uncovered area " << m_rect << " returned:" << given );
	}
#endif
	auto httpRequest = HttpRequest::create(request,"GET");
	//httpRequest->addRequestHeader("User-Agent","Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.31 (KHTML, like Gecko) Chrome/26.0.1410.64 Safari/537.31");

	if(!httpRequest->downloadResponse(FileUtils::pathToString(m_fileLocation)))
	{
		PYXTHROW (PYXException, "Failed to download map: " << request);
	}
}

std::string SourceBandInfo::createRequest()
{
	std::stringstream bingRequest;
	bingRequest
		<< std::setprecision(11)
		<< "http://dev.virtualearth.net/REST/v1/Imagery/Map/"
		<< m_mapType <<"/"
		<< m_center.x() << "," << m_center.y() << "/"
		<< m_zoom << "?"
		<< "&mapSize=" <<  m_imageSize + s_padding * 2 <<","<< m_imageSize + s_padding * 2
		<< "&format="  << "JPEG"
		//<< "&key=AjYCO1LdcgBJxGQ9iiVY0LoUwIY9y79I2xUBbdDGZduFw8deV1Ja93H5YFGTgrfc";
		<< "&key=" << m_bingKey;

	return bingRequest.str();
}
//std::string SourceBandInfo::createRequest()
//{
//	std::stringstream bingRequest;
//
//			//Google Maps
//
//	bingRequest << std::setprecision(11)
//	<< "http://maps.googleapis.com/maps/api/staticmap?"
//	<< "center=" << m_center.x() << "," << m_center.y()
//	<< "&zoom=" << m_zoom
//	<< "&size="  << m_imageSize<<"x"<<m_imageSize
//	<< "&sensor=false&format=png32";
//
//	return bingRequest.str();
//}

boost::filesystem::path SourceBandInfo::getFileName( )
{
	return AppServices::makeTempFile(".JPEG");
}

/*
Creates the xml request for one of the WMS grid tiles.
*/
std::string SourceBandInfo::createSourceBand( int index )
{
	unsigned int pixelCoordX;
	unsigned int pixelCoordY;
	latLongToPixelXY(m_center.x(), m_center.y(), m_zoom,    pixelCoordX,  pixelCoordY);

	int size= mapSize(m_zoom);
	double scale = BingNativeDomainSize * 2 / size;

	// calculating the image offset
	PYXRect2DInt srcOffset(s_padding,s_padding,m_imageSize,m_imageSize);
	PYXRect2DInt dstOffset(pixelCoordX - m_imageSize/2 ,pixelCoordY - m_imageSize/2 ,m_imageSize,m_imageSize);

	if(pixelCoordY < m_imageSize/2 )
	{
		srcOffset.setYMin(m_imageSize/2  - pixelCoordY + s_padding);
		dstOffset.setYMin(0);
	}

	if(pixelCoordX < m_imageSize/2 )
	{
		srcOffset.setXMin(m_imageSize/2 - pixelCoordX + s_padding);
		dstOffset.setXMin(0);
	}

	std::stringstream band;
	band

		<< "<SimpleSource>"
		<< "<SourceFilename relativeToVRT=\"0\">" << FileUtils::pathToString(m_fileLocation) << "</SourceFilename>"
		<< "<SourceBand>"<<index<<"</SourceBand>"
		<< "<SrcRect xOff=\""<< srcOffset.xMin() <<"\" yOff=\""<< srcOffset.yMin() <<"\" xSize=\"" <<srcOffset.xMax()<<"\" ySize=\""<<srcOffset.yMax()<<"\"/>"
		<< "<DstRect xOff=\""<< dstOffset.xMin() <<"\" yOff=\""<< dstOffset.yMin() <<"\" xSize=\"" <<dstOffset.xMax()<<"\" ySize=\""<<dstOffset.yMax()<<"\"/>"
		<< "</SimpleSource>";

	std::string request = band.str();
	return request;
}

void BingMetaData::getMetaData(const std::string& mapURL )
{
	m_imageMetaData.erase();

	std::stringstream request;
	request << mapURL << "&mapMetadata=1&o=xml";

	boost::intrusive_ptr <HttpRequest> metaData= HttpRequest::create(request.str(),"get");

	if(metaData->getResponse())
	{
		m_imageMetaData = metaData->getResponseBody();
	}
}

PYXRect2DDouble BingMetaData::getRectangle()
{
	double south=getInnerXml("SouthLatitude");
	double west=getInnerXml("WestLongitude");
	double north=getInnerXml("NorthLatitude");
	double east=getInnerXml("EastLongitude");
	return PYXRect2DDouble(south,west,north,east);
}
int BingMetaData::getResolution()
{
	return (int) getInnerXml("Zoom");
}
PYXCoord2DDouble BingMetaData::getCenter()
{
	double lat=getInnerXml("Latitude");
	double lon=getInnerXml("Longitude");
	return PYXCoord2DDouble(lat,lon);
}

double BingMetaData::getInnerXml(const std::string& key)
{
	//TODO: replace with a real xml parser!
	double result;
	int start= m_imageMetaData.find("<"+ key + ">")+key.length()+2;
	int end= m_imageMetaData.find("</" + key + ">");
	result =  strToDouble(m_imageMetaData.substr(start,end-start));
	return result;
}
/*!
Converts a string to a double.
*/
double BingMetaData::strToDouble(const std::string& strData) const
{
	std::istringstream inStr(strData);
	double fValue;
	inStr >> fValue;
	return fValue;
}

// {9614E4E6-247D-446f-914A-205D79BC7518}
PYXCOM_DEFINE_CLSID(BingCoordConverter,
					0x9614e4e6, 0x247d, 0x446f, 0x91, 0x4a, 0x20, 0x5d, 0x79, 0xbc, 0x75, 0x18);

PYXCOM_CLASS_INTERFACES(BingCoordConverter, ICoordConverter::iid, PYXCOM_IUnknown::iid);

//! Serialize.
std::basic_ostream<char>& BingCoordConverter::serialize(std::basic_ostream<char>& out) const
{
	return out;
}

//! Deserialize.
std::basic_istream<char>& BingCoordConverter::deserialize(std::basic_istream<char>& in)
{
	return in;
}

//! Serialize the COM object.
void BingCoordConverter::serializeCOM(std::basic_ostream<char>& out) const
{
	out << clsid;
}

//! Convert native coordinates to a PYXIS index.
void BingCoordConverter::nativeToPYXIS(	const PYXCoord2DDouble& native,
									   PYXIcosIndex* pIndex,
									   int nResolution	) const
{
	CoordLatLon ll;
	nativeToLatLon(native,&ll);
	SnyderProjection::getInstance()->nativeToPYXIS(ll,pIndex,nResolution);
}

//! Convert a PYXIS index to native coordinates.
void BingCoordConverter::pyxisToNative(	const PYXIcosIndex& index,
									   PYXCoord2DDouble* pNative	) const
{
	CoordLatLon ll;
	SnyderProjection::getInstance()->pyxisToNative(index,&ll);
	latLonToNative(ll,pNative);
}

//! Convert a PYXIS index to native coordinates.
bool BingCoordConverter::tryPyxisToNative( const PYXIcosIndex& index, PYXCoord2DDouble* pNative	) const
{
	//this will always works (what is the correct range of latLon?)
	pyxisToNative(index,pNative);
	return true;
}

//! Convert a native coordinate to a geocentric LatLon coordinates.
void BingCoordConverter::nativeToLatLon(const PYXCoord2DDouble& native,
										CoordLatLon * pLatLon) const
{
	pLatLon->setLonInDegrees( (native.x() / 20037508.34) * 180);
	pLatLon->setLatInDegrees( 180/PI * (2 * atan(exp(native.y() / 20037508.34 * PI )) - PI / 2) );
}

//! Convert a native coordinate to a geocentric LatLon coordinates.
void BingCoordConverter::latLonToNative(const CoordLatLon & latLon,
										PYXCoord2DDouble * pNative) const
{
	CoordLatLon ll = WGS84::getInstance()->toDatum(latLon);
	double x = ll.lonInDegrees() * 20037508.34 / 180;
	double y = log(tan((90 + ll.latInDegrees()) * PI / 360)) / (PI / 180);
	y = y * 20037508.34 / 180;
	pNative->setX(x);
	pNative->setY(y);
}