#ifndef PYXIS__UTILITY__JOBLISTSP_H
#define PYXIS__UTILITY__JOBLISTSP_H
/******************************************************************************
joblist.h

begin		: 2009-10-24
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

// boost includes
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/interprocess/sync/interprocess_semaphore.hpp>

// std includes
#include <queue>
#include <vector>

/*!
A thread safe class for handing out a list of items. For our usage
we have called the items jobs, as that suits our situation, but the list
could be used for any type of items.  The items are handed out in the same
order that they are added.  If there are no items available when a client
asks for one, the calling thread is blocked until an item is available.
The blocking is done using semaphores which will not eat up extra CPU cycles.

Typical usage for this class would be to create a thread that is in a loop
getting jobs from the list and acting on them, while another thread is
preparing jobs and placing them into the list.

The job lists maintain smart pointers to the job items, and returns those same pointers.
The job list will never free a job item, but becuase of the use of boost intrusive pointers
management of the storage for job items should be handled automatically.

See JobList for a job list that works with bald pointers.
*/
template<class T> class JobListsp
{
public:
	//! Default constructor, with no set end limit.
	JobListsp<T>() :
		m_jobsToDo(-1)
	{
		initialize();
	}

	//! Constructor with a given maximum number of jobs to hand out.
	JobListsp<T>(int nJobsToComplete) :
		m_jobsToDo(nJobsToComplete)
	{
		initialize();
	}

	//! Destructor
	~JobListsp<T> () 
	{
		CloseHandle(m_hSemaphore);
	}

	//! Add a job to the list of jobs that will be handed out.
	bool AddJob(PYXPointer<T> spJob)
	{
		{
			boost::recursive_mutex::scoped_lock lock(m_jobsMutex);
			// if we are finished don't add the job and report back failure.
			if (m_bFinished)
			{
				return false;
			}

			m_jobs.push(spJob);
			if (m_jobs.size() > m_nLargestListSize)
			{
				m_nLargestListSize = m_jobs.size();
			}
		}
		ReleaseSemaphore(m_hSemaphore, 1, 0);
		return true;
	}

	//! Get a job from the list of jobs.
	// An empty smart pointer returned means there are no more jobs available
	// and no more jobs will be coming.
	PYXPointer<T> GetJob()
	{
		PYXPointer<T> spJob;
		// As long as we are not marked as finished, keep trying to get a job.
		// If the job list is empty, then we must have removed a job and there will
		// be extra Releases in the queue that will be eaten up by the while loop here.
		while (!spJob)
		{
			// wait on the semaphore
			WaitForSingleObject(m_hSemaphore, INFINITE);      

			// lock data access
			boost::recursive_mutex::scoped_lock lock(m_jobsMutex);

			if (m_bFinished)
			{
				// let any other pending thread through.
				ReleaseSemaphore(m_hSemaphore, 1, 0);
				return PYXPointer<T>();
			}

			if (!m_jobs.empty())
			{
				spJob = m_jobs.front();
				m_jobs.pop();

				m_jobsHandedOut += 1;

				if (m_jobsToDo != -1  && m_jobsHandedOut >= m_jobsToDo)
				{
					m_bFinished = true;
					// let any other pending thread through.
					ReleaseSemaphore(m_hSemaphore, 1, 0);
				}
			}
		}

		return spJob;
	}

	//! Clear all pending jobs in the list -- fills in a vector with the removed jobs.
	void ClearJobs(std::vector< PYXPointer<T> >& removedJobs)
	{
		// lock data access
		boost::recursive_mutex::scoped_lock lock(m_jobsMutex);

		while (m_jobs.size() > 0)
		{
			removedJobs.push_back(m_jobs.front());
			m_jobs.pop();
		}
	}

	/*! Clear all pending jobs and set the finished flag so that 
	    threads will get shut down when they ask for the next job.
	*/
	void Stop(std::vector< PYXPointer<T> >& removedJobs)
	{
		// lock data access
		boost::recursive_mutex::scoped_lock lock(m_jobsMutex);

		m_bFinished = true;
		ClearJobs(removedJobs);

		// let any other pending thread through.
		ReleaseSemaphore(m_hSemaphore, 1, 0);
	}

	template<class Less>
	void Sort(Less less)
	{
		// lock data access
		boost::recursive_mutex::scoped_lock lock(m_jobsMutex);
		
		std::vector<PYXPointer<T>> allJobs;
		ClearJobs(allJobs);

		std::sort(allJobs.begin(),allJobs.end(),less);

		for(std::vector<PYXPointer<T>>::iterator it = allJobs.begin();it != allJobs.end();++it)
		{
			m_jobs.push(*it);
		}
	}

private:

	//! Common initialization routine called by all constructors.
	void initialize()
	{
		m_hSemaphore = CreateSemaphore( 
        NULL,           // default security attributes
        0,				// initial count
        2000000000,		// maximum count
        NULL);          // unnamed semaphore

		m_jobsHandedOut = 0;
		m_nLargestListSize = 0;
		m_bFinished = false;
	}

	//! The jobs that are waiting to be processed.
	std::queue< PYXPointer<T> > m_jobs;

	//! The number of jobs that have been handed out.
	int m_jobsHandedOut;

	/*! The number of jobs to hand out before we stop handing out jobs.
	    -1 will indicate no job limit.
	*/
	int m_jobsToDo;

	//! Set to true if the last job has been handed out.
	bool m_bFinished;

	//! Semaphore used to wait until a job is available.
	// NOTE: do not change this to use boost::interprocess_semaphore
	// performance will suffer dramatically.
	HANDLE m_hSemaphore;

	//! Must be locked to access any data in the class except the semaphore.
	boost::recursive_mutex m_jobsMutex;

	//! Largest list size.
	unsigned int m_nLargestListSize;
};

#endif

