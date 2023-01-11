/***************************************************************************
pyxis_pipe_builder.cpp

begin		: 2007-08-09
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
***************************************************************************/
#include "stdafx.h"
#define MODULE_PYXIS_COVERAGES_SOURCE

#include "pyxis_pipe_builder.h"

// pyxlib includes
#include "pyxis/pipe/pipe_manager.h"
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/procs/path.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/tester.h"

// {662BA78D-8C57-401e-AF9B-A4FE68A835BD}
PYXCOM_DEFINE_CLSID(PyxisPipeBuilder, 
0x662ba78d, 0x8c57, 0x401e, 0xaf, 0x9b, 0xa4, 0xfe, 0x68, 0xa8, 0x35, 0xbd);

PYXCOM_CLASS_INTERFACES(PyxisPipeBuilder, IPipeBuilder::iid, PYXCOM_IUnknown::iid);


Tester<PyxisPipeBuilder> gTester;
void PyxisPipeBuilder::test()
{
	/*
	See the PipeBuilderManager::test for how this class is tested.
	*/
}

PyxisPipeBuilder::PyxisPipeBuilder()
{
	m_vecSupportedExtensions.push_back("ppl");
}

/*!
Builds the boost file system path it receives into an individual pipeline.
Determine if the particular path has been processed yet or not. If the path 
has not been processed then we build a pipeline. If building the pipeline
was successful we return a boost intrusive pointer to the process which 
represents the pipeline. Otherwise an unitialized intrusive pointer is 
returned. Attempting to construct a pipeline for a path which has already been
processed results in an unsuccessful creation of the pipeline.

\param pDataSet	Describes the data set to be opened.

\return A pointer to the head of the pipeline or 0 if no pipeline was created.
*/
PYXPointer<IProcess> PyxisPipeBuilder::buildPipeline(PYXPointer<PYXDataSet> pDataSet) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	boost::intrusive_ptr<IProcess> spProc = 0;

	// Check to see if this path is empty or not.
	auto path = FileUtils::stringToPath(pDataSet->getUri());
	if (!path.empty())
	{
		if (isDataSourceSupported(pDataSet->getUri(), eCheckOptions::knLenient))
		{
			spProc = PipeManager::readPipelineFromFile(pDataSet->getUri());
		}

		if (spProc)
		{
			// cycle through all of the paths in the pipeline
			std::vector< boost::intrusive_ptr<PYXCOM_IUnknown> > vecPaths;
			int nCount = PipeUtils::findProcsOfType(spProc, IPath::iid, vecPaths);
			assert(nCount == static_cast<int>(vecPaths.size()));
			for (int nIndex = 0; nIndex < nCount; ++nIndex)
			{
				boost::intrusive_ptr<IPath> spPath;
				vecPaths[nIndex]->QueryInterface(IPath::iid, (void**) &spPath);
				assert(spPath);
				
				boost::filesystem::path startPath = FileUtils::stringToPath(spPath->getPath());
				if (!startPath.is_complete())
				{
					// Prepend process path if it was specified relative on an import.
					boost::filesystem::path tempPath = PipeManager::getProcessPath() / startPath;
					if (tempPath.is_complete() && FileUtils::exists(tempPath))
					{
						if (FileUtils::isDirectory(tempPath))
						{
							spPath->setPath(FileUtils::pathToString(tempPath));
						}
						else
						{
							spPath->setPath(FileUtils::pathToString(tempPath));
						}
					}
				}
			}

			//make sure the process init it self before we import it into the library 
			spProc->initProc(true);

			return spProc;
		}
	}

	return spProc;
}
