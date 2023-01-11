/******************************************************************************
grib_pipe_builder.cpp

begin		: 2007-09-24
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define GRIB_SOURCE

//local includes
#include "grib_pipe_builder.h"
#include "grib_process.h"

// pyxlib includes
#include "pyxis/data/coverage.h"
#include "pyxis/procs/path.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/string_utils.h"

// {A55F5385-2EEE-45f5-9654-BA4DAB18FC96}
PYXCOM_DEFINE_CLSID(GRIBPipeBuilder, 
0xa55f5385, 0x2eee, 0x45f5, 0x96, 0x54, 0xba, 0x4d, 0xab, 0x18, 0xfc, 0x96);

PYXCOM_CLASS_INTERFACES(GRIBPipeBuilder, IPipeBuilder::iid, PYXCOM_IUnknown::iid);

//! Constants
const std::string GRIBPipeBuilder::kstrColourizerDefaultMin = "0";
const std::string GRIBPipeBuilder::kstrColourizerDefaultMax = "256";

GRIBPipeBuilder::GRIBPipeBuilder()
{
	// Use of this class is deprecated and it is retained only for backwards compatibility.
	// GDAL is now used for reading GRIB1 and GRIB2 data files.
	// m_vecSupportedExtensions.push_back("grb");
}

/*!
Build a pipeline for the given path.

\param pDataSet	Describes the data set to be opened.

\return The head of the pipeline or nullptr if the pipeline was unable to be built.
*/
PYXPointer<IProcess> GRIBPipeBuilder::buildPipeline(PYXPointer<PYXDataSet> pDataSet) const
{
	// TODO: create a proper catalog
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if (isDataSourceSupported(pDataSet->getUri(), eCheckOptions::knLenient))
	{
		boost::filesystem::path path = FileUtils::stringToPath(pDataSet->getUri());
		std::vector< boost::intrusive_ptr<IProcess> > vecPipelines;
		buildPipelineForEachChannel(path, &vecPipelines);
		if (vecPipelines.size() > 0)
		{
			return vecPipelines[0];
		}
	}

	return nullptr;
}

/*!
Build an individual pipeline for a given channel in a grib datasource. The pipeline built is,
a GribProcess(n channel)->Default Sampler(Nearest Neighbour)->GreyScaleColourizer.

\param path		The boost path to the uri to build a pipeline on a given channel for.
\param nRecord	The record number in the Grib data source to build a pipeline for.

\return An intrusive ptr to the head process in the pipeline, if the pipeline was 
 		successfully created or a null pointer if it was not.
*/
boost::intrusive_ptr<IProcess> GRIBPipeBuilder::buildAPipeline(
	const boost::filesystem::path& path, int nRecord) const
{
	boost::intrusive_ptr<GRIBProcess> spGribProc = boost::intrusive_ptr<GRIBProcess>(new GRIBProcess);

	if (spGribProc)
	{
		std::map<std::string, std::string> attrMap;
    	attrMap["image_index"] = intToString(nRecord,0);

		boost::intrusive_ptr<IProcess> spPathParam;
		PYXCOMCreateInstance(PathProcess::clsid, 0, IProcess::iid, (void**) &spPathParam);
		std::map<std::string, std::string> mapAttr;
		mapAttr["uri"] = FileUtils::pathToString(path);
		spPathParam->setAttributes(mapAttr);
		
		try
		{
			spPathParam->initProc();
		}
		catch(...)
		{
			TRACE_ERROR("Grib pipe builder failed to create valid path process.");
			return boost::intrusive_ptr<IProcess>();
		}
			
		spGribProc->getParameter(0)->addValue(spPathParam);
		spGribProc->setAttributes(attrMap);	
				
		try
		{
			spGribProc->initProc();
		}
		catch(...)
		{
			TRACE_ERROR("Grib pipe builder failed to create valid grib process.");
			return boost::intrusive_ptr<IProcess>();
		}

		boost::intrusive_ptr<IProcess> spSamplerProc = getDefaultSampler();
		if (spSamplerProc && spGribProc)
		{
			spSamplerProc->getParameter(0)->addValue(spGribProc);
			
			boost::intrusive_ptr<IProcess> spColourizer;
			PYXCOMCreateInstance(
				strToGuid(
					"{8B64253C-7DA2-4d0c-988A-1148BADFF24F}"), 0, IProcess::iid,
						(void**) &spColourizer );
			if (spColourizer)
			{
				GribRecord dataRecord = spGribProc->getDataSetReader().getRecord();
				std::map<std::string, std::string> mapAttr;
				mapAttr["min"] = intToString(static_cast<int>(dataRecord.getMinValue()), 0);
				mapAttr["max"] = intToString(static_cast<int>(dataRecord.getMaxValue()), 0);
				spColourizer->setAttributes(mapAttr);

				spColourizer->getParameter(0)->addValue(spSamplerProc);
					
				spColourizer->setProcName(FileUtils::pathToString(path.leaf()));
					
				//TODO: Get the description of the record from the GRib Process.
				spColourizer->setProcDescription(
					" Channel: " + intToString(nRecord, 0) +
						dataRecord.getDescription() );
				spColourizer->initProc(true);
				return spColourizer;
			}
		}
	}
	
	return boost::intrusive_ptr<IProcess>();
}

/*!
Builds a pipeline for each channel of a given grib datasource. Default to the zeroth channel, then 
using information provided in the data definition loop through fields one to field count building,
through delegation to the buildAPipeline method, a pipeline for each record corresponding to a 
field definition in the data definition. Upon successful building of a pipeline the pipeline is 
pushed onto the vector that is pointed to by pVec.

\param path - The boost path to build to a GRIB uri to build a pipeline for.
\param pVec - Out parameter pointing to a vector of pipelines which have been built.

\return bool - Indicating wether all channels of a pipeline have been built for a particular
			   uri or not.
*/
bool GRIBPipeBuilder::buildPipelineForEachChannel(const boost::filesystem::path &path,
												std::vector<
													boost::intrusive_ptr<IProcess> > *pVec) const
{
	boost::intrusive_ptr<IProcess> spPipeline;
	spPipeline = buildAPipeline(path);

	if (spPipeline)
	{
		boost::intrusive_ptr<IXYCoverage> spGribProcAsCov;
		spPipeline->getParameter(0)->getValue(0)->getParameter(0)->getValue(0)->getOutput()->QueryInterface(
			IXYCoverage::iid, (void**) &spGribProcAsCov );


		if (spGribProcAsCov->getDefinition()->getFieldCount() > 0 && 
			spGribProcAsCov->getDefinition()->getFieldDefinition(0).isNumeric())
		{
			pVec->push_back(spPipeline);
		}

		int nFieldCount = spGribProcAsCov->getDefinition()->getFieldCount();

		for (int nFieldIndex = 1; nFieldIndex < nFieldCount; ++nFieldIndex)
		{	
			boost::intrusive_ptr<IProcess> spChannelPipeline;
			spChannelPipeline = buildAPipeline(path, nFieldIndex);
			
			if (spChannelPipeline)
			{
				pVec->push_back(spChannelPipeline);
			}
		}
		return true;
	}
	return false;
}
