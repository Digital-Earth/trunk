/******************************************************************************
fill_utils.cpp

begin		: 2007-10-05
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "fill_utils.h"
#include "performance_counter.h"

// view model includes
#include "stile_fill_manager.h"

// pyxlib includes
#include "pyxis/procs/viewpoint.h"

namespace
{
	//! This mutex can be used to lock the whole fill system.
	boost::recursive_mutex fillMutex;

	//! This string holds any error from the fill threads.
	std::string strFillError;

	//! The current fill manager.
	PYXPointer<STileFillManager> spFillManager;


	//! This deque keeps track of the 500 most recently on screen tiles.
	std::deque<std::pair<ProcRef,PYXIcosIndex>> deqTiles;
}

void FillUtils::setFillTiles(
	PYXPointer<IViewModel> view,
	boost::intrusive_ptr<ICoverage> spCoverage, 
	int nMipLevel, 
	ProcRef procRef, 
	boost::intrusive_ptr<IViewPoint> spViewPoint, 
	const std::vector<PYXPointer<STile>>& vecTiles, 
	std::string& strError)
{
	boost::recursive_mutex::scoped_lock lock(fillMutex);

	// NOTE: the fill ViewPointProcess and fill tiles are always locked via this code.

	if (spFillManager)
	{
		ProcRef fillProcRef = spFillManager->getProcRef();

		if (procRef != fillProcRef)
		{
			spFillManager->Stop();
			STile::unlockPipe(fillProcRef);

			if (procRef == ProcRef())
			{
				spFillManager.reset();
			}
			else
			{
				createFillManager(view,spCoverage, nMipLevel, procRef, spViewPoint);
			}

			strFillError.clear();
		}
	}
	else
	{
		if (procRef != ProcRef())
		{
			createFillManager(view,spCoverage, nMipLevel, procRef, spViewPoint);
		}
	}

	for (int n = static_cast<int>(vecTiles.size()) - 1; n != -1; --n)
	{
		PYXPointer<STile> spSTile = vecTiles[n];
		std::pair<ProcRef,PYXIcosIndex> pair(procRef, spSTile->getRootIndex());

		std::deque<std::pair<ProcRef,PYXIcosIndex>>::iterator it = std::find(deqTiles.begin(), deqTiles.end(), pair);
		if (it != deqTiles.end())
		{
			// Effectively, keep its lock but move it to the front.
			deqTiles.erase(it);
		}
		
		deqTiles.push_front(pair);
	}

	if (spFillManager)
	{
		// Make a list of the tiles to fill
		std::vector< PYXPointer<STile> > vecFillTiles;
		for (int n = 0; n < static_cast<int>(vecTiles.size()); ++n)
		{
			PYXPointer<STile> spSTile = vecTiles[n];
			if (!spSTile->isValid() || spSTile->needsReloading())
			{
				vecFillTiles.push_back(spSTile);
			}
		}
		spFillManager->SetTiles(vecFillTiles);
	}

	std::deque<std::pair<ProcRef,PYXIcosIndex>>::iterator it = deqTiles.begin();
	
	int sameProcInQueue = 0;
	while( it != deqTiles.end())
	{
		if (it->first == procRef)
		{
			sameProcInQueue++;
		}
		++it;
	}
	
	PerformanceCounter::getValuePerformanceCounter("total STile in map",1.0f,0.5f,0.5f)->setMeasurement(STile::getTotalCount());

	PerformanceCounter::getValuePerformanceCounter("Current Proc STile in queue",1.0f,0.5f,0.5f)->setMeasurement(sameProcInQueue);

	PerformanceCounter::getValuePerformanceCounter("Current Proc STile in tileSet",0.5f,1.0f,0.5f)->setMeasurement(STile::getCount(procRef));

	PerformanceCounter::getValuePerformanceCounter("STile queue",1.0f,0.5f,0.5f)->setMeasurement(deqTiles.size());

	while (knMaxSTileCacheSize < static_cast<int>(deqTiles.size()))
	{
		// erase it from our big map of maps.
		std::pair<ProcRef,PYXIcosIndex> & pair = deqTiles.back();
		STile::erase(pair.first,pair.second);

		// pop it off the back
		deqTiles.pop_back();
	}

	// Error string.
	if (strError.empty())
	{
		strError = strFillError;
		strFillError.clear();
	}
}

void FillUtils::closeAllResources()
{
	STileFillManager::waitForThreadsToFinish();
}


void FillUtils::createFillManager(
	PYXPointer<IViewModel> view,
	boost::intrusive_ptr<ICoverage> spCoverage, 
	int nMipLevel, 
	ProcRef procRef, 
	boost::intrusive_ptr<IViewPoint> spViewPoint)
{
	STile::lockPipe(procRef);
	spFillManager = STileFillManager::create(view,spCoverage, nMipLevel, procRef, spViewPoint);
}