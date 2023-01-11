/******************************************************************************
blender.cpp

begin		: 2007-04-26
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_ANALYSIS_PROCS_SOURCE
#include "blender.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/pipe/process_identity.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"
#include "pyxis/pipe/process_local_storage.h"
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/geometry/geometry_serializer.h"

// standard includes
#include <cassert>

// required for testing
#include "pyxis/procs/const_coverage.h"

// {00B7D55E-433A-4767-9C77-B5E276762A97}
PYXCOM_DEFINE_CLSID(Blender, 
0xb7d55e, 0x433a, 0x4767, 0x9c, 0x77, 0xb5, 0xe2, 0x76, 0x76, 0x2a, 0x97);
PYXCOM_CLASS_INTERFACES(Blender, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(Blender, "Blender", "A coverage that blends multiple input coverages into a single output coverage.", "Analysis/Coverages",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 0, -1, "Coverage(s) to Blend", "A coverage to merge with all other input coverages.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<Blender> gTester;

}

Blender::Blender() : m_mode(knConstant)
{
}

Blender::~Blender()
{
}

void Blender::test()
{
	boost::intrusive_ptr<IProcess> spInput1;
	boost::intrusive_ptr<ConstCoverage> spCoverage1(new ConstCoverage);
	assert(spCoverage1);
	spCoverage1->setFieldCount(1);
	spCoverage1->setReturnValue(PYXValue(18), PYXFieldDefinition::knContextRGB, 0);
	spCoverage1->QueryInterface(IProcess::iid, (void**)&spInput1);

	boost::intrusive_ptr<IProcess> spInput2;
	boost::intrusive_ptr<ConstCoverage> spCoverage2(new ConstCoverage);
	assert(spCoverage2);
	spCoverage2->setFieldCount(15);
	spCoverage2->setReturnValue(PYXValue(42), PYXFieldDefinition::knContextRGB, 0);
	spCoverage2->QueryInterface(IProcess::iid, (void**)&spInput2);

	// Get the identity for each, and make sure they are different.
	std::string strIdentity1 = spInput1->getIdentity();
	std::string strIdentity2 = spInput2->getIdentity();
	TEST_ASSERT(strIdentity1 != strIdentity2);

	// Create a blender, set the test coverages as input values, and get the identity.
	{
		boost::intrusive_ptr<IProcess> spBlender1;
		PYXCOMCreateInstance(Blender::clsid, 0, IProcess::iid, (void**)&spBlender1);
		assert(spBlender1);
		{
			PYXPointer<Parameter> spParameter = spBlender1->getParameter(0);
			assert(spParameter);
			spParameter->addValue(spInput1);
			spParameter->addValue(spInput2);
		}
		strIdentity1 = spBlender1->getIdentity();
	}

	// Create another blender, with the input values reversed, and get the identity.
	{
		boost::intrusive_ptr<IProcess> spBlender2;
		PYXCOMCreateInstance(Blender::clsid, 0, IProcess::iid, (void**)&spBlender2);
		assert(spBlender2);
		{
			PYXPointer<Parameter> spParameter = spBlender2->getParameter(0);
			assert(spParameter);
			spParameter->addValue(spInput2);
			spParameter->addValue(spInput1);
		}
		strIdentity2 = spBlender2->getIdentity();
	}

	// Make sure the identities match.
	TEST_ASSERT(strIdentity1 == strIdentity2);

	// TODO Add more tests
}

////////////////////////////////////////////////////////////////////////////////
// ConstantBlenderInt - ValueBlender
////////////////////////////////////////////////////////////////////////////////

class ConstantBlenderInt : public Blender::ValueBlender 
{
protected:
	PYXValue m_value;
	int m_sampleCount;
	int m_nArraySize;
	bool m_done;

public:
	ConstantBlenderInt(const PYXFieldDefinition & definition)
	{
		m_nArraySize = definition.getCount();
		m_value = PYXValue::create(PYXValue::knInt32, 0, m_nArraySize, 0);
		m_done = false;
	}

	ConstantBlenderInt(const ConstantBlenderInt & blender)
	{
		m_nArraySize = blender.m_nArraySize;
		m_value = blender.m_value;
		m_done = false;
	}

	virtual void reset()
	{
		for (int n = 0; n != m_nArraySize; ++n)
		{
			m_value.setInt32(n, 0);
		}
		m_sampleCount = 0;
		m_done = false;
	}

	static PYXPointer<ConstantBlenderInt> create(const PYXFieldDefinition & definition) 
	{
		return PYXNEW(ConstantBlenderInt,definition);
	}

	virtual PYXPointer<ValueBlender> clone()
	{
		return PYXNEW(ConstantBlenderInt,*this);
	}

	void blend(int visiblieResolution,int covResolution,const PYXValue & covValue)
	{
		for (int n = 0; n != m_nArraySize; ++n)
		{
			m_value.setInt32(n, m_value.getInt32(n) + covValue.getInt32(n));
		}
		m_sampleCount++;
	}

	virtual const PYXValue & getValue() 
	{
		if (m_sampleCount>1 && !m_done)
		{
			for (int n = 0; n != m_nArraySize; ++n)
			{
				m_value.setInt32(n, m_value.getInt32(n) / m_sampleCount);
			}
		}
		m_done = true;

		return m_value;
	}
};

////////////////////////////////////////////////////////////////////////////////
// ConstantBlenderDouble - ValueBlender
////////////////////////////////////////////////////////////////////////////////

class ConstantBlenderDouble : public Blender::ValueBlender 
{
protected:
	PYXValue m_value;
	int m_sampleCount;
	int m_nArraySize;
	bool m_done;

public:
	ConstantBlenderDouble(const PYXFieldDefinition & definition)
	{			
		m_nArraySize = definition.getCount();
		m_value = PYXValue::create(PYXValue::knDouble, 0, m_nArraySize, 0);
		m_done = false;
	}

	ConstantBlenderDouble(const ConstantBlenderDouble & blender)
	{			
		m_nArraySize = blender.m_nArraySize;
		m_value = blender.m_value;
		m_done = false;
	}

	virtual void reset()
	{
		for (int n = 0; n != m_nArraySize; ++n)
		{
			m_value.setDouble(n, 0);
		}
		m_sampleCount = 0;
		m_done = false;
	}

	static PYXPointer<ConstantBlenderDouble> create(const PYXFieldDefinition & definition) 
	{
		return PYXNEW(ConstantBlenderDouble,definition);
	}

	virtual PYXPointer<ValueBlender> clone()
	{
		return PYXNEW(ConstantBlenderDouble,*this);
	}

	void blend(int visiblieResolution,int covResolution,const PYXValue & covValue)
	{
		for (int n = 0; n != m_nArraySize; ++n)
		{
			m_value.setDouble(n, m_value.getDouble(n) + covValue.getDouble(n));
		}
		m_sampleCount++;
	}

	virtual const PYXValue & getValue() 
	{
		if (m_sampleCount>1 && !m_done)
		{
			for (int n = 0; n != m_nArraySize; ++n)
			{
				m_value.setDouble(n, m_value.getDouble(n) / m_sampleCount);
			}
		}
		m_done = true;

		return m_value;
	}
};

////////////////////////////////////////////////////////////////////////////////
// ResolutionDependentBlender - ValueBlender
////////////////////////////////////////////////////////////////////////////////

class ResolutionDependentBlender : public Blender::ValueBlender 
{
protected:
	PYXValue m_value;
	double m_totalFactor;
	int m_nArraySize;
	bool m_done;

public:
	ResolutionDependentBlender(const PYXFieldDefinition & definition)
	{
		m_nArraySize = definition.getCount();
		m_value = PYXValue::create(PYXValue::knDouble, 0, m_nArraySize, 0);
		m_done = false;
	}

	ResolutionDependentBlender(const ResolutionDependentBlender & blender)
	{
		m_nArraySize = blender.m_nArraySize;
		m_value = blender.m_value;
		m_done = false;
	}

	virtual void reset()
	{
		for (int n = 0; n != m_nArraySize; ++n)
		{
			m_value.setDouble(n, 0);
		}
		m_totalFactor = 0;
		m_done = false;
	}

	static PYXPointer<ResolutionDependentBlender> create(const PYXFieldDefinition & definition) 
	{
		return PYXNEW(ResolutionDependentBlender,definition);
	}

	virtual PYXPointer<ValueBlender> clone()
	{
		return PYXNEW(ResolutionDependentBlender,*this);
	}

	void blend(int visiblieResolution,int covResolution,const PYXValue & covValue)
	{
		double factor = getBlendFactor(covResolution,visiblieResolution);

		for (int n = 0; n != m_nArraySize; ++n)
		{
			m_value.setDouble(n, m_value.getDouble(n) + factor*covValue.getDouble(n));
		}
		m_totalFactor+=factor;
	}

	/*!
	calcaulate the blending factor for a coverage.

	covResolution is the native resolution of the input coverage (taken from the coverage geometry).
	visilibeResolution is the output resolution.
	*/
	double getBlendFactor(int covResolution,int visibleResolution) const
	{
		//The weight function is 1/resolutionDifferent, and 1 if visiblieResolution == covResolution
		if (visibleResolution > covResolution)
		{
			return 1/(double)(visibleResolution-covResolution);
		}
		else if (visibleResolution < covResolution)
		{
			return 1/(double)(covResolution-visibleResolution);
		}
		return 1.0;
	}

	virtual const PYXValue & getValue() 
	{
		if (m_totalFactor>0 && !m_done)
		{
			for (int n = 0; n != m_nArraySize; ++n)
			{
				m_value.setDouble(n, m_value.getDouble(n) / m_totalFactor);
			}
		}
		m_done = true;

		return m_value;
	}
};

////////////////////////////////////////////////////////////////////////////////
// HighestResolutionBlender - ValueBlender
////////////////////////////////////////////////////////////////////////////////

class HighestResolutionBlender : public Blender::ValueBlender 
{
protected:
	PYXValue m_value;
	int m_sampleCount;
	int m_resolution;
	int m_nArraySize;
	bool m_done;

public:
	HighestResolutionBlender(const PYXFieldDefinition & definition)
	{
		m_nArraySize = definition.getCount();
		m_value = PYXValue::create(PYXValue::knDouble, 0, m_nArraySize, 0);
		m_done = false;
	}

	HighestResolutionBlender(const HighestResolutionBlender & blender)
	{
		m_nArraySize = blender.m_nArraySize;
		m_value = blender.m_value;
		m_done = false;
	}

	virtual void reset()
	{
		for (int n = 0; n != m_nArraySize; ++n)
		{
			m_value.setDouble(n, 0);
		}
		m_sampleCount = 0;
		m_resolution = 0;
		m_done = false;
	}

	static PYXPointer<HighestResolutionBlender> create(const PYXFieldDefinition & definition) 
	{
		return PYXNEW(HighestResolutionBlender,definition);
	}

	virtual PYXPointer<ValueBlender> clone()
	{
		return PYXNEW(HighestResolutionBlender,*this);
	}

	void blend(int visiblieResolution,int covResolution,const PYXValue & covValue)
	{
		//we don't use it...
		if (m_resolution > covResolution)
		{
			return;
		}
		else if (m_resolution < covResolution)
		{
			for (int n = 0; n != m_nArraySize; ++n)
			{
				m_value.setDouble(n, covValue.getDouble(n));
			}
			m_sampleCount = 1;
			m_resolution = covResolution;
		}
		else
		{
			for (int n = 0; n != m_nArraySize; ++n)
			{
				m_value.setDouble(n, m_value.getDouble(n) + covValue.getDouble(n));
			}
			++m_sampleCount;
		}
	}
	
	virtual const PYXValue & getValue() 
	{
		if (m_sampleCount>1 && !m_done)
		{
			for (int n = 0; n != m_nArraySize; ++n)
			{
				m_value.setDouble(n, m_value.getDouble(n) / m_sampleCount);
			}
		}
		m_done = true;

		return m_value;
	}
};

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

IProcess::eInitStatus Blender::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	PYXPointer<State> newState = State::create();

	// Set up inputs
	for (int nInput = 0; nInput != getParameter(0)->getValueCount(); ++nInput)
	{
		boost::intrusive_ptr<ICoverage> spCov = getParameter(0)->getValue(nInput)->getOutput()->QueryInterface<ICoverage>();
		assert(spCov);
		newState->addInput(spCov,spCov->getGeometry()->getCellResolution());		
	}

	// Set up coverage definition
	m_spCovDefn = PYXTableDefinition::create();

	for (int nOffset = 0; nOffset < newState->getInputCount(); ++nOffset)
	{
		// get the n-th cov defn.
		PYXPointer<PYXTableDefinition> currentCov = 
			newState->getInput(nOffset)->getCoverageDefinition();

		assert(0 < currentCov->getFieldCount());
		if(m_spCovDefn->getFieldCount() == 0)
		{
			assert(currentCov->getFieldDefinition(0).getType() != PYXValue::knNull &&
					"Can't have null field definition.");

			// if it's a valid coverage
			m_spCovDefn = newState->getInput(nOffset)->getCoverageDefinition()->clone();
		}
		else
		{
			// verify that the input matches the output coverage defn.
			if (m_spCovDefn->getFieldCount() != currentCov->getFieldCount())
			{
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError("Field counts between coverages do not match.");
				return knFailedToInit;
			}

			// Ensure that field definitions between coverages match.
			for (int nIndex = 0; nIndex < m_spCovDefn->getFieldCount(); ++nIndex)
			{
				//if the fields count is different we can't blend
				if (currentCov->getFieldDefinition(nIndex).getCount() !=
					m_spCovDefn->getFieldDefinition(nIndex).getCount())
				{
					m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
					m_spInitError->setError("Trying to blend two processes that do not have compatible fields.");
					return knFailedToInit;
				}

				if (currentCov->getFieldDefinition(nIndex).getType() !=
					m_spCovDefn->getFieldDefinition(nIndex).getType())
				{
					PYXValue::eType outputType = findCompatibleBlendOutput(currentCov->getFieldDefinition(nIndex).getType(),m_spCovDefn->getFieldDefinition(nIndex).getType());

					if (outputType != PYXValue::knNull)
					{
						m_spCovDefn = changeDefinitionOutputType(m_spCovDefn,nIndex,outputType);
					}
					else
					{
						m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
						m_spInitError->setError("Trying to blend two processes that do not have compatible fields.");
						return knFailedToInit;
					}
				}
			}
		}
	}

	if (m_spCovDefn->getFieldCount() == 0)
	{
		// add a null field
		m_spCovDefn->addNullField();
	}

	newState->setCoverageDefinition(getCoverageDefinition()->clone());

	switch (m_mode)
	{
	case knConstant:
		if (m_spCovDefn->getFieldDefinition(0).getType() == PYXValue::knDouble || m_spCovDefn->getFieldDefinition(0).getType() == PYXValue::knFloat)
		{
			newState->setBlender(ConstantBlenderDouble::create(m_spCovDefn->getFieldDefinition(0)));
		}
		else
		{
			newState->setBlender(ConstantBlenderInt::create(m_spCovDefn->getFieldDefinition(0)));
		}
		break;

	case knResolutionDependent:
		newState->setBlender(ResolutionDependentBlender::create(m_spCovDefn->getFieldDefinition(0)));
		break;

	case knUseHighestResolution:
		newState->setBlender(HighestResolutionBlender::create(m_spCovDefn->getFieldDefinition(0)));
		break;
	}

	m_strID = "Blender: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	//switch to the new state!
	m_state = newState;
	m_spGeom.reset();

	return knInitialized;
}

std::string Blender::getIdentity() const
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	PYXPointer<ProcessSpec> spSpec = getSpec();
	assert(spSpec);

	ProcessIdentity identity(spSpec->getClass());
	identity.setData(getData());
	identity.setAttributes(getAttributes());

	const int nParameterCount = getParameterCount();
	for (int n = 0; n < nParameterCount; ++n)
	{
		PYXPointer<Parameter> spParam = getParameter(n);
		assert(spParam);
		identity.addInput(*spParam, false);
	}

	return identity();
}


std::map<std::string, std::string> STDMETHODCALLTYPE Blender::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	switch(m_mode)
	{
	case knConstant:
		mapAttr["Mode"] = "Constant";
		break;

	case knResolutionDependent:
		mapAttr["Mode"] = "ResolutionDependent";
		break;

	case knUseHighestResolution:
		mapAttr["Mode"] = "UseHighestResolution";
		break;
	}	

	return mapAttr;
}

std::string STDMETHODCALLTYPE Blender::getAttributeSchema() const
{
	return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:simpleType name=\"BlendMode\">"
			"<xs:restriction base=\"xs:string\">"
			    "<xs:enumeration value=\"Constant\" />"
				"<xs:enumeration value=\"ResolutionDependent\" />"
				"<xs:enumeration value=\"UseHighestResolution\" />"
			"</xs:restriction>"
		  "</xs:simpleType>"
		  "<xs:element name=\"Blender\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"Mode\" type=\"BlendMode\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Blend Mode</friendlyName>"
					"<description>Blending mode to applay to input coverage.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"	
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";	
}

void STDMETHODCALLTYPE Blender::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::string mode;
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"Mode",mode);

	if (mode == "Constant")
	{
		m_mode = knConstant;
	}
	else if (mode == "ResolutionDependent")
	{
		m_mode = knResolutionDependent;
	}
	else
	{
		m_mode = knUseHighestResolution;
	}
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue Blender::getCoverageValue(	const PYXIcosIndex& index,
									int nFieldIndex	) const
{
	PYXPointer<State> state;
	PYXPointer<ValueBlender> blender;
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);

		state = m_state;

		blender = state->getBlender()->clone();
	}

	PYXValue val;

	if (state->getInputCount() != 0)
	{
		const PYXFieldDefinition& fieldDefn = state->getCoverageDefinition()->getFieldDefinition(0);
		int nArraySize = fieldDefn.getCount();

		blender->reset();

		bool hasSample = false;

		for (int nInput = 0; nInput != state->getInputCount(); ++nInput)
		{
			PYXValue valInput = state->getInput(nInput)->getCoverageValue(index, nFieldIndex);
			if (!valInput.isNull())
			{
				blender->blend(index.getResolution(),state->getInputResolution(nInput),valInput);
				hasSample = true;
			}
		}

		if (hasSample)
		{
			val = PYXValue::create(fieldDefn.getType(), 0, nArraySize, 0);
			for (int n = 0; n != nArraySize; ++n)
			{
				val.setDouble(n, blender->getValue().getDouble(n) );
			}
		}
	}

	return val;
}

PYXCost STDMETHODCALLTYPE Blender::getFieldTileCost(	const PYXIcosIndex& index,
														int nRes,
														int nFieldIndex ) const
{	
	PYXPointer<State> state;
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);

		state = m_state;
	}

	if (state->getInputCount() == 0)
	{
		return PYXCost::knImmediateCost;
	}

	if (state->getInputCount() == 1)
	{
		return state->getInput(0)->getFieldTileCost(index, nRes, nFieldIndex);
	}

	PYXCost cost;
	for (int nInput = 0; nInput != state->getInputCount(); ++nInput)
	{
		cost += state->getInput(nInput)->getFieldTileCost(index, nRes, nFieldIndex);
	}
	//TODO: realy caclulate the blending cost
	PYXCost blendCost = PYXCost::knImmediateCost;

	return cost + blendCost;
}

PYXPointer<PYXValueTile> Blender::getFieldTile(	const PYXIcosIndex& index,
												int nRes,
												int nFieldIndex	) const
{
	PYXPointer<State> state;
	PYXPointer<ValueBlender> blender;
	{
		boost::recursive_mutex::scoped_lock lock(m_procMutex);

		state = m_state;
		blender = state->getBlender()->clone();
	}

	if (state->getInputCount() == 1)
	{
		return state->getInput(0)->getFieldTile(index, nRes, nFieldIndex);
	}

	PYXPointer<PYXValueTile> spValueTile;

	if (state->getInputCount() != 0)
	{
		std::vector<PYXPointer<PYXValueTile>> vecInputTile;
		std::vector<int> vecInputResolution;

		for (int nInput = 0; nInput != state->getInputCount(); ++nInput)
		{
			PYXPointer<PYXValueTile> spInputTile = state->getInput(nInput)->getFieldTile(index, nRes, nFieldIndex);
			if (spInputTile)
			{
				vecInputTile.push_back(spInputTile);
				vecInputResolution.push_back(state->getInputResolution(nInput));
			}
		}

		int nVecSize = static_cast<int>(vecInputTile.size());

		if (nVecSize == 1)
		{
			//TODO[shatzi]: if the blend did some upcast - we need to change the output type type as well.

			// we only had one tile that returned data.
			return vecInputTile[0];
		}

		if (nVecSize != 0)
		{
			const PYXFieldDefinition& fieldDefn = state->getCoverageDefinition()->getFieldDefinition(0);
			int nArraySize = fieldDefn.getCount();
			spValueTile = PYXValueTile::create(index, nRes, state->getCoverageDefinition());

			int nCellCount = spValueTile->getNumberOfCells();

			std::vector<PYXValue> valInputs;
			for (int nInput = 0; nInput != nVecSize; ++nInput)
			{
				valInputs.push_back(vecInputTile[nInput]->getTypeCompatibleValue(0));
			}
			PYXValue valOutput = PYXValue::create(fieldDefn.getType(), 0, nArraySize, 0);

			bool isOutputTypeDouble = valOutput.getType() == PYXValue::knFloat || valOutput.getType() == PYXValue::knDouble;

			// TODO: use new getValue from ValueTile that takes a *PYXValue
			// TODO: optimize this loop
			for (int nCell = 0; nCell != nCellCount; ++nCell)
			{
				blender->reset();
				bool hasSample = false;
				
				for (int nInput = 0; nInput != nVecSize; ++nInput)
				{
					if (vecInputTile[nInput]->getValue(nCell,0,&valInputs[nInput]))
					{
						blender->blend(nRes,vecInputResolution[nInput],valInputs[nInput]);
						hasSample = true;
					}
				}

				//if (nInputCount != 0)
				if (hasSample)
				{
					if (isOutputTypeDouble)
					{
						for (int n = 0; n != nArraySize; ++n)
						{
							valOutput.setDouble(n, blender->getValue().getDouble(n) );
						}
						spValueTile->setValue(nCell, 0, valOutput);
					}
					else
					{
						for (int n = 0; n != nArraySize; ++n)
						{
							valOutput.setInt32(n, blender->getValue().getInt32(n) );
						}
						spValueTile->setValue(nCell, 0, valOutput);
					}
				}
			}
		}
	}

	return spValueTile;
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

void Blender::createGeometry() const
{
	//load geometry if we have a stable identity...
	PYXPointer<PYXLocalStorage> storage;

	//TODO[shatzi]: our default pipelines doesn't have stable id (there is no checksum on the input files)
	//if (PipeUtils::isPipelineIdentityStable(const_cast<Blender*>(this)))
	{
		storage = PYXProcessLocalStorage::create(getIdentity());

		std::auto_ptr<PYXConstWireBuffer> geomBuffer = storage->get("blender:geom");

		if (geomBuffer.get() != 0)
		{
			(*geomBuffer) >> m_spGeom;

			if (m_spGeom)
			{
				return;
			}
		}
	}

	//create the geometry from inputs
	m_spGeom = PYXEmptyGeometry::create();
	for (int nInput = 0; nInput != getParameter(0)->getValueCount(); ++nInput)
	{
		PYXPointer<PYXGeometry> spGeometry = m_state->getInput(nInput)->getGeometry();
		if (0 != spGeometry)
		{
			// Union between itself and the geometry.
			m_spGeom = m_spGeom->disjunction(*spGeometry);
		}
	}

	//write the result to the storage if we can..
	if (storage)
	{
		PYXStringWireBuffer buffer;
		buffer << *m_spGeom;

		storage->set("blender:geom",buffer);
	}
}

PYXValue::eType Blender::findCompatibleBlendOutput(PYXValue::eType typeA,PYXValue::eType typeB) const
{
	if (! PYXValue::isNumeric(typeA) || ! PYXValue::isNumeric(typeB))
	{
		return PYXValue::knNull;
	}

	if (typeA > typeB)
	{
		std::swap(typeA,typeB);
	}

	if (typeA == typeB)
	{
		return typeA;
	}

	//switch over the maximum type
	switch(typeB)
	{	
	case PYXValue::knBool:
	case PYXValue::knChar:
		return PYXValue::knNull;
	
	case PYXValue::knInt8:
	case PYXValue::knInt16:
	case PYXValue::knInt32:
		return typeB;

	case PYXValue::knUInt8:
		if (PYXValue::isSigned(typeA))
		{
			return PYXValue::knInt16;
		}
		return PYXValue::knUInt8;
	
	case PYXValue::knUInt16:
		if (PYXValue::isSigned(typeA))
		{
			return PYXValue::knInt32;
		}
		return PYXValue::knUInt16;

	case PYXValue::knUInt32:
		if (PYXValue::isSigned(typeA))
		{
			return PYXValue::knDouble;
		}
		return PYXValue::knUInt32;

	case PYXValue::knFloat:
	case PYXValue::knDouble:
		return typeB;
	
	case PYXValue::knString:
	case PYXValue::knNull:
		return PYXValue::knNull;
	default:
		assert(false && "Unknown data type.");
		return PYXValue::knNull;
	}
}

PYXPointer<PYXTableDefinition> Blender::changeDefinitionOutputType(PYXPointer<PYXTableDefinition> definition,int nIndex,PYXValue::eType outType) const
{
	PYXPointer<PYXTableDefinition> newDefinition = PYXTableDefinition::create();

	for(int i=0;i<definition->getFieldCount();i++)
	{
		const PYXFieldDefinition & field = definition->getFieldDefinition(i);

		if (i==nIndex)
		{
			//generate a new field definition with a different output type
			newDefinition->addFieldDefinition(field.getName(),field.getContext(),outType,field.getCount());
		}
		else
		{
			//just copy the definition
			newDefinition->addFieldDefinition(field);
		}
	}

	return newDefinition;
}