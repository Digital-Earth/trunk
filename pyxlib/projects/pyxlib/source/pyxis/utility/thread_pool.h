#ifndef PYXIS__UTILITY__THREAD_POOL_H
#define PYXIS__UTILITY__THREAD_POOL_H
/******************************************************************************
thread_pool.h

begin		: 2010-03-15
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/utility/object.h"
#include "pyxis/utility/notifier.h"

// boost includes
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

// std includes
#include <queue>
#include <stack>

//////////////////////////////////////////////////////////////////////////////
// PYXThreadPool
//////////////////////////////////////////////////////////////////////////////

class PYXLIB_DECL PYXThreadPool
{
public:
	typedef boost::function<void()> Func;
	typedef boost::function<void(int)> FuncWithThreadId;

	//! run a short task in the thread pool. functions signature is void()
	static void addTask(const Func & func);	

	//! run a long waiting task in the thread pool. functions signature is void()
	static void addSlowTask(const Func & func);

	//! run a short task in the thread pooks.function signature is void(int) - int is the thread ID.
	/*!
	this function allow better thread-safe context managmenet. 
	For example: Map-Reduce, if the map function can use the thtreadID number to access it own private storage
	*/
	static void addTaskWithThreadId(const FuncWithThreadId & func);

	//! trace thread pool statistics and reset them.
	static void showThreadStats();

	//! stop the thread pool
	static void shutdown();	

	//! check if current thread is inside one of the thread running fast functions (not slowtask).
	static bool isCurrentThreadInPool();

	//! return the current thread id: 0...MAX_THREADS-1
	static int getCurrentThreadId();

	//! helps the thread pool to execture tasks for a short while (useful for join.task)
	static bool helpThreadPool(bool taskHasChildren);

private:
	static void taskWrapper(int helper,boost::function<void()> * func);	
	static void workingThreadFunc(int threadId);

public:
	static const int MAX_THREADS = 8;

private:
	static boost::thread_group			m_workingThreads;
	static boost::mutex					m_poolMutex;
	static bool m_started;
	static bool m_running;
	static int m_taskCount[MAX_THREADS];
	static int m_taskDepth[MAX_THREADS];
	static unsigned long m_threadHandle[MAX_THREADS];
};

//////////////////////////////////////////////////////////////////////////////
// PYXTaskGroup
//////////////////////////////////////////////////////////////////////////////

class PYXLIB_DECL PYXTaskGroup
{
public:
	//! run a short task in the thread pool. functions signature is void()
	void addTask(const boost::function<void()> & func);
	//! run a long waiting task in the thread pool. functions signature is void()
	void addTaskWithThreadId(const boost::function<void(int)> & func);
	//! run a long waiting task in the thread pool. functions signature is void()
	void addSlowTask(const boost::function<void()> & func);

	PYXTaskGroup();
	virtual ~PYXTaskGroup();

	//! waits for all tasks inside the task group to complete.
	//Throws if one of the sub task thrown an exception
	void joinAll(bool throwOnError = true);

private:
	void doTask(boost::function<void()> func);
	void doTaskWithThreadId(boost::function<void(int)> func,int threadId);

private:
	bool m_error;
	std::string m_errorString;

	boost::detail::atomic_count m_taskCount;

	boost::mutex				m_groupMutex;
	boost::condition_variable	m_taskCompletedCondition;
};

//////////////////////////////////////////////////////////////////////////////
// PYXTaskGroupWithLocalStorage
//////////////////////////////////////////////////////////////////////////////

template<class T>
class PYXTaskGroupWithLocalStorage 
{
public:	
	typedef typename boost::function<void(T & storage)> Func;

	PYXTaskGroupWithLocalStorage() : m_localStorage(PYXThreadPool::MAX_THREADS)
	{
	}

	void addTask(const Func & func)
	{
		m_tasks.addTaskWithThreadId(boost::bind(&PYXTaskGroupWithLocalStorage::doTask,this,_1,new Func(func)));
	}

	void joinAll()
	{
		m_tasks.joinAll();
	}

	void initLocalStorage(const Func & func)
	{
		for(int i=0;i<(int)m_localStorage.size();++i)
		{
			func(m_localStorage[i]);
		}
	}

	T & getLocalStorage(int index)
	{
		return m_localStorage[index];
	}

	int getLocalStorageCount()
	{
		return (int)m_localStorage.size();
	}

private:
	void doTask(int threadId,Func * func)
	{
		std::auto_ptr<Func> safeFunc(func);

		(*safeFunc)(m_localStorage[threadId]);
	}

private:
	PYXTaskGroup m_tasks;
	std::vector<T> m_localStorage;
};

////////////////////////////////////////////////////////////////////////////////
// PYXTaskCanceledException
////////////////////////////////////////////////////////////////////////////////

class PYXLIB_DECL PYXTaskCanceledException : public PYXException
{
public:
	PYXTaskCanceledException(const std::string & message) : PYXException(message)
	{
	}

	virtual ~PYXTaskCanceledException()
	{
	}
};

////////////////////////////////////////////////////////////////////////////////
// PYXTaskCancelationToken & PYXTaskCancelationSource
////////////////////////////////////////////////////////////////////////////////

class PYXLIB_DECL PYXTaskCancelationSource : public PYXObject
{
public:
	static PYXPointer<PYXTaskCancelationSource> create()
	{
		return PYXNEW(PYXTaskCancelationSource);
	}

	PYXTaskCancelationSource();

private: //don't allow to copy...
	PYXTaskCancelationSource(const PYXTaskCancelationSource & other);
	PYXTaskCancelationSource & operator=(const PYXTaskCancelationSource & other);

public:
	virtual void setCanceled();
	virtual bool wasCanceled() const;

public:
	Notifier & getOnCanceledNotifier() 
	{
		return m_notifier;
	}

private:
	bool m_canceled;
	Notifier m_notifier;
};

class PYXLIB_DECL PYXTaskCancelationToken 
{
public:
	PYXTaskCancelationToken();
	PYXTaskCancelationToken(const PYXTaskCancelationToken & other);
	PYXTaskCancelationToken(const PYXPointer<PYXTaskCancelationSource> & cancelSource);

public:
	PYXTaskCancelationToken & operator=(const PYXTaskCancelationToken & other);

public:
	bool wasCanceled() const;
	void throwIfCanceled() const;

private:
	PYXPointer<PYXTaskCancelationSource> m_source;

private:
	friend class PYXTaskDependentCancelationSource;
};

class PYXLIB_DECL PYXTaskDependentCancelationSource : public PYXTaskCancelationSource
{
public:
	static PYXPointer<PYXTaskCancelationSource> create(const PYXPointer<PYXTaskDependentCancelationSource> & source);
	static PYXPointer<PYXTaskCancelationSource> create(const PYXTaskCancelationToken & token);

	PYXTaskDependentCancelationSource(const PYXPointer<PYXTaskCancelationSource> & source);
	virtual ~PYXTaskDependentCancelationSource();

private:
	void onParentCanceled(PYXPointer<NotifierEvent> spEvent);

private:
	PYXPointer<PYXTaskCancelationSource> m_source;
};

////////////////////////////////////////////////////////////////////////////////
// PYXTaskSource
////////////////////////////////////////////////////////////////////////////////

class PYXTaskWithContinuation;
template<typename T> class PYXTaskWithResult;

class PYXLIB_DECL PYXTaskSource : public PYXObject
{
public:
	static PYXPointer<PYXTaskSource> create()
	{
		return PYXNEW(PYXTaskSource);
	}

	PYXTaskSource();
	virtual ~PYXTaskSource();

protected:
	//set the task to be completed.
	virtual void setCompleted();

	virtual void setCanceled();	
	virtual void setCompletedWithError(const std::string & error);

	virtual void setHasChildTasks(bool value);

public:
	virtual bool isCompleted() const;
	virtual bool isCanceled() const;

	//! waits for all tasks inside the task to complete.
	//Throws if task ended with an error or was canceled
	virtual void join() const;

	//! run a short task in the thread pool once the task completed to run.
	//Use this instead of join on a task. this allow the current task to simple run the next task without the need to do join and stuff
	virtual PYXPointer<PYXTaskSource> onTaskCompleted(const boost::function<void()> & func);

	template<typename T>
	PYXPointer<PYXTaskWithResult<T>> onTaskCompleted(const boost::function<T()> & func)
	{
		boost::mutex::scoped_lock lock(m_taskMutex);
		if (m_completed)
		{
			return PYXTaskWithResult<T>::start(func);
		}
		else 
		{
			PYXPointer<PYXTaskWithResult<T>> task = PYXTaskWithResult<T>::create(func);
			m_tasksToRun.push_back(task);
			return task;
		}
	}

private:
	bool m_hasChildTasks;
	bool m_completed;
	bool m_canceled;
	std::vector<PYXPointer<PYXTaskWithContinuation>> m_tasksToRun;

	bool m_error;
	std::string m_errorString;

protected:
	mutable boost::mutex				m_taskMutex;
	mutable boost::condition_variable	m_taskCompletedCondition;
};


////////////////////////////////////////////////////////////////////////////////
// PYXTaskSourceWithResult<T>
////////////////////////////////////////////////////////////////////////////////

template<typename T>
class PYXLIB_DECL PYXTaskSourceWithResult : public PYXTaskSource
{
public:
	static PYXPointer<PYXTaskSourceWithResult> create()
	{
		return PYXNEW(PYXTaskSourceWithResult);
	}

	PYXTaskSourceWithResult()
	{
	}

protected:
	T & getResultInternal()
	{
		return m_result;
	}

public:
	T & getResult() 
	{
		join();
		return m_result;
	}

private:
	T m_result;
};

////////////////////////////////////////////////////////////////////////////////
// PYXTaskWithContinuation
////////////////////////////////////////////////////////////////////////////////

class PYXLIB_DECL PYXTaskWithContinuation : public PYXTaskSource
{
public:
	//create a new empty task - use this to generate a group of tasks. 
	//you can use task->startChild to add sub tasks and wait for all sub tasks to complete
	static PYXPointer<PYXTaskWithContinuation> create();

	//create a new task with a given function
	//note, this task will not start until you call task->start
	static PYXPointer<PYXTaskWithContinuation> create(const boost::function<void()> & func);

	//create a new task with a given function start to run it on the threa pool
	static PYXPointer<PYXTaskWithContinuation> start(const boost::function<void()> & func);

	//! start a task as child of the current running stack.
	//Use this to make sure the current running is not completed until this new task finish
	static PYXPointer<PYXTaskWithContinuation> startAsChild(const boost::function<void()> & func);

protected:
	PYXTaskWithContinuation();
	PYXTaskWithContinuation(const boost::function<void()> & func);
	static void emptyFunc() {}

public:
	virtual ~PYXTaskWithContinuation() {}


protected:
	virtual void setCompleted();

public:
	void start();
	PYXPointer<PYXTaskWithContinuation> startChild(const boost::function<void()> & func);

protected:
	void addChild(const PYXPointer<PYXTaskWithContinuation> & childTask);


protected:
	void sendTaskToThreadPool();
	static void doTask(PYXPointer<PYXTaskWithContinuation> task,int threadID);

	static void childCompeleted(PYXPointer<PYXTaskWithContinuation> task);

protected:
	boost::function<void()> m_func;

private:
	PYXTaskWithContinuation * m_parent;

	bool m_started;
	bool m_funcCompleted;

	boost::detail::atomic_count m_childCount;
	boost::detail::atomic_count m_childCompleted;

	static PYXPointer<PYXTaskWithContinuation> m_currentTasks[PYXThreadPool::MAX_THREADS];
};

////////////////////////////////////////////////////////////////////////////////
// PYXTaskWithResult
////////////////////////////////////////////////////////////////////////////////

template<typename T>
class PYXTaskWithResult : public PYXTaskWithContinuation
{
public:

	//create a new task with a given function
	//note, this task will not start until you call task->start
	static PYXPointer<PYXTaskWithResult> create(const boost::function<T()> & func)
	{
		return PYXNEW(PYXTaskWithResult,func);
	}	

	//create a new task with a given function start to run it on the threa pool
	static PYXPointer<PYXTaskWithResult> start(const boost::function<T()> & func)
	{
		PYXPointer<PYXTaskWithResult> task = PYXNEW(PYXTaskWithResult,func);
		PYXPointer<PYXTaskWithContinuation> baseTask = task;
		baseTask ->start();
		return task;
	}

	//! start a task as child of the current running stack.
	//Use this to make sure the current running is not completed until this new task finish
	static PYXPointer<PYXTaskWithResult> startAsChild(const boost::function<T()> & func)
	{
		if (PYXThreadPool::isCurrentThreadInPool())
		{
			PYXPointer<PYXTaskWithContinuation> task = m_currentTasks[PYXThreadPool::getCurrentThreadId()];
			if (task)
			{
				PYXPointer<PYXTaskWithResult> childTask = PYXTaskWithResult::create(func);
				task->addChild(childTask);
				return childTask;
			}
		}

		return PYXTaskWithResult::start(func);
	}

protected:
	PYXTaskWithResult(const boost::function<T()> & func) : PYXTaskWithContinuation()
	{
		m_func = boost::bind(&PYXTaskWithResult::doTaskWithResult,this,func);
	}

	void doTaskWithResult(boost::function<T()> func)
	{
		m_result = func();
	}

public:
	const T & getResult() const
	{
		join();
		return m_result;
	}

private:
	T m_result;
};


#endif

