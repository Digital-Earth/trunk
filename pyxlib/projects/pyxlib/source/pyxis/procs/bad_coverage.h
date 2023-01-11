#ifndef BAD_COVERAGE_H
#define BAD_COVERAGE_H
/******************************************************************************
bad_coverage.h

begin		: 2006-04-18
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes

// pyxis includes
#include "pyxis/data/coverage_base.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/string_utils.h"

/*!
A global data source that will never initialize.  It will throw exceptions on all
access that should not be done until initialize is called.  This is a testing tool
that can be used to make sure that processes that use a coverage can deal properly
with a coverage that can not initialize.
*/
//! A global data source that will never initialize.
class PYXLIB_DECL BadCoverage : public ProcessImpl<BadCoverage>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	BadCoverage();

	//! Test method
	static void test();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(ICoverage)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST(BadCoverage, IProcess);

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

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(const PYXIcosIndex& index,
																	int nRes,
																	int nFieldIndex = 0	) const
	{
		PYXTHROW(PYXException, "getFieldTile() called when process was not initialized.");
	}

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getCoverageTile(const PYXTile& tile) const
	{
		PYXTHROW(PYXException, "getCoverageTile() called when process was not initialized.");
	}

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const
	{
		PYXTHROW(PYXException, "getCoverageValue() called when process was not initialized.");
	}

private:

	virtual void createGeometry() const
	{
		PYXTHROW(PYXException, "createGeometry() called when process was not initialized.");
	}

};

#endif