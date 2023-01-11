#ifndef CALCULATOR_PROCESS_H
#define CALCULATOR_PROCESS_H
/******************************************************************************
calculator_process.h

begin		: 2008-02-20
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_image_processing_procs.h"

#include "pyxis/data/coverage_base.h"
#include "pyxis/pipe/process.h"

#include "../../third_party/expreval34/expreval.h"

// standard includes
#include <cassert>
#include <vector>

//! Helper class for calculator threading.
class CalcMachine : public PYXObject
{
public:
	enum CalcMachineMode {
		knSingleExpression,
		knMultiExpressions
	};

	static PYXPointer<CalcMachine> create(CalcMachineMode mode, PYXValue::eType outputType)
	{
		return PYXNEW(CalcMachine,mode,outputType);
	}

	CalcMachine(CalcMachineMode mode, PYXValue::eType outputType);

	~CalcMachine();	

private:
	void clearExpressions();

public:

	bool setInputs(const std::vector<boost::intrusive_ptr<ICoverage>> & inputs);
	bool setSingleExpression(const std::string & expression);
	bool setMultiExpressions(const std::vector<std::string> & expressions);
	bool needLocation();
	const std::string & getBadExpression() { return m_badExpression; } ;

//	bool setOutputTypee(PYXValue::eType type);
	PYXValue calc(const std::vector<PYXValue> & values, const std::vector<double> & userValues) const;

private:
	bool addExpression(const std::string & expression);	

private:
	CalcMachineMode m_mode;
	PYXValue::eType m_type;
	PYXValue		m_valueSpec;
	bool			m_needLocation;

	//! the storage for the variables
	mutable std::vector<double> m_vecVarValues;

	mutable std::vector<double> m_vecUserDefinedValues;
	
	//! the size of each input
	std::vector<int> m_vecInputsValueSize;

	//! the expression(or several) that we will evaluate for each cell.
	mutable std::vector<ExprEval::Expression*> m_expressions;

	//! the list of values that are used by the expression
	std::auto_ptr<ExprEval::ValueList> m_spVlist;

	//! the list of functions available to the expression
    std::auto_ptr<ExprEval::FunctionList> m_spFlist;

	//! the expression that failed to complile.
	std::string m_badExpression;
};

/*!
Calculator process.

Input processes must have numerical coverage definitions.
*/
//! Calculator coverage process.
class MODULE_IMAGE_PROCESSING_PROCS_DECL Calculator : public ProcessImpl<Calculator>, public CoverageBase
{
	PYXCOM_DECLARE_CLASS();

private:
	//! Controls the number of threads that will be used to run the calculation.
	static const int N_THREADS = 8;

public:

	//! Constructor
	Calculator();

protected:
	//! Destructor
	virtual ~Calculator();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(ICoverage)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST( Calculator, IProcess);

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

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

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


private:
	//! the expresion to be evaluated for each cell as a string representation.
	std::vector<std::string> m_strExpresions;

	//! if not empty- will force the output type.
	std::string m_outputType;

	//! if to run the same expression on all inputs
	bool m_singleExpression;

	class CalcState : public PYXObject
	{
	public:
		static PYXPointer<CalcState> create()
		{
			return PYXNEW(CalcState);
		}

		CalcState() {}

		//! the input coverages
		std::vector<PYXPointer<CalcMachine>> m_calcMachines;

		//! calc machine for each thraed
		std::vector<boost::intrusive_ptr<ICoverage>> m_vecCov;

		PYXPointer<PYXTableDefinition> m_covDef;

		int getInputCount() const { return static_cast<int>(m_vecCov.size()); }

		boost::intrusive_ptr<ICoverage> getInput(int n) const
		{
			assert(n < (int)m_vecCov.size());
			return m_vecCov[n];
		}
	};

	PYXPointer<CalcState> m_state;
};

#endif // guard
