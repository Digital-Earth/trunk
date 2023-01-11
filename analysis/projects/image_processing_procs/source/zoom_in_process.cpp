/*****************************************************************************
zoom_in_process.cpp

begin		: 2006-04-11
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "zoom_in_process.h" 

// pyxis data sources includes
#include "null_coverage.h"

//pyblib includes 
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/procs/const_coverage.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value_math.h"

// {18215D3B-64C9-4e14-9B41-A63018219F22}
PYXCOM_DEFINE_CLSID(PYXZoomInProcess,
0x18215d3b, 0x64c9, 0x4e14, 0x9b, 0x41, 0xa6, 0x30, 0x18, 0x21, 0x9f, 0x22);

PYXCOM_CLASS_INTERFACES(PYXZoomInProcess, 
						IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(PYXZoomInProcess, "Zoom in Process", "Subdivides the grid to a higher resolution from n to n + 1 resolutions.",  "Drop", //"Image Processing",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Input Coverage", "The coverage to zoom in to a higher resolution")
IPROCESS_SPEC_END 

//! The name of the class
const std::string PYXZoomInProcess::kstrScope = "PYXZoomInProcess";
const std::string PYXZoomInProcess::kstrBlurringAlgor = "Blurring_Algorithm";
const std::string PYXZoomInProcess::kstrIncNulls = "Inlclude_Nulls";
const std::string PYXZoomInProcess::kstrYes = "yes";
const std::string PYXZoomInProcess::kstrNo = "no";

//! Tester Class
Tester<PYXZoomInProcess> gTester;

//! Test Method 
void PYXZoomInProcess::test()
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
		spInputCoverage->setReturnValue(PYXValue(arrayValue,3),PYXFieldDefinition::knContextRGB);
		boost::intrusive_ptr<PYXZoomInProcess> spZoomInProcess (new PYXZoomInProcess);
		
		PYXPointer<ParameterSpec> spParamSpec = 
			ParameterSpec::create(ICoverage::iid, 1, 1, "input coverage", "");
		PYXPointer<Parameter> spParam = Parameter::create(spParamSpec);
		spParam->addValue(spInputCoverage);
		std::vector<PYXPointer<Parameter> > vecParams;
		vecParams.push_back(spParam);

		spZoomInProcess->setParameters(vecParams);
		std::map<std::string, std::string> attribs;
		attribs[kstrBlurringAlgor] = kstrNo;
		attribs[kstrIncNulls] = kstrNo;
		attribs.clear();
		spZoomInProcess->setAttributes(attribs);
		spZoomInProcess->initProc();

		PYXValue testValue(&arrayValue[0], 3);
		TEST_ASSERT(testValue == spZoomInProcess->getCoverageValue(rootIndex));
		TEST_ASSERT(testValue == spZoomInProcess->getCoverageValue(index1));
		TEST_ASSERT(testValue == spZoomInProcess->getCoverageValue(index2));
		TEST_ASSERT(testValue == spZoomInProcess->getCoverageValue(index3));
		TEST_ASSERT(testValue == spZoomInProcess->getCoverageValue(index4));
		
	}

	{ // When input returns Nulls

		PYXValue testValue;
		boost::intrusive_ptr<NullCoverage> spNullCov(new NullCoverage);
		boost::intrusive_ptr<PYXZoomInProcess> spZoomInProcess (new PYXZoomInProcess);

		PYXPointer<ParameterSpec> spSpec = ParameterSpec::create(ICoverage::iid, 1, 1, "","");
		PYXPointer<Parameter> spParam = Parameter::create(spSpec);
		spParam->addValue(spNullCov);
		std::vector<PYXPointer<Parameter> > vecParam;
		vecParam.push_back(spParam);
		spZoomInProcess->setParameters(vecParam);
		std::map<std::string, std::string> attribs;
		attribs.clear();
		attribs[kstrBlurringAlgor] = kstrNo;
		attribs[kstrIncNulls] = kstrNo;
		spZoomInProcess->setAttributes(attribs);
		spZoomInProcess->initProc(true);

		TEST_ASSERT(testValue == spZoomInProcess->getCoverageValue(rootIndex));
		TEST_ASSERT(testValue == spZoomInProcess->getCoverageValue(index1));
		TEST_ASSERT(testValue == spZoomInProcess->getCoverageValue(index2));
		TEST_ASSERT(testValue == spZoomInProcess->getCoverageValue(index3));
		TEST_ASSERT(testValue == spZoomInProcess->getCoverageValue(index4));
		
	}
	
	{ // Test that exceptions occur.
		PYXValue testVal(&arrayValue[0], 3);
		
		boost::intrusive_ptr<PYXZoomInProcess> spZoomProc(new PYXZoomInProcess);
		boost::intrusive_ptr<ConstCoverage> spConstCov(new ConstCoverage);

		//String coverages cannot be averaged therefore we die on initalization.
		std::string vals[3] = {"PYXIS","INNOVATION","inc"};
		spConstCov->setFieldCount(3);
		spConstCov->setReturnValue(PYXValue(&arrayValue[0], 3), PYXFieldDefinition::knContextRGB, 0);
		spConstCov->setReturnValue(PYXValue(&vals[0], 3), PYXFieldDefinition::knContextNone, 1);
		spConstCov->setReturnValue(PYXValue(true), PYXFieldDefinition::knContextNone, 2);
		PYXPointer<ParameterSpec> spSpec = ParameterSpec::create(ICoverage::iid, 1,1, "","");
		PYXPointer<Parameter> spParam = Parameter::create(spSpec);
		spParam->addValue(spConstCov);
		std::vector<PYXPointer<Parameter> > vecParam;
		vecParam.push_back(spParam);
		spZoomProc->setParameters(vecParam);
		TEST_ASSERT(spZoomProc->initProc() == IProcess::knInitialized);

		TEST_ASSERT(spZoomProc->getCoverageValue(index1, 0) == testVal);
		
		//Exception non-numeric channel type
		TEST_ASSERT_EXCEPTION(spZoomProc->getCoverageValue(index2,1),PYXException);

		//Exception non-numeric channel type
		TEST_ASSERT_EXCEPTION(spZoomProc->getCoverageValue(index2,2),PYXException);
	
		spConstCov = boost::intrusive_ptr<ConstCoverage>(new ConstCoverage);
		spConstCov->setReturnValue(PYXValue(&arrayValue[0], 3), PYXFieldDefinition::knContextRGB, 0);
		spSpec = ParameterSpec::create(ICoverage::iid, 1,1, "","");
		spParam = Parameter::create(spSpec);
		spParam->addValue(spConstCov);
		vecParam.clear();
		vecParam.push_back(spParam);
		spZoomProc->setParameters(vecParam);
		std::map<std::string, std::string> attribs;
		attribs[kstrBlurringAlgor] = kstrNo;
		attribs[kstrIncNulls] = kstrNo;
		spZoomProc->setAttributes(attribs);
		spZoomProc->initProc();
		PYXIcosIndex aNullIndex;

		// Exception Null Index
		TEST_ASSERT_EXCEPTION(spZoomProc->getCoverageValue(aNullIndex), PYXException); 

		// Exception Vector subsricpt out of range.
		TEST_ASSERT_EXCEPTION(spZoomProc->getCoverageValue(rootIndex, 10), PYXException); 

		attribs.clear();
		attribs[kstrBlurringAlgor] = "Hexagons are better";
		attribs[kstrIncNulls] = "PYXIS Innovation";
		spZoomProc->setAttributes(attribs);

		std::map<std::string, std::string> mapAttr = spZoomProc->getAttributes();

		TEST_ASSERT(mapAttr[kstrBlurringAlgor] == std::string(kstrNo));
		TEST_ASSERT(mapAttr[kstrIncNulls] == std::string(kstrNo));
	}
}

/*! 
Gets the attributes to that need to be serialized or displayed in the pipe editor
with this process. The attributes are entered into a map of string(key)
to string(values) and the map is retuned to be written out.

\return A stl map containing the attributes to be saved, or returned to the pipe editor.
*/
std::map<std::string, std::string> PYXZoomInProcess::getAttributes() const
{
	std::map <std::string, std::string> attrib;
	attrib.clear();
	attrib[kstrBlurringAlgor] = m_bBlurring ? kstrYes : kstrNo;
	attrib[kstrIncNulls] = m_bIncludeNulls ?  kstrYes : kstrNo;
	return attrib;
}

/*!
Sets the attributes either when the process is deserializing or when
the attributes have been altered in the pipe editor. The stl map
passed in is searched for key-value pairs and the values are parsed
out into their respective variables. 

\param mapAttr A map of the attributes to be set in this process.
*/
void PYXZoomInProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string, std::string>::const_iterator it = mapAttr.find(kstrBlurringAlgor);
	if (it != mapAttr.end())
	{
		m_bBlurring = (_stricmp(it->second.c_str(), kstrYes.c_str()) == 0);
	}

	it = mapAttr.find(kstrIncNulls);
	if (it != mapAttr.end())
	{
		m_bIncludeNulls = (_stricmp(it->second.c_str(), kstrYes.c_str()) == 0);
	}
}

/*!
Get the coverage value at the specified PYXIS index.

\param	index		The PYXIS index
\param	nFieldIndex	The field index.

\return	The value.
*/
PYXValue PYXZoomInProcess::getCoverageValue(const PYXIcosIndex& index, int nFieldIndex) const
{
	if (!m_spInputCoverage)
	{
		PYXTHROW(PYXException, "Missing input coverage value.");
		return PYXValue();
	}

	int nFieldCount = m_spInputCoverage->getCoverageDefinition()->getFieldCount();
	if (nFieldIndex > nFieldCount)
	{
		PYXTHROW(PYXException, "Cannot request a field that doesn't exist. Field: " + intToString(nFieldIndex, 0) + " doesn't exist.");
		return PYXValue();
	}

	const PYXFieldDefinition& fieldDef =
		m_spInputCoverage->getCoverageDefinition()->getFieldDefinition(nFieldIndex);
	if (!fieldDef.isNumeric())
	{
		PYXTHROW(PYXException, "Zoom in process only works on numeric types.");
		return PYXValue();
	}

	// for this to work, we must get data starting with the correct resolution.
	PYXIcosIndex newIndex = index;
	newIndex.setResolution(m_spGeom->getCellResolution());

	PYXIcosIndex parentIndex = PYXIcosMath::getParent(newIndex);
	PYXMath::eHexClass type = PYXMath::getHexClass(parentIndex.getResolution());
	PYXMath::eHexDirection nCellDirection = PYXMath::knDirectionZero;
	PYXIcosMath::directionFromParent(newIndex, &nCellDirection);

	// create a temporary PYXValue that can hold doubles with the same number of elements
	// as our data source is returning.
	int nFieldComponents = fieldDef.getCount();
	PYXValue sumValue = PYXValue::create(PYXValue::knDouble, 0, nFieldComponents, 0);

	PYXValue parentValue = m_spInputCoverage->getCoverageValue(parentIndex, nFieldIndex);
	TRACE_DEBUG("Zoom in Input Value: " << parentValue);

	// put in the parent value
	double fTotalWeight = 0.0;
	if (!parentValue.isNull() || m_bIncludeNulls)
	{
		PYXValueMath::assignInto(&sumValue, parentValue);
		fTotalWeight += 1.0;
	}

	if (parentIndex.isPentagon() || (nCellDirection == PYXMath::knDirectionZero))
	{
		if (m_bBlurring)
		{
			if (!parentValue.isNull() || m_bIncludeNulls)
			{
				fTotalWeight = 5.0;
				// weight the centroid 5 times to a vertex's 2 times.
				PYXValueMath::multiplyBy(&sumValue, fTotalWeight);
			}

			PYXValue val;
			// iterate over vertices, accumulating
			for (PYXVertexIterator itVertex(parentIndex); !itVertex.end(); itVertex.next())
			{
				val = m_spInputCoverage->getCoverageValue(itVertex.getIndex(), nFieldIndex);
				if (!val.isNull() || m_bIncludeNulls)
				{
					PYXValueMath::addInto(&sumValue, val);
					PYXValueMath::addInto(&sumValue, val);
					fTotalWeight += 2.0;
				}
			}

			if (fTotalWeight != 0.0)
			{
				// divide by total weight
				PYXValueMath::divideBy(&sumValue, fTotalWeight);
			}

			// put the result back into the parent value so that we return data of the correct type.
			PYXValueMath::assignInto(&parentValue, sumValue);
		}

		TRACE_DEBUG("Zoom in Returning: " << parentValue);
		return parentValue;
	}

	// Sum in the vertex in the direction of the original child
	PYXValue vertexValue = 
		m_spInputCoverage->getCoverageValue(PYXIcosMath::move(parentIndex, nCellDirection), nFieldIndex);
	if (!vertexValue.isNull() || m_bIncludeNulls)
	{
		PYXValueMath::addInto(&sumValue, vertexValue);
		fTotalWeight += 1.0;
	}

	// Sum in an adjacent vertex (chosen depending on hex class)
	PYXMath::eHexDirection nCellDirection2 =
		PYXMath::rotateDirection(nCellDirection, (type == PYXMath::knClassI) ? 1 : -1);
	vertexValue =
		m_spInputCoverage->getCoverageValue(PYXIcosMath::move(parentIndex, nCellDirection2), nFieldIndex); 
	if (!vertexValue.isNull() || m_bIncludeNulls)
	{
		PYXValueMath::addInto(&sumValue, vertexValue);
		fTotalWeight += 1.0;
	}

	if (fTotalWeight != 0.0)
	{
		PYXValueMath::divideBy(&sumValue, fTotalWeight);
	}

	// put the result back into the parent value so that we return data of the correct type.
	PYXValueMath::assignInto(&parentValue, sumValue);
	TRACE_DEBUG("Zoom in Returning: " << parentValue);
	return parentValue;
}

/*!
Creates the geometry by getting the cell resolution of the geometry from
our input. Then we clone our input's geometry and increase the cell
resolution because we are zooming in by resampling from resolution n to
resolution n + 1. Therefore the our cell resolution must be one resolution
greater then the cell resolution of our input.
*/
void PYXZoomInProcess::createGeometry() const
{
	if (!m_spInputCoverage)
	{
		PYXTHROW(PYXException, "Cannot create geometry. Missing input coverage value.");
	}
	int nCellRes = m_spInputCoverage->getGeometry()->getCellResolution();
	++nCellRes;
	m_spGeom = m_spInputCoverage->getGeometry()->clone();
	m_spGeom->setCellResolution(nCellRes);
}
