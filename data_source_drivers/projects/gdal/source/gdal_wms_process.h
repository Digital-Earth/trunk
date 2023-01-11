#ifndef GDAL_WMS_PROCESS_H
#define GDAL_WMS_PROCESS_H

/******************************************************************************
gdal_wms_process.h

begin      : 1/25/2008 9:57:18 AM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_gdal.h"
#include "coord_converter_impl.h"
#include "gdal_metadata.h"
#include "gdal_xy_coverage.h"
#include "ows_reference.h"
#include "pyx_rtree.h"

// pyxlib includes
#include "pyxis/derm/horizontal_datum.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/path.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_iterator.h"
#include "pyxis/procs/user_credentials_provider.h"

// GDAL includes
#include "ogr_spatialref.h"

// boost includes
#include <boost/filesystem/path.hpp>
#include <boost/thread/recursive_mutex.hpp>

// local forward declarations
class ProjectionMethod;
class PYXGeometry;
class GDALXYCoverage;
class PYXSpatialReferenceSystem;

/*!
Provides access to WMS data through the GDAL library.
*/
class MODULE_GDAL_DECL GDALWMSProcess : public ProcessImpl<GDALWMSProcess>, public XYCoverageBase, IOWSReference
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
		IUNKNOWN_QI_CASE(IOWSReference)
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

public: //IOWSReference
	virtual PYXPointer<OWSFormat> getDefaultOutputFormat() const;
	virtual bool supportOutputType(const OWSFormat & format) const;
	virtual std::string getOWSReference(ReferenceType referenceType, const OWSFormat & format) const;

private:

	//! Creates an XML request for a WMS data source.
	std::string createWMSXML() const;

	//! Calculate the minimum resolution in native units/pixel.
	double calculateMinimumResolution(const PYXRect2DDouble& rect) const;

private:	

	//! The parent coverage. This is the WMS data source at max resolution.
	boost::intrusive_ptr<GDALXYCoverage> m_spParentXYCoverage;

	//! crednetails to use (if any)
	boost::intrusive_ptr<IUsernameAndPasswordCredentials> m_credentials;

	//! The service name (e.g. "WMS", "AGS" etc.)
	std::string m_strServiceName;

	//! The name of the server serving the WMS datasource.
	std::string m_strServer;

	//! The name of the layer (datasource) being retrieved from the server.
	std::string m_strLayer;

	//! type of image format being retrieved from the server.
	std::string m_strFormat;

	//! spatial reference system to use.
	std::string m_strSrs;
	
	//! Styles in which the layer is to be rendered.
	std::string m_strStyles;

	//! The minimum resolution of the WMS data source.
	std::string m_strMinResolution;

	//! The maximum resolution of the WMS data source.
	std::string m_strMaxResolution;

	//! The minimum latitude of the data source.
	std::string m_strMinLat;

	//! The maximum latitude of the data source.
	std::string m_strMaxLat;

	//! The minimum longitude of the data source.
	std::string m_strMinLon;

	//! The maximum longitude of the data source.
	std::string m_strMaxLon;

	//! The rtree to store file coordintates.
	mutable std::map<int, boost::shared_ptr<PYXrTree> > m_mapRTrees;

	//! Mutex for thread safety.
	mutable boost::recursive_mutex m_mutex;

	//! The default resolution for geographic coordinate systems
	static const double kfDefaultGeographicResolutionInDegrees;

	//! The default PYXIS resolution when for calculating units/pixel
	static const int knDefaultPyxisResolution;

	//! The maximum number of pixels in the width or height dimension for an overview
	static const int knMaxOverviewWidthOrHeightPixels;

private:
	class WmsImplementation : public PYXObject
	{
	public:
		virtual void tileLoadHint(const PYXTile& tile) const = 0;
		virtual void tileLoadHintDone(const PYXTile& tile) const = 0;
		virtual boost::intrusive_ptr<IXYCoverage> getCoverage() const = 0;
	};

	class WmsRequestPerTileImplementation : public WmsImplementation
	{
	public:
		WmsRequestPerTileImplementation(GDALWMSProcess & parent);
		
		static PYXPointer<WmsRequestPerTileImplementation> create(GDALWMSProcess & parent)
		{
			return PYXNEW(WmsRequestPerTileImplementation,parent);
		}

	public:
		virtual void tileLoadHint(const PYXTile& tile) const;
		virtual void tileLoadHintDone(const PYXTile& tile) const;
		virtual boost::intrusive_ptr<IXYCoverage> getCoverage() const;

	private:
		class WmsRequestInfo : public PYXObject
		{
		public:
			std::string url;
			PYXRect2DDouble area;
			PYXCoord2DInt imageSize;
			boost::filesystem::path imagesPath;
			boost::filesystem::path oldImagesPath;

		public:
			WmsRequestInfo() {};

			static PYXPointer<WmsRequestInfo> create()
			{
				return PYXNEW(WmsRequestInfo);
			}
		};

	private:
		mutable std::list<PYXPointer<WmsRequestInfo>> m_requests;

		//! The current data source.
		mutable boost::intrusive_ptr<IXYCoverage> m_spCurrentDataSrc;
		
		//! The service name for GDAL (i.e. WMS, AGS...)
		std::string m_strServiceName;
		std::string m_baseUrl;
		std::string m_fileExtension;
		PYXRect2DDouble m_boundary;
		std::string m_srs;

		//! credentials to use (if any)
		boost::intrusive_ptr<IUsernameAndPasswordCredentials> m_credentials;

		//! Coordinate converter to use (ownership retained by xyCoverage)
		const ICoordConverter* m_coordConverter;

		std::list<PYXPointer<WmsRequestInfo>> createRequests(const PYXTile & tile) const;

		std::string createSingleFileVrt(const PYXPointer<WmsRequestInfo> & request) const;
		std::string createTwoFilesVrt(const PYXPointer<WmsRequestInfo> & request1,const PYXPointer<WmsRequestInfo> & request2) const;
	};

	class WmsGdalOverviewImplementation : public WmsImplementation
	{
	public:
		WmsGdalOverviewImplementation(const boost::intrusive_ptr<GDALXYCoverage> & parent);

		static PYXPointer<WmsGdalOverviewImplementation> create(const boost::intrusive_ptr<GDALXYCoverage> & parent)
		{
			return PYXNEW(WmsGdalOverviewImplementation,parent);
		}

	public:
		virtual void tileLoadHint(const PYXTile& tile) const;
		virtual void tileLoadHintDone(const PYXTile& tile) const;
		virtual boost::intrusive_ptr<IXYCoverage> getCoverage() const;

	private:
		mutable std::vector<boost::intrusive_ptr<GDALXYCoverage>> m_spOverviews;

		//! The current data source.
		mutable boost::intrusive_ptr<IXYCoverage> m_spCurrentDataSrc;
	};

private:
	PYXPointer<WmsImplementation> m_implementation;
};

#endif