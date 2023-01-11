#ifndef PYX_OGR_DATA_SOURCE_H
#define PYX_OGR_DATA_SOURCE_H
/******************************************************************************
pyx_ogr_data_source.h

begin		: 2004-10-19
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_gdal.h"

#include "coord_converter_impl.h"
#include "pyx_ogr_feature.h"

// pyxlib includes
#include "pyxis/data/feature_collection.h"
#include "pyxis/procs/srs.h"
#include "pyxis/utility/local_storage.h"
#include "pyxis/utility/rect_2d.h"
#include "pyxis/utility/tester.h"

#include "pyx_rtree.h"
//#include "vector_mapper.h"

// boost includes
#include <boost/thread/recursive_mutex.hpp>
#include <boost/algorithm/string/trim.hpp>

// standard includes
#include <set>
#include <iostream>
#include <fstream>

// local forward declarations
class CoordConverterImpl;
class PYXCell;
class PYXFeature;
class PYXGeocentricLatLonBounds;
class PYXGeometry;
class PYXTile;

// OGR forward declarations
class OGRDataSource;
class OGREnvelope;
class OGRFeature;
class OGRGeometry;
class OGRLayer;
class OGRLineString;
class OGRPolygon;

//! Helper class that enables thread-safe destruction of OGRFeature objects
class OGRFeatureObject : public PYXObject
{
private:
	OGRFeature * m_feature;

public:
	static PYXPointer<OGRFeatureObject> create(OGRFeature * feature)
	{
		return PYXNEW(OGRFeatureObject, feature);
	}
	OGRFeatureObject(OGRFeature* feature);
	virtual ~OGRFeatureObject();

	OGRFeature * GetFeature()
	{
		return m_feature;
	}

	OGRFeature * GetFeature() const
	{
		return m_feature;
	}
};

/*!
PYXOGRDataSource wraps an OGR data source to provide a PYXDataSource interface.
*/
//! Provides access to data sources through the OGR class library.
class PYXOGRDataSource : public IFeatureCollection
{
	friend class PYXSharedGDALDataSet;
	friend class OGRFeatureObject;
	friend class OgrPipeBuilder;
	friend class OGRProcess;
	friend class PYXOGRFeature;
	friend class PYXOGRAllFeaturesNativeIterator;

// TODO should have PYXCOM class etc.?

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IRecord)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public: // IRecord

	IRECORD_IMPL();

public: // IFeature

	virtual bool STDMETHODCALLTYPE isWritable() const
	{
		return false;
	}

	virtual const std::string& STDMETHODCALLTYPE getID() const
	{
		static std::string str("PYXOGRDataSource");
		return str;
	}

	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
	{
		return m_spGeometry;
	}

	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
	{
		return m_spGeometry;
	}

	virtual std::string STDMETHODCALLTYPE getStyle() const
	{
		return m_strStyle;
	}

	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const
	{
		PYXPointer<CSharpXMLDoc> styleDoc = CSharpXMLDoc::create(m_strStyle);
		return styleDoc->getNodeText("/style/" + strStyleToGet);
	}

	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const
	{
		return m_vecStyles;
	}	

public: // IFeatureCollection

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const
	{
		return getFeatureIterator();
	}

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const
	{
		return getFeatureIterator(geometry);
	}

	virtual bool STDMETHODCALLTYPE canRasterize() const 
	{
		return true;
	}

	IFEATURECOLLECTION_IMPL_HINTS();
	
public: // misc holding area

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const
	{
		return m_spFeatDefn;
	}

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition()
	{
		return m_spFeatDefn;
	}


	int getResolution() const
	{
		if (m_spGeometry)
		{
			return m_spGeometry->getCellResolution();
		}
		PYXTHROW(PYXException,"resolution is not set until data set was properly opened");
	}

public:

	// Property Tags
	static const std::string kstrScope;

	//! Destructor();
	virtual ~PYXOGRDataSource();

	//! Set the resolution of the data source and create the geometry.
	virtual void setResolution(int nResolution);

	//! Does the data source have a spatial reference system.
	virtual bool hasSpatialReferenceSystem() const;

	//! Specify the spatial reference system for the data source. 
	virtual void setSpatialReferenceSystem(PYXPointer<PYXSpatialReferenceSystem> spSRS);

	//! Return the class name of this observer class.
	virtual std::string getObserverDescription() const 
	{
		static std::string str("PYXOGRDataSource");
		return str;
	}

	//! Return the name of the notification class.
	virtual std::string getNotifierDescription() const {return getObserverDescription();}

	//! Open the data source.
	void open(PYXPointer<PYXSharedGDALDataSet> pOGRDataSource,
			  boost::intrusive_ptr<ISRS> spSRS,
			  bool flipAxis,
			  int nLayerIndex,
			  const std::string & sLayerName,
			  PYXPointer<PYXLocalStorage> storage);

	//! Get an iterator to the features.
	virtual PYXPointer<FeatureIterator> getFeatureIterator() const;

	//! Get an iterator to the features.
	virtual PYXPointer<FeatureIterator> getFeatureIterator(
		const PYXGeometry& geometry	) const;

	//! Get an iterator to the features.
	virtual PYXPointer<FeatureIterator> getFeatureIterator(
		const std::string& strWhere	) const;

	//! Get an iterator to the features.
	virtual PYXPointer<FeatureIterator> getFeatureIterator(
		const PYXGeometry& geometry,
		const std::string& strWhere	) const;

	//! Get a specific feature based on its ID.
	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(
		const std::string& strFeatureID) const;

	virtual void setStyle(const std::string& strStyle)
	{
		m_strStyle = strStyle;
	}

	//! Reading lock - set to true when feature iteration is in progress
	void setReadingLock(bool bLock)
	{
		m_bReadingLock = bLock;
	}

protected:

	//! Add the metadata for the data set.
	static void addDataSetMetadata(
		IRecord* pRecord,
		PYXPointer<PYXSharedGDALDataSet> pGDALDataSet,
		OGRLayer* pLayer,
		GIntBig nFeatures);

	//! Add the definition for the data set's content
	static void addContentDefinition(OGRLayer* pOGRLayer, PYXPointer<PYXTableDefinition> pTableDefn);

	//! Check min and max y bounds to ensure they are correct.
	static void PYXOGRDataSource::correctOGREnvelope(OGREnvelope* pBounds);

	//! Get the bounds of the data set in native coordinates.
	virtual const PYXRect2DDouble& getBounds() const {return m_bounds;}

	//! Get a specific ogr feature based on its ID.
	boost::intrusive_ptr<OGRFeatureObject> getOGRFeature(int nID) const;

	//! Get a specific feature based on its ID.
	boost::intrusive_ptr<IFeature> getFeature(int nID) const;

	//! Try to set a spatial reference system for a coordinate converter from a well known text without throwing an exception
	bool safeSetSpatialReferenceSystemFromWkt(boost::intrusive_ptr<CoordConverterImpl>& coordConverter, const std::string& srsWkt) const;

private:

	//! Constructor
	PYXOGRDataSource();

	//! Disable copy constructor
	PYXOGRDataSource(const PYXOGRDataSource&);

	//! Disable copy assignment
	void operator=(const PYXOGRDataSource&);

	//! Build an rTree for the layer
	void buildRTree(	const std::string& strName,
						const std::string& strLayerName,
						int nLayerIndex	);

	//! Create PYXIS geometry for the data source.
	void createGeometry(int nResolution);

	//! Calculate the bounding rectangles that cover the specified PYXIS geometry.
	void calcBoundingRects(	const PYXGeometry* pGeometry,
							PYXRect2DDouble* pRect1,
							PYXRect2DDouble* pRect2	) const;

	//! Calculate the bounding rectangles that cover the specified PYXIS cell.
	void calcCellBoundingRects(	const PYXCell& cell,
								PYXRect2DDouble* pRect1,
								PYXRect2DDouble* pRect2	) const;

	//! Calculate the bounding rectangles that cover the specified PYXIS tile.
	void calcTileBoundingRects(	const PYXTile& tile,
								PYXRect2DDouble* pRect1,
								PYXRect2DDouble* pRect2	) const;

	//! Calculate the minimum distance for a line string.
	double calcMinDistance(const OGRLineString& ogrLineString);

	//! Calculate the minimum distance for a polygon.
	double calcMinDistance(const OGRPolygon& ogrPolygon);

	//! Determine a reasonable resolution for the data. Read from specified file if it exists.
	int determineResolution(std::string& strFileName = std::string());

	//!	Initialize the metadata for the data source.
	void initMetaData();
	
private:

	//! Mutex to serialize concurrent access by multiple threads
	mutable boost::recursive_mutex m_mutex;

	//! The bounds in native coordinates
	mutable PYXRect2DDouble m_bounds;

	//! The OGR data source
	PYXPointer<PYXSharedGDALDataSet> m_pOGRDataSource;

	//! The OGR layer (OGR Manual: should never be destroyed by application, unless a temporary layer was created)
	OGRLayer* m_pOGRLayer;

	//! The AutoCAD layer name to use in attribute filters.
	std::string m_strAutoCADLayerName;

	//! The Geometry
	PYXPointer<PYXGeometry> m_spGeometry;

	//! The coordinate converter
	boost::intrusive_ptr<CoordConverterImpl> m_internalCoordConverter;

	//! The coordinate converter we are going to use (could be axis flip)
	boost::intrusive_ptr<ICoordConverter> m_spCoordConverter;

	PYXPointer<PYXLocalStorage> m_localStorage;

	//! rTree used for spatial query optimization
	//PYXrTree m_rTree;

	//! The feature definition.
	PYXPointer<PYXTableDefinition> m_spFeatDefn;	

	//! The styles associated with the feature.
	mutable std::vector<FeatureStyle> m_vecStyles;

	//! Style to be used to visualize features.
	std::string m_strStyle;

private:

	//! typedef for a set of OGR features
	typedef std::set<boost::intrusive_ptr<IFeature> > FeatureSet;

private:

	bool m_addExtentToMetadata;
	bool m_axisFlip;

	//! Set to true when a feature iteration is in progress
	mutable bool m_bReadingLock;

	/*!
	The PYXFeatureIterator iterates over a collection of features.
	*/
	//! Abstract base for classes that iterate over PYXIS features.
	class PYXOGRFeatureIterator : public FeatureIterator
	{
	public:

		//! Dynamic creator
		static PYXPointer<PYXOGRFeatureIterator> create(
			boost::shared_ptr<FeatureSet> psetFeature,
			PYXPointer<const PYXTableDefinition> spDefn,
			int nResolution	)
		{
			return PYXNEW(PYXOGRFeatureIterator, psetFeature, spDefn, nResolution);
		}

		//! Constructor
		PYXOGRFeatureIterator(
			boost::shared_ptr<FeatureSet> psetFeature,
			PYXPointer<const PYXTableDefinition> spDefn,
			int nResolution	);

		//! Destructor
		virtual ~PYXOGRFeatureIterator();

	public: // FeatureIterator

		//! Move to the next feature.
		virtual void next();

		//! See all the features have been.
		virtual bool end() const;

		//! Get the current PYXIS feature.
		virtual boost::intrusive_ptr<IFeature> getFeature() const;

	private:

		//! The set of features
		boost::shared_ptr<FeatureSet> m_spsetFeature;

		//! The feature definition
		PYXPointer<const PYXTableDefinition> m_spDefn;

		//! The resolution at which to create the geometry.
		int m_nResolution;

		//! The PYXIS feature
		boost::intrusive_ptr<IFeature> m_spFeature;

		//! The current iterator
		FeatureSet::const_iterator m_it;

		//! The end iterator
		FeatureSet::const_iterator m_itEnd;		
	};


	/*!
	The PYXFeatureIterator iterates over a collection of features.
	*/
	//! Abstract base for classes that iterate over PYXIS features.
	class PYXOGRFeatureLazyIterator : public FeatureIterator
	{
	public:

		//! Dynamic creator
		static PYXPointer<PYXOGRFeatureLazyIterator> create(
			boost::shared_ptr<PYXrTree::KeyList> psetFeature,
			boost::intrusive_ptr<const PYXOGRDataSource> spDataSource,
			PYXPointer<const PYXTableDefinition> spDefn,
			int nResolution	)
		{
			return PYXNEW(PYXOGRFeatureLazyIterator, psetFeature, spDataSource, spDefn, nResolution);
		}

		//! Constructor
		PYXOGRFeatureLazyIterator(
			boost::shared_ptr<PYXrTree::KeyList> psetFeature,
			boost::intrusive_ptr<const PYXOGRDataSource> spDataSource,
			PYXPointer<const PYXTableDefinition> spDefn,
			int nResolution	);

		//! Destructor
		virtual ~PYXOGRFeatureLazyIterator();

	public: // FeatureIterator

		//! Move to the next feature.
		virtual void next();

		//! See all the features have been.
		virtual bool end() const;

		//! Get the current PYXIS feature.
		virtual boost::intrusive_ptr<IFeature> getFeature() const;

	private:

		void createFeature();

		//! The set of features
		boost::shared_ptr<PYXrTree::KeyList> m_spsetFeature;

		//! The data source to create features from
		boost::intrusive_ptr<const PYXOGRDataSource> m_spDataSource;

		//! The feature definition
		PYXPointer<const PYXTableDefinition> m_spDefn;

		//! The resolution at which to create the geometry.
		int m_nResolution;

		//! The PYXIS feature
		boost::intrusive_ptr<IFeature> m_spFeature;

		//! The current iterator
		PYXrTree::KeyList::const_iterator m_it;

		//! The end iterator
		PYXrTree::KeyList::const_iterator m_itEnd;
	};

	/*!
	The PYXFeatureIterator iterates over a collection of features.
	*/
	//! Abstract base for classes that iterate over PYXIS features.
	class PYXOGRAllFeaturesNativeIterator : public FeatureIterator
	{
	public:

		//! Dynamic creator
		static PYXPointer<PYXOGRAllFeaturesNativeIterator> create(			
			boost::intrusive_ptr<PYXOGRDataSource> spDataSource,
			PYXPointer<const PYXTableDefinition> spDefn,
			int nResolution	)
		{
			return PYXNEW(PYXOGRAllFeaturesNativeIterator, spDataSource, spDefn, nResolution);
		}

		//! Constructor
		PYXOGRAllFeaturesNativeIterator(			
			boost::intrusive_ptr<PYXOGRDataSource> spDataSource,
			PYXPointer<const PYXTableDefinition> spDefn,
			int nResolution	);

		//! Destructor
		virtual ~PYXOGRAllFeaturesNativeIterator();

	public: // FeatureIterator

		//! Move to the next feature.
		virtual void next();

		//! See all the features have been.
		virtual bool end() const;

		//! Get the current PYXIS feature.
		virtual boost::intrusive_ptr<IFeature> getFeature() const;

	private:

		void createFeature();
		
		//! The data source to create features from
		boost::intrusive_ptr<PYXOGRDataSource> m_spDataSource;

		//! The feature definition
		PYXPointer<const PYXTableDefinition> m_spDefn;

		//! The resolution at which to create the geometry.
		int m_nResolution;

		//! The PYXIS feature
		boost::intrusive_ptr<IFeature> m_spFeature;

		//! The current iterator
		PYXrTree::KeyList::const_iterator m_it;

		//! The end iterator
		PYXrTree::KeyList::const_iterator m_itEnd;
	};

public:
	//! Test method
	static void test();
};

#endif
