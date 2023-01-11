/******************************************************************************
rhombus_tuils.cpp

begin		: 2014-10-15
copyright	: (C) 2014 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/rhombus/rhombus_bitmap.h"
#include "pyxis/rhombus/rhombus_utils.h"

#include "pyxis/geometry/tile_collection.h"
#include "pyxis/data/value_tile.h"

// third party includes
#include "zlib.h"

// standard includes
#include <cassert>
#include <set>
#include <map>
#include <vector>


/////////////////////////////////////////////////////////////////////////////////////////
// PYXRhombusUtils
/////////////////////////////////////////////////////////////////////////////////////////

void fillBitmap(PYXRhombusFiller & filler,BYTE * bitmap,int size,RhombusBitmapColorizer::IColorizer & colorizer)
{
	auto it = filler.getIteratorWithLUT(0);

	while(!it.end())
	{
		if (it.getOffsetCoord() != 0) 
		{
			++it;
			continue;
		}

		unsigned char * pixel = &bitmap[(it.getUCoord()+it.getVCoord()*size)*4];

		if (it.hasValue())
		{
			colorizer.colorPixel(it.getValue(),pixel);
		}
		else
		{
			memset(pixel,0,4);			
		}
		++it;
	}
}

void loadFiller(PYXRhombusFiller & filler,const boost::intrusive_ptr<ICoverage> & coverage)
{
	while(!filler.isReady())
	{
		auto tile = filler.getNeededTile();
		auto valueTile = coverage->getCoverageTile(*tile);
		filler.addTile(tile,valueTile);
	}
}

void RGBAtoBGRA(BYTE * bitmap,int size)
{
	auto length = size*size;
	for(int i=0;i<length;i++)
	{
		auto temp = bitmap[i*4];
		bitmap[i*4] = bitmap[i*4+2];
		bitmap[i*4+2] = temp;
	}
}

bool PYXRhombusUtils::willLoadFast(const PYXRhombus & rhombus,int depth, const boost::intrusive_ptr<ICoverage> & coverage)
{
	PYXRhombusFiller filler(rhombus,depth,11);

	auto tiles = filler.getAllNeededTiles();

	for(auto & tile : tiles)
	{
		auto tileCost = coverage->getTileCost(*tile);
		if (tileCost.getMaximumCost() >= PYXCost::knDefaultCost.getMaximumCost())
		{
			return false;
		}
	}
	return true;
}

std::string PYXRhombusUtils::loadRhombusBGRA(const PYXRhombus & rhombus,int depth, const boost::intrusive_ptr<ICoverage> & coverage)
{
	PYXRhombusFiller filler(rhombus,depth,11);

	loadFiller(filler,coverage);

	auto size = rhombus.getUVMax(depth)+1;

	auto valueCount = size*size;
	
	boost::scoped_array<BYTE> bitmap(new BYTE[valueCount*4]);
	
	auto pixelType = coverage->getCoverageDefinition()->getFieldDefinition(0).getType();
	if (pixelType  == PYXValue::knInt16 || pixelType == PYXValue::knUInt16 )
	{
		RhombusBitmapColorizer::RGB16BitConstAlphaColorizer colorizer;
		fillBitmap(filler,bitmap.get(),size,colorizer);	
	}
	else if (pixelType == PYXValue::knFloat) {
		RhombusBitmapColorizer::RGBFloat32ConstAlphaColorizer colorizer;
		fillBitmap(filler, bitmap.get(), size, colorizer);
	}
	else
	{
		RhombusBitmapColorizer::RGBConstAlphaColorizer colorizer;
		fillBitmap(filler,bitmap.get(),size,colorizer);	
	}
	
	RGBAtoBGRA(bitmap.get(),size);

	std::string resultAsString((char*)bitmap.get(),valueCount*4);
	return XMLUtils::toBase64(resultAsString);
}

std::string PYXRhombusUtils::loadRhombusBGRAPalette(const PYXRhombus & rhombus,int depth, const boost::intrusive_ptr<ICoverage> & coverage,const std::string & palette)
{
	PYXRhombusFiller filler(rhombus,depth,11);

	loadFiller(filler,coverage);

	auto size = rhombus.getUVMax(depth)+1;

	auto valueCount = size*size;
	boost::scoped_array<BYTE> bitmap(new BYTE[valueCount*4]);
	
	RhombusBitmapColorizer::PaletteColorizer colorizer(palette);

	fillBitmap(filler,bitmap.get(),size,colorizer);
	RGBAtoBGRA(bitmap.get(),size);

	std::string resultAsString((char*)bitmap.get(),valueCount*4);
	return XMLUtils::toBase64(resultAsString);
}

template <typename T>	T		inline castValue(const PYXValue & v)			{ return T; };
template <>				double	inline castValue<double>(const PYXValue & v)	{ return v.getDouble(); };
template <>				float	inline castValue<float>(const PYXValue & v)		{ return v.getFloat(); };
template <>				int		inline castValue<int>(const PYXValue & v)		{ return v.getInt(); };

template <typename T>
std::string loadRhombus(const PYXRhombus & rhombus,int depth, const boost::intrusive_ptr<ICoverage> & coverage)
{
	PYXRhombusFiller filler(rhombus,depth,11);

	loadFiller(filler,coverage);

	auto size = rhombus.getUVMax(depth)+1;

	auto valueCount = size*size;
	boost::scoped_array<bool> hasValues (new bool[valueCount]);
	boost::scoped_array<T> values (new T[valueCount]);
	
	auto it = filler.getIteratorWithLUT(0);

	int i=0;
	while(!it.end())
	{
		hasValues[i] = it.hasValue();
		if (hasValues[i])
		{
			values[i] = castValue<T>(it.getValue());
		}
		++i;
		++it;
	}

	std::string hasValuesString((char*)hasValues.get(),valueCount);
	std::string valuesAsString((char*)values.get(),valueCount*sizeof(T));
	return XMLUtils::toBase64(hasValuesString+valuesAsString);
};

std::string PYXRhombusUtils::loadRhombusDouble(const PYXRhombus & rhombus,int depth, const boost::intrusive_ptr<ICoverage> & coverage)
{
	return loadRhombus<double>(rhombus,depth,coverage);
}

std::string PYXRhombusUtils::loadRhombusFloat(const PYXRhombus & rhombus,int depth, const boost::intrusive_ptr<ICoverage> & coverage)
{
	return loadRhombus<float>(rhombus,depth,coverage);
}

std::string PYXRhombusUtils::loadRhombusInt(const PYXRhombus & rhombus,int depth, const boost::intrusive_ptr<ICoverage> & coverage)
{
	return loadRhombus<int>(rhombus,depth,coverage);	
}



PYXRhombusRasterizer::PYXRhombusRasterizer(const PYXRhombus & rhombus, int resolutionDepth) : m_filler(rhombus,resolutionDepth)
{
	
}

PYXRhombusRasterizer::PYXRhombusRasterizer(const PYXRhombus & rhombus, int resolutionDepth, int tileDepth) : m_filler(rhombus,resolutionDepth,tileDepth)
{
	
}

PYXRhombusRasterizer::~PYXRhombusRasterizer()
{
	
}


int PYXRhombusRasterizer::getCellResolution() const
{
	return m_filler.getCellResolution();
}

PYXPointer<PYXTile> PYXRhombusRasterizer::getNeededTile() const
{
	return m_filler.getNeededTile();
}


bool PYXRhombusRasterizer::isReady() const
{
	return m_filler.isReady();
}

void PYXRhombusRasterizer::setTileGeometry(const PYXPointer<PYXTile> & tile, const PYXPointer<PYXGeometry> & geometry)
{
	std::vector<PYXValue::eType> types;
	types.push_back(PYXValue::eType::knUInt8);
	auto valueTile = PYXValueTile::create(*tile, types);
	auto trueValue = PYXValue(static_cast<int8_t>(1));

	auto tileCollection = PYXTileCollection::create();
	auto intersection = geometry->intersection(*tile);
	intersection->copyTo(tileCollection.get(),tile->getCellResolution());
	
	auto tileIterator = tileCollection->getTileIterator();
	auto & root = tile->getRootIndex();

	while(!tileIterator->end())
	{
		auto currentTile = tileIterator->getTile();

		auto currentTileRoot = currentTile->getRootIndex();

		if (currentTileRoot.getResolution() == tile->getCellResolution())
		{
			auto pos = PYXIcosMath::calcCellPosition(root,currentTileRoot);
			valueTile->setValue(pos,0,trueValue);
		}
		else
		{
			auto count = currentTile->getCellCount();
			currentTileRoot.setResolution(tile->getCellResolution());
			auto pos = PYXIcosMath::calcCellPosition(root,currentTileRoot);
			for(auto i = 0; i < count; i++)
			{
				valueTile->setValue(pos+i,0,trueValue);
			}
		}

		tileIterator->next();
	}

	m_filler.addTile(tile,valueTile);
}

void PYXRhombusRasterizer::setGeometry(const PYXPointer<PYXGeometry> & geometry)
{
	for(auto & tile : m_filler.getAllNeededTiles())
	{
		setTileGeometry(tile,geometry);
	}
}

std::string PYXRhombusRasterizer::rasterToBase64()
{
	auto size = m_filler.getRhombus().getUVMax(m_filler.getResolutionDepth())+1;

	auto valueCount = size*size;
	boost::scoped_array<bool> hasValues (new bool[valueCount]);
	
	auto it = m_filler.getIteratorWithLUT(0);

	auto i=0;
	while(!it.end())
	{
		hasValues[i] = it.hasValue();
		++i;
		++it;
	}

	std::string hasValuesString((char*)hasValues.get(),valueCount);
	return XMLUtils::toBase64(hasValuesString);
}