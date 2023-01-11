/******************************************************************************
thread_pool.cpp

begin		: 2010-03-15
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/thread_pool.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/exception.h"


#define REDUCE_THREAD_PRIORITY_ON_WINDOWS

#ifdef REDUCE_THREAD_PRIORITY_ON_WINDOWS
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#endif 


//////////////////////////////////////////////////////////////////////////////
// TaskQueue
//////////////////////////////////////////////////////////////////////////////

class TaskQueue
{
private:
	mutable boost::mutex						 m_mutex;
	std::stack<PYXThreadPool::FuncWithThreadId*> m_tasks;
	size_t m_estimatedSize;

public:
	TaskQueue() : m_estimatedSize(0)
	{
	}

	PYXThreadPool::FuncWithThreadId * pop()
	{
		boost::mutex::scoped_lock lock(m_mutex);

		if (m_tasks.empty())
		{
			return 0;
		}

		PYXThreadPool::FuncWithThreadId * task = m_tasks.top();
		m_tasks.pop();
		m_estimatedSize--;
		return task;
	};

	void push(PYXThreadPool::FuncWithThreadId * task)
	{
		boost::mutex::scoped_lock lock(m_mutex,boost::try_to_lock);

		while(!lock)
		{
			lock.try_lock();
		}

		m_tasks.push(task);
		m_estimatedSize++;
	}

	size_t size() const
	{
		boost::mutex::scoped_lock lock(m_mutex,boost::try_to_lock);

		while(!lock)
		{
			lock.try_lock();
		}

		return m_tasks.size();
	};

	size_t estimatedSize() const 
	{
		return m_estimatedSize;
	}
};

//////////////////////////////////////////////////////////////////////////////
// PYXThreadPool static tasks queues functions
//////////////////////////////////////////////////////////////////////////////

TaskQueue s_tasksQueues[PYXThreadPool::MAX_THREADS+1];

void pushTask(int threadID, PYXThreadPool::FuncWithThreadId * task)
{
	s_tasksQueues[threadID+1].push(task);
}

PYXThreadPool::FuncWithThreadId * fetchTask(int threadID)
{
	//try to fetch from this thread queue...
	PYXThreadPool::FuncWithThreadId * task = s_tasksQueues[threadID+1].pop();

	if (task != 0)
	{
		return task;
	}

	//try to fetch from main queue
	task = s_tasksQueues[0].pop();
	if (task != 0)
	{
		return task;
	}

	//try to fetch from other threads queue...
	for(int i=1;i<PYXThreadPool::MAX_THREADS+1;i++)
	{
		task = s_tasksQueues[i].pop();

		if (task != 0)
		{
			return task;
		}
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////
// PYXThreadPool
//////////////////////////////////////////////////////////////////////////////

void PYXThreadPool::addTask(const PYXThreadPool::Func & func)
{
	addTaskWithThreadId(boost::bind(&PYXThreadPool::taskWrapper,_1,new Func(func)));
}

void PYXThreadPool::addTaskWithThreadId(const PYXThreadPool::FuncWithThreadId & func)
{
	int threadID = PYXThreadPool::getCurrentThreadId();

	pushTask(threadID,new FuncWithThreadId(func));

	if (!m_started)
	{
		//do a busy wait
		boost::mutex::scoped_lock lock(m_poolMutex,boost::try_to_lock);

		while(!lock)
			lock.try_lock();

		if (!m_started)
		{
			SYSTEM_INFO systemInfo;
			GetSystemInfo(&systemInfo);

			for(int i=0;i<MAX_THREADS;++i)
			{
				m_threadHandle[i] = 0;
			}

			size_t numberOfThreads = std::max((size_t)2,std::min((size_t)systemInfo.dwNumberOfProcessors,(size_t)MAX_THREADS));

			TRACE_INFO("ThreadPool: generating " << numberOfThreads << " threads ( found " << systemInfo.dwNumberOfProcessors << " CPUs)");

			
			while (m_workingThreads.size() < numberOfThreads)
			{
				m_workingThreads.create_thread(boost::bind(&PYXThreadPool::workingThreadFunc,(int)m_workingThreads.size()));
			}			
		}

		m_started = true;
	}

}

void PYXThreadPool::showThreadStats()
{
	for(int i=0;i<(int)m_workingThreads.size();i++)
	{
		TRACE_INFO("Thread " << i << "performed " << m_taskCount[i] << "Tasks");
		m_taskCount[i]=0;
	}
}

void PYXThreadPool::addSlowTask(const PYXThreadPool::Func & func)
{
	//create a new thread
	boost::thread slowThread(func);
}

void PYXThreadPool::shutdown()
{
	if (m_running) 
	{
		m_running = false;
		m_workingThreads.join_all();
	}
}

void PYXThreadPool::taskWrapper(int helper,PYXThreadPool::Func * func)
{
	(*func)();
	delete func;
}

void PYXThreadPool::workingThreadFunc(int threadId)
{

#ifdef REDUCE_THREAD_PRIORITY_ON_WINDOWS
	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
#endif
	m_threadHandle[threadId] = GetCurrentThreadId();

	int noTaskCount = 0;

	while(m_running)
	{
		FuncWithThreadId * func = fetchTask(threadId);

		if (func == 0)
		{
			if (noTaskCount < 10)
			{
				noTaskCount++;
				boost::thread::yield();
				continue;
			}

			if (noTaskCount < 20)
			{
				noTaskCount++;
			}

			boost::system_time const timeout=boost::get_system_time()+ boost::posix_time::milliseconds(noTaskCount - 10);
			boost::thread::sleep(timeout);
			
			continue;
		}
		noTaskCount = 0;
		m_taskCount[threadId]++;
		m_taskDepth[threadId]++;
		try
		{
			(*func)(threadId);
		}
		catch(...)
		{
		}
		m_taskDepth[threadId]--;
		delete func;
	}
}

bool PYXThreadPool::helpThreadPool(bool taskHasChildren)
{
	if (isCurrentThreadInPool())
	{
		int threadId = getCurrentThreadId();

		//stop when we are too deep.
		if (taskHasChildren)
		{
			if (m_taskDepth[threadId]>50)
			{
				return false;
			}
		}
		else
		{
			//stop when we are too deep.
			if (m_taskDepth[threadId]>25)
			{
				return false;
			}
		}

		FuncWithThreadId * func = fetchTask(threadId);

		if (func == 0)
		{
			return false;
		}

		m_taskCount[threadId]++;
		m_taskDepth[threadId]++;
		try
		{
			(*func)(threadId);
		}
		catch(...)
		{
		}
		m_taskDepth[threadId]--;
		delete func;
		return true;
	}
	else 
	{
		return false;
	}
}


bool PYXThreadPool::isCurrentThreadInPool()
{
	return getCurrentThreadId() != -1;
}

int PYXThreadPool::getCurrentThreadId()
{
	DWORD handle = GetCurrentThreadId();
	for(int i=0;i<MAX_THREADS;++i)
	{
		if (m_threadHandle[i]==handle)
		{
			return i;
		}
	}
	return -1;
}

bool						PYXThreadPool::m_started = false;
bool						PYXThreadPool::m_running = true;
boost::thread_group			PYXThreadPool::m_workingThreads;
boost::mutex				PYXThreadPool::m_poolMutex;

int PYXThreadPool::m_taskCount[PYXThreadPool::MAX_THREADS];
int PYXThreadPool::m_taskDepth[PYXThreadPool::MAX_THREADS];
unsigned long PYXThreadPool::m_threadHandle[MAX_THREADS];


//////////////////////////////////////////////////////////////////////////////
// PYXTaskGroup
//////////////////////////////////////////////////////////////////////////////

void PYXTaskGroup::addTask(const boost::function<void()> & func)
{
	++m_taskCount;
	PYXThreadPool::addTask(boost::bind(&PYXTaskGroup::doTask,this,func));
}

void PYXTaskGroup::addTaskWithThreadId(const boost::function<void(int)> & func)
{
	++m_taskCount;
	PYXThreadPool::addTaskWithThreadId(boost::bind(&PYXTaskGroup::doTaskWithThreadId,this,func,_1));
}

void PYXTaskGroup::addSlowTask(const boost::function<void()> & func)
{
	++m_taskCount;
	PYXThreadPool::addSlowTask(boost::bind(&PYXTaskGroup::doTask,this,func));
}

PYXTaskGroup::PYXTaskGroup() : m_taskCount(0)
{
	m_error = false;
}

PYXTaskGroup::~PYXTaskGroup()
{
	joinAll(false);
}

void PYXTaskGroup::joinAll(bool throwOnError)
{
	//TRACE_INFO(" completed " << m_taskCompleted << " of " << m_taskCount);

	while(m_taskCount>0)
	{
		//TRACE_INFO(" completed " << m_taskCompleted << " of " << m_taskCount);
		boost::system_time const timeout=boost::get_system_time()+ boost::posix_time::milliseconds(100);

		//try to do help the thread pool if possible...
		if (!PYXThreadPool::helpThreadPool(true))
		{
			boost::mutex::scoped_lock lock(m_groupMutex);
			m_taskCompletedCondition.timed_wait(lock,timeout);	
		}
	}

	if (m_error && throwOnError)
	{
		PYXTHROW(PYXException,m_errorString);
	}
	//TRACE_INFO(" completed " << m_taskCompleted << " of " << m_taskCount);
}

void PYXTaskGroup::doTask(boost::function<void()> func)
{
	try
	{
		func();
	}
	catch(PYXException & e)
	{
		boost::mutex::scoped_lock lock(m_groupMutex);
		m_errorString += e.getFullErrorString() + "\n";
		m_error = true;
	}
	catch(std::exception & stdex)
	{
		boost::mutex::scoped_lock lock(m_groupMutex);
		m_errorString += std::string(stdex.what()) + "\n";
		m_error = true;
	}
	catch(...)
	{
		boost::mutex::scoped_lock lock(m_groupMutex);
		m_errorString += "Unknown error.\n";
		m_error = true;
	}

	--m_taskCount;

	if (m_taskCount==0)
	{
		boost::mutex::scoped_lock lock(m_groupMutex);
		m_taskCompletedCondition.notify_all();
	}
}

void PYXTaskGroup::doTaskWithThreadId(boost::function<void(int)> func,int threadId)
{
	try
	{
		func(threadId);
	}
	catch(PYXException & e)
	{
		boost::mutex::scoped_lock lock(m_groupMutex);
		m_errorString += e.getFullErrorString() + "\n";
		m_error = true;
	}
	catch(std::exception & stdex)
	{
		boost::mutex::scoped_lock lock(m_groupMutex);
		m_errorString += std::string(stdex.what()) + "\n";
		m_error = true;
	}
	catch(...)
	{
		boost::mutex::scoped_lock lock(m_groupMutex);
		m_errorString += "Unknown error.\n";
		m_error = true;
	}

	--m_taskCount;

	if (m_taskCount==0)
	{
		boost::mutex::scoped_lock lock(m_groupMutex);
		m_taskCompletedCondition.notify_all();
	}
}

//////////////////////////////////////////////////////////////////////////////
// PYXTaskWithContinuation
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////// 
// STATIC MEMBERS
//////////////////////////////// 

//static pointer to current running tasks...
PYXPointer<PYXTaskWithContinuation> PYXTaskWithContinuation::m_currentTasks[PYXThreadPool::MAX_THREADS];

//////////////////////////////// 
// STATIC CTORs
//////////////////////////////// 

PYXPointer<PYXTaskWithContinuation> PYXTaskWithContinuation::create()
{
	PYXPointer<PYXTaskWithContinuation> task = PYXNEW(PYXTaskWithContinuation,boost::bind(&PYXTaskWithContinuation::emptyFunc));
	return task; 
}

PYXPointer<PYXTaskWithContinuation> PYXTaskWithContinuation::create(const boost::function<void()> & func)
{
	PYXPointer<PYXTaskWithContinuation> task = PYXNEW(PYXTaskWithContinuation,func);
	return task; 
}

PYXPointer<PYXTaskWithContinuation> PYXTaskWithContinuation::start(const boost::function<void()> & func)
{
	PYXPointer<PYXTaskWithContinuation> task = PYXNEW(PYXTaskWithContinuation,func);
	task->start();
	return task; 
}

PYXPointer<PYXTaskWithContinuation> PYXTaskWithContinuation::startAsChild(const boost::function<void()> & func)
{
	if (PYXThreadPool::isCurrentThreadInPool())
	{
		PYXPointer<PYXTaskWithContinuation> task = m_currentTasks[PYXThreadPool::getCurrentThreadId()];
		if (task)
		{
			return task->startChild(func);
		}
	}
	
	return start(func);
}

PYXTaskWithContinuation::PYXTaskWithContinuation() 
	: m_func(), m_started(false), m_funcCompleted(false), m_childCount(0), m_childCompleted(0), m_parent(0)
{
}

PYXTaskWithContinuation::PYXTaskWithContinuation(const boost::function<void()> & func) 
	: m_func(func), m_started(false), m_funcCompleted(false), m_childCount(0), m_childCompleted(0), m_parent(0)
{
}

void PYXTaskWithContinuation::doTask(PYXPointer<PYXTaskWithContinuation> task,int threadID)
{
	//TRACE_INFO("start task func " << (int)task.get() << " on thread " << threadID);
	PYXPointer<PYXTaskWithContinuation> oldTask = m_currentTasks[threadID];
	m_currentTasks[threadID] = task;
	try
	{
		task->m_func();
	}
	catch(PYXTaskCanceledException&)
	{
		task->setCanceled();
	}
	catch(PYXException & e)
	{
		task->setCompletedWithError(e.getFullErrorString());
		if (task->m_parent)
		{
			task->m_parent->setCompletedWithError(e.getFullErrorString());
		}
	}
	catch(std::exception & stdex)
	{
		task->setCompletedWithError(stdex.what());
		if (task->m_parent)
		{
			task->m_parent->setCompletedWithError(stdex.what());
		}		
	}
	catch(...)
	{
		task->setCompletedWithError("Unknown error.");
		if (task->m_parent)
		{
			task->m_parent->setCompletedWithError("Unknown error.");
		}
	}

	{
		boost::mutex::scoped_lock lock(task->m_taskMutex);
		task->m_funcCompleted = true;
	}

	m_currentTasks[threadID] = oldTask;

	//TRACE_INFO("end task func " << (int)task.get() << " on thread " << threadID);

	task->setCompleted();
}

void PYXTaskWithContinuation::start()
{
	boost::mutex::scoped_lock lock(m_taskMutex);
	{
		if (m_started)
		{
			PYXTHROW(PYXException,"Can't start a task twice");
		}
		else
		{
			sendTaskToThreadPool();
		}
	}
}

void PYXTaskWithContinuation::sendTaskToThreadPool()
{
	if (m_started == false)
	{
		m_started = true;
		PYXThreadPool::addTaskWithThreadId(boost::bind(&PYXTaskWithContinuation::doTask,PYXPointer<PYXTaskWithContinuation>(this),_1));
	}
}

PYXPointer<PYXTaskWithContinuation> PYXTaskWithContinuation::startChild(const boost::function<void()> & func)
{
	PYXPointer<PYXTaskWithContinuation> child = PYXTaskWithContinuation::create(func);

	addChild(child);
	child->start();
	return child;
}

void PYXTaskWithContinuation::addChild(const PYXPointer<PYXTaskWithContinuation> & childTask)
{
	if (m_funcCompleted == true)
	{
		PYXTHROW(PYXException,"Can't add child task to a completed task");
	}

	childTask->m_parent = this;
	++m_childCount;
	setHasChildTasks(true);
	childTask->onTaskCompleted(boost::bind(&PYXTaskWithContinuation::childCompeleted,PYXPointer<PYXTaskWithContinuation>(this)));
}

void PYXTaskWithContinuation::childCompeleted(PYXPointer<PYXTaskWithContinuation> task)
{
	++(task->m_childCompleted);

	//TRACE_INFO("child of task " << (int)task.get() << " was competed (" << task->m_childCompleted << " or " << task->m_childCount << " tasks)");
	task->setCompleted();
}

void PYXTaskWithContinuation::setCompleted()
{
	bool wasFullyCompleted = false;
	{
		boost::mutex::scoped_lock lock(m_taskMutex);

		if (isCompleted())
			return;

		if (m_funcCompleted && m_childCompleted == m_childCount)
		{
			wasFullyCompleted = true;
		}
	}

	if (wasFullyCompleted)
	{
		PYXTaskSource::setCompleted();
	}
}


////////////////////////////////////////////////////////////////////////////////
// PYXTaskCancelationSource
////////////////////////////////////////////////////////////////////////////////

PYXTaskCancelationSource::PYXTaskCancelationSource() : m_canceled(false)
{
}

void PYXTaskCancelationSource::setCanceled()
{
	m_canceled = true;
	m_notifier.notify(0);
}

bool PYXTaskCancelationSource::wasCanceled() const
{
	return m_canceled;
}

////////////////////////////////////////////////////////////////////////////////
// PYXTaskCancelationToken
////////////////////////////////////////////////////////////////////////////////

PYXTaskCancelationToken::PYXTaskCancelationToken()
{
}

PYXTaskCancelationToken::PYXTaskCancelationToken(const PYXTaskCancelationToken & other) : m_source(other.m_source)
{
}

PYXTaskCancelationToken::PYXTaskCancelationToken(const PYXPointer<PYXTaskCancelationSource> & cancelSource) : m_source(cancelSource)
{
}

PYXTaskCancelationToken & PYXTaskCancelationToken::operator=(const PYXTaskCancelationToken & other)
{
	m_source = other.m_source;
	return *this;
}

bool PYXTaskCancelationToken::wasCanceled() const
{
	return m_source && m_source->wasCanceled();
}

void PYXTaskCancelationToken::throwIfCanceled() const
{
	if (m_source && m_source->wasCanceled())
	{
		PYXTHROW(PYXTaskCanceledException,"Task been canceled");
	}
}

////////////////////////////////////////////////////////////////////////////////
// PYXTaskDependentCancelationSource
////////////////////////////////////////////////////////////////////////////////

PYXPointer<PYXTaskCancelationSource> PYXTaskDependentCancelationSource::create(const PYXPointer<PYXTaskDependentCancelationSource> & source)
{
	if (source)
	{
		return PYXNEW(PYXTaskDependentCancelationSource,source);
	}
	else 
	{
		return PYXNEW(PYXTaskCancelationSource);
	}
}

PYXPointer<PYXTaskCancelationSource> PYXTaskDependentCancelationSource::create(const PYXTaskCancelationToken & token)
{
	if (token.m_source)
	{
		return PYXNEW(PYXTaskDependentCancelationSource,token.m_source);
	}
	else 
	{
		return PYXNEW(PYXTaskCancelationSource);
	}
}

PYXTaskDependentCancelationSource::PYXTaskDependentCancelationSource(const PYXPointer<PYXTaskCancelationSource> & source) : m_source(source)
{
	if (m_source)
	{
		m_source->getOnCanceledNotifier().attach(this,&PYXTaskDependentCancelationSource::onParentCanceled);

		//make sure we didn't miss the canceled event...
		if (!this->wasCanceled() && m_source->wasCanceled())
		{
			setCanceled();
		}
	}
}

PYXTaskDependentCancelationSource::~PYXTaskDependentCancelationSource()
{
	if (m_source)
	{
		m_source->getOnCanceledNotifier().detach(this,&PYXTaskDependentCancelationSource::onParentCanceled);
	}
}

void PYXTaskDependentCancelationSource::onParentCanceled(PYXPointer<NotifierEvent> spEvent)
{
	setCanceled();
}

////////////////////////////////////////////////////////////////////////////////
// PYXTaskSource
////////////////////////////////////////////////////////////////////////////////

PYXTaskSource::PYXTaskSource() : m_error(false), m_canceled(false), m_completed(false), m_hasChildTasks(false)
{
}

PYXTaskSource::~PYXTaskSource()
{
}

void PYXTaskSource::setCompleted()
{
	std::vector<PYXPointer<PYXTaskWithContinuation>> tasksToRun;
	{
		boost::mutex::scoped_lock lock(m_taskMutex);

		if (m_completed)
		{
			return;
		}

		m_completed = true;
		std::swap(tasksToRun,m_tasksToRun);
		m_taskCompletedCondition.notify_all();
	}

	for(std::vector<PYXPointer<PYXTaskWithContinuation>>::iterator it = tasksToRun.begin();it!= tasksToRun.end();++it)
	{
		(*it)->start();
	}	
}

void PYXTaskSource::setCompletedWithError(const std::string & error)
{
	{
		boost::mutex::scoped_lock lock(m_taskMutex);

		if (!m_completed)
		{
			m_error = true;
			m_errorString += error + "\n";
		}
	}

	setCompleted();
}

void PYXTaskSource::setHasChildTasks(bool value)
{
	m_hasChildTasks = value;
}

bool PYXTaskSource::isCompleted() const
{
	return m_completed;
}

bool PYXTaskSource::isCanceled() const
{
	return m_canceled;
}

void PYXTaskSource::setCanceled()
{
	{
		boost::mutex::scoped_lock lock(m_taskMutex);
		if (!m_completed)
		{
			m_canceled = true;
		}
	}

	setCompleted();
}

void PYXTaskSource::join() const
{
	while(!m_completed)
	{
		boost::system_time const timeout=boost::get_system_time()+ boost::posix_time::milliseconds(100);

		if (!PYXThreadPool::helpThreadPool(m_hasChildTasks))
		{
			boost::mutex::scoped_lock lock(m_taskMutex);
			m_taskCompletedCondition.timed_wait(lock,timeout);
		}
	}

	if (m_canceled)
	{
		PYXTHROW(PYXTaskCanceledException,"Task been canceled");
	}

	if (m_error)
	{
		PYXTHROW(PYXException,m_errorString);
	}
}

PYXPointer<PYXTaskSource> PYXTaskSource::onTaskCompleted(const boost::function<void()> & func)
{
	boost::mutex::scoped_lock lock(m_taskMutex);
	if (m_completed)
	{
		return PYXTaskWithContinuation::start(func);
	}
	else 
	{
		PYXPointer<PYXTaskWithContinuation> task = PYXTaskWithContinuation::create(func);
		m_tasksToRun.push_back(task);
		return task;
	}
}


class PYXTaskTester
{
public:
	static int add(int a,int b)
	{
		return a+b;
	}
	static int add2(PYXPointer<PYXTaskWithResult<int>> task,int b)
	{
		return task->getResult()+b;
	}

	static void inc(int & counter)
	{
		counter++;
	}

	static void throwError()
	{
		PYXTHROW(PYXException,"Error message");
	}

	static void waitForCancel(bool & completed, PYXTaskCancelationToken token)
	{
		while(!token.wasCanceled()) {
			boost::system_time const timeout=boost::get_system_time()+ boost::posix_time::milliseconds(1);
			boost::thread::sleep(timeout);
		}
		completed = true;
	}

	static void waitForCancelAndThrow(bool & completed, PYXTaskCancelationToken token)
	{
		while(true) {
			token.throwIfCanceled();
			boost::system_time const timeout=boost::get_system_time()+ boost::posix_time::milliseconds(1);
			boost::thread::sleep(timeout);
		}
		completed = true;
	}

	static void childStartNoWait(int depth,int & counter)
	{
		if (depth>0)
		{
			PYXTaskWithContinuation::startAsChild(boost::bind(&PYXTaskTester::childStartNoWait,depth-1,boost::ref(counter)));
		}
		else
		{
			boost::system_time const timeout=boost::get_system_time()+ boost::posix_time::milliseconds(1);
			boost::thread::sleep(timeout);
			counter++;
		}
	}
	
	static void fibonacci(int n, int & result)
	{
		if (n<=0)
		{
			result = 0;
			return;
		}

		if (n==1)
		{
			result = 1;
			return;
		}

		int fibonacci_n1;
		
		PYXPointer<PYXTaskWithContinuation> n1 = PYXTaskWithContinuation::startAsChild(boost::bind(PYXTaskTester::fibonacci,n-1,boost::ref(fibonacci_n1)));

		int fibonacci_n2;
		fibonacci(n-2,fibonacci_n2);

		n1->join();
		result = fibonacci_n2 + fibonacci_n1;
	}

	static void test()
	{
		{
			//Testing PYXTaskWithResult<int> api.
			PYXPointer<PYXTaskWithResult<int>> addTask = PYXTaskWithResult<int>::start(boost::bind(&PYXTaskTester::add,4,5));

			PYXPointer<PYXTaskWithResult<int>> addTask2 = addTask->onTaskCompleted<int>(boost::bind(&PYXTaskTester::add2,addTask,5));
		
			TEST_ASSERT(addTask2->getResult()==14);
			TEST_ASSERT(addTask->getResult()==9);		
		}

		{
			//Testing PYXTaskGroup joinAll:		
			PYXTaskGroup group;
			int counter = 0;

			group.addTask(boost::bind(&PYXTaskTester::inc,boost::ref(counter)));
			group.addTask(boost::bind(&PYXTaskTester::inc,boost::ref(counter)));
			group.addTask(boost::bind(&PYXTaskTester::inc,boost::ref(counter)));
			group.addSlowTask(boost::bind(&PYXTaskTester::inc,boost::ref(counter)));

			group.joinAll();

			TEST_ASSERT(counter==4);
		}

		{
			//Testing PYXTaskGroup joinAll with exception:		
			PYXTaskGroup group;
			int counter = 0;

			group.addTask(boost::bind(&PYXTaskTester::inc,boost::ref(counter)));
			group.addTask(boost::bind(&PYXTaskTester::inc,boost::ref(counter)));
			group.addTask(boost::bind(&PYXTaskTester::throwError));
			//sleep to make sure the throwError has completed
			boost::system_time const timeout=boost::get_system_time()+ boost::posix_time::milliseconds(10);
			boost::thread::sleep(timeout);
			group.addTask(boost::bind(&PYXTaskTester::inc,boost::ref(counter)));
			group.addTask(boost::bind(&PYXTaskTester::inc,boost::ref(counter)));

			//joinAll should join until all tasks have completed + thorw an exception if one of the tasks has failed
			TEST_ASSERT_EXCEPTION(group.joinAll(),PYXException);			
			//check that all other tasks has been completed
			TEST_ASSERT_EQUAL(counter,4);
		}

		{
			//Testing PYXTaskWithContinuation startChild api:
			int counter = 0;
			
			PYXPointer<PYXTaskWithContinuation> complexTask = PYXTaskWithContinuation::start(boost::bind(&PYXTaskTester::childStartNoWait,4,boost::ref(counter)));

			//this should wait for all child tasks to complete..
			complexTask->join();

			TEST_ASSERT(counter == 1);
		}

		{
			//Testing PYXTaskWithContinuation startChild + join  api:
			int fibonacci10 = 0;
			
			PYXPointer<PYXTaskWithContinuation> fibonacciTask = PYXTaskWithContinuation::start(boost::bind(&PYXTaskTester::fibonacci,10,boost::ref(fibonacci10)));

			fibonacciTask->join();

			TEST_ASSERT(fibonacci10 == 55);
		}

		{
			//Testing PYXTaskCancelationSource api
			PYXPointer<PYXTaskCancelationSource> cancelationSource = PYXTaskCancelationSource::create();

			bool completed = false;
			PYXPointer<PYXTaskWithContinuation> task = PYXTaskWithContinuation::start(boost::bind(&PYXTaskTester::waitForCancel,boost::ref(completed),PYXTaskCancelationToken(cancelationSource)));

			TEST_ASSERT(completed == false);

			//sleep to make sure the completed is still false
			boost::system_time const timeout=boost::get_system_time()+ boost::posix_time::milliseconds(10);
			boost::thread::sleep(timeout);

			TEST_ASSERT(completed == false);

			cancelationSource->setCanceled();

			task->join();

			TEST_ASSERT(completed == true);
		}

		{
			//Testing PYXTaskCancelationSource api
			PYXPointer<PYXTaskCancelationSource> cancelationSource = PYXTaskCancelationSource::create();

			bool completed = false;
			PYXPointer<PYXTaskWithContinuation> task = PYXTaskWithContinuation::start(boost::bind(&PYXTaskTester::waitForCancelAndThrow,boost::ref(completed),PYXTaskCancelationToken(cancelationSource)));

			TEST_ASSERT(completed == false);

			//sleep to make sure the completed is still false
			boost::system_time const timeout=boost::get_system_time()+ boost::posix_time::milliseconds(10);
			boost::thread::sleep(timeout);

			TEST_ASSERT(completed == false);

			cancelationSource->setCanceled();

			TEST_ASSERT_EXCEPTION(task->join(),PYXTaskCanceledException);
			
			//we never get to the completion point as the task was canceled.
			TEST_ASSERT(completed == false);
		}
	}
};

Tester<PYXTaskTester> gTester;