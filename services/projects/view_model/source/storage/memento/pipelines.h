#pragma once
/******************************************************************************
pipelines.h

begin		: 2010-05-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model_api.h"
#include "storage/visible_store.h"

#include <map>
#include <vector>
#include <set>


namespace Storage
{
namespace Memento
{
		
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// VisiblePipeline implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T>
class VisiblePipelines : public PYXObject
{
protected:
	boost::recursive_mutex m_mutex;
	std::map<ProcRef,T> m_mementos;
	PYXPointer<Stroage::VisibleStore> m_dataStore;

public:
	VisiblePipelines(const PYXPointer<Stroage::VisibleStore> & dataStore) : m_dataStore(dataStore)
	{	
		m_dataStore->getPipelineRemovedNotifier().attach(this,&VisiblePipelines::pipelineRemoved);
	}

	virtual ~VisiblePipelines()
	{		
		m_dataStore->getPipelineRemovedNotifier().detach(this,&VisiblePipelines::pipelineRemoved);
	}

	virtual T & get(const PYXPointer<Stroage::VisibleStore::VisibleKey> & key)
	{
		return get(key->getProcRef());
	}

	virtual T & get(const ProcRef & pipeline)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		return m_mementos[pipeline];
	}

	void pipelineRemoved(PYXPointer<NotifierEvent> notifierEvent)
	{
		VisiblePipelineEvent * pipeline = dynamic_cast<Stroage::VisiblePipelineEvent *>(notifierEvent.get());

		if (pipeline != 0)
		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);

			m_mementos.erase(pipeline->getProcRef());			
		}
	}
};

//namespace Memento
}

//namespace Storage
}