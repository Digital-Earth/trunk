#ifndef FEATURE_RASTERIZER_H
#define FEATURE_RASTERIZER_H
/******************************************************************************
feature_rasterizer.h

begin		: 2007-05-30
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_analysis_procs.h"

#include "pyxis/data/coverage_base.h"
#include "pyxis/derm/index.h"
#include "pyxis/pipe/process.h"

/*!
This process converts a feature collection input into a rasterized coverage.
Each cell in the coverage will contain an encoded representation of the
features which rasterize into that cell.
*/
//! Feature rasterizer coverage process.
class MODULE_ANALYSIS_PROCS_DECL FeatureRasterizer : public ProcessImpl<FeatureRasterizer>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	FeatureRasterizer();

	//! Destructor
	~FeatureRasterizer();

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

private:

	boost::intrusive_ptr<IFeatureCollection> m_spFC;

	bool m_bMask;

};

#endif // guard
