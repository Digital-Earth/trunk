#ifndef COLOURIZER_H
#define COLOURIZER_H
/******************************************************************************
colourizer.h

begin		: 2007-05-03
copyright	: (C) 2007 by the PYXIS innovation inc.
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
class MODULE_ANALYSIS_PROCS_DECL Colourizer : public ProcessImpl<Colourizer>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	Colourizer();

	//! Destructor
	~Colourizer();

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

	virtual PYXPointer<PYXValueTile> STDMETHODCALLTYPE getFieldTile(	const PYXIcosIndex& index,
																		int nRes,
																		int nFieldIndex = 0	) const;

	virtual PYXCost STDMETHODCALLTYPE getFieldTileCost(	const PYXIcosIndex& index,
														int nRes,
														int nFieldIndex = 0	) const;
public:

	static void test();

private:

	virtual void createGeometry() const;

private:

	double m_fMin;
	double m_fMax;
	std::string m_palette;

	class State : public PYXObject
	{
	public:		
		PYXPointer<PYXTableDefinition> m_definition;

		double m_fMin;
		double m_fMax;
		double m_fRange;
		PYXPointer<PYXColorPalette> m_palette;

		boost::intrusive_ptr<ICoverage> m_spCov;

		State(const boost::intrusive_ptr<ICoverage> & spCov, double minRange,double maxRange,const std::string & palette)
			:	m_fMin(minRange),
				m_fMax(maxRange),
				m_spCov(spCov)
		{
			m_fRange = m_fMax - m_fMin;

			if (palette == "")
			{
				//simple black to white using range...
				m_palette = PYXColorPalette::create("2 " + StringUtils::toString(minRange) + " 0 0 0 255 " + StringUtils::toString(maxRange) + " 255 255 255 255");
			} else {
				m_palette = PYXColorPalette::create(palette);
			}
		}

		static PYXPointer<State> create(const boost::intrusive_ptr<ICoverage> & spCov, double minRange,double maxRange,const std::string & palette)
		{
			return PYXNEW(State,spCov,minRange,maxRange,palette);
		}
	};

	PYXPointer<State> m_state;
};

#endif // guard
