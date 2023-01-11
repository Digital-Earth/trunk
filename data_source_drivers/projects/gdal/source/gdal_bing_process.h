#ifndef GDAL_BING_PROCESS_H
#define GDAL_BING_PROCESS_H

/******************************************************************************
gdal_bing_process.h

begin      : Wednesday, September 12, 2012 9:56:57 PM
copyright  : (c) 2012 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "coord_converter_impl.h"
#include "gdal_metadata.h"
#include "gdal_xy_coverage.h"
#include "module_gdal.h"
#include "pyx_rtree.h"

// pyxlib includes
#include "pyxis/derm/horizontal_datum.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/path.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_iterator.h"
#include "pyxis/utility/cache_map.h"

// GDAL includes
#include "ogr_spatialref.h"

// boost includes
#include <boost/filesystem/path.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include "pyxis/procs/srs.h"

// local forward declarations
class BingMetaData
{
public:	
	BingMetaData()
	{
	}
	
	BingMetaData(const std::string& imageURL)
	{
		getMetaData(imageURL);
	}

	void getMetaData(const std::string& mapURL);
	PYXRect2DDouble getRectangle ();
	int getResolution();
	PYXCoord2DDouble getCenter();

private: // Methods
	double getInnerXml(const std::string& key);
	double strToDouble(const std::string& strData) const;

private: // Members
	std::string m_imageMetaData;
};




class SourceBandInfo
{
public:
	SourceBandInfo(std::string mapType,std::string bingKey, unsigned int imageSize, int zoom, PYXRect2DDouble rect)
		:m_fileLocation(),
		m_rect(rect),
		m_center(rect.center()),
		m_zoom(zoom),
		m_bingKey(bingKey),
		m_imageSize(imageSize),
		m_mapType(mapType)
	{
	}
	SourceBandInfo():m_fileLocation(),m_rect(),m_center()
	{
	}
	void getBingFile( );
	boost::filesystem::path getFileName( );
	std::string createRequest();
	std::string createSourceBand( int index );

public:
	PYXRect2DDouble m_rect;
	boost::filesystem::path m_fileLocation;
	int m_zoom;
	PYXCoord2DDouble m_center;
	std::string m_mapType;
	unsigned int m_imageSize;
	std::string m_bingKey;
	static const int s_padding = 45;
};
/*!
Provides access to BingMap data through the GDAL library.
*/
class MODULE_GDAL_DECL GDALBingProcess : public ProcessImpl<GDALBingProcess>, public XYCoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:
	//! Test method
	static void test();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IXYCoverage)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IRecord

	IRECORD_IMPL();
	

public: // IFeature

	IFEATURE_IMPL();

public: // IFeatureCollection

	IFEATURECOLLECTION_IMPL();

public:

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IXYCoverage*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IXYCoverage*>(this);
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public:
	GDALBingProcess() : m_downloadedTiles(40)
	{}

public: // IXYCoverage

	virtual PYXPointer<XYAsyncValueGetter> STDMETHODCALLTYPE getAsyncCoverageValueGetter(
			const XYAsyncValueConsumer & consumer,
			int matrixWidth,
			int matrixHeight
		) const;

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition() const;

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getCoverageDefinition();

	virtual bool STDMETHODCALLTYPE getCoverageValue(const PYXCoord2DDouble& native,
													PYXValue* pValue) const;

	virtual void STDMETHODCALLTYPE getMatrixOfValues(const PYXCoord2DDouble& nativeCentre,
													 PYXValue* pValues,
													 int sizeX,
													 int sizeY) const;

	virtual bool STDMETHODCALLTYPE hasSpatialReferenceSystem() const;

	virtual void STDMETHODCALLTYPE setSpatialReferenceSystem(boost::intrusive_ptr<ISRS> spSRS);

	virtual const ICoordConverter* STDMETHODCALLTYPE getCoordConverter() const;

	virtual double STDMETHODCALLTYPE getSpatialPrecision() const;

	virtual const PYXRect2DDouble& STDMETHODCALLTYPE getBounds() const;

	virtual PYXCoord2DDouble STDMETHODCALLTYPE getStepSize() const;

	virtual PYXCoord2DDouble STDMETHODCALLTYPE nativeToRasterSubPixel(const PYXCoord2DDouble & native) const;

	virtual void STDMETHODCALLTYPE tileLoadHint (const PYXTile& tile) const;

	virtual void STDMETHODCALLTYPE tileLoadDoneHint (const PYXTile& tile) const;

private:

	//! Get the data source that contains the native point.
	boost::intrusive_ptr<IXYCoverage> getDataSource(const PYXCoord2DDouble& native,
													int nResolution) const;


private:

	class TileInfo : public PYXObject
	{
	public:
		TileInfo (const PYXTile & aTile, const std::string & mapType,const std::string & bingKey);

		static PYXPointer<TileInfo> create(const PYXTile & aTile, const std::string & mapType,const std::string & bingKey)
		{
			return PYXNEW(TileInfo,aTile, mapType, bingKey);
		}

		virtual ~TileInfo();

	public:
		void prepareTile();
		std::string createBingVRT( PYXRect2DDouble rect, PYXRect2DDouble rect2 );
		void InitSourceBands( PYXRect2DDouble &rect, PYXRect2DDouble &rect2, SourceBandInfo * info2,  SourceBandInfo * info1 );
		void downloadParts( SourceBandInfo info1[] ,PYXRect2DDouble &rect, int imageSize, int resolution, PYXTaskGroup &tasks );
		void fixRectForBingRequest( PYXRect2DDouble &rect1 );
		void findZoomlevelandImageSize( PYXRect2DDouble & rect,int & resolution,unsigned int & imageSize );
	public:	
		PYXTile m_tile;
		unsigned int m_imageSize;
		static const std::string m_colors[];

		std::string m_bingKey;
		std::string m_mapType;

		std::string fileName;
		std::string vrt;
		bool downloaded;
		bool failed;
	private:
		mutable boost::recursive_mutex m_mutex;
		std::set<boost::filesystem::path> filesToDelete;

		static const	int s_numberOfparts ;
	};

	static void GDALBingProcess::downloadTile(PYXPointer<GDALBingProcess::TileInfo> tileInfo);

private:

	//! The parent coverage. This is the Bing data source at max resolution.
	mutable boost::intrusive_ptr<GDALXYCoverage> m_spParentXYCoverage;

	//! The current data source.
	mutable boost::intrusive_ptr<GDALXYCoverage> m_spCurrentDataSrc;

	//! Mutex for thread safety.
	mutable boost::recursive_mutex m_mutex;

	mutable PYXPointer< PYXTableDefinition> m_definition;

	boost::filesystem::path m_pathCache;

	mutable CacheMap<PYXTile,PYXPointer<TileInfo>> m_downloadedTiles;

	std::string m_bingKey;

	std::string m_mapType;
	
	int m_instance;
};

class MODULE_GDAL_DECL BingCoordConverter : public PYXCoordConverter
{
	PYXCOM_DECLARE_CLASS();

public:

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(PYXCOM_IUnknown)
		IUNKNOWN_QI_CASE(ICoordConverter)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public:
	//! Constructor.
	BingCoordConverter()
	{}

	//! Destructor.
	virtual ~BingCoordConverter()
	{}

	//! Serialize.
	std::basic_ostream<char>& serialize(std::basic_ostream<char>& out) const;

	//! Deserialize.
	std::basic_istream<char>& deserialize(std::basic_istream<char>& in);

	//! Serialize the COM object.
	virtual void serializeCOM(std::basic_ostream<char>& out) const;

	//! Clone.
	virtual boost::intrusive_ptr<ICoordConverter> clone() const { return new BingCoordConverter(); }
	
	//! Convert native coordinates to WGS84 coordinates.
	void nativeToWGS84(	const PYXCoord2DDouble& native,
						CoordLatLon* pLatLon	) const;
	
	//! Convert WGS84 coordinates to native coordinates.
	void wgs84ToNative(	const CoordLatLon& latLon,
						PYXCoord2DDouble* pNative	) const;

	//! Convert WGS84 coordinates to a PYXIS index.
	void wgs84ToPYXIS(	const CoordLatLon& ll,
						PYXIcosIndex* pIndex,
						int nResolution	) const;

	//! Convert a PYXIS index to WGS84 coordinates.
	void pyxisToWGS84(	const PYXIcosIndex& index,	
						CoordLatLon* pll	) const;

	//! Convert native coordinates to a PYXIS index.
	virtual void nativeToPYXIS(	const PYXCoord2DDouble& native,
								PYXIcosIndex* pIndex,
								int nResolution	) const;

	//! Convert a PYXIS index to native coordinates.
	virtual void pyxisToNative(	const PYXIcosIndex& index,
								PYXCoord2DDouble* pNative	) const;

	//! Convert a PYXIS index to native coordinates.
	virtual bool tryPyxisToNative(	const PYXIcosIndex& index,
								PYXCoord2DDouble* pNative	) const;

	//! Convert a native coordinate to a geocentric LatLon coordinates.
	virtual void nativeToLatLon(const PYXCoord2DDouble& native,
								CoordLatLon * pLatLon) const;

	//! Convert a native coordinate to a geocentric LatLon coordinates.
	virtual void latLonToNative(const CoordLatLon & latLon,
								PYXCoord2DDouble * pNative) const;

	/*!
	Determine if the native coordinate system is projected.

	\return	true if projected, otherwise false.
	*/
	bool isProjected() const
	{
		return true; //we need to check if this is true or not
	}

private:

	//! Copy constructor.
	BingCoordConverter(const BingCoordConverter&){}

	//! Copy assignment not implemented.
	BingCoordConverter& operator=(const BingCoordConverter&){
		return *this;
	}
};

#endif