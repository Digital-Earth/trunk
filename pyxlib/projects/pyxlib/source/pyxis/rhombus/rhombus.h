#pragma once
#ifndef PYXLIB__RHOMBUS_H
#define PYXLIB__RHOMBUS_H
/******************************************************************************
rhombus.h

begin		: 2009-11-01
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"
#include "pyxis/utility/object.h"
#include "pyxis/derm/cursor.h"
#include "pyxis/derm/index.h"
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
class PYXLIB_DECL PYXRhombus
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

class PYXLIB_DECL PYXRhombusCursor : public PYXAbstractIterator
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

#endif