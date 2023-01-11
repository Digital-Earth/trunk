#ifndef FEATURE_PROCESSING_PROCS__UNION_H
#define FEATURE_PROCESSING_PROCS__UNION_H

/******************************************************************************
union.h

begin		: 2008-02-01
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/data/feature_collection.h"

class FeatureCollectionProcess;

/*!
Inputs: AOI feature, collection of data features
Output: collection of data features, each of which having a geometry that is the union with the AOI
*/
//! Filters out features not equal to an AOI.
// TODO: Modify this class to do its computation as lazily as possible.  See Equals and Intersects.
class MODULE_FEATURE_PROCESSING_PROCS_DECL Union : public ProcessImpl<Union>, public IFeatureCollection
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
		return m_spFeaturesInput->isWritable();
	}

	virtual const std::string& STDMETHODCALLTYPE getID() const
	{
		return m_strID;
	}

	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
	{
		return m_spAOIGeometry;
	}

	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
	{
		return m_spAOIGeometry;
	}

	virtual std::string STDMETHODCALLTYPE getStyle() const
	{
		return m_spFeaturesInput->getStyle();
	}

	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const
	{
		return m_spFeaturesInput->getStyle(strStyleToGet);
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
	Union();

	//! Destructor
	~Union();

	//! Test
	static void test();

private:

	//! Populate the output field.
	void populateOutput() const;

private:

	//! The input AOI geometry.
	PYXPointer<PYXGeometry> m_spAOIGeometry;

	//! The input feature collection.
	boost::intrusive_ptr<IFeatureCollection> m_spFeaturesInput;

	//! The output.
	boost::intrusive_ptr<IFeatureCollection> m_spOutput;

	//! Whether or not the output is populated.
	mutable bool m_bOutputPopulated;
};

#endif
