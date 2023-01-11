#ifndef HILLSHADE_PROCESS_H
#define HILLSHADE_PROCESS_H
/******************************************************************************
hillshade_process.h

begin		: 2010-03-09
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#pragma once

#include "module_image_processing_procs.h"

#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"

// standard includes
#include <cassert>
#include <vector>

/*!
HillShader process.

Input processes must have numerical coverage definitions.
*/
//! Hill Shader coverage process.
class MODULE_IMAGE_PROCESSING_PROCS_DECL HillShader : public ProcessImpl<HillShader>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

private:
	//! Controls the number of threads that will be used to run the calculation.
	static const int N_THREADS = 8;

public:

	//! Constructor
	HillShader();

protected:
	//! Destructor
	virtual ~HillShader();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(ICoverage)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const ICoverage*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<ICoverage*>(this);
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // ICoverage

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(		const PYXIcosIndex& index,
																			int nRes,
																			int nFieldIndex = 0	) const;

public:

	static void test();

private:
	virtual void createGeometry() const;

	// the colour that we will use as a bse to generate the hill shade.
	PYXValue m_colourValue;

	// the input coverage
	boost::intrusive_ptr<ICoverage> m_spCov;
};

#endif // guard
