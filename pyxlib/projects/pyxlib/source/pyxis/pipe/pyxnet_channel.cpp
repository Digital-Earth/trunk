/******************************************************************************
pyxnet_channel.cpp

begin		: July 13, 2012
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/pipe/pyxnet_channel.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/exceptions.h"

#include "pyxis/utility/thread_pool.h"

// standard includes
#include <cassert>

PYXNETChannel::Handle::Handle(const ProcRef & processProcRef, const std::string & code) : m_procRef(processProcRef), m_dataCode(code), m_handle(-1), m_foundRemotely(false), m_published(false), m_refCount(0)
{
}


PYXPointer<PYXNETChannel> PYXNETChannel::create(const ProcRef & processProcRef, const std::string & code)
{
	PYXPointer<PYXNETChannel> channel = PYXNEW(PYXNETChannel,processProcRef,code);

	channel->m_handle->m_handle = PYXNETChannelProvider::getInstance()->createChannel(processProcRef,code);

	return channel;
}

PYXPointer<PYXNETChannel> PYXNETChannel::create(const PYXPointer<PYXNETChannel::Handle> & handle)
{
	//assume handle already been attached to PYXNETChannelProvider
	return PYXNEW(PYXNETChannel,handle);
}

PYXNETChannel::PYXNETChannel(const PYXPointer<PYXNETChannel::Handle> & handle) : m_handle(handle)
{
	++m_handle->m_refCount;
}

PYXNETChannel::PYXNETChannel(const ProcRef & processProcRef, const std::string & code) : m_handle(PYXNETChannel::Handle::create(processProcRef,code))
{
	++m_handle->m_refCount;
}

PYXNETChannel::~PYXNETChannel()
{
	long oldRef = --m_handle->m_refCount;
	if (oldRef == 0) 
	{
		auto channelProvider = PYXNETChannelProvider::getInstance();
		if (channelProvider) 
		{
			channelProvider->releaseChannel(m_handle->m_handle);
		}
	}
}


const ProcRef & PYXNETChannel::getProcRef() const { return m_handle->m_procRef; }
const std::string  & PYXNETChannel::getDataCode() const { return m_handle->m_dataCode; }
int PYXNETChannel::getHandle() const { return m_handle->m_handle; }

void PYXNETChannel::channelWasPublished() { m_handle->m_published = true; } 
void PYXNETChannel::channelFoundRemotely() { m_handle->m_foundRemotely = true; } 


class GetKeyTask : public PYXNETChannel::GetKeyTaskResult
{
public:
	friend class PYXNETChannel;

	static PYXPointer<GetKeyTask> create(const std::string & key)
	{
		return PYXNEW(GetKeyTask,key);
	}
	GetKeyTask(const std::string & key) : m_key(key)
	{
	}

private:
	void setResult(const PYXPointer<PYXConstBuffer> & result)
	{
		getResultInternal() = result;
		setCompleted();
	}

	void setError(const std::string & error)
	{
		setCompletedWithError(error);
	}

private:
	std::string m_key;
};

void PYXNETChannel::addPendingTask(PYXPointer<PYXNETChannel::Handle> handle, const std::string & key,PYXPointer<GetKeyTaskResult> task)
{
	boost::recursive_mutex::scoped_lock lock(handle->m_mutex);
	handle->m_pendingTasks.insert(std::make_pair(key,task));
	task->onTaskCompleted(boost::bind(&PYXNETChannel::removePendingTask,handle,key,task));
}

void PYXNETChannel::removePendingTask(PYXPointer<PYXNETChannel::Handle> handle, std::string key,PYXPointer<GetKeyTaskResult> task)
{
	boost::recursive_mutex::scoped_lock lock(handle->m_mutex);
	std::pair<Handle::PendingTaskMap::iterator,Handle::PendingTaskMap::iterator> range = handle->m_pendingTasks.equal_range(key);
	for(Handle::PendingTaskMap::iterator it = range.first;it!= range.second;++it)
	{
		if (it->second.get() == task)
		{
			handle->m_pendingTasks.erase(it);
			return;
		}
	}
}

PYXPointer<PYXNETChannel::GetKeyTaskResult> PYXNETChannel::requestKey(const std::string & key)
{

	PYXPointer<GetKeyTask> task = GetKeyTask::create(key);
	addPendingTask(m_handle,key,task);

	try
	{
		PYXNETChannelProvider::getInstance()->requestKey(m_handle->m_handle,key);
	}
	catch(PYXException& e)
	{
		task->setError("Failed to request Key: " + key + " with error: " + e.getFullErrorString());		
	}
	catch(...)
	{
		task->setError("Failed to request Key: " + key);
	}

	return task;
}

void PYXNETChannel::keyProvidedFailed(const std::string & key)
{
	boost::recursive_mutex::scoped_lock lock(m_handle->m_mutex);
	std::pair<Handle::PendingTaskMap::iterator,Handle::PendingTaskMap::iterator> range = m_handle->m_pendingTasks.equal_range(key);
	for(Handle::PendingTaskMap::iterator it = range.first;it!= range.second;++it)
	{
		GetKeyTask * task = dynamic_cast<GetKeyTask *>(it->second.get());

		if (task != 0)
		{
			task->setError("Failed to get key from pyxnet: " + key);
		}
	}
}

void PYXNETChannel::keyProvided(const std::string & key,const std::string & value)
{
	boost::recursive_mutex::scoped_lock lock(m_handle->m_mutex);
	std::pair<Handle::PendingTaskMap::iterator,Handle::PendingTaskMap::iterator> range = m_handle->m_pendingTasks.equal_range(key);
	for(Handle::PendingTaskMap::iterator it = range.first;it!= range.second;++it)
	{
		GetKeyTask * task = dynamic_cast<GetKeyTask *>(it->second.get());

		if (task != 0)
		{
			task->setResult(PYXConstBuffer::create(value));
		}
	}
}

void PYXNETChannel::publish()
{
	PYXNETChannelProvider::getInstance()->publishChannel(m_handle->m_handle);
}

void PYXNETChannel::unpublish()
{
	auto channelProvider = PYXNETChannelProvider::getInstance();
	if (channelProvider) 
	{
		channelProvider->unpublishChannel(m_handle->m_handle);
		m_handle->m_published = false;	
	}
}

void PYXNETChannel::attachLocalProvider(const PYXPointer<PYXNETChannelKeyProvider> & provider)
{
	PYXNETChannelProvider::getInstance()->addLocalProvider(m_handle->m_handle,provider);
}


PYXPointer<PYXNETChannelProvider> PYXNETChannelProvider::m_spProvider;

PYXPointer<PYXNETChannel> PYXNETChannelProvider::getChannel(int channelId)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	ChannelsMapByHandle::iterator it = m_channelByHandle.find(channelId);
	if (it != m_channelByHandle.end())
	{
		return PYXNETChannel::create(it->second);
	}

	return 0;
}

PYXPointer<PYXNETChannel> PYXNETChannelProvider::getOrCreateChannel(const ProcRef & processProcRef, const std::string & code )
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	for(ChannelsMapByHandle::iterator it = m_channelByHandle.begin(); it != m_channelByHandle.end(); ++it)
	{
		if (it->second->m_procRef == processProcRef && it->second->m_dataCode == code)
		{
			return PYXNETChannel::create(it->second);
		}
	}

	PYXPointer<PYXNETChannel> result = PYXNETChannel::create(processProcRef,code);

	m_channelByHandle[result->getHandle()] = result->m_handle;

	return result;
}

void PYXNETChannelProvider::releaseChannel(int channelId)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	ChannelsMapByHandle::iterator it = m_channelByHandle.find(channelId);
	if (it != m_channelByHandle.end())
	{
		removeChannel(channelId);
		m_channelByHandle.erase(it);
	}	
}

int PYXNETChannelProvider::createChannel(const ProcRef & processProcRef,const std::string & code)
{
	PYXTHROW(PYXException,"PYXNETChannelProvider has not been set");
}

void PYXNETChannelProvider::removeChannel(int channelId)
{
	PYXTHROW(PYXException,"PYXNETChannelProvider has not been set");
}

void PYXNETChannelProvider::requestKey(int channelId, const std::string & keyBase64)
{
	PYXTHROW(PYXException,"PYXNETChannelProvider has not been set");
}

void PYXNETChannelProvider::cancelRequestKey(int channelId, const std::string & keyBase64)
{
	PYXTHROW(PYXException,"PYXNETChannelProvider has not been set");
}

void PYXNETChannelProvider::addLocalProvider(int channelId, PYXPointer<PYXNETChannelKeyProvider> provider)
{
	PYXTHROW(PYXException,"PYXNETChannelProvider has not been set");
}

void PYXNETChannelProvider::publishChannel(int channelId)
{
	PYXTHROW(PYXException,"PYXNETChannelProvider has not been set");
}

void PYXNETChannelProvider::unpublishChannel(int channelId)
{
	PYXTHROW(PYXException,"PYXNETChannelProvider has not been set");
}

void PYXNETChannelProvider::onChannelFoundRemotely(int channelId)
{
	PYXPointer<PYXNETChannel> channel = getChannel(channelId);

	if (channel)
	{
		channel->channelFoundRemotely();
	}
}

void PYXNETChannelProvider::onChannelPublished(int channelId)
{
	PYXPointer<PYXNETChannel> channel = getChannel(channelId);

	if (channel)
	{
		channel->channelWasPublished();
	}
}

void PYXNETChannelProvider::keyProvidedFailed(int channelId, const std::string & key)
{
	PYXPointer<PYXNETChannel> channel = getChannel(channelId);

	if (channel)
	{
		channel->keyProvidedFailed(key);
	}
}

void PYXNETChannelProvider::keyProvided(int channelId, const std::string & key,const std::string & dataBase64)
{
	PYXPointer<PYXNETChannel> channel = getChannel(channelId);

	if (channel)
	{
		channel->keyProvided(key,XMLUtils::fromBase64(dataBase64));
	}
}

PYXPointer<PYXNETChannelProvider> PYXNETChannelProvider::getInstance()
{
	return m_spProvider;
}

void PYXNETChannelProvider::setInstance( PYXPointer<PYXNETChannelProvider> spProvider)
{
	m_spProvider = spProvider;
}