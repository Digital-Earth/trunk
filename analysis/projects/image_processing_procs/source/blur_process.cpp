/******************************************************************************
blur_process.cpp

begin		: 2006-08-22
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_IMAGE_PROCESSING_PROCS_SOURCE
#include "blur_process.h"

// local includes
#include "exceptions.h"
#include "zoom_in_process.h"
#include "zoom_out_process.h"

// pyxis data sources includes
#include "coverage_cache.h"
#include "null_coverage.h"

// pyxlib includes
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/procs/const_coverage.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value_math.h"

// {32C48166-6FDB-46fc-9FAB-705E7469F9A4}
PYXCOM_DEFINE_CLSID(BlurProcess, 
0x32c48166, 0x6fdb, 0x46fc, 0x9f, 0xab, 0x70, 0x5e, 0x74, 0x69, 0xf9, 0xa4);

PYXCOM_CLASS_INTERFACES(
	BlurProcess, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid );

IPROCESS_SPEC_BEGIN(BlurProcess, "Blur Process", "Applies a blurring algorithm to input.", "Drop", //"Image Processing",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid )
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Input Coverage", "The coverage to blur.")
IPROCESS_SPEC_END

//! The unit test class
Tester<BlurProcess> gTester;

// Constants
const std::string BlurProcess::kstrSteps = "Number_of_Steps";

/*!
The unit test method for the class.
*/
void BlurProcess::test()
{
	//TODO[shatzi,nov 2012]: I remove those test because they take tool long.
	return;

	PYXIcosIndex rootIndex = "A-000000000";
	PYXIcosIndex index1 = "A-01000";
	PYXIcosIndex index2 = "5-000000";
	PYXIcosIndex index3 = "C-020504";
	PYXIcosIndex index4 = "C-02050204";
	uint8_t arrayValue[3] = {120, 120, 120};
	uint8_t arrayValue1[3] = {180, 180, 180};
	uint8_t arrayValue2[3] = {210, 210, 210};
	uint8_t arrayValue3[3] = {240, 240, 240};
	int nSteps = 3;
	int nRootIndexRes = 10; 

	{ // Test normal behaviour. Single-Channel Input.
		boost::intrusive_ptr<ConstCoverage> spInputCoverage(new ConstCoverage); 
		spInputCoverage->setFieldCount(1);
		spInputCoverage->setReturnValue(PYXValue(arrayValue,3),PYXFieldDefinition::knContextRGB,0);
		boost::intrusive_ptr<ICoverage> spBlurProcess;
		PYXCOMCreateInstance( BlurProcess::clsid,
			0, ICoverage::iid, (void**) &spBlurProcess);
		
		boost::intrusive_ptr<IProcess> spInputAsProcess;
		boost::intrusive_ptr<IProcess> spBlurAsProcess;
		spInputCoverage->QueryInterface(IProcess::iid, (void**) &spInputAsProcess);
		spBlurProcess->QueryInterface(IProcess::iid, (void**) &spBlurAsProcess);

		spBlurAsProcess->getParameter(0)->addValue(spInputAsProcess);
		std::map<std::string, std::string> attribs;
		attribs.clear();
		attribs[kstrSteps] = intToString(nSteps,0);
		spBlurAsProcess->setAttributes(attribs);
		TEST_ASSERT(spBlurAsProcess->initProc(true) == IProcess::knInitialized);

		TEST_ASSERT(spBlurProcess->getCoverageDefinition()->getFieldCount() == (nSteps +1));
		
		PYXValue testValue(arrayValue, 3);

		for (int nChannel = 0; nChannel < nSteps; ++nChannel)
		{
			TEST_ASSERT(testValue == spBlurProcess->getCoverageValue(rootIndex, nChannel));
			TEST_ASSERT(testValue == spBlurProcess->getCoverageValue(index1, nChannel));
			TEST_ASSERT(testValue == spBlurProcess->getCoverageValue(index2, nChannel));
			TEST_ASSERT(testValue == spBlurProcess->getCoverageValue(index3, nChannel));
			TEST_ASSERT(testValue == spBlurProcess->getCoverageValue(index4, nChannel));
		}
	}

	{ // When input returns Nulls
		nSteps = 2;
		PYXValue testValue;
		boost::intrusive_ptr<NullCoverage> spNullCov(new NullCoverage);
		boost::intrusive_ptr<BlurProcess> spBlurProc (new BlurProcess);

		PYXPointer<ParameterSpec> spSpec = ParameterSpec::create(ICoverage::iid, 1, 1, "","");
		PYXPointer<Parameter> spParam = Parameter::create(spSpec);
		spParam->addValue(spNullCov);
		std::vector<PYXPointer<Parameter> > vecParam;
		vecParam.push_back(spParam);
		spBlurProc->setParameters(vecParam);
		std::map<std::string, std::string> attrib;
		attrib.clear();
		attrib[kstrSteps] = "2";
		spBlurProc->setAttributes(attrib);
		TEST_ASSERT(spBlurProc->initProc(true) == IProcess::knInitialized);
		for (int nChannel = 0; nChannel < nSteps; ++nChannel)
		{
			TEST_ASSERT(testValue == spBlurProc->getCoverageValue(rootIndex, nChannel));
			TEST_ASSERT(testValue == spBlurProc->getCoverageValue(index1, nChannel));
			TEST_ASSERT(testValue == spBlurProc->getCoverageValue(index2, nChannel));
			TEST_ASSERT(testValue == spBlurProc->getCoverageValue(index3, nChannel));
			TEST_ASSERT(testValue == spBlurProc->getCoverageValue(index4, nChannel));
		}
	}
	
	{ 
		boost::intrusive_ptr<BlurProcess> spBlurProc (new BlurProcess);
		boost::intrusive_ptr<ConstCoverage> spConstCov(new ConstCoverage);

		//String coverages cannot be averaged therefore we die on initalization.
		std::string vals[3] = {"PYXIS","INNOVATION","inc"};
		spConstCov->setReturnValue(PYXValue(vals, 3), PYXFieldDefinition::knContextNone, 0);
		PYXPointer<ParameterSpec> spSpec = ParameterSpec::create(ICoverage::iid, 1,1, "","");
		PYXPointer<Parameter> spParam = Parameter::create(spSpec);
		spParam->addValue(spConstCov);
		std::vector<PYXPointer<Parameter> > vecParam;
		vecParam.push_back(spParam);
		spBlurProc->setParameters(vecParam);
		std::map<std::string, std::string> attrib;
		attrib.clear();
		attrib[kstrSteps] = "2";
		spBlurProc->setAttributes(attrib);

		//Exception String input.
		TEST_ASSERT(spBlurProc->initProc() == IProcess::knFailedToInit);
		spConstCov = boost::intrusive_ptr<ConstCoverage>(new ConstCoverage);
		spConstCov->setReturnValue(PYXValue(&arrayValue[0], 3), PYXFieldDefinition::knContextRGB, 0);
		spSpec = ParameterSpec::create(ICoverage::iid, 1,1, "","");
		spParam = Parameter::create(spSpec);
		spParam->addValue(spConstCov);
		vecParam.clear();
		vecParam.push_back(spParam);
		spBlurProc->setParameters(vecParam);
		std::map<std::string, std::string> attr;
		attr.clear();
		attr[kstrSteps] = "2";
		spBlurProc->setAttributes(attr);
		TEST_ASSERT(spBlurProc->initProc(true) == IProcess::knInitialized);
		PYXIcosIndex aNullIndex;

		// Boolean coverages can't be averaged so we die upon initialization.
		spConstCov = boost::intrusive_ptr<ConstCoverage>(new ConstCoverage);
		spConstCov->setReturnValue(PYXValue(true), PYXFieldDefinition::knContextNone, 0);
		spSpec = ParameterSpec::create(ICoverage::iid, 1,1, "","");
		spParam = Parameter::create(spSpec);
		spParam->addValue(spConstCov);
		vecParam.clear();
		vecParam.push_back(spParam);
		spBlurProc->setParameters(vecParam);
		TEST_ASSERT(spBlurProc->initProc() == IProcess::knFailedToInit); 
	}
}

std::map<std::string, std::string> BlurProcess::getAttributes() const
{
	std::map<std::string, std::string> attrib;
	attrib[kstrSteps] = StringUtils::toString(m_nSteps);
	return attrib;
}

void BlurProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::map<std::string, std::string>::const_iterator it = mapAttr.find(kstrSteps);
	if (it != mapAttr.end())
	{
		StringUtils::fromString(it->second, &m_nSteps);
	}
}

/*!
Utility helper method to set up the blurring pipeline which consists of a series of
zoom in, zoom out, and cache processes all connected together. A single step
consists of a zoom out, zoom in and a cache.
*/
void BlurProcess::setUpFilteringPipeline()
{	
	m_vecOutputCaches.clear();
	if (m_spInputCoverage)
	{
		// resize the cache vector
		m_vecOutputCaches.resize(m_nSteps + 1, boost::intrusive_ptr<ICoverage>());

		// examine the input coverage and store the unblurred version in channel 0
		if (boost::dynamic_pointer_cast<ICache>(m_spInputCoverage))
		{
			m_vecOutputCaches[0] = m_spInputCoverage;
		}
		else
		{
			// the input is not cached, add one to it
			boost::intrusive_ptr<PYXCoverageCache> spInputCache(new PYXCoverageCache);
			spInputCache->getParameter(0)->addValue(boost::dynamic_pointer_cast<IProcess>(m_spInputCoverage));
			spInputCache->initProc();
			spInputCache->setCachePersistence(true);
			//spInputCache->setCoverageDefinition(m_spInputCoverage->getCoverageDefinition());
			m_vecOutputCaches[0] = spInputCache;
		}
		
		// for each step plug the previous input into a new blur level
		for (size_t nLevel = 0; nLevel < static_cast<size_t>(m_nSteps); ++nLevel)
		{
			// First level is a zoom out
			boost::intrusive_ptr<PYXZoomOutProcess> spZoomOut (new PYXZoomOutProcess());
			std::map<std::string, std::string> zoomOutAttrib;
			zoomOutAttrib.clear();
			zoomOutAttrib[PYXZoomOutProcess::kstrAverage] = PYXZoomOutProcess::kstrYes;
			zoomOutAttrib[PYXZoomOutProcess::kstrIncNulls] = PYXZoomOutProcess::kstrNo;
			spZoomOut->setAttributes(zoomOutAttrib);
			spZoomOut->getParameter(0)->addValue(
				boost::dynamic_pointer_cast<IProcess>(m_vecOutputCaches[nLevel]));
			spZoomOut->initProc();

			// Cache the zoom out
			boost::intrusive_ptr<PYXCoverageCache> spZoomOutCache(new PYXCoverageCache());
			spZoomOutCache->setCachePersistence(false);
			spZoomOutCache->getParameter(0)->addValue(
				boost::dynamic_pointer_cast<IProcess>(spZoomOut));
			spZoomOutCache->initProc();
			//spZoomOutCache->setCoverageDefinition(m_spInputCoverage->getCoverageDefinition());
			
			/*
			For every zoom out in the accumulated chain we need to zoom in so second loop creates
			the zoom in parts of this algorithm and links them together.
			*/
			boost::intrusive_ptr<ICoverage> spZoomInInput = spZoomOutCache;
			for (size_t nZoomInCount = 0; nZoomInCount <= nLevel; ++nZoomInCount)
			{
				// Create a zoom in process
				boost::intrusive_ptr<PYXZoomInProcess> spZoomIn(new PYXZoomInProcess);
				std::map<std::string, std::string> zoomInAttrib;
				zoomInAttrib.clear();
				zoomInAttrib[PYXZoomInProcess::kstrBlurringAlgor] = PYXZoomInProcess::kstrYes;
				zoomInAttrib[PYXZoomInProcess::kstrIncNulls] = PYXZoomInProcess::kstrNo;
				spZoomIn->setAttributes(zoomInAttrib);
				spZoomIn->getParameter(0)->addValue(
					boost::dynamic_pointer_cast<IProcess>(spZoomInInput));
				spZoomIn->initProc();

				// cache the zoom in process output
				boost::intrusive_ptr<PYXCoverageCache> spCache (new PYXCoverageCache);
				spCache->setCachePersistence(false);
				spCache->getParameter(0)->addValue(
					boost::dynamic_pointer_cast<IProcess>(spZoomIn));
				spCache->initProc();
				//spCache->setCoverageDefinition(m_spInputCoverage->getCoverageDefinition());

				// prime the input for the next loop
				spZoomInInput = spCache;
			}

			//Set up the output caches
			m_vecOutputCaches[nLevel + 1] = spZoomInInput;
		}
		createMetaData();
	}
	else
	{
		PYXTHROW(ImageProcessingException, "No input provided for blur process.");
	}
}

/*!
Helper method to amend the coverage definition meta data to add any of field
field definitions, to the coverage definition. Changing the meta data
then makes the data look like it is a multi channel data source. 
*/
void BlurProcess::createMetaData()
{
	m_spCovDefn = PYXTableDefinition::create();
	if (m_spCovDefn)
	{
		if (!m_spInputCoverage)
		{
			PYXTHROW(ImageProcessingException, "Cannot create meta data for this process before input coverage properly set.");
		}
		else
		{
	
			PYXFieldDefinition originalDefinition = 
					m_spInputCoverage->getCoverageDefinition()->getFieldDefinition(0);

			std::string firstChannelName("Original input: ");
			firstChannelName.append(originalDefinition.getName());
			m_spCovDefn->addFieldDefinition(firstChannelName,
				originalDefinition.getContext(),
				originalDefinition.getType(),
				originalDefinition.getCount());

			for (size_t nVecIndex = 1; nVecIndex < m_vecOutputCaches.size(); ++nVecIndex)
			{
				std::stringstream name;
				name << "Channel " << static_cast<unsigned int>(nVecIndex);
				if (nVecIndex == 1)
				{
					name << ": slightly blurred input.";
				}
				else if (nVecIndex == (m_vecOutputCaches.size() - 1))
				{
					name << ": most blurred input.";
				}
				m_spCovDefn->addFieldDefinition(
					name.str(),
					originalDefinition.getContext(),
					originalDefinition.getType(),
					originalDefinition.getCount());
			}
		}
	}
	else
	{
		assert(false && "Failed to create table definition for this process.");
	}
}

PYXValue BlurProcess::getCoverageValue(
		const PYXIcosIndex& index, int nFieldIndex) const
{
	return m_vecOutputCaches[nFieldIndex]->getCoverageValue(index);
}

PYXPointer<PYXValueTile> BlurProcess::getFieldTile(	const PYXIcosIndex& index,
													int nRes,
													int nFieldIndex	) const
{
	return m_vecOutputCaches[nFieldIndex]->getFieldTile(index, nRes, 0);
}

/*!
Utility helper method to create the geometry of this coverage. In this case
the geometry of our input is copied as our own.
*/
void BlurProcess::createGeometry() const
{
	if (!m_spInputCoverage)
	{
		PYXTHROW(ImageProcessingException, "Cannot create the geometry without proper initalization of input coverage.");
	}

	m_spGeom = m_spInputCoverage->getGeometry()->clone();
}

IProcess::eInitStatus BlurProcess::initImpl()
{
	m_strID = "Blur Process: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	m_spInputCoverage = getParameter(0)->getValue(0)->getOutput()->QueryInterface<ICoverage>();
	if (!m_spInputCoverage)
	{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Unable to acquire input coverage for the Blur process.");
			return knFailedToInit;
	}

	PYXFieldDefinition fieldDef = m_spInputCoverage->getCoverageDefinition()->getFieldDefinition(0);
	if (!fieldDef.isNumeric())
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Cannot Blur non numeric types.");
		return knFailedToInit;
	}
	
	if (m_nSteps <= 0 || m_nSteps > 40)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("The number of steps of the blur process must between 0 - 40");
		return knFailedToInit;
	}

	setUpFilteringPipeline();
	return knInitialized;
}