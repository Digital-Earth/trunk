#ifndef FEATURE_PROCESSING_PROCS__CONCAT_FEATURES_H
#define FEATURE_PROCESSING_PROCS__CONCAT_FEATURES_H

/******************************************************************************
concat_features.h

begin		: 2012-06-11
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/data/feature_collection.h"

/*!
Inputs: AOI feature, collection of data features
Output: collection of data features, each of which having a geometry that is the intersection with the AOI
*/
//! Filters out features not equal to an AOI.
// TODO: Modify this class to do its computation as lazily as possible.  See Equals and Intersects.
class MODULE_FEATURE_PROCESSING_PROCS_DECL ConcatFeatures : public ProcessImpl<ConcatFeatures>, public IFeatureCollection
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
		if (m_inputFCs && m_inputFCs->size()>0)
		{
			return (*m_inputFCs)[0]->getStyle();
		}
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
	ConcatFeatures();

	//! Destructor
	~ConcatFeatures();

	//! Test
	static void test();

private:
	typedef std::vector<boost::intrusive_ptr<IFeatureCollection>> FeatureCollectionVector;

private:
	class Iterator : public FeatureIterator
	{
	public:
		static PYXPointer<FeatureIterator> create(const boost::shared_ptr<FeatureCollectionVector> inputs,const PYXPointer<PYXGeometry> & geometry)
		{
			return PYXNEW(Iterator,inputs,geometry);
		}

	private:
		Iterator(const boost::shared_ptr<FeatureCollectionVector> inputs,const PYXPointer<PYXGeometry> & geometry);

	public:
		virtual bool end() const;

		virtual void next();

		virtual boost::intrusive_ptr<IFeature> getFeature() const;

	private:
		void findNextNonEmptyDataset();

	private:
		boost::shared_ptr<FeatureCollectionVector> m_inputs;

		PYXPointer<PYXGeometry> m_geometry;

		FeatureCollectionVector::iterator m_currentDataset;

		int m_curentDatasetIndex;

		PYXPointer<FeatureIterator> m_currentDatasetIterator;
	};

	class Feature : public IFeature
	{
	public:
		static boost::intrusive_ptr<IFeature> create(const boost::intrusive_ptr<IFeature> & inputFeature,int datasetIndex)
		{
			return new Feature(inputFeature,StringUtils::toString(datasetIndex)+"-"+inputFeature->getID());
		}

	private:
		Feature(const boost::intrusive_ptr<IFeature> & inputFeature,const std::string & newID) : m_inputFeature(inputFeature), m_newID(newID)
		{
		}

	public: //IUNKNOWN
		IUNKNOWN_QI_BEGIN
			IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_END

		IUNKNOWN_RC_IMPL();

	public: // IRecord
		IRECORD_IMPL_PROXY(*m_inputFeature);

	public: // IFeature
		virtual bool STDMETHODCALLTYPE isWritable() const
		{
			return false;
		}

		virtual const std::string& STDMETHODCALLTYPE getID() const
		{
			return m_newID;
		}

		virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
		{
			return m_inputFeature->getGeometry();
		}

		virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
		{
			return m_inputFeature->getGeometry();
		}

		virtual std::string STDMETHODCALLTYPE getStyle() const
		{
			return m_inputFeature->getStyle();
		}

		virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const
		{
			return m_inputFeature->getStyle(strStyleToGet);
		}

	private:
		boost::intrusive_ptr<IFeature> m_inputFeature;

		std::string m_newID;
	};

private:

	PYXPointer<PYXGeometry> m_spGeometry;

	//! The input feature collection.
	boost::shared_ptr<FeatureCollectionVector> m_inputFCs;

	PYXPointer<PYXTableDefinition> m_featuresDefinition;
};

#endif
