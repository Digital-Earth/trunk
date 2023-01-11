/******************************************************************************
gdal_xy_coverage.h

begin      : 9/25/2007 2:31:14 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#pragma once

// local includes
#include "coord_converter_impl.h"
#include "gdal_pipe_builder.h"

// pyxlib includes
#include "pyxis/data/record.h"
#include "pyxis/derm/horizontal_datum.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/srs.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_iterator.h"
#include "pyxis/region/xy_bounds_region.h"
#include "pyxis/utility/abstract_iterator.h"
#include "pyxis/utility/coord_2d.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/value.h"
#include "pyxis/utility/memory_manager.h"
#include "pyxis/utility/cache_map.h"

// GDAL includes
#include "gdal_metadata.h"
#include "gdal_priv.h"
#include "module_gdal.h"
#include "ogr_spatialref.h"

// boost includes
#include <boost/filesystem/path.hpp>
#include <boost/thread/recursive_mutex.hpp>

// local forward declarations
class GDALColorTable;
class GDALDataset;
class ProjectionMethod;
class PYXGeometry;
class PYXValue;
class GDALRasterBand;

/*!
GDALIterator iterates over points retrieved through the GDAL Library
*/
//! Iterates over GDAL Data Sources.
class GDALDataIterator : public PYXObject, public PYXAbstractIterator
{

public:

	//! Destructor
	virtual ~GDALDataIterator() {}

	/*!
	Get the value at the current iteration.

	\return	The value.
	*/
	//! Get the value at the current position.
	virtual PYXValue getValue() const = 0;

	//!	Get the raster coordinates of the current point.
	virtual const PYXCoord2DInt& getXY() const = 0;

};

class GDALXYCoverage;

//! XYCoverageValueGetter optimized for non CLUT data
class MODULE_GDAL_DECL XYCoverageValueGetterNoCLUT : public XYCoverageValueGetter
{
public:
	//! Constructor
	XYCoverageValueGetterNoCLUT();

	void STDMETHODCALLTYPE setXYCoverageValue (GDALXYCoverage* pXYCoverage);

	virtual bool STDMETHODCALLTYPE getCoverageValue(const PYXCoord2DDouble& native,
		PYXValue* pValue) const;

private:
	GDALXYCoverage* m_pXYCoverage;
};

/*!
The XY Coverage used by the GDALFileProcess
*/
class MODULE_GDAL_DECL GDALXYCoverage: public XYCoverageBase
{
	friend class GDALXYAsyncValueGetter;
	friend class GDALBingProcess;

	PYXCOM_DECLARE_CLASS();

public:

	//! Properties
	static const std::string kstrScope;

	//! The maximum count to retry for. 
	static const int knMaxRetryCount;

	//! Time to wait inbetween retries to read data.
	static const int knWaitBetweenRetries;

	//! Constructor
	GDALXYCoverage();

	//!Destructor
	virtual ~GDALXYCoverage();


public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(PYXCOM_IUnknown)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IXYCoverage)
		IUNKNOWN_QI_END

		IUNKNOWN_RC_IMPL();

	IUNKNOWN_DEFAULT_CAST( GDALXYCoverage, IXYCoverage);

public: // IRecord

	IRECORD_IMPL();

public: // IFeature

	IFEATURE_IMPL();

public: // IFeatureCollection

	IFEATURECOLLECTION_IMPL_WITHOUT_HINTS();

public: // IXYCoverage

	virtual XYCoverageValueGetter* STDMETHODCALLTYPE getCoverageValueGetter() const;

	virtual PYXPointer<XYAsyncValueGetter> STDMETHODCALLTYPE getAsyncCoverageValueGetter(
		const XYAsyncValueConsumer & consumer,
		int matrixWidth,
		int matrixHeight
		) const;

	virtual bool STDMETHODCALLTYPE getCoverageValue(const PYXCoord2DDouble& native,
		PYXValue* pValue) const;

	virtual void STDMETHODCALLTYPE getMatrixOfValues(const PYXCoord2DDouble& nativeCentre,
		PYXValue* pValues,
		int sizeX,
		int sizeY) const;

	bool STDMETHODCALLTYPE getCoverageValueNoCLUT(const PYXCoord2DDouble& native,
		PYXValue* pValue) const;

	/*!
	Specify the spatial reference for the data source. Call this method to set
	the spatial reference if after the data source is opened
	hasSpatialReference() returns false.

	\param	spSRS	The spatial reference system.
	*/
	virtual void STDMETHODCALLTYPE setSpatialReferenceSystem(
		boost::intrusive_ptr<ISRS> spSRS	);


	//! Returns a coordinate converter used to create a geometry. (ownership retained)
	virtual const ICoordConverter* STDMETHODCALLTYPE getCoordConverter() const
	{
		if (m_coordConverterExternal)
		{
			return m_coordConverterExternal.get();
		}
		return m_coordConverter.get();
	}

	/*!
	Get the spatial precision of the data.

	\return The spatial precision in metres or -1 if unknown.
	*/
	virtual double STDMETHODCALLTYPE getSpatialPrecision() const;

	virtual const GDALMetaData * getMetaDataGDAL() const
	{
		return &m_metaDataGDAL;
	}

	virtual PYXCoord2DDouble STDMETHODCALLTYPE nativeToRasterSubPixel(const PYXCoord2DDouble & native) const
	{
		PYXCoord2DDouble raster;
		m_metaDataGDAL.nativeToRasterSubPixel(native, &raster);
		return raster;
	}

	virtual void STDMETHODCALLTYPE tileLoadHint (const PYXTile& tile) const;

	virtual void STDMETHODCALLTYPE tileLoadDoneHint (const PYXTile& tile) const;

	virtual void STDMETHODCALLTYPE geometryHint(PYXPointer<PYXGeometry> spGeom);

	virtual void STDMETHODCALLTYPE endGeometryHint(PYXPointer<PYXGeometry> spGeom);

	//GDAL specific interface
public:
	//! Open the underlying file.
	bool open(
		const std::string& strFileName,
		boost::intrusive_ptr<ISRS> spSRS,
		const std::vector<int>& bands,
		const std::string& strLayerName = ""	);

	//! Open the underlying file.
	bool openAsRGB(
		const std::string& strFileName,
		boost::intrusive_ptr<ISRS> spSRS,
		const std::string& strLayerName = ""	)
	{
		int rgb [] = {1,2,3};
		std::vector<int> bands (&rgb[0], &rgb[3]);
		return open(strFileName, spSRS, bands, strLayerName);
	}

	//! Open the underlying file using the default band(s)
	bool openAsDefault(
		const std::string& strFileName,
		boost::intrusive_ptr<ISRS> spSRS,
		const std::string& strLayerName = ""	)
	{
		// leave band vector empty to select the default band(s)
		std::vector<int> bands;
		return open(strFileName, spSRS, bands, strLayerName);
	}

	//! Add user defined null value
	void forceNullValue(const PYXValue & nullValue);

	//! Tracks limit on download size, in axis units.  Used for EOX's weird WCS implementation. -1 means no such limit.
	void setAxisLimit(int limit);

	bool hasOverview();

	//! open the next overview for this coverage.
	PYXPointer<GDALXYCoverage> openOverview();

	//! Calculate the context for an individual band.
	static PYXFieldDefinition::eContextType calcContext(
		PYXPointer<PYXSharedGDALDataSet> pGDALDataSource,
		GDALRasterBand* pBand,
		int nBandCount);

	//! Add the metadata for the data set.
	static void GDALXYCoverage::addDataSetMetadata(
		IRecord* pRecord,
		PYXPointer<PYXSharedGDALDataSet> pGDALDataSet,
		GDALRasterBand* pBand,
		PYXFieldDefinition::eContextType nContext);

	//! Handle the special GRIB metadata fields and record them with PYXIS keys.
	static void handleSpecialGRIBFields(
		IRecord* pRecord,
		const char* pszKey,
		const char* pszValue);

	//! Handle the special netCDF metadata fields and record them with PYXIS keys.
	static void handleSpecialNetCDFFields(
		IRecord* pRecord,
		const char* pszKey,
		const char* pszValue,
		const std::string& strNetCDFTimeReference);

	//! Add the definition for the data set's content
	static void GDALXYCoverage::addContentDefinition(
		PYXPointer<PYXTableDefinition> pTableDefn,
		const std::vector<int>& vecBands,
		const std::string& strFieldName,
		PYXFieldDefinition::eContextType nContext,
		GDALDataType nGDALDataType);

private:

	static int knBufferLimiter;

	//! Fill the buffer by reading from the datasource.
	int fillBuffer(int nBandBytes, int nLineBytes, int nPixelBytes,
		GByte *pabyBandData, const PYXRect2DInt& readBounds) const;

	//! load a buffer of raster data into memory.
	void loadBuffer (byte** ppBuffer, PYXRect2DInt* bufferBounds, const PYXRect2DInt& readBounds) const;

	//! retrieve a single value from a raster buffer.
	void readBuffer (byte* pReadBuffer, const PYXRect2DInt& bufferBounds, const PYXCoord2DInt& raster) const;

	//! Clear out the prefetch buffers used by the hint system.
	void clearBuffers() const;

	//! Open the data set.
	void open(PYXPointer<PYXSharedGDALDataSet> pGDALDataSource, const std::vector<int>& bands);

	//!	Get the data value for the given raster coordinates.
	bool getDataValue(const PYXCoord2DInt& raster, PYXValue* pValue) const;

	/*!
	Return the colour table associated with the data source.  There can only
	be a colour table for single band sources.

	\return A pointer to the colour table or 0 if no colour table exists.
	*/
	//! Return the colour table associated with the data source
	GDALColorTable* getColourTable() const {return m_pColourTable;}

	//! Get the data type.
	PYXValue::eType getDataType() const {return m_nDataType;}

	/*!
	Get the number of bands in the data set.

	\return	The number of bands.
	*/
	//! Return the number of bands that are in the GDALDataset
	int getBandCount() const {return m_vecSelectedBands.size();}

	//! Set which overview to read from.
	void setOverview(int nWhichOverview) { m_nOverview = nWhichOverview; }

	//! Get the raster band for this data source (using correct overview).
	GDALRasterBand* getRasterBand(int nBand) const;

	//! Get the context of the data.
	PYXFieldDefinition::eContextType getContext() const {return m_nContext;}

	//! Verify that all bands in the data set are compatible
	void verifyCompatibleBands();

	//! Determine and store the null value for the data set
	void determineNullValue();

	//! Store the null for a band in the null value buffer.
	void storeNullValueForBand(double fNullValue, int nBand);

	//! Calculate the spatial precision for projected coordinates.
	double calcSpatialPrecisionProjected() const;

	//! Calculate the spatial precision for geographic lat/lon coordinates.
	double calcSpatialPrecisionGeographical(HorizontalDatum const * pDatum) const;

	//! Get the value at the specified native raster coordinate.
	bool getValue(const PYXCoord2DInt& raster, PYXValue* pValue) const;

	//! Perform a table lookup.
	bool lookupValue(int nIndex, PYXValue* pValue) const;

	////! Read the underlying file.
	void readFile();

	//! Initialize the metadata.
	void initMetaData();

private:
	//! The OGR data source
	PYXPointer<PYXSharedGDALDataSet> m_pGDALDataSource;

	mutable XYCoverageValueGetterNoCLUT m_getterNoCLUT;

	//! Mutex to serialize concurrent access by multiple threads
	mutable boost::recursive_mutex m_mutex;

	//! The Metadata object used to determine geospatial information.
	GDALMetaData m_metaDataGDAL;

	//! The band iterator.
	PYXPointer<GDALDataIterator> m_spDataIterator;

	//! The coordinate converter.
	PYXPointer<CoordConverterImpl> m_coordConverter;

	PYXPointer<ICoordConverter> m_coordConverterExternal;

	//! Selected bands to open
	std::vector<int> m_vecSelectedBands;

	//! The type of data stored in the data set.
	PYXValue::eType m_nDataType;

	//! The context of the data.
	mutable PYXFieldDefinition::eContextType m_nContext;

	//! The number of pixels in a band (in the x direction).
	int m_nBandXSize;

	//! The number of lines in a band (in the y direction).
	int m_nBandYSize;

	//! The number of pixels in a block (in the x direction).
	int m_nBlockXSize;

	//! The number of lines in a block (in the y direction).
	int m_nBlockYSize;

	//! a rectangle that is the size in pixels of this data source.
	PYXRect2DInt m_dataRasterBounds;

	//! The GDAL data type.
	GDALDataType m_nGDALDataType;

	//! The size in bytes of the data values in the raster
	int m_nValueByteSize;

	//! Temporary storage for the GDAL value retrieved.
	mutable void* m_pGDALBuffer;

	mutable bool m_limitSize;

	//! Tracks limit on download size, in axis units.  Used for EOX's weird WCS implementation. -1 means no such limit.
	int m_axisLimit;

	//! Control access to the read buffer pointer.
	mutable boost::recursive_mutex m_bufferMutex;	

	//! The Null value.
	mutable void* m_pNullValue;

	//! Forced Null value.
	mutable PYXValue m_forcedNullValue;

	//! default value spec to use for this coverage.
	PYXValue m_coverageValueSpec;

	//! The colour table for the source 
	GDALColorTable* m_pColourTable;

	//! The overview.  -1 to read default raster band.
	int m_nOverview;

	//! a flag to specify if the coverage values wrap around
	bool m_wrapX;

private:

	class Buffer : public PYXObject, public ObjectMemoryUsageCounter<Buffer>
	{
	private:		
		byte *				m_pReadBuffer;

		PYXRect2DInt		m_readBufferBounds;

		GDALDataType		m_nGDALDataType;
		int					m_nBands;
		int					m_nPixelBytes;
		int					m_nLineBytes;
		int					m_nBandBytes;

	public:
		static PYXPointer<Buffer> create(const PYXRect2DInt & bounds,GDALDataType type,int nBands)
		{
			try
			{
				PYXPointer<Buffer> buffer = PYXNEW(Buffer);
				buffer->alloc(bounds,type,nBands);

				if (buffer->m_pReadBuffer == 0)
				{
					return 0;
				}

				return buffer;
			}
			catch (std::bad_alloc&)
			{
				return 0;
			}
		}

		Buffer();

		virtual ~Buffer();

		//allocate the buffer
		void alloc(const PYXRect2DInt & bounds,GDALDataType type,int nBands);

		//return true if read was successful
		bool read(const PYXCoord2DInt& raster,byte * dest);

		//pixel size in bytes
		int getPixelSize() const 
		{
			return m_nPixelBytes;
		}

		//line size in bytes
		int getLineSize() const 
		{
			return m_nLineBytes;
		}

		//pixel size in bytes
		int getBandSize() const 
		{
			return m_nBandBytes;
		}

		//buffer size in bytes
		int getBufferSize() const 
		{
			return m_nBandBytes * m_nBands;
		}

		byte * getBuffer() const 
		{
			return m_pReadBuffer;
		}

		const PYXRect2DInt & getReadBounds() const 
		{
			return m_readBufferBounds;
		}
	};

	/*
	class PixelRect : public PYXObject
	{
	private:
		static const int MinBlockSize = 256;
		static const int SubBlockCount = 8;

		PYXRect2DInt m_pixelRect;
		PYXPointer<PYXXYBoundsRegion> m_region;
		std::vector<PYXPointer<PixelRect>> m_subRects;
		int m_subRectX;
		int m_subRectY;


	public:
		static PYXPointer<PixelRect> create(const GDALMetaData & metaData,const PYXRect2DInt & pixelRect, const PYXRect2DDouble & bounds,const PYXPointer<CoordConverterImpl> & coordConverter)
		{
			return PYXNEW(PixelRect,metaData,pixelRect,bounds,coordConverter);
		}

		PixelRect(const GDALMetaData & metaData,const PYXRect2DInt & pixelRect, const PYXRect2DDouble & bounds,const PYXPointer<CoordConverterImpl> & coordConverter);

		PYXRegion::CellIntersectionState intersectTile(const PYXIcosIndex & index) const;
		bool isBigRegion() const;
		void getTileBounds(const PYXIcosIndex & index,PYXRect2DInt & rect1,PYXRect2DInt & rect2) const;

		void GDALXYCoverage::PixelRect::collectRects(const PYXIcosIndex & index,std::list<PYXRect2DInt> & rects) const;
		void getTileBounds2(const PYXIcosIndex & index,PYXRect2DInt & rect1,PYXRect2DInt & rect2) const;
	};
	*/

	class BufferCache
	{
	public:
		BufferCache();

		virtual ~BufferCache();		

	public:
		PYXPointer<Buffer> findContainingBuffer(const PYXRect2DInt& bufferBounds);
		PYXPointer<Buffer> findContainingBuffer(const PYXCoord2DInt & raster);
		void add(PYXPointer<Buffer> buffer);
		void freeLargeBuffers();

	private:
		static boost::mutex s_mutex;		
		static CacheMap<BufferCache*,std::list<PYXPointer<Buffer> > > s_allBufferCache;
	};


private:
	mutable BufferCache m_readBuffers;

	PYXPointer<Buffer> getReadBuffer(const PYXRect2DInt& bufferBounds) const;

	PYXPointer<Buffer> findContainingBuffer(const PYXRect2DInt& bufferBounds) const;

	void STDMETHODCALLTYPE getMatrixOfValues(const PYXPointer<Buffer> & buffer,
		const PYXCoord2DInt& raster,
		PYXValue* pValues,
		bool* pHasValues,
		int sizeX,
		int sizeY) const;

	bool getValue(const PYXPointer<Buffer> & buffer, const PYXCoord2DInt& raster, PYXValue* pValue) const;

	bool loadBuffer (const PYXPointer<Buffer> & buffer) const;

private:
	friend class GDALDSReaderIterator;

	friend class GDALPipeBuilder;
};

