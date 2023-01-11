#pragma once
#ifndef PYXLIB__RHOMBUS_UTILS_H
#define PYXLIB__RHOMBUS_UTILS_H
/******************************************************************************
rhombus_utils.h

begin		: 2014-10-15
copyright	: (C) 2014 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"
#include "pyxis/rhombus/rhombus.h"
#include "pyxis/rhombus/rhombus_filler.h"
#include "pyxis/data/coverage.h"

/*!
PYXRhombusUtils - helper class to load rhombus into value array or RGB textures.
*/
class PYXLIB_DECL PYXRhombusUtils
{
public:
	//! willLoadFast return true if all tiles required for the given rhombus already in cache.
	static bool willLoadFast(const PYXRhombus & rhombus, int depth, const boost::intrusive_ptr<ICoverage> & coverage);

	//! return base64 string of a bitmap array of BGRA (32bit) pixels. assume coverage is RGB/RGBA values already
	static std::string loadRhombusBGRA(const PYXRhombus & rhombus, int depth, const boost::intrusive_ptr<ICoverage> & coverage);

	//! return base64 string of a bitmap array of BGRA (32bit) pixels. using a palette to covert the value to RGBA pixels
	static std::string loadRhombusBGRAPalette(const PYXRhombus & rhombus, int depth, const boost::intrusive_ptr<ICoverage> & coverage,const std::string & palette);

	//! return base64 string of a boolean buffer(1byte per value) followed by double values buffer (8byte per value) of the given rhombus. t
	static std::string loadRhombusDouble(const PYXRhombus & rhombus, int depth, const boost::intrusive_ptr<ICoverage> & coverage);

	//! return base64 string of a boolean buffer(1byte per value) followed by float values buffer (4byte per value) of the given rhombus
	static std::string loadRhombusFloat(const PYXRhombus & rhombus, int depth, const boost::intrusive_ptr<ICoverage> & coverage);

	//! return base64 string of a boolean buffer(1byte per value) followed by int values buffer (4byte per value) of the given rhombus
	static std::string loadRhombusInt(const PYXRhombus & rhombus, int depth, const boost::intrusive_ptr<ICoverage> & coverage);
};

class PYXLIB_DECL PYXRhombusRasterizer : public PYXObject
{
private:
#ifndef SWIG
	PYXRhombusFiller m_filler;
#endif

public:
	PYXRhombusRasterizer(const PYXRhombus & rhombus, int resolutionDepth);
	PYXRhombusRasterizer(const PYXRhombus & rhombus, int resolutionDepth, int tileDepth);
	virtual ~PYXRhombusRasterizer();

public:
	int getCellResolution() const;
	PYXPointer<PYXTile> getNeededTile() const;
	bool isReady() const;
	
	void setTileGeometry(const PYXPointer<PYXTile> & tile, const PYXPointer<PYXGeometry> & geometry);
	void setGeometry(const PYXPointer<PYXGeometry> & geometry);

public:
	//! return base64 string of a boolean buffer(1byte per value) for all cells intersect the geometry
	std::string rasterToBase64();
};

#endif