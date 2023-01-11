/******************************************************************************
calculator_process.cpp

begin		: 2008-02-20
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "calculator_process.h"
#include "calculator_functions.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/derm/point_location.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

// boost includes
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

// standard includes
#include <cassert>

// {FDA4208F-0042-4b4b-86A8-B4DBEFA43733}
PYXCOM_DEFINE_CLSID(Calculator, 
					0xfda4208f, 0x42, 0x4b4b, 0x86, 0xa8, 0xb4, 0xdb, 0xef, 0xa4, 0x37, 0x33);
PYXCOM_CLASS_INTERFACES(Calculator, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(Calculator, "Calculator", "A coverage that returns a result based on a mathematical formula performed on its input coverages.",  "Analysis/Coverages",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
					IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 26, "Input Coverage", "An input coverage to be used in the calculation and referenced as a through z.")
					IPROCESS_SPEC_END

					namespace
{

	//! Tester class
	Tester<Calculator> gTester;

}

Calculator::Calculator() : m_singleExpression(true)
{
}

Calculator::~Calculator()
{
}

void Calculator::test()
{
	// TODO: test something
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

/*! 
Serialize the process to a map of strings.

\return the attribute schema as a string.
*/
std::string Calculator::getAttributeSchema() const
{
	return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
		"elementFormDefault=\"qualified\" "
		"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		"<xs:simpleType name=\"ExpressionMode\">"
		"<xs:restriction base=\"xs:string\">"
		"<xs:enumeration value=\"SameExpression\" />"
		"<xs:enumeration value=\"DifferentExpressions\" />"
		"</xs:restriction>"
		"</xs:simpleType>"
		"<xs:simpleType name=\"OutputMode\">"
		"<xs:restriction base=\"xs:string\">"
		"<xs:enumeration value=\"\" />"
		"<xs:enumeration value=\"boolean\" />"
		"<xs:enumeration value=\"byte\" />"
		"<xs:enumeration value=\"int\" />"
		"<xs:enumeration value=\"double\" />"
		"</xs:restriction>"
		"</xs:simpleType>"
		"<xs:element name=\"CalculatorProcess\">"
		"<xs:complexType>"
		"<xs:sequence>"
		"<xs:element name=\"expression\" type=\"xs:string\" nillable=\"false\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Expression</friendlyName>"
		"<description></description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"
		"<xs:element name=\"mode\" type=\"ExpressionMode\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Mode</friendlyName>"
		"<description>If to run different expressions on different channels</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"
		"<xs:element name=\"outputType\" type=\"OutputMode\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>OutputType</friendlyName>"
		"<description>set the output type (default: same as input)</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"
		"</xs:sequence>"
		"</xs:complexType>"
		"</xs:element>"
		"</xs:schema>";
}

/*!
Get the attributes for this process.
*/
std::map<std::string, std::string> Calculator::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;	
	if (m_strExpresions.size() == 1)
	{
		mapAttr["expression"] = m_strExpresions[0];
	}
	else 
	{
		for(unsigned i=0;i<m_strExpresions.size();i++)
		{
			mapAttr["expression_"+StringUtils::toString(i)] = m_strExpresions[i];
		}
	}

	mapAttr["mode"] = m_singleExpression?"SameExpression":"DifferentExpressions";
	mapAttr["outputType"] = m_outputType;
	return mapAttr;
}

/*!
Set the attributes for this process.
*/
void STDMETHODCALLTYPE Calculator::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{
	//helper map to reorder the expressions in the right order
	std::map<int,std::string> expressions;

	m_initState = knNeedsInit;

	//get all expresions and attributes.
	for(std::map<std::string, std::string>::const_iterator it = mapAttr.begin();it!=mapAttr.end();++it)
	{
		if (it->first == "expression")
		{
			expressions[0] = it->second;
		}
		else if (it->first.substr(0,11) == "expression_")
		{
			expressions[StringUtils::fromString<int>(it->first.substr(11))] = it->second;
		}
		else if (it->first == "mode")
		{
			m_singleExpression = (it->second) != "DifferentExpressions";
		}
		else if (it->first == "outputType")
		{
			m_outputType = it->second;
		}
	}

	//add all expresions in the right order...
	if (expressions.size()>0)
	{
		m_strExpresions.clear();
		for(std::map<int,std::string>::iterator it = expressions.begin();it != expressions.end();++it)
		{
			m_strExpresions.push_back(it->second);
		}
	}
}

IProcess::eInitStatus Calculator::initImpl()
{
	m_strID = "Calculator: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	if (m_strExpresions.size() == 0 || m_strExpresions[0].size() == 0)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("No expression was set");
		return knFailedToInit;
	}

	PYXPointer<CalcState> state = CalcState::create();

	// Set up inputs
	int nNumberInputs = getParameter(0)->getValueCount();
	for (int nInput = 0; nInput != nNumberInputs; ++nInput)
	{

		assert(getParameter(0)->getValue(nInput));
		boost::intrusive_ptr<ICoverage> spCov = getParameter(0)->getValue(nInput)->getOutput()->QueryInterface<ICoverage>();			
		assert(spCov);
		state->m_vecCov.push_back(spCov);
	}

	if (m_singleExpression)
	{
		// Set up coverage definition by copying the definition of the first input.
		state->m_covDef = PYXTableDefinition::create();
		const PYXFieldDefinition & field = state->getInput(0)->getCoverageDefinition()->getFieldDefinition(0);
		if (m_outputType=="")
		{
			state->m_covDef->addFieldDefinition(field.getName(),PYXFieldDefinition::knContextNone,field.getType(),field.getCount());
		}
		else if (m_outputType=="boolean")
		{
			state->m_covDef->addFieldDefinition("Value",PYXFieldDefinition::knContextNone,PYXValue::knBool,field.getCount());
		}
		else if (m_outputType=="byte")
		{
			state->m_covDef->addFieldDefinition("Value",PYXFieldDefinition::knContextNone,PYXValue::knUInt8,field.getCount());
		}
		else if (m_outputType=="int")
		{
			state->m_covDef->addFieldDefinition("Value",PYXFieldDefinition::knContextNone,PYXValue::knInt32,field.getCount());
		}
		else if (m_outputType=="double")
		{
			state->m_covDef->addFieldDefinition("Value",PYXFieldDefinition::knContextNone,PYXValue::knDouble,field.getCount());
		} else {
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Insupported output type:" + m_outputType);
			return knFailedToInit;
		}


		// check that all inputs have numerical definitions and the same number of channels.
		for (int nOffset = 0; nOffset < static_cast<int>(state->m_vecCov.size()); ++nOffset)
		{
			// get the cov defn. for n
			PYXPointer<PYXTableDefinition> currentCov = 
				state->getInput(nOffset)->getCoverageDefinition();

			assert(0 < currentCov->getFieldCount());

			// make sure that the field counts match
			if (state->m_covDef->getFieldCount() != currentCov->getFieldCount())
			{
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError("Field counts between coverages do not match.");
				return knFailedToInit;
			}

			// Ensure that all field definitions are numerical.
			for (int nIndex = 0; nIndex < state->m_covDef->getFieldCount(); ++nIndex)
			{
				if (!currentCov->getFieldDefinition(nIndex).isNumeric())
				{
					m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
					m_spInitError->setError("Non-numeric input to the calculator detected.");
					return knFailedToInit;
				}
				if (currentCov->getFieldDefinition(nIndex).getCount() !=
					state->m_covDef->getFieldDefinition(nIndex).getCount())
				{
					m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
					m_spInitError->setError("Input coverages to the calculator have a different number of values for the same field.");
					return knFailedToInit;
				}
			}
		}
	}
	else 
	{
		// Set up coverage definition by copying the definition of the first input.
		state->m_covDef = PYXTableDefinition::create();
		const PYXFieldDefinition & field = state->getInput(0)->getCoverageDefinition()->getFieldDefinition(0);
		if (m_outputType=="")
		{
			state->m_covDef->addFieldDefinition("Value",PYXFieldDefinition::knContextNone,field.getType(),m_strExpresions.size());
		}
		else if (m_outputType=="boolean")
		{
			state->m_covDef->addFieldDefinition("Value",PYXFieldDefinition::knContextNone,PYXValue::knBool,m_strExpresions.size());
		}
		else if (m_outputType=="byte")
		{
			state->m_covDef->addFieldDefinition("Value",PYXFieldDefinition::knContextNone,PYXValue::knUInt8,m_strExpresions.size());
		}
		else if (m_outputType=="int")
		{
			state->m_covDef->addFieldDefinition("Value",PYXFieldDefinition::knContextNone,PYXValue::knInt32,m_strExpresions.size());
		}
		else if (m_outputType=="double")
		{
			state->m_covDef->addFieldDefinition("Value",PYXFieldDefinition::knContextNone,PYXValue::knDouble,m_strExpresions.size());
		} else {
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Insupported output type:" + m_outputType);
			return knFailedToInit;
		}
	}

	for (int count = 0; count < N_THREADS; ++count)
	{
		PYXPointer<CalcMachine> machine = CalcMachine::create(
			m_singleExpression?CalcMachine::knSingleExpression:CalcMachine::knMultiExpressions,
			state->m_covDef->getFieldDefinition(0).getType());

		if (!machine->setInputs(state->m_vecCov))
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Failed to set up inputs.");
			return knFailedToInit;
		}
		if (m_singleExpression)
		{
			if (!machine->setSingleExpression(m_strExpresions[0]))
			{
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError("Bad Expression.");
				return knFailedToInit;
			}
		}
		else
		{
			if (!machine->setMultiExpressions(m_strExpresions))
			{
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError("Bad Expression: " + machine->getBadExpression() );
				return knFailedToInit;
			}
		}

		state->m_calcMachines.push_back(machine);
	}

	m_spCovDefn = state->m_covDef;
	m_state = state;

	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue Calculator::getCoverageValue(	const PYXIcosIndex& index,
									  int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	// Return a null value if we are outside our geometry.
	if (!getGeometry()->intersects(PYXCell(index)))
	{
		return PYXValue();
	}

	//! storage for each coverage value
	std::vector<PYXValue> vecCoverageValues;
	std::vector<double> vecOtherValues;

	int numInputs = m_state->getInputCount();

	//Add lat long
	if(m_state->m_calcMachines[0]->needLocation())
	{
		auto location = PointLocation::fromPYXIndex(index);
		vecOtherValues.push_back( location.asWGS84().x());
		vecOtherValues.push_back( location.asWGS84().y());
	}

	for (int nInput = 0; nInput != numInputs; ++nInput)
	{
		vecCoverageValues.push_back(m_state->getInput(nInput)->getCoverageValue(index, nFieldIndex));
	}

	return m_state->m_calcMachines[0]->calc(vecCoverageValues,vecOtherValues);
}

void CalculatePartialTile (int firstIndex, int lastIndex, int numInputs,
						   const PYXFieldDefinition& fieldDefn,
						   PYXPointer<const PYXGeometry> spGeometry,
						   std::vector<PYXPointer<PYXValueTile>>& vecInputTile,
						   PYXValueTile* spValueTile,
						   PYXPointer<CalcMachine> machine)
{
	// TODO: we should be able to be more efficient than this -- look for
	// or add support for setting the index in the iterator to part way along.

	//Travel to the correct position
	PYXPointer<PYXIterator> it = spValueTile->getIterator();
	for (int nCell = 0; nCell < firstIndex; ++nCell)
	{
		it->next();
	}

	//Create vector of pyxvalues to pass to machine to calculate
	std::vector<PYXValue> vecCoverageValues;
	std::vector<double> vecOtherValues;
	vecOtherValues.push_back(0);
	vecOtherValues.push_back(0);

	for (int nInput = 0; nInput != numInputs; ++nInput)
	{
		if (vecInputTile[nInput])
		{
			vecCoverageValues.push_back(vecInputTile[nInput]->getTypeCompatibleValue(0));
		}
		else
		{
			return;
		}
	}

	int nArraySize = spValueTile->getTypeCompatibleValue(0).getArraySize();	

	for (int nCell = firstIndex; nCell <= lastIndex; ++nCell)
	{
		auto index = it->getIndex();
		bool bIsPartOfThisDataSet = spGeometry->intersects(PYXCell(index));

		it->next();
		if (bIsPartOfThisDataSet)
		{
			bool allValuesAreValid = true;
			if(machine->needLocation())
			{
				auto location = PointLocation::fromPYXIndex(index);
				vecOtherValues[0] = location.asWGS84().x();
				vecOtherValues[1] = location.asWGS84().y();
			}
			for (int nInput = 0 ; nInput != numInputs ; ++nInput)
			{
				//try to set the value. will return zero if the value is null
				if(!vecInputTile[nInput]->getValue(nCell, 0, &vecCoverageValues[nInput]))
				{
					allValuesAreValid = false;
					break;
				}
			}
			if (allValuesAreValid)
			{
				spValueTile->setValue(nCell, 0, machine->calc(vecCoverageValues,vecOtherValues));
			}
		}
	}
}

PYXPointer<PYXValueTile> Calculator::getFieldTile(	const PYXIcosIndex& index,
												  int nRes,
												  int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	// If we don't have data for any of this tile return a null tile.
	PYXPointer<const PYXGeometry> spGeometry = getGeometry();
	if (!spGeometry->intersects(PYXTile(index, nRes)))
	{
		return PYXPointer<PYXValueTile>();
	}

	// our goal tile, which we are constructing	
	const PYXFieldDefinition& fieldDefn = getCoverageDefinition()->getFieldDefinition(nFieldIndex);
	PYXPointer<PYXValueTile> spValueTile = PYXValueTile::create(index, nRes, getCoverageDefinition());

	// get data from all of our sources.
	std::vector<PYXPointer<PYXValueTile>> vecInputTile;
	for (int nInput = 0; nInput != m_state->getInputCount(); ++nInput)
	{
		vecInputTile.push_back(m_state->getInput(nInput)->getFieldTile(index, nRes, nFieldIndex));
	}

	PYXPointer<PYXGeometry> tileGeometry = spGeometry->intersection(PYXTile(index,nRes));

	int nCellCount = spValueTile->getNumberOfCells();
	int cellsPerThread = (nCellCount + N_THREADS - 1) / N_THREADS;
	int firstIndex = 0;
	boost::thread_group threads;
	for (int count = 0; count < N_THREADS; ++count)
	{
		int lastIndex = firstIndex + cellsPerThread - 1;
		if (lastIndex >= nCellCount)
		{
			lastIndex = nCellCount - 1;
		}
		threads.create_thread(
			boost::bind (&CalculatePartialTile,
			firstIndex, lastIndex,
			m_state->getInputCount(), fieldDefn,
			tileGeometry, vecInputTile, 
			spValueTile.get(), m_state->m_calcMachines[count]));
		firstIndex += cellsPerThread;
	}

	// wait for all the threads to finish.
	threads.join_all();

	return spValueTile;
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

void Calculator::createGeometry() const
{
	m_spGeom = m_state->getInput(0)->getGeometry()->clone();
	for (int nInput = 1; nInput != getParameter(0)->getValueCount(); ++nInput)
	{
		PYXPointer<PYXGeometry> spGeometry = m_state->getInput(nInput)->getGeometry();
		if (0 != spGeometry)
		{
			// Intersection of all input geometries.
			m_spGeom = m_spGeom->intersection(*spGeometry);
		}
	}
}



////////////////////////////////////////////////////////////////////////////////
// CalcMachine
////////////////////////////////////////////////////////////////////////////////


CalcMachine::CalcMachine(CalcMachineMode mode,PYXValue::eType outputType) : m_mode(mode), m_type(outputType)
{
	m_needLocation = false;
}

CalcMachine::~CalcMachine() 
{
	clearExpressions();
}

void CalcMachine::clearExpressions() {
	for(int i=0;i<(int)m_expressions.size();i++)
	{
		delete m_expressions[i];
	}
	m_expressions.clear();
	m_badExpression.clear();
}

bool CalcMachine::setInputs(const std::vector<boost::intrusive_ptr<ICoverage>> & inputs)
{
	m_spFlist.reset(new ExprEval::FunctionList());
	m_spFlist->AddDefaultFunctions();

	m_spFlist->Add(new FunctionFactory<Dist_FunctionNode>("dist"));

	m_spVlist.reset(new ExprEval::ValueList());
	m_spVlist->AddDefaultValues();

	m_vecUserDefinedValues.clear();
	m_vecUserDefinedValues.push_back(1.0);
	m_vecUserDefinedValues.push_back(1.0);

	m_spVlist->AddAddress("WGS84Lon", &(m_vecUserDefinedValues[0]));
	m_spVlist->AddAddress("WGS84Lat", &(m_vecUserDefinedValues[1]));

	m_vecVarValues.clear();

	if (m_mode == knSingleExpression)
	{	
		for (int nInput = 0; nInput < (int)inputs.size(); ++nInput)
		{
			// create the storage space for the variable's value
			m_vecVarValues.push_back(1.0);
		}
		for (int nInput = 0; nInput < (int)inputs.size(); ++nInput)
		{
			std::string varName = "";
			varName += char(nInput)+'a';

			// add the new variable to the value list
			m_spVlist->AddAddress(varName, &(m_vecVarValues[nInput]));
		}

		if (inputs.size()>0)
		{
			PYXPointer<const PYXTableDefinition> definition = inputs[0]->getCoverageDefinition();
			int count = definition->getFieldDefinition(0).getCount();
			m_vecInputsValueSize.push_back(count);

			m_valueSpec = PYXValue::create(m_type, 0, count, 0);
		}
		else
		{
			m_vecInputsValueSize.push_back(1); //make it a const expression
			m_valueSpec = PYXValue::create(m_type, 0, 1, 0);
		}
	} 
	else 
	{
		for (int nInput = 0; nInput < (int)inputs.size(); ++nInput)
		{
			// create the storage space for the variable's value
			//use only the first param.
			PYXPointer<const PYXTableDefinition> definition = inputs[nInput]->getCoverageDefinition();
			int count = definition->getFieldDefinition(0).getCount();
			for(int i = 0; i < count ; i++)
			{
				m_vecVarValues.push_back(1.0);
			}
		}

		int varIndex = 0;

		for (int nInput = 0; nInput < (int)inputs.size(); ++nInput)
		{			
			std::string varName = "";
			varName += char(nInput)+'a';

			//use only the first param.
			PYXPointer<const PYXTableDefinition> definition = inputs[nInput]->getCoverageDefinition();
			int count = definition->getFieldDefinition(0).getCount();
			for(int i = 0; i < count ; i++)
			{
				m_spVlist->AddAddress(varName + StringUtils::toString(i), &(m_vecVarValues[varIndex]));
				varIndex++;
			}

			m_vecInputsValueSize.push_back(count);
		}

		m_valueSpec = PYXValue::create(m_type, 0, 1, 0);
	}

	return true;
}

bool CalcMachine::setSingleExpression(const std::string & expression)
{
	m_needLocation = expression.find("WGS84Lat") !=std::string::npos || expression.find("WGS84Lon") != std::string::npos;
	clearExpressions();
	if (m_mode == knSingleExpression)
	{
		if (!addExpression(expression))
		{
			return false;
		}
		
		return true;
	}
	return false;
}
bool CalcMachine::needLocation()
{
	return m_needLocation;
}
bool CalcMachine::setMultiExpressions(const std::vector<std::string> & expressions)
{
	for each (std::string expression in expressions)
	{
		if(expression.find("WGS84Lat") !=std::string::npos || expression.find("WGS84Lon") != std::string::npos)
		{
			m_needLocation = true;
			break;
		}
	}
	
	clearExpressions();
	if (m_mode == knMultiExpressions)
	{
		for(int i=0;i<(int)expressions.size();i++)
		{
			if (!addExpression(expressions[i]))
			{
				m_badExpression = expressions[i];
				return false;
			}
		}
		m_valueSpec = PYXValue::create(m_type, 0, expressions.size(), 0);
		return true;
	}
	return false;
}


PYXValue CalcMachine::calc(const std::vector<PYXValue> & values,const std::vector<double> & otherValues) const 
{
	for (unsigned int input = 0 ; input < otherValues.size(); input++)
	{
		m_vecUserDefinedValues[input] = otherValues[input];
	}

	if (m_mode == knSingleExpression)
	{
		PYXValue result = m_valueSpec;
		for(int index = 0; index < m_vecInputsValueSize[0]; index ++ )
		{
			for(unsigned int input = 0 ; input<values.size();input++)
			{
				m_vecVarValues[input] = values[input].getDouble(index);
			}

			result.setDouble(index,m_expressions[0]->Evaluate());
		}
		return result;
	}
	else
	{
		int index=0;
		for(unsigned int input=0;input<values.size();input++)
		{
			for (int i = 0 ; i < m_vecInputsValueSize[input]; i++)
			{
				m_vecVarValues[index] = values[input].getDouble(i);
				index++;
			}
		}

		PYXValue result = m_valueSpec;
		for(int index = 0; index < (int)m_expressions.size(); index ++ )
		{
			result.setDouble(index,m_expressions[index]->Evaluate());
		}
		return result;
	}
}

bool CalcMachine::addExpression(const std::string & expressionStr)
{
	std::auto_ptr<ExprEval::Expression> expression(new ExprEval::Expression());

	expression->SetValueList(m_spVlist.get());
	expression->SetFunctionList(m_spFlist.get());
	try
	{
		expression->Parse(expressionStr);
	}
	catch (...)
	{
		return false;
	}

	m_expressions.push_back(expression.release());
	return true;
}
