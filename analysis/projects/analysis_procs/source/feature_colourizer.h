#ifndef FEATURE_COLOURIZER_H
#define FEATURE_COLOURIZER_H
/******************************************************************************
feature_colourizer.h

begin		: 2007-06-28
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_analysis_procs.h"

#include "pyxis/data/coverage_base.h"
#include "pyxis/derm/index.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/sxs.h"

/*!
*/
//! Feature Colourizer coverage process.
class MODULE_ANALYSIS_PROCS_DECL FeatureColourizer : public ProcessImpl<FeatureColourizer>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	FeatureColourizer();

	//! Destructor
	~FeatureColourizer();

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
	
	/*! Colour the feature using its style.
	Note: This is inline because it can be called thousands of times during 
	visualization and navigation.  This method is only called from 2 locations.
	*/
	inline void colourize(const std::string& strFeatureID, PYXValue& v) const;	

private:

	boost::intrusive_ptr<ICoverage> m_spCov;

	boost::intrusive_ptr<IFeatureCollection> m_spFC;

	int m_nColourIndex;
};

#endif // guard
