#ifndef FIRST_NON_NULL_H
#define FIRST_NON_NULL_H
/******************************************************************************
first_non_null.h

begin		: 2007-05-02
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
First Non-null class. This class acts as a process to select data from the first non-null
datasource.

Input processes must have identical coverage definitions.
*/
//! First Non Null coverage process.
class MODULE_ANALYSIS_PROCS_DECL FirstNonNull : public ProcessImpl<FirstNonNull>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Constructor
	FirstNonNull();

	//! Destructor
	~FirstNonNull();

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

	virtual PYXCost STDMETHODCALLTYPE getFieldTileCost(	const PYXIcosIndex& index,
														int nRes,
														int nFieldIndex = 0	) const;
public:

	static void test();

private:

	virtual void createGeometry() const;

private:

	class State : public PYXObject
	{
	private:

		std::vector<boost::intrusive_ptr<ICoverage> > m_vecCov;
		PYXPointer<PYXTableDefinition> m_definition;

	public:
		State() {}

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

		void addInput(const boost::intrusive_ptr<ICoverage> & input)
		{
			m_vecCov.push_back(input);
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
