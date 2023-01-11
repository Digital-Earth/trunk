/******************************************************************************
rhombus.cpp

begin		: 2009-11-01
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "rhombus.h"
#include "exceptions.h"

#include "pyxis/derm/index_math.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/icos_iterator.h"
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
// PYXRhombus
/////////////////////////////////////////////////////////////////////////////////////////

PYXRhombus::PYXRhombus()
{
}

PYXRhombus::PYXRhombus(int rootNumber)
{
	int icoshedriaRhombusNumber = rootNumber / 9;
	int icoshedriaRhombusOffset = rootNumber % 9;
	int v = icoshedriaRhombusOffset / 3;
	int u = icoshedriaRhombusOffset % 3;
	int direction = icoshedriaRhombusNumber < 5 ? icoshedriaRhombusNumber : icoshedriaRhombusNumber - 5;
	PYXIcosIndex pool = icoshedriaRhombusNumber < 5 ? PYXIcosIndex("1-0") : PYXIcosIndex("12-0");
	
	for (int dir = 1; dir <= 6; dir++)
	{
		if (PYXIcosMath::isValidDirection(pool.getPrimaryResolution(), (PYXMath::eHexDirection)dir))
		{
			if (direction>0)
			{
				direction--;
				continue;
			}
			PYXCursor c(pool,(PYXMath::eHexDirection)dir);
			c.left();
			c.forward(v);
			c.right();
			c.forward(u);

			*this = PYXRhombus(c);
			return;
		}
	}
}

PYXRhombus::PYXRhombus(const PYXCursor & cursor) 
{
	PYXCursor c(cursor.getIndex(),cursor.getDir());
	//c.zoomIn();

	m_indices[0] = c.getIndex();
	m_direction[0] = c.getDir();

	c.forward();	
	m_indices[1] = c.getIndex();
	if (m_indices[1].isHexagon())
	{
		c.left(1);
	}
	m_direction[1] = c.getDir();


	c.forward();
	c.left(1);
	m_indices[2] = c.getIndex();
	if (m_indices[2].isHexagon())
	{
		c.left(1);
	}
	m_direction[2] = c.getDir();

	c.forward();
	m_indices[3] = c.getIndex();
	if (m_indices[3].isHexagon())
	{
		c.left(1);
	}
	m_direction[3] = c.getDir();

	c.forward();
	assert(c.getIndex() == m_indices[0]);

	//do the other way around...
	c.setDir(cursor.getDir());	
	c.left();
	
	m_indices[0] = c.getIndex();
	m_revDirection[0] = c.getDir();

	c.forward();
	assert(m_indices[3] == c.getIndex());
	c.right(1);	
	m_revDirection[3] = c.getDir();

	c.forward();
	c.right(1);
	assert(m_indices[2] == c.getIndex());
	c.right(1);	
	m_revDirection[2] = c.getDir();

	c.forward();	
	assert(m_indices[1] == c.getIndex());
	c.right(1);	
	m_revDirection[1] = c.getDir();
	c.forward();

	assert(c.getIndex() == m_indices[0]);

#ifdef _DEBUG
	c.setDir(m_revDirection[0]);
	c.forward();
	assert(c.getIndex() == m_indices[3]);
	c.setDir(m_revDirection[3]);
	c.forward();
	assert(c.getIndex() == m_indices[2]);
	c.setDir(m_revDirection[2]);
	c.forward();
	assert(c.getIndex() == m_indices[1]);
	c.setDir(m_revDirection[1]);
	c.forward();
	assert(c.getIndex() == m_indices[0]);
#endif
	//TRACE_INFO(m_indices[0] << "(" << m_direction[0] << ")," << m_indices[1] << "," << m_indices[2] << "," << m_indices[3]);
}

PYXRhombus::PYXRhombus(const PYXIcosIndex & index,const PYXMath::eHexDirection & direction) 
{
	PYXCursor c(index,direction);
	//c.zoomIn();

	m_indices[0] = c.getIndex();
	m_direction[0] = c.getDir();

	c.forward();
	m_indices[1] = c.getIndex();
	if (m_indices[1].isHexagon())
	{
		c.left(1);
	}
	m_direction[1] = c.getDir();


	c.forward();
	c.left(1);
	m_indices[2] = c.getIndex();
	if (m_indices[2].isHexagon())
	{
		c.left(1);
	}
	m_direction[2] = c.getDir();

	c.forward();
	m_indices[3] = c.getIndex();
	if (m_indices[3].isHexagon())
	{
		c.left(1);
	}
	m_direction[3] = c.getDir();

	c.forward();
	assert(c.getIndex() == m_indices[0]);

	//do the other way around...
	c.setDir(direction);
	c.left();
	
	m_indices[0] = c.getIndex();
	m_revDirection[0] = c.getDir();

	c.forward();
	assert(m_indices[3] == c.getIndex());
	c.right(1);	
	m_revDirection[3] = c.getDir();

	c.forward();
	c.right(1);
	assert(m_indices[2] == c.getIndex());
	c.right(1);	
	m_revDirection[2] = c.getDir();

	c.forward();
	assert(m_indices[1] == c.getIndex());
	c.right(1);	
	m_revDirection[1] = c.getDir();
	c.forward();

	assert(c.getIndex() == m_indices[0]);

#ifdef _DEBUG
	c.setDir(m_revDirection[0]);
	c.forward();
	assert(c.getIndex() == m_indices[3]);
	c.setDir(m_revDirection[3]);
	c.forward();
	assert(c.getIndex() == m_indices[2]);
	c.setDir(m_revDirection[2]);
	c.forward();
	assert(c.getIndex() == m_indices[1]);
	c.setDir(m_revDirection[1]);
	c.forward();
	assert(c.getIndex() == m_indices[0]);
#endif
}

PYXRhombus::PYXRhombus(const PYXRhombus & other)
{
	for(int i=0;i<4;i++)
	{
		m_indices[i] = other.m_indices[i];
		m_direction[i] = other.m_direction[i];
		m_revDirection[i] = other.m_revDirection[i];
	}
}

PYXRhombus & PYXRhombus::operator=(const PYXRhombus & other)
{
	for(int i=0;i<4;i++)
	{
		m_indices[i] = other.m_indices[i];
		m_direction[i] = other.m_direction[i];
		m_revDirection[i] = other.m_revDirection[i];
	}
	return *this;
}

bool PYXRhombus::operator==(const PYXRhombus & other) const
{
	return m_indices[0] == other.m_indices[0] && m_indices[1] == other.m_indices[1] && m_indices[2] == other.m_indices[2] && m_indices[3] == other.m_indices[3];
}

PYXRhombus PYXRhombus::getSubRhombus(const int & index) const
{
	return getSubRhombus(index % 3,index / 3);
}

PYXRhombus PYXRhombus::getSubRhombus(const int & u,const int & v) const
{
	assert(u>=0 && u<=2 && v>=0 && v<=2 && "UV is out of range");

	PYXCursor c(m_indices[0],m_direction[0]);
	c.zoomIn();
	c.zoomIn();
	c.forward(u);
	c.left();
	c.forward(v);
	c.right();

	return PYXRhombus(c);
}

std::vector<PYXRhombus> PYXRhombus::getSubRhombi() const
{
	std::vector<PYXRhombus> result(9);

	PYXCursor c(m_indices[0],m_direction[0]);
	c.zoomIn();
	c.zoomIn();
	c.left();
	for(int v=0;v<3;v++)
	{
		PYXCursor c2(c);
		c2.right();
		for(int u=0;u<3;u++)
		{
			result[u+v*3] = PYXRhombus(c);
			c2.forward();
		}
		c.forward();
	}

	return result;
}

PYXIcosIndex PYXRhombus::getSubIndex(int u,int v) const
{
	assert(u>=0 && u<=3 && v>=0 && v<=3 && "UV is out of range");

	PYXCursor c(m_indices[0],m_direction[0]);
	
	c.zoomIn();
	c.zoomIn();
	c.forward(u);
	c.left();
	c.forward(v);
	
	return c.getIndex();
}

PYXIcosIndex PYXRhombus::getSubIndex(int u,int v,int depth) const
{
	int uvMax = getUVMax(depth);
	assert(u>=0 && u<=uvMax && v>=0 && v<=uvMax && "UV is out of range");

	PYXCursor c(m_indices[0],m_direction[0]);

	for(int d=0;d<depth;d++)
	{
		c.zoomIn();
	}
	c.forward(u);
	c.left();
	c.forward(v);

	return c.getIndex();
}

const PYXIcosIndex & PYXRhombus::getIndex(const int vertex) const
{
	assert(vertex >= 0 && vertex <= 3 && "Index is out of range");
	return m_indices[vertex];
}

const PYXMath::eHexDirection & PYXRhombus::getDirection(const int vertex) const
{
	assert(vertex >= 0 && vertex <= 3 && "Index is out of range");
	return m_direction[vertex];
}

const PYXMath::eHexDirection & PYXRhombus::getRevDirection(const int vertex) const
{
	assert(vertex >= 0 && vertex <= 3 && "Index is out of range");
	return m_revDirection[vertex];
}


bool PYXRhombus::isInside(const PYXIcosIndex & index) const
{
	int u,v;

	return isInside(index,&u,&v);
}

bool PYXRhombus::isInside(const PYXIcosIndex & index,double * u,double * v) const
{
	int intU,intV;

	if (isInside(index,&intU,&intV))
	{
		int coord_max = getUVMax(index.getResolution()-getIndex(0).getResolution());
		
		*u = static_cast<double>(intU)/coord_max;
		*v = static_cast<double>(intV)/coord_max;

		assert(*u>=0 && *u<=1 && "U is out of range");
		assert(*v>=0 && *v<=1 && "V is out of range");


		return true;
	}
	return false;
}

bool PYXRhombus::isInside(const PYXIcosIndex & index,int * u,int * v) const
{
	assert(	index.getResolution()>m_indices[0].getResolution() && 
			(index.getResolution()-m_indices[0].getResolution()) % 2 == 0 &&
			"Index resoltuion must be at with even offset from rhombus resolution");
	PYXIcosIndex rootIndex;
	int vertex=0;

	for(int ver=0;ver<4;ver++)
	{
		if (m_indices[ver].isAncestorOf(index))
		{
			rootIndex = m_indices[ver];
			vertex = ver;
			break;
		}
	}
	
	//trival case: we didn't find any parent
	if (rootIndex.isNull())
	{
		return false;
	}

	//trival case: the index is equal to the root.
	if (rootIndex == index)
	{
		return true;
	}
	
	//do further check
	return isInside(vertex,index,u,v);
}

bool PYXRhombus::intersects(const PYXIcosIndex & index) const
{
	for(int ver=0;ver<4;ver++)
	{
		if (m_indices[ver].isAncestorOf(index) || index.isAncestorOf(m_indices[ver]))
		{
			return true;
		}
	}
	return false;
}

int PYXRhombus::getUVMax(int resDepth) const
{
	return (int)(pow(3.0,resDepth/2));
}

bool PYXRhombus::factor(const PYXMath::eHexDirection & dir1,const PYXMath::eHexDirection & dir2,int * moveDir1,int * moveDir2, const PYXIndex & index) const
{
	int move2,move6;

	PYXMath::factor(index ,&move2,&move6);

	//convert the factor to our needed directions
	*moveDir1 = 0;
	*moveDir2 = 0;

	int iter=0;
	
	//while we didn't finish translating the coordinates
	while((move2 != 0 || move6 != 0) && iter++<5)
	{
		switch(dir1)
		{
		case PYXMath::knDirectionOne:
			if (move6==0)
			{
				*moveDir1 += move2;
				move6 -= move2;
				move2 = 0;
			}
			else
			{
				*moveDir1 += move6;
				move2 -= move6;
				move6 = 0;
			}
			break;
		case PYXMath::knDirectionTwo:
			*moveDir1 += move2;
			move2 = 0;
			break;
		case PYXMath::knDirectionThree:
			*moveDir1 += -move6;
			move6 = 0;
			break;
		case PYXMath::knDirectionFour:
			if (move6==0)
			{
				*moveDir1 -= move2;
				move6 -= move2;
				move2 = 0;
			}
			else
			{
				*moveDir1 -= move6;
				move2 -= move6;
				move6 = 0;
			}
			break;
		case PYXMath::knDirectionFive:
			*moveDir1 += -move2;
			move2 = 0;
			break;
		case PYXMath::knDirectionSix:
			*moveDir1 += move6;
			move6 = 0;
			break;
		}
		
		switch(dir2)
		{
		case PYXMath::knDirectionOne:
			if (move6==0)
			{
				*moveDir2 += move2;
				move6 -= move2;
				move2 = 0;
			}
			else
			{
				*moveDir2 += move6;
				move2 -= move6;
				move6 = 0;
			}
			break;
		case PYXMath::knDirectionTwo:
			*moveDir2 += move2;
			move2 = 0;
			break;
		case PYXMath::knDirectionThree:
			*moveDir2 += -move6;
			move6 = 0;
			break;
		case PYXMath::knDirectionFour:
			if (move6==0)
			{
				*moveDir2 -= move2;
				move6 -= move2;
				move2 = 0;
			}
			else
			{
				*moveDir2 -= move6;
				move2 -= move6;
				move6 = 0;
			}
			break;
		case PYXMath::knDirectionFive:
			*moveDir2 += -move2;
			move2 = 0;
			break;
		case PYXMath::knDirectionSix:
			*moveDir2 += move6;
			move6 = 0;
			break;
		}
	}

	return move2 ==0 && move6 == 0;
}

bool PYXRhombus::normalizeUV(int vertex,int resDepth,int * intU,int * intV) const
{
	int coord_max = getUVMax(resDepth);
	
	switch(vertex)
	{
	case 0:
		break;
	case 1:
		{
			int u = *intU;
			*intU = coord_max - *intV;
			*intV = u;
			break;
		}
	case 2:
		{
			*intU = coord_max - *intU;
			*intV = coord_max - *intV;
			break;
		}
	case 3:
		{
			int v = *intV;
			*intV = coord_max - *intU;
			*intU = v;
			break;
		}
	}

	return (*intU >= 0 && *intV >= 0 && *intU <= coord_max && *intV <= coord_max);
}

bool PYXRhombus::isInside(int vertex, const PYXIcosIndex & index, int * u,int * v) const
{
	//find factor in direction 2 and 6.
	PYXIndex subIndex = index.getSubIndex().subseq(m_indices[vertex].getSubIndex().getDigitCount());
	subIndex.prependDigit(0);
	
	if (m_indices[vertex].isPentagon())
	{
		if (m_indices[vertex].isNorthern())
		{
			//gap is direction 1 - which create a mass to us
			if (m_direction[vertex] == PYXMath::knDirectionFive && m_revDirection[vertex] == PYXMath::knDirectionTwo)
			{
				//we need the 2 direction - lets use it...
				if (subIndex.mostSignificant(NULL) == 2)
				{
					if (factor(PYXMath::knDirectionSix,PYXMath::knDirectionTwo,u,v,subIndex))
					{
						return normalizeUV(vertex,subIndex.getDigitCount(),u,v);
					}
				}
				else
				{
					//we don't need the 2 direction - move it 1...
					if (factor(PYXMath::knDirectionFive,PYXMath::knDirectionOne,u,v,subIndex))
					{
						return normalizeUV(vertex,subIndex.getDigitCount(),u,v);						
					}
				}
			}
			//we need to reduce the gap at 1
			else if (m_direction[vertex] == PYXMath::knDirectionSix && m_revDirection[vertex] == PYXMath::knDirectionTwo)
			{		
				if (factor(PYXMath::knDirectionSix,PYXMath::knDirectionTwo,u,v,subIndex))
				{
					normalizeUV(vertex,subIndex.getDigitCount(),u,v);					
					//reduce the missing gap
					if (subIndex.mostSignificant(NULL) == 2)
					{
						*v -= *u;
					}
					else
					{
						*u -= *v;
					}
					int coord_max = getUVMax(subIndex.getDigitCount());
					return (*u >= 0 && *v >= 0 && *u <= coord_max && *v <= coord_max );
				}
			}
		}
		else
		{
			//gap is direction 4 - which create a mass to us
			if (m_direction[vertex] == PYXMath::knDirectionTwo && m_revDirection[vertex] == PYXMath::knDirectionFive)
			{
				//we need the 5 direction - lets use it...
				if (subIndex.mostSignificant(NULL) == 5)
				{
					if (factor(PYXMath::knDirectionThree,PYXMath::knDirectionFive,u,v,subIndex))
					{
						return normalizeUV(vertex,subIndex.getDigitCount(),u,v);
					}
				}
				else
				{
					//we don't need the 5 direction - move it 4...
					if (factor(PYXMath::knDirectionTwo,PYXMath::knDirectionFour,u,v,subIndex))
					{
						return normalizeUV(vertex,subIndex.getDigitCount(),u,v);						
					}
				}
			}
			//we need to reduce the gap at 1
			else if (m_direction[vertex] == PYXMath::knDirectionThree && m_revDirection[vertex] == PYXMath::knDirectionFive)
			{		
				if (factor(PYXMath::knDirectionThree,PYXMath::knDirectionFive,u,v,subIndex))
				{
					normalizeUV(vertex,subIndex.getDigitCount(),u,v);
					//reduce the missing gap
					if (subIndex.mostSignificant(NULL) == 5)
					{
						*v -= *u;
					}
					else
					{
						*u -= *v;
					}
					int coord_max = getUVMax(subIndex.getDigitCount());
					return (*u >= 0 && *v >= 0 && *u <= coord_max && *v <= coord_max );
				}
			}
		}
	}
	
	if (factor(m_direction[vertex],m_revDirection[vertex],u,v,subIndex))
	{
		return normalizeUV(vertex,subIndex.getDigitCount(),u,v);
	}
	else
	{
		//we failed for some reason - this is bad!
		return false;
	}
}

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

void PYXRhombusFiller::addTile(PYXPointer<PYXTile> tile, PYXPointer<PYXValueTile> valueTile)
{
	//add the value tile into the map - but only if it not null - which could happen.
	if (valueTile.get() != NULL)
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

/////////////////////////////////////////////////////////////////////////////////////////
// Testing
/////////////////////////////////////////////////////////////////////////////////////////

Tester<PYXRhombus> gTester;

void PYXRhombus::test()
{
	//NOTE: This unit tests was automaticly generated by sampling the Globe.
	//      I don't beleive the tests are worng. I have create the test after validating the U,V and isInside manualy.
	//      [shatzi]

	int u,v;
	PYXRhombus rhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionSix);

	rhombus = PYXRhombus(PYXIcosIndex("3-002"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("3-00004020103"),&u,&v));
	TEST_ASSERT(u == 25 && v == 71);
	rhombus = PYXRhombus(PYXIcosIndex("3-002"),PYXMath::knDirectionFive);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("3-00050203006"),&u,&v));
	TEST_ASSERT(u == 68 && v == 34);
	rhombus = PYXRhombus(PYXIcosIndex("3-002"),PYXMath::knDirectionFive);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("3-00060100401"),&u,&v));
	TEST_ASSERT(u == 20 && v == 18);
	rhombus = PYXRhombus(PYXIcosIndex("3-205"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("3-00301020503"),&u,&v));
	TEST_ASSERT(u == 55 && v == 74);
	rhombus = PYXRhombus(PYXIcosIndex("3-040"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("3-00040050503"),&u,&v));
	TEST_ASSERT(u == 53 && v == 43);
	rhombus = PYXRhombus(PYXIcosIndex("3-300"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("3-30040505020"),&u,&v));
	TEST_ASSERT(u == 16 && v == 49);
	rhombus = PYXRhombus(PYXIcosIndex("3-203"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("A-05000502003"),&u,&v));
	TEST_ASSERT(u == 76 && v == 11);
	rhombus = PYXRhombus(PYXIcosIndex("3-202"),PYXMath::knDirectionFive);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("3-20001050040"),&u,&v));
	TEST_ASSERT(u == 65 && v == 26);
	rhombus = PYXRhombus(PYXIcosIndex("B-030"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("3-60206020203"),&u,&v));
	TEST_ASSERT(u == 55 && v == 14);
	rhombus = PYXRhombus(PYXIcosIndex("3-600"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("3-60502040004"),&u,&v));
	TEST_ASSERT(u == 10 && v == 54);
	rhombus = PYXRhombus(PYXIcosIndex("3-050"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("3-50100000401"),&u,&v));
	TEST_ASSERT(u == 2 && v == 81);
	rhombus = PYXRhombus(PYXIcosIndex("3-504"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("3-50306020104"),&u,&v));
	TEST_ASSERT(u == 29 && v == 63);
	rhombus = PYXRhombus(PYXIcosIndex("3-402"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("3-40000202050"),&u,&v));
	TEST_ASSERT(u == 11 && v == 59);
	rhombus = PYXRhombus(PYXIcosIndex("F-001"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("F-00603060103"),&u,&v));
	TEST_ASSERT(u == 16 && v == 62);
	rhombus = PYXRhombus(PYXIcosIndex("A-040"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("A-04005040030"),&u,&v));
	TEST_ASSERT(u == 11 && v == 26);
	rhombus = PYXRhombus(PYXIcosIndex("A-002"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("A-00402000103"),&u,&v));
	TEST_ASSERT(u == 79 && v == 53);
	rhombus = PYXRhombus(PYXIcosIndex("1-300"),PYXMath::knDirectionTwo);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("A-06010000600"),&u,&v));
	TEST_ASSERT(u == 51 && v == 57);
	rhombus = PYXRhombus(PYXIcosIndex("B-002"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("B-00030500106"),&u,&v));
	TEST_ASSERT(u == 41 && v == 73);
	rhombus = PYXRhombus(PYXIcosIndex("4-302"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("4-30020050401"),&u,&v));
	TEST_ASSERT(u == 29 && v == 36);
	rhombus = PYXRhombus(PYXIcosIndex("5-0000002"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-000000000030100"),&u,&v));
	TEST_ASSERT(u == 6 && v == 72);
	rhombus = PYXRhombus(PYXIcosIndex("5-0000205"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-000002000060606"),&u,&v));
	TEST_ASSERT(u == 68 && v == 13);
	rhombus = PYXRhombus(PYXIcosIndex("5-0000206"),PYXMath::knDirectionFive);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-000006006040060"),&u,&v));
	TEST_ASSERT(u == 61 && v == 28);
	rhombus = PYXRhombus(PYXIcosIndex("5-0000603"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-000005001050302"),&u,&v));
	TEST_ASSERT(u == 57 && v == 5);
	rhombus = PYXRhombus(PYXIcosIndex("5-0000406"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-000004030200600"),&u,&v));
	TEST_ASSERT(u == 21 && v == 42);
	rhombus = PYXRhombus(PYXIcosIndex("5-0000030"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-000003040105010"),&u,&v));
	TEST_ASSERT(u == 14 && v == 23);
	rhombus = PYXRhombus(PYXIcosIndex("5-0000002"),PYXMath::knDirectionFive);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-000000006030105"),&u,&v));
	TEST_ASSERT(u == 60 && v == 19);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-00040002040"),&u,&v));
	TEST_ASSERT(u == 31 && v == 22);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionThree);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-00030040050"),&u,&v));
	TEST_ASSERT(u == 25 && v == 37);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionTwo);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-00020030050"),&u,&v));
	TEST_ASSERT(u == 26 && v == 35);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionTwo);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-00020010001"),&u,&v));
	TEST_ASSERT(u == 37 && v == 17);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionSix);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-00205060605"),&u,&v));
	TEST_ASSERT(u == 12 && v == 41);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionSix);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-00060050206"),&u,&v));
	TEST_ASSERT(u == 34 && v == 21);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionFive);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-00050100606"),&u,&v));
	TEST_ASSERT(u == 9 && v == 40);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionFive);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-00050305020"),&u,&v));
	TEST_ASSERT(u == 38 && v == 11);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionTwo);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-00000000306"),&u,&v));
	TEST_ASSERT(u == 0 && v == 2);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionThree);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-00004010020"),&u,&v));
	TEST_ASSERT(u == 2 && v == 17);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionThree);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-00000301000"),&u,&v));
	TEST_ASSERT(u == 12 && v == 3);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionSix);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-00002050001"),&u,&v));
	TEST_ASSERT(u == 1 && v == 18);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionSix);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-00000604040"),&u,&v));
	TEST_ASSERT(u == 13 && v == 1);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionFive);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-00005020005"),&u,&v));
	TEST_ASSERT(u == 19 && v == 0);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionFive);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-00600300050"),&u,&v));
	TEST_ASSERT(u == 10 && v == 64);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionTwo);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-00200030605"),&u,&v));
	TEST_ASSERT(u == 80 && v == 6);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionThree);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-00300050200"),&u,&v));
	TEST_ASSERT(u == 75 && v == 6);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-04010000103"),&u,&v));
	TEST_ASSERT(u == 52 && v == 53);
	rhombus = PYXRhombus(PYXIcosIndex("1-005"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-04060602040"),&u,&v));
	TEST_ASSERT(u == 13 && v == 31);
	rhombus = PYXRhombus(PYXIcosIndex("1-000"),PYXMath::knDirectionFive);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-05000204060"),&u,&v));
	TEST_ASSERT(u == 77 && v == 71);
	rhombus = PYXRhombus(PYXIcosIndex("1-006"),PYXMath::knDirectionSix);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-06000402001"),&u,&v));
	TEST_ASSERT(u == 3 && v == 67);
	rhombus = PYXRhombus(PYXIcosIndex("1-002"),PYXMath::knDirectionTwo);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-20504030300"),&u,&v));
	TEST_ASSERT(u == 54 && v == 39);
	rhombus = PYXRhombus(PYXIcosIndex("1-003"),PYXMath::knDirectionThree);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-03010406020"),&u,&v));
	TEST_ASSERT(u == 17 && v == 41);
	rhombus = PYXRhombus(PYXIcosIndex("1-004"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("1-04030030050"),&u,&v));
	TEST_ASSERT(u == 62 && v == 47);
	rhombus = PYXRhombus(PYXIcosIndex("12-000"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-00000000103"),&u,&v));
	TEST_ASSERT(u == 2 && v == 1);
	rhombus = PYXRhombus(PYXIcosIndex("12-000"),PYXMath::knDirectionFive);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-00050001005"),&u,&v));
	TEST_ASSERT(u == 22 && v == 30);
	rhombus = PYXRhombus(PYXIcosIndex("12-000"),PYXMath::knDirectionThree);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-00030060100"),&u,&v));
	TEST_ASSERT(u == 18 && v == 24);
	rhombus = PYXRhombus(PYXIcosIndex("12-000"),PYXMath::knDirectionTwo);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-00020040505"),&u,&v));
	TEST_ASSERT(u == 14 && v == 36);
	rhombus = PYXRhombus(PYXIcosIndex("12-000"),PYXMath::knDirectionTwo);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-00020603004"),&u,&v));
	TEST_ASSERT(u == 32 && v == 16);
	rhombus = PYXRhombus(PYXIcosIndex("12-000"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-00010306003"),&u,&v));
	TEST_ASSERT(u == 14 && v == 34);
	rhombus = PYXRhombus(PYXIcosIndex("12-000"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-00010500006"),&u,&v));
	TEST_ASSERT(u == 37 && v == 8);
	rhombus = PYXRhombus(PYXIcosIndex("12-000"),PYXMath::knDirectionSix);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-00060103002"),&u,&v));
	TEST_ASSERT(u == 14 && v == 43);
	rhombus = PYXRhombus(PYXIcosIndex("12-000"),PYXMath::knDirectionSix);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-00060504010"),&u,&v));
	TEST_ASSERT(u == 47 && v == 14);
	rhombus = PYXRhombus(PYXIcosIndex("12-000"),PYXMath::knDirectionSix);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-00600020500"),&u,&v));
	TEST_ASSERT(u == 75 && v == 6);
	rhombus = PYXRhombus(PYXIcosIndex("12-000"),PYXMath::knDirectionThree);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-00500040104"),&u,&v));
	TEST_ASSERT(u == 7 && v == 81);
	rhombus = PYXRhombus(PYXIcosIndex("12-003"),PYXMath::knDirectionThree);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-00300003020"),&u,&v));
	TEST_ASSERT(u == 5 && v == 2);
	rhombus = PYXRhombus(PYXIcosIndex("12-002"),PYXMath::knDirectionTwo);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-00200020500"),&u,&v));
	TEST_ASSERT(u == 6 && v == 0);
	rhombus = PYXRhombus(PYXIcosIndex("12-001"),PYXMath::knDirectionSix);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-06020040200"),&u,&v));
	TEST_ASSERT(u == 24 && v == 21);
	rhombus = PYXRhombus(PYXIcosIndex("12-006"),PYXMath::knDirectionFive);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-05010010401"),&u,&v));
	TEST_ASSERT(u == 20 && v == 34);
	rhombus = PYXRhombus(PYXIcosIndex("12-000"),PYXMath::knDirectionThree);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-03060203005"),&u,&v));
	TEST_ASSERT(u == 74 && v == 49);
	rhombus = PYXRhombus(PYXIcosIndex("12-020"),PYXMath::knDirectionTwo);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-02000201000"),&u,&v));
	TEST_ASSERT(u == 15 && v == 6);
	rhombus = PYXRhombus(PYXIcosIndex("12-205"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-20105040205"),&u,&v));
	TEST_ASSERT(u == 72 && v == 56);
	rhombus = PYXRhombus(PYXIcosIndex("12-100"),PYXMath::knDirectionSix);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-10006010040"),&u,&v));
	TEST_ASSERT(u == 28 && v == 7);
	rhombus = PYXRhombus(PYXIcosIndex("12-600"),PYXMath::knDirectionSix);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-60602040500"),&u,&v));
	TEST_ASSERT(u == 57 && v == 15);
	rhombus = PYXRhombus(PYXIcosIndex("12-505"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("11-50506060105"),&u,&v));
	TEST_ASSERT(u == 39 && v == 44);
	rhombus = PYXRhombus(PYXIcosIndex("12-300"),PYXMath::knDirectionThree);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("T-05040200500"),&u,&v));
	TEST_ASSERT(u == 45 && v == 69);
	rhombus = PYXRhombus(PYXIcosIndex("12-301"),PYXMath::knDirectionTwo);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-30200105001"),&u,&v));
	TEST_ASSERT(u == 16 && v == 68);
	rhombus = PYXRhombus(PYXIcosIndex("12-202"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("12-20200020104"),&u,&v));
	TEST_ASSERT(u == 2 && v == 9);
	rhombus = PYXRhombus(PYXIcosIndex("Q-040"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("Q-00500304020"),&u,&v));
	TEST_ASSERT(u == 59 && v == 8);
	rhombus = PYXRhombus(PYXIcosIndex("R-030"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("R-03010050103"),&u,&v));
	TEST_ASSERT(u == 29 && v == 19);
	rhombus = PYXRhombus(PYXIcosIndex("9-005"),PYXMath::knDirectionTwo);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("9-00000000002"),&u,&v));
	TEST_ASSERT(u == 81 && v == 1);
	rhombus = PYXRhombus(PYXIcosIndex("9-010"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("9-00010000506"),&u,&v));
	TEST_ASSERT(u == 53 && v == 58);
	rhombus = PYXRhombus(PYXIcosIndex("9-005"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("9-00602030002"),&u,&v));
	TEST_ASSERT(u == 72 && v == 37);
	rhombus = PYXRhombus(PYXIcosIndex("9-005"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("9-00050003030"),&u,&v));
	TEST_ASSERT(u == 19 && v == 31);
	rhombus = PYXRhombus(PYXIcosIndex("9-005"),PYXMath::knDirectionTwo);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("9-00030060005"),&u,&v));
	TEST_ASSERT(u == 36 && v == 17);
	rhombus = PYXRhombus(PYXIcosIndex("9-005"),PYXMath::knDirectionTwo);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("9-00020006030"),&u,&v));
	TEST_ASSERT(u == 58 && v == 52);
	rhombus = PYXRhombus(PYXIcosIndex("9-005"),PYXMath::knDirectionTwo);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("9-00300006040"),&u,&v));
	TEST_ASSERT(u == 5 && v == 77);
	rhombus = PYXRhombus(PYXIcosIndex("9-502"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("9-00506004060"),&u,&v));
	TEST_ASSERT(u == 26 && v == 50);
	rhombus = PYXRhombus(PYXIcosIndex("9-502"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("9-05020100002"),&u,&v));
	TEST_ASSERT(u == 63 && v == 64);
	rhombus = PYXRhombus(PYXIcosIndex("9-060"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("9-06000103030"),&u,&v));
	TEST_ASSERT(u == 1 && v == 13);
	rhombus = PYXRhombus(PYXIcosIndex("9-103"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("9-00102010401"),&u,&v));
	TEST_ASSERT(u == 74 && v == 54);
	rhombus = PYXRhombus(PYXIcosIndex("9-010"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("9-01004000502"),&u,&v));
	TEST_ASSERT(u == 27 && v == 2);
	rhombus = PYXRhombus(PYXIcosIndex("9-306"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("9-02000200004"),&u,&v));
	TEST_ASSERT(u == 71 && v == 18);
	rhombus = PYXRhombus(PYXIcosIndex("9-030"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("9-02040504020"),&u,&v));
	TEST_ASSERT(u == 59 && v == 35);
	rhombus = PYXRhombus(PYXIcosIndex("9-005"),PYXMath::knDirectionTwo);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("9-00503000301"),&u,&v));
	TEST_ASSERT(u == 1 && v == 29);
	rhombus = PYXRhombus(PYXIcosIndex("5-002"),PYXMath::knDirectionFive);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-00000006005"),&u,&v));
	TEST_ASSERT(u == 75 && v == 4);
	rhombus = PYXRhombus(PYXIcosIndex("5-002"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-00030060106"),&u,&v));
	TEST_ASSERT(u == 41 && v == 64);
	rhombus = PYXRhombus(PYXIcosIndex("5-040"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-00040000604"),&u,&v));
	TEST_ASSERT(u == 56 && v == 51);
	rhombus = PYXRhombus(PYXIcosIndex("5-002"),PYXMath::knDirectionFive);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-00050060605"),&u,&v));
	TEST_ASSERT(u == 42 && v == 67);
	rhombus = PYXRhombus(PYXIcosIndex("5-002"),PYXMath::knDirectionFive);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-00060050405"),&u,&v));
	TEST_ASSERT(u == 30 && v == 37);
	rhombus = PYXRhombus(PYXIcosIndex("5-002"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-00020003002"),&u,&v));
	TEST_ASSERT(u == 33 && v == 23);
	rhombus = PYXRhombus(PYXIcosIndex("5-002"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-00200040606"),&u,&v));
	TEST_ASSERT(u == 5 && v == 4);
	rhombus = PYXRhombus(PYXIcosIndex("5-060"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-05002020106"),&u,&v));
	TEST_ASSERT(u == 77 && v == 46);
	rhombus = PYXRhombus(PYXIcosIndex("5-503"),PYXMath::knDirectionOne);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-04060300030"),&u,&v));
	TEST_ASSERT(u == 34 && v == 64);
	rhombus = PYXRhombus(PYXIcosIndex("5-003"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-03001050010"),&u,&v));
	TEST_ASSERT(u == 53 && v == 8);
	rhombus = PYXRhombus(PYXIcosIndex("5-020"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-00303030005"),&u,&v));
	TEST_ASSERT(u == 36 && v == 46);
	rhombus = PYXRhombus(PYXIcosIndex("5-002"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-00020505005"),&u,&v));
	TEST_ASSERT(u == 15 && v == 52);
	rhombus = PYXRhombus(PYXIcosIndex("5-002"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-00000206005"),&u,&v));
	TEST_ASSERT(u == 3 && v == 67);
	rhombus = PYXRhombus(PYXIcosIndex("5-002"),PYXMath::knDirectionFour);
	TEST_ASSERT(rhombus.isInside(PYXIcosIndex("5-00004000304"),&u,&v));
	TEST_ASSERT(u == 31 && v == 78);
}
