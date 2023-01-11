/******************************************************************************
multi_res_spatial_analysis_process.cpp

begin		: 2006-09-05
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "multi_res_spatial_analysis_process.h"

// local includes
#include "channel_selector_process.h"

// pyxlib includes
#include "pyxis/derm/index.h"
#include "pyxis/procs/const_coverage.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/exception.h"

// {AF57FF42-F97E-4cea-B60D-FA89A9CEB779}
PYXCOM_DEFINE_CLSID(MultiResSpatialAnalysisProcess,
0xaf57ff42, 0xf97e, 0x4cea, 0xb6, 0xd, 0xfa, 0x89, 0xa9, 0xce, 0xb7, 0x79);

PYXCOM_CLASS_INTERFACES(
	MultiResSpatialAnalysisProcess, IProcess::iid,ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(MultiResSpatialAnalysisProcess, 
					"Multi-Resolution Spatial Analysis Process", 
					"Applies a multi-resolutional spectral analysis algorithm to input.",
					"Drop", // "Image Processing",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Image", "The coverage to run through the MrSap algorithm")
IPROCESS_SPEC_END

//! The unit test class
Tester<MultiResSpatialAnalysisProcess> gTester;

// Constants
const std::string MultiResSpatialAnalysisProcess::kstrSteps = "Number_of_Steps";

/*!
The unit test method for the class.
*/
void MultiResSpatialAnalysisProcess::test()
{
	//TODO[shatzi,nov 2012]: I remove those test because they take tool long.
	return;

	uint8_t channel_1[3] = {180, 180, 180};
	uint8_t channel_2[3] = {90, 90, 90};
	uint8_t channel_3[3] = {60, 60, 60};
	uint8_t testVal[3] = {0,0,0};
	uint8_t oddChannel[4] = {200, 200, 200, 200};
	int nSteps = 3;

	PYXIcosIndex rootIndex = "A-000000000";
	PYXIcosIndex index1 = "A-01000";
	PYXIcosIndex index2 = "5-000000";
	PYXIcosIndex index3 = "C-020504";
	PYXIcosIndex index4 = "C-02050204";
	int channelSelected = 0;

	boost::intrusive_ptr<ConstCoverage> spConstCoverage(new ConstCoverage);
	boost::intrusive_ptr<ConstCoverage> spOddCoverage (new ConstCoverage);
	boost::intrusive_ptr<ConstCoverage> spNullCoverage(new ConstCoverage);
	spNullCoverage->setReturnValue(PYXValue(), PYXFieldDefinition::knContextRGB);
	
	spConstCoverage->setFieldCount(3);


	spConstCoverage->setReturnValue(PYXValue(channel_1,3),PYXFieldDefinition::knContextRGB,0);
	spConstCoverage->setReturnValue(PYXValue(channel_2,3),PYXFieldDefinition::knContextRGB,1);
	spConstCoverage->setReturnValue(PYXValue(channel_3,3),PYXFieldDefinition::knContextRGB,2);
	
	spOddCoverage->setFieldCount(3);
	spOddCoverage->setReturnValue(PYXValue(channel_1,3),PYXFieldDefinition::knContextRGB,0);
	spOddCoverage->setReturnValue(PYXValue(oddChannel,4),PYXFieldDefinition::knContextNone,1);
	spOddCoverage->setReturnValue(PYXValue(channel_3,3),PYXFieldDefinition::knContextRGB,2);
	boost::intrusive_ptr<MultiResSpatialAnalysisProcess> spMultiResProcess (new MultiResSpatialAnalysisProcess);


	boost::intrusive_ptr<ChannelSelectorProcess> spChannelSelectorProc (new ChannelSelectorProcess);

	{ // Set Attributes and inputs for MRSAP.
		std::map<std::string, std::string> attribs;
		attribs.clear();
		attribs[kstrSteps] = intToString(nSteps,0);

		PYXPointer<ParameterSpec> spParamSpec = 
			ParameterSpec::create(ICoverage::iid, 1, 1, "input coverage", "");
		PYXPointer<Parameter> spParam = Parameter::create(spParamSpec);
		spParam->addValue(spConstCoverage);
		std::vector<PYXPointer<Parameter> > vecParam;
		vecParam.push_back(spParam);
		spMultiResProcess->setParameters(vecParam);
		spMultiResProcess->setAttributes(attribs);
		TEST_ASSERT(spMultiResProcess->initProc(true) == IProcess::knInitialized);
	}

	{ // Testing with Channel Zero selected.
		{ // Set Attributes and inputs for Channel Selector
			std::map<std::string, std::string> attribs;
			attribs.clear();
			attribs[ChannelSelectorProcess::kstrSelectedChannel] = intToString(channelSelected,0);
			PYXPointer<ParameterSpec> spParamSpec = 
				ParameterSpec::create(ICoverage::iid, 1, 1, "Multi-Channel input.", "");
			PYXPointer<Parameter> spParam = Parameter::create(spParamSpec);
			spParam->addValue(spMultiResProcess);
			std::vector<PYXPointer<Parameter> > vecParam;
			vecParam.push_back(spParam);
			spChannelSelectorProc->setParameters(vecParam);
			TEST_ASSERT(spChannelSelectorProc->initProc(true) == IProcess::knInitialized);
			spChannelSelectorProc->setAttributes(attribs);
			TEST_ASSERT(spChannelSelectorProc->initProc(true) == IProcess::knInitialized);
		}
	
		PYXValue testVal(testVal, 3);
		
		TEST_ASSERT(testVal == spChannelSelectorProc->getCoverageValue(rootIndex));
		TEST_ASSERT(testVal == spChannelSelectorProc->getCoverageValue(index1));
		TEST_ASSERT(testVal == spChannelSelectorProc->getCoverageValue(index2));
		TEST_ASSERT(testVal == spChannelSelectorProc->getCoverageValue(index3));
		TEST_ASSERT(testVal == spChannelSelectorProc->getCoverageValue(index4));
	}
	
	channelSelected = 1; 
	{ // Testing with channel 1 selected.
		{
			std::map<std::string, std::string> attribs;
			attribs.clear();
			attribs[ChannelSelectorProcess::kstrSelectedChannel] = intToString( channelSelected, 0);
			spChannelSelectorProc->setAttributes(attribs);
			TEST_ASSERT(spChannelSelectorProc->initProc(true) == IProcess::knInitialized);
		}
		
		
		PYXValue testVal(testVal, 3);
		
		TEST_ASSERT(testVal == spChannelSelectorProc->getCoverageValue(rootIndex));
		TEST_ASSERT(testVal == spChannelSelectorProc->getCoverageValue(index1));
		TEST_ASSERT(testVal == spChannelSelectorProc->getCoverageValue(index2));
		TEST_ASSERT(testVal == spChannelSelectorProc->getCoverageValue(index3));
		TEST_ASSERT(testVal == spChannelSelectorProc->getCoverageValue(index4));
	}
	channelSelected = 2;

	{ // Testing with channel 2 selected.
		{
			std::map<std::string, std::string> attribs;
			attribs.clear();
			attribs[ChannelSelectorProcess::kstrSelectedChannel] = intToString( channelSelected, 0);
			spChannelSelectorProc->setAttributes(attribs);
			TEST_ASSERT(spChannelSelectorProc->initProc(true) == IProcess::knInitialized);
		}
		
		PYXValue testVal(testVal, 3);
		
		TEST_ASSERT(testVal == spChannelSelectorProc->getCoverageValue(rootIndex));
		TEST_ASSERT(testVal == spChannelSelectorProc->getCoverageValue(index1));
		TEST_ASSERT(testVal == spChannelSelectorProc->getCoverageValue(index2));
		TEST_ASSERT(testVal == spChannelSelectorProc->getCoverageValue(index3));
		TEST_ASSERT(testVal == spChannelSelectorProc->getCoverageValue(index4));
	}

	{ // Set Attributes and inputs for Channel Selector
		std::map<std::string, std::string> attribs;
		attribs.clear();
		attribs[ChannelSelectorProcess::kstrSelectedChannel] = intToString(channelSelected,0);
		PYXPointer<ParameterSpec> spParamSpec = 
			ParameterSpec::create(ICoverage::iid, 1, 1, "Multi-Channel input.", "");
		PYXPointer<Parameter> spParam = Parameter::create(spParamSpec);
		spParam->addValue(spMultiResProcess);
		std::vector<PYXPointer<Parameter> > vecParam;
		vecParam.push_back(spParam);
		spChannelSelectorProc->setParameters(vecParam);
		TEST_ASSERT(spChannelSelectorProc->initProc(true) == IProcess::knInitialized);
		spChannelSelectorProc->setAttributes(attribs);
		TEST_ASSERT(spChannelSelectorProc->initProc(true) == IProcess::knInitialized);
	}

	int nVal = spChannelSelectorProc->getCoverageValue(rootIndex).getInt(0);
	
	{
		PYXPointer<ParameterSpec> spParamSpec = 
			ParameterSpec::create(ICoverage::iid, 1, 1, "input coverage", "");
		PYXPointer<Parameter> spParam = Parameter::create(spParamSpec);
		spParam->addValue(spOddCoverage);
		std::vector<PYXPointer<Parameter> > vecParam;
		vecParam.push_back(spParam);
		spMultiResProcess->setParameters(vecParam);
		TEST_ASSERT(spMultiResProcess->initProc(true) == IProcess::knInitialized);
	}

}

IProcess::eInitStatus MultiResSpatialAnalysisProcess::initImpl()
{
	m_strID = "MrSAP: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	m_spBlurProc = 0;
	PYXCOMCreateInstance(
		strToGuid("{32C48166-6FDB-46fc-9FAB-705E7469F9A4}"), 0,
		ICoverage::iid, (void**) &m_spBlurProc);

	m_spInputCoverage = getParameter(0)->getValue(0)->getOutput()->QueryInterface<ICoverage>();

	assert(m_spBlurProc && m_spInputCoverage);
	boost::intrusive_ptr<IProcess> spBlurProc;
	m_spBlurProc->QueryInterface(IProcess::iid, (void**) &spBlurProc);
	PYXPointer<ParameterSpec> spParamSpec = 
		ParameterSpec::create(m_spInputCoverage->iid, 1, 1, "Input Coverage", "Input to the process");
	PYXPointer<Parameter> spParam = Parameter::create(spParamSpec);
	boost::intrusive_ptr<IProcess> spProcToAdd;
	m_spInputCoverage->QueryInterface(IProcess::iid, (void**) &spProcToAdd);
	spParam->addValue(spProcToAdd);
	std::vector<PYXPointer<Parameter> > vecParam;
	vecParam.push_back(spParam);
	spBlurProc->setParameters(vecParam);
	spBlurProc->setAttributes(getAttributes());
	spBlurProc->initProc();
	createMetaData();
	return knInitialized;
}


/*! 
Gets the attributes to that need to be serialized or displayed in the pipe editor
with this process. The attributes are entered into a map of string(key)
to string(values) and the map is retuned to be written out.

\return A stl map containing the attributes to be saved, or returned to the pipe editor.
*/
std::map<std::string, std::string> MultiResSpatialAnalysisProcess::getAttributes() const
{
	std::map<std::string, std::string> attrib;
	attrib.clear();
	attrib[kstrSteps] = intToString(m_nNumberOfSteps, 0);
	return attrib;
}

/*!
Sets the attributes either when the process is deserializing or when
the attributes have been altered in the pipe editor. The stl map 
passed in is searched for key-value pairs and the values are parsed
out into their respective variables. 

\param mapAttr A map of the attributes to be set in this process.
*/
void MultiResSpatialAnalysisProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string, std::string>::const_iterator it = mapAttr.find(kstrSteps);
	if (it != mapAttr.end())
	{
		m_nNumberOfSteps = atoi(const_cast<char*>(it->second.c_str()));
	}
}

/*!
Helper method to create the meta data which describes this multi channel coverage.
Each call to this method creates a new coverage definition and replaces the existing one.
every channel from recieve it's own field definition. 
*/
void MultiResSpatialAnalysisProcess::createMetaData()
{ 
	m_spCovDefn = PYXTableDefinition::create();
	PYXFieldDefinition currentDefinition = 	
	m_spInputCoverage->getCoverageDefinition()->getFieldDefinition(0);

	for (int nStep = 0; nStep <= m_nNumberOfSteps; ++nStep)
	{
		std::string strTemp;
		std::string strFieldName;
		std::stringstream ss; 
		ss << nStep; 
		ss >> strTemp;
		strFieldName = "Channel: " + strTemp;
		if (nStep == 0)
		{
			strFieldName.append(" Highest Frequency Data");
		}
		if (nStep == m_nNumberOfSteps)
		{
			strFieldName.append(" Lowest Frequency Data");
		}
		m_spCovDefn->addFieldDefinition(
			strFieldName,currentDefinition.getContext(),currentDefinition.getType(),
			currentDefinition.getCount() );
	}
}

/*!
Calculates the coverage value to return. The value returned for a 
particular channel is if a value is requested for channel n, then the 
value returned is channel n - channel n + 1. The higher the channel value
requested the lower the frequency data is displayed in the image. Therefore
a channel value of ten will yield a very low pass frequency. A channel 
value of one will display the very high frequency of the image. This 
is describe in the Mr. SAP algorithm. The blur filter performs the 
zoom in and out operations and this filter performs the subtraction 
of the zoom in/out images, completing MR. SAP.

\param index		The PYXIcosIndex that the value is requested for.
\param nFieldIndex	The channel index that a value is requested for at 
					a specific index.

\return		A PYXValue representing either high, medium, low, really low
			frequency data from the image.
*/
PYXValue MultiResSpatialAnalysisProcess::getCoverageValue(
	const PYXIcosIndex& index, int nFieldIndex) const
{
	if (index.isNull())
	{
		PYXTHROW(PYXException, "Cannot have a null index.");
	}
	if (nFieldIndex < 0 || nFieldIndex > m_nNumberOfSteps)
	{
		PYXTHROW(PYXException, "Field index must be between 0 - " + intToString(m_nNumberOfSteps, 0));
	}
	if (!m_spBlurProc)
	{
		PYXTHROW(PYXException, "Cannot perform analysis, Blur process not initialized.");
	}

	PYXValue leftHandSideValue = m_spBlurProc->getCoverageValue(index, nFieldIndex);
	TRACE_DEBUG("Left hand Side Value: " << leftHandSideValue);

	// the lowest frequncy data is just the last channel of the blur filter.
	if (nFieldIndex == m_nNumberOfSteps)
	{
		return leftHandSideValue;
	}

	PYXValue rightHandSideValue = m_spBlurProc->getCoverageValue(index, (nFieldIndex + 1));
	TRACE_DEBUG("Right hand size Value: " << rightHandSideValue);
	PYXValue rtnValue(leftHandSideValue);
	
	if ((leftHandSideValue.getArraySize() != rightHandSideValue.getArraySize()) ||
		leftHandSideValue.getArrayType() != rightHandSideValue.getArrayType() || 
		leftHandSideValue.isNull() || rightHandSideValue.isNull() )
	{
		return rtnValue;
	}

	int valArraySize = leftHandSideValue.getArraySize();

	for (int nArrayElement = 0; nArrayElement < valArraySize; ++nArrayElement)
	{
		double fValue = 
			leftHandSideValue.getDouble(nArrayElement) - rightHandSideValue.getDouble(nArrayElement);
		rtnValue.setDouble(nArrayElement, fValue);
	}

	return rtnValue;
}

/*!
Creates a geometry. Since the MRSAP is being performed on our orginal input. It
is ok to create the geometry by cloning our input's geometry.
*/
void MultiResSpatialAnalysisProcess::createGeometry() const
{
	if (!m_spInputCoverage)
	{
		PYXTHROW(PYXException, "Cannot create geometry. Input coverage value missing.");
	}
	m_spGeom = m_spInputCoverage->getGeometry()->clone();
}
