#ifndef FEATURE_COLLECTION_FILTER_H
#define FEATURE_COLLECTION_FILTER_H
/******************************************************************************
feature_collection_filter.h

begin		: 2013-4-22
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_analysis_procs.h"

#include "pyxis/utility/thread_pool.h"
#include "pyxis/procs/feature_calculator.h"
#include "feature_condition_calculator.h"
#include "pyxis/data/feature_group.h"


/*!
This process converts a feature collection input into another feature collection filtering some of the features based on an input condition

*/

class MODULE_ANALYSIS_PROCS_DECL FeatureCollectionFilter : public ProcessImpl<FeatureCollectionFilter>, IFeatureCollection
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	FeatureCollectionFilter();

	//! Destructor
	~FeatureCollectionFilter();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_END

		IUNKNOWN_RC_IMPL_FINALIZE();

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

	virtual std::string STDMETHODCALLTYPE FeatureCollectionFilter::getAttributeSchema() const;

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();



public: // IRecord

	IRECORD_IMPL();

protected:  // IFeature
	
	bool m_bWritable; 
	mutable std::string m_strID; 

public:  // IFeature
	virtual bool STDMETHODCALLTYPE isWritable() const
	{
		return m_inputFC->isWritable();
	}
	virtual const std::string& STDMETHODCALLTYPE getID() const
	{
		if (m_strID == "") m_strID = "Uninitialized PYXIS Feature ID";
		return m_strID;
	}
	virtual PYXPointer<const PYXGeometry> STDMETHODCALLTYPE getGeometry() const
	{
		// what should this be ?
		return m_inputFC->getGeometry();
	}
	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry()
	{
		return m_inputFC->getGeometry();
	}
	virtual std::string STDMETHODCALLTYPE getStyle() const
	{
		return m_inputFC->getStyle();
	}
	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const
	{
		return m_inputFC->getStyle(strStyleToGet);
	}


public: // IFeatureCollection

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator() const;

	virtual PYXPointer<FeatureIterator> STDMETHODCALLTYPE getIterator(const PYXGeometry& geometry) const;

	virtual std::vector<FeatureStyle> STDMETHODCALLTYPE getFeatureStyles() const;

	virtual boost::intrusive_ptr<IFeature> STDMETHODCALLTYPE getFeature(const std::string& strFeatureID) const;

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition() const;

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getFeatureDefinition();

	virtual bool STDMETHODCALLTYPE canRasterize() const;

	IFEATURECOLLECTION_IMPL_HINTS();

public:
	static void test();

private:
	boost::intrusive_ptr<IFeatureCollection> m_inputFC;
	boost::intrusive_ptr<IFeatureGroup> m_inputFGroup;
	std::vector<boost::intrusive_ptr<IFeatureCalculator>> m_conditions;

};

#endif // guard
