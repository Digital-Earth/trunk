#ifndef RESOLUTION_FILTER_H
#define RESOLUTION_FILTER_H
/******************************************************************************
resolution_filter.h

begin		: June 24, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/data/feature_collection.h"
#include "pyxis/pipe/process.h"

//! A process that filters a feature collection by resolution.
class MODULE_FEATURE_PROCESSING_PROCS_DECL FeatureCollectionResolutionFilter : 
	public ProcessImpl<FeatureCollectionResolutionFilter>, public IFeatureCollection
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	FeatureCollectionResolutionFilter();

	//! Destructor
	~FeatureCollectionResolutionFilter();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IFeature)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST(FeatureCollectionResolutionFilter, IProcess);

public: // IRecord

	IRECORD_IMPL();

public: // IFeature

	IFEATURE_IMPL();

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

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

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

	int m_nMinRelRes;
	int m_nMaxRelRes;
	int m_nMinAbsRes;
	int m_nMaxAbsRes;

	int m_nMinRes;
	int m_nMaxRes;
	boost::intrusive_ptr<IFeatureCollection> m_spInputFC;
};

#endif // guard
