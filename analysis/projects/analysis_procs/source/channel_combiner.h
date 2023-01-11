#ifndef CHANNEL_COMBINER_H
#define CHANNEL_COMBINER_H
/******************************************************************************
channel_combiner.h

begin		: 2007-04-26
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_analysis_procs.h"

#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"

/*!
Combines the first field in any number of coverages into a single multi-field 
process.

*/
//! Channel combiner coverage process.
class MODULE_ANALYSIS_PROCS_DECL ChannelCombiner : public ProcessImpl<ChannelCombiner>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	ChannelCombiner();

	//! Destructor
	~ChannelCombiner();

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

	virtual std::string STDMETHODCALLTYPE getData() const;

	virtual void STDMETHODCALLTYPE setData(const std::string& strData);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: // ICoverage

	virtual PYXValue STDMETHODCALLTYPE getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex = 0	) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(	const PYXIcosIndex& index,
																		int nRes,
																		int nFieldIndex = 0	) const;

	virtual PYXCost STDMETHODCALLTYPE getFieldTileCost(	const PYXIcosIndex& index,
														int nRes,
														int nFieldIndex = 0	) const;
public:

	static void test();

private:

	virtual boost::intrusive_ptr<ICoverage> getInput(int n) const;

	virtual void createGeometry() const;

};

#endif // guard
