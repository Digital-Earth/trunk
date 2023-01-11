#pragma once
#ifndef VIEW_MODEL__OPEN_GL_PROGRAM_H
#define VIEW_MODEL__OPEN_GL_PROGRAM_H
/******************************************************************************
open_gl_program.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "open_gl_resource.h"
#include "open_gl_shader.h"
#include "open_gl_texture.h"

/*!

OpenGLPorgram - a wrapper class for GL_PROGRAM class of openGL.

- to attach/detach OpenGLShaders, use the attach/detach functions. you can attach only Valid shaders (which means - after it was compiled with no errors)
- to link a program - use link function
- to access uniform variables - use getUniformVariable which return a OpenGLUnifomVariable class that wrap it. assert checking for variables names
- to use a program - use the 'use/dontUse' functions

*/
//! OpenGLPorgram - a wrapper class for GL_PROGRAM class of openGL.
class OpenGLProgram : public OpenGLResource
{	
//public definitions
public:
	friend class OpenGLUniformVariable;
	friend class OpenGLAttributeVariable;
	static const int knInvalidHandle;

//ctor and dtor
public:

	//! create a program
	OpenGLProgram(void);

	//! create a program and attach a shader to it
	OpenGLProgram(OpenGLShader & shader1);

	//! create a program and attach a two shaders to it
	OpenGLProgram(OpenGLShader & shader1,OpenGLShader & shader2);

	//! create a program and attach a three shaders to it
	OpenGLProgram(OpenGLShader & shader1,OpenGLShader & shader2,OpenGLShader & shader3);

	virtual ~OpenGLProgram(void);

	//OpenGLResource support
	virtual void forceRelease();

//program handle support
protected:	
	int	 m_handle;
	void createHandle();

	//! get the handle of OpenGL program object. throws if there is no handle
	int getHandle();

//attach detach support
public:
	//! attach a shader to the program. the shader must be valid before attaching it to the program
	void attach(OpenGLShader & shader);

	//! detach a shader from the program
	void detach(OpenGLShader & shader);
	
//link support
public:
	//! link the program, if there were no link errors, the program is mark as Valid and function return true
	bool link();
	//! get link errors
	std::string getLinkErrors();

//open GL usage
public:
	//! set this program as active program in the OpenGL State Machine
	void startUsing();

	//! release the program from the OpenGL State Machine
	void stopUsing();

//uniform variables access
protected:
	//! build number. each the time program is linked, m_buildNum inc by one
	int m_buildNum;
	//! get the build number
	const int & getBuildNumber() const { return m_buildNum; }

public:
	//! Create a Uniform Variable to the Program. throws when failed to find variable name
	OpenGLUniformVariable getUniformVariable(const std::string & name);	

	//! Create a Attribute Variable to the Program. throws when failed to find variable name
	OpenGLAttributeVariable getAttributeVariable(const std::string & name);

//texture unit support
protected:
	std::map<std::string,GLenum> m_usedTextures;
	GLenum m_unsedTextureIndex;

	//! the handle of high priority texture
	int m_priorityTextureHandle;

	//! the texture unit if this UniformVariable is used as sampler with high priority.
	GLenum m_priorityTextureUnit;


	GLenum getTextureUnit(const std::string & name);
	GLenum getTextureUnit(OpenGLUniformVariable & variable);
	
	GLenum getPriorityTextureUnit();
	bool isPriorityTexture(OpenGLTexture & texture);

public:
	void definePriorityTexture(OpenGLTexture & texture);
};



/*!

OpenGLUniformVariable - helper class to set/get program uniform variables

*/

class OpenGLUniformVariable
{
public:
	OpenGLUniformVariable();
	OpenGLUniformVariable(const OpenGLUniformVariable & variable);
	OpenGLUniformVariable(OpenGLProgram & program,const std::string & name);
	virtual ~OpenGLUniformVariable() {};

	const OpenGLUniformVariable & operator=(const OpenGLUniformVariable & variable);


protected:
	//! name of the variable
	std::string m_name;
	//! gl location of the variable
	int m_location;
	
	//! the texture unit if this UniformVariable is used as sampler.
	GLenum m_textureUnit;

	//! the last value assign to UniformVariable 
	GLenum m_lastTextureUnit;

	//! the build number when the variable was created
	int m_buildNum;
	//! the program reference
	OpenGLProgram * m_program;

public:
	//! return if the variable is valid for using
	bool isValid();
	
	//! update the variable location (automatically used if the program build number has changed
	void updateLocation();

	//! get the uniform variable name
	const std::string & getName() const;

	void set(const float & value1);
	void set(const float & value1,const float & value2);
	void set(const float & value1,const float & value2,const float & value3);
	void set(const float & value1,const float & value2,const float & value3,const float & value4);

	void set(const int & value1);
	void set(const int & value1,const int & value2);
	void set(const int & value1,const int & value2,const int & value3);
	void set(const int & value1,const int & value2,const int & value3,const int & value4);	

	void get(float & value1);
	void get(float & value1, float & value2);
	void get(float & value1, float & value2, float & value3);
	void get(float & value1, float & value2, float & value3, float & value4);

	void get(int & value1);
	void get(int & value1, int & value2);
	void get(int & value1, int & value2, int & value3);
	void get(int & value1, int & value2, int & value3, int & value4);	

	void attachTexture(OpenGLTexture & texture);
};

/*!

OpenGLAttributeVariable - helper class to set/get program vertex attributes 

*/

class OpenGLAttributeVariable
{
public:
	OpenGLAttributeVariable();
	OpenGLAttributeVariable(const OpenGLAttributeVariable & varialbe);
	OpenGLAttributeVariable(OpenGLProgram & program,const std::string & name);
	virtual ~OpenGLAttributeVariable() {};

	const OpenGLAttributeVariable & operator=(const OpenGLAttributeVariable & variable);

protected:
	//! name of the variable
	std::string m_name;
	//! gl location of the variable
	int m_location;
	//! the build number when the variable was created
	int m_buildNum;
	//! the program reference
	OpenGLProgram * m_program;

public:
	//! return if the variable is valid for using
	bool isValid();
	
	//! update the variable location (automatically used if the program build number has changed
	void updateLocation();

	//! get the uniform variable name
	const std::string & getName();

	void startUsing(GLint size, GLenum type, GLboolean normalized,GLsizei stride, const GLvoid *pointer);
	void stopUsing();
};


#endif