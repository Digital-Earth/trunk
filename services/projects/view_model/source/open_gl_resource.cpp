/******************************************************************************
open_gl_resource.cpp

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "open_gl_resource.h"
#include "open_gl_resource_pool.h"

#include "pyxis/utility/trace.h"

OpenGLResource::OpenGLResource(void) : m_valid(false), m_used(false), m_pool(NULL)
{
}

OpenGLResource::~OpenGLResource(void)
{
	detachFromPool();
}

OpenGLResourcePool & OpenGLResource::getResourcePool()
{
	assert(m_pool != NULL && "OpenGLResource is assigend to a pool");
	return *m_pool;
}

void OpenGLResource::markUsed()
{
	m_used=true;	
}

void OpenGLResource::attachToPool(OpenGLResourcePool & pool)
{
	detachFromPool();
	pool.attach(*this);
}

void OpenGLResource::detachFromPool()
{
	if (insideResourcePool())
	{
		m_pool->detach(*this);		
	}
}

bool OpenGLResource::insideResourcePool() const
{
	return m_pool != NULL;
}

void OpenGLResource::clearErrors()
{
	OPENGL_CLEAR_ERRORS;
}

unsigned int OpenGLResource::getLastError()
{
	return glGetError();
}

std::string OpenGLResource::formatError(unsigned int errorCode)
{
	return (char*)gluErrorString(errorCode);
}