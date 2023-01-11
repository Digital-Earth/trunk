#include "StdAfx.h"
#include "open_gl_resource_pool.h"

#include <boost/scoped_array.hpp>

#include <cassert>

/******************************************************************************
open_gl_resource_pool.cpp

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/


OpenGLResourcePool::OpenGLResourcePool(void)
{
}

OpenGLResourcePool::~OpenGLResourcePool(void)
{	
	detachAll();
}


void OpenGLResourcePool::attach(OpenGLResource & resource)
{
	if (std::find(m_resources.begin(), m_resources.end(), &resource) != m_resources.end())
	{
		//resource allready inside the pool
		return;	
	}

	if (resource.insideResourcePool())
		resource.getResourcePool().detach(resource);

	//add the resource to the pool
	m_resources.push_front(&resource);	
	
	//mark the resource to point to this pool
	resource.m_pool = this;
}

void OpenGLResourcePool::touch(OpenGLResource *resource)
{
	ResourcePool::iterator item = std::find(m_resources.begin(), m_resources.end(), resource);
	if(item != m_resources.end())
	{
		// remove resource from pool
		m_resources.erase(item);
		// and put it back in the front of the list
		m_resources.push_front(resource);	
	}
}

void OpenGLResourcePool::detach(OpenGLResource & resource)
{
	ResourcePool::iterator item = std::find(m_resources.begin(), m_resources.end(), &resource);
	assert(item != m_resources.end() && "The item to detach must be in the list.");
	resource.m_pool = NULL;
	m_resources.erase(item);
}
	
void OpenGLResourcePool::detachAll()
{
	ResourcePool::iterator item = m_resources.begin();

	while(item != m_resources.end())
	{		
		OpenGLResource & currentItem = (**item);
		currentItem.m_pool = NULL;
		item++;
	}

	m_resources.clear();
}

const int OpenGLResourcePool::size() const
{
	return m_resources.size();
}

const int OpenGLResourcePool::getValidCount() 
{
	int validCount = 0;

	ResourcePool::iterator item = m_resources.begin();

	while(item != m_resources.end())
	{
		if ((**item).isValid())
		{
			validCount++;
		}
		item++;
	}

	return validCount;
}


bool OpenGLResourcePool::orderResourcesByUsage(OpenGLResource * a,OpenGLResource * b)
{
	//order a before b if a was used and b wasn't
	return a->m_used && ! b->m_used;
}

void OpenGLResourcePool::clearResourcesUsage(OpenGLResource * a)
{
	a->m_used = false;
}

//release amount of resources from GPU
void OpenGLResourcePool::forceRelease(unsigned int amount)
{
	assert(m_resources.size() >= amount && "can't find resources because pool is empty");

	m_resources.sort(&OpenGLResourcePool::orderResourcesByUsage);	

	while ((amount > 0)  && (m_resources.size() > 0))
	{
		OpenGLResource* resource = m_resources.back();
		resource->forceRelease();
		resource->m_pool = NULL;
		m_resources.pop_back();
		--amount;
	}

	std::for_each(m_resources.begin(),m_resources.end(),&OpenGLResourcePool::clearResourcesUsage);
}

//release all resource inside pool
void OpenGLResourcePool::forceReleaseAll()
{
	ResourcePool::iterator item = m_resources.begin();
	while(item != m_resources.end())
	{
		OpenGLResource & resource = **item;
		resource.forceRelease();
		item++;
	}
}
