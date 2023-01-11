#ifndef BLENDER_H
#define BLENDER_H
/******************************************************************************
blender.h

begin		: 2007-04-26
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_analysis_procs.h"

#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"

// standard includes
#include <cassert>
#include <vector>

/*!
Blender class. This class averages all of the input values to blend together data.

Processes being blended must have identical coverage definitions.
*/
//! Blender coverage process.
class MODULE_ANALYSIS_PROCS_DECL Blender : public ProcessImpl<Blender>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	Blender();

	//! Destructor
	~Blender();

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

	virtual std::string STDMETHODCALLTYPE getIdentity() const;

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

	PYXValue::eType findCompatibleBlendOutput(PYXValue::eType typeA,PYXValue::eType typeB) const;

	PYXPointer<PYXTableDefinition> changeDefinitionOutputType(PYXPointer<PYXTableDefinition> definition,int nIndex,PYXValue::eType outType) const;

private:

	
private:


	enum BlendMode
	{
		knConstant,
		knResolutionDependent,
		knUseHighestResolution,
	};

	BlendMode m_mode;

public:	
	class ValueBlender : public PYXObject
	{
	public:
		virtual PYXPointer<ValueBlender> clone() = 0;

		virtual void reset() = 0;

		virtual void blend(int visiblieResolution,int covResolution,const PYXValue & covValue) = 0;

		virtual const PYXValue & getValue() = 0;
	};

protected:
	class State : public PYXObject
	{
	protected:
		std::vector<boost::intrusive_ptr<ICoverage> > m_vecCov;
		std::vector<int> m_vecCovResolutions;
		PYXPointer<ValueBlender> m_valueBlender;
		PYXPointer<PYXTableDefinition> m_definition;

	public:
		State() {};

		static PYXPointer<State> create()
		{
			return PYXNEW(State);
		}

		int getInputCount() const { return static_cast<int>(m_vecCov.size()); }

		boost::intrusive_ptr<ICoverage> getInput(int n) const
		{
			assert(n < (int)m_vecCov.size());
			return m_vecCov[n];
		}

		int getInputResolution(int n) const
		{
			assert(n < (int)m_vecCovResolutions.size());
			return m_vecCovResolutions[n];
		}

		void addInput(const boost::intrusive_ptr<ICoverage> & coverage,int resolution)
		{
			m_vecCov.push_back(coverage);
			m_vecCovResolutions.push_back(resolution);
		}

		void setBlender(const PYXPointer<ValueBlender> & blender)
		{
			m_valueBlender = blender;
		}

		const PYXPointer<ValueBlender> & getBlender() const
		{
			return m_valueBlender;
		}

		void setCoverageDefinition(const PYXPointer<PYXTableDefinition> & definition)
		{
			m_definition = definition;
		}

		const PYXPointer<PYXTableDefinition> & getCoverageDefinition() const
		{ 
			return m_definition;
		}
	};

	PYXPointer<State> m_state;
};

#endif // guard
