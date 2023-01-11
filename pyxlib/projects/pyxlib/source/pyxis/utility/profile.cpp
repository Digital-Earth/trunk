/******************************************************************************
profile.cpp

begin		: 2012-06-15
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include "pyxis/utility/profile.h"
#include "pyxis/utility/string_utils.h"

//////////////////////////////////////////////////////////////////////////////
// PYXHighQualityTimer
//////////////////////////////////////////////////////////////////////////////

void PYXHighQualityTimer::start()
{
	QueryPerformanceCounter(&m_start);
}

double PYXHighQualityTimer::tick()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	LARGE_INTEGER m_now;
	QueryPerformanceCounter(&m_now);

	return static_cast<double>(m_now.QuadPart-m_start.QuadPart)/freq.QuadPart;
}

void PYXHighQualityTimer::stop()
{
	m_seconds = tick();
}

//////////////////////////////////////////////////////////////////////////////
// PYXHighQualityTimer
//////////////////////////////////////////////////////////////////////////////

std::map<PYXTotalTimeCounter*,std::string> & getAllCountersMap()
{
	static std::map<PYXTotalTimeCounter*,std::string> s_allCountersMap;
	return s_allCountersMap;
}

boost::recursive_mutex & getAllCountersMapMutex()
{
	static boost::recursive_mutex s_allCountersMapMutex;
	return s_allCountersMapMutex;
}

PYXTotalTimeCounter::PYXTotalTimeCounter(const std::string & name)
{
	boost::recursive_mutex::scoped_lock lock(getAllCountersMapMutex());
	getAllCountersMap()[this] = name;
}

PYXTotalTimeCounter::~PYXTotalTimeCounter()
{
	boost::recursive_mutex::scoped_lock lock(getAllCountersMapMutex());
	getAllCountersMap().erase(this);
}

void PYXTotalTimeCounter::traceAllCounters()
{
	std::string message = "Total Time Counters Status:\n";
	{
		boost::recursive_mutex::scoped_lock lock(getAllCountersMapMutex());
		std::map<PYXTotalTimeCounter*,std::string> & s_allCountersMap = getAllCountersMap();
		for(std::map<PYXTotalTimeCounter*,std::string>::iterator it = s_allCountersMap.begin(); it != s_allCountersMap.end(); ++it)
		{
			message += it->second + " = " + StringUtils::toString(it->first->getTotalTime());
		}
	}
	TRACE_INFO(message);
}
