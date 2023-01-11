/******************************************************************************
open_gl_context.cpp

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "StdAfx.h"
#include "open_gl_context.h"
#include "exceptions.h"


//! created with current thread OpenGLContext - not owned
OpenGLContext::OpenGLContext(void)
{
	m_hdc = wglGetCurrentDC();
	m_hglrc = wglGetCurrentContext();
	m_ownContext = false;
}

//! copy constructor - take over ownership of the context.
OpenGLContext::OpenGLContext(OpenGLContext & context)
{	
	m_hdc = context.m_hdc;
	m_hglrc = context.m_hglrc;

	//pass the responsibly...
	m_ownContext = context.m_ownContext;	
	context.m_ownContext = false;
}

//! assignment - take over ownership of the context.
OpenGLContext & OpenGLContext::operator=(OpenGLContext & context)
{	
	dispose();

	m_hdc = context.m_hdc;
	m_hglrc = context.m_hglrc;

	//pass the responsibly...
	m_ownContext = context.m_ownContext;	
	context.m_ownContext = false;

	return *this;
}

//! destory a context. if context is owned, the openGL context would be destoryed.
OpenGLContext::~OpenGLContext(void)
{
	dispose();
}


void OpenGLContext::dispose()
{
	//if we own the context - destory it
	if (m_ownContext)
	{
		//delete context
		wglMakeCurrent(m_hdc,0);
		wglDeleteContext(m_hglrc);
	}
}

//! get the current OpenGL context for the thread
OpenGLContext OpenGLContext::getCurrentContext()
{
	return OpenGLContext();
}

//! create a new OpenGL context for the calling thread that share the list of the original context
OpenGLContext OpenGLContext::attachToContext(const OpenGLContext & context)
{
	OpenGLContext newContext;

	//use the same HDC
	newContext.m_hdc = context.m_hdc;

	//create a new context to and share the list with original context
	newContext.m_hglrc = wglCreateContext(newContext.m_hdc);
	wglShareLists(context.m_hglrc,newContext.m_hglrc);

	//make this context the curret for this thread
	newContext.startUsing();	

	//mark that this context should be destroyed when OpenGLContext class is been destroyed
	newContext.m_ownContext = true;

	return newContext;
}

//! swap the buffers on the Device Context
bool OpenGLContext::swapBuffers()
{
	//Old drivers might have problem with swaping before finishing... baaaa
	glFinish();	
	return SwapBuffers(m_hdc) == TRUE;			
}

bool OpenGLContext::isCurrent()
{
	return wglGetCurrentContext() == m_hglrc;
}

void OpenGLContext::startUsing()
{
	//make this context the curret for this thread
	if (wglMakeCurrent(m_hdc,m_hglrc) == FALSE)
	{
		PYXTHROW(PYXException,"Can't make OpenGLContext current to current thread (Error Code: " << GetLastError() << ")");
	}
}

void OpenGLContext::stopUsing()
{
	//make this context the curret for this thread
	if (wglMakeCurrent(m_hdc,NULL) == FALSE)
	{
		PYXTHROW(PYXException,"failed to set the current OpenGL rendering context to NULL");
	}
}