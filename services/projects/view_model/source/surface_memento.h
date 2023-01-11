#pragma once
#ifndef VIEW_MODEL__SURFACE_MEMENTO_H
#define VIEW_MODEL__SURFACE_MEMENTO_H
/******************************************************************************
surface_memento.h

begin		: 2009-11-01
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "camera.h"
#include "surface.h"
#include "cml_utils.h"

#include "pyxis/utility/object.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/notifier.h"
#include "pyxis/derm/index.h"
#include "pyxis/pipe/process.h"

#include <map>
#include <list>
#include <vector>

#include <boost/thread/recursive_mutex.hpp>
#include <boost/function.hpp>

/*!

MementoCreator<T> - memento creator interface.

MementoCreator interface is used by SurfaceMemento<T> to create a memento when needed.

the MementoCreator API is:
1. createMemento(patch) - create memento for the given patch
2. destroyMemento(patch) - if the creator has a private storage,clear the data about this patch - called when the SurfaceMemento decided to clear the data for this patch
3. validateMemento(patch,memento) - give the mementoCreator a change to validate/remove/change the current memento - called before a SurfaceMemento returns the memento.

Note, the MementoCreator<T> should be thread safe. the create/validate/forget can be called from different threads
*/
template<class T>
class MementoCreator : public PYXObject
{
public:
	virtual PYXPointer<T> createMemento(const PYXPointer<Surface::Patch> & patch) = 0;
	virtual void destroyMemento(const PYXPointer<Surface::Patch> & patch,PYXPointer<T> & memento) = 0;
	virtual PYXPointer<T> validateMemento(const PYXPointer<Surface::Patch> & patch,const PYXPointer<T> & memento) = 0;
};

template<class T>
class VersionedMemento : public PYXObject
{
protected:
	PYXPointer<Surface::Patch> m_patch;

	PYXPointer<T> m_data;
	PYXPointer<T> m_newData;
	PYXPointer<T> m_oldData;

	bool m_loading;
	
	int  m_tag;
	int  m_newTag;
	int  m_oldTag;

	VersionedMemento(const PYXPointer<Surface::Patch> & patch) : m_patch(patch),m_loading(false),m_tag(0),m_newTag(0),m_oldTag(0)
	{
	}

public:
	const PYXPointer<Surface::Patch> getPatch() const { return m_patch; }

	virtual bool isValid() = 0;

	int getTag() const { return m_tag; }
	bool isLoading() const { return m_loading; }

	virtual void startLoad() = 0;
	virtual void stopLoad() = 0;

	const PYXPointer<T> & getData() { return m_data; }	
	const PYXPointer<T> & getNewData() { return m_newData; }
	const PYXPointer<T> & getOldData() { return m_oldData; }
	
	virtual void refreshData()
	{
		m_oldTag = m_tag;
		m_tag = m_newTag;

		std::swap(m_oldData,m_data);
		std::swap(m_data,m_newData);
		m_newData.reset();
	}

	virtual void clearData()
	{
		m_oldData.reset();
		m_data.reset();
		m_newData.reset();
	}

	virtual void clearOldData()
	{
		m_oldData.reset();
	}
	
	//make sure the memento hold the latest data, return true if data was refreshed
	bool validate()
	{
		if (!isValid())
		{
			if (m_newData)
			{
				refreshData();
				clearOldData();
				return true;
			}
			else if (!isLoading())
			{
				startLoad();
			}
		}
		return false;
	}
};

/*!
SimpleMementoCreator - helper class to just create a T class when needed
*/
template<class T>
class SimpleMementoCreator : public MementoCreator<T>
{
public:
	static PYXPointer<SimpleMementoCreator> create()
	{
		return PYXNEW(SimpleMementoCreator);
	}

	SimpleMementoCreator()
	{
	};

public:
	virtual PYXPointer<T> createMemento(const PYXPointer<Surface::Patch> & patch)
	{
		return PYXNEW(T);
	}

	virtual void destroyMemento(const PYXPointer<Surface::Patch> & patch,PYXPointer<T> & memento)
	{
	}

	virtual PYXPointer<T> validateMemento(const PYXPointer<Surface::Patch> & patch,const PYXPointer<T> & memento)
	{
		return memento;
	}
};

/*!
WorkerThreadWithPriorities - Helper class to run tasks on a thread with dynamic priorities

Used by WorkingThreadMementoCreator
*/
template<class Key,class Comp=std::greater<Key>>
class WorkerThreadWithPriorities : public PYXObject
{
protected:
	typedef typename std::multimap<PYXPointer<Key>,boost::function<void(void)>> JobsMap;	

	mutable boost::recursive_mutex	m_mutex;
	JobsMap							m_jobs;

	boost::thread_group		  m_threads;
	boost::mutex			  m_threadMutex;
	boost::condition_variable m_hasJobsCondition;
	bool m_stop;
	int m_needWakeup;

public:
	WorkerThreadWithPriorities(int threadCount = 1)  : m_stop(false), m_needWakeup(0)
	{	
		for(int i=0;i<threadCount;i++)
		{
			m_threads.add_thread(new boost::thread(boost::bind(&WorkerThreadWithPriorities::threadFunction,this)));
		}
	}

	virtual ~WorkerThreadWithPriorities()
	{		
		if (!m_stop)
		{
			stop();
		}
	}

	void stop()
	{
		m_stop = true;
		m_hasJobsCondition.notify_all();
		m_threads.join_all();
	}

	void addJob(const PYXPointer<Key> & key,const boost::function<void(void)> & function)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		m_jobs.insert(std::make_pair(key,function));
		/*if (m_needWakeup>0)
		{
			m_hasJobsCondition.notify_one();
			m_needWakeup--;
		}*/
	}

	void addJobs(const std::vector<std::pair<PYXPointer<Key>,boost::function<void(void)>>> & jobs)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		for(auto & job : jobs)
		{
			m_jobs.insert(job);
		}
		
		unsigned int jobCount = jobs.size();

		/*while (jobCount > 0 && m_needWakeup>0)
		{
			m_hasJobsCondition.notify_one();
			m_needWakeup--;
			jobCount--;
		}*/
	}

	void cancelJob(const PYXPointer<Key> & key,const boost::function<void(void)> & function)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		JobsMap::iterator it = m_jobs.find(key);

		while(it != m_jobs.end() && it->first == patch)
		{
			if (boost::function_equal(it->second,function))
			{
				m_jobs.erase(it);
			}
			++it;
		}
	}

	void cancelJobs(const PYXPointer<Key> & key)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		JobsMap::iterator it = m_jobs.find(key);

		while(it != m_jobs.end() && it->first == key)
		{
			JobsMap::iterator safeIt = it;
			++it;
			m_jobs.erase(safeIt);
		}
	}

	void cancelAllJobs()
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		m_jobs.clear();
	}

	unsigned int getWaitingJobsCount() const
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		return m_jobs.size();
	}

protected:

	typename JobsMap::iterator findNextJob()
	{
		Comp comp;

		JobsMap::iterator it = m_jobs.begin();

		JobsMap::iterator selected = it;

		while (it != m_jobs.end())
		{
			
			if (comp(*(it->first),*(selected->first)))
			{
				selected = it;
			}
			++it;
		}

		return selected;
	}

	void threadFunction()
	{
		while (!m_stop)
		{
			boost::function<void(void)> f;
			bool foundJob = false;

			{
				boost::recursive_mutex::scoped_lock lock(m_mutex);
				JobsMap::iterator it = findNextJob();

				if (it != m_jobs.end())
				{					
					f = it->second;
					m_jobs.erase(it);
					foundJob = true;
				}
				else
				{
					//m_needWakeup++;
				}
			}

			if (foundJob)
			{
				try
				{
					f();
				}
				catch(...)
				{
					TRACE_ERROR("Exception on operating a function.");
				}
			}
			else
			{
				boost::mutex::scoped_lock lock(m_threadMutex);
				boost::system_time const timeout=boost::get_system_time() + boost::posix_time::milliseconds(100);
				m_hasJobsCondition.timed_wait(lock, timeout);	
			}
		}
	}
};

/*!
WorkingThreadMementoCreator<T> - a class that create memento on a background thread

The WorkingThreadMementoCreator<T> class uses a MementoCreator<T> to create the actual mementos, but it Queue the request and run them on a background thread.

This class is useful when the creation of the Memento<T> class can take long time. this class would return null if there is no data available until the working thread
finish to create the needed memento.

The working thread is using the Surface::Patch::getPriority to decide which Patch to load first
*/
template<class T>
class WorkingThreadMementoCreator : public MementoCreator<T>
{
public:
	typedef WorkerThreadWithPriorities<Surface::Patch,Surface::Patch::ScreenSizeCompare> WorkerThread;

	class Request : public PYXObject
	{
	public:
		PYXPointer<Surface::Patch> patch;
		PYXPointer<T> data;
		bool		  requested;

		static PYXPointer<Request> create(const PYXPointer<Surface::Patch> & patch)
		{
			return PYXNEW(Request,patch);
		}

		Request(const PYXPointer<Surface::Patch> & aPatch)
			: patch(aPatch), requested(false)
		{
		}
	};

	typedef Surface::Tree<Request> RequestedMementoTree;

protected:
	//PYXPointer<Surface> m_surface;
	PYXPointer<MementoCreator<T>> m_creator;

	boost::recursive_mutex m_mutex;
	RequestedMementoTree m_requests;

	WorkerThread m_thread;

public:
	static PYXPointer<WorkingThreadMementoCreator> create(const PYXPointer<MementoCreator<T>> creator)
	{
		return PYXNEW(WorkingThreadMementoCreator,creator);
	}

	WorkingThreadMementoCreator(const PYXPointer<MementoCreator<T>> & creator) : m_creator(creator)
	{
	}

	virtual ~WorkingThreadMementoCreator()
	{	
		m_thread.stop();
	}	

public:

	void createMementoForNode(PYXPointer<typename RequestedMementoTree::Node> node)
	{
		try
		{
			PYXPointer<Request> request;
			
			{
				boost::recursive_mutex::scoped_lock lock(m_mutex);
				request = node->getData();
			}

			if (!request)
			{
				return;
			}

			PYXPointer<T> result = m_creator->createMemento(request->patch);
			
			{
				boost::recursive_mutex::scoped_lock lock(m_mutex);

				if (!result)
				{
					//no data returned - delete the request and would re tried in the future.
					node->setData(PYXPointer<Request>());
				}
				else
				{
					request->data = result;
					request->requested = false;
				}
			}
		}
		catch(...)
		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);
			//there was an error - clear the request.
			node->setData(PYXPointer<Request>());
		}
	}

	virtual PYXPointer<T> createMemento(const PYXPointer<Surface::Patch> & patch)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		PYXPointer<RequestedMementoTree::Node> node = m_requests.createNode(patch->getKey());

		if (node->hasData())
		{
			if (node->getData()->data)
			{
				PYXPointer<T> validatedData = m_creator->validateMemento(patch,node->getData()->data);				

				//validation failed - send a request to reproduce the data...
				if (!validatedData)
				{
					if (! node->getData()->requested)
					{
						node->getData()->requested = true;
						m_thread.addJob(patch,boost::bind(&WorkingThreadMementoCreator::createMementoForNode,this,node));
					}
				}
				else
				{
					node->getData()->data = validatedData;
				}
			}
			return node->getData()->data;
		}
		else
		{
			node->setData(Request::create(patch));
			node->getData()->requested = true;
			m_thread.addJob(patch,boost::bind(&WorkingThreadMementoCreator::createMementoForNode,this,node));
			//m_hasDataToFillCondition.notify_one();
		}
		return PYXPointer<T>();
	}

	virtual void destroyMemento(const PYXPointer<Surface::Patch> & patch,PYXPointer<T> & memento)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		PYXPointer<RequestedMementoTree::Node> node = m_requests.getNode(patch->getKey());

		//if we have a node on the tree - remove it's data (but not the node itself... it's the parent job to clean after the children)
		if (node)
		{
			//remove children (if any)
			node->unify();
			//make sure the creator forget the memento too
			if (node->getData())
			{
				m_creator->destroyMemento(patch,node->getData()->data);
			}
			else
			{
				m_creator->destroyMemento(patch,PYXPointer<T>());
			}
			//forget the request...
			node->setData(PYXPointer<Request>());
		} 
		else
		{
			//make sure the creator forget the memento too
			m_creator->destroyMemento(patch,PYXPointer<T>());
		}

		//stop any pending jobs for this patch
		m_thread.cancelJobs(patch);
	}

	virtual PYXPointer<T> validateMemento(const PYXPointer<Surface::Patch> & patch,const PYXPointer<T> & oldData)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		PYXPointer<RequestedMementoTree::Node> node = m_requests.createNode(patch->getKey());

		if (node->hasData())
		{
			if (node->getData()->data)
			{
				PYXPointer<T> validatedData = m_creator->validateMemento(patch,node->getData()->data);

				//validation failed - send a request to reproduce the data...
				if (!validatedData)
				{
					if (! node->getData()->requested)
					{
						node->getData()->requested = true;
						m_thread.addJob(patch,boost::bind(&WorkingThreadMementoCreator::createMementoForNode,this,node));
					}
				}
				else
				{
					node->getData()->data = validatedData;
				}
			}
			return node->getData()->data;
		}
		return PYXPointer<T>();
	}
};


template<class T>
class CostBasedThreadedMementoCreator : public MementoCreator<VersionedMemento<T>>
{
public:
	typedef WorkerThreadWithPriorities<Surface::Patch,Surface::Patch::ScreenSizeCompare> WorkerThread;

	class MementoItem : public VersionedMemento<T>, ObjectMemoryUsageCounter<MementoItem>
	{
	protected:
		CostBasedThreadedMementoCreator & m_creator;

	public:
		static PYXPointer<MementoItem> create(CostBasedThreadedMementoCreator & creator,const PYXPointer<Surface::Patch> & patch)
		{
			return PYXNEW(MementoItem,creator,patch);
		}

		MementoItem(CostBasedThreadedMementoCreator & creator,const PYXPointer<Surface::Patch> & patch) : m_creator(creator), VersionedMemento<T>(patch)
		{
		}

	public:
		virtual bool isValid()
		{
			return m_creator.getTag() == getTag();
		}

		virtual void startLoad()
		{
			return m_creator.startLoad(this);
		}

		virtual void stopLoad()
		{
			return m_creator.stopLoad(this);
		}

	public:
		void setLoading(bool loading)
		{
			m_loading = loading;
		}

		void setNewData(const PYXPointer<T> & data,int tag)
		{
			m_newData = data;
			m_newTag = tag;
		}
	};


protected:
	boost::recursive_mutex m_mutex;
	int m_tag;
	
	WorkerThread m_fastThread;
	WorkerThread m_slowThread;

public:

	static PYXPointer<CostBasedThreadedMementoCreator> create()
	{
		return PYXNEW(CostBasedThreadedMementoCreator);
	}

	CostBasedThreadedMementoCreator(int fastThreadCount = 1,int slowThreadCount = 1) : m_tag(0), m_fastThread(fastThreadCount), m_slowThread(slowThreadCount)
	{
	}

	virtual ~CostBasedThreadedMementoCreator()
	{
		m_fastThread.stop();
		m_slowThread.stop();
	}

public:
	virtual PYXPointer<VersionedMemento<T>> createMemento(const PYXPointer<Surface::Patch> & patch)
	{
		return MementoItem::create(*this,patch);
	}

	virtual void destroyMemento(const PYXPointer<Surface::Patch> & patch,PYXPointer<VersionedMemento<T>> & memento)
	{
		MementoItem * item = dynamic_cast<MementoItem*>(memento.get());

		if (item)
		{
			stopLoad(item);
			item->clearData();
		}
	}

	virtual PYXPointer<VersionedMemento<T>> validateMemento(const PYXPointer<Surface::Patch> & patch,const PYXPointer<VersionedMemento<T>> & memento)
	{
		return memento;
	}

	void startLoad(PYXPointer<MementoItem> memento)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		if (!memento->isLoading())
		{
			memento->setLoading(true);
			//send request to the fast thread. 
			//if the willLoadFast will return true, the fast thread will load it.
			//if returned false, it will be sent to the slow thread
			m_fastThread.addJob(memento->getPatch(),boost::bind(&CostBasedThreadedMementoCreator::doLoad,this,memento,true));
		}
	}

	void stopLoad(PYXPointer<MementoItem> memento)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		
		if (memento->isLoading())
		{
			m_fastThread.cancelJobs(memento->getPatch());
			m_slowThread.cancelJobs(memento->getPatch());
			memento->setLoading(false);	
		}
	}

	void doLoad(PYXPointer<MementoItem> memento,bool fast)
	{
		try
		{
			if (fast && ! willLoadFast(memento->getPatch(),memento))
			{
				//send it to the slow thread...
				m_slowThread.addJob(memento->getPatch(),boost::bind(&CostBasedThreadedMementoCreator::doLoad,this,memento,false));
				return;
			}

			loadMemento(memento->getPatch(),memento);
		}
		catch(...)
		{
		}

		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);
			memento->setLoading(false);
		}
	}

	void bumpTag() { m_tag++; }

	int getTag() const { return m_tag; }

protected:
	virtual bool willLoadFast(const PYXPointer<Surface::Patch> & patch,PYXPointer<MementoItem> & memento)
	{
		return true;
	}

	virtual void loadMemento(const PYXPointer<Surface::Patch> & patch,PYXPointer<MementoItem> & memento)
	{
	}
};


/*!
SurfaceMemento<T> - store a object of T for each request patch on the surface tree

The SurfaceMemento<T> class is used a memory cache to store class T for each patch inside the surface.
it uses a given MementoCreator<T> to create the memento for each patch.

the SurfaceMemento API is:
1. PYXPointer<T> get(patch) - return (and try to create if there is none) a memento for the request patch
2. PYXPointer<T> getIfExists(patch) - return the memento - if there is one (not try to create one if there is none)
3. findParentWithMemento(patch) - find a parent patch that has memento.
4. prepareSubPatches(patch) - request memento for the sub patches and return true if they all have data
5. clearAllMementos(patch) - remove all data from the memento

If you want to go over all visible patches / all patches in the surface and check their memento. use SurfaceMemento::Iterator

the SurfaceMemento::Iterator has the following method:
1. getPatch() - return current patch
2. get() - return (and create if needed) the current memento
3. getIfExists() - return the current memento if have one (dont create if there is none)
4. hasData() - return true if has a memento 
5. isVisible() - return true if patch is visible
6. isDivided() - return true if patch is divided
7. hasVisiblityBlock() - return true if patch has visibility block

As an iterator, the navigation can be done by:
1. start() - move to the first patch in the tree ("00")
2. next() - move to the next patch - going over children also
3. end() - return true of finished
5. startVisible() - move to the first visible patch in the tree
6. nextVisible() - move to the next visible patch in the tree

For more general tree navigation:
1. zoomIn(index) - move into child patch
2. zoomOut() - move to parent
3. moveTo(Key) - move to a specific patch.
*/
template<class T>
class SurfaceMemento : public PYXObject
{
public:
	mutable boost::recursive_mutex m_mutex;
	typedef Surface::Tree<T> MementosTree;
	
protected:
	MementosTree m_mementos;
	PYXPointer<Surface> m_surface;
	PYXPointer<MementoCreator<T>> m_creator;


public:
	static PYXPointer<SurfaceMemento> create(const PYXPointer<Surface> & surface,const PYXPointer<MementoCreator<T>> creator)
	{
		return PYXNEW(SurfaceMemento,surface,creator);
	}

	SurfaceMemento(const PYXPointer<Surface> & surface,const PYXPointer<MementoCreator<T>> creator)
		: m_surface(surface),
		  m_creator(creator)
	{
		Surface::Visitor::visitAll(*m_surface,boost::bind(&SurfaceMemento::createEmptyNode,this,_1));

		m_surface->getBeforePatchUnifiedNotifer().attach(this,&SurfaceMemento::beforePatchUnified);
		m_surface->getAfterPatchDividedNotifer().attach(this,&SurfaceMemento::afterPatchDivided);
	}

	~SurfaceMemento()
	{
		m_surface->getBeforePatchUnifiedNotifer().detach(this,&SurfaceMemento::beforePatchUnified);
		m_surface->getAfterPatchDividedNotifer().detach(this,&SurfaceMemento::afterPatchDivided);
	}

public:
	const PYXPointer<Surface> & getSurface() 
	{
		return m_surface;
	}

public:
	PYXPointer<T> get(const PYXPointer<Surface::Patch> & patch)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		PYXPointer<MementosTree::Node> mementoNode = m_mementos.getNode(patch->getKey());
		
		//if there is no node in tree, the memento is not visible - dont even create a memento
		if (!mementoNode)
		{
			return PYXPointer<T>();
		}
		assert(patch->getKey() == mementoNode->getKey() && "keys must be the same");

		PYXPointer<T> memento = mementoNode->getData();

		if (!memento)
		{
			memento = m_creator->createMemento(patch);

			if (memento)
			{
				mementoNode->setData(memento);
			}
		}
		else
		{
			memento = m_creator->validateMemento(patch,memento);
		}

		return memento;
	}

	PYXPointer<T> getIfExists(const PYXPointer<Surface::Patch> & patch)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		PYXPointer<MementosTree::Node> mementoNode = m_mementos.getNode(patch->getKey());
		if (!mementoNode)
		{
			return PYXPointer<T>();
		}

		PYXPointer<T> memento = mementoNode->getData();

		if (memento)
		{
			memento = m_creator->validateMemento(patch,memento);
		}

		return memento;
	}

	PYXPointer<Surface::Patch> findParentWithMemento(const PYXPointer<Surface::Patch> & patch)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		PYXPointer<MementosTree::Node> mementoNode = m_mementos.getNode(patch->getKey());

		while(mementoNode && ! mementoNode->hasData())
		{
			mementoNode = mementoNode->getParent();
		}

		if (mementoNode)
		{
			return m_surface->getPatch(mementoNode->getKey());
		}
		else
		{
			return PYXPointer<Surface::Patch>();
		}
	}

	bool prepareSubPatches(const PYXPointer<Surface::Patch> & patch)
	{
		bool ready = true;

		if (patch->isDivided())
		{
			for(int i=0;i<9;i++)
			{
				if (!get(patch->getSubPatch(i)))
				{
					ready = false;
				}
			}
		}

		return ready;
	}

	void clearAllMementos()
	{
		Surface::Visitor::visitAll(*m_surface,boost::bind(&SurfaceMemento<T>::clearMemento,this,_1));
	}

protected:

	PYXPointer<T> get(const PYXPointer<Surface::Patch> & patch,const PYXPointer<typename MementosTree::Node> & mementoNode)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		PYXPointer<T> memento = mementoNode->getData();

		if (!memento)
		{
			memento = m_creator->createMemento(patch);

			if (memento)
			{
				mementoNode->setData(memento);
			}
		}
		else
		{
			memento = m_creator->validateMemento(patch,memento);
			mementoNode->setData(memento);
		}

		return memento;
	}

	PYXPointer<T> getIfExists(const PYXPointer<Surface::Patch> & patch,const PYXPointer<typename MementosTree::Node> & mementoNode)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		PYXPointer<T> memento = mementoNode->getData();

		if (memento)
		{
			memento = m_creator->validateMemento(patch,memento);
			mementoNode->setData(memento);
		}

		return memento;
	}

	void clearMemento(const PYXPointer<Surface::Patch> & patch)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		PYXPointer<MementosTree::Node> mementoNode = m_mementos.getNode(patch->getKey());

		if (mementoNode)
		{
			m_creator->destroyMemento(patch,mementoNode->getData());
			mementoNode->setData(PYXPointer<T>());
		}
		else
		{
			m_creator->destroyMemento(patch,PYXPointer<T>());
		}
	}


	void beforePatchUnified(PYXPointer<NotifierEvent> e)
	{
		Surface::Event * surfaceEvent = dynamic_cast<Surface::Event*>(e.get());

		if (surfaceEvent)
		{
			Surface::Visitor::visitAllChildren(surfaceEvent->getPatch(),boost::bind(&SurfaceMemento<T>::onPatchUnified,this,_1));

			{
				boost::recursive_mutex::scoped_lock lock(m_mutex);
				m_mementos.getNode(surfaceEvent->getPatch()->getKey())->unify();
			}
		}
	}

	void onPatchUnified(const PYXPointer<Surface::Patch> & patch)
	{
		clearMemento(patch);
	}

	void createEmptyNode(const PYXPointer<Surface::Patch> & patch)
	{
		m_mementos.createNode(patch->getKey());
	}

	void afterPatchDivided(PYXPointer<NotifierEvent> e)
	{
		Surface::Event * surfaceEvent = dynamic_cast<Surface::Event*>(e.get());

		if (surfaceEvent)
		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);
			m_mementos.getNode(surfaceEvent->getPatch()->getKey())->divide();
		}
	}

public:

	class Iterator : public PYXObject
	{
	protected:
		Surface::Patch * m_patch;
		typename MementosTree::Node * m_node;
		PYXPointer<SurfaceMemento> m_surfaceMemento;

	public:
		const Surface::Patch::Key & getKey() const { return m_patch->getKey(); }
		Surface::Patch & getPatch() const { return *m_patch; }
		PYXPointer<T> get() { return m_surfaceMemento->get(m_patch,m_node); }
		PYXPointer<T> getIfExists() { return m_surfaceMemento->getIfExists(m_patch,m_node); }
		bool hasData() const { return m_node->hasData(); }
		bool isVisible() const { return m_patch->isVisible(); }
		bool isDivided() const { return m_patch->isDivided(); }
		bool hasVisiblityBlock() const { return m_patch->hasVisiblityBlock(); }

	public:
		void zoomIn(int index) 
		{
			if (m_patch->isDivided())
			{
				m_patch = m_patch->getSubPatchPtr(index);
				m_node = m_node->getSubNodePtr(index);
			}
		}

		void zoomOut()
		{
			if (m_patch->getParentPtr() != NULL)
			{
				m_patch = m_patch->getParentPtr();
				m_node  = m_node->getParentPtr();
			}
		}

		void moveTo(const Surface::Patch::Key & key)
		{
			m_patch = m_surfaceMemento->m_surface->getPatch(key).get();
			m_node  = m_surfaceMemento->m_mementos.getNode(key).get();
		}

	public:
		Iterator(PYXPointer<SurfaceMemento> surfaceMemento) : m_surfaceMemento(surfaceMemento)
		{
			start();
		}

	public:
		void start()
		{
			Surface::Patch::Key firstKey("00");
			m_patch = m_surfaceMemento->m_surface->getPatch(firstKey).get();
			m_node = m_surfaceMemento->m_mementos.getNode(firstKey).get();
		}

		void startVisible()
		{
			start();
			if (!isVisible())
			{
				nextVisible();
			}
		}

		bool end() const { return m_patch == NULL; }

		void next() const 
		{
			if (m_patch == NULL)
			{
				return;
			}

			if (m_patch->isDivided())
			{
				zoomIn(0);
				return;
			}

			while(m_patch != NULL)
			{
				const Surface::Patch::Key & key = getKey();

				if (key.isPrimResolution())
				{
					if (key.getLastIndex()+1 < 90)
					{
						//move to your brother
						moveTo(Surface::Patch::Key(key.getLastIndex()+1));
						return;
					}
					else
					{
						//this is the end
						m_patch = NULL;
						m_node = NULL;
					}
				}
				else
				{
					if (key.getLastIndex()+1 < 9)
					{
						//move to the next brother
						zoomOut();
						zoomIn(key.getLastIndex()+1);
						return;
					}
					else
					{
						zoomOut();
						//we done with our brothers. go to the parent and let it find it's brother in the next loop
					}
				}
			}
		}

		void nextVisible()
		{
			if (m_patch == NULL)
			{
				return;
			}

			if (m_patch->isDivided() && isVisible())
			{
				zoomIn(0);
				if (isVisible())
				{
					return;
				}
			}

			while(m_patch)
			{
				const Surface::Patch::Key & key = getKey();

				if (key.isPrimResolution())
				{
					if (key.getLastIndex()+1 < 90)
					{
						//move to your brother
						moveTo(Surface::Patch::Key(key.getLastIndex()+1));
						if (isVisible())
						{
							return;
						}
						//if not visible - move to the next prim...
					}
					else
					{
						//this is the end
						m_patch = NULL;
						m_node = NULL;
						return;
					}
				}
				else
				{
					if (key.getLastIndex()+1 < 9)
					{
						//move ot the next borther
						zoomOut();
						zoomIn(key.getLastIndex()+1);

						if (isVisible())
						{
							return;
						}
					}
					else
					{
						zoomOut();
						//we done with our brothers. go to the parent and let it find it's brother in the next loop
					}
				}
			}
		}

	};

public:
	PYXPointer<Iterator> createIterator()
	{
		return PYXNEW(Iterator,this);
	}
};

#endif