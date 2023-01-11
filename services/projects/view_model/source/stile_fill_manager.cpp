/******************************************************************************
stile_fill_manager.cpp

begin		: 2009-10-19
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "stile_fill_manager.h"


#include "vector_utils.h"

// pyxlib includes
#include "pyxis/derm/iterator.h"
#include "pyxis/geometry/circle_geometry.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/procs/viewpoint.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/string_utils.h"


namespace
{
	//! group of pending threads
	boost::thread_group s_pendingThreads;	
}


//! Helper function for adding a job to a list and maintaing reference counting.
void AddJob(JobListsp<WeakPointerSTile>* list, PYXPointer<WeakPointerSTile> spJob)
{
	if (!list->AddJob(spJob))
	{
		PYXPointer<STile> spSTile = spJob->get();

		if (spSTile)
		{
			spSTile->decrementProcessing();
		}
	}
}


// Note: Every thread needs a smart pointer to the STileManager so that
// the STileManager will live as long as the threads do.

void getFromCacheThread(PYXPointer<STileFillManager> spManager, 
						 JobListsp<WeakPointerSTile>* from, JobListsp<WeakPointerSTile>* imageFillJobs, 
						 JobListsp<WeakPointerSTile>* elevationFillJobs, JobListsp<WeakPointerSTile>* vectorFillJobs,
						 JobListsp<WeakPointerSTile>* iconFillJobs)
{
	PYXPointer<WeakPointerSTile> spJob = from->GetJob();
	while (spJob)
	{
		PYXPointer<STile> spSTile = spJob->get();
		if (spSTile)
		{
			// TODO: cache the elevation using its identity.
			if ((spSTile->isRasterReloading() || !spSTile->isValid()) && spSTile->isOnScreen())
			{
				if (spSTile->isRasterReloading() || (!spSTile->isRasterFilled() && !spSTile->fillFromCache(spManager->getIdentity(), spManager->getMipLevel())))
				{
					AddJob(imageFillJobs, spJob);
				}
				else
				{
					// We got the image from cache now layer on the vector.
					AddJob(vectorFillJobs, spJob);
				}				

				// spawn off a parallel job to fill elevation
				spSTile->incrementProcessing();
				AddJob(elevationFillJobs, spJob);

				// spawn off a parallel job to fill icons
				spSTile->incrementProcessing();
				AddJob(iconFillJobs, spJob);
			}
			else
			{
				// We are done with it.
				spSTile->decrementProcessing();
			}
		}

		// Get the next job.
		spJob = from->GetJob();
	}
}

void getRasterDataThread(PYXPointer<STileFillManager> spManager, 
						 JobListsp<WeakPointerSTile>* from, JobListsp<WeakPointerSTile>* to, boost::intrusive_ptr<ICoverage> spCov)
{
	PYXPointer<WeakPointerSTile> spJob = from->GetJob();
	while (spJob)
	{
		PYXPointer<STile> spSTile = spJob->get();
		if (spSTile)
		{
			if (spSTile->isOnScreen())
			{
				spSTile->getImageData(spCov);
				AddJob(to, spJob);
			}
			else
			{
				// It is off screen, we are done with it.
				spSTile->decrementProcessing();
			}
		}
		spJob = from->GetJob();
	}
}

void fillRasterDataThread(PYXPointer<STileFillManager> spManager, 
						  JobListsp<WeakPointerSTile>* from, JobListsp<WeakPointerSTile>* to)
{
	PYXPointer<WeakPointerSTile> spJob = from->GetJob();
	while (spJob)
	{
		PYXPointer<STile> spSTile = spJob->get();
		if (spSTile)
		{
			spSTile->fillImageData();
			AddJob(to, spJob);
		}
		spJob = from->GetJob();
	}
}

unsigned char colours[][3] =
{
	// http://en.wikipedia.org/wiki/Web_colors
	{ 0x00, 0xff, 0xff }, // aqua
	{ 0x00, 0x00, 0x00 }, // black
	{ 0x00, 0x00, 0xff }, // blue
	{ 0xff, 0x00, 0xff }, // fuschia
	{ 0x00, 0x80, 0x00 }, // green
	{ 0x80, 0x80, 0x80 }, // grey
	{ 0x00, 0xff, 0x00 }, // lime
	{ 0x80, 0x00, 0x00 }, // maroon
	{ 0x00, 0x00, 0x80 }, // navy
	{ 0x80, 0x80, 0x00 }, // olive
	{ 0x80, 0x00, 0x80 }, // purple
	{ 0xff, 0x00, 0x00 }, // red
	{ 0xc0, 0xc0, 0xc0 }, // silver
	{ 0x00, 0x80, 0x80 }, // teal
	{ 0xff, 0xff, 0xff }, // white
	{ 0xff, 0xff, 0x00 }  // yellow
};

// TODO: not sure how big this gets -- probably needs some managment code.
// maybe this could be part of the STileFillManager, or would that defeat the purpose???
std::map<ProcRef, boost::shared_ptr<VectorPYXTree> > mapVectorTree;

// return true means fully filled from vtrees
bool fillVectorTile(PYXPointer<STileFillManager> spManager, PYXPointer<STile> spSTile)
{
	for (std::vector<VectorInput>::iterator it = spManager->m_featureCoverages.begin();
		it != spManager->m_featureCoverages.end(); ++it)
	{
		PYXPointer<VectorRGBData> vectorData = VectorRGBData::get((*it).m_procrefFC, spSTile->getRootIndex());
		//keep track of vectorData for each STile
		spSTile->m_vectorsGenerated.push_back(vectorData);
		
		if (vectorData->hasRGB())
		{
			// Already filled vectors for this tile for this feature collection.
			continue;
		}

		unsigned char* buf = new unsigned char[3*244*244*4];

		bool bAny = false;
		try
		{
			bAny = spSTile->fillVector((*it).m_spCov, buf, (*it).m_rgbaColour);
		}
		catch (...)
		{
			TRACE_DEBUG("There was an exception in STile::FillVector.");
		}
		if (bAny)
		{
			vectorData->setRGB(buf);						
		}
		else
		{
			delete[] buf;
		}
	}

	spSTile->setVectorFilled(true);
	spSTile->setVectorNeedsWritingToGL(true);
	return true;
}

void vectorThread(PYXPointer<STileFillManager> spManager, 
				  JobListsp<WeakPointerSTile>* from, JobListsp<WeakPointerSTile>* to)
{
	PYXPointer<WeakPointerSTile> spJob = from->GetJob();
	while (spJob != 0)
	{
		PYXPointer<STile> spSTile = spJob->get();
		if (spSTile)
		{
			if (spSTile->isOnScreen())
			{
				fillVectorTile(spManager, spSTile);			
			}

			AddJob(to, spJob);
		}
		spJob = from->GetJob();
	}
}

void fillIconTile(PYXPointer<IViewModel> view, ProcRef & procref, boost::intrusive_ptr<IViewPoint> spViewPoint, PYXPointer<STile> stile)
{
	//TODO: need to remove this from the stile_fill_manager	

	stile->setIconFilled(true);
}

void getIconThread(PYXPointer<STileFillManager> spManager, 
				   JobListsp<WeakPointerSTile>* from, JobListsp<WeakPointerSTile>* to)
{
	PYXPointer<WeakPointerSTile> spJob = from->GetJob();
	while (spJob)
	{
		PYXPointer<STile> spSTile = spJob->get();
		if (spSTile)
		{
			if (spSTile->isOnScreen())
			{
				// TODO: instead of passing in the ProcRef, we should do the work
				// of finding the feature collection at init time in the fill manager and pass
				// the feature collection (goes for vector filling too)
				fillIconTile(spManager->getViewHandle(), spManager->getProcRef(), spManager->getViewPoint(), spSTile);
				AddJob(to, spJob);
			}
			else
			{
				// It is off screen, we are done with it so release the count.
				spSTile->decrementProcessing();
			}
		}
		spJob = from->GetJob();
	}
}

void getElevationDataThread(PYXPointer<STileFillManager> spManager, 
						    JobListsp<WeakPointerSTile>* from, JobListsp<WeakPointerSTile>* to, boost::intrusive_ptr<ICoverage> spCov)
{
	PYXPointer<WeakPointerSTile> spJob = from->GetJob();
	while (spJob)
	{
		PYXPointer<STile> spSTile = spJob->get();
		if (spSTile)
		{
			if (spSTile->isOnScreen())
			{
				spSTile->getElevationData(spCov);
				AddJob(to, spJob);
			}
			else
			{
				// It is off screen, we are done with it
				spSTile->decrementProcessing();
			}
		}
		spJob = from->GetJob();
	}
}

void fillElevationDataThread(PYXPointer<STileFillManager> spManager, 
						     JobListsp<WeakPointerSTile>* from, JobListsp<WeakPointerSTile>* to)
{
	PYXPointer<WeakPointerSTile> spJob = from->GetJob();
	while (spJob != 0)
	{
		PYXPointer<STile> spSTile = spJob->get();
		if (spSTile)
		{
			spSTile->fillElevationData();
			AddJob(to, spJob);
		}
		spJob = from->GetJob();
	}
}

void writeCacheThread(PYXPointer<STileFillManager> spManager, 
				      JobListsp<WeakPointerSTile>* jobs, JobListsp<WeakPointerSTile>* toJobs)
{
	PYXPointer<WeakPointerSTile> spJob = jobs->GetJob();
	while (spJob)
	{
		PYXPointer<STile> spSTile = spJob->get();
		if (spSTile)
		{
			// If the STile is dirty and the raster data is filled, then write it to the STile cache.
			if (spSTile->isRasterFilled() && spSTile->needsWritingToDisk())
			{
				if (spSTile->writeToFile(spManager->getIdentity()))
				{
					spSTile->setNeedsWritingToDisk(false);
				}
			}
			AddJob(toJobs, spJob);
		}
		spJob = jobs->GetJob();
	}
}

void cleanupThread(PYXPointer<STileFillManager> spManager, 
				      JobListsp<WeakPointerSTile>* jobs)
{
	PYXPointer<WeakPointerSTile> spJob = jobs->GetJob();
	while (spJob)
	{
		PYXPointer<STile> spSTile = spJob->get();
		if (spSTile)
		{
			spSTile->decrementProcessing();
		}
		spJob = jobs->GetJob();
	}
}

void cleanupAllThread(boost::shared_ptr<boost::thread_group> threads)
{	
	threads->join_all();	
}


STileFillManager::STileFillManager(PYXPointer<IViewModel> view,
	boost::intrusive_ptr<ICoverage> spCov, 
	int mipLevel, 
	const ProcRef& procRef, 
	boost::intrusive_ptr<IViewPoint> spViewPoint)
{
	m_view = view;
	m_spCoverage = spCov;
	boost::intrusive_ptr<IProcess> spProcess;
	m_spCoverage->QueryInterface(IProcess::iid, (void**) &spProcess);
	assert(spProcess);
	// WARNING: fragile code -- needs updating when replacing the top of the view chain.
	m_strProcessIdentity =  spProcess->getParameter(0)->getValue(0)->getIdentity();
	m_mipLevel = mipLevel;
	m_procRef = procRef;
	m_spViewPoint = spViewPoint;

	std::vector<boost::intrusive_ptr<IProcess> > vecProc = m_spViewPoint->getFeatureCollectionPipelines();
	for (std::vector<boost::intrusive_ptr<IProcess> >::iterator it = vecProc.begin();
		it != vecProc.end(); ++it)
	{
		boost::intrusive_ptr<IProcess> spProc = *it;
		ProcRef procrefFC(spProc);

		/*
		If there is any colour style associated with the process then grab
		it here, otherwise use a colour table to select some colour.
		*/
		boost::intrusive_ptr<IFeatureCollection> spFC;
		spProc->QueryInterface(IFeatureCollection::iid, (void**) &spFC);
		std::string strRGBA = spFC->getStyle("LineColour");

		unsigned int rgba;
		bool haveStyleColour = false;
		if (!strRGBA.empty())
		{
			char *cstrColour, *type, *red, *green, *blue;
			unsigned int cstrColourSize = strRGBA.size() + 1;
			cstrColour = new char[cstrColourSize];
			
			// parse the colour string - break out if there is a parsing problem.
			strcpy_s(cstrColour, cstrColourSize , strRGBA.c_str());
			char * colourContext;
			type = strtok_s(cstrColour, " ", &colourContext);	// ignore type stored at front of string
			red = strtok_s(NULL, " ", &colourContext);
			green = strtok_s(NULL, " ", &colourContext);
			blue = strtok_s(NULL, " ", &colourContext);

			if (red && green && blue)
			{
				rgba =
					0xcc000000				// alpha (slightly trans)
					| (atoi(blue) << 16)	// blue
					| (atoi(green) << 8)	// green
					| atoi(red);			// red

				haveStyleColour = true;  // ahhh... all went well.
			}
		}

		if (!haveStyleColour)
		{ 
			/* 
			either there is no colour style specified, or we had a problem parsing
			the colour style.
			*/

			int nHash = 0;
			std::string strProcrefFC(procRefToStr(procrefFC));
			for (std::string::iterator it = strProcrefFC.begin();
				it != strProcrefFC.end(); ++it)
			{
				nHash += *it;
			}

			rgba =
				0xcc000000                       // alpha (slightly trans)
				| (colours[nHash % 16][2] << 16) // blue
				| (colours[nHash % 16][1] << 8)  // green
				| colours[nHash % 16][0];        // red
		}

		// Wrap with rasterizer.
		boost::intrusive_ptr<IProcess> spRasterizerProc;
		PYXCOMCreateInstance(strToGuid("{90A2B533-CFD2-4afb-AA89-8AD9A4E8FCB8}"), 0, IProcess::iid, (void**) &spRasterizerProc);
		assert(spRasterizerProc);
		spRasterizerProc->getParameter(0)->addValue(spProc);
		std::map<std::string, std::string> mapAttr;
		mapAttr["mode"] = "mask";
		spRasterizerProc->setAttributes(mapAttr);
		spRasterizerProc->initProc();

		// Add on a cache.
		boost::intrusive_ptr<IProcess> spCacheProc;
		PYXCOMCreateInstance(strToGuid("{83F35C37-5D0A-41c9-A937-F8C9C1E86850}"), 0, IProcess::iid, (void**) &spCacheProc);
		assert(spCacheProc);
		spCacheProc->getParameter(0)->addValue(spRasterizerProc);
		spCacheProc->initProc();

		// Save the coverage for use later
		boost::intrusive_ptr<ICoverage> spCov;
		spCacheProc->getOutput()->QueryInterface(ICoverage::iid, (void**) &spCov);
		assert(spCov);

		VectorInput vi(spCov, rgba, procrefFC);
		m_featureCoverages.push_back(vi);
	}
}

//  Flow of jobs --
//     Read from cache ------> Read coverage data ---> fill coverage data ---> write to cache ---> process vector data ------> cleanup and release
//                        |--> Read elevation data --> fill elevation data -----------------------------------------------/
//                        \--> Read and process icons -------------------------------------------------------------------/
//
//     If we successfully read from the cache then we skip the read fill and write cache steps and go directly to process vector data.


//! This method should only be called with a pointer to "this", and is not exposed to the outside world.
void STileFillManager::initialize(PYXPointer<STileFillManager> spThis)
{
	//create a threads group
	boost::shared_ptr<boost::thread_group> threads(new boost::thread_group());	

	// Create all the processing threads hooked to the appropriate job lists.
	threads->create_thread(boost::bind(&getFromCacheThread, spThis, &m_getFromCache, 
		&m_getRasterData, &m_getElevationData, &m_vector, &m_IconFill));
	threads->create_thread(boost::bind(&getRasterDataThread, spThis, &m_getRasterData, &m_fillRasterData, m_spCoverage));
	threads->create_thread(boost::bind(&fillRasterDataThread, spThis, &m_fillRasterData, &m_writeCache));
	threads->create_thread(boost::bind(&writeCacheThread, spThis, &m_writeCache, &m_vector));
	threads->create_thread(boost::bind(&vectorThread, spThis, &m_vector, &m_cleanUp));
	// We don't get any speed up from multithreading this now because the process is mutexed so hard
	// that we are basically single threaded.
	//threads.create_thread(boost::bind(&vectorThread, spThis, &m_vector, &m_cleanUp));
	threads->create_thread(boost::bind(&getElevationDataThread, spThis, &m_getElevationData, &m_fillElevationData, m_spCoverage));
	threads->create_thread(boost::bind(&fillElevationDataThread, spThis, &m_fillElevationData, &m_cleanUp));
	threads->create_thread(boost::bind(&getIconThread, spThis, &m_IconFill, &m_cleanUp));
	threads->create_thread(boost::bind(&cleanupThread, spThis, &m_cleanUp));

	//create a thread that would wait until all threads are done.
	s_pendingThreads.create_thread(boost::bind(&cleanupAllThread, threads));
}

void STileFillManager::SetTiles(std::vector<PYXPointer<STile>>& vecTiles)
{
	m_priorities.clear();

	// Add these tiles as new jobs to the list.
	for (std::vector< PYXPointer<STile> >::iterator it = vecTiles.begin(); it != vecTiles.end(); ++it)
	{
		if (!(*it)->isProcessing())
		{
			(*it)->incrementProcessing();
			(*it)->setNeedsReloading(false);
			(*it)->setRasterReloading((*it)->isRasterFilled());
			AddJob(&m_getFromCache, WeakPointerSTile::create(m_procRef,(*it)->getRootIndex()));
		}
		m_priorities.push_back((*it)->getRootIndex());
	}

	SortQueue(m_getFromCache);
	SortQueue(m_getRasterData);
	SortQueue(m_fillRasterData);
	SortQueue(m_getElevationData);
	SortQueue(m_fillElevationData);
	SortQueue(m_vector);
	SortQueue(m_writeCache);
	SortQueue(m_cleanUp);
	SortQueue(m_IconFill);	
}

void STileFillManager::SortQueue(JobListsp<WeakPointerSTile> & jobList)
{
	jobList.Sort(boost::bind<bool>(&STileFillManager::CompareJobs,this,_1,_2));
}

//! Helper function for STileFillManager::Stop()
void StopAndClearList(JobListsp<WeakPointerSTile>& list)
{
	std::vector< PYXPointer<WeakPointerSTile> > removedJobs;
	list.Stop(removedJobs);
	for(unsigned int index = 0; index < removedJobs.size(); ++index)
	{
		PYXPointer<STile> spSTile = removedJobs[index]->get();

		if (spSTile)
		{
			spSTile->decrementProcessing();
		}
	}
}

void STileFillManager::Stop()
{
	// Stop and empty all the job lists so that the threads will all
	// go away after the next processing.
	StopAndClearList(m_getFromCache);
	StopAndClearList(m_getRasterData);
	StopAndClearList(m_fillRasterData);
	StopAndClearList(m_getElevationData);
	StopAndClearList(m_fillElevationData);
	StopAndClearList(m_vector);
	StopAndClearList(m_writeCache);
	StopAndClearList(m_cleanUp);
	StopAndClearList(m_IconFill);
}


void STileFillManager::waitForThreadsToFinish()
{
	s_pendingThreads.join_all();	
}

bool STileFillManager::CompareJobs(const PYXPointer<WeakPointerSTile> & a, const PYXPointer<WeakPointerSTile> & b)
{
	//if a and b are the same - return false
	if (a->getIndex() == b->getIndex())
	{
		return false;
	}

	//if we find a before b - return true. if we found b before a - return false
	for(std::vector<PYXIcosIndex>::iterator it= m_priorities.begin();it!= m_priorities.end();++it)
	{
		if (*it == a->getIndex())
		{
			return true;
		}

		if (*it == b->getIndex())
		{
			return false;
		}
	}

	//we didn't find a nor b - return false
	return false;
}