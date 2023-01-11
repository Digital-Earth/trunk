/******************************************************************************
coverage_mask_process.cpp

begin		: 2006-09-06
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "coverage_mask_process.h"

//  pyxlib includes
#include "pyxis/derm/index.h"
#include "pyxis/procs/const_coverage.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/exception.h"

// {6C52C788-46B3-4324-B451-B3FCA6D771E0}
PYXCOM_DEFINE_CLSID(CoverageMaskProcess,
0x6c52c788, 0x46b3, 0x4324, 0xb4, 0x51, 0xb3, 0xfc, 0xa6, 0xd7, 0x71, 0xe0);

PYXCOM_CLASS_INTERFACES(CoverageMaskProcess, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(CoverageMaskProcess, "Mask by Value", "Masks input coverage based on mask Coverage.", "Analysis/Coverages",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Data Coverage", "The main data input coverage.")
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Mask Coverage", "The control coverage to use as a mask for the data coverage.")
IPROCESS_SPEC_END

//! Constant string data.
const std::string CoverageMaskProcess::kstrScope = "CoverageMaskProcess";

const char* const opStr[] =
{
	"Mask has data",
	"Mask does not have data",
	"Mask data equals",
	"Mask data does not equal"
};

//! The unit test class
Tester<CoverageMaskProcess> gTester;

/*!
The unit test method for the class.
*/
void CoverageMaskProcess::test()
{
	//Our Inputs
	boost::intrusive_ptr<ConstCoverage> spInputCoverage(new ConstCoverage);
	boost::intrusive_ptr<ConstCoverage> spNullCoverage (new ConstCoverage);

	// places to look for data
	PYXIcosIndex index1 = "A-0500";
	PYXIcosIndex index2 = "A-02000000";
	PYXIcosIndex index3 = "3-02001040004";  // near Kingston
	PYXValue nullPYXValue;
	uint8_t initArray [3] = {200, 200, 200};
	PYXValue nonNullPYXValue = PYXValue::create(PYXValue::knUInt8, &initArray, 3, 0);
	
	spInputCoverage->setReturnValue(nonNullPYXValue, PYXFieldDefinition::knContextRGB);
	boost::intrusive_ptr<IProcess> spInput;
	spInputCoverage->QueryInterface(IProcess::iid, (void**) &spInput);
	spNullCoverage->setReturnValue(nullPYXValue, PYXFieldDefinition::knContextNone);
	boost::intrusive_ptr<IProcess> spControlInput;
	spNullCoverage->QueryInterface(IProcess::iid, (void**) &spControlInput);
	
	{
		//Create a mask process set up it's inputs and configuration.
		boost::intrusive_ptr<ICoverage> spMask;
		PYXCOMCreateInstance(
			CoverageMaskProcess::clsid,
			0, ICoverage::iid, (void**) &spMask);
		boost::intrusive_ptr<IProcess> spMaskAsProcess;
		spMask->QueryInterface(IProcess::iid, (void**) &spMaskAsProcess);

		PYXPointer<ParameterSpec> spParamSpec = 
		ParameterSpec::create(ICoverage::iid, 1, 1, "Input Coverage", "");
		PYXPointer<Parameter> spParam = Parameter::create(spParamSpec);
		spParam->addValue(spInputCoverage);
		std::vector<PYXPointer<Parameter> > vecParams;
		vecParams.push_back(spParam);
		vecParams.push_back(spParam);

		spMaskAsProcess->setParameters(vecParams);
		std::map<std::string, std::string> mapAttr;

		//Set up to return if control has data.
		mapAttr["DataValue"] = "uint8_t[3] 0 0 0";
		mapAttr["OperatingMode"] = opStr[knReturnIfControlHasData];
		spMaskAsProcess->setAttributes(mapAttr);
		spMaskAsProcess->initProc();

		// Test that the value being returned is the value from the input coverage.
		TEST_ASSERT(spMask->getCoverageValue(index1, 0) == 
		spInputCoverage->getCoverageValue(index1, 0));   
		TEST_ASSERT(spMask->getCoverageValue(index2, 0) == 
		spInputCoverage->getCoverageValue(index2, 0));   
		TEST_ASSERT(spMask->getCoverageValue(index3, 0) == 
		spInputCoverage->getCoverageValue(index3, 0));   

		//// ask for data to come back if the control has no data
		mapAttr.clear();
		mapAttr["DataValue"] = "uint8_t[3] 0 0 0";
		mapAttr["OperatingMode"] = opStr[knReturnIfControlHasNoData];

		spMaskAsProcess->setAttributes(mapAttr);
		spMaskAsProcess->initProc();

		//// Test that the value being returned is a null PYXValue.
		TEST_ASSERT(spMask->getCoverageValue(index1, 0) == nullPYXValue);
		TEST_ASSERT(spMask->getCoverageValue(index2, 0) == nullPYXValue);
		TEST_ASSERT(spMask->getCoverageValue(index3, 0) == nullPYXValue);  
	}

	{
		//Create a mask process set up its inputs and configuration.
		boost::intrusive_ptr<ICoverage> spMask;
		PYXCOMCreateInstance(
			CoverageMaskProcess::clsid, 0,
			ICoverage::iid, (void**) &spMask);
		boost::intrusive_ptr<IProcess> spMaskAsProcess;
		spMask->QueryInterface(IProcess::iid, (void**) &spMaskAsProcess);

		PYXPointer<ParameterSpec> spParamSpec =
		ParameterSpec::create(ICoverage::iid, 1, 1, "Input Coverage", "");
		PYXPointer<Parameter> spParamInput = Parameter::create(spParamSpec);
		PYXPointer<Parameter> spParamControl = Parameter::create(spParamSpec);
		spParamInput->addValue(spInputCoverage);
		spParamControl->addValue(spNullCoverage);
		std::vector<PYXPointer<Parameter> > vecParams;
		vecParams.push_back(spParamInput);
		vecParams.push_back(spParamControl);
	
		spMaskAsProcess->setParameters(vecParams);
		std::map<std::string, std::string> mapAttr;

		//Set up to return if control has data.
		mapAttr["DataValue"] = "uint8_t[3] 0 0 0";
		mapAttr["OperatingMode"] = opStr[knReturnIfControlHasData];
		spMaskAsProcess->setAttributes(mapAttr);
		spMaskAsProcess->initProc();

		// Test that the value being returned is a null PYXValue.
		TEST_ASSERT(spMask->getCoverageValue(index1, 0) == nullPYXValue);
		TEST_ASSERT(spMask->getCoverageValue(index2, 0) == nullPYXValue);  
		TEST_ASSERT(spMask->getCoverageValue(index3, 0) == nullPYXValue);


		mapAttr.clear();

		//Set up to return if control has no data.
		mapAttr["DataValue"] = "uint8_t[3] 0 0 0";
		mapAttr["OperatingMode"] = opStr[knReturnIfControlHasNoData];

		spMaskAsProcess->setAttributes(mapAttr);
		spMaskAsProcess->initProc();

		// Test that the value being returned is the value from the input coverage.
		TEST_ASSERT(spMask->getCoverageValue(index1, 0) == 
		spInputCoverage->getCoverageValue(index1, 0));   
		TEST_ASSERT(spMask->getCoverageValue(index2, 0) ==
		 spInputCoverage->getCoverageValue(index2, 0));   
		TEST_ASSERT(spMask->getCoverageValue(index3, 0) == 
		spInputCoverage->getCoverageValue(index3, 0));  
	}
}

/*!
Default Constructor.
*/
CoverageMaskProcess::CoverageMaskProcess()
{
	m_mode = knReturnIfControlHasData;
	uint8_t val[3] = {0,0,0};
	m_compareValue = PYXValue::create(PYXValue::knUInt8,&val,3,0);
}


/*!
Intialize this process by querying the two input parameters for their values and 
storing those values in an input and control variable to eliminate the need for 
querying the interface every time we wish to access these values.
*/
IProcess::eInitStatus CoverageMaskProcess::initImpl()
{
	m_strID = "Coverage Mask: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));
	m_spInputCoverage = getParameter(0)->getValue(0)->getOutput()->QueryInterface<ICoverage>();
	m_spControlInput = getParameter(1)->getValue(0)->getOutput()->QueryInterface<ICoverage>();
	return knInitialized;
}

/*! 
Gets the attributes to that need to be serialized or displayed in the pipe editor
with this process. The attributes are entered into a map of string(key)
to string(values) and the map is retuned to be written out.

\return A stl map containing the attributes to be saved, or returned to the pipe editor.
*/
std::map<std::string, std::string> CoverageMaskProcess::getAttributes() const
{
	std::map<std::string, std::string> attrib;
	attrib.clear();

	attrib["DataValue"] = StringUtils::toString(m_compareValue);
	attrib["OperatingMode"] = opStr[m_mode];
	return attrib;
}

/*!
Sets the attributes either when the process is deserializing or when 
the attributes have been altered in the pipe editor. The stl map 
passed in is searched for key-value pairs and the values are parsed
out into their respective variables. 

\param attribs A map of the attributes to be set in this process.
*/
void CoverageMaskProcess::setAttributes(const std::map<std::string, std::string>& attribs)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string,std::string>::const_iterator it = attribs.find("DataValue");
	if (it != attribs.end())
	{
		StringUtils::fromString(it->second, &m_compareValue);
	}

	it = attribs.find("OperatingMode");
	if (it != attribs.end())
	{
		for (int n = 0; n != sizeof(opStr)/sizeof(opStr[0]); ++n)
		{
			if (strcmp(opStr[n], it->second.c_str()) == 0)
			{
				m_mode = static_cast<CoverageMaskProcess::eFilterMode>(n);
			}
		}
	}
}

/*!
Get the coverage value at specified index.
\param	index		The PYXIS index
\param	nFieldIndex	The field index.

\return	Data from the input based on the exsistence of data on the control.
*/
PYXValue CoverageMaskProcess::getCoverageValue(
	const PYXIcosIndex& index,
	int nFieldIndex	) const
{
	PYXValue controlValue = m_spControlInput->getCoverageValue(index, nFieldIndex);
	if ( (controlValue.isNull() && (m_mode == knReturnIfControlHasNoData)) ||
		 (!controlValue.isNull() && (m_mode == knReturnIfControlHasData))
		)
	{
		return m_spInputCoverage->getCoverageValue(index, nFieldIndex);
	}

	if ((m_mode == knReturnIfControlDataEquals || m_mode == knReturnIfControlDataNotEqual)
		&& !controlValue.isNull())
	{
		bool valuesEqual = (controlValue.getArraySize() <= m_compareValue.getArraySize());
		int valueIndex = 0;
		while (valuesEqual && valueIndex < controlValue.getArraySize())
		{
			valuesEqual = valuesEqual &&
				(controlValue.getDouble(valueIndex) == m_compareValue.getDouble(valueIndex));
			valueIndex++;
		}
		if ((valuesEqual && m_mode == knReturnIfControlDataEquals) ||
			(!valuesEqual && m_mode == knReturnIfControlDataNotEqual))
		{
			return m_spInputCoverage->getCoverageValue(index, nFieldIndex);
		}
	}

	// we failed the test, so return a null PYXValue
	return PYXValue(); 
}

PYXPointer<PYXValueTile> CoverageMaskProcess::getFieldTile(	const PYXIcosIndex& index,
															int nRes,
															int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	// our goal tile, which we are constructing	will be a copy of the data tile
	// with the data NULLed out where it does not pass the criteria.
	PYXPointer<PYXValueTile> spValueTile;

	{
		// get the data input tile
		PYXPointer<PYXValueTile> spInputTile =
			m_spInputCoverage->getFieldTile(index, nRes, nFieldIndex);

		// if we got a null tile from the input we are done processing.
		if (!spInputTile)
		{
			return spInputTile;
		}

		// copy all values into our output tile.
		spValueTile = spInputTile->cloneFieldTile(0);
	}

	PYXPointer<PYXValueTile> spControlTile =
		m_spControlInput->getFieldTile(index, nRes, nFieldIndex);

	// if we got a null tile from the control we can process the whole tile at one go.
	if (!spControlTile)
	{
		if ((m_mode == knReturnIfControlHasNoData) ||
		    (m_mode == knReturnIfControlDataEquals && m_compareValue.isNull()) ||
		    (m_mode == knReturnIfControlDataNotEqual && !m_compareValue.isNull()))
		{
			return spValueTile;
		}

		return spControlTile;
	}

	PYXValue controlValue = 
		m_spControlInput->getCoverageDefinition()->getFieldDefinition(nFieldIndex).getTypeCompatibleValue();
	PYXValue nullValue;
	int nCellCount = spValueTile->getNumberOfCells();
	for (int nCell = 0; nCell != nCellCount; ++nCell)
	{
		bool passValue;
		if (!spControlTile->getValue(nCell, 0, &controlValue))
		{
			// control had a null value
			passValue = (m_mode == knReturnIfControlHasNoData) ||
				        (m_mode == knReturnIfControlDataEquals && m_compareValue.isNull()) ||
						(m_mode == knReturnIfControlDataNotEqual && !m_compareValue.isNull());
		}
		else
		{
			if (m_mode == knReturnIfControlDataEquals || m_mode == knReturnIfControlDataNotEqual)
			{
				bool valuesEqual = (controlValue.getArraySize() <= m_compareValue.getArraySize());
				int valueIndex = 0;
				while (valuesEqual && valueIndex < controlValue.getArraySize())
				{
					valuesEqual = valuesEqual &&
						(controlValue.getDouble(valueIndex) == m_compareValue.getDouble(valueIndex));
					valueIndex++;
				}
				passValue = ((valuesEqual && m_mode == knReturnIfControlDataEquals) ||
							 (!valuesEqual && m_mode == knReturnIfControlDataNotEqual));
			}
			else
			{
				passValue = (m_mode == knReturnIfControlHasData);
			}
		}

		if (!passValue)
		{
			spValueTile->setValue(nCell, 0, nullValue);
		}
	}

	return spValueTile;
}

/*!
Create the geometry. Since the geometry is not altered. Then it is 
fine to just clone our input geometry.
*/
void CoverageMaskProcess::createGeometry() const
{
	assert(m_spInputCoverage && "Input coverage not initalized properly.");
	m_spGeom = m_spInputCoverage->getGeometry()->clone();
}


std::string CoverageMaskProcess::getAttributeSchema() const
{
	std::stringstream ss;
	ss << m_compareValue;
	std::string strXSD = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">";

	// add an enum type for operating mode.
	std::string strOperMode = "<xs:simpleType name=\"Mode\">"
		"<xs:restriction base=\"xs:string\">";
	for (int n = 0; n != sizeof(opStr)/sizeof(opStr[0]); ++n)
	{
		strOperMode += "<xs:enumeration value=\"";
		strOperMode += opStr[n];
		strOperMode += "\" />";
	}
	strOperMode += "</xs:restriction></xs:simpleType>";
	strXSD += strOperMode;

	strXSD +=
		"<xs:element name=\"CoverageMaskProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"OperatingMode\" type=\"Mode\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Operating Mode</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"DataValue\" type=\"xs:string\" default=\"" + ss.str() + "\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Data Value</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"			  
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";

	return strXSD;
}


