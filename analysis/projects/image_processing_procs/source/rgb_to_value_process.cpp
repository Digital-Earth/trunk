/******************************************************************************
rgb_to_value_process.cpp

begin		: 2017-05-02
copyright	: (C) 2017 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "rgb_to_value_process.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/procs/const_coverage.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/utility/math_utils.h"

// boost includes
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

// standard includes
#include <cassert>


// {E23B6008-7F32-4BA7-BA37-D4038B30FB6D}
PYXCOM_DEFINE_CLSID(RgbToValueProcess, 
0xe23b6008, 0x7f32, 0x4ba7, 0xba, 0x37, 0xd4, 0x3, 0x8b, 0x30, 0xfb, 0x6d);

PYXCOM_CLASS_INTERFACES(RgbToValueProcess, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(RgbToValueProcess, "RBG To Values", "transform rgb coverage into values.", "Analysis/Coverages",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Coverage", "rgb coverage to transform.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<RgbToValueProcess> gTester;

}

RgbToValueProcess::RgbToValueProcess()
{	
	m_outputType = "double";
	m_threshold = 50;
}

RgbToValueProcess::~RgbToValueProcess()
{
}

void RgbToValueProcess::test()
{
	//TODO [shatzi]: add some unit testing!
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

IProcess::eInitStatus RgbToValueProcess::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_strID = "Transform Values " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	// Get the input coverage.
	m_spCov = 0;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(
		ICoverage::iid, (void**) &m_spCov	);
	
	if (!m_spCov)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input coverage not set");
		return knFailedToInit;
	}

	// Get the input coverage definition.
	PYXPointer<PYXTableDefinition> spCovDefn = m_spCov->getCoverageDefinition();

	// Create a new field definition, in which the greyscale fields are verified and converted to rgb.
	m_spCovDefn = PYXTableDefinition::create();
	if (spCovDefn->getFieldCount() == 0)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Input coverage should always have at least one field");
		return knFailedToInit;
	}
	else
	{
		for (int nOffset = 0; nOffset < spCovDefn->getFieldCount(); ++nOffset)
		{
			const PYXFieldDefinition& field = spCovDefn->getFieldDefinition(nOffset);
			
			if (m_outputType == "byte")
			{
				m_spCovDefn->addFieldDefinition(field.getName(), PYXFieldDefinition::knContextClass, PYXValue::knUInt8, 1 );		
			}
			else if (m_outputType == "int")
			{
				m_spCovDefn->addFieldDefinition(field.getName(), PYXFieldDefinition::knContextClass, PYXValue::knInt16, 1 );
			}
			else if (m_outputType == "float")
			{
				m_spCovDefn->addFieldDefinition(field.getName(), PYXFieldDefinition::knContextClass, PYXValue::knFloat, 1 );
			} 
			else
			{
				m_spCovDefn->addFieldDefinition(field.getName(), PYXFieldDefinition::knContextClass, PYXValue::knDouble, 1 );
			}
		}
	}
	return knInitialized;
}

//! Get the attributes in this process.
std::map<std::string, std::string> STDMETHODCALLTYPE RgbToValueProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	
	mapAttr["Threshold"] = StringUtils::toString(m_threshold);

	mapAttr["Output"] = m_outputType;

	std::stringstream transformString;

	transformString << m_transform.size();
	for(auto & keyValue : m_transform)
	{
		transformString <<  ' ' << keyValue.first << ' ' << keyValue.second;
	}

	mapAttr["Transform"] = transformString.str();

	return mapAttr;
}

//! Set the attributes in this process.
void STDMETHODCALLTYPE RgbToValueProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	std::string output;
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"Output",output);
	
	if (output == "byte" || output == "int" || output == "float")
	{
		m_outputType = output;
	} 
	else 
	{
		m_outputType = "double";
	}

	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"Threshold",int,m_threshold);

	std::string transform;
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"Transform",transform);

	if (!transform.empty())
	{
		m_transform.clear();

		int size;

		std::stringstream transformString(transform);

		transformString >> size;

		for(auto i = 0;i<size;i++)
		{
			PYXValue from;
			PYXValue to;

			transformString >> from >> to;

			m_transform[from] = to;
		}
	}
}


std::string STDMETHODCALLTYPE RgbToValueProcess::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:simpleType name=\"OutputType\">"
			"<xs:restriction base=\"xs:string\">"
				"<xs:enumeration value=\"byte\" />"
				"<xs:enumeration value=\"int\" />"
				"<xs:enumeration value=\"float\" />"
				"<xs:enumeration value=\"double\" />"
			"</xs:restriction>"
		  "</xs:simpleType>"
		  "<xs:element name=\"RgbToValueProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"Output\" type=\"OutputType\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Output</friendlyName>"
					"<description>Define transform output type.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"		
			  "<xs:element name=\"Transform\" type=\"string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Transform</friendlyName>"
					"<description>Define value transform.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"		
			  "<xs:element name=\"Threshold\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Threshold</friendlyName>"
					"<description>Threshold for color matching.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue RgbToValueProcess::getCoverageValue(
	const PYXIcosIndex& index,
	int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	assert(m_spCov);
	return convert(index, m_spCov->getCoverageValue(index, nFieldIndex), nFieldIndex);
}

PYXPointer<PYXValueTile> RgbToValueProcess::getFieldTile(
	const PYXIcosIndex& index,
	int nRes,
	int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	assert(m_spCov);

	// get the tile from the input coverage
	PYXPointer<PYXValueTile> spInputValueTile = m_spCov->getFieldTile(index, nRes, nFieldIndex);

	// Return null tiles right away.
	if (!spInputValueTile)
	{
		return spInputValueTile;
	}

	// create the tile to return to the caller
	PYXPointer<PYXTableDefinition> spCovDefn = PYXTableDefinition::create();
	spCovDefn->addFieldDefinition(getCoverageDefinition()->getFieldDefinition(nFieldIndex));
	PYXPointer<PYXValueTile> spValueTile = PYXValueTile::create(index, nRes, spCovDefn);

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

		// just call it directly if you want single threaded... and comment out the create_thread below
		//calculatePartialTile(firstIndex,lastIndex,index,spInputValueTile.get(),spValueTile.get(),nFieldIndex,nRes);

		threads.create_thread(
			boost::bind (&RgbToValueProcess::calculatePartialTile,this,
				firstIndex, lastIndex, index, spInputValueTile.get(), spValueTile.get(), nFieldIndex, nRes));
				
		firstIndex += cellsPerThread;
	}

	// wait for all the threads to finish.
	threads.join_all();
	
	return spValueTile;
}

void RgbToValueProcess::calculatePartialTile (int firstIndex, int lastIndex,
							   const PYXIcosIndex & index,
							   PYXValueTile* spInputTile,
						       PYXValueTile* spOutputTile,
							   int nFieldIndex,int nRes) const
{
	// convert all of the values from the input coverage tile
	PYXValue inVal = m_spCov->getCoverageDefinition()->getFieldDefinition(nFieldIndex).getTypeCompatibleValue();
	PYXValue outVal = getCoverageDefinition()->getFieldDefinition(nFieldIndex).getTypeCompatibleValue();
	
	// TODO: we should be able to be more efficient than this -- look for
	// or add support for setting the index in the iterator to part way along.
	PYXPointer<PYXIterator> it = spOutputTile->getIterator();

	for (int n = 0; n < firstIndex; ++n)
	{
		it->next();
	}	
	
	for (int n = firstIndex; n <= lastIndex; ++n)
	{
		PYXIcosIndex curIndex(it->getIndex());
		it->next();
	
		if (spInputTile->getValue(n, 0, &inVal))
		{			
			spOutputTile->setValue(n, 0, convert(curIndex,inVal, nFieldIndex));
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

void RgbToValueProcess::createGeometry() const
{
	assert(m_spCov);
	PYXPointer<PYXGeometry> spGeometry = m_spCov->getGeometry();
	if (!spGeometry)
	{
		m_spGeom = PYXEmptyGeometry::create();
	}
	else
	{
		m_spGeom = spGeometry->clone();
	}
}

//! Convert a greyscale value to an RGB value.
PYXValue RgbToValueProcess::convert(const PYXIcosIndex & index, const PYXValue & valIn, int nFieldIndex) const
{
	int minDistance = 0;
	std::map<PYXValue,PYXValue>::const_iterator foundEntry = m_transform.cend();

	if (!valIn.isNull())
	{
		const unsigned char * valInColor = valIn.getUInt8Ptr(0);

		for(auto entry = m_transform.cbegin(); entry != m_transform.cend(); ++entry)
		{
			auto distance = colorDistance(valInColor, entry->first.getUInt8Ptr(0));

			if (foundEntry == m_transform.end() || distance < minDistance)
			{
				foundEntry = entry;
				minDistance = distance;
			}
		}

		if (foundEntry != m_transform.cend() && minDistance < m_threshold)
		{
			return foundEntry->second;
		}
		
		return PYXValue();
	}
	return valIn;
}

int RgbToValueProcess::colorDistance(const unsigned char * a,const unsigned char * b) const
{
	//TODO: consider better way to calcualte the distance between colors
	auto distance = 0;

	distance += abs(a[0]-b[0]);
	distance += abs(a[1]-b[1]);
	distance += abs(a[2]-b[2]);

	return distance;
}