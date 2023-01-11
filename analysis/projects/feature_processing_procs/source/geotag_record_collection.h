#ifndef GEOTAG_RECORD_COLLECTION_H
#define GEOTAG_RECORD_COLLECTION_H

/******************************************************************************
geotag_record_collection.h

begin		: 2013-06-11
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/data/feature_collection.h"
#include "pyxis/data/record_collection.h"
#include "pyxis/procs/geometry_provider.h"

/*!
Inputs: Geometry Provider, collection of data records
Output: collection of data features, each of which having a geometry that is provided by the Geometry provider
*/

class MODULE_FEATURE_PROCESSING_PROCS_DECL GeotagRecordCollection : public ProcessImpl<GeotagRecordCollection>, public IFeatureCollection
{
	PYXCOM_DECLARE_CLASS();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IFeature)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IRecord

	IRECORD_IMPL();

protected: // IFeature

	mutable std::string m_strID;

public: // IFeature

	virtual bool STDMETHODCALLTYPE isWritable() const
	{
		return false;
	}

	virtual const std::string& STDMETHODCALLTYPE getID() const
	{
		return m_strID;
	}

	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
	{
		return m_spGeometry;
	}

	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
	{
		return m_spGeometry;
	}

	virtual std::string STDMETHODCALLTYPE getStyle() const
	{
		return "";
	}

	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const
	{
		std::string style = getStyle();
		if (style == "")
		{
			return "";
		}
		PYXPointer<CSharpXMLDoc> styleDoc = CSharpXMLDoc::create(style);
		return styleDoc->getNodeText("/style/" + strStyleToGet);
	}

public: // IProcess 

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IFeatureCollection*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IFeatureCollection*>(this);
	}

	//! Get the attributes in this process.
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const
	{
		return std::map<std::string, std::string>();
	}

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // IFeatureCollection

	//! Get an iterator to all the features in this collection.
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const;

	//! Get an iterator to all the features in this collection that intersect this geometry.
	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const;

	//! Get styles that determine how to visualize features in this collection.
	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const;

	//! Get the feature with the specified ID.
	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(const std::string& strFeatureID) const;

	//! Get the feature definition.
	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const;

	//! Get the feature definition.
	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition();

	virtual bool STDMETHODCALLTYPE canRasterize() const;

	IFEATURECOLLECTION_IMPL_HINTS();

public:

	//! Default Constructor
	GeotagRecordCollection();

	//! Destructor
	~GeotagRecordCollection();

	//! Test
	static void test();

private:
	typedef std::vector<boost::intrusive_ptr<IFeatureCollection>> FeatureCollectionVector;

private:
	class Iterator : public FeatureIterator
	{
	public:
		static PYXPointer<FeatureIterator> create(const boost::intrusive_ptr<IRecordCollection> & inputs,
			const boost::intrusive_ptr<IGeometryProvider> & geometryProvider)
		{
			return PYXNEW(Iterator,inputs,geometryProvider);
		}

	private:
		Iterator(const boost::intrusive_ptr<IRecordCollection> & inputs, 
			const boost::intrusive_ptr<IGeometryProvider> & geometryProvider);

	public:
		virtual bool end() const;

		virtual void next();

		virtual boost::intrusive_ptr<IFeature> getFeature() const;

	private:
		PYXPointer<RecordIterator> m_recordIterator;
		mutable PYXPointer<IFeature> m_currentFeature;
		int m_currentID;
		boost::intrusive_ptr<IGeometryProvider> m_geometryProvider;
	};

	class Feature : public IFeature
	{
	public:
		static boost::intrusive_ptr<IFeature> create(const boost::intrusive_ptr<IRecord> & inputRecord, PYXPointer<PYXGeometry> & geometry,const std::string & newID)
		{
			return new Feature(inputRecord, geometry, newID);
		}

	private:
		Feature(const boost::intrusive_ptr<IRecord> & inputRecord, PYXPointer<PYXGeometry> & geometry ,const std::string & newID) : m_inputRecord(inputRecord),m_geometry(geometry), m_id(newID)
		{
		}

	public: //IUNKNOWN
		IUNKNOWN_QI_BEGIN
			IUNKNOWN_QI_CASE(IFeature)
			IUNKNOWN_QI_CASE(IRecord)
		IUNKNOWN_QI_END

		IUNKNOWN_RC_IMPL();

	public: // IRecord
		IRECORD_IMPL_PROXY(*m_inputRecord);

	public: // IFeature
		virtual bool STDMETHODCALLTYPE isWritable() const
		{
			return false;
		}

		virtual const std::string& STDMETHODCALLTYPE getID() const
		{
			return m_id;
		}

		virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
		{
			return m_geometry;
		}

		virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
		{
			return m_geometry;
		}

		virtual std::string STDMETHODCALLTYPE getStyle() const
		{
			return "";
		}

		virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const
		{
			return "";
		}

	private:
		boost::intrusive_ptr<IRecord> m_inputRecord;
		PYXPointer<PYXGeometry> m_geometry;
		std::string m_id;
	};

private:

	PYXPointer<PYXGeometry> m_spGeometry;
	PYXPointer<PYXTableDefinition> m_featuresDefinition;
	boost::intrusive_ptr<IRecordCollection> m_inputRC;
	boost::intrusive_ptr<IGeometryProvider> m_geometryProvider;
};

#endif
