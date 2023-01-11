#pragma once
#ifndef VIEW_MODEL__OPEN_GL_TEXTURE_H
#define VIEW_MODEL__OPEN_GL_TEXTURE_H
/******************************************************************************
open_gl_texture.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"

#include "open_gl_resource.h"

#include "pyxis/utility/memory_manager.h"

#include <boost/scoped_ptr.hpp>
#include <vector>

/*!

OpenGLTexture - a wrapper class for a texture of openGL.

*/
//! OpenGLTexture - a wrapper class for texture of openGL.
class OpenGLTexture : public OpenGLResource, public ObjectMemoryUsageCounter<OpenGLTexture>
{
	friend class OpenGLProgram;

public:

	static bool canHaveTextureWithNonPowerOfTwo();

	static bool canSupportMultitextures();

	static const int knInvalidHandle;
	
	enum TextureType //texture types
	{
		knTexture1D = GL_TEXTURE_1D,
		knTexture2D = GL_TEXTURE_2D,
		knTexture3D = GL_TEXTURE_3D
	};

	enum TextureWarpMode
	{
		knTextureClamp = GL_CLAMP,
		knTextureClampToBorder = GL_CLAMP_TO_BORDER,
		knTextureClampToEdge = GL_CLAMP_TO_EDGE,
		knTextureRepeat = GL_REPEAT,
		knTextureMirroredRepeat = GL_MIRRORED_REPEAT
	};

	enum TextureWarpAxis
	{
		knTextureWarpS = GL_TEXTURE_WRAP_S,
		knTextureWarpT = GL_TEXTURE_WRAP_T,
		knTextureWarpR = GL_TEXTURE_WRAP_R
	};

	enum TextureMagFilter
	{
		knTextureMagNearest = GL_NEAREST,
		knTextureMagLinear  = GL_LINEAR
	};

	enum TextureMinFilter
	{

		knTextureMinNearest = GL_NEAREST,
		knTextureMinLinear  = GL_LINEAR,
		knTextureMinNearestMipMapNearest = GL_NEAREST_MIPMAP_NEAREST,
		knTextureMinLinearMipMapNearest = GL_LINEAR_MIPMAP_NEAREST,
		knTextureMinNearestMipMapLinear = GL_NEAREST_MIPMAP_LINEAR,
		knTextureMinLinearMipMapLinear = GL_LINEAR_MIPMAP_LINEAR,
	};	

	//! TextureFormat that is supported by us
	enum TextureFormat
	{
		/*
		knTextureAlpha = GL_ALPHA, 
		knTextureAlpha4 = GL_ALPHA4, 
		knTextureAlpha8 = GL_ALPHA8, 
		knTextureAlpha12 = GL_ALPHA12, 
		knTextureAlpha16 = GL_ALPHA16, 
		knTextureCompressedAlpha = GL_COMPRESSED_ALPHA, 
		knTextureCompressedLuminance = GL_COMPRESSED_LUMINANCE, 
		knTextureCompressedLuminanceAlpha = GL_COMPRESSED_LUMINANCE_ALPHA, 
		knTextureCompressedInensity = GL_COMPRESSED_INTENSITY, 
		knTextureCompressedRGB = GL_COMPRESSED_RGB, 
		knTextureCompressedRGBA = GL_COMPRESSED_RGBA, 
		knTextureDepth = GL_DEPTH_COMPONENT, 
		knTextureDepth16 = GL_DEPTH_COMPONENT16, 
		knTextureDepth24 = GL_DEPTH_COMPONENT24, 
		knTextureDepth32 = GL_DEPTH_COMPONENT32, 
		knTextureLuminance = GL_LUMINANCE, 
		knTextureLuminance4 = GL_LUMINANCE4, 
		knTextureLuminance8 = GL_LUMINANCE8, 
		knTextureLuminance12 = GL_LUMINANCE12, 
		knTextureLuminance16 = GL_LUMINANCE16, 
		knTextureLuminanceAlpha = GL_LUMINANCE_ALPHA, 
		knTextureLuminance4Alpha4 = GL_LUMINANCE4_ALPHA4, 
		knTextureLuminance6Alpha2 = GL_LUMINANCE6_ALPHA2, 
		knTextureLuminance8Alpha8 = GL_LUMINANCE8_ALPHA8, 
		knTextureLuminance12Alpha4 = GL_LUMINANCE12_ALPHA4, 
		knTextureLuminance12Alpha12 = GL_LUMINANCE12_ALPHA12, 
		knTextureLuminance16Alpha16 = GL_LUMINANCE16_ALPHA16, 
		knTextureInensity = GL_INTENSITY, 
		knTextureInensity4 = GL_INTENSITY4, 
		knTextureInensity8 = GL_INTENSITY8, 
		knTextureInensity12 = GL_INTENSITY12, 
		knTextureInensity16 = GL_INTENSITY16, 
		knTextureR3G3B2 = GL_R3_G3_B2, 
		*/
		knTextureRGB = GL_RGB, 
		/*
		knTextureRGB4 = GL_RGB4, 
		knTextureRGB5 = GL_RGB5, 
		knTextureRGB8 = GL_RGB8, 
		knTextureRGB10 = GL_RGB10, 
		knTextureRGB12 = GL_RGB12, 
		knTextureRGB16 = GL_RGB16, 
		*/
		knTextureRGBA = GL_RGBA
		/*
		knTextureRGBA2 = GL_RGBA2, 
		knTextureRGBA4 = GL_RGBA4, 
		knTextureRGB5A1 = GL_RGB5_A1, 
		knTextureRGBA8 = GL_RGBA8, 
		knTextureRGB10A2 = GL_RGB10_A2, 
		knTextureRGBA12 = GL_RGBA12, 
		knTextureRGBA16 = GL_RGBA16, 
		knTextureSLuminance = GL_SLUMINANCE, 
		knTextureSLuminance8 = GL_SLUMINANCE8, 
		knTextureSLuminanceAlpha = GL_SLUMINANCE_ALPHA, 
		knTextureSLuminance8Alpha8 = GL_SLUMINANCE8_ALPHA8, 
		knTextureSRGB = GL_SRGB, 
		knTextureSRGB8 = GL_SRGB8, 
		knTextureSRGBAlpha = GL_SRGB_ALPHA, 
		knTextureSRGB8Alpha8 = GL_SRGB8_ALPHA8
		*/
	};

	//! Data formats that are allowed to transfer data to the texture
	enum DataFormat
	{
		/*
		knDataColorIndex = GL_COLOR_INDEX, 
		knDataRed = GL_RED, 
		knDataGreen = GL_GREEN, 
		knDataBlue = GL_BLUE, 
		*/
		knDataAlpha = GL_ALPHA, 		
		knDataRGB = GL_RGB, 
		knDataBGR = GL_BGR, 
		knDataRGBA = GL_RGBA, 
		knDataBRGA = GL_BGRA
		/*
		knDatraLuminance = GL_LUMINANCE,
		knDatraLuminanceAlpha = GL_LUMINANCE_ALPHA
		*/
	};

	//! Data type that are allowed to transfer data to the texture
	enum DataType
	{
		knTextelUnsignedByte = GL_UNSIGNED_BYTE, 
		/*
		knTextelByte = GL_BYTE, 
		knTextelBitmap = GL_BITMAP, 
		*/
		knTextelUnsignedShort = GL_UNSIGNED_SHORT, 
		/*
		knTextelShort = GL_SHORT, 
		knTextelUnsignedInt = GL_UNSIGNED_INT, 
		*/
		knTextelInt = GL_INT, 
		knTextelFloat = GL_FLOAT
		/*
		knTextelUnsignedByte_3_3_2 = GL_UNSIGNED_BYTE_3_3_2, 
		knTextelUnsignedByte_3_3_2_Rev = GL_UNSIGNED_BYTE_2_3_3_REV, 
		knTextelUnsignedShort_5_6_5 = GL_UNSIGNED_SHORT_5_6_5, 
		knTextelUnsignedShort_5_6_5_Rev = GL_UNSIGNED_SHORT_5_6_5_REV, 
		knTextelUnsignedShort_4_4_4_4 = GL_UNSIGNED_SHORT_4_4_4_4, 
		knTextelUnsignedShort_4_4_4_4_Rev = GL_UNSIGNED_SHORT_4_4_4_4_REV, 
		knTextelUnsignedShort_5_5_5_1 = GL_UNSIGNED_SHORT_5_5_5_1, 
		knTextelUnsignedShort_5_5_5_1_Rev = GL_UNSIGNED_SHORT_1_5_5_5_REV, 
		knTextelUnsignedInt_8_8_8_8 = GL_UNSIGNED_INT_8_8_8_8, 
		knTextelUnsignedInt_8_8_8_8_Rev = GL_UNSIGNED_INT_8_8_8_8_REV, 
		knTextelUnsignedInt_10_10_10_2 = GL_UNSIGNED_INT_10_10_10_2, 
		knTextelUnsignedInt_10_10_10_2_Rev = GL_UNSIGNED_INT_2_10_10_10_REV
		*/
	};

public:
	OpenGLTexture(const TextureType & textureType,const TextureFormat & textureFormat);

	static PYXPointer<OpenGLTexture> create(const TextureType & textureType,const TextureFormat & textureFormat) 
	{ 
		return PYXNEW(OpenGLTexture,textureType,textureFormat); 
	}

	virtual ~OpenGLTexture(void);

	//! release the texture from GPU
	virtual void forceRelease();

//texture variables
protected:
	TextureType m_type;
	TextureFormat m_format;
	int m_handle;

	unsigned int m_width;
	unsigned int m_height;

//handle support
protected:	
	void createHandle();
	int getHandle();
	void bind();

//memory management
protected:
	int m_memoryUsed;

//open GL usage
public:
	//! set this as active texture in the OpenGL State Machine
	void startUsing();

	//! release the texture from the OpenGL State Machine
	void stopUsing();


	unsigned int getWidth();
	unsigned int getHeight();

	void setWarp(const TextureWarpMode & mode);
	void setWarp(const TextureWarpAxis & axis,const TextureWarpMode & mode);	

	void setMinFilter(const TextureMinFilter & minFilter);
	void setMagFilter(const TextureMagFilter & magFilter);

	void setSize(unsigned int width,unsigned int height,const DataFormat & format,const DataType & type);

	void generateMipmap();

	void setData(unsigned int width,unsigned int height,const DataFormat & format,const DataType & type,const void * data);
	void setData(unsigned int width,unsigned int height,const DataFormat & format,const unsigned char * data);
	void setData(unsigned int width,unsigned int height,const DataFormat & format,const int * data);
	void setData(unsigned int width,unsigned int height,const DataFormat & format,const float * data);

	void setMipLevelData(unsigned int mipLevel, unsigned int width,unsigned int height,const DataFormat & format,const DataType & type,const void * data);
	void setMipLevelData(unsigned int mipLevel, unsigned int width,unsigned int height,const DataFormat & format,const unsigned char * data);
	void setMipLevelData(unsigned int mipLevel, unsigned int width,unsigned int height,const DataFormat & format,const int * data);
	void setMipLevelData(unsigned int mipLevel, unsigned int width,unsigned int height,const DataFormat & format,const float * data);

	void setDataRegion(int x,int y,unsigned int width,unsigned int height,const DataFormat & format,const DataType & type,const void * data);
	void setDataRegion(int x,int y,unsigned int width,unsigned int height,const DataFormat & format,const unsigned char * data);
	void setDataRegion(int x,int y,unsigned int width,unsigned int height,const DataFormat & format,const int * data);
	void setDataRegion(int x,int y,unsigned int width,unsigned int height,const DataFormat & format,const float * data);
};

//! utility class for creating an array of textures of the same type
class OpenGLTextureArray : public PYXObject
{
protected:	
	typedef std::vector<PYXPointer<OpenGLTexture>> OpenGLTexturePointerVector;
	
	//! vector of PYXPointer<OpenGLTexture>
	OpenGLTexturePointerVector m_textures;
	
	//type and format of textures
	OpenGLTexture::TextureType m_type;
	OpenGLTexture::TextureFormat m_format;

public:
	OpenGLTextureArray(int length, const OpenGLTexture::TextureType & textureType,const OpenGLTexture::TextureFormat & textureFormat);

	//! return the size of the array
	int size();
	
	//! accessing a texture
	PYXPointer<OpenGLTexture> operator[](const int & index);
	
	//! add a new texture at the end of the array
	PYXPointer<OpenGLTexture> addNewTexture();
	
	//! remove a texture from the array
	void remove(const int & index);

	//! return true if all texture are valid
	bool isAllValid();

	void setAllWarp(const OpenGLTexture::TextureWarpMode & mode);
	void setAllWarp(const OpenGLTexture::TextureWarpAxis & axis,const OpenGLTexture::TextureWarpMode & mode);	

	void setAllMinFilter(const OpenGLTexture::TextureMinFilter & minFilter);
	void setAllMagFilter(const OpenGLTexture::TextureMagFilter & magFilter);

	void setAllSize(unsigned int width,unsigned int height,const OpenGLTexture::DataFormat & format,const OpenGLTexture::DataType & type);
};

#endif
