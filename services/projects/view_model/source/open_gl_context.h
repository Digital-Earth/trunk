#pragma once
#ifndef VIEW_MODEL__OPEN_GL_CONTEXT_H
#define VIEW_MODEL__OPEN_GL_CONTEXT_H
/******************************************************************************
open_gl_context.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

//! represent a Windows OpenGLContext - not platform independent
class OpenGLContext
{
public:	
	//! created with current thread OpenGLContext - not owned
	OpenGLContext(void);

	//! copy constructor - take over ownership of the context.
	OpenGLContext(OpenGLContext & context);

	//! assignment - take over ownership of the context.
	OpenGLContext & operator=(OpenGLContext & context);

	//! destory a context. if context is owned, the openGL context would be destroyed.
	virtual ~OpenGLContext(void);

	//! get the current OpenGL context for the thread
	static OpenGLContext getCurrentContext(); 

	//! create a new OpenGL context for the calling thread that share the list of the original context
	static OpenGLContext attachToContext(const OpenGLContext & context); 

protected:
	//! Windows Decive Context handle
	HDC m_hdc;	 
	//! OpenGL Rendering Context handle
	HGLRC m_hglrc;

	//! true when the Context was created and owned by the OpenGLContext class
	bool m_ownContext;

public:

	//! swap the buffers on the Device Context
	bool swapBuffers();

	//! return true when the this context is the current OpenGLcontext for the calling thread
	bool isCurrent();

	//! distroy the context - if owned
	void dispose();

	//! bind the context to the current thread
	void startUsing();

	//! detach the context from the current thread
	void stopUsing();
};

#endif