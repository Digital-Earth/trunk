#pragma once
#ifndef VIEW_MODEL__OPEN_GL_RESOURCE_POOL_H
#define VIEW_MODEL__OPEN_GL_RESOURCE_POOL_H
/******************************************************************************
open_gl_resource_pool.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "open_gl_resource.h"

#include <list>

class OpenGLResource;

class OpenGLResourcePool
{
public:
	OpenGLResourcePool(void);
	virtual ~OpenGLResourcePool(void);

protected:
	typedef std::list<OpenGLResource*> ResourcePool;
	ResourcePool m_resources;

	//! helper function for foraceRelease method
	static bool orderResourcesByUsage(OpenGLResource * a,OpenGLResource * b);

	//! helper function for foraceRelease method
	static void clearResourcesUsage(OpenGLResource * a);

public:
	//! attach resource to the pool. if resource was inside other pool, it would be deatch from its old pool
	void attach(OpenGLResource & resource);

	//! mark this resource as recently used.
	void touch(OpenGLResource *resource);

	//! detach a resource from the pool. the resource would have no pool after deatching
	void detach(OpenGLResource & resource);

	//! detach all resource from pool
	void detachAll();

	//! get size of all resource
	const int size() const;

	//! get count of all valid resources.
	const int getValidCount();

//auto release support
public:
	//! release amount of resources from GPU
	void forceRelease(unsigned int amount=1);

	//! release all resource inside pool
	void forceReleaseAll();

};

#endif