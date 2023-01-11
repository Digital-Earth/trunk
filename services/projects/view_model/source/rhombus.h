#pragma once
#ifndef VIEW_MODEL__RHOMBUS_H
#define VIEW_MODEL__RHOMBUS_H
/******************************************************************************
rhombus.h

begin		: 2009-11-01
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "camera.h"
#include "cml_utils.h"

#include "pyxis/utility/object.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/notifier.h"
#include "pyxis/derm/cursor.h"
#include "pyxis/derm/index.h"
#include "pyxis/pipe/process.h"
#include "pyxis/data/value_tile.h"

#include <boost/thread/recursive_mutex.hpp>

#include <map>
#include <vector>

/*!
PYXRhombus - class that represent a Rhombus on a PYXGrid.

a Rhombus is defined by 4 neighbours cells at a resolution.
To construct a Rhombus you need to specify a PYXIcosIndex and a direction. and a Rhombus would selected the following cells:
 Index[0] = given Index
 Index[1] = move from given Index on the given Direction
 Index[2] = move from Index[1] on the given Direction + 1 Left
 Index[3] = move from Index[2] on the reverse direction of given Direction (which is +3)

The Rhombus class handle Pentagons as well (and rotations between neighbour cells)

you can access the PYXRhobmus indices and direction with the following API:
1. getIndex(vertex) - return the PYXIcosIndex of the vertex
2. getDirection(vertex) - return the direction to move from vertex to vertex+1
3. getRevDirection(vertex) - return the direction to move from vertex to vertex-1

A fundamental issue with Rhombus is whether a PYXIcosIndex is inside A Rhombus. 
the function isInsde(PYXIcosIndex) would return true if the cell intersect with a "Virtal Rhombus" created by using the center of each 4 vertices.
Therefore, the 4 cells created the Rhombus are inside the Rhombus, and so their centeriod children.

Moreover, a Rhombus has an internal coordinates: UV.
where U axis is on the Direction(0) - the direction from index #0 to index #1
and V axis is on the RevDirection(0) - the direction from index #0 to index #3.

Use Rhombus::isInsde(PYXIcosIndex,&u,&v) - to get the UV coordinate of that index if it inside the rhombus.
If UV are of type int. the values is between 0 and getUVMax(resolution difference between Index[0] and the given PYXIcosIndex).
IF UV are of type double. the values is between 0 and 1 (normalized by getUVMax).

Each Rhombus contains 9 sub Rhombi (which is two pyxis index resolutions deep). you can create those sub rhombus by using:
1. getSubRhombus(u,v) - where u and v are [0..2] - which represent the uv of the root index of the sub rhombus.
2. getSubRhombus(index) - where index is [0..8] - where index is translated to uv as follows: u = index%3, v=index/3. 
*/
class VIEW_MODEL_API PYXRhombus 
{
public:
#ifndef SWIG //swig ignore
	//!the root indices that define the rhombus
	PYXIcosIndex m_indices[4];
	//!the directions to move from index[i] to index[i+1]
	PYXMath::eHexDirection m_direction[4];
	//!the directions to move from index[i] to index[i-1]
	PYXMath::eHexDirection m_revDirection[4];
#endif
public:	
	PYXRhombus();

	//create rhombus from a root number.
	explicit PYXRhombus(int rootNumber);

	//create from index on its direction.
	PYXRhombus(const PYXCursor & cursor);
	//create from index on its direction.
	PYXRhombus(const PYXIcosIndex & index,const PYXMath::eHexDirection & direction);

	PYXRhombus(const PYXRhombus & other);

	PYXRhombus & operator=(const PYXRhombus & other);

	bool operator==(const PYXRhombus & other) const;

	//! get sub rhombus from index (value between 0 to 8 - included)
	PYXRhombus getSubRhombus(const int & index) const;

	//! get sub rhombus from u v (values between 0 and 2 - included)
	PYXRhombus getSubRhombus(const int & u,const int &  v) const;

	//! get all 9 sub rhombi
	std::vector<PYXRhombus> getSubRhombi() const;
	
	PYXIcosIndex getSubIndex(int u,int v) const;

	PYXIcosIndex getSubIndex(int u,int v,int depth) const;

	//! return the PYXIcosIndex of the vertex
	const PYXIcosIndex & getIndex(const int vertex) const;

	//! return the direction to move from vertex to vertex+1
	const PYXMath::eHexDirection & getDirection(const int vertex) const;

	//! return the direction to move from vertex to vertex-1
	const PYXMath::eHexDirection & getRevDirection(const int vertex) const;

	//! retrun true if index is inside the rhombus
	bool isInside(const PYXIcosIndex & index) const;

	//! retrun true if index intersects the rhombus
	bool intersects(const PYXIcosIndex & index) const;

	//! return true if index is inside the rhombus and return the integer UV coordinates of that index
	bool isInside(const PYXIcosIndex & index, int * coordU, int * coordV) const;

	//! return true if index is inside the rhombus and return the normalized UV coordinates of that index
	bool isInside(const PYXIcosIndex & index, double * coordU, double * coordV) const;

	//! return the maximum UV coordinates value for any given resolution difference.
	int getUVMax(int resDepth) const;	

protected:
	bool isInside(int vertex, const PYXIcosIndex & index,int * coordU,int * coordV) const;	
	bool factor(const PYXMath::eHexDirection & dir1,const PYXMath::eHexDirection & dir2,int * moveDir1,int * moveDir2, const PYXIndex & index) const;
	bool normalizeUV(int vertex,int resDepth,int * intU,int * intV) const;

public:
	static void test();
};

class VIEW_MODEL_API PYXRhombusCursor : public PYXAbstractIterator
{
private:
	PYXRhombus m_rhombus;
	int m_resolutionDepth;

	PYXCursor m_u_cursor;
	PYXCursor m_v_cursor;
	PYXIcosIndex m_index;
	int m_u;
	int m_v;
	int m_max;

	bool m_bEnd;

	bool m_oddResolution;
	int m_uvFactor;

public:
	PYXRhombusCursor(const PYXRhombus & rhombus,int resolutionDepth);
	PYXRhombusCursor(const PYXRhombusCursor & cursor);
	PYXRhombusCursor & operator=(const PYXRhombusCursor & cursor);

	virtual PYXRhombusCursor & operator ++() 
	{
		next();
		return *this;
	}

	virtual void next();
		
	virtual bool end() const;
		
	const PYXIcosIndex & getIndex() const { return m_index; }
	int getUCoord() const {return m_u / m_uvFactor;}
	int getVCoord() const {return m_v / m_uvFactor;}
	int getOffsetCoord() const {return m_v % m_uvFactor;}
	int getMaxUV() const {return m_max / m_uvFactor;}
	bool isOddResolutionDepth() const { return m_oddResolution; }
};

/*!
PYXRhombusFiller - class that responsible to loading needed PYXTile in order to fill a Rhombus with data.

Create a PYXRhombusFiller from a rhombus and a resolutionDepth (must be even number: 2,4,8,...) of the requested data to be field.

once created a PYXRhombusFiller:
1. use getNeededTile to request for needed tile.
2. once the data arrived, use addTile to specify the tile was loaded.
3. call isReady() to check if all tiles where added to the PYXRhombusFiller.

once a PYXRhombusFiller is ready. create a PYXRhombusFiller::Iterator to go over the loaded data in UV space.

the PYXRhombusFiller::Iterator API is:

1. getIndex() - return the current index of the iterator.
1. getValue() - return the current value loaded for that index.
3. hasValue() - return true if the current index has a value (if return false, the getValue has no meaning).
4. getUCoord(),getVCoord() - return the integer UV coordinates.
5. getOffsetCoord() - return the offset that need to be used to convert odd resolution data into 3 even resolution data.

as a PYXAbstractIterator, the PYXRhombusFiller::Iterator have next() and end() and operator++().

if the ResolutionDepth is even, a user can use the getOffsetCoord to sort the values into 3 rectangular tables.
data[u][v][offset=0] - values of all centriod child.
data[u][v][offset=1] - values of the upper vertex child. the cell that beetween [u][v]   - [u+1][v] - [u][v+1]
data[u][v][offset=2] - values of the lower vertex child. the cell that beetween [u+1][v] - [u][v+1] - [u+1][v+1]
*/
class VIEW_MODEL_API PYXRhombusFiller : public PYXObject
{
protected:
	PYXRhombus m_rhombus;
	int		   m_resolutionDepth;
	int		   m_tileDepth;

	typedef std::vector<PYXIcosIndex> IndciesVector;
	typedef std::map< PYXIcosIndex,PYXPointer<PYXValueTile> > TilesMap;

	IndciesVector m_needTiles;
	TilesMap	  m_tiles;	

public:
	PYXRhombusFiller(const PYXRhombus & rhombus, int resolutionDepth);
	PYXRhombusFiller(const PYXRhombus & rhombus, int resolutionDepth, int tileDepth);
	virtual ~PYXRhombusFiller();

	const PYXRhombus & getRhombus() const;
	const int & getResolutionDepth() const;
	int getCellResolution() const;

	bool isReady() const;
	//! return true if all the values tiles are null
	bool allValuesTilesAreNull() const;
	PYXPointer<PYXTile> getNeededTile() const;
	std::vector< PYXPointer<PYXTile> > getAllNeededTiles() const;
	void addTile(PYXPointer<PYXTile> tile,PYXPointer<PYXValueTile> valueTile);

	//! Get value for given cell index and channel, returns false for null value
	bool getValue(	const PYXIcosIndex& cellIndex,
					const int nChannelIndex,
					PYXValue* pValue) const;
	
public:
	class Iterator : public PYXAbstractIterator
	{
	protected:
		PYXRhombusFiller * m_filler;
		int m_channel;

		PYXValue m_value;
		bool m_hasValue;
		
		PYXRhombusCursor m_cursor;

	public:

		Iterator(PYXRhombusFiller & filler,int nChannelIndex);
		Iterator(const Iterator&);
		void operator =(const Iterator&);

		//! Destructor
		virtual ~Iterator() {}

		//! Simple operator of the form "++myIterator".
		virtual Iterator& operator ++() 
		{
			next();
			return *this;
		}
		
		virtual void next();
		
		virtual bool end() const;
		
		const PYXIcosIndex & getIndex() const { return m_cursor.getIndex(); }
		const PYXValue & getValue() const {return m_value; }
		const bool & hasValue() const {return m_hasValue; }
		int getUCoord() const {return m_cursor.getUCoord(); }
		int getVCoord() const {return m_cursor.getVCoord(); }
		int getOffsetCoord() const {return m_cursor.getOffsetCoord(); }
		int getMaxUV() const {return m_cursor.getMaxUV(); }
		bool isOddResolutionDepth() const { return m_cursor.isOddResolutionDepth(); }
	};

	class LUT : public PYXObject
	{
	protected:
		std::string m_key;
		int m_resolutionDepth;
		int m_max;
		int m_maxSqaure;
		boost::scoped_array<char> m_tileIndex;
		boost::scoped_array<int>  m_posIndex;

		static std::string getLUTIdentity(const PYXRhombusFiller & it);

	public:
		LUT(const PYXRhombusFiller & filler);

	public:
		int getMaxUV() const { return m_max-1; } //the uv can be from 0 to getMaxUV (included)
		bool isOddResolutionDepth() const { return m_resolutionDepth % 2 != 0; } 
		char getTileIndex(int u,int v,int offset) const { return m_tileIndex[m_max * v + u + m_maxSqaure * offset]; }
		int getPosIndex(int u,int v,int offset) const { return m_posIndex[m_max * v + u + m_maxSqaure * offset]; }

	protected:
		bool load(const PYXRhombusFiller & filler);
		void generate(const PYXRhombusFiller & filler);
		void save(const PYXRhombusFiller & filler);

	public:
		static PYXPointer<LUT> create(const PYXRhombusFiller & filler);

	private:
		static std::map< std::string, PYXPointer<LUT> >	s_LUTCache;
		static boost::recursive_mutex					s_LUTCacheMutex;
	};

	class IteratorWithLUT : public PYXAbstractIterator
	{
	protected:
		PYXRhombusFiller * m_filler;
		PYXPointer<LUT>    m_lut;
		std::vector< PYXPointer<PYXValueTile> > m_tiles;
		std::vector<int> m_tilesOffset;
		int m_channel;

		PYXValue m_value;
		bool m_hasValue;

		int m_u;
		int m_v;
		int m_offset;

		bool m_bEnd;

	public:

		IteratorWithLUT(PYXRhombusFiller & filler,int nChannelIndex);
		IteratorWithLUT(const IteratorWithLUT&);
		void operator =(const IteratorWithLUT&);

		//! Destructor
		virtual ~IteratorWithLUT() {}

		//! Simple operator of the form "++myIterator".
		virtual IteratorWithLUT & operator ++() 
		{
			next();
			return *this;
		}

		virtual void next();

		virtual bool end() const;

		const PYXValue & getValue() const {return m_value;}
		const bool & hasValue() const {return m_hasValue;}
		int getUCoord() const {return m_u;}
		int getVCoord() const {return m_v;}
		int getOffsetCoord() const {return m_offset;}
		
	};

	PYXRhombusFiller::Iterator getIterator(const int & nChannelIndex);

	PYXRhombusFiller::IteratorWithLUT getIteratorWithLUT(const int & nChannelIndex);
};

#endif