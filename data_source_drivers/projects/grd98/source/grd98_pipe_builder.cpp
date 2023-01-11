/***************************************************************************
grd98_pipe_builder.cpp

begin		: 2008-04-24
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
***************************************************************************/
#include "stdafx.h"
#define MODULE_GRD98_SOURCE
#include "grd98_pipe_builder.h"

#include "pyxis/data/exceptions.h"
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/utility/file_utils.h"

// {323FD624-010E-48ee-B991-89FF3FC96F54}
PYXCOM_DEFINE_CLSID(GRD98PipeBuilder, 
0x323fd624, 0x10e, 0x48ee, 0xb9, 0x91, 0x89, 0xff, 0x3f, 0xc9, 0x6f, 0x54);

PYXCOM_CLASS_INTERFACES(GRD98PipeBuilder, IPipeBuilder::iid, PYXCOM_IUnknown::iid);

/*!
Sets the required data types that this pipebuilder is going to support.
*/
GRD98PipeBuilder::GRD98PipeBuilder()
{
	//TODO: Add any other types that can be read through GRD98.
	m_vecSupportedExtensions.push_back("g98");
}

/*!
Build the catalog describing the data source.

\param strPath	The path to the data source. May be a file, directory or url.

\return The catalog or nullptr if the data source is not supported.
*/
PYXPointer<const PYXCatalog> STDMETHODCALLTYPE GRD98PipeBuilder::buildCatalog(const std::string& strPath) const
{
	PYXPointer<PYXCatalog> pCatalog = nullptr;

	if (isDataSourceSupported(strPath, eCheckOptions::knLenient))
	{
		auto pProcess = new GRD98Process();
		if (pProcess != nullptr)
		{
			try
			{
				if (pProcess->open(strPath))
				{
					pCatalog = PYXCatalog::create();
					pCatalog->setUri(strPath);
					pCatalog->setName(strPath);

					auto strLeaf = FileUtils::pathToString(FileUtils::stringToPath(strPath).leaf());
					auto pDataSet = PYXDataSet::create(strPath, strLeaf);
					GRD98Process::addContentDefinition(pDataSet->getContentDefinition());

					// record the number of points in the data set
					pDataSet->addField(
						PYXDataSet::s_strPyxisPixelCount,
						PYXFieldDefinition::knContextNone,
						PYXValue::knDouble,
						1,
						PYXValue((double) pProcess->getPointCount()));

					pCatalog->addDataSet(pDataSet);
				}

			}
			catch (...)
			{
				// fall through
			}

			delete pProcess;
		}
	}

	return pCatalog;
}

/*!
Build a pipeline for the path.

\param pDataSet  Describes the data set to be opened.

\return The head of the pipeline or 0 if no pipeline created.
*/
boost::intrusive_ptr<IProcess> GRD98PipeBuilder::buildPipeline(PYXPointer<PYXDataSet> pDataSet) const
{
	boost::intrusive_ptr<IProcess> spProc;

	if (isDataSourceSupported(pDataSet->getUri(), eCheckOptions::knLenient))
	{
		spProc = buildAPipeline(FileUtils::stringToPath(pDataSet->getUri()));
	}

	return spProc;
}

/*!
Build an individual pipeline for a given gdal datasource. The pipeline built is,
a PathProcess->GRD98Driver->Default Sampler(Nearest Neighbour)->Default Coverage Cache->
GreyscaleToRGBConverter(if Needed).

\param path  The boost path to the uri to build a pipeline on a given channel for.

\return An intrusive ptr to the head process in the pipeline, if the pipeline was 
 		successfully created or a null pointer if it was not.
*/
boost::intrusive_ptr<IProcess> GRD98PipeBuilder::buildAPipeline(const boost::filesystem::path &path) const
{
	try
	{
		boost::intrusive_ptr<IProcess> spPathProcess;
		PYXCOMCreateInstance(strToGuid("{7B50BE20-F6A5-401e-8E76-98C5226DAF0D}"), 0, IProcess::iid, (void**) &spPathProcess);
		if (spPathProcess)
		{
			std::map<std::string, std::string> attrMap;
			attrMap["uri"] = FileUtils::pathToString(path);
			spPathProcess->setAttributes(attrMap);

			boost::intrusive_ptr<IProcess> spGrd98Proc;
			PYXCOMCreateInstance(strToGuid("{751D51B6-4B30-4bdc-8DCD-AFBBF056B96F}"), 0, IProcess::iid, (void**) &spGrd98Proc);
			if (spGrd98Proc)
			{
				spGrd98Proc->getParameter(0)->addValue(spPathProcess);
				boost::intrusive_ptr<IProcess> spSamplerProc = getDefaultSampler();
				if (spSamplerProc)
				{
					spSamplerProc->getParameter(0)->addValue(spGrd98Proc);
					spSamplerProc->setProcName("Sampling: " + FileUtils::pathToString(path.leaf()));

					boost::intrusive_ptr<IProcess> spCacheProc;
					PYXCOMCreateInstance(strToGuid("{83F35C37-5D0A-41c9-A937-F8C9C1E86850}"), 0, IProcess::iid, (void**) &spCacheProc);
					if (spCacheProc)
					{
						spCacheProc->getParameter(0)->addValue(spSamplerProc);
						spCacheProc->setProcName(FileUtils::pathToString(path.leaf()));
							
						if (spCacheProc->initProc(true) != IProcess::knInitialized)
						{
							TRACE_INFO("Could not build a pipeline for: " << FileUtils::pathToString(path));
							return boost::intrusive_ptr<IProcess>();
						}
						return spCacheProc;
					}
				}
			}
		}
	}
	catch(PYXException&)
	{
		TRACE_INFO("Unable to create a pipeline for: " + FileUtils::pathToString(path));
	}

	return boost::intrusive_ptr<IProcess> ();
}
