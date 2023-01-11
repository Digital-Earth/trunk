/******************************************************************************
open_gl_program.cpp

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "open_gl_program.h"

#include "gl_utils.h"
#include "exceptions.h"

#include <boost/scoped_array.hpp>

#include <cassert>
#include <list>
#include <vector>

const int OpenGLProgram::knInvalidHandle = -1;

OpenGLProgram::OpenGLProgram(void) : m_handle(knInvalidHandle), m_unsedTextureIndex(GL_TEXTURE0), m_priorityTextureUnit(-1), m_priorityTextureHandle(-1)
{
}

OpenGLProgram::OpenGLProgram(OpenGLShader & shader1)  : m_handle(knInvalidHandle), m_unsedTextureIndex(GL_TEXTURE0), m_priorityTextureUnit(-1), m_priorityTextureHandle(-1)
{
	attach(shader1);
}
OpenGLProgram::OpenGLProgram(OpenGLShader & shader1,OpenGLShader & shader2)  : m_handle(knInvalidHandle), m_unsedTextureIndex(GL_TEXTURE0), m_priorityTextureUnit(-1), m_priorityTextureHandle(-1)
{
	attach(shader1);
	attach(shader2);
}

OpenGLProgram::OpenGLProgram(OpenGLShader & shader1,OpenGLShader & shader2,OpenGLShader & shader3)  : m_handle(knInvalidHandle), m_unsedTextureIndex(GL_TEXTURE0), m_priorityTextureUnit(-1), m_priorityTextureHandle(-1)
{
	attach(shader1);
	attach(shader2);
	attach(shader3);
}


OpenGLProgram::~OpenGLProgram(void)
{
	forceRelease();
}

void OpenGLProgram::forceRelease()
{
	if (m_handle != knInvalidHandle)
	{
		m_usedTextures.clear();
		m_unsedTextureIndex = GL_TEXTURE0;

		glDeleteProgram(m_handle);
		m_handle = knInvalidHandle;
		setValid(false);
	}
}


void OpenGLProgram::createHandle()
{
	if (m_handle == knInvalidHandle)
	{
		m_handle = glCreateProgram();
	}
}

int OpenGLProgram::getHandle()
{
	if (m_handle == knInvalidHandle)
	{
		PYXTHROW(PYXException,"OpenGLProgram doesn't have an handle");
	}

	return m_handle;
}

void OpenGLProgram::attach(OpenGLShader & shader)
{
	assert(shader.isValid() && "shader is not valid - can't be attached");

	if (! shader.isValid())
	{
		PYXTHROW(PYXException,"shader is not valid - can't be attached"); 
	}

	createHandle();
	
	glAttachShader(m_handle,shader.m_handle);
}

void OpenGLProgram::detach(OpenGLShader & shader)
{
	assert(m_handle != knInvalidHandle && "program doesn't have an handle");
	
	if (shader.m_handle != knInvalidHandle)
	{
		glDetachShader(m_handle,shader.m_handle);
	}
}
	
bool OpenGLProgram::link()
{
	//usage checking
	assert(m_handle != knInvalidHandle && "program doesn't have an handle, can't link");

	//clear texture unit usage
	m_usedTextures.clear();
	m_unsedTextureIndex = GL_TEXTURE0;

	//usage checking - do we have shaders attached to porgram?
	int attachedShadersCount = 0;
	glGetProgramiv(m_handle,GL_ATTACHED_SHADERS,&attachedShadersCount);
	assert(attachedShadersCount && "not shaders attached to program, can't link");

	setValid(false);

	if (attachedShadersCount == 0)
	{
		return false;
	}	
	
	//inc build number
	m_buildNum++;

	//link the program
	glLinkProgram(m_handle);

	//check for errors
	int linkedOk;
	glGetProgramiv(m_handle,GL_LINK_STATUS,&linkedOk);

	//set valid flag
	setValid(linkedOk == GL_TRUE);	

	return isValid();
}

std::string OpenGLProgram::getLinkErrors()
{
	assert(m_handle != knInvalidHandle && "can't retrive compile errors because shader was not created");

	//this mean that the shader is ok - there would be no errors
	if (isValid())
	{
		return "";
	}
	
	int logLength = 0;
	int accualLength = 0;

	glGetProgramiv(m_handle,GL_INFO_LOG_LENGTH,&logLength);
	
	//no erros returned...
	if (logLength == 0)
	{
		return "";
	}

	//alocate memory
	boost::scoped_array<char> log(new char[logLength+1]);

	//get string data
	glGetProgramInfoLog(m_handle,logLength,&accualLength,log.get());

	return std::string(log.get());
}

void OpenGLProgram::startUsing()
{
	assert(isValid() && "program is not valid - can't use it");
	
	if (! isValid())
	{
		PYXTHROW(PYXException,"program is not valid - can't use it"); 
	}

	glUseProgram(m_handle);

	markUsed();
}

void OpenGLProgram::stopUsing()
{
	glUseProgram(0);
}

OpenGLUniformVariable OpenGLProgram::getUniformVariable(const std::string & name) 
{ 
	return OpenGLUniformVariable(*this,name); 
}

OpenGLAttributeVariable OpenGLProgram::getAttributeVariable(const std::string & name) 
{ 
	return OpenGLAttributeVariable(*this,name);
}


GLenum OpenGLProgram::getTextureUnit(const std::string & name)
{
	if (m_usedTextures.find(name) == m_usedTextures.end())
	{
		m_usedTextures[name] = m_unsedTextureIndex;
		m_unsedTextureIndex++;
	}
	return m_usedTextures[name];
}

GLenum OpenGLProgram::getTextureUnit(OpenGLUniformVariable & variable)
{
	return getTextureUnit(variable.getName());	
}

void OpenGLProgram::definePriorityTexture(OpenGLTexture & texture)
{	
	if (m_priorityTextureUnit == -1)
	{
		m_priorityTextureUnit = getTextureUnit("[priority]");
	}
	m_priorityTextureHandle = texture.getHandle();	
	glActiveTexture(m_priorityTextureUnit);	
	texture.startUsing();
}

GLenum OpenGLProgram::getPriorityTextureUnit() {
	return m_priorityTextureUnit;
}

bool OpenGLProgram::isPriorityTexture(OpenGLTexture & texture)
{
	return m_priorityTextureHandle == texture.getHandle();	
}

/////////////////////////////////////////////////////////////////////////////////////////////

OpenGLUniformVariable::OpenGLUniformVariable() : m_program(NULL),m_buildNum(0),m_location(-1),m_textureUnit(-1),m_lastTextureUnit(-1)
{
}

//copy ctor
OpenGLUniformVariable::OpenGLUniformVariable(const OpenGLUniformVariable & variable) : 
	m_program(variable.m_program), m_location(variable.m_location), m_buildNum(variable.m_buildNum), m_name(variable.m_name), m_textureUnit(variable.m_textureUnit), m_lastTextureUnit(variable.m_lastTextureUnit)
{ 
}

//normal ctor
OpenGLUniformVariable::OpenGLUniformVariable(OpenGLProgram & program,const std::string & name) : m_program(&program), m_buildNum(program.getBuildNumber()), m_name(name)
{
	updateLocation();	
}

const OpenGLUniformVariable & OpenGLUniformVariable::operator=(const OpenGLUniformVariable & variable)
{
	m_program = variable.m_program;
	m_buildNum = variable.m_buildNum;
	m_name = variable.m_name;
	m_location = variable.m_location;
	m_textureUnit = variable.m_textureUnit;
	m_lastTextureUnit = variable.m_lastTextureUnit;
	return *this;
}

bool OpenGLUniformVariable::isValid()
{
	if (m_program == NULL)
	{
		return false;
	}

	if (m_program->isValid())
	{
		if (m_program->getBuildNumber() != m_buildNum)
		{
			updateLocation();
		}
		return true;
	}
	return false;
}
	
const std::string & OpenGLUniformVariable::getName() const
{
	return m_name;
}

void OpenGLUniformVariable::updateLocation()
{
	if (m_program==NULL || !m_program->isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}
	m_location = glGetUniformLocation(m_program->getHandle(),m_name.c_str());	
	m_buildNum = m_program->getBuildNumber();
	m_textureUnit = -1;
	m_lastTextureUnit = -1;

	if (m_location == -1)
	{
		PYXTHROW(PYXException,"can't find uniform variable program, variable name:" + m_name);
	}
}

// set functions
//////////////////////////////////

void OpenGLUniformVariable::set(const float & value1)
{
	assert(m_location != -1 && "can't find uniform variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}

	glUniform1f(m_location,value1);
}

void OpenGLUniformVariable::set(const float & value1,const float & value2)
{
	assert(m_location != -1 && "can't find uniform variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}

	glUniform2f(m_location,value1,value2);
}

void OpenGLUniformVariable::set(const float & value1,const float & value2,const float & value3)	
{
	assert(m_location != -1 && "can't find uniform variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}
	glUniform3f(m_location,value1,value2,value3);
}

void OpenGLUniformVariable::set(const float & value1,const float & value2,const float & value3,const float & value4)
{
	assert(m_location != -1 && "can't find uniform variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}
	glUniform4f(m_location,value1,value2,value3,value4);
}

void OpenGLUniformVariable::set(const int & value1)
{
	assert(m_location != -1 && "can't find uniform variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}
	glUniform1i(m_location,value1);
}

void OpenGLUniformVariable::set(const int & value1,const int & value2)
{
	assert(m_location != -1 && "can't find uniform variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}
	glUniform2i(m_location,value1,value2);
}

void OpenGLUniformVariable::set(const int & value1,const int & value2,const int & value3)
{
	assert(m_location != -1 && "can't find uniform variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}
	glUniform3i(m_location,value1,value2,value3);
}

void OpenGLUniformVariable::set(const int & value1,const int & value2,const int & value3,const int & value4)
{
	assert(m_location != -1 && "can't find uniform variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}
	glUniform4i(m_location,value1,value2,value3,value4);
}

// get functions
//////////////////////////////////

void OpenGLUniformVariable::get(float & value1)
{
	assert(m_location != -1 && "can't find uniform variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}
	glGetUniformfv(m_program->m_handle,m_location,&value1);
}

void OpenGLUniformVariable::get(float & value1, float & value2)
{
	assert(m_location != -1 && "can't find uniform variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}
	float values[2];
	glGetUniformfv(m_program->m_handle,m_location,values);
	value1 = values[0];
	value2 = values[1];
}

void OpenGLUniformVariable::get(float & value1, float & value2, float & value3)
{
	assert(m_location != -1 && "can't find uniform variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}
	float values[3];
	glGetUniformfv(m_program->m_handle,m_location,values);
	value1 = values[0];
	value2 = values[1];
	value3 = values[2];
	
}

void OpenGLUniformVariable::get(float & value1, float & value2, float & value3, float & value4)
{
	assert(m_location != -1 && "can't find uniform variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}
	float values[4];
	glGetUniformfv(m_program->m_handle,m_location,values);
	value1 = values[0];
	value2 = values[1];
	value3 = values[2];
	value4 = values[3];

}

void OpenGLUniformVariable::get(int & value1)
{
	assert(m_location != -1 && "can't find uniform variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}
	glGetUniformiv(m_program->m_handle,m_location,&value1);
}

void OpenGLUniformVariable::get(int & value1, int & value2)
{
	assert(m_location != -1 && "can't find uniform variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}
	int values[2];
	glGetUniformiv(m_program->m_handle,m_location,values);
	value1 = values[0];
	value2 = values[1];
}

void OpenGLUniformVariable::get(int & value1, int & value2, int & value3)
{
	assert(m_location != -1 && "can't find uniform variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}
	int values[3];
	glGetUniformiv(m_program->m_handle,m_location,values);
	value1 = values[0];
	value2 = values[1];
	value3 = values[2];
}
void OpenGLUniformVariable::get(int & value1, int & value2, int & value3, int & value4)
{
	assert(m_location != -1 && "can't find uniform variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}
	int values[4];
	glGetUniformiv(m_program->m_handle,m_location,values);
	value1 = values[0];
	value2 = values[1];
	value3 = values[2];
	value4 = values[3];
}


// attachTexture functions
//////////////////////////////////

void OpenGLUniformVariable::attachTexture(OpenGLTexture & texture)
{
	assert(m_location != -1 && "can't find uniform variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use uniform variable location");
	}

	if (m_program->isPriorityTexture(texture)) {
		auto priorityTextureUnit = m_program->getPriorityTextureUnit();		
		if (m_lastTextureUnit != priorityTextureUnit) {
			m_lastTextureUnit = priorityTextureUnit;
 			set((int)(priorityTextureUnit-GL_TEXTURE0));
		}
		//priority texture is already been attached
	}
	else 
	{
		if (m_textureUnit == -1)
		{
			m_textureUnit = m_program->getTextureUnit(*this);				
		}
		if (m_lastTextureUnit != m_textureUnit) {
			m_lastTextureUnit = m_textureUnit;
			set((int)(m_textureUnit-GL_TEXTURE0));
		}

		glActiveTexture(m_textureUnit);	
		texture.startUsing();
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////

//default ctor
OpenGLAttributeVariable::OpenGLAttributeVariable() :
	m_program(NULL) , m_location(-1), m_buildNum(0), m_name()
{
}

//copy ctor
OpenGLAttributeVariable::OpenGLAttributeVariable(const OpenGLAttributeVariable & variable) : 
	m_program(variable.m_program) , m_location(variable.m_location), m_buildNum(variable.m_buildNum), m_name(variable.m_name)
{ 
}

//normal ctor
OpenGLAttributeVariable::OpenGLAttributeVariable(OpenGLProgram & program,const std::string & name) : m_program(&program), m_buildNum(program.getBuildNumber()), m_name(name)
{
	updateLocation();	
}

const OpenGLAttributeVariable & OpenGLAttributeVariable::operator=(const OpenGLAttributeVariable & variable)
{
	m_program = variable.m_program;
	m_location = variable.m_location;
	m_buildNum = variable.m_buildNum;
	m_name = variable.m_name;

	return *this;
}


bool OpenGLAttributeVariable::isValid()
{
	if (m_program == NULL)
	{
		return false;
	}
	
	if (m_program->isValid())
	{
		if (m_program->getBuildNumber() != m_buildNum)
		{
			updateLocation();
		}
		return true;
	}
	return false;
}
	
const std::string & OpenGLAttributeVariable::getName()
{
	return m_name;
}

void OpenGLAttributeVariable::updateLocation()
{
	if (!m_program->isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use attribute variable location");
	}
	m_location = glGetAttribLocation(m_program->getHandle(),m_name.c_str());	
	m_buildNum = m_program->getBuildNumber();

	if (m_location == -1)
	{
		PYXTHROW(PYXException,"can't find attribute variable program, variable name:" + m_name);
	}
}

void OpenGLAttributeVariable::startUsing(GLint size, GLenum type, GLboolean normalized,GLsizei stride, const GLvoid *pointer)
{
	assert(m_location != -1 && "can't find attribute variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use attribute variable location");
	}

	glEnableVertexAttribArray(m_location);
	glVertexAttribPointer(m_location,size,type,normalized,stride,pointer);
}

void OpenGLAttributeVariable::stopUsing()
{
	assert(m_location != -1 && "can't find attribute variable program");
	if (!isValid())
	{
		PYXTHROW(PYXException,"Program is not valid anymore, can't use attribute variable location");
	}

	glDisableVertexAttribArray(m_location);
}