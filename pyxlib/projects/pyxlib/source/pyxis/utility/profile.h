#ifndef PYXIS__UTILITY__PROFILE_H
#define PYXIS__UTILITY__PROFILE_H
/******************************************************************************
profile.h

begin		: 2012-06-21
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/utility/object.h"
#include "pyxis/utility/trace.h"

// boost includes
#include <boost/thread/locks.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/recursive_mutex.hpp>

// std includes

//////////////////////////////////////////////////////////////////////////////
// PYXHighQualityTimer
//////////////////////////////////////////////////////////////////////////////

#ifndef _WINNT_
typedef __int64 LARGE_INTEGER;
#endif

class PYXLIB_DECL PYXHighQualityTimer
{
private:
	LARGE_INTEGER m_start;
	double m_seconds;

public:

	PYXHighQualityTimer() : m_seconds(0) {};

	void start();
	double tick(); //current time from start call. (return time in seconds)
	void stop();

	double getTime() { return m_seconds; };
};

class PYXLIB_DECL PYXTotalTimeCounter
{
private:
	boost::recursive_mutex m_mutex;
	double m_totalTime;

public:
	void add(double time)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		m_totalTime += time;
	}

	double getTotalTime()
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		return m_totalTime;
	}


	PYXTotalTimeCounter(const std::string & name); 
	~PYXTotalTimeCounter();

	static void traceAllCounters();	

public:
	class MeasureScopeTime
	{
	private:
		PYXHighQualityTimer m_timer;
		PYXTotalTimeCounter & m_counter;
	public:
		MeasureScopeTime(PYXTotalTimeCounter & counter) : m_counter(counter)
		{
			m_timer.start();
		}

		~MeasureScopeTime()
		{
			m_counter.add(m_timer.tick());
		}
	};
};

class PYXTimeLimitWarning
{
private:
	PYXHighQualityTimer m_timer;
	double m_maxTime;
	std::string m_message;
public:
	PYXTimeLimitWarning(const std::string & message,double timeLimit) : m_message(message),m_maxTime(timeLimit)
	{
		m_timer.start();
	}

	~PYXTimeLimitWarning()
	{
		m_timer.stop();
		if (m_timer.getTime() > m_maxTime)
		{
			TRACE_INFO(m_message << " took " << m_timer.getTime() << "[sec]");
		}
	}
};

#define WARN_IF_FUNCTION_TOOK_MORE_THAN_SEC(TIME) PYXTimeLimitWarning(std::string(__FUNCTION__) + " at line: " + std::to_string(__LINE__),TIME)

#endif

