/******************************************************************************
visible_store.cpp

begin		: 2010-05-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "StdAfx.h"
#include "visible_store.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/utility/app_services.h"

namespace Storage
{

VisibleStore::VisibleStore()
{
}

VisibleStore::~VisibleStore()
{
	clearAllData();
}


PYXPointer<VisibleStore::VisibleTile> VisibleStore::operator[](const PYXIcosIndex & index)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	std::set<PYXIcosIndex>::iterator it = m_visibleTilesSet.find(index);
	if (it != m_visibleTilesSet.end())
	{
		return PYXNEW(VisibleStore::VisibleTile,this,index);
	}
	else
	{
		return PYXPointer<VisibleStore::VisibleTile>();
	}
}

PYXPointer<VisibleStore::VisiblePipeline> VisibleStore::operator[](const ProcRef & procRef)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	std::set<ProcRef>::iterator it = m_pipelinesSet.find(procRef);
	if (it != m_pipelinesSet.end())
	{
		return PYXNEW(VisibleStore::VisiblePipeline,this,procRef);
	}
	else
	{
		return PYXPointer<VisibleStore::VisiblePipeline>();
	}
}

PYXPointer<VisibleStore::VisibleKey> VisibleStore::getKey(const PYXIcosIndex & index,const ProcRef & procRef)
{
	//!note - can be not visible
	return PYXNEW(VisibleStore::VisibleKey,this,index,procRef);
}

bool VisibleStore::hasKey(const VisibleKey & key) const
{
	return hasKey(key.first,key.second);
}

bool VisibleStore::hasKey(const PYXIcosIndex & index,const ProcRef & procRef) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	if (m_viewPointProcessProcRef == procRef)
	{
		return m_visibleTilesSet.find(index) != m_visibleTilesSet.end();
	}
	else
		return (m_pipelinesSet.find(procRef) != m_pipelinesSet.end() && m_visibleTilesSet.find(index) != m_visibleTilesSet.end());
}

void VisibleStore::setViewPointProcess(boost::intrusive_ptr<IViewPoint> viewPointProcess)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	m_viewPointProcessProcRef = ProcRef(viewPointProcess->QueryInterface<IProcess>());

	std::vector<boost::intrusive_ptr<IProcess>> allPipelines(viewPointProcess->getAllPipelines());

	std::set<ProcRef> newPiplines;
	
	for(std::vector<boost::intrusive_ptr<IProcess>>::iterator it = allPipelines.begin(); it != allPipelines.end(); ++ it)
	{
		newPiplines.insert(ProcRef(*it));
	}

	std::list<ProcRef> addedPipelines;
	std::list<ProcRef> removedPipelines;
	std::set_difference(newPiplines.begin(),newPiplines.end(),m_pipelinesSet.begin(),m_pipelinesSet.end(),std::inserter(addedPipelines,addedPipelines.end()));
	std::set_difference(m_pipelinesSet.begin(),m_pipelinesSet.end(),newPiplines.begin(),newPiplines.end(),std::inserter(removedPipelines,removedPipelines.end()));

	//remove pipelines
	for(std::list<ProcRef>::iterator it = removedPipelines.begin(); it != removedPipelines.end(); ++it)
	{
		m_pipelineRemovedNotifier.notify(PYXNEW(VisiblePipelineEvent,this,*it));
	}

	//update pipelines lists...
	m_viewPointProcess = viewPointProcess;
	m_pipelinesSet = newPiplines;
	m_pipelines = std::vector<ProcRef>(m_pipelinesSet.begin(),m_pipelinesSet.end());

	//add new pipelines
	for(std::list<ProcRef>::iterator it = addedPipelines.begin(); it != addedPipelines.end(); ++it)
	{
		m_pipelineAddedNotifier.notify(PYXNEW(VisiblePipelineEvent,this,*it));
	}
}

boost::intrusive_ptr<IViewPoint> VisibleStore::getViewPointProcess() const 
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	return m_viewPointProcess;
}

void VisibleStore::setVisibleTiles(const std::vector<PYXIcosIndex> & visibleTiles) 
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	
	std::set<PYXIcosIndex> newVisibleTiles(visibleTiles.begin(),visibleTiles.end());

	std::list<PYXIcosIndex> addedTiles;
	std::list<PYXIcosIndex> removedTiles;
	std::set_difference(newVisibleTiles.begin(),newVisibleTiles.end(),m_visibleTilesSet.begin(),m_visibleTilesSet.end(),std::inserter(addedTiles,addedTiles.end()));
	std::set_difference(m_visibleTilesSet.begin(),m_visibleTilesSet.end(),newVisibleTiles.begin(),newVisibleTiles.end(),std::inserter(removedTiles,removedTiles.end()));

	//remove pipelines
	for(std::list<PYXIcosIndex>::iterator it = removedTiles.begin(); it != removedTiles.end(); ++it)
	{
		m_tileRemovedNotifier.notify(PYXNEW(VisibleTileEvent,this,*it));
	}

	//update visible lists...	
	m_visibleTilesSet = newVisibleTiles;
	m_visibleTiles = visibleTiles;

	//add new pipelines
	for(std::list<PYXIcosIndex>::iterator it = addedTiles.begin(); it != addedTiles.end(); ++it)
	{
		m_tileAddedNotifier.notify(PYXNEW(VisibleTileEvent,this,*it));
	}
}

void VisibleStore::clearAllData()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	//remove pipelines
	for(std::vector<PYXIcosIndex>::iterator it = m_visibleTiles.begin(); it != m_visibleTiles.end(); ++it)
	{
		m_tileRemovedNotifier.notify(PYXNEW(VisibleTileEvent,this,*it));
	}

	//remove pipelines
	for(std::vector<ProcRef>::iterator it = m_pipelines.begin(); it != m_pipelines.end(); ++it)
	{
		m_pipelineRemovedNotifier.notify(PYXNEW(VisiblePipelineEvent,this,*it));
	}

	//update lists
	m_visibleTilesSet.clear();
	m_visibleTiles.clear();
	m_pipelinesSet.clear();
	m_pipelines.clear();
	m_viewPointProcess.reset();
}

std::vector<ProcRef> VisibleStore::getPipelines() const 
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	return m_pipelines;
}

std::vector<PYXIcosIndex> VisibleStore::getVisibleTiles() const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	return m_visibleTiles;
}


VisibleStore::VisibleKeysVector VisibleStore::VisibleTile::getKeys()
{
	boost::recursive_mutex::scoped_lock lock(m_dataStore->m_mutex);

	VisibleKeysVector result;

	for(std::vector<ProcRef>::iterator it = m_dataStore->m_pipelines.begin();it != m_dataStore->m_pipelines.end(); ++it)
	{
		result.push_back(PYXNEW(VisibleStore::VisibleKey,m_dataStore,m_index,*it));
	}
	return result;
}


VisibleStore::VisibleKeysVector VisibleStore::VisiblePipeline::getKeys()
{
	boost::recursive_mutex::scoped_lock lock(m_dataStore->m_mutex);

	VisibleKeysVector result;

	for(std::vector<PYXIcosIndex>::iterator it = m_dataStore->m_visibleTiles.begin();it != m_dataStore->m_visibleTiles.end(); ++it)
	{
		result.push_back(PYXNEW(VisibleStore::VisibleKey,m_dataStore,*it,m_procRef));
	}
	return result;
}

}