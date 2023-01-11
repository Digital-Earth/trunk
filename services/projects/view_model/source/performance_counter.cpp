/******************************************************************************
performance_counter.cpp

begin		: 2007-07-24
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "performance_counter.h"

int PerformanceCounter::m_currentFrame = 0;


std::map<std::string,PYXPointer<TimePerformanceCounter>> PerformanceCounter::m_timePerformanceCounters;
std::map<std::string,PYXPointer<ValuePerformanceCounter>> PerformanceCounter::m_valuePerformanceCounters;
std::map<std::string,PYXPointer<FunctionCallPerformanceCounter>> PerformanceCounter::m_functionCallPerformanceCounters;
std::set<PerformanceCounter*> PerformanceCounter::m_visualCounters;
boost::recursive_mutex PerformanceCounter::m_mutex;


std::vector<int> PerformanceCounter::m_measurements;
int PerformanceCounter::m_countersCount = 0;

bool PerformanceCounter::m_recording = false;
std::ofstream PerformanceCounter::m_out;

void PerformanceCounter::startRecord(std::string & path)
{
	m_out.open(path.c_str(), std::ios::out | std::ios::trunc);
	m_recording = true;

	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		std::map<std::string,PYXPointer<TimePerformanceCounter>>::iterator it = m_timePerformanceCounters.begin();

		while (it != m_timePerformanceCounters.end())
		{
			TimePerformanceCounter & counter = *(it->second);
			m_out << counter.m_index << "," << counter.m_name << "," << "TimePerformanceCounter" << "," << counter.getSampleRatio() 
				  << "," << counter.m_color[0] << "," << counter.m_color[1] << "," << counter.m_color[2] << std::endl;		
			++it;
		}	
	}

	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		std::map<std::string,PYXPointer<ValuePerformanceCounter>>::iterator it = m_valuePerformanceCounters.begin();

		while (it != m_valuePerformanceCounters.end())
		{
			ValuePerformanceCounter & counter = *(it->second);
			m_out << counter.m_index << "," << counter.m_name << "," << "ValuePerformanceCounter" << "," << counter.getSampleRatio() 
				  << "," << counter.m_color[0] << "," << counter.m_color[1] << "," << counter.m_color[2] << std::endl;		
			++it;
		}	
	}

	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		std::map<std::string,PYXPointer<FunctionCallPerformanceCounter>>::iterator it = m_functionCallPerformanceCounters.begin();

		while (it != m_functionCallPerformanceCounters.end())
		{
			FunctionCallPerformanceCounter & counter = *(it->second);
			m_out << counter.m_index << "," << counter.m_name << "," << "FunctionCallPerformanceCounter" << "," << counter.getSampleRatio() 
				  << "," << counter.m_color[0] << "," << counter.m_color[1] << "," << counter.m_color[2] << std::endl;		
			++it;
		}	
	}

	m_out << std::endl;
}

void PerformanceCounter::recordFrame()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	for(int i=0;i<m_countersCount;i++)
	{
		m_out << m_measurements[knMesurmentsHistorySize*i+m_currentFrame];
		if (i<m_countersCount-1)
		{
			m_out << ",";
		}
	}
	m_out << std::endl;
}

void PerformanceCounter::stopRecord()
{
	m_out.close();
	m_recording = false;
}

void PerformanceCounter::traceTimeFrame(int offset)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
		
	std::multimap<double,std::string> orderByTime;

	for(auto & it : m_timePerformanceCounters)
	{
		TimePerformanceCounter & counter = *(it.second);

		double time;
		if (offset>=0)
		{	
			time = ((double)(counter.getMeasurement(offset)))/counter.getSampleRatio();
		}
		else 
		{
			time = ((double)(counter.getMeasurement(PerformanceCounter::knMesurmentsHistorySize+offset)))/counter.getSampleRatio();
		}

		orderByTime.insert(std::make_pair(time,it.first));
	}

	for(auto & it : orderByTime)
	{
		TRACE_INFO(it.second << " value was " << it.first);
	}
}


PerformanceCounter::PerformanceCounter(const std::string & name,const int & index) : m_name(name), m_index(index)
{
	m_color[0] = 1.0f;
	m_color[1] = 1.0f;
	m_color[2] = 1.0;

	m_visualCounters.insert(this);
}

PerformanceCounter::~PerformanceCounter()
{
}

PYXPointer<TimePerformanceCounter> PerformanceCounter::getTimePerformanceCounter(const std::string & name,const float & red,const float & green,const float & blue)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if (! m_timePerformanceCounters[name])
	{
		m_timePerformanceCounters[name] = PYXNEW(TimePerformanceCounter,name,m_countersCount);		
		m_timePerformanceCounters[name]->setColor(red,green,blue);

		m_countersCount++;
		m_measurements.resize(m_countersCount*knMesurmentsHistorySize);
	}

	return m_timePerformanceCounters[name];
}

PYXPointer<ValuePerformanceCounter> PerformanceCounter::getValuePerformanceCounter(const std::string & name,const float & red,const float & green,const float & blue)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if (! m_valuePerformanceCounters[name])
	{
		m_valuePerformanceCounters[name] = PYXNEW(ValuePerformanceCounter,name,m_countersCount);
		m_valuePerformanceCounters[name]->setColor(red,green,blue);

		m_countersCount++;
		m_measurements.resize(m_countersCount*knMesurmentsHistorySize);
	}

	return m_valuePerformanceCounters[name];
}

PYXPointer<FunctionCallPerformanceCounter> PerformanceCounter::getFunctionCallPerformanceCounter(const std::string & name,const float & red,const float & green,const float & blue)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	if (! m_functionCallPerformanceCounters[name])
	{
		m_functionCallPerformanceCounters[name] = PYXNEW(FunctionCallPerformanceCounter,name,m_countersCount);
		m_functionCallPerformanceCounters[name]->setColor(red,green,blue);

		m_countersCount++;
		m_measurements.resize(m_countersCount*knMesurmentsHistorySize);
	}

	return m_functionCallPerformanceCounters[name];

}

//! set the start measurement for the right frame
void PerformanceCounter::startMeasurement()
{	
	TimePerformanceCounter::finishMeasurement();
	ValuePerformanceCounter::finishMeasurement();

	if (m_recording)
	{
		recordFrame();
	}

	m_currentFrame++;
	m_currentFrame%=knMesurmentsHistorySize;

	//reset measurements
	for(int i=0;i<m_countersCount;i++)
	{
		m_measurements[knMesurmentsHistorySize*i+m_currentFrame] = 0;
	}

	TimePerformanceCounter::startMeasurement();
	ValuePerformanceCounter::startMeasurement();
}

void PerformanceCounter::setColor(const float & red,const float & green,const float & blue)
{
	m_color[0] = red;
	m_color[1] = green;
	m_color[2] = blue;
}

void PerformanceCounter::setVisible(const bool & visible)
{
	if (visible)
	{
		if (m_visualCounters.find(this) == m_visualCounters.end())
		{
			m_visualCounters.insert(this);
		}
	}
	else
	{
		if (m_visualCounters.find(this) != m_visualCounters.end())
		{
			m_visualCounters.erase(this);
		}
	}
}


LARGE_INTEGER TimePerformanceCounter::m_start = {0,0};

void TimePerformanceCounter::startMeasurement()
{
	//start a new frame
	QueryPerformanceCounter(&m_start);
}

void TimePerformanceCounter::finishMeasurement()
{
	//finish the last frame
	static PYXPointer<TimePerformanceCounter> endMesurment;
	
	if (! endMesurment)
	{
		endMesurment = getTimePerformanceCounter("end");
		endMesurment->setColor(0.2f,0.2f,0.2f);
	}

	if (m_start.QuadPart != 0)
	{
		endMesurment->makeMeasurement();
	}	
}


TimePerformanceCounter::TimePerformanceCounter(const std::string & name,const int & index) : PerformanceCounter(name,index)
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	//TODO: risky? maybe to change this into LONGLONG?
	m_ratio = static_cast<int>(freq.QuadPart);
}

TimePerformanceCounter::~TimePerformanceCounter()
{
}

void TimePerformanceCounter::makeMeasurement()
{
	LARGE_INTEGER m_now;
	QueryPerformanceCounter(&m_now);

	m_measurements[knMesurmentsHistorySize*m_index+m_currentFrame] = static_cast<int>(m_now.QuadPart-m_start.QuadPart);
}


ValuePerformanceCounter::ValuePerformanceCounter(const std::string & name,const int & index) : PerformanceCounter(name,index),m_currentValue(0)
{
	m_ratio = 1;
}

ValuePerformanceCounter::~ValuePerformanceCounter()
{
}

void ValuePerformanceCounter::startMeasurement()
{
	
}

void ValuePerformanceCounter::finishMeasurement()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	std::map<std::string,PYXPointer<ValuePerformanceCounter>>::iterator it = m_valuePerformanceCounters.begin();

	while (it != m_valuePerformanceCounters.end())
	{
		m_measurements[knMesurmentsHistorySize*it->second->m_index+m_currentFrame] = it->second->m_currentValue;
		++it;
	}
}

	
void ValuePerformanceCounter::setMeasurement(const int & value)
{
	m_currentValue = value;
}


void ValuePerformanceCounter::addToMeasurement(const int & value)
{
	m_currentValue += value;
}



FunctionCallPerformanceCounter::FunctionCallPerformanceCounter(const std::string & name,const int & index) : PerformanceCounter(name,index),m_currentValue(0)
{
	m_ratio = 1;
}

FunctionCallPerformanceCounter::~FunctionCallPerformanceCounter()
{
}

void FunctionCallPerformanceCounter::startMeasurement()
{	
}

void FunctionCallPerformanceCounter::finishMeasurement()
{
}


void FunctionCallPerformanceCounter::addToMeasurement(const int & value)
{
	m_measurements[knMesurmentsHistorySize*m_index+PerformanceCounter::m_currentFrame] += value;
}


void Performance::HighQualityTimer::start()
{
	QueryPerformanceCounter(&m_start);
}

double Performance::HighQualityTimer::tick()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	LARGE_INTEGER m_now;
	QueryPerformanceCounter(&m_now);

	return static_cast<double>(m_now.QuadPart-m_start.QuadPart)/freq.QuadPart;
}

void Performance::HighQualityTimer::stop()
{
	m_seconds = tick();
}