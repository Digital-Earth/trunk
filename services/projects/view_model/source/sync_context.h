#pragma once
#ifndef VIEW_MODEL__SYNC_CONTEXT_H
#define VIEW_MODEL__SYNC_CONTEXT_H
/******************************************************************************
sync_context.h

begin		: 2009-11-01
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#pragma once
#include "pyxis\utility\object.h"
#include "pyxis\utility\exception.h"

#include <boost/shared_ptr.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>


/*! 

Sync::Context can be used to sync invocation of functions for a specific thread.
Example:

  class MySyncContext : public Sync::Context
  {
  public:
	int doSomething() {return 5;}

	int doSomethingElse(int x,int y) { return x+y; }
  }

To create sync context and invoke method on its thread do the following:

  {
     MySyncContext context();

     int result = context.invoke(&MyThread::doSomething); //thread automaticly started on first invoke
     int result2 = context.invoke(&MyThread::doSomethingElse,2,4);
  }
  //thread automatically stoped on context destruction


What happens:
1. the Sync::Context you created creates a worker thread that waits for tasks would be send to it.
2. the invoke method sends the function pointer and arguments (creating delegate templates as needed) and wait until the functions execution is returned.
3. the function doSomething and doSomethingElse will run in the worker thread scope in a sync orderer (fifo)

You can use beginInvoke to send a task to the Sync::Context without waiting for it to be completed. the beginInvoke will return a Sync::ValueJob<R>.
In order to wait until a job, simply do the following:

  PYXPointer<Sync::ValueJob<int>> job = context.beginInvoke(&MySyncContext::doSomething);
  try 
  {
     job->endInvoke(); //throws if job has errors
	 int result = job->getResult();
  }
  catch (Sync::PYXInvokeException& ex)
  {
     ... handle error ... 
	 // use ex.getInternalExceptionTypeName to get the acutal exception type that happend during job execution
  }

Sync::Context allow you to schedule tasks that would be exectued ASAP - or placed at the begining of the Job list.
To do so, call invokeASAP or beginInvokeASAP.

You can sync operations for many objects inisde a context using invokeOn/beginInvokeOn or Proxies.
First, an invokeOn example:

  class Foo
  {
     void doSomething();
  }
  
  Foo a;
  context.invokeOn(a,&Foo::doSomething); //will run a.doSomething() inside context in a sync order. (a passed as ref)

Or you can create a proxy object to Foo a.
The Proxy class have invoke/beginInvoke functions that would be invoked inside the thread on the requested object.
Example:

  Foo a;
  Proxy proxy = context.makeProxy(a);
  proxy.invoke(&Foo::doSomething); //will run a.doSomething() in a sync order inside thread context - so cool!!!

InvokeOn just creates a reference for the object. It does not use PYXPointer or someother smart pointers. 
Therefore, it is your responsiblity to handle correct destruction for those objects.
Meaning: don't kill an object that has pending Sync::Job on it.

That's it.

*/


namespace Sync
{
	template<class T> class Invoker;

	//! Thrown by SyncContext->invoke()/ Sync::Job->endInvoke()
	class PYXInvokeException : public PYXException
	{
	protected:
		std::string m_internalExceptionTypeName;
	public:
		//! Constructor
		PYXInvokeException(const std::string & internalExceptionType,const std::string & strError) : 
		  PYXException(strError), m_internalExceptionTypeName(internalExceptionType) {;}

		//! Get a localized error string.
		virtual const std::string getLocalizedErrorString() const {return "A visualization error occurred.";}	

		const std::string & getInternalExceptionTypeName() { return m_internalExceptionTypeName; };
	};

	//! base class for all delegates. allow the Sync::Context to store and execute all delegates (which most of the time are templates)
	class AbstractDelegate
	{
		friend class AbstractContext;				

	public:
		AbstractDelegate(PYXPointer<Sync::AbstractContext> context);
		virtual ~AbstractDelegate();

	protected:
		PYXPointer<Sync::AbstractContext> m_context;

	private:
		//! virtual invocation of the delegate. protected method: only Sync::AbstractContext can execute this operator
		virtual void operator()(void) = 0;

	public:
		//! get the context that own this delegate. only the owned context should execute the delegate
		Sync::AbstractContext & getContext() { return *m_context; };
	};

	//! base class for all value return delegates.
	template <typename R>
	class AbstractValueDelegate : public AbstractDelegate
	{
		friend class AbstractContext;				

	public:		
		AbstractValueDelegate(PYXPointer<Sync::AbstractContext> context) : AbstractDelegate(context) {};
		virtual ~AbstractValueDelegate() {};

	protected:
		R m_result;

	public:
		R & getResult() { return m_result; };
	};

	template <>
	class AbstractValueDelegate<void> : public AbstractDelegate
	{
		friend class AbstractContext;		
		friend class AbstractInvoker;

	public:		
		AbstractValueDelegate(PYXPointer<AbstractContext> context) : AbstractDelegate(context) {};
		virtual ~AbstractValueDelegate() {};
	
	public:
		void getResult() { PYXTHROW(PYXException,"can't get value of AbstractValueDelegate<void>"); };
	};

	//! delegate for a member function with no parameters and a return value
	template <typename R,class F> class Delegate0 : public AbstractValueDelegate<R>
	{
		friend class AbstractContext;			
		friend class Invoker<F>;

	public:
		typedef R (F::*MemberDelegateFunc)();

		Delegate0<R,F>(PYXPointer<AbstractContext> context, F & object, MemberDelegateFunc func) : AbstractValueDelegate(context), m_object(object), m_func(func) {};

	private:	
		MemberDelegateFunc m_func;
		F & m_object;

	private:
		virtual void operator()(void) { m_result = (m_object.*m_func)(); }
	};

	//! delegate for a member function with no parameters and a void return value
	template <class F>
	class Delegate0<void,F> : public AbstractValueDelegate<void>
	{
		friend class AbstractContext;			
		friend class Invoker<F>;

	public:
		typedef void (F::*MemberDelegateFunc)();

		Delegate0<void,F>(PYXPointer<AbstractContext> context, F & object, MemberDelegateFunc func) : AbstractValueDelegate(context), m_object(object), m_func(func) {};

	private:	
		MemberDelegateFunc m_func;
		F & m_object;

	private:
		virtual void operator()(void) { (m_object.*m_func)(); }
	};

	//! delegate for a member function with 1 parameter and a return value
	template <typename R,class F,typename A1> class Delegate1 : public AbstractValueDelegate<R>
	{
		friend class AbstractContext;	
		friend class Invoker<F>;

	public:
		typedef R (F::*MemberDelegateFunc)(A1);

		Delegate1<R,F,A1>(PYXPointer<AbstractContext> context, F & object, MemberDelegateFunc func,A1 & a1) : AbstractValueDelegate(context), m_object(object), m_func(func), m_a1(a1) {};

	private:	
		MemberDelegateFunc m_func;
		F & m_object;
		A1 m_a1;

	private:
		virtual void operator()(void) {	m_result = (m_object.*m_func)(m_a1); }
	};

	//! delegate for a member function with 1 parameter and a void return value
	template <class F,typename A1> class Delegate1<void,F,A1> : public AbstractValueDelegate<void>
	{
		friend class AbstractContext;	
		friend class Invoker<F>;

	public:
		typedef void (F::*MemberDelegateFunc)(A1);

		Delegate1<void,F,A1>(PYXPointer<AbstractContext> context, F & object, MemberDelegateFunc func,A1 & a1) : AbstractValueDelegate(context), m_object(object), m_func(func), m_a1(a1) {};

	private:	
		MemberDelegateFunc m_func;
		F & m_object;
		A1 m_a1;

	private:
		virtual void operator()(void) {	(m_object.*m_func)(m_a1); }
	};

	//! delegate for a member function with 2 parameter and a return value
	template <typename R,class F,typename A1,typename A2> class Delegate2 : public AbstractValueDelegate<R>
	{
		friend class AbstractContext;	
		friend class Invoker<F>;

	public:
		typedef R (F::*MemberDelegateFunc)(A1,A2);

		Delegate2<R,F,A1,A2>(PYXPointer<AbstractContext> context, F & object, MemberDelegateFunc func,A1 & a1,A2 & a2) : AbstractValueDelegate(context), m_object(object), m_func(func), m_a1(a1), m_a2(a2) {};

	private:	
		MemberDelegateFunc m_func;
		F & m_object;
		A1 m_a1;
		A2 m_a2;

	private:
		virtual void operator()(void) {	m_result = (m_object.*m_func)(m_a1,m_a2); }
	};

	//! delegate for a member function with 2 parameter and a return value
	template <class F,typename A1,typename A2> class Delegate2<void,F,A1,A2> : public AbstractValueDelegate<void>
	{
		friend class AbstractContext;	
		friend class Invoker<F>;

	public:
		typedef void (F::*MemberDelegateFunc)(A1,A2);

		Delegate2<void,F,A1,A2>(PYXPointer<AbstractContext> thread, F & object, MemberDelegateFunc func,A1 & a1,A2 & a2) : AbstractValueDelegate(thread), m_object(object), m_func(func), m_a1(a1), m_a2(a2) {};

	private:	
		MemberDelegateFunc m_func;
		F & m_object;
		A1 m_a1;
		A2 m_a2;

	private:
		virtual void operator()(void) {	(m_object.*m_func)(m_a1,m_a2); }
	};


	//! delegate for a member function with 3 parameter and a return value
	template <typename R,class F,typename A1,typename A2, typename A3> class Delegate3 : public AbstractValueDelegate<R>
	{
		friend class AbstractContext;	
		friend class Invoker<F>;

	public:
		typedef R (F::*MemberDelegateFunc)(A1,A2,A3);

		Delegate3<R,F,A1,A2,A3>(PYXPointer<AbstractContext> context, F & object, MemberDelegateFunc func,A1 & a1,A2 & a2,A3 & a3) : AbstractValueDelegate(context), m_object(object), m_func(func), m_a1(a1), m_a2(a2),m_a3(a3) {};

	private:	
		MemberDelegateFunc m_func;
		F & m_object;
		A1 m_a1;
		A2 m_a2;
		A3 m_a3;

	private:
		virtual void operator()(void) {	m_result = (m_object.*m_func)(m_a1,m_a2,m_a3); }
	};

	//! delegate for a member function with 3 parameter and a return value
	template <class F,typename A1,typename A2, typename A3> class Delegate3<void,F,A1,A2,A3> : public AbstractValueDelegate<void>
	{
		friend class AbstractContext;	
		friend class Invoker<F>;

	public:
		typedef void (F::*MemberDelegateFunc)(A1,A2,A3);

		Delegate3<void,F,A1,A2,A3>(PYXPointer<AbstractContext> context, F & object, MemberDelegateFunc func,A1 & a1,A2 & a2,A3 & a3) : AbstractValueDelegate(context), m_object(object), m_func(func), m_a1(a1), m_a2(a2),m_a3(a3) {};

	private:	
		MemberDelegateFunc m_func;
		F & m_object;
		A1 m_a1;
		A2 m_a2;
		A3 m_a3;

	private:
		virtual void operator()(void) {	(m_object.*m_func)(m_a1,m_a2,m_a3); }
	};
	
	/*!

	The Job class is created for every job sent to the Sync::AbstractContext. 
	The Job class saves a pointer to the delegate of the job and stats about the job exectuion (done/wasError/errorString)
	The delegate itself saves the return value (for every function we would have different template to serve it)
	
	In order to wait for a job to be completed. call the endInvoke() fuction. the Job itself doesn't have mutexes - it use the Sync::AbstractContext mutex and condition variable for waiting.
	By doing so, we improve the performance because we don't need to create mutex and condtion vairable for every job - but only for the thread itself.

	*/
	//! represents a Job for a Sync::AbstractContext.
	class Job : public PYXObject
	{
		friend class AbstractContext;	

	protected:
		boost::shared_ptr<AbstractDelegate> m_delegate;
		bool m_jobDone;
		bool m_wasError;
		std::string m_errorString;
		std::string m_internalExceptionTypeName;

	protected:
		//! protected - only Sync::AbstractContext can create a job.
		Job(boost::shared_ptr<AbstractDelegate> aDelegate) : m_delegate(aDelegate),m_jobDone(false),m_wasError(false) {};
		Job(AbstractDelegate * aDelegate) : m_delegate(aDelegate),m_jobDone(false),m_wasError(false) {};
		virtual ~Job() {};

	public:
		//! get the Sync::AbstractContext for the job
		AbstractContext & getContext() { return m_delegate->getContext(); };
		//! true in the job was done
		bool jobDone() { return m_jobDone; };
		//! true if exception was thrown during job execution
		bool wasError() { return m_wasError; };
		//! if there was an error - get the error string.
		const std::string & getErrorString() { return m_errorString; };
		//! if there was an error - get the exception typeID.
		const std::string & getInternalExceptionTypeName() { return m_internalExceptionTypeName; };

		//! used by the invoke method to rethrow exceptions that happened during the job execution.
		void rethrowError() { throw PYXInvokeException(getInternalExceptionTypeName(),getErrorString()); };

		//! block the calling thread until job execution is done. if exception was thrown during job exectuion, it will be rethrown by endInvoke() method
		void endInvoke();
	};	

	template<typename R>
	class ValueJob : public Job
	{
		friend class AbstractContext;		

	protected:
		ValueJob(boost::shared_ptr<AbstractValueDelegate<R>> aValueDelegate) : Job(aValueDelegate) {};
		ValueJob(AbstractValueDelegate<R> * aValueDelegate) : Job(aValueDelegate) {};
		virtual ~ValueJob() {};
	
	public:
		R & getResult() { return (static_cast<AbstractValueDelegate<R>*>(m_delegate.get()))->getResult(); };
	};

		
	template<>
	class ValueJob<void> : public Job
	{
		friend class AbstractContext;		

	protected:
		ValueJob(boost::shared_ptr<AbstractValueDelegate<void>> aValueDelegate) : Job(aValueDelegate) {};
		ValueJob(AbstractValueDelegate<void> * aValueDelegate) : Job(aValueDelegate) {};
		virtual ~ValueJob() {};
	
	public:
		void getResult() { PYXTHROW(PYXException,"can't get value of ValueJob<void>"); };
	};

	template<class T,typename R>
	class ValueJobHelper : public ValueJob<R>
	{
		friend class Invoker<T>;

	protected:
		ValueJobHelper(boost::shared_ptr<AbstractValueDelegate<R>> aValueDelegate) : ValueJob<R>(aValueDelegate) {};
		ValueJobHelper(AbstractValueDelegate<R> * aValueDelegate) : ValueJob<R>(aValueDelegate) {};
		virtual ~ValueJobHelper() {};
	};

	//! used by the thread to store it's internal jobs execution list
	typedef std::list<PYXPointer<Job>> InternalJobList;

	//! thread-safe JobList for sending jobs from other threads
	class JobList 
	{
	private:
		InternalJobList			  m_jobs;
		boost::mutex			  m_jobsMutex;
		boost::condition_variable m_hasJobCondition;

	public:
		//! add a job to the JobList at the end of job list
		void addJob(PYXPointer<Job> job);

		//! add a job to the JobList at the begining of the job list
		void addJobAsFirst(PYXPointer<Job> job);

		//! return true when has jobs inisde queue
		bool hasJob();

		//! remove all jobs from inside queue
		void clear();
		
		//! none blocking, if no block inside list - return pointer to NULL
		PYXPointer<Job> fetchJob();
		
		//! blocking. waiting until a job would be insterted into the list
		PYXPointer<Job> fetchJobBlocking();

		int getWaitingJobsCount();
	};

	template<class T> class Invoker
	{	
	public:
		typedef void (T::*InvokeFuncImpl)(PYXPointer<Job> spJob);
		Invoker() : m_context(NULL) {};
		Invoker(T & context,InvokeFuncImpl invokeFunc,InvokeFuncImpl beginInvokeFunc) : m_context(&context), m_invokeFunc(invokeFunc), m_beginInvokeFunc(beginInvokeFunc) {};
		virtual ~Invoker() {};

		void assign(T & context,InvokeFuncImpl invokeFunc,InvokeFuncImpl beginInvokeFunc) 
		{
			//only assign once.
			if (m_context == NULL)
			{
				m_context = &context;
				m_invokeFunc = invokeFunc;
				m_beginInvokeFunc = beginInvokeFunc;
			}
		}

	protected:
		AbstractContext * m_context;
		InvokeFuncImpl m_invokeFunc;
		InvokeFuncImpl m_beginInvokeFunc;

	private:
		Invoker(const AbstractInvoker &) {};
		Invoker& operator=(const AbstractInvoker &) {};
	
	public:
		//beginInvoke<R>
		template<typename R,class F> PYXPointer<ValueJob<R>> beginInvoke(R (F::*func)())
		{	
			return beginInvokeOn(*(static_cast<F*>(m_context)),func);			
		}
		template<typename R,class F,typename A1> PYXPointer<ValueJob<R>> beginInvoke(R (F::*func)(A1),A1 a1)
		{	
			return beginInvokeOn(*(static_cast<F*>(m_context)),func,a1);			
		}
		template<typename R,class F,typename A1,typename A2> PYXPointer<ValueJob<R>> beginInvoke(R (F::*func)(A1,A2),A1 a1,A2 a2)
		{	
			return beginInvokeOn(*(static_cast<F*>(m_context)),func,a1,a2);			
		}
		template<typename R,class F,typename A1,typename A2,typename A3> PYXPointer<ValueJob<R>> beginInvoke(R (F::*func)(A1,A2,A3),A1 a1,A2 a2,A3 a3)
		{				
			return beginInvokeOn(*(static_cast<F*>(m_context)),func,a1,a2,a3);			
		}

		//void invoke
		template<class F> void invoke(void (F::*func)())
		{
			invokeOn<F>(*(static_cast<F*>(m_context)),func);
		}
		template<class F,typename A1> void invoke(void (F::*func)(A1),A1 a1)
		{
			invokeOn<F,A1>(*(static_cast<F*>(m_context)),func,a1);
		}
		template<class F,typename A1,typename A2> void invoke(void (F::*func)(A1,A2),A1 a1,A2 a2)
		{
			invokeOn<F,A1,A2>(*(static_cast<F*>(m_context)),func,a1,a2);
		}
		template<class F,typename A1,typename A2,typename A3> void invoke(void (F::*func)(A1,A2,A3),A1 a1,A2 a2,A3 a3)
		{
			invokeOn<F,A1,A2,A3>(*(static_cast<F*>(m_context)),func,a1,a2,a3);		
		}

		//invoke<R>
		template<typename R,class F> R invoke(R (F::*func)())
		{
			return invokeOn(*(static_cast<F*>(m_context)),func);		
		}
		template<typename R,class F,typename A1> R invoke(R (F::*func)(A1),A1 a1)
		{
			return invokeOn(*(static_cast<F*>(m_context)),func,a1);	
		}
		template<typename R,class F,typename A1,typename A2> R invoke(R (F::*func)(A1,A2),A1 a1,A2 a2)
		{
			return invokeOn(*(static_cast<F*>(m_context)),func,a1,a2);		
		}
		template<typename R,class F,typename A1,typename A2,typename A3> R invoke(R (F::*func)(A1,A2,A3),A1 a1,A2 a2,A3 a3)
		{
			return invokeOn(*(static_cast<F*>(m_context)),func,a1,a2,a3);		
		}

		//beginInvokeOn<R>
		template<typename R,class F> PYXPointer<ValueJob<R>> beginInvokeOn(F & object,R (F::*func)())
		{		
			PYXPointer<ValueJob<R>> spJob = PYXObject::pointerTo(new ValueJobHelper<T,R>(new Delegate0<R,F>(m_context,object,func)));
			beginInvoke(spJob);	
			return spJob;		
		}
		template<typename R,class F,typename A1> PYXPointer<ValueJob<R>> beginInvokeOn(F & object,R (F::*func)(A1),A1 a1)
		{			
			PYXPointer<ValueJob<R>> spJob = PYXObject::pointerTo(new ValueJobHelper<T,R>(new Delegate1<R,F,A1>(m_context,object,func,a1)));	
			beginInvoke(spJob);	
			return spJob;
		}
		template<typename R,class F,typename A1,typename A2> PYXPointer<ValueJob<R>> beginInvokeOn(F & object,R (F::*func)(A1,A2),A1 a1,A2 a2)
		{			
			PYXPointer<ValueJob<R>> spJob = PYXObject::pointerTo(new ValueJobHelper<T,R>(new Delegate2<R,F,A1,A2>(m_context,object,func,a1,a2)));
			beginInvoke(spJob);	
			return spJob;
		}
		template<typename R,class F,typename A1,typename A2,typename A3> PYXPointer<ValueJob<R>> beginInvokeOn(F & object,R (F::*func)(A1,A2,A3),A1 a1,A2 a2,A3 a3)
		{			
			PYXPointer<ValueJob<R>> spJob = PYXObject::pointerTo(new ValueJobHelper<T,R>(new Delegate3<R,F,A1,A2,A3>(m_context,object,func,a1,a2,a3)));
			beginInvoke(spJob);	
			return spJob;
		}

		//void invokeOn
		template<class F> void invokeOn(F & object,void (F::*func)())
		{
			PYXPointer<Job> spJob = PYXObject::pointerTo(new ValueJobHelper<T,void>(new Delegate0<void,F>(m_context,object,func)));
			invoke(spJob);				
		}
		template<class F,typename A1> void invokeOn(F & object,void (F::*func)(A1),A1 a1)
		{			
			PYXPointer<Job> spJob = PYXObject::pointerTo(new ValueJobHelper<T,void>(new Delegate1<void,F,A1>(m_context,object,func,a1)));	
			invoke(spJob);	
		}
		template<class F,typename A1,typename A2> void invokeOn(F & object,void (F::*func)(A1,A2),A1 a1,A2 a2)
		{			
			PYXPointer<Job> spJob = PYXObject::pointerTo(new ValueJobHelper<T,void>(new Delegate2<void,F,A1,A2>(m_context,object,func,a1,a2)));	
			invoke(spJob);	
		}
		template<class F,typename A1,typename A2,typename A3> void invokeOn(F & object,void (F::*func)(A1,A2,A3),A1 a1,A2 a2,A3 a3)
		{			
			PYXPointer<Job> spJob = PYXObject::pointerTo(new ValueJobHelper<T,void>(new Delegate3<void,F,A1,A2,A3>(m_context,object,func,a1,a2,a3)));	
			invoke(spJob);	
		}
		

		//invokeOn<R>
		template<typename R,class F> R invokeOn(F & object,R (F::*func)())
		{
			PYXPointer<ValueJob<R>> spJob = PYXObject::pointerTo(new ValueJobHelper<T,R>(new Delegate0<R,F>(m_context,object,func)));
			invoke(spJob);	
			return spJob->getResult();			
		}
		template<typename R,class F,typename A1> R invokeOn(F & object,R (F::*func)(A1),A1 a1)
		{
			PYXPointer<ValueJob<R>> spJob = PYXObject::pointerTo(new ValueJobHelper<T,R>(new Delegate1<R,F,A1>(m_context,object,func,a1)));
			invoke(spJob);	
			return spJob->getResult();			
		}
		template<typename R,class F,typename A1,typename A2> R invokeOn(F & object,R (F::*func)(A1,A2),A1 a1,A2 a2)
		{
			PYXPointer<ValueJob<R>> spJob = PYXObject::pointerTo(new ValueJobHelper<T,R>(new Delegate2<R,F,A1,A2>(m_context,object,func,a1,a2)));
			invoke(spJob);	
			return spJob->getResult();			
		}
		template<typename R,class F,typename A1,typename A2,typename A3> R invokeOn(F & object,R (F::*func)(A1,A2,A3),A1 a1,A2 a2,A3 a3)
		{
			PYXPointer<ValueJob<R>> spJob = PYXObject::pointerTo(new ValueJobHelper<T,R>(new Delegate3<R,F,A1,A2,A3>(m_context,object,func,a1,a2,a3)));
			invoke(spJob);	
			return spJob->getResult();			
		}

	public:
		//! performing the actual beginInvoke
		virtual void beginInvoke(PYXPointer<Job> spJob)
		{
			((*(static_cast<T*>(m_context))).*m_beginInvokeFunc)(spJob);
		}

		//! performing the actual invoke
		virtual void invoke(PYXPointer<Job> spJob)
		{
			((*(static_cast<T*>(m_context))).*m_invokeFunc)(spJob);
		}
	};

	class AbstractContext : public PYXObject
	{	
		friend class Job;
		template<class T> friend class Proxy;

	public:
		AbstractContext(void);
		virtual ~AbstractContext(void);

	private:
		boost::thread				m_thread;

		boost::mutex				m_jobDoneMutex;
		boost::condition_variable	m_jobDoneCond;
		
		bool						m_running;	
		bool						m_started;

	protected:
		boost::condition_variable & getJobDoneConditionVariable() { return m_jobDoneCond; };
		boost::mutex & getJobDoneMutex() { return m_jobDoneMutex; };

	public:
		//! start the thread - no blocking. automaticly called on invoke/beginInvoke if not called.
		void start();
		
		//! stop the thread - blocking until the thread is stoped. automaticly called on distruction
		void stop();

	protected:
		//! called at the beginning of the mainloop
		virtual void onWorkerThreadStart() {};

		//! called at the end of the mainloop
		virtual void onWorkerThreadStop() {};

		//! called in the message thread before starting to perform a job
		virtual void beforePerform(Job & job) {};

		//! called in the message thread after performing a job
		virtual void afterPerform(Job & job) {};

	public:
		//! return true if the thread is the worker thread of the Context
		bool inWorkerThread();		

		//! return true if the message thread is running
		bool isRunning() {return m_running;};

	protected:
		//! Helper function to protected "must be invoked" functions
		inline void threadGuard(const std::string & message) { if (! inWorkerThread()) PYXTHROW(PYXException,message); };

		//! make sure that the worker thread is running. useful for beginInvoke
		void makeSureWorkerThreadIsRunning();

		
	protected:
		//! the main loop of the worker thread of the Context
		void mainLoop();

		//! abort all jobs from jobs queue when thread is stopes
		virtual void abortAllJobs() = 0;

		//! send stop to invoked Thread
		virtual void invokeStopOnWorkerThread() = 0;

		//! featch a job for the worker thread
		virtual PYXPointer<Sync::Job> fetchJob() = 0;

		//! performaing a specific job (catching exception)
		virtual void performJob(Job & job);

		//! mark job as aborted and notifiy all sleeping threads
		virtual void abortJob(Job & job);
	};	

	class WorkerThreadContext : public AbstractContext
	{
	public:
		WorkerThreadContext();
		virtual ~WorkerThreadContext();

	protected:
		JobList						m_jobs;		

	protected:
		//! called in the invoking thread before invoke begins
		virtual void beforeInvoke(Job & job) {};

		//! called in the invoking thread after invoke ends
		virtual void afterInvoke(Job & job) {};

		//! called in the message thread before starting to perform a job
		virtual void beforePerform(Job & job) {};

		//! called in the message thread after performing a job
		virtual void afterPerform(Job & job) {};


	public:
		//! return an ASAP invoker that put the job at the begining of the JobList
		Invoker<WorkerThreadContext> ASAP;

		//! return a Normal inokver that put the job at the end of the JobList
		Invoker<WorkerThreadContext> normal;

	protected:
		//! abort all jobs from jobs queue when thread is stopes
		virtual void abortAllJobs();

		//! return an Invoker that send messages to the worker thread
		virtual void invokeStopOnWorkerThread();

		//! featch a job for the worker thread
		virtual PYXPointer<Sync::Job> fetchJob();

		//! performaing a specific job (catching exception)
		virtual void performJob(Job & job);

	protected:
		//! would be called by the normalInvoker
		void beginInvoke(PYXPointer<Job> spJob);
		
		//! would be called by the normalInvoker
		void invoke(PYXPointer<Job> spJob);

		//! would be called by the ASAPInvoker
		void beginInvokeASAP(PYXPointer<Job> spJob);
		
		//! would be called by the ASAPInvoker
		void invokeASAP(PYXPointer<Job> spJob);
	};

	class JobListContext : public AbstractContext
	{
	protected:
		boost::recursive_mutex m_sharedResourceMutex;

	public:
		JobListContext();
		virtual ~JobListContext();

	protected:
		JobList						m_jobs;		

	protected:
		//! called in the invoking thread before invoke begins
		virtual void beforeInvoke(Job & job) {};

		//! called in the invoking thread after invoke ends
		virtual void afterInvoke(Job & job) {};

		//! called in the message thread before starting to perform a job
		virtual void beforePerform(Job & job) {};

		//! called in the message thread after performing a job
		virtual void afterPerform(Job & job) {};

	public:
		//! return an ASAP invoker that put the job at the begining of the JobList. invoke would run on calling thread 
		Invoker<JobListContext> ASAP;

		//! return a Normal invoker that put the job at the end of the JobList. invoke would run on calling thread 
		Invoker<JobListContext> normal;

		//! return a ASAP invoker that put the job at the end of the JobList. invoke would run on worker thread 
		Invoker<JobListContext> onWorkerThread;

		//! perform one job from JobList. return false if not jobs were performed.
		bool performOneJob();

		//! perform jobs from JobList for given millisecond of time or until not more jobs in JobList. return true if all jobs were executed
		bool performJobsWithTimeout(int millisecond);

		int getWaitingJobsCount();

	protected:
		//! abort all jobs from jobs queue when thread is stopes
		virtual void abortAllJobs();

		//! return an Invoker that send messages to the worker thread
		virtual void invokeStopOnWorkerThread();

		//! featch a job for the worker thread
		virtual PYXPointer<Sync::Job> fetchJob();

		//! performaing a specific job (catching exception)
		virtual void performJob(Job & job);

	protected:
		//! would be called by the normalInvoker
		void beginInvoke(PYXPointer<Job> spJob);
		
		//! would be called by the normalInvoker
		void invoke(PYXPointer<Job> spJob);

		//! would be called by the ASAPInvoker
		void beginInvokeASAP(PYXPointer<Job> spJob);
		
		//! would be called by the ASAPInvoker
		void invokeOnWorkerThread(PYXPointer<Job> spJob);
	};

	class SharedResourceContext : public AbstractContext
	{
	protected:
		boost::recursive_mutex m_sharedResourceMutex;

	public:
		SharedResourceContext();
		virtual ~SharedResourceContext();

	protected:
		JobList						m_jobs;		

	protected:
		//! called in the invoking thread before invoke begins
		virtual void beforeInvoke(Job & job) {};

		//! called in the invoking thread after invoke ends
		virtual void afterInvoke(Job & job) {};

		//! called in the message thread before starting to perform a job
		virtual void beforePerform(Job & job) {};

		//! called in the message thread after performing a job
		virtual void afterPerform(Job & job) {};

	public:
		//! return an ASAP invoker that put the job at the begining of the JobList. invoke would run on calling thread 
		Invoker<SharedResourceContext> ASAP;

		//! return a Normal invoker that put the job at the end of the JobList. invoke would run on calling thread 
		Invoker<SharedResourceContext> normal;

		//! return a ASAP invoker that put the job at the end of the JobList. invoke would run on worker thread 
		Invoker<SharedResourceContext> onWorkerThread;

	protected:
		//! abort all jobs from jobs queue when thread is stopes
		virtual void abortAllJobs();

		//! return an Invoker that send messages to the worker thread
		virtual void invokeStopOnWorkerThread();

		//! featch a job for the worker thread
		virtual PYXPointer<Sync::Job> fetchJob();

		//! performaing a specific job (catching exception)
		virtual void performJob(Job & job);

	protected:
		//! would be called by the normalInvoker
		void beginInvoke(PYXPointer<Job> spJob);
		
		//! would be called by the normalInvoker
		void invoke(PYXPointer<Job> spJob);

		//! would be called by the ASAPInvoker
		void beginInvokeASAP(PYXPointer<Job> spJob);
		
		//! would be called by the ASAPInvoker
		void invokeOnWorkerThread(PYXPointer<Job> spJob);
	};
}

/*
class Foo
{
public:
	void boo(int a);
	int boba();
};

class MyThread : public Sync::WorkerThreadContext
{
protected:
	Foo f;

public:

	void func1()
	{				
		ASAP.beginInvokeOn(f,&Foo::boo,1);
		int i = normal.invokeOn(f,&Foo::boba);

		normal.invoke(&MyThread::func2,1);
	}

	void func2(int a)
	{
		ASAP.beginInvoke(&MyThread::func1);
		normal.invoke(&MyThread::func1);
		int i = ASAP.invoke(&MyThread::func3,4);
		normal.invoke(&MyThread::func3,4);
	}

	int func3(int a)
	{

	}

};
*/

#endif