/******************************************************************************
open_gl_texture.cpp

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "open_gl_texture.h"
#include "performance_counter.h"

#include "pyxis/utility/trace.h"
#include "exceptions.h"

const int OpenGLTexture::knInvalidHandle = -1;

OpenGLTexture::OpenGLTexture(const TextureType & textureType,const TextureFormat & textureFormat) : 
	m_handle(knInvalidHandle), 
	m_type(textureType), 
	m_format(textureFormat),
	m_memoryUsed(0)
{
	PerformanceCounter::getValuePerformanceCounter("Textures",0.0f,0.0f,1.0f)->addToMeasurement(1);
}

OpenGLTexture::~OpenGLTexture(void)
{
	PerformanceCounter::getValuePerformanceCounter("Textures",0.0f,0.0f,1.0f)->addToMeasurement(-1);
	forceRelease();
}

void OpenGLTexture::forceRelease()
{
	if (m_handle != knInvalidHandle)
	{
		glDeleteTextures(1,(GLuint*)&m_handle);
		m_handle = knInvalidHandle;
		setValid(false);
	}
	if (m_memoryUsed>0)
	{
		releaseMemory(m_memoryUsed);
		m_memoryUsed=0;
	}
}

void OpenGLTexture::createHandle()
{
	if (m_handle == knInvalidHandle)
	{
		glGenTextures(1,(GLuint*)&m_handle);
		glBindTexture(m_type, m_handle);
	}
}

int OpenGLTexture::getHandle()
{
	if (m_handle == knInvalidHandle)
	{
		PYXTHROW(PYXException,"OpenGLTexture doesn't have an handle");
	}

	return m_handle;
}

void OpenGLTexture::bind()
{
	if (m_handle == knInvalidHandle)
	{
		createHandle();
	}

	glBindTexture(m_type,m_handle);
}

void OpenGLTexture::startUsing()
{
	assert(isValid() && "texture is not valid - can't use it");
	
	if (! isValid())
	{
		PYXTHROW(PYXException,"texture is not valid - can't use it"); 
	}

	bind();

	markUsed();
}

void OpenGLTexture::stopUsing()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLTexture::setWarp(const TextureWarpMode & mode)
{
	bind();
	
	glTexParameterf(m_type,GL_TEXTURE_WRAP_S,static_cast<GLfloat>(mode));

	if (m_type == knTexture1D)
	{
		stopUsing();
		return;
	}

	glTexParameterf(m_type,GL_TEXTURE_WRAP_T,static_cast<GLfloat>(mode));

	if (m_type == knTexture2D)
	{
		stopUsing();
		return;
	}	
	glTexParameterf(m_type,GL_TEXTURE_WRAP_R,static_cast<GLfloat>(mode));

	stopUsing();
}

void OpenGLTexture::setWarp(const TextureWarpAxis & axis,const TextureWarpMode & mode)
{
	bind();

	glTexParameterf(m_type,static_cast<GLenum>(axis),static_cast<GLfloat>(mode));

	stopUsing();
}

void OpenGLTexture::setMinFilter(const TextureMinFilter & minFilter)
{
	bind();

	glTexParameterf(m_type,GL_TEXTURE_MIN_FILTER,static_cast<GLfloat>(minFilter));

	stopUsing();
}

void OpenGLTexture::setMagFilter(const TextureMagFilter & magFilter)
{
	bind();

	glTexParameterf(m_type,GL_TEXTURE_MIN_FILTER,static_cast<GLfloat>(magFilter));

	stopUsing();
}

unsigned int OpenGLTexture::getWidth()
{
	return m_width;
}

unsigned int OpenGLTexture::getHeight()
{
	return m_height;
}

void OpenGLTexture::setSize(unsigned int width,unsigned int height,const DataFormat & format,const DataType & type)
{
	m_width = width;
	m_height = height;

	bind();

	glTexImage2D(m_type,0,m_format,width,height,0,format,type,NULL);	

	int usedMemory = width*height*4;
	if (m_memoryUsed != usedMemory)
	{
		consumeMemory(usedMemory-m_memoryUsed);
		m_memoryUsed=usedMemory;
	}
	setValid(true);
	
	stopUsing();
}


void OpenGLTexture::generateMipmap()
{
	bind();

	glTexParameterf(m_type,GL_GENERATE_MIPMAP,GL_TRUE);

	stopUsing();

	setValid(true);
}

void OpenGLTexture::setMipLevelData(unsigned int mipLevel, unsigned int width,unsigned int height,const DataFormat & format,const DataType & type,const void * data)
{
	bind();

	glTexImage2D(m_type,mipLevel,m_format,width,height,0,format,type,data);	

	stopUsing();

	setValid(true);
}


void OpenGLTexture::setMipLevelData(unsigned int mipLevel, unsigned int width,unsigned int height,const DataFormat & format,const unsigned char * data)
{
	setMipLevelData(mipLevel,width,height,format,knTextelUnsignedByte,data);
}

void OpenGLTexture::setMipLevelData(unsigned int mipLevel, unsigned int width,unsigned int height,const DataFormat & format,const int * data)
{
	setMipLevelData(mipLevel,width,height,format,knTextelInt,data);
}

void OpenGLTexture::setMipLevelData(unsigned int mipLevel, unsigned int width,unsigned int height,const DataFormat & format,const float * data)
{
	setMipLevelData(mipLevel,width,height,format,knTextelFloat,data);
}


void OpenGLTexture::setData(unsigned int width,unsigned int height,const DataFormat & format,const DataType & type,const void * data)
{
	m_width = width;
	m_height = height;

	bind();

	glTexImage2D(m_type,0,m_format,width,height,0,format,type,data);	

	int usedMemory = width*height*4;
	if (m_memoryUsed != usedMemory)
	{
		consumeMemory(usedMemory-m_memoryUsed);
		m_memoryUsed=usedMemory;
	}
	setValid(true);

	stopUsing();
}

void OpenGLTexture::setData(unsigned int width,unsigned int height,const DataFormat & format,const unsigned char * data)
{
	setData(width,height,format,knTextelUnsignedByte,data);
}

void OpenGLTexture::setData(unsigned int width,unsigned int height,const DataFormat & format,const int * data)
{
	setData(width,height,format,knTextelInt,data);
}

void OpenGLTexture::setData(unsigned int width,unsigned int height,const DataFormat & format,const float * data)
{
	setData(width,height,format,knTextelFloat,data);
}


void OpenGLTexture::setDataRegion(int x, int y, unsigned int width, unsigned int height, const DataFormat & format, const DataType & type, const void * data)
{
	assert(isValid() && "texture is not valid - can't update a region");
	
	if (! isValid())
	{
		PYXTHROW(PYXException,"texture is not valid - can't update a region"); 
	}

	bind();

	glTexSubImage2D(m_type,0,x,y,width,height,format,type,data);

	stopUsing();
}

void OpenGLTexture::setDataRegion(int x, int y, unsigned int width, unsigned int height, const DataFormat & format, const unsigned char * data)
{
	setDataRegion(x,y,width,height,format,knTextelUnsignedByte,data);
}

void OpenGLTexture::setDataRegion(int x, int y, unsigned int width, unsigned int height, const DataFormat & format, const int * data)
{
	setDataRegion(x,y,width,height,format,knTextelInt,data);
}

void OpenGLTexture::setDataRegion(int x, int y, unsigned int width, unsigned int height, const DataFormat & format, const float * data)
{
	setDataRegion(x,y,width,height,format,knTextelFloat,data);
}

bool OpenGLTexture::canHaveTextureWithNonPowerOfTwo()
{
	return GLEE_ARB_texture_non_power_of_two == GL_TRUE;
}

bool OpenGLTexture::canSupportMultitextures()
{
	return GLEE_ARB_multitexture == GL_TRUE;
}

//////////////////////////////////////////////////////////////////
//
// OpenGLTextureArray
//
//////////////////////////////////////////////////////////////////

OpenGLTextureArray::OpenGLTextureArray(int length, const OpenGLTexture::TextureType & textureType,const OpenGLTexture::TextureFormat & textureFormat) : m_textures(length), m_type(textureType), m_format(textureFormat)
{
	for (int i=0;i<length;i++)
	{
		m_textures[i] = PYXNEW(OpenGLTexture,m_type,m_format);
	}
}

int OpenGLTextureArray::size() 
{ 
	return m_textures.size(); 
}

PYXPointer<OpenGLTexture> OpenGLTextureArray::operator[](const int & index) 
{ 
	return m_textures[index]; 
}

PYXPointer<OpenGLTexture> OpenGLTextureArray::addNewTexture()
{
	PYXPointer<OpenGLTexture> newTexture = PYXNEW(OpenGLTexture,m_type,m_format);
	m_textures.push_back(newTexture);
	return newTexture;
}

void OpenGLTextureArray::remove(const int & index)
{
	m_textures.erase(m_textures.begin()+index);
}

bool OpenGLTextureArray::isAllValid()
{
	for (OpenGLTexturePointerVector::iterator it = m_textures.begin();it != m_textures.end();++it)
	{
		if (!(**it).isValid())
		{
			return false;
		}
	}
	return true;
}


void OpenGLTextureArray::setAllWarp(const OpenGLTexture::TextureWarpMode & mode)
{
	for (OpenGLTexturePointerVector::iterator it = m_textures.begin();it != m_textures.end();++it)
	{
		(**it).setWarp(mode);
	}
}

void OpenGLTextureArray::setAllWarp(const OpenGLTexture::TextureWarpAxis & axis,const OpenGLTexture::TextureWarpMode & mode)
{
	for (OpenGLTexturePointerVector::iterator it = m_textures.begin();it != m_textures.end();++it)
	{
		(**it).setWarp(axis,mode);
	}
}

void OpenGLTextureArray::setAllMinFilter(const OpenGLTexture::TextureMinFilter & minFilter)
{
	for (OpenGLTexturePointerVector::iterator it = m_textures.begin();it != m_textures.end();++it)
	{
		(**it).setMinFilter(minFilter);
	}
}

void OpenGLTextureArray::setAllMagFilter(const OpenGLTexture::TextureMagFilter & magFilter)
{
	for (OpenGLTexturePointerVector::iterator it = m_textures.begin();it != m_textures.end();++it)
	{
		(**it).setMagFilter(magFilter);
	}
}

void OpenGLTextureArray::setAllSize(unsigned int width,unsigned int height,const OpenGLTexture::DataFormat & format,const OpenGLTexture::DataType & type)
{
	for (OpenGLTexturePointerVector::iterator it = m_textures.begin();it != m_textures.end();++it)
	{
		(**it).setSize(width,height,format,type);
	}
}