#pragma once
#ifndef VIEW_MODEL__PERFORMANCE_COUNTER_H
#define VIEW_MODEL__PERFORMANCE_COUNTER_H
/******************************************************************************
performance_counter.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "component.h"
#include "components\container_component.h"

#include <fstream>
#include <sstream>
#include <map>
#include <set>

//! utility macro to mesure function calles. FUNC_NAME is a string!
#define MEASURE_FUNC_CALL(FUNC_NAME) (PerformanceCounter::getFunctionCallPerformanceCounter((FUNC_NAME))->addToMeasurement(1))

//! case class for all PerformanceCounters
class PerformanceCounter : public PYXObject
{
	friend class PerformanceRenderer;
	friend class TimePerformanceCounter;
	friend class ValuePerformanceCounter;
	friend class FunctionCallPerformanceCounter;

protected:	
	static bool m_disposed;
	static int m_currentFrame;
	static std::map<std::string,PYXPointer<TimePerformanceCounter>> m_timePerformanceCounters;
	static std::map<std::string,PYXPointer<ValuePerformanceCounter>> m_valuePerformanceCounters;
	static std::map<std::string,PYXPointer<FunctionCallPerformanceCounter>> m_functionCallPerformanceCounters;
	static std::set<PerformanceCounter*> m_visualCounters;
	static boost::recursive_mutex m_mutex;

	const static int knMesurmentsHistorySize = 100;
	static std::vector<int> m_measurements;
	static int m_countersCount;

	static bool m_recording;
	static std::ofstream m_out;
	
protected:
	std::string m_name;
	int m_index;
	float m_color[3];
	int m_ratio;

protected:
	PerformanceCounter(const std::string & name,const int & index);
	virtual ~PerformanceCounter();

public:	
	static void traceTimeFrame(int offset);

	static void startRecord(std::string & path);
	static void recordFrame();
	static void stopRecord();

	//! set the start measurement for the right frame
	static void startMeasurement();

	//! get pointer to time performance counter
	static PYXPointer<TimePerformanceCounter> getTimePerformanceCounter(const std::string & name,const float & red = 1.0f,const float & green = 1.0f,const float & blue = 1.0f);

	//! get pointer to time performance counter
	static PYXPointer<ValuePerformanceCounter> getValuePerformanceCounter(const std::string & name,const float & red = 1.0f,const float & green = 1.0f,const float & blue = 1.0f);

	//! get pointer to time performance counter
	static PYXPointer<FunctionCallPerformanceCounter> getFunctionCallPerformanceCounter(const std::string & name,const float & red = 1.0f,const float & green = 1.0f,const float & blue = 1.0f);

	inline const int & getMeasurement(const int & offset)
	{
		return m_measurements[knMesurmentsHistorySize*m_index + ((m_currentFrame+1+offset)%knMesurmentsHistorySize)];
	}

	int getSampleRatio() { return m_ratio; };

	void setColor(const float & red,const float & green,const float & blue);	

	void setVisible(const bool & visible);
};

class TimePerformanceCounter : public PerformanceCounter
{
	friend class PerformanceCounter;

protected:
	static LARGE_INTEGER m_start;

protected:
	//! called by PerformceCounter::startMeasurement
	static void startMeasurement();

	//! called by PerformceCounter::startMeasurement
	static void finishMeasurement();

public:
	TimePerformanceCounter(const std::string & name,const int & index);
	virtual ~TimePerformanceCounter();

	void makeMeasurement();
};

class ValuePerformanceCounter : public PerformanceCounter
{
	friend class PerformanceCounter;

protected:
	//! called by PerformceCounter::startMeasurement
	static void startMeasurement();

	//! called by PerformceCounter::startMeasurement
	static void finishMeasurement();
	
	int m_currentValue;

public:
	ValuePerformanceCounter(const std::string & name,const int & index);
	virtual ~ValuePerformanceCounter();
	
	void setMeasurement(const int & value);
	void addToMeasurement(const int & value);
};

class FunctionCallPerformanceCounter : public PerformanceCounter
{
	friend class PerformanceCounter;

protected:
	//! called by PerformceCounter::startMeasurement
	static void startMeasurement();

	//! called by PerformceCounter::startMeasurement
	static void finishMeasurement();
	
	int m_currentValue;

public:
	FunctionCallPerformanceCounter(const std::string & name,const int & index);
	virtual ~FunctionCallPerformanceCounter();
		
	void addToMeasurement(const int & value);
};

namespace Performance
{
	class HighQualityTimer
	{
	protected:
		LARGE_INTEGER m_start;
		double m_seconds;

	public:

		HighQualityTimer() : m_seconds(0) {};

		void start();
		double tick(); //current time from start call. (return time in seconds)
		void stop();

		double getTime() { return m_seconds; };
	};

	class ScopedTimer
	{
	protected:
		HighQualityTimer m_timer;
		std::string m_message;
		std::string m_tickMessages;
		double m_threshold;
		double m_lastTick;

	public:
		ScopedTimer(const std::string & message, double threshold = 0.0) : m_message(message), m_threshold(threshold), m_lastTick(0)
		{
			m_timer.start();
		}

		void tick(const std::string & message)
		{
			double tick = m_timer.tick();
			double period = tick-m_lastTick;
			m_lastTick = tick;
			m_tickMessages += message + " took on " + StringUtils::toString(period) + " sec\n";
		}

		~ScopedTimer()
		{
			m_timer.stop();
			if (m_timer.getTime() >= m_threshold)
			{
				TRACE_INFO(m_message << " took " << m_timer.getTime() << " sec");
				if (!m_tickMessages.empty())
				{
					TRACE_INFO("Ticks\n" << m_tickMessages);
				}
			}
		}
	};
}

#endif
