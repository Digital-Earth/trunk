/******************************************************************************
open_gl_vbo.cpp

begin		: 2010-08-20
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "open_gl_vbo.h"

#include "gl_utils.h"

#include <boost/scoped_array.hpp>

#include <cassert>
#include <list>
#include <vector>


const int OpenGLVBO::knInvalidHandle = -1;

OpenGLVBO::OpenGLVBO(const VBOType & vboType, const VBOUsage & vboUsage) : m_handle(knInvalidHandle), m_type(vboType), m_usage(vboUsage)
{	
}

OpenGLVBO::~OpenGLVBO(void)
{	
	forceRelease();
}

void OpenGLVBO::createHandle()
{
	if (m_handle == knInvalidHandle)
	{
		glGenBuffers(1,(GLuint*)&m_handle);
		glBindBuffer(m_type, m_handle);		
	}
}

void OpenGLVBO::forceRelease()
{
	if (m_handle != knInvalidHandle)
	{
		glDeleteBuffers(1,(GLuint*)&m_handle);
		m_handle = knInvalidHandle;
		setValid(false);
	}	
}

int OpenGLVBO::getHandle()
{
	if (m_handle == knInvalidHandle)
	{
		PYXTHROW(PYXException,"OpenGLVBO doesn't have an handle");
	}

	return m_handle;
}

void OpenGLVBO::bind()
{
	if (m_handle == knInvalidHandle)
	{
		createHandle();
	}
	glBindBuffer(m_type, m_handle);
}

void OpenGLVBO::setData(size_t size,const void * data)
{
	bind();
	glBufferData(m_type,size,data,m_usage);
	setValid(true);
	stopUsing();
}

void OpenGLVBO::startUsing()
{
	assert(isValid() && "VBO is not valid - can't use it");
	
	if (! isValid())
	{
		PYXTHROW(PYXException,"VBO is not valid - can't use it"); 
	}

	bind();

	markUsed();
}

void OpenGLVBO::stopUsing()
{
	glBindBuffer(m_type, 0);	
}

