#ifndef FEATURE_PROCESSING_PROCS__SELECT_FEATURE_BY_ID_H
#define FEATURE_PROCESSING_PROCS__SELECT_FEATURE_BY_ID_H

/******************************************************************************
select_feature_by_id.h

begin		: 2013-04-18
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/data/feature_collection.h"

class FeatureCollectionProcess;

/*!
Inputs: a feature collection
Output: a single feature.
*/
//! Outputs only a single feature by the given feature ID
class MODULE_FEATURE_PROCESSING_PROCS_DECL SelectFeatureByIdProcess : public ProcessImpl<SelectFeatureByIdProcess>
{
	PYXCOM_DECLARE_CLASS();

public: // PYXCOM_IUnknown
	 
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)		
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IProcess 

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IFeature*>(m_outputFeature.get());
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return m_outputFeature;
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

private:

	//! The output feature.
	boost::intrusive_ptr<IFeature> m_outputFeature;

	//! The output feature ID.
	std::string m_featureID;
};

#endif
