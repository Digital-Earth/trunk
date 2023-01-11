/******************************************************************************
viewpoint_process.cpp

begin      : August 18, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_PYXIS_COVERAGES_SOURCE
#include "viewpoint_process.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/utility/tester.h"

#define Airborne_Imaging_Demo

// {85ECCFCB-1D9B-4DF9-807F-391D03FCB1FB}
PYXCOM_DEFINE_CLSID(ViewPointProcess, 
0x85eccfcb, 0x1d9b, 0x4df9, 0x80, 0x7f, 0x39, 0x1d, 0x3, 0xfc, 0xb1, 0xfb);
PYXCOM_CLASS_INTERFACES(ViewPointProcess, IViewPoint::iid, IProcess::iid, ICoverage::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(ViewPointProcess, "ViewPoint Process", "Specifies a combination of pipelines to display in a ViewPoint "
		"and an optional default camera location.", "Hidden",
					IViewPoint::iid, ICoverage::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 0, -1, "Input Coverage(s)", "Assorted coverages.")
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 0, -1, "Input Feature Collection(s)", 
	"Assorted feature collections.")
IPROCESS_SPEC_END

Tester<ViewPointProcess> gTester;

// TODO[kabiraman]: Add more unit tests.
void ViewPointProcess::test()
{
	// create a ViewPointProcess
	boost::intrusive_ptr<IProcess> spViewPointProcess;
	PYXCOMCreateInstance(
		strToGuid("{85ECCFCB-1D9B-4df9-807F-391D03FCB1FB}"), 
		0, 
		IProcess::iid, 
		(void**) &spViewPointProcess);
	boost::intrusive_ptr<IViewPoint> spViewPoint = 
		boost::dynamic_pointer_cast<IViewPoint>(spViewPointProcess);

	TEST_ASSERT(spViewPointProcess && "Pointer to ViewPointProcess is null!");
	TEST_ASSERT((spViewPointProcess->initProc(true) == knInitialized) && 
		"ViewPointProcess with no input pipelines failed to initialize!");

	// create a ConstCoverage
	boost::intrusive_ptr<IProcess> spConstCoverage;
	PYXCOMCreateInstance(
		strToGuid("{8517369E-B91F-46be-BC8A-82E3F414D6AA}"), 
		0, 
		IProcess::iid, 
		(void**) &spConstCoverage);
	spViewPointProcess->getParameter(0)->addValue(spConstCoverage);	

	TEST_ASSERT(spViewPoint->getCoveragePipelines().size() == 1);
	TEST_ASSERT(spViewPoint->getFeatureCollectionPipelines().size() == 0);
	TEST_ASSERT(spViewPoint->getAllPipelines().size() == 1);

	// create an OGR File Reader
	boost::intrusive_ptr<IProcess> spOgrFileReader;
	PYXCOMCreateInstance(
		strToGuid("{C621458A-9E1D-41eb-B01E-C0569743C0B8}"), 
		0, 
		IProcess::iid, 
		(void**) &spOgrFileReader);
	spViewPointProcess->getParameter(1)->addValue(spOgrFileReader);

	TEST_ASSERT(spViewPoint->getCoveragePipelines().size() == 1);
	TEST_ASSERT(spViewPoint->getFeatureCollectionPipelines().size() == 1);
	TEST_ASSERT(spViewPoint->getAllPipelines().size() == 2);
}

ViewPointProcess::ViewPointProcess()
{
	setProcName("ViewPoint");
	setProcDescription("Specifies a combination of pipelines to display in a ViewPoint "
		"and an optional default camera location.");
}

ViewPointProcess::~ViewPointProcess()
{
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

IProcess::eInitStatus ViewPointProcess::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	if (getInitState() == IProcess::knInitialized)
	{
		return IProcess::knInitialized;
	}

	m_strID = "ViewPoint Process: " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	std::vector<boost::intrusive_ptr<IProcess> > vecInputCoveragePipelines;
	std::vector<boost::intrusive_ptr<IProcess> > vecInputFCPipelines;

	PYXPointer<Parameter> parameter = getParameter(0);
	int nNumberOfCoverages = parameter->getValueCount();
	
	for (int i = 0; i != nNumberOfCoverages; ++i)
	{
		boost::intrusive_ptr<IProcess> spProcess = parameter->getValue(i);
		vecInputCoveragePipelines.push_back(spProcess);
	}

	parameter = getParameter(1);
	int nNumberOfFeatureCollections = parameter->getValueCount();
	
	for (int i = 0; i != nNumberOfFeatureCollections; ++i)
	{
		boost::intrusive_ptr<IProcess> spProcess = parameter->getValue(i);
		vecInputFCPipelines.push_back(spProcess);
	}

	createVisualizationPipeline(vecInputCoveragePipelines, vecInputFCPipelines);

	// listen to parameter changes
	getParameter(0)->getChangeNotifier().attach(this, &ViewPointProcess::parameterChanged);
	getParameter(1)->getChangeNotifier().attach(this, &ViewPointProcess::parameterChanged);	
	
	return knInitialized;
}

std::map<std::string, std::string> STDMETHODCALLTYPE ViewPointProcess::getAttributes() const
{
	return std::map<std::string, std::string>();
}

std::string STDMETHODCALLTYPE ViewPointProcess::getAttributeSchema() const
{
	return "";
}

void STDMETHODCALLTYPE ViewPointProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

// TODO[kabiraman]: remember spCoverage below

PYXValue ViewPointProcess::getCoverageValue(
	const PYXIcosIndex& index, 
	int nFieldIndex) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	boost::intrusive_ptr<ICoverage> spCoverage;

	m_spVisualizationPipeline->getParameter(0)->getValue(nFieldIndex)->getOutput()->QueryInterface(
		ICoverage::iid, (void**) &spCoverage);
	
	return spCoverage->getCoverageValue(index, 0);
}

PYXPointer<PYXValueTile> ViewPointProcess::getFieldTile(
	const PYXIcosIndex& index, 
	int nRes, 
	int nFieldIndex) const
{
	boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

	boost::intrusive_ptr<ICoverage> spCoverage;

	m_spVisualizationPipeline->getParameter(0)->getValue(nFieldIndex)->getOutput()->QueryInterface(
		ICoverage::iid, (void**) &spCoverage);
	
	return spCoverage->getFieldTile(index, nRes, 0);
}

////////////////////////////////////////////////////////////////////////////////
// ViewPointProcess
////////////////////////////////////////////////////////////////////////////////


bool ViewPointProcess::isElevation(const boost::intrusive_ptr<IProcess> & process)
{
	boost::intrusive_ptr<ICoverage> spCoverage = process->getOutput()->QueryInterface<ICoverage>();
	assert(spCoverage);
	assert(0 < spCoverage->getCoverageDefinition()->getFieldCount());

	std::string showAsElevation = spCoverage->getStyle("Coverage/ShowAsElevation");
	if (showAsElevation == "0" || showAsElevation == "false")
		return false;
	
	PYXFieldDefinition definition = 
		spCoverage->getCoverageDefinition()->getFieldDefinition(0);
	
	return
		((definition.getContext() == PYXFieldDefinition::knContextElevation) || 
		(definition.getContext() == PYXFieldDefinition::knContextNone) ||
		(definition.getContext() == PYXFieldDefinition::knContextGreyScale));
}

void ViewPointProcess::createVisualizationPipeline(
	std::vector<boost::intrusive_ptr<IProcess> > vecInputCoveragePipelines, 
		std::vector<boost::intrusive_ptr<IProcess> > vecInputFCPipelines)
{
	boost::intrusive_ptr<IProcess> spVisualizationPipeline;

	// first of rgb blender or elevation colourizer
	boost::intrusive_ptr<IProcess> spFirstNonNull;

	boost::intrusive_ptr<IProcess> spRgbBlender;
	boost::intrusive_ptr<IProcess> spElevationColourizer;
	boost::intrusive_ptr<IProcess> spElevationBlender;

	// Build the visualization pipeline.
	{
		boost::intrusive_ptr<IProcess> spChannelCombiner;
		PYXCOMCreateInstance(
			strToGuid("{6BCC2C4D-1353-442b-A7B2-6D57ED42C12D}"), 
			0, 
			IProcess::iid, 
			(void**) &spChannelCombiner);
		assert(spChannelCombiner);
		spVisualizationPipeline = spChannelCombiner;
	}
	{
		PYXCOMCreateInstance(
			strToGuid("{79E1D5B2-F816-449e-876B-9EAF0B1CE118}"), 
			0, 
			IProcess::iid, 
			(void**) &spFirstNonNull);
		assert(spFirstNonNull);
		spVisualizationPipeline->getParameter(0)->addValue(spFirstNonNull);
	}
	{
		
		PYXCOMCreateInstance(
			strToGuid("{00B7D55E-433A-4767-9C77-B5E276762A97}"), 
			0, 
			IProcess::iid, 
			(void**) &spRgbBlender);
		assert(spRgbBlender);
		
		std::map<std::string, std::string> attributes;
		attributes["Mode"]="ResolutionDependent";
		spRgbBlender->setAttributes(attributes);

		spFirstNonNull->getParameter(0)->addValue(spRgbBlender);
	}
	{
		PYXCOMCreateInstance(
			strToGuid("{8B64253C-7DA2-4d0c-988A-1148BADFF24F}"), 
			0, 
			IProcess::iid, 
			(void**) &spElevationColourizer);
		assert(spElevationColourizer);
		spFirstNonNull->getParameter(0)->addValue(spElevationColourizer);
	}
	{
		PYXCOMCreateInstance(
			strToGuid("{00B7D55E-433A-4767-9C77-B5E276762A97}"), 
			0, 
			IProcess::iid, 
			(void**) &spElevationBlender);
		assert(spElevationBlender);

		std::map<std::string, std::string> attributes;
		attributes["Mode"]="UseHighestResolution";
		spElevationBlender->setAttributes(attributes);

		spVisualizationPipeline->getParameter(0)->addValue(spElevationBlender);
	}

	spElevationColourizer->getParameter(0)->addValue(spElevationBlender);

	if (spVisualizationPipeline->initProc(true) == IProcess::knInitialized)
	{
		boost::intrusive_ptr<IProcess> spModifiedVisualizationPipeline;
		std::map<boost::intrusive_ptr<IProcess>, boost::intrusive_ptr<IProcess> > cloneMap;

		for (std::vector<boost::intrusive_ptr<IProcess> >::iterator it = vecInputCoveragePipelines.begin(); 
			it != vecInputCoveragePipelines.end(); ++it)
		{
			boost::intrusive_ptr<IProcess> spPipeline = *it;
			boost::intrusive_ptr<ICoverage> spCoverage = spPipeline->getOutput()->QueryInterface<ICoverage>();
			assert(spCoverage);
			assert(0 < spCoverage->getCoverageDefinition()->getFieldCount());

			PYXFieldDefinition definition = 
				spCoverage->getCoverageDefinition()->getFieldDefinition(0);

			bool bElev = isElevation(spPipeline);

#ifdef Airborne_Imaging_Demo

			// get only the first channel for visualization
			if (spCoverage->getCoverageDefinition()->getFieldCount() > 1)
			{
				boost::intrusive_ptr<IProcess> spChannelSelector;
				PYXCOMCreateInstance(
					strToGuid("{16B34600-CB76-4613-B3B4-7D1B3C4FB499}"), 
					0, 
					IProcess::iid, 
					(void**) &spChannelSelector);
				assert(spChannelSelector);

				spChannelSelector->getParameter(0)->addValue(spPipeline);
				spChannelSelector->initProc();

				spPipeline = spChannelSelector;
			}

#endif

			if (!bElev) {
				//make sure we the coverage schema is valid for non elevation pipelines
				spPipeline = convertCoverageToRgbIfNeeded(spPipeline);
			}

			// Get blender
			boost::intrusive_ptr<IProcess> spBlender =
				bElev ? spElevationBlender : spRgbBlender;
			// Modify vispipe
			spModifiedVisualizationPipeline = 
				PipeUtils::modifyPipeline(spVisualizationPipeline, spBlender, cloneMap);

			// Get modified blender
			boost::intrusive_ptr<IProcess> spModifiedBlender =
				bElev ?
					spModifiedVisualizationPipeline->getParameter(0)->getValue(1) :
					spModifiedVisualizationPipeline->getParameter(0)->getValue(0)->getParameter(0)->getValue(0);

			// Add pipeline to modified blender
			spModifiedBlender->getParameter(0)->addValue(spPipeline);

			// Init modified vispipe
			if (IProcess::knInitialized == spModifiedVisualizationPipeline->initProc(true))
			{
				m_spVisualizationPipeline = spModifiedVisualizationPipeline;
			}
			else
			{
				TRACE_ERROR("Could not add pipeline '" << *spPipeline << "' to Visualization Pipeline, will not initialize!");
			}
		}

		for (std::vector<boost::intrusive_ptr<IProcess> >::iterator it = vecInputFCPipelines.begin(); 
			it != vecInputFCPipelines.end(); ++it)
		{
			// Modify vispipe
			spModifiedVisualizationPipeline = 
				PipeUtils::modifyPipeline(spVisualizationPipeline, spVisualizationPipeline, cloneMap);
			spModifiedVisualizationPipeline->getParameter(1)->addValue(*it);

			// Init modified vispipe
			if (IProcess::knInitialized == spModifiedVisualizationPipeline->initProc(true))
			{
				m_spVisualizationPipeline = spModifiedVisualizationPipeline;
			}
			else
			{
				TRACE_ERROR("Could not add pipeline '" << *it << "' to Visualization Pipeline, will not initialize!");
			}
		}
	}
}

boost::intrusive_ptr<IProcess> ViewPointProcess::convertCoverageToRgbIfNeeded(const boost::intrusive_ptr<IProcess> & spPipeline) 
{
	auto spCoverage = spPipeline->getOutput()->QueryInterface<ICoverage>();

	if (!spCoverage) 
	{
		return spPipeline;
	}
	
	auto definition = spCoverage->getCoverageDefinition();

	if (definition->getFieldCount() != 1)
	{
		return spPipeline;
	}

	auto fieldDefiniton = definition->getFieldDefinition(0);

	if (fieldDefiniton.getType() == PYXValue::knUInt8 && fieldDefiniton.getCount() == 3) 
	{
		return spPipeline;
	}

	auto palette = spCoverage->getStyle("Coverage/Palette");

	if (palette == "")
	{
		return spPipeline;
	}

	return ProcessInitHelper("{8B64253C-7DA2-4d0c-988A-1148BADFF24F}") //colorizer process
		.addInput(0,spPipeline)
		.borrowNameAndDescription(spPipeline)
		.setAttribute("palette",palette)
		.getProcess(true);
}

void ViewPointProcess::parameterChanged(PYXPointer<NotifierEvent> spEvent)
{
	// Note: Re-attaching will happen in the call to initProc() below.
	getParameter(0)->getChangeNotifier().detach(this, &ViewPointProcess::parameterChanged);
	getParameter(1)->getChangeNotifier().detach(this, &ViewPointProcess::parameterChanged);

	this->setProcVersion(this->getProcVersion() + 1);
	this->initProc(true);

	//Note[shatzi]: There is no need to import the process on every change. let the client decided if it want to do it.
	//PipeManager::import(this);

	getViewPointChangedNotifier().notify(ViewPointChangedEvent::create());
}

// TODO[kabiraman]: Do a union of coverage, fc and elevation.
void ViewPointProcess::createGeometry() const
{
	boost::intrusive_ptr<ICoverage> spCoverage;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(
		ICoverage::iid, (void**) &spCoverage);

	if (spCoverage->getGeometry())
	{
		m_spGeom = spCoverage->getGeometry()->clone();
	}
	else
	{
		m_spGeom = PYXGlobalGeometry::create(10);
	}
}

////////////////////////////////////////////////////////////////////////////////
// IViewPoint
////////////////////////////////////////////////////////////////////////////////

std::vector<boost::intrusive_ptr<IProcess> > ViewPointProcess::getCoveragePipelines()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	std::vector<boost::intrusive_ptr<IProcess> > vecCoveragePipelines;

	PYXPointer<Parameter> parameter = getParameter(0);
	int nNumberOfCoverages = parameter->getValueCount();
	
	for (int i = 0; i != nNumberOfCoverages; ++i)
	{
		boost::intrusive_ptr<IProcess> spPipeline = parameter->getValue(i);
		vecCoveragePipelines.push_back(spPipeline);
	}

	return vecCoveragePipelines;
}

std::vector<boost::intrusive_ptr<IProcess> > ViewPointProcess::getFeatureCollectionPipelines()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	std::vector<boost::intrusive_ptr<IProcess> > vecFeatureCollectionPipelines;

	PYXPointer<Parameter> parameter = getParameter(1);
	int nNumberOfFeatureCollections = parameter->getValueCount();
	
	for (int i = 0; i != nNumberOfFeatureCollections; ++i)
	{
		boost::intrusive_ptr<IProcess> spPipeline = parameter->getValue(i);
		vecFeatureCollectionPipelines.push_back(spPipeline);
	}

	return vecFeatureCollectionPipelines;
}

std::vector<boost::intrusive_ptr<IProcess> > ViewPointProcess::getElevationPipelines()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	std::vector<boost::intrusive_ptr<IProcess> > vecElevationPipelines;

	PYXPointer<Parameter> parameter = getParameter(0);
	int nNumberOfCoverages = parameter->getValueCount();
	
	for (int i = 0; i != nNumberOfCoverages; ++i)
	{
		boost::intrusive_ptr<IProcess> spPipeline = parameter->getValue(i);
		if (isElevation(spPipeline))
		{
			vecElevationPipelines.push_back(spPipeline);
		}
	}

	return vecElevationPipelines;
}

std::vector<boost::intrusive_ptr<IProcess> > ViewPointProcess::getAllPipelines()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	std::vector<boost::intrusive_ptr<IProcess> > vecAllPipelines;
	std::vector<boost::intrusive_ptr<IProcess> > vecFeatureCollectionPipelines;

	vecAllPipelines = getCoveragePipelines();
	vecFeatureCollectionPipelines = getFeatureCollectionPipelines();

	vecAllPipelines.insert(vecAllPipelines.end(), vecFeatureCollectionPipelines.begin(), 
		vecFeatureCollectionPipelines.end());

	return vecAllPipelines;
}