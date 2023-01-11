/******************************************************************************
garbage_collector.cpp

begin		: 2010-12-29
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "garbage_collector.h"

#include "pyxis/utility/trace.h"


GarbageCollector *		GarbageCollector::m_instance = 0;
boost::recursive_mutex	GarbageCollector::m_mutex;

GarbageCollector * GarbageCollector::getInstance()
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	if (m_instance == 0)
	{
		m_instance = new GarbageCollector();
	}
	return m_instance;
}

void GarbageCollector::collect(const PYXPointer<PYXObject> & object)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_objects.push_back(object);
}

void GarbageCollector::destroyObjects()
{
	std::vector<PYXPointer<PYXObject>> collected;

	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);
		swap(m_objects,collected);
	}
}

void GarbageCollector::startDestroyObjects()
{
	m_garbageCollector.normal.beginInvokeOn(*this,&GarbageCollector::destroyObjects);
}

void GarbageCollector::waitUntilAllObjectsDestroy()
{
	m_garbageCollector.normal.invokeOn<GarbageCollector>(*this,&GarbageCollector::destroyObjects);
}
