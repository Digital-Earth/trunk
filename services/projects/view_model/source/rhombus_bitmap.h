#pragma once
#ifndef VIEW_MODEL__RHOMBUS_BITMAP_H
#define VIEW_MODEL__RHOMBUS_BITMAP_H
/******************************************************************************
rhombus_bitmap.h

begin		: 2011-02-01
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "rhombus.h"
#include "cml_utils.h"
#include "pyxis/procs/viewpoint.h"
#include "pyxis/utility/color_palette.h"
#include "pyxis/utility/instance_counter.h"

/*!
RhombusRGBA - store RGBA information about a rhombus.

RhombusRGBA can store even and odd resolution, while odd resolutions store the RGBA data in 3 buffers.

RhombusRGBA main API functions:
1. getPixel(int u,int v,int buffer). - to get a the address of a single pixel inside the RhombusRGBA. used for get/set pixel.
2. samplePixel(double u,double v) - do a hexagon nearest neighbour sampling from the RhombusRGBA.
3. zoomIn - do "near fast" zoom in operation to display low resolution rhombus on high resolution rhombus.
4. isAllOpaque/isAllTransparent - used to speed up operations when blending layers together
*/
class RhombusRGBA : public PYXObject
{
//definitions
public:
	static const int width=82;
	static const int height=82;
	static const int channels=4;

	enum ResolutionType
	{
		knEvenResolution = 0, //store 1 rgba[height][width]
		knOddResolution = 1   //store 3 rgba[height][width]
	};

	struct RGBABuffer
	{
		unsigned char rgba[height][width][channels];
	};

//ctors
public:
	static PYXPointer<RhombusRGBA> create(ResolutionType resolutionType) { return PYXNEW(RhombusRGBA,resolutionType); }
	PYXPointer<RhombusRGBA> clone() { return PYXNEW(RhombusRGBA,*this); }

	RhombusRGBA(ResolutionType resolutionType);
	RhombusRGBA(const RhombusRGBA & other);
	virtual ~RhombusRGBA() {};

//methods
public:
	//! get buffer count: 1 - for even resolution, 3 - for odd resolution
	int getBufferCount() const;

	//! get the first pixel of the given buffer
	RGBABuffer & getBuffer(int bufferIndex = 0);

	//! get the first pixel of the given buffer - const mode
	const RGBABuffer & getBuffer(int bufferIndex = 0) const;

	//! fetch a single pixel from a buffer
	unsigned char * getPixel(int u,int v,int bufferIndex = 0);

	//! get the nearest neighbour pixel from a double coordinate - useful for by zoomIn operations
	unsigned char * getPixel(double u,double v);

	//! get the nearest neighbour pixel from a double coordinate - useful for by zoomIn operations
	void samplePixel(double u,double v,unsigned char * outputColor);

	//! perform the zooming operation using two Rhombus keys
	PYXPointer<RhombusRGBA> zoomIn(const Surface::Patch::Key & destination,const Surface::Patch::Key & source,ResolutionType resolutionType);

	//! fast operation of converting an even RhombusRGBA into a odd resolution one - used by the zoomIn if detected
	PYXPointer<RhombusRGBA> expandToOddResolution();

	//! fill the RGBA with a given color and alpha
	void fillWith(int color,int alpha);

	//! overlay otherLayer on top of this layer with additional globalAlpha factor (that will multiply the otherLayer alpha channel)
	void overlay(RhombusRGBA & otherLayer,const unsigned char globalAlpha = 255);

	//! get if all pixels are fully opaque
	bool isAllOpaque() const { return m_allOpaque; }

	//! get if all pixels are fully transparent
	bool isAllTransparent() const { return m_allTransparent; }

	//! set if all pixels are fully opaque - usually set by the zoomIn operation or the RhombusRGBAFiller
	void setAllOpaque(bool opaque) { m_allOpaque = opaque; }

	//! set if all pixels are fully transparent - usually set by the zoomIn operation or the RhombusRGBAFiller
	void setAllTransparent(bool transparent) { m_allTransparent = transparent; }

protected:
	static void mix(unsigned char * outputColor,unsigned char * color1,unsigned char * color2);
	static void mix(unsigned char * outputColor,unsigned char * color1,unsigned char * color2,unsigned char * color3);

//data items
protected:
	ResolutionType m_type;
	boost::scoped_array<RGBABuffer> m_buffers;
	bool m_allOpaque;
	bool m_allTransparent;
};

namespace RhombusBitmapColorizer
{
/*!
IColorizer - helper class to covert a PYXValue into RGBA
*/
class IColorizer
{
public:
	virtual void colorPixel(const PYXValue & value,unsigned char * rgba) const = 0;

	virtual ~IColorizer()
	{
	}
};

/*!
GrayScaleColorizer - convert the value to a gray scale palette using min and max values
*/
class GrayScaleColorizer : public IColorizer
{
protected: 
	double m_minValue;
	double m_maxValue;
	unsigned char m_alpha;

	bool m_useInt;

	double m_doubleFactor;
	int m_intFactor;
	int m_intMinValue;


public:
	GrayScaleColorizer(double minValue,double maxValue,unsigned char alpha = 255);

	virtual void colorPixel(const PYXValue & value,unsigned char * rgba) const;
};

/*!
PaletteColorizer - convert the value to RGBA using the given palette
*/
class PaletteColorizer : public IColorizer
{
protected:
	PYXColorPalette m_palette;

public:
	PaletteColorizer(const std::string palette);

	virtual void colorPixel(const PYXValue & value,unsigned char * rgba) const;
};



/*!
RGBWithAlphaColorizer - copy the PYXValue as uint8[4] - the last one is the alpha channel
*/
class RGBWithAlphaColorizer : public IColorizer
{
protected: 	
	//! used to multiply the input alpha. 255 - copy as 100% opacity 0 - copy as fully transparent
	unsigned char m_alpha;

public:
	RGBWithAlphaColorizer(unsigned char alpha = 255);

	virtual void colorPixel(const PYXValue & value,unsigned char * rgba) const;
};

/*!
RGBConstAlphaColorizer - copy the PYXValue as uint8[3] and add a constant alpha
*/
class RGBConstAlphaColorizer : public IColorizer
{
protected: 
	unsigned char m_alpha;

public:
	RGBConstAlphaColorizer(unsigned char alpha = 255);

	virtual void colorPixel(const PYXValue & value,unsigned char * rgba) const;
};


	/*!
RGB16BitConstAlphaColorizer - convert 16bit pixels to 8bit, copy the PYXValue as uint16[3] and add a constant alpha
*/
class RGB16BitConstAlphaColorizer : public IColorizer
{
protected: 
	unsigned char m_alpha;

public:
	RGB16BitConstAlphaColorizer(unsigned char alpha = 255);

	virtual void colorPixel(const PYXValue & value,unsigned char * rgba) const;
};


} //namespace RhombusBitmapColorizer


/*!
RhombusRGBAFiller - helper class to load RGBA information from a coverage.

RhombusRGBAFiller uses the given coverage and IColorizer to generate a valid RGBA:
1. load(Rhobmus) - will force the coverage to generate the given rhombus bitmap
2. loadFast(Rhombus) - will generate a bitmap if all needed tiles are in-cache
*/
class RhombusRGBAFiller
{
protected:
	unsigned char m_defaultAlpha;
	std::string m_name;
	boost::intrusive_ptr<ICoverage> m_spCoverage;
	const RhombusBitmapColorizer::IColorizer & m_colorizer;

public:
	RhombusRGBAFiller(const std::string & name, const boost::intrusive_ptr<ICoverage> & coverage,const RhombusBitmapColorizer::IColorizer & colorizer);

	//! load the layer
	PYXPointer<RhombusRGBA> load(const PYXRhombus & rhombus);

	//! load the layer only if all rhombuses cost is small (AKA - in-cache)
	PYXPointer<RhombusRGBA> loadFast(const PYXRhombus & rhombus);

protected:
	//! convert the PYXTiles into RGBA data
	PYXPointer<RhombusRGBA> fill(PYXRhombusFiller & filler);
};


class RhombusRGBABlender
{
public:
	struct RGBAIntegerBuffer
	{
		unsigned int rgba[RhombusRGBA::height][RhombusRGBA::width][RhombusRGBA::channels];
	};

public:
	RhombusRGBABlender(RhombusRGBA::ResolutionType resolutionType);

	void addRhombusRGBA(RhombusRGBA & rgba,unsigned char & globalAlpha);
	void toRhombusRGBA(RhombusRGBA & rgba);

protected:
	//! get buffer count: 1 - for even resolution, 3 - for odd resolution
	int getBufferCount() const;

	//! get the first pixel of the given buffer
	RGBAIntegerBuffer & getBuffer(int bufferIndex = 0);

	//! fetch a single pixel from a buffer
	unsigned int * getPixel(int u,int v,int bufferIndex = 0);

	boost::scoped_array<RGBAIntegerBuffer> m_buffers;
	RhombusRGBA::ResolutionType m_type;
};


#endif