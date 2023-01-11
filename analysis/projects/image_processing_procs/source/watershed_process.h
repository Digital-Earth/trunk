#ifndef WATERSHED_PROCESS_H
#define WATERSHED_PROCESS_H
/******************************************************************************
watershed_process.h

begin		: 2015-09-07
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxis/data/feature.h"
#include "pyxis/data/coverage.h"
#include "pyxis/pipe/process.h"

// standard includes
#include <cassert>
#include <vector>

/*!
Apply a watershed analysis on the input geometry using a coverage as elevation data
*/
//! Apply a watershed analysis on the input geometry using a coverage as elevation data
class MODULE_IMAGE_PROCESSING_PROCS_DECL WatershedProcess : public ProcessImpl<WatershedProcess>, public IFeature
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	WatershedProcess();

protected:
	//! Destructor
	virtual ~WatershedProcess();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IFeature*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IFeature*>(this);
	}

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

	//! Get the attributes in this process.
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	//! Set the attributes in this process.
	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;	

public: // IFeature

	IFEATURE_IMPL();

public: // IRecord

	IRECORD_IMPL();

public:

	static void test();

};


/*!
Apply a watershed analysis on the input geometry using a coverage as elevation data and generate a flow pattern for the watershed
*/
//! Apply a watershed analysis on the input geometry using a coverage as elevation data and generate a flow pattern for the watershed
class MODULE_IMAGE_PROCESSING_PROCS_DECL WatershedFlowProcess : public ProcessImpl<WatershedFlowProcess>, public IFeature
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	WatershedFlowProcess();

protected:
	//! Destructor
	virtual ~WatershedFlowProcess();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IFeature*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IFeature*>(this);
	}

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

	//! Get the attributes in this process.
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	//! Set the attributes in this process.
	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;	

public: // IFeature

	IFEATURE_IMPL();

public: // IRecord

	IRECORD_IMPL();

public:

	static void test();

};

#endif // guard
