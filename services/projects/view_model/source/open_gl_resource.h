#pragma once
#ifndef VIEW_MODEL__OPEN_GL_RESOURCE_H
#define VIEW_MODEL__OPEN_GL_RESOURCE_H
/******************************************************************************
open_gl_resource.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxis\utility\object.h"

/*!

OpenGLResource - abstract base class for all OpenGLResources. 

This base class inherits from PYXObject - so you can use reference counting in order to share an object between multple classes.
Moreover, this class could be attached to OpenGLResourcePool class that allow smart resource management system.

How does this work: 
the OpenGLResource class has the following members:
  - Valid flag - used by Renderers to determine if they could use this resource. the Resource should mark it self by Valid only after it have created the OpenGL resources.
  - Usage Coutner - every time a class is been used, the usage counter increase by one. this is used by ResourcePool
  - ResourcePool - pointer to the ResourcePool that the Resource is belong to.

When the GPU resource become low, the ResourcePool can be used to release the GPU resources from all / or several of it's resources. 
This happen calling the forceRelease function of the least used resource in the pool.

when inherating a OpenGLResource class, you would have to do the following:
  - MUST IMPLEMENT - forceRelease function - this function would be called by OpenGLResourcePool. this function must call setValid(flase)
  - MUST USE	   - setValid(true) when the resource is read to used
  - MUST USE       - markUsed() every time the resource is been used. for example: when a texture is binded

*/
//!OpenGLResource - abstract base class for all OpenGLResources
class OpenGLResource : public PYXObject
{
	friend class OpenGLResourcePool;
	friend class OpenGLErrorCatcher;

public:
	virtual ~OpenGLResource(void);

	//protect from assignment and copy constructor.
protected:
	OpenGLResource(void); //can't create resource - only child class can be created

private:
	OpenGLResource(const OpenGLResource & resource) {};
	OpenGLResource & operator=(const OpenGLResource & resource) {};


//valid support
public:
	//! check if the object is valid
	inline const bool & isValid() { return m_valid; }

protected:
	//! set the Valid flag. valid object can be used at OpenGL
	void setValid(const bool & valid) { m_valid = valid; }

private:
	bool m_valid;

//usage and release support
protected:
	bool m_used;

	 //! let our resource pool know that we are bing used
	void markUsed();

public:
	//! must be implemented, release the OpenGL resources.
	virtual void forceRelease() = 0;

//Pool related interface
protected:
	OpenGLResourcePool* m_pool;

	//! get the ResourcePool of the object. throw if the object is not in a pool
	OpenGLResourcePool & getResourcePool();

	//! return true if object is inside a pool
	bool insideResourcePool() const;

	//! attach object to a resource Pool
	void attachToPool(OpenGLResourcePool & pool);

	//! detach object from his resource pool
	void detachFromPool();

//OpenGL error handling
public:
	static void clearErrors();
	static unsigned int  getLastError();
	static std::string formatError(unsigned int errorCode);
};

class OpenGLErrorCatcher
{
private:
	OpenGLResource * m_resource;
public:
	OpenGLErrorCatcher() : m_resource(nullptr)
	{
		OpenGLResource::clearErrors();
	}

	OpenGLErrorCatcher(OpenGLResource & resource) : m_resource(&resource)
	{
		OpenGLResource::clearErrors();
	}


	bool checkErrors() {
		auto errorFound = false;
		auto lastError = OpenGLResource::getLastError();
		if (lastError != GL_NO_ERROR)
		{
			errorFound = true;
			while (lastError  != GL_NO_ERROR)
			{
				TRACE_ERROR("glError: " << OpenGLResource::formatError(lastError ));
				lastError = glGetError();
			} 
			if (m_resource)
			{
				m_resource->setValid(false);
			}
		}
		return errorFound;
	}

	~OpenGLErrorCatcher()
	{
		checkErrors();
	}
};


#define OPENGL_CLEAR_ERRORS \
	{ \
		GLenum err = glGetError(); \
		while (err != GL_NO_ERROR) { \
			TRACE_ERROR("glError: " << OpenGLResource::formatError(err));  \
			err = glGetError(); \
		} \
	} 

#if defined(NDEBUG) 
#define OPENGL_CLEAR_ERRORS_DEBUG_ONLY
#else
#define OPENGL_CLEAR_ERRORS_DEBUG_ONLY OPENGL_CLEAR_ERRORS
#endif 

#endif