/******************************************************************************
rhombus.cpp

begin		: 2009-11-01
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/rhombus/rhombus_filler.h"

#include "pyxis/derm/icos_iterator.h"
#include "pyxis/data/record.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"

// third party includes
#include "zlib.h"

// standard includes
#include <cassert>
#include <set>
#include <map>
#include <vector>

/////////////////////////////////////////////////////////////////////////////////////////
// PYXRhombusFiller
/////////////////////////////////////////////////////////////////////////////////////////

PYXRhombusFiller::PYXRhombusFiller(const PYXRhombus & rhombus,int resolutionDepth,int tileDepth) : m_rhombus(rhombus),m_resolutionDepth(resolutionDepth),m_tileDepth(tileDepth)
{
	std::set<PYXIcosIndex> needTiles;

	PYXIcosIndex index = rhombus.getIndex(0);
	int rootResolution = index.getResolution()+ resolutionDepth - m_tileDepth;

	//Pipe doesnt work when trying to fetch a "Prim" resolution - Tile has not bounding cell...
	if (rootResolution <= 2)
	{
		rootResolution = 2;
		m_tileDepth = index.getResolution()+ resolutionDepth - rootResolution;
	}

	for (int i=0;i<4;i++)
	{
		PYXIcosIndex rootIndex = rhombus.getIndex(i);

		rootIndex.setResolution(rootResolution);

		needTiles.insert(rootIndex);
	}

	for(std::set<PYXIcosIndex>::iterator it = needTiles.begin(); it != needTiles.end(); ++it)
	{
		m_tiles[*it] = NULL;

		m_needTiles.push_back(*it);
	}
}

PYXRhombusFiller::PYXRhombusFiller(const PYXRhombus & rhombus, int resolutionDepth) : m_rhombus(rhombus),m_resolutionDepth(resolutionDepth),m_tileDepth(resolutionDepth)
{
	for (int i=0;i<4;i++)
	{
		PYXIcosIndex rootIndex = rhombus.getIndex(i);

		//Pipe doesnt work when trying to fetch a "Prim" resolution - Tile has not bounding cell...
		//rootIndex.decrementResolution();

		m_tiles[rootIndex] = NULL;

		m_needTiles.push_back(rootIndex);
	}
}

PYXRhombusFiller::~PYXRhombusFiller()
{
}

const PYXRhombus & PYXRhombusFiller::getRhombus() const
{
	return m_rhombus;
}

const int & PYXRhombusFiller::getResolutionDepth() const
{
	return m_resolutionDepth;
}

int PYXRhombusFiller::getCellResolution() const
{
	return m_needTiles[0].getResolution() + m_tileDepth;
}

bool PYXRhombusFiller::isReady() const
{
	return m_needTiles.size()==0;
}

bool PYXRhombusFiller::allValuesTilesAreNull() const
{
	for(TilesMap::const_iterator it = m_tiles.begin(); it != m_tiles.end(); ++it)
	{
		if (it->second)
		{
			return false;
		}
	}
	return true;
}

PYXPointer<PYXTile> PYXRhombusFiller::getNeededTile() const
{
	if (m_needTiles.size()==0)
	{
		return NULL;
	}
	
	return PYXTile::create(m_needTiles[0],m_needTiles[0].getResolution()+m_tileDepth);	
}

std::vector<PYXPointer<PYXTile>> PYXRhombusFiller::getAllNeededTiles() const
{
	std::vector<PYXPointer<PYXTile>> result;

	for(unsigned int i=0;i<m_needTiles.size();i++)
	{
		result.push_back(PYXTile::create(m_needTiles[i],m_needTiles[i].getResolution()+m_tileDepth));
	}

	return result;
}

void PYXRhombusFiller::addTile(const PYXPointer<PYXTile> & tile, const PYXPointer<PYXValueTile> & valueTile)
{
	//add the value tile into the map - but only if it not null - which could happen.
	if (valueTile)
	{
		assert(valueTile->getTile() == *tile && "Tile and ValueTile must be the same");

		assert(valueTile->getTile().getDepth() == m_tileDepth && "New PYXValueTile is not in the right depth");

		PYXIcosIndex index = valueTile->getTile().getRootIndex();

		assert(m_tiles.find(index) != m_tiles.end() && "NEW PYXValueTile is not belong to Rhombus");

		m_tiles[index] = valueTile;
	}
	//remove the tile from the needed tiles
	m_needTiles.erase(std::find(m_needTiles.begin(),m_needTiles.end(),tile->getRootIndex()));
}

//! Get value for given cell index and channel, returns false for null value
bool PYXRhombusFiller::getValue(	const PYXIcosIndex& cellIndex,
				const int nChannelIndex,
				PYXValue* pValue) const
{
	try
	{
	assert(m_needTiles.size() == 0 && "TilesFiller is not ready");

	TilesMap::const_iterator it = m_tiles.begin();

	while(it != m_tiles.end())
	{
		if (it->first.isAncestorOf(cellIndex))
		{
			if (it->second.get() != NULL)
			{
				try
				{
					//make sure you value type is currect...
					if (pValue->getArrayType() != it->second->getDataChannelType(nChannelIndex))
					{
						*pValue = it->second->getTypeCompatibleValue(nChannelIndex);
					}
					return it->second->getValue(cellIndex,nChannelIndex,pValue);
				}
				catch(...)
				{
					TRACE_INFO("damm");
				}
			}
			else
			{
				return false;
			}
		}
		++it;
	}
	}
	catch(...)
	{
		TRACE_INFO("Failed To get a Value on PYXRhombusFiller");
	}

	PYXTHROW(PYXException,"Can't find Cell in tiles");
}

PYXRhombusFiller::Iterator PYXRhombusFiller::getIterator(const int & nChannelIndex)
{
	return Iterator(*this,nChannelIndex);
}


PYXRhombusFiller::IteratorWithLUT PYXRhombusFiller::getIteratorWithLUT(const int & nChannelIndex)
{
	return IteratorWithLUT(*this,nChannelIndex);
}

/////////////////////////////////////////////////////////////////////////////////////////
// PYXRhombusCursor
/////////////////////////////////////////////////////////////////////////////////////////

PYXRhombusCursor::PYXRhombusCursor(const PYXRhombus & rhombus,int resolutionDepth)
	:	m_rhombus(rhombus),
		m_resolutionDepth(resolutionDepth),
		m_u(0),
		m_v(0),
		m_bEnd(false)
{
	m_oddResolution = m_resolutionDepth % 2 != 0;

	m_u_cursor.reset(m_rhombus.getIndex(0),m_rhombus.getDirection(0));
	for(int i=0;i<m_resolutionDepth;i++)
	{
		m_u_cursor.zoomIn();
	}
	if (m_oddResolution)
	{
		//if we do oddResolution - then we need to enter the next resolution also
		m_u_cursor.zoomIn();
	}

	m_v_cursor = m_u_cursor;
	m_v_cursor.left();
	
	if (m_oddResolution)
	{
		//if we do oddResolution - then we need to enter the next resolution also
		m_max = m_rhombus.getUVMax(m_resolutionDepth+1);
	}
	else
	{
		m_max = m_rhombus.getUVMax(m_resolutionDepth);
	}

	m_index = m_u_cursor.getIndex();

	if (m_oddResolution)
	{		
		m_index.setResolution(m_index.getResolution()-1); //reduce resolution by one before asking for the value.	
	}

	if (m_oddResolution)
	{
		m_uvFactor = 3;
	}
	else
	{
		m_uvFactor = 1;
	}
}

PYXRhombusCursor::PYXRhombusCursor(const PYXRhombusCursor & other)
	:	m_rhombus(other.m_rhombus),
		m_resolutionDepth(other.m_resolutionDepth),
		m_u_cursor(other.m_u_cursor),
		m_v_cursor(other.m_v_cursor),
		m_index(other.m_index),
		m_max(other.m_max),
		m_u(other.m_u),
		m_v(other.m_v),
		m_oddResolution(other.m_oddResolution),
		m_uvFactor(other.m_uvFactor),
		m_bEnd(other.m_bEnd)
{
}

PYXRhombusCursor & PYXRhombusCursor::operator=(const PYXRhombusCursor & other)
{
	m_rhombus = other.m_rhombus;
	m_resolutionDepth = other.m_resolutionDepth;

	m_v = other.m_v;
	m_u = other.m_u;
	m_v_cursor = other.m_v_cursor;
	m_u_cursor = other.m_u_cursor;
	m_index = other.m_index;
	m_max = other.m_max;

	m_oddResolution = other.m_oddResolution;
	m_uvFactor = other.m_uvFactor;

	m_bEnd = other.m_bEnd;

	return *this;
}

void PYXRhombusCursor::next()
{
	while(true)
	{
		if (m_v == m_max && m_u == m_max)
		{
			m_bEnd = true;
			return;
		}

		if (m_u==m_max)
		{
			m_v++;
			m_v_cursor.forward();
			m_u_cursor = m_v_cursor;
			m_u_cursor.right();
			m_u = 0;
		}
		else
		{
			m_u_cursor.forward();
			m_u++;
		}

		if (!m_oddResolution || m_u_cursor.getIndex().hasVertexChildren())
		{
			break;
		}
	}

	m_index = m_u_cursor.getIndex();

	if (m_oddResolution)
	{
		m_index.setResolution(m_index.getResolution()-1); //reduce resolution by one before asking for the value.	
	}
}

bool PYXRhombusCursor::end() const
{
	return m_bEnd;
}

/////////////////////////////////////////////////////////////////////////////////////////
// PYXRhombusFiller::Iterator
/////////////////////////////////////////////////////////////////////////////////////////

PYXRhombusFiller::Iterator::Iterator(PYXRhombusFiller & filler,int nChannelIndex) : m_filler(&filler),m_channel(nChannelIndex),m_cursor(filler.m_rhombus,filler.m_resolutionDepth)
{
	assert(m_filler->isReady() && "Filler is not ready, can't create iterator from it.");

	PYXRhombusFiller::TilesMap::iterator it = m_filler->m_tiles.begin();

	//try to find a ValueTile...
	while (it != m_filler->m_tiles.end() && it->second.get() == NULL)
	{
		++it;
	}

	//create compatilbe value
	if (it != m_filler->m_tiles.end())
	{
		m_value = it->second->getTypeCompatibleValue(0);
	}

	m_hasValue = m_filler->getValue(m_cursor.getIndex(),m_channel,&m_value);
}

PYXRhombusFiller::Iterator::Iterator(const Iterator& other) : m_filler(other.m_filler),m_channel(other.m_channel),m_cursor(other.m_cursor)
{
}

void PYXRhombusFiller::Iterator::operator =(const Iterator& other)
{
	m_filler = other.m_filler;
	m_channel = other.m_channel;

	m_value = other.m_value;
	m_hasValue = other.m_hasValue;

	m_cursor = other.m_cursor;
}

void PYXRhombusFiller::Iterator::next()
{
	m_cursor.next();
	m_hasValue = m_filler->getValue(m_cursor.getIndex(),m_channel,&m_value);
}

bool PYXRhombusFiller::Iterator::end() const
{
	return m_cursor.end();
}

/////////////////////////////////////////////////////////////////////////////////////////
// PYXRhombusFiller::IteratorWithLUT
/////////////////////////////////////////////////////////////////////////////////////////

PYXRhombusFiller::IteratorWithLUT::IteratorWithLUT(PYXRhombusFiller & filler,int nChannelIndex)
	:	m_filler(&filler),
		m_channel(nChannelIndex),
		m_u(0),
		m_v(0),
		m_offset(0),
		m_hasValue(false),
		m_bEnd(false)
{
	assert(m_filler->isReady() && "Filler is not ready, can't create iterator from it.");

	m_lut = LUT::create(filler);

	PYXRhombusFiller::TilesMap::iterator it = m_filler->m_tiles.begin();

	//try to find a ValueTile...
	while (it != m_filler->m_tiles.end() && it->second.get() == NULL)
	{
		++it;
	}

	//create compatilbe value
	if (it != m_filler->m_tiles.end())
	{
		m_value = it->second->getTypeCompatibleValue(0);
	}

	for (int i=0;i<4;i++)
	{
		for (it = m_filler->m_tiles.begin();it != m_filler->m_tiles.end();++it)
		{
			PYXIcosIndex index(filler.getRhombus().getIndex(i));
			if (it->first.isAncestorOf(index))
			{
				//make the index at the rigth resoltion;
				index.setResolution(index.getResolution()+m_filler->getResolutionDepth());
				int offset = PYXIcosMath::calcCellPosition(it->first, index);

				m_tiles.push_back(it->second);
				m_tilesOffset.push_back(offset);
				break;
			}
		}
	}

	int offset = m_lut->getTileIndex(m_u,m_v,m_offset);
	if (m_tiles[offset])
	{
		m_hasValue = m_tiles[offset]->getValue(m_lut->getPosIndex(m_u,m_v,m_offset)+m_tilesOffset[offset],m_channel,&m_value);
	}
	else
	{
		m_hasValue = false;
	}
}

PYXRhombusFiller::IteratorWithLUT::IteratorWithLUT(const IteratorWithLUT & other) 
	:	m_filler(other.m_filler),
		m_channel(other.m_channel),
		m_lut(other.m_lut),
		m_tiles(other.m_tiles),
		m_tilesOffset(other.m_tilesOffset),
		m_u(other.m_u),
		m_v(other.m_v),
		m_offset(other.m_offset),
		m_hasValue(other.m_hasValue),
		m_value(other.m_value),
		m_bEnd(other.m_bEnd)
{
}

void PYXRhombusFiller::IteratorWithLUT::operator =(const IteratorWithLUT & other)
{
	m_filler = other.m_filler;
	m_channel = other.m_channel;

	m_value = other.m_value;
	m_hasValue = other.m_hasValue;

	m_lut = other.m_lut;
	m_tiles = other.m_tiles;
	m_tilesOffset = other.m_tilesOffset;
	
	m_u = other.m_u;
	m_v = other.m_v;
	m_offset = other.m_offset;
	
	m_bEnd = other.m_bEnd;
}

void PYXRhombusFiller::IteratorWithLUT::next()
{
	int max = m_lut->getMaxUV();
	int maxOffset = 0;
	if (m_lut->isOddResolutionDepth())
	{
		maxOffset = 2;
	}
	if (m_v == max && m_u == max && m_offset == maxOffset)
	{
		m_bEnd = true;
		return;
	}

	if (m_u==max)
	{
		if (m_v==max && m_offset < maxOffset)
		{
			m_offset++;
			m_u=0;
			m_v=0;
		}
		else
		{
			m_v++;
			m_u = 0;
		}
	}
	else
	{
		m_u++;
	}

	int offset = m_lut->getTileIndex(m_u,m_v,m_offset);
	if (m_tiles[offset])
	{
		m_hasValue = m_tiles[offset]->getValue(m_lut->getPosIndex(m_u,m_v,m_offset)+m_tilesOffset[offset],m_channel,&m_value);
	}
	else
	{
		m_hasValue = false;
	}
}

bool PYXRhombusFiller::IteratorWithLUT::end() const
{
	return m_bEnd;
}

/////////////////////////////////////////////////////////////////////////////////////////
// PYXRhombusFiller::LUT
/////////////////////////////////////////////////////////////////////////////////////////

//static decleartion
std::map<std::string,PYXPointer<PYXRhombusFiller::LUT>>	 PYXRhombusFiller::LUT::s_LUTCache;
boost::recursive_mutex	PYXRhombusFiller::LUT::s_LUTCacheMutex;

std::string PYXRhombusFiller::LUT::getLUTIdentity(const PYXRhombusFiller & filler)
{
	std::string key; //start with empty name

	key += StringUtils::toString(filler.getResolutionDepth())+ "_"; // add depth.

	for(int i=0;i<4;i++)
	{
		const PYXIcosIndex & index = filler.getRhombus().getIndex(i);

		PYXMath::eHexDirection direction = filler.getRhombus().getDirection(i);

		if (index.isNorthern())
		{
			key += "N"; //Northern
		}
		else
		{
			key += "S"; //Southern
		}

		if (index.isHexagon())
		{
			key += "H"; //Hexagon
		}
		else
		{
			key += "P"; //Pentagon
		}

		key += StringUtils::toString(index.getClass()); //add class
		
		if (index.isMajor())
		{
			key += "C"; //centriod
		}
		else
		{
			key += "V"; //vertex
		}

		key += StringUtils::toString<int>(direction);

		if (i<3)
		{
			key += "_";
		}
	}
	return key;	
}

PYXRhombusFiller::LUT::LUT(const PYXRhombusFiller& filler) : m_resolutionDepth(filler.getResolutionDepth())
{
	m_key = getLUTIdentity(filler);
	if (!load(filler))
	{
		generate(filler);
		save(filler);
	}
}

bool PYXRhombusFiller::LUT::load(const PYXRhombusFiller & filler)
{
	boost::filesystem::path fileName = AppServices::getCacheDir("lut");

	fileName /= FileUtils::stringToPath(m_key + ".rlut");

	bool bLUTLoaded = false;

	try
	{
		if (FileUtils::exists(fileName ))
		{
			// load the table
			std::ifstream file(FileUtils::pathToString(fileName).c_str(), std::ios::binary);

			//magic = "Rhbomus Lookup Table"
			const char* const magic = "RLT"; // note this is 4 bytes not 3!
			char buf[8] = { 0 }; // init buffer to zero

			file.read(buf, 8);
			if (memcmp(magic, buf, 4) == 0)
			{
				int& nVersion = *reinterpret_cast<int*>(buf + 4);
				if (nVersion == 1)
				{
					//create buffers
					PYXRhombusCursor cursor(filler.m_rhombus,filler.m_resolutionDepth);

					m_max = cursor.getMaxUV()+1;
					m_maxSqaure = m_max*m_max;

					int size = m_maxSqaure;
					if (cursor.isOddResolutionDepth())
					{
						size *= 3;
					}

					m_tileIndex.reset(new char[size]);
					m_posIndex.reset(new int[size]);

					unsigned long nUnCompressedLength;
					unsigned long nCompressedBufferLength;
					file.read((char*)&nUnCompressedLength,sizeof(unsigned long));
					file.read((char*)&nCompressedBufferLength,sizeof(unsigned long));
					boost::scoped_array<char> pCompressedDataBuffer(new char[nCompressedBufferLength]);
					file.read(pCompressedDataBuffer.get(), nCompressedBufferLength);
					file.close();
					boost::scoped_array<char> pUnCompressedDataBuffer(new char[nUnCompressedLength]);
					int returnCode = uncompress((Bytef *)pUnCompressedDataBuffer.get(), &nUnCompressedLength,
										   (const Bytef *)pCompressedDataBuffer.get(), nCompressedBufferLength);

					// Check to see if the compress was successful.
					if (returnCode != Z_OK)
					{
						PYXTHROW(PYXException, "Uncompression failed while loading an lut.");
					}

					// stream it in from the uncompressed buffer.
					std::string compressedLUT(pUnCompressedDataBuffer.get(), nUnCompressedLength);
					std::istringstream memIn(compressedLUT);
					memIn.read(reinterpret_cast<char*>(m_tileIndex.get()), size);
					memIn.read(reinterpret_cast<char*>(m_posIndex.get()), size * sizeof(int));
					bLUTLoaded = memIn.good();
				}
			}
		}
	}
	catch (PYXException ex)
	{
		TRACE_ERROR("Failed lo load lut file : " + ex.getFullErrorString());
	}
	catch (...)
	{
		TRACE_ERROR("Failed lo load lut file : unknown error");
	}

	return bLUTLoaded;	
}

void PYXRhombusFiller::LUT::save(const PYXRhombusFiller & filler)
{
	int size = m_maxSqaure;
	//if odd resolution
	if (m_resolutionDepth % 2 != 0)
	{
		size *= 3;
	}

	// create a memory stream for the tile so we can compress it before it goes on disk.
	std::ostringstream memOut;

	memOut.write(reinterpret_cast<char*>(m_tileIndex.get()), size);
	memOut.write(reinterpret_cast<char*>(m_posIndex.get()), size * sizeof(int));

	unsigned long nUnCompressedLength = static_cast<unsigned long>(memOut.str().length());
	unsigned long nCompressedBufferLength = compressBound(nUnCompressedLength);
	boost::scoped_array<char> pCompressedDataBuffer(new char[nCompressedBufferLength]);
	int returnCode = compress((Bytef *)pCompressedDataBuffer.get(), &nCompressedBufferLength,
                                (const Bytef *)memOut.str().c_str(), nUnCompressedLength);

	// Check to see if the compress was successful.
	if (returnCode != Z_OK)
	{
		PYXTHROW(PYXException, "Compression failed while saving an lut.");
	}

	boost::filesystem::path fileName = AppServices::getCacheDir("lut");

	fileName  /= m_key + ".rlut";

	// create the output file
	std::ofstream file(FileUtils::pathToString(fileName).c_str(), std::ios::binary);

	// Write magic number.
	file.write("RLT", 4);

	// Write version.
	int nVersion = 1;
	file.write(reinterpret_cast<const char*>(&nVersion), 4);

	// write the compresed data
	file.write((char*)&nUnCompressedLength,sizeof(unsigned long));
	file.write((char*)&nCompressedBufferLength,sizeof(unsigned long));
	file.write(pCompressedDataBuffer.get(), nCompressedBufferLength);

	file.close();
}

void PYXRhombusFiller::LUT::generate(const PYXRhombusFiller & filler)
{
	assert(filler.isReady() && "Filler is not ready, can't create LUT from it.");

	PYXRhombusCursor cursor(filler.m_rhombus,filler.m_resolutionDepth);

	m_max = cursor.getMaxUV()+1;
	m_maxSqaure = m_max*m_max;

	int size = m_maxSqaure;
	if (cursor.isOddResolutionDepth())
	{
		size *= 3;
	}

	m_posIndex.reset(new int[size]);
	m_tileIndex.reset(new char[size]);

	while (!cursor.end())
	{
		const PYXIcosIndex & index = cursor.getIndex();

		char tileIndex = 0;
		int  pos = 0;

		for(tileIndex=0;tileIndex<4;tileIndex++)
		{
			const PYXIcosIndex & rootIndex = filler.getRhombus().getIndex(tileIndex);
			if (rootIndex.isAncestorOf(index))
			{
				pos = PYXIcosMath::calcCellPosition(rootIndex, index);
				break;
			}
		}

		int i = m_max * cursor.getVCoord() + cursor.getUCoord() + m_maxSqaure * cursor.getOffsetCoord();
		m_posIndex[i] = pos;
		m_tileIndex[i] = tileIndex;

		cursor.next();
	}
}

PYXPointer<PYXRhombusFiller::LUT> PYXRhombusFiller::LUT::create(const PYXRhombusFiller & filler)
{
	std::string key = getLUTIdentity(filler);

	std::map<std::string,PYXPointer<LUT>>::iterator it;

	{
		//try to find it in the cahce;
		boost::recursive_mutex::scoped_lock	lock(s_LUTCacheMutex);
		it = s_LUTCache.find(key);
		if (it != s_LUTCache.end())
		{
			return it->second;
		}
	}

	//this can take a little bit :D
	PYXPointer<LUT> lut = PYXNEW(PYXRhombusFiller::LUT,filler);

	{
		//add it to the cache
		boost::recursive_mutex::scoped_lock	lock(s_LUTCacheMutex);
		if (s_LUTCache.size()>1000)
		{
			s_LUTCache.clear();
		}

		s_LUTCache[key] = lut;
	}

	return lut;
}

