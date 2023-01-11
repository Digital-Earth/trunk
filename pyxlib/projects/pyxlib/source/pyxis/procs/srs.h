#ifndef PYXIS__PROCS__SRS_H
#define PYXIS__PROCS__SRS_H
/******************************************************************************
srs.h

begin		: 2007-04-16
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/sampling/spatial_reference_system.h"

//! Interface for spatial reference systems.
struct PYXLIB_DECL ISRS : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	virtual PYXPointer<PYXSpatialReferenceSystem> STDMETHODCALLTYPE getSRS() = 0;
};

/*!
*/
//! A process for spatial reference systems.
class PYXLIB_DECL SRSProc : public ProcessImpl<SRSProc>, public ISRS
{
	PYXCOM_DECLARE_CLASS();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(ISRS)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const ISRS*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<ISRS*>(this);
	}

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::string STDMETHODCALLTYPE getData() const;

	virtual void STDMETHODCALLTYPE setData(const std::string& strData);

public: // ISRS

	virtual PYXPointer<PYXSpatialReferenceSystem> STDMETHODCALLTYPE getSRS()
	{
		return m_spSRS;
	}

public:

	SRSProc() : m_spSRS(PYXSpatialReferenceSystem::create()) {}

private:

	PYXPointer<PYXSpatialReferenceSystem> m_spSRS;
};

#endif // guard
