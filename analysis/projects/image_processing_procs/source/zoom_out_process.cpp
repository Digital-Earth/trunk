/******************************************************************************
zoom_out_process.cpp

begin		: 2006-04-07
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "zoom_out_process.h" 

// pyxis data sources includes
#include "null_coverage.h"

//pyxlib includes
#include "pyxis/derm/index.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/procs/const_coverage.h"
#include "pyxis/utility/tester.h"

// {41F333BF-2C1C-41be-BD72-090AB583AAD6}
PYXCOM_DEFINE_CLSID(PYXZoomOutProcess,
0x41f333bf, 0x2c1c, 0x41be, 0xbd, 0x72, 0x9, 0xa, 0xb5, 0x83, 0xaa, 0xd6);

PYXCOM_CLASS_INTERFACES(PYXZoomOutProcess, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(PYXZoomOutProcess, "Zoom out Process", "Aggregates cells from resolution n to n - 1", "Drop", //"Image Processing",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Input Coverage", "The coverage to zoom out ot a lower resolution")
IPROCESS_SPEC_END

//! The name of the class
const std::string PYXZoomOutProcess::kstrScope = "PYXZoomOutProcess";
const std::string PYXZoomOutProcess::kstrAverage = "Average";
const std::string PYXZoomOutProcess::kstrIncNulls = "Include_Nulls";
const std::string PYXZoomOutProcess::kstrNo = "no";
const std::string PYXZoomOutProcess::kstrYes = "yes";

//! Tester class 
Tester<PYXZoomOutProcess> gTester;

//! Test method 
void PYXZoomOutProcess::test()
{
	PYXIcosIndex rootIndex = "A-000000000";
	PYXIcosIndex index1 = "A-01000";
	PYXIcosIndex index2 = "5-000000";
	PYXIcosIndex index3 = "C-020504";
	PYXIcosIndex index4 = "C-02050204";

	uint8_t arrayValue[3] = {120, 120, 120};
	boost::intrusive_ptr<ConstCoverage> spInputCoverage(new ConstCoverage);

	{ // Test normal behaviour.
		boost::intrusive_ptr<ConstCoverage> spInputCoverage(new ConstCoverage); 
		spInputCoverage->setReturnValue(PYXValue(arrayValue, 3), PYXFieldDefinition::knContextRGB);
		boost::intrusive_ptr<PYXZoomOutProcess> spZoomOutProcess (new PYXZoomOutProcess);
		
		PYXPointer<ParameterSpec> spParamSpec = 
			ParameterSpec::create(ICoverage::iid, 1, 1, "input coverage", "");
		PYXPointer<Parameter> spParam = Parameter::create(spParamSpec);
		spParam->addValue(spInputCoverage);
		std::vector<PYXPointer<Parameter> > vecParams;
		vecParams.push_back(spParam);

		spZoomOutProcess->setParameters(vecParams);
		std::map<std::string, std::string> attribs;
		attribs[kstrAverage] = kstrNo;
		attribs[kstrIncNulls] = kstrNo;
		attribs.clear();
		spZoomOutProcess->setAttributes(attribs);
		spZoomOutProcess->initProc();

		PYXValue testValue(&arrayValue[0], 3);
		TEST_ASSERT(testValue == spZoomOutProcess->getCoverageValue(rootIndex));
		TEST_ASSERT(testValue == spZoomOutProcess->getCoverageValue(index1));
		TEST_ASSERT(testValue == spZoomOutProcess->getCoverageValue(index2));
		TEST_ASSERT(testValue == spZoomOutProcess->getCoverageValue(index3));
		TEST_ASSERT(testValue == spZoomOutProcess->getCoverageValue(index4));
	}

	{ // When input returns Nulls

		PYXValue testValue;
		boost::intrusive_ptr<NullCoverage> spNullCov(new NullCoverage);
		boost::intrusive_ptr<PYXZoomOutProcess> spZoomOutProcess (new PYXZoomOutProcess);

		PYXPointer<ParameterSpec> spSpec = ParameterSpec::create(ICoverage::iid, 1, 1, "","");
		PYXPointer<Parameter> spParam = Parameter::create(spSpec);
		spParam->addValue(spNullCov);
		std::vector<PYXPointer<Parameter> > vecParam;
		vecParam.push_back(spParam);
		spZoomOutProcess->setParameters(vecParam);
		std::map<std::string, std::string> attribs;
		attribs.clear();
		attribs[kstrAverage] = kstrNo;
		attribs[kstrIncNulls] = kstrNo;
		spZoomOutProcess->setAttributes(attribs);
		spZoomOutProcess->initProc(true);

		TEST_ASSERT(testValue == spZoomOutProcess->getCoverageValue(rootIndex));
		TEST_ASSERT(testValue == spZoomOutProcess->getCoverageValue(index1));
		TEST_ASSERT(testValue == spZoomOutProcess->getCoverageValue(index2));
		TEST_ASSERT(testValue == spZoomOutProcess->getCoverageValue(index3));
		TEST_ASSERT(testValue == spZoomOutProcess->getCoverageValue(index4));
		
	}
	
	{ // Test that exceptions occur.
		PYXValue testVal(&arrayValue[0], 3);
		
		boost::intrusive_ptr<PYXZoomOutProcess> spZoomProc(new PYXZoomOutProcess);
		boost::intrusive_ptr<ConstCoverage> spConstCov(new ConstCoverage);

		//Here we test operation where the first channel is numeric but the next two are 
		//string and bolean. We expect an exception when we attempt to operatate on the string
		//and boolean channels but not the numeric one.
		std::string vals[3] = {"PYXIS", "INNOVATION", "inc"};
		spConstCov->setFieldCount(3);
		spConstCov->setReturnValue(PYXValue(&arrayValue[0], 3), PYXFieldDefinition::knContextRGB, 0);
		spConstCov->setReturnValue(PYXValue(&vals[0], 3), PYXFieldDefinition::knContextNone, 1);
		spConstCov->setReturnValue(PYXValue(true), PYXFieldDefinition::knContextNone, 2);
		PYXPointer<ParameterSpec> spSpec = ParameterSpec::create(ICoverage::iid, 1, 1, "", "");
		PYXPointer<Parameter> spParam = Parameter::create(spSpec);
		spParam->addValue(spConstCov);
		std::vector<PYXPointer<Parameter> > vecParam;
		vecParam.push_back(spParam);
		spZoomProc->setParameters(vecParam);
		spZoomProc->initProc();

		TEST_ASSERT(spZoomProc->getCoverageValue(index1, 0) == testVal);
		
		//Exception non-numeric channel type
		TEST_ASSERT_EXCEPTION(spZoomProc->getCoverageValue(index2, 1), PYXException);

		//Exception non-numeric channel type
		TEST_ASSERT_EXCEPTION(spZoomProc->getCoverageValue(index2, 2), PYXException);
	
		spConstCov = boost::intrusive_ptr<ConstCoverage>(new ConstCoverage);
		spConstCov->setReturnValue(PYXValue(&arrayValue[0], 3), PYXFieldDefinition::knContextRGB, 0);
		spSpec = ParameterSpec::create(ICoverage::iid, 1, 1, "", "");
		spParam = Parameter::create(spSpec);
		spParam->addValue(spConstCov);
		vecParam.clear();
		vecParam.push_back(spParam);
		spZoomProc->setParameters(vecParam);
		std::map<std::string, std::string> attribs;
		attribs[kstrAverage] = kstrNo;
		attribs[kstrIncNulls] = kstrNo;
		spZoomProc->setAttributes(attribs);
		spZoomProc->initProc();
		PYXIcosIndex aNullIndex;

		// Exception Null Index
		TEST_ASSERT_EXCEPTION(spZoomProc->getCoverageValue(aNullIndex), PYXException); 

		// Exception Vector subsricpt out of range.
		TEST_ASSERT_EXCEPTION(spZoomProc->getCoverageValue(rootIndex, 10), PYXException); 

		attribs.clear();
	
		attribs[kstrAverage] = "Hexagons are better";
		attribs[kstrIncNulls] = "PYXIS Innovation";
		spZoomProc->setAttributes(attribs);

		std::map<std::string, std::string> mapAttr = spZoomProc->getAttributes();

		TEST_ASSERT(mapAttr[kstrAverage] == std::string(kstrNo));
		TEST_ASSERT(mapAttr[kstrIncNulls] == std::string(kstrNo));
	}
}

/*! 
Gets the attributes to that need to be serialized or displayed in the pipe editor
with this process. The attributes are entered into a map of string(key) 
to string(values) and the map is retuned to be written out.

\return A STL map containing the attributes to be saved, or returned to the pipe editor.
*/
std::map<std::string, std::string> PYXZoomOutProcess::getAttributes() const
{
	std::map<std::string, std::string> attrib;
	attrib.clear();

	attrib[kstrAverage] = m_bAverage ? kstrYes : kstrNo;
	attrib[kstrIncNulls] = m_bIncludeNulls ? kstrYes : kstrNo;

	return attrib;
}

/*!
Sets the attributes either when the process is deserializing or when
the attributes have been altered in the pipe editor. The stl map 
passed in is searched for key-value pairs and the values are parsed
out into their respective variables. 

\param mapAttr A map of the attributes to be set in this process.
*/
void PYXZoomOutProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string, std::string>::const_iterator it = mapAttr.find(kstrAverage);
	if (it != mapAttr.end())
	{
		m_bAverage = (_stricmp(it->second.c_str(), kstrYes.c_str()) == 0);
	}

	it = mapAttr.find(kstrIncNulls);
	if (it != mapAttr.end())
	{
		m_bIncludeNulls = (_stricmp(it->second.c_str(), kstrNo.c_str()) == 0);
	}
}

/*!
Creates a geometry by gathering the cell resolution from the input geometry.
Copying the input geometry to our own geometry and then decreasing the cell resolution
from resolutuion n to n - 1. It is necessary to decrease the resolution of the geometry
because we zoom out by resampling to a lower resolution.
*/
void PYXZoomOutProcess::createGeometry() const
{
	if (!m_spInputCoverage)
	{
		PYXTHROW(PYXException, "Cannot create geometry. Missing input coverage value.");
	}
	int nRes = m_spInputCoverage->getGeometry()->getCellResolution();
	m_spGeom = m_spInputCoverage->getGeometry()->clone();
	m_spGeom->setCellResolution(nRes - 1);
}


/*!
Get the coverage value at the specified PYXIS index. For hexagons only
If the index is a pentagon, then the value of the pentagon is returned.

\param	index		The PYXIS index
\param	nFieldIndex	The field index.

\return	The value.
*/
PYXValue PYXZoomOutProcess::getCoverageValue(const PYXIcosIndex& index, int nFieldIndex) const
{	
	if (!m_spInputCoverage)
	{
		PYXTHROW(PYXException, "Cannot zoom out, missing input coverage value.");
	}

	int nFieldCount = m_spInputCoverage->getCoverageDefinition()->getFieldCount();
	if (nFieldIndex > nFieldCount)
	{
		PYXTHROW(PYXException, "Cannot request a field that doesn't exist. Field: " << 
			nFieldIndex << " Doesn't exist.");
		return PYXValue();
	}
	const PYXFieldDefinition& fieldDef =
		m_spInputCoverage->getCoverageDefinition()->getFieldDefinition(nFieldIndex);

	if (!fieldDef.isNumeric())
	{
		PYXTHROW(PYXException, "Zoom Out process only works on numeric types.");
	}

	// create a vector with one accumulator per component
	int nFieldComponents = fieldDef.getCount();
	std::vector<double> vecAccumulator;
	vecAccumulator.resize(nFieldComponents);

	// initialize accumulator to centroid value
	PYXIcosIndex centroidIndex = index;

	// We are changing the resolution in two steps here
	// because it is different to set a res 14 index to
	// res 12 than it is to set a res 14 index to res 11
	// and then increment the resolution.  This method
	// ensures that we end up with a centroid hex.
	centroidIndex.setResolution(m_spGeom->getCellResolution());
	centroidIndex.incrementResolution();
	double fTotalWeight = 0.0;
	PYXValue val = m_spInputCoverage->getCoverageValue(centroidIndex, nFieldIndex);
	TRACE_DEBUG("ZoomOut Input Value: " << val);
	if (!val.isNull() || m_bIncludeNulls)
	{
		for (int nComponent = 0; nComponent < nFieldComponents; nComponent++)
		{
			vecAccumulator[nComponent] = val.getDouble(nComponent);
		}
		fTotalWeight += 1.0;
	}

	// iterate over vertices, accumulating with weight 1/3
	const double kfOneThird = 1.0/3.0;
	for (PYXVertexIterator itVertex(index); !itVertex.end(); itVertex.next())
	{
		val = m_spInputCoverage->getCoverageValue(itVertex.getIndex(), nFieldIndex);
		if (!val.isNull() || m_bIncludeNulls)
		{
			for (int nComponent = 0; nComponent < nFieldComponents; nComponent++)
			{
				vecAccumulator[nComponent] += kfOneThird * val.getDouble(nComponent);
			}
			fTotalWeight += kfOneThird;
		}
	}

	if (m_bAverage)
	{
		if (fTotalWeight != 0.0)
		{
			// divide by total weight to get average
			double fRecipWeight = 1.0/fTotalWeight;
			for (int nComponent = 0; nComponent < nFieldComponents; nComponent++)
			{
				vecAccumulator[nComponent] *= fRecipWeight;
			}
		}
	}

	// return the result in the same format as the original
	for (int nComponent = 0; nComponent < nFieldComponents; nComponent++)
	{
		val.setDouble(nComponent, vecAccumulator[nComponent]);
	}
	TRACE_DEBUG("Zoom out Returning: " << val);
	return val;
}
