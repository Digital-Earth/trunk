#pragma once
#ifndef VIEW_MODEL__GARBAGE_COLLECTOR_H
#define VIEW_MODEL__GARBAGE_COLLECTOR_H
/******************************************************************************
garbage_collector.h

begin		: 2010-12-29
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxis\utility\object.h"
#include "sync_context.h"

#include <boost/thread/recursive_mutex.hpp>

#include <set>

class GarbageCollector 
{
private:
	static GarbageCollector *			m_instance;
	static boost::recursive_mutex		m_mutex;

private:
	Sync::WorkerThreadContext			m_garbageCollector;	
	std::vector<PYXPointer<PYXObject>>	m_objects;

private:
	GarbageCollector() {};
	GarbageCollector(const GarbageCollector & other) {};

protected:
	void destroyObjects();

public:
	void collect(const PYXPointer<PYXObject> & object);

	void startDestroyObjects();

	void waitUntilAllObjectsDestroy();

public:
	static GarbageCollector * getInstance();
};

#endif
