/******************************************************************************
const_coverage.cpp

begin		: 2006-04-18
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "const_coverage.h" 

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/pipe/pipe_manager.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/file_utils.h"

// boost includes
#include <boost/filesystem/path.hpp>
#include <boost/thread/xtime.hpp>

// {8517369E-B91F-46be-BC8A-82E3F414D6AA}
PYXCOM_DEFINE_CLSID(ConstCoverage,
0x8517369e, 0xb91f, 0x46be, 0xbc, 0x8a, 0x82, 0xe3, 0xf4, 0x14, 0xd6, 0xaa);
PYXCOM_CLASS_INTERFACES(ConstCoverage, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(ConstCoverage, "Constant Value", "A global data set that is constant across its geometry.", "Development/Tools",
						  ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END

// Tester class
Tester<ConstCoverage> gTester;

// Test method
void ConstCoverage::test()
{
	boost::intrusive_ptr<IProcess> spTestCoverageProcess(new ConstCoverage);
	ConstCoverage* pConstCoverage = dynamic_cast<ConstCoverage*>(spTestCoverageProcess.get());
	assert(pConstCoverage != 0);

	ConstCoverage testCoverage;
	TEST_ASSERT(testCoverage == *pConstCoverage);

	pConstCoverage->setFieldCount(8);
	TEST_ASSERT(testCoverage != *pConstCoverage);
	PYXPointer<PYXTableDefinition> spDefn = pConstCoverage->getCoverageDefinition();
	TEST_ASSERT(spDefn->getFieldCount() == 8);

	PYXIcosIndex index("A-000");
	PYXValue val;
	for (int n = 1; n < 8; ++n)
	{
		val = pConstCoverage->getCoverageValue(index, n);
		TEST_ASSERT(val.isNull());
	}

	val = pConstCoverage->getCoverageValue(index, 0);
	TEST_ASSERT(val.isArray());
	TEST_ASSERT(val.getArraySize() == 3);
	TEST_ASSERT(val.getArrayType() == PYXValue::knUInt8);

	std::string strValue = "test";
	PYXValue val2(strValue);
	pConstCoverage->setReturnValue(val2, PYXFieldDefinition::knContextClass);

	spDefn = pConstCoverage->getCoverageDefinition();
	TEST_ASSERT(spDefn->getFieldDefinition(0).getType() == PYXValue::knString);
	TEST_ASSERT(spDefn->getFieldDefinition(0).getContext() == PYXFieldDefinition::knContextClass);
	val = pConstCoverage->getCoverageValue(index);
	TEST_ASSERT(val == val2);

	pConstCoverage->setFieldCount(2);
	TEST_ASSERT(pConstCoverage->getCoverageDefinition()->getFieldCount() == 2);
	TEST_ASSERT(pConstCoverage->getCoverageDefinition()->getFieldDefinition(0).getType() == PYXValue::knString);
	val = pConstCoverage->getCoverageValue(index);
	TEST_ASSERT(val == val2);

	boost::filesystem::path tempPath = AppServices::makeTempFile(".ppl");
	TRACE_INFO("Example ConstCoverage file created: " << FileUtils::pathToString(tempPath));
	PipeManager::writeProcessToFile(FileUtils::pathToString(tempPath), spTestCoverageProcess);

	// TODO: Verify the process is read in from file correctly.
}

void ConstCoverage::colourChangeThread(ConstCoverage* cov)
{
	boost::xtime time;
	while (!cov->m_bNeedToStop)
	{
		// Sleep a bit.
		int nCount = 0;
		while (!cov->m_bNeedToStop && nCount < cov->m_nSecondsBetweenChange)
		{
			boost::xtime_get(&time, boost::TIME_UTC_);
			time.sec += 1;
			boost::thread::sleep(time);
			++nCount;
		}
		if (!cov->m_bNeedToStop)
		{
			unsigned char nColour[3];
			nColour[0] = rand() * 255 / RAND_MAX;
			nColour[1] = rand() * 255 / RAND_MAX;
			nColour[2] = rand() * 255 / RAND_MAX;
			PYXValue pyxValue(nColour, 3);
			// change the colour
			cov->setReturnValue(pyxValue, PYXFieldDefinition::knContextRGB, 0);
		}
	}
	cov->m_bIsStopped = true;
}


/*!
Creates a single channel constant coverage at resolution '10'  that returns an
array of 3 unsigend characters (RGB) with values of '42'.
*/
ConstCoverage::ConstCoverage() :
	m_nResolution(10)
{
	// TODO: find out why this AddRef is needed.
	m_RC.AddRef();

	setFieldCount(1);
	const unsigned char nValue[3] = {42, 42, 42};
	PYXValue pyxValue(nValue, 3);
	setReturnValue(pyxValue, PYXFieldDefinition::knContextRGB, 0);

	m_bIsStopped = true;
	m_bNeedToStop = false;

	m_bIsRandom = false;
	m_nSecondsBetweenChange = 10;
	setGeometryResolution(m_nResolution);

	m_RC.Release();
}

ConstCoverage::~ConstCoverage()
{
	if (!m_bIsStopped)
	{
		m_bNeedToStop = true;
		m_threads.join_all();
	}
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue ConstCoverage::getCoverageValue(	const PYXIcosIndex& index,
											int nFieldIndex	) const
{
	assert(nFieldIndex < static_cast<signed int>(m_vecFields.size()));
	return m_vecFields[nFieldIndex];
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

// TODO: A new version of this function exists below.  This copy is being 
// kept as we may be allowing this coverage to accept multiple 'context-value' 
// pairs and the code in the commented function will serve as reference.
//std::map<std::string, std::string> ConstCoverage::getAttributes() const
//{
//	std::map<std::string, std::string> mapAttr;
//	mapAttr["resolution"] = toString(m_nResolution);
//	mapAttr["field_count"] = toString(static_cast<signed int>(m_vecFields.size()));
//	
//	int nFieldOffset = 0;
//	std::vector<PYXValue>::const_iterator it = m_vecFields.begin();
//	
//	for (; it != m_vecFields.end(); ++it, ++nFieldOffset)
//	{
//		
//		std::string strCount = "field_" + toString(nFieldOffset);
//
//		std::ostringstream ost;
//		ost << *it; 
//		mapAttr[strCount] = ost.str();
//
//		strCount = "context_" + toString(nFieldOffset);
//		ost.str("");
//		ost << m_vecContext[nFieldOffset];
//		mapAttr[strCount] = ost.str();
//	}
//
//	return mapAttr;
//}

std::map<std::string, std::string> ConstCoverage::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr["resolution"] = StringUtils::toString(m_nResolution);
		
	int nFieldOffset = 0;
	std::vector<PYXValue>::const_iterator it = m_vecFields.begin();
	
	// NOTE: we only support one channel of data right now.
	if(it != m_vecFields.end())
	{	
		std::ostringstream ost;
		ost << *it; 
		mapAttr["value"] = ost.str();

		ost.str("");
		ost << m_vecContext[nFieldOffset];
		mapAttr["context"] = ost.str();
	}

	PYXValue value(m_bIsRandom);
	std::ostringstream ost;
	ost << value; 
	mapAttr["IsRandom"] = ost.str();

	ost.str("");
	ost << m_nSecondsBetweenChange;
	mapAttr["RandomSeconds"] = ost.str();

	return mapAttr;
}

std::string STDMETHODCALLTYPE ConstCoverage::getAttributeSchema() const
{
	std::string strXSD = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"ConstCoverage\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"resolution\" type=\"xs:int\" default=\"" + 
			  StringUtils::toString(m_nResolution) + "\" >"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Resolution</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>";
	
	int nFieldOffset = 0;
	std::vector<PYXValue>::const_iterator it = m_vecFields.begin();

	for (; it != m_vecFields.end(); ++it, ++nFieldOffset)
	{		
		std::ostringstream ost;
		ost << *it;

		strXSD += "<xs:element name=\"value\" type=\"xs:string\" default=\"" 
			+ ost.str() + "\" >";		
		strXSD += "<xs:annotation>"
		  "<xs:appinfo>"
			"<friendlyName>Value</friendlyName>"
			"<description></description>"
		  "</xs:appinfo>"
		"</xs:annotation>"
	  "</xs:element>";

		ost.str("");
		ost << m_vecContext[nFieldOffset];
		
		strXSD += "<xs:element name=\"context\" type=\"xs:string\" default=\"" 
			+ ost.str() + "\" >";
		strXSD += "<xs:annotation>"
		  "<xs:appinfo>"
			"<friendlyName>Context</friendlyName>"
			"<description></description>"
		  "</xs:appinfo>"
		"</xs:annotation>"
	  "</xs:element>";
	}

	strXSD += "<xs:element name=\"IsRandom\" type=\"xs:boolean\">"
	"<xs:annotation>"
	  "<xs:appinfo>"
		"<friendlyName>Change Randomly?</friendlyName>"
		"<description></description>"
	  "</xs:appinfo>"
	"</xs:annotation>"
	"</xs:element>";

	strXSD += "<xs:element name=\"RandomSeconds\" type=\"xs:int\">"
	"<xs:annotation>"
	  "<xs:appinfo>"
	    "<friendlyName>Seconds between change</friendlyName>"
		"<description></description>"
	  "</xs:appinfo>"
	"</xs:annotation>"
	"</xs:element>";

	strXSD += "</xs:sequence></xs:complexType></xs:element></xs:schema>";

	return strXSD;
}

void ConstCoverage::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string, std::string>::const_iterator it;

	it = mapAttr.find("resolution");
	if (it != mapAttr.end())
	{
		setGeometryResolution(atoi(it->second.c_str()));
	}

	PYXValue value;

	// extract the value
	bool isValueSet = false;
	it = mapAttr.find("value");
	if (it != mapAttr.end())
	{
		isValueSet = true;
		StringUtils::fromString(it->second, &value);

	}

	// extract the context
	it = mapAttr.find("context");
	PYXFieldDefinition::eContextType nContext;
	if (it != mapAttr.end())
	{
		if (!isValueSet)
		{
			PYXTHROW(	ConstCoverageException, 
				"Attempt to set context without setting value.");
		}
		nContext = static_cast<PYXFieldDefinition::eContextType>(atoi(it->second.c_str()));
	}
	else
	{
		if (isValueSet)
		{
			PYXTHROW(	ConstCoverageException, 
				"Attempt to set value without setting context.");
		}
		else
		{ // no value is set therefore the context is set to none
			nContext = PYXFieldDefinition::eContextType::knContextNone;
		}
	}

	// set the extracted value
	setReturnValue(value, nContext, 0);

	it = mapAttr.find("IsRandom");
	if (it != mapAttr.end())
	{
		PYXValue value;
		StringUtils::fromString(it->second, &value);
		m_bIsRandom = value.getBool();
	}

	it = mapAttr.find("RandomSeconds");
	if (it != mapAttr.end())
	{
		m_nSecondsBetweenChange = atoi(it->second.c_str());
	}
}

IProcess::eInitStatus ConstCoverage::initImpl()
{
	// initialize the feature definition.
	m_strID = "Constant Coverage: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	// create the background thread to change the colour.
	if (m_bIsRandom)
	{
		m_bNeedToStop = false;
		if (m_bIsStopped)
		{
			m_bIsStopped = false;
			m_threads.create_thread(boost::bind(&colourChangeThread, this));
		}
	}
	else
	{
		m_bNeedToStop = true;
		m_threads.join_all();
	}

	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

/*!
Create a new definition to describe the current state of the constant
coverage values.
*/
void ConstCoverage::buildCoverageDefinition()
{
	PYXPointer<PYXTableDefinition> spNewDefn = PYXTableDefinition::create();

	// cycle through each of the elements in the array	
	int nFieldOffset = 0;
	std::vector<PYXValue>::const_iterator it = m_vecFields.begin();
	for (; it != m_vecFields.end(); ++it, ++nFieldOffset)
	{
		PYXValue value = *it;
		std::string strName = StringUtils::toString(nFieldOffset);
		PYXValue::eType nType = value.getType();
		int nCount = 1;
		if (nType == PYXValue::knArray)
		{
			nType = value.getArrayType();
			nCount = value.getArraySize();
		}
		
		spNewDefn->addFieldDefinition(
			strName, m_vecContext[nFieldOffset], nType, nCount);		
	}

	// swap in the new definition.
	m_spCovDefn = spNewDefn;
}

/*!
Set the number of fields to use in the coverage. The new value can be
larger or smaller than the current value but must be greater than 0. Changes 
to the field count will either be appended to or removed from the end of the 
list.

\param nFieldCount	The total number of fields that are desired for the coverage.
*/
void ConstCoverage::setFieldCount(int nFieldCount)
{
	if (nFieldCount <= 0)
	{
		PYXTHROW(	ConstCoverageException, 
					"Can't set the number of fields to '" << 
					nFieldCount << "'.");
	}
	
	// add more fields if necessary
	while (static_cast<signed int>(m_vecFields.size()) < nFieldCount)
	{
		m_vecFields.push_back(PYXValue());
		m_vecContext.push_back(PYXFieldDefinition::knContextNone);
	}

	// remove fields if necessary
	while (nFieldCount < static_cast<signed int>(m_vecFields.size()))
	{
		m_vecFields.pop_back();
		m_vecContext.pop_back();
	}

	buildCoverageDefinition();
}

/*!
Set the value that will be returned by the coverage for a particular field. If the 
field does not already exist in the coverage an exception is thrown.

\param value	Constant value to be returned. 
\param nContext	The type of data being displayed.
\param nField	The channel to set the return value on.
*/
void ConstCoverage::setReturnValue(	const PYXValue& value, 
									PYXFieldDefinition::eContextType nContext, 
									int nField) 
{
	if (nField < 0 || static_cast<signed int>(m_vecFields.size()) <= nField)
	{
		PYXTHROW(	ConstCoverageException, 
					"Can't set the return value for channel '" << 
					nField << "', the channel does not exist in the coverage.");
	}
	m_vecFields[nField] = value;
	m_vecContext[nField] = nContext;
	buildCoverageDefinition();

	onDataChanged( this->getGeometry());
}

//! The equality operator.
bool operator ==(const ConstCoverage& lhs, const ConstCoverage& rhs)
{
	if (	lhs.getProcVersion() == rhs.getProcVersion() &&
			lhs.m_nResolution == rhs.m_nResolution &&
			lhs.m_vecFields == rhs.m_vecFields &&
			lhs.m_vecContext == rhs.m_vecContext )
	{
		return true;
	}
	return false;
}

//! The inequality operator.
bool operator !=(const ConstCoverage& lhs, const ConstCoverage& rhs)
{
	return !(lhs == rhs);
}


PYXPointer<PYXValueTile> STDMETHODCALLTYPE ConstCoverage::getFieldTile(	const PYXIcosIndex& index,
																		int nRes,
																		int nFieldIndex) const
{
	assert(nFieldIndex < static_cast<signed int>(m_vecFields.size()));

	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	// create a new tile to use that is the proper depth
	PYXPointer<PYXTableDefinition> spCovDefn = PYXTableDefinition::create();
	spCovDefn->addFieldDefinition(getCoverageDefinition()->getFieldDefinition(nFieldIndex));
	PYXPointer<PYXValueTile> spValueTile = PYXValueTile::create(index, nRes, spCovDefn);

	PYXValue returnValue = m_vecFields[nFieldIndex];

	int nCellCount = spValueTile->getNumberOfCells();
	for (int nIndexOffset = 0; nIndexOffset < nCellCount; ++nIndexOffset)
	{
		spValueTile->setValue(
			nIndexOffset, 
			0,
			returnValue);
	}

	return spValueTile;
}

