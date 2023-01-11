#ifndef FEATURE_PROCESSING_PROCS__FIRST_NOT_NULL_GEOMETRY_PROVIDER_H
#define FEATURE_PROCESSING_PROCS__FIRST_NOT_NULL_GEOMETRY_PROVIDER_H
/******************************************************************************
first_not_null_geometry_provider.h

begin		: 2013-6-17
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_feature_processing_procs.h"

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/procs/geometry_provider.h"
#include "pyxis/data/feature_collection.h"
#include "pyxis/data/record.h"
#include "pyxis/pipe/process.h"



/*!
Given a set of geometry providers and a record. this class trys each geometry provider to find a geometry for the given record.
The first geometry that is returned from a geometry provider is selected as the geometry for the record
*/
class MODULE_FEATURE_PROCESSING_PROCS_DECL FirstNotNullGeometryProvider : public ProcessImpl<FirstNotNullGeometryProvider>, public IGeometryProvider
{
	PYXCOM_DECLARE_CLASS();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IGeometryProvider)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IProcess 

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IGeometryProvider*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IGeometryProvider*>(this);
	}

	//! Get the attributes in this process.
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const
	{
		return std::map<std::string, std::string>();
	}
	
	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr)
	{
	}

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public:

	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry(boost::intrusive_ptr<IRecord> & record) const;

private:

	std::vector<boost::intrusive_ptr<IGeometryProvider>> m_geometryProviders;
};

#endif // guard
