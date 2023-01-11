/******************************************************************************
open_gl_shader.cpp

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "open_gl_shader.h"

#include "gl_utils.h"

#include <boost/scoped_array.hpp>

#include <cassert>
#include <list>
#include <vector>


const int OpenGLShader::knInvalidHandle = -1;

OpenGLShader::OpenGLShader(const ShaderType & shaderType) : m_handle(knInvalidHandle), m_code("")
{
	m_type = shaderType;
}

OpenGLShader::OpenGLShader(const ShaderType & shaderType,const std::string & code) : m_handle(knInvalidHandle)
{
	m_type = shaderType;
	m_code = code;
}

OpenGLShader::~OpenGLShader(void)
{	
	forceRelease();
}

void OpenGLShader::setCode(const std::string & code)
{
	m_code = code;
	setValid(false);
}

void OpenGLShader::forceRelease()
{
	if (m_handle != knInvalidHandle)
	{
		glDeleteShader(m_handle);
		m_handle = knInvalidHandle;
		setValid(false);		
	}	
}

bool OpenGLShader::compile()
{
	setValid(false);
	
	if (m_handle == knInvalidHandle)
	{
		m_handle = glCreateShader(m_type);
	}

	if (m_handle != knInvalidHandle)
	{
		attachCodeToShader();

		glCompileShader(m_handle);

		int compiledOk;
		glGetShaderiv(m_handle,GL_COMPILE_STATUS,&compiledOk);

		setValid(compiledOk == GL_TRUE);
	}

	return isValid();
}

void OpenGLShader::attachCodeToShader()
{
	assert(m_handle != knInvalidHandle && "can't attach code because shader was not created");

	//copy the string - because we need to split it.
	boost::scoped_array<char> code(new char[m_code.length()+1]);
	memcpy(code.get(),m_code.c_str(),m_code.length()+1);

	std::list<int> lineStartIndex;

	//first line start at index 0.
	lineStartIndex.push_back(0);

	//change '\n' into null terminated string	
	for(size_t i=0;i<m_code.length();i++)
	{
		if (code[i] == '\n')
		{
			//split the string.
			code[i] = 0;
			
			//push a start index to the next line (if there would be a next line)
			if (i+1<m_code.length())
			{
				lineStartIndex.push_back(i+1);
			}
		}
	}

	std::vector<char *> lineString(lineStartIndex.size());
	std::vector<int> lineLength(lineStartIndex.size());
	
	//to make line length computation much more easy... (next_line_start-current_line_start-1)
	lineStartIndex.push_back(m_code.length());
	std::list<int>::iterator index = lineStartIndex.begin();

	//populate values lineString and lineLength vectors	
	for(  size_t i=0;i<lineString.size();i++)
	{
		int current_index = *index;
		index++;
		lineString[i] = code.get() + current_index; //where the string start at memory
		lineLength[i] = *index - current_index - 1; //the length of the line
	}

	glShaderSource(m_handle,lineString.size(),const_cast<const GLchar**>(&lineString[0]),const_cast<const GLint*>(&lineLength[0]));
}

std::string OpenGLShader::getCompileErrors()
{
	assert(m_handle != knInvalidHandle && "can't retrieve compile errors because shader was not created");

	//this mean that the shader is ok - there would be no errors
	if (isValid())
	{
		return "";
	}
	
	int logLength = 0;
	int accualLength = 0;

	glGetShaderiv(m_handle,GL_INFO_LOG_LENGTH,&logLength);
	
	//no errors returned...
	if (logLength == 0)
	{
		return "";
	}

	//allocate memory
	boost::scoped_array<char> log(new char[logLength+1]);

	glGetShaderInfoLog(m_handle,logLength,&accualLength,log.get());

	return std::string(log.get());
}