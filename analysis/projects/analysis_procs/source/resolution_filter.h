#ifndef RESOLUTION_FILTER_H
#define RESOLUTION_FILTER_H
/******************************************************************************
resolution_filter.h

begin		: 2007-04-27
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_analysis_procs.h"

#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"

/*!
*/
//! Resolution filter coverage process.
class MODULE_ANALYSIS_PROCS_DECL ResolutionFilter : public ProcessImpl<ResolutionFilter>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	ResolutionFilter();

	//! Destructor
	~ResolutionFilter();

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

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // ICoverage

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(		const PYXIcosIndex& index,
																			int nRes,
																			int nFieldIndex = 0	) const;

	virtual PYXCost STDMETHODCALLTYPE getFieldTileCost(	const PYXIcosIndex& index,
														int nRes,
														int nFieldIndex = 0	) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getCoverageTile(const PYXTile& tile) const;

	virtual PYXCost STDMETHODCALLTYPE getTileCost(const PYXTile& tile) const;

public:

	static void test();

private:

	virtual boost::intrusive_ptr<ICoverage> getInput(int n) const;

	virtual void createGeometry() const;

private:

	int m_nMinRelRes;
	int m_nMaxRelRes;
	int m_nMinAbsRes;
	int m_nMaxAbsRes;

	int m_nMinRes;
	int m_nMaxRes;
	boost::intrusive_ptr<ICoverage> m_spCov;
};

#endif // guard
