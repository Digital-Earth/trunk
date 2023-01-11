#pragma once
#ifndef VIEW_MODEL__OPEN_GL_VBO_H
#define VIEW_MODEL__OPEN_GL_VBO_H
/******************************************************************************
open_gl_vbo.h

begin		: 2010-08-20
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "open_gl_resource.h"

/*!

OpenGLVBO- a wapper class for GL_VBO class of openGL.

*/
//! OpenGLShader - OpenGLVBO- a wapper class for GL_VBO class of openGL.
class OpenGLVBO : public OpenGLResource
{
//public definitions
public:
	static const int knInvalidHandle;

	enum VBOType //VBO types
	{
		knArrayBuffer = GL_ARRAY_BUFFER,
		knElementArrayBuffer = GL_ELEMENT_ARRAY_BUFFER
	};

	enum VBOUsage //VBO usages (only needed once)
	{
		knStatic = GL_STATIC_DRAW,
		knDynamic = GL_DYNAMIC_DRAW,
		knStream = GL_STREAM_DRAW
	};

//ctor and dtor
public:

	static PYXPointer<OpenGLVBO> create(const VBOType & vboType, const VBOUsage & vboUsage)
	{
		return PYXNEW(OpenGLVBO,vboType,vboUsage);
	}

	//! create a VBO of vboType
	OpenGLVBO(const VBOType & vboType, const VBOUsage & vboUsage);
	virtual ~OpenGLVBO(void); 
	
	//! release the VBO from GPU
	virtual void forceRelease();

protected:
	VBOType		m_type;
	VBOUsage    m_usage;
	int			m_handle;

protected:	
	void createHandle();
	int getHandle();
	void bind();

public:	
	void setData(size_t size,const void * data);
	void startUsing();
	void stopUsing();
};

#endif