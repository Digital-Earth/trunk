#ifndef GREYSCALE_TO_RGB_PROCESS_H
#define GREYSCALE_TO_RGB_PROCESS_H
/******************************************************************************
greyscale_to_rgb_process.h

begin		: 2007-10-11
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_image_processing_procs.h"

#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"

// standard includes
#include <cassert>
#include <vector>

/*!
This class acts as a process to convert greyscale values to rgb. In order to work properly
every field on the input must be a Greyscale type.
*/
//! Greyscale to RGB coverage process.
class MODULE_IMAGE_PROCESSING_PROCS_DECL GreyscaleToRGBProcess : public ProcessImpl<GreyscaleToRGBProcess>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	GreyscaleToRGBProcess();

protected:
	//! Destructor
	virtual ~GreyscaleToRGBProcess();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
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

	//! Convert a greyscale value to an RGB value.
	PYXValue convert(const PYXValue& valIn) const;

private:

	//! The input coverage.
	boost::intrusive_ptr<ICoverage> m_spCov;
};

#endif // guard
