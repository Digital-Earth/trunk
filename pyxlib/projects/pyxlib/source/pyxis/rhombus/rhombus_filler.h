#pragma once
#ifndef PYXLIB__RHOMBUS_FILLER_H
#define PYXLIB__RHOMBUS_FILLER_H
/******************************************************************************
rhombus.h

begin		: 2009-11-01
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"
#include "pyxis/rhombus/rhombus.h"
#include "pyxis/derm/cursor.h"
#include "pyxis/derm/index.h"
#include "pyxis/data/value_tile.h"

#include <boost/thread/recursive_mutex.hpp>

#include <map>
#include <vector>


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
class PYXLIB_DECL PYXRhombusFiller : public PYXObject
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
	void addTile(const PYXPointer<PYXTile> & tile,const PYXPointer<PYXValueTile> & valueTile);

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