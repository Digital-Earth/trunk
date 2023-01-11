/******************************************************************************
sync_context.cpp

begin		: 2009-11-01
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "sync_context.h"
#include "performance_counter.h"

Sync::AbstractDelegate::AbstractDelegate(PYXPointer<Sync::AbstractContext> context) : m_context(context) 
{ 
};
Sync::AbstractDelegate::~AbstractDelegate() 
{ 
};

void Sync::Job::endInvoke()
{
	boost::mutex::scoped_lock lock(getContext().getJobDoneMutex());
	while (!m_jobDone)
	{
		getContext().getJobDoneConditionVariable().wait(lock);
	}

	if (wasError())
	{
		rethrowError();
	}
}


void Sync::JobList::addJob(PYXPointer<Sync::Job> job)
{
	bool wasEmpty = true;

	{
		boost::mutex::scoped_lock lock(m_jobsMutex);
		wasEmpty = m_jobs.empty();
		m_jobs.push_back(job);
	}

	if (wasEmpty)
	{
		m_hasJobCondition.notify_one();
	}
}

void Sync::JobList::addJobAsFirst(PYXPointer<Sync::Job> job)
{
	bool wasEmpty = true;

	{
		boost::mutex::scoped_lock lock(m_jobsMutex);
		wasEmpty = m_jobs.empty();
		m_jobs.push_front(job);
	}

	if (wasEmpty)
	{
		m_hasJobCondition.notify_one();
	}
}

bool Sync::JobList::hasJob()
{
	{
		boost::mutex::scoped_lock lock(m_jobsMutex);
		return ! m_jobs.empty();
	}
}

void Sync::JobList::clear()
{
	{
		boost::mutex::scoped_lock lock(m_jobsMutex);
		m_jobs.clear();
	}
}

//! non blocking
PYXPointer<Sync::Job> Sync::JobList::fetchJob()
{
	PYXPointer<Sync::Job> job;

	{
		boost::mutex::scoped_lock lock(m_jobsMutex);
		if (! m_jobs.empty())
		{
			job = *(m_jobs.begin());
			m_jobs.pop_front();
		}
	}

	return job;
}

//! blocking
PYXPointer<Sync::Job> Sync::JobList::fetchJobBlocking()
{
	
	PYXPointer<Sync::Job> job;

	{
		boost::mutex::scoped_lock lock(m_jobsMutex);
		while (m_jobs.empty())
		{
			m_hasJobCondition.wait(lock);
		}

		job = *(m_jobs.begin());		
		m_jobs.pop_front();		
	}

	return job;
}

int Sync::JobList::getWaitingJobsCount()
{
	boost::mutex::scoped_lock lock(m_jobsMutex);
	return (int)m_jobs.size();
}


Sync::AbstractContext::AbstractContext (void) : m_running(false), m_started(false)
{
}

Sync::AbstractContext::~AbstractContext (void)
{	
}

void Sync::AbstractContext::start()
{
	if (m_running)
	{
		PYXTHROW(PYXException,"can't start already running thread");
	}

	if (m_started)
	{
		PYXTHROW(PYXException,"can't start already started thread");
	}	

	if (inWorkerThread())
	{
		PYXTHROW(PYXException,"can't call start from inside the thread");
	}
	else
	{
		m_started = true;
		m_thread = boost::thread(boost::bind(&Sync::AbstractContext::mainLoop,this));
	}
}

void Sync::AbstractContext::makeSureWorkerThreadIsRunning()
{
	//make sure that our thread is running...
	if (! m_running && ! m_started)
	{
		start();
	}
}

void Sync::AbstractContext::stop()
{	
	if (! m_running)
	{
		PYXTHROW(PYXException,"can't stop an unrunning thread");
	}

	//check if we are inside the Sync::Context
	if (inWorkerThread())
	{
		//will exit the main loop
		m_running = false;		
		
		//abort all pending jobs
		abortAllJobs();
	}
	else
	{
		//tell the thread to stop it self
		invokeStopOnWorkerThread();
		
		//wait until thread is stoped completly
		m_thread.join();

		//mark that the thread can be started again
		m_started = false;
	}
}

bool Sync::AbstractContext::inWorkerThread()
{
	return m_thread.get_id() == boost::this_thread::get_id();
}



void Sync::AbstractContext::mainLoop()
{
	m_running = true;

	onWorkerThreadStart();

	while (m_running)
	{
		PYXPointer<Sync::Job> job = fetchJob();

		if (job.get() != NULL)
		{
			performJob(*job);
		}		
	}

	onWorkerThreadStop();
}

void Sync::AbstractContext::abortJob(Sync::Job & job)
{
	job.m_errorString = "Job has been aborted";
	job.m_internalExceptionTypeName = "PYXInvokeException";
	job.m_wasError = true;

	{
		boost::mutex::scoped_lock lock(m_jobDoneMutex);
		job.m_jobDone = true;
	}

	// Wake up any (sleeping) thread that is waiting on this thread's jobs.
	m_jobDoneCond.notify_all();
}

void Sync::AbstractContext::performJob(Sync::Job & job)
{		
	if (job.m_delegate->getContext().m_thread != m_thread)
	{
		PYXTHROW(PYXException,"Sync::Job wasn't executed from right Sync::Context");
	}

	beforePerform(job);

	try
	{
		(*job.m_delegate)();
	} 
	catch(PYXException& ex)
	{
		TRACE_ERROR("Exception occured on Sync::Context while performing a job:" + ex.getFullErrorString());					
		job.m_errorString = ex.getFullErrorString();
		job.m_internalExceptionTypeName = typeid(ex).name();
		job.m_wasError = true;
	}
	catch(...)
	{
		TRACE_ERROR("Exception occured on Sync::Context while performing a job");		
		job.m_errorString = "Unknown error occured";		
		job.m_internalExceptionTypeName = "Unknown";
		job.m_wasError = true;
	}

	{
		boost::mutex::scoped_lock lock(m_jobDoneMutex);
		job.m_jobDone = true;
	}

	afterPerform(job);

	// Wake up any (sleeping) thread that is waiting on this thread's jobs.
	m_jobDoneCond.notify_all();
}

/////////////////////////////////////////////////////////////////////////////////

Sync::WorkerThreadContext::WorkerThreadContext() 	
{
	ASAP.assign(*this,&Sync::WorkerThreadContext::invokeASAP,&Sync::WorkerThreadContext::beginInvokeASAP);
	normal.assign(*this,&Sync::WorkerThreadContext::invoke,&Sync::WorkerThreadContext::beginInvoke);
}

Sync::WorkerThreadContext::~WorkerThreadContext() 
{
	if (isRunning())
	{
		stop();
	}
}

void Sync::WorkerThreadContext::beginInvoke(PYXPointer<Sync::Job> spJob)
{
	beforeInvoke(*spJob);

	if (!inWorkerThread())
	{
		makeSureWorkerThreadIsRunning();
	}

	m_jobs.addJob(spJob);
}


void Sync::WorkerThreadContext::invoke(PYXPointer<Sync::Job> spJob)
{
	if (inWorkerThread())
	{		
		performJob(*spJob);
	}
	else
	{
		beginInvoke(spJob);
	}

	spJob->endInvoke();
	afterInvoke(*spJob);
}

void Sync::WorkerThreadContext::beginInvokeASAP(PYXPointer<Sync::Job> spJob)
{
	beforeInvoke(*spJob);
	if (!inWorkerThread())
	{
		makeSureWorkerThreadIsRunning();
	}

	m_jobs.addJobAsFirst(spJob);
}


void Sync::WorkerThreadContext::invokeASAP(PYXPointer<Sync::Job> spJob)
{
	if (inWorkerThread())
	{		
		performJob(*spJob);
	}
	else
	{
		beginInvokeASAP(spJob);
	}

	spJob->endInvoke();
	afterInvoke(*spJob);
}



//! abort all jobs from jobs queue when thread is stopes
void Sync::WorkerThreadContext::abortAllJobs()
{
	while (m_jobs.hasJob())
	{
		PYXPointer<Sync::Job> job = m_jobs.fetchJob();
		abortJob(*job);
	}
}

//! return an Invoker that send messages to the worker thread
void Sync::WorkerThreadContext::invokeStopOnWorkerThread()
{
	return ASAP.invoke(&Sync::AbstractContext::stop);
}

//! featch a job for the worker thread
PYXPointer<Sync::Job> Sync::WorkerThreadContext::fetchJob()
{
	return m_jobs.fetchJobBlocking();
}

//! performaing a specific job (catching exception)
void Sync::WorkerThreadContext::performJob(Job & job)
{
	threadGuard("perfroming job outside of working thread");

	AbstractContext::performJob(job);
}

/////////////////////////////////////////////////////////////////////////////////

Sync::JobListContext::JobListContext()	
{
	ASAP.assign(*this,&Sync::JobListContext::invoke,&Sync::JobListContext::beginInvokeASAP);
	normal.assign(*this,&Sync::JobListContext::invoke,&Sync::JobListContext::beginInvoke);
	onWorkerThread.assign(*this,&Sync::JobListContext::invokeOnWorkerThread,&Sync::JobListContext::beginInvokeASAP);
}

Sync::JobListContext::~JobListContext() 
{	
	abortAllJobs();
}

void Sync::JobListContext::beginInvoke(PYXPointer<Sync::Job> spJob)
{
	beforeInvoke(*spJob);	

	m_jobs.addJob(spJob);
}

void Sync::JobListContext::beginInvokeASAP(PYXPointer<Sync::Job> spJob)
{
	beforeInvoke(*spJob);
	
	m_jobs.addJobAsFirst(spJob);
}

void Sync::JobListContext::invoke(PYXPointer<Sync::Job> spJob)
{
	//perform in calling thread
	beforeInvoke(*spJob);
	performJob(*spJob);	
	spJob->endInvoke();
	afterInvoke(*spJob);
}

void Sync::JobListContext::invokeOnWorkerThread(PYXPointer<Sync::Job> spJob)
{
	PYXTHROW(PYXException,"JobListContext has no worker thread");	
}


//! abort all jobs from jobs queue when thread is stopes
void Sync::JobListContext::abortAllJobs()
{
	while (m_jobs.hasJob())
	{
		PYXPointer<Sync::Job> job = m_jobs.fetchJob();
		abortJob(*job);
	}
}

//! return an Invoker that send messages to the worker thread
void Sync::JobListContext::invokeStopOnWorkerThread()
{
	return;
}

//! featch a job for the worker thread
PYXPointer<Sync::Job> Sync::JobListContext::fetchJob()
{
	return m_jobs.fetchJob();
}

//! performaing a specific job (catching exception)
void Sync::JobListContext::performJob(Job & job)
{
	//lock shared resource
	{
		boost::recursive_mutex::scoped_lock lock(m_sharedResourceMutex);
		AbstractContext::performJob(job);
	}
}

bool Sync::JobListContext::performOneJob()
{
	PYXPointer<Sync::Job> spJob = fetchJob();
	if (spJob)
	{
		performJob(*spJob);
		return true;
	}
	return false;
}

bool Sync::JobListContext::performJobsWithTimeout(int millisecond)
{
	boost::posix_time::ptime startTime = boost::posix_time::microsec_clock::local_time();
	
	while ( static_cast<int>((boost::posix_time::microsec_clock::local_time() - startTime).total_milliseconds()) < millisecond )
	{
		//perform jobs. if return false - there are no more jobs. quit
		if (!performOneJob())
			return true;
	}
	return false;
}

int Sync::JobListContext::getWaitingJobsCount()
{
	return m_jobs.getWaitingJobsCount();
}

/////////////////////////////////////////////////////////////////////////////////

Sync::SharedResourceContext::SharedResourceContext() 	
{
	ASAP.assign(*this,&Sync::SharedResourceContext::invoke,&Sync::SharedResourceContext::beginInvokeASAP);
	normal.assign(*this,&Sync::SharedResourceContext::invoke,&Sync::SharedResourceContext::beginInvoke);
	onWorkerThread.assign(*this,&Sync::SharedResourceContext::invokeOnWorkerThread,&Sync::SharedResourceContext::beginInvokeASAP);
}

Sync::SharedResourceContext::~SharedResourceContext() 
{
	if (isRunning())
	{
		stop();
	}
}

void Sync::SharedResourceContext::beginInvoke(PYXPointer<Sync::Job> spJob)
{
	beforeInvoke(*spJob);

	if (!inWorkerThread())
	{
		makeSureWorkerThreadIsRunning();
	}

	m_jobs.addJob(spJob);
}

void Sync::SharedResourceContext::beginInvokeASAP(PYXPointer<Sync::Job> spJob)
{
	beforeInvoke(*spJob);
	if (!inWorkerThread())
	{
		makeSureWorkerThreadIsRunning();
	}

	m_jobs.addJobAsFirst(spJob);
}

void Sync::SharedResourceContext::invoke(PYXPointer<Sync::Job> spJob)
{
	//perform in calling thread
	beforeInvoke(*spJob);
	performJob(*spJob);	
	spJob->endInvoke();
	afterInvoke(*spJob);
}

void Sync::SharedResourceContext::invokeOnWorkerThread(PYXPointer<Sync::Job> spJob)
{
	if (inWorkerThread())
	{		
		performJob(*spJob);
	}
	else
	{
		beginInvokeASAP(spJob);
	}

	spJob->endInvoke();
	afterInvoke(*spJob);
}


//! abort all jobs from jobs queue when thread is stopes
void Sync::SharedResourceContext::abortAllJobs()
{
	while (m_jobs.hasJob())
	{
		PYXPointer<Sync::Job> job = m_jobs.fetchJob();
		abortJob(*job);
	}
}

//! return an Invoker that send messages to the worker thread
void Sync::SharedResourceContext::invokeStopOnWorkerThread()
{
	return onWorkerThread.invoke(&Sync::AbstractContext::stop);
}

//! featch a job for the worker thread
PYXPointer<Sync::Job> Sync::SharedResourceContext::fetchJob()
{
	return m_jobs.fetchJobBlocking();
}

//! performaing a specific job (catching exception)
void Sync::SharedResourceContext::performJob(Job & job)
{
	//lock shared resource
	{
		boost::recursive_mutex::scoped_lock lock(m_sharedResourceMutex);
		AbstractContext::performJob(job);
	}
}