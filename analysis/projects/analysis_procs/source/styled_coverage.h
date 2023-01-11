#ifndef STYLED_COVERAGE_H
#define STYLED_COVERAGE_H
/******************************************************************************
styled_coverage.h

begin		: 2011-09-08
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_analysis_procs.h"

#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/color_palette.h"

// standard includes
#include <cassert>
#include <vector>

/*!
*/
//! Blender coverage process.
class MODULE_ANALYSIS_PROCS_DECL StyledCoverage : public ProcessImpl<StyledCoverage>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	StyledCoverage();

	//! Destructor
	~StyledCoverage();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(ICoverage)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();


public: // IFeaute
	virtual std::string STDMETHODCALLTYPE getStyle() const
	{
		return m_strStyle;
	}
	virtual std::string STDMETHODCALLTYPE getStyle(const std::string& strStyleToGet) const
	{
		if (m_strStyle.size() == 0)
		{
			return "";
		}
		PYXPointer<CSharpXMLDoc> styleDoc = CSharpXMLDoc::create(m_strStyle);
		return styleDoc->getNodeText("/style/" + strStyleToGet);
	}


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

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(	const PYXIcosIndex& index,
																		int nRes,
																		int nFieldIndex = 0	) const;

	virtual PYXCost STDMETHODCALLTYPE getFieldTileCost(	const PYXIcosIndex& index,
														int nRes,
														int nFieldIndex = 0	) const;

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getCoverageTile(const PYXTile& tile) const;

public:

	static void test();

private:

	virtual void createGeometry() const;

private:

	bool m_showAsElevation;
	std::string m_palette;
	std::string m_strStyle;

	class State : public PYXObject
	{
	public:		
		PYXPointer<PYXTableDefinition> m_definition;

		bool m_showAsElevation;
		std::string m_palette;

		boost::intrusive_ptr<ICoverage> m_spCov;

		State(const boost::intrusive_ptr<ICoverage> & spCov,const std::string & palette)
			:	m_palette(palette),
				m_spCov(spCov)
		{
		}

		static PYXPointer<State> create(const boost::intrusive_ptr<ICoverage> & spCov, const std::string & palette)
		{
			return PYXNEW(State,spCov,palette);
		}
	};

	PYXPointer<State> m_state;
};

#endif // guard
