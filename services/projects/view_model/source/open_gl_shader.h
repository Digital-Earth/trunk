#pragma once
#ifndef VIEW_MODEL__OPEN_GL_SHADER_H
#define VIEW_MODEL__OPEN_GL_SHADER_H
/******************************************************************************
open_gl_shader.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "open_gl_resource.h"

/*!

OpenGLShader - a wapper class for GL_SHADER class of openGL.

*/
//! OpenGLShader - a wapper class for GL_SHADER class of openGL.
class OpenGLShader : public OpenGLResource
{
//public definitions
public:
	friend class OpenGLProgram;

	static const int knInvalidHandle;

	enum ShaderType //shaders types
	{
		knVertexShader = GL_VERTEX_SHADER,
		knFragmentShader = GL_FRAGMENT_SHADER
	};

//ctor and dtor
public:
	//! create a shader of ShaderType
	OpenGLShader(const ShaderType & shaderType);
	//! create a shader of ShaderType and attach code to it
	OpenGLShader(const ShaderType & shaderType,const std::string & code);
	virtual ~OpenGLShader(void); 
	
	//! release the shader from GPU
	virtual void forceRelease();

protected:
	ShaderType  m_type;
	int			m_handle;

//code support
protected:
	std::string m_code;
	void attachCodeToShader();

public:
	//! update the code of the shader. shader woudl become inValid until recompiled
	void setCode(const std::string & code);

//compile support
public:
	//! compile the shader. if there no compile errors, the shader would be mark as valid and the function return true
	bool compile();

	//! return compile error
	std::string getCompileErrors();
};

#endif