#pragma once
#ifndef VIEW_MODEL__STILE_H
#define VIEW_MODEL__STILE_H
/******************************************************************************
stile.h

begin		: 2007-08-08
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model.h"

// view model includes
#include "cml_utils.h"
#include "vdata.h"
#include "stile_mesh.h"

// pyxlib includes
#include "pyxis/data/coverage.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/derm/index.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/stl_utils.h"
#include "pyxis/utility/coord_lat_lon.h"
#include "pyxis/utility/memory_manager.h"

// standard includes
#include <map>
#include <vector>

// boost includes
#include <boost/detail/atomic_count.hpp>

#define STILE_COORD_TYPE float

//forward declearation
class Ray;

// Utility class to encapsulate a set of indexes that intersect with the STile.
class TileIndexSet
{
public:
	TileIndexSet (const PYXIcosIndex& rootIndex)
	{
	    m_indices[0] = rootIndex;
		m_indexDirections[0] = 0;
		m_nIndexCount = 1;
		if (rootIndex.isMinor())
		{
			// We're a minor tile so we have 3 major neighbours.
			std::vector<PYXIcosIndex> vec;
			PYXIcosMath::getNeighbours(rootIndex, &vec);
			for (int n = 0; n != 6; ++n)
			{
				if (!vec[n].isNull() && vec[n].isMajor())
				{
					m_indices[m_nIndexCount] = vec[n];
					m_indexDirections[m_nIndexCount] = n+1;
					m_nIndexCount++;
				}
			}
		}
	}

	// the number of index directions that are in use.
	int m_nIndexCount;

	// The indices of the corresponding tiles.
	PYXIcosIndex m_indices[4];

	// The directions from the main tile to the coresponding tile.
	int m_indexDirections[4];
};

// A utility class to help keep track of the imagery data
// and the elevation data between data fetching and data usage.
class TileDataSet
{
public:
	TileDataSet ()
	{
		clear();
	}

	void clear()
	{
		m_nTileCount = 0;
		for (int i=0; i<4; ++i)
		{
			m_tiles[i].swap(PYXPointer<PYXValueTile>());
		}
	}

	bool hasNullTile(int nCount)
	{
		for (int i=0; i < nCount; ++i)
		{
			if (!m_tiles[i])
			{
				return true;
			}
		}
		return false;
	}

	PYXPointer<PYXValueTile> m_tiles[4];

	// a count of how many data tiles are in the tiles array.
	int m_nTileCount;
};

/*!
A "square" tile.
*/
//! A "square" tile.
class VIEW_MODEL_API STile : public PYXObject, MemoryUsed
{
public:

	enum
	{
		knMeshWidth = 10,
		knMeshHeight = 10,
		knTexWidth = 244,
		knTexHeight = 244,
		knTexWidthPOT = 256,
		knTexHeightPOT = 256,

		// TODO run the math to figure out best numbers to use here (these seem OK but still)
		knLocalResMin =  23,  // determines when we switch between global and local origin (23 determined empirically)
		knLocalResDelta = 12, // not currently used [20080905] ticket #1297

		// Multum in parvo
		knMipLevels = 6,

		// MTF
		knTBufSize = 3*2*2*3 + 3*4*4*3 + 3*10*10*3 + 3*28*28*3 + 3*82*82*3 + 3*244*244*3
	};

	static PYXPointer<STile> create(const PYXIcosIndex& index)
	{
		return PYXNEW(STile, index);
	}

private:

	static const unsigned short m_triStripIndices[(knMeshHeight-1)*(knMeshWidth*2)];
	static const unsigned short m_triStripInfo[knMeshHeight];
	static const unsigned short m_triStripIndicesPent[knMeshHeight*knMeshWidth - 1];
	static const unsigned short m_triStripInfoPent[knMeshHeight];

	static const unsigned short m_grid0Indices[19];
	static const unsigned short m_grid0Info[2];
	static const unsigned short m_grid0IndicesPent[10];
	static const unsigned short m_grid0InfoPent[2];

	static const unsigned short m_grid2Indices[34];
	static const unsigned short m_grid2Info[2];
	static const unsigned short m_grid2IndicesPent[19];
	static const unsigned short m_grid2InfoPent[2];

	static const unsigned short m_grid4Indices[108];
	static const unsigned short m_grid4Info[5];
	static const unsigned short m_grid4IndicesPent[58];
	static const unsigned short m_grid4InfoPent[5];

	static const int m_mipResIncr[knMipLevels];
	static const int m_mipPO3[knMipLevels];
	static const int m_mipTriLength[knMipLevels];
	static const int m_mipTriOffset[knMipLevels][3];
	static const int m_mipTotalLength[knMipLevels];
	static const int m_mipOriginUV[knMipLevels][2];

	static STILE_COORD_TYPE m_texCoords[knMipLevels][2 * knMeshHeight*knMeshWidth];
	static STILE_COORD_TYPE m_texCoordsPent[knMipLevels][2 * ((knMeshHeight/2)*(knMeshWidth+1))];

public:

	static void init();

	static int getTextureWidth()
	{
		return 244;
	}

	static int getTextureHeight()
	{
		return 244;
	}

	static int getTextureSize()
	{
		return 3 * getTextureWidth() * getTextureHeight();
	}

	static void dumpTexels(const PYXIcosIndex& index, const unsigned char* pTexels);

	static const unsigned short* getTriStripIndices()
	{
		return m_triStripIndices;
	}

	static const unsigned short* getTriStripInfo()
	{
		return m_triStripInfo;
	}

	static const int getMipPO3(int nMipLevel);

	static const int* getMipOriginUV(int nMipLevel);

	static const unsigned char* getCheckerTexels();

	static const unsigned char* getCheckerTexelsPent();

public:

	static int getCount(const ProcRef& pipe);

	static int getTotalCount();

	static bool exists(const ProcRef& pipe, const PYXIcosIndex& index);

	static PYXPointer<STile> STile::getIfExists(const ProcRef& pipe, const PYXIcosIndex& index);

	static PYXPointer<STile> get(const ProcRef& pipe, const PYXIcosIndex& index);

	static void erase(const ProcRef& pipe, const PYXIcosIndex& index);

    static void invalidate(const ProcRef& pipe, PYXPointer<PYXGeometry> geometry);

	// TODO: investigate these to see if they are used.  Seems to smell. BR
	static void lockPipe(const ProcRef& pipe);
	static void unlockPipe(const ProcRef& pipe);

public:

	const PYXIcosIndex& getRootIndex() const
	{
		return m_index;
	}

	const vec3 & getCenterPosition() const
	{
		return m_pos;
	}

private:

	STile(const PYXIcosIndex& index);
	virtual ~STile();	

public:
	const STILE_COORD_TYPE* getVertexPointer(int n) const
	{
		return m_mesh->getFirstVertex(n);
	}

	const unsigned char* getTexels(int nMipLevel, int nTri) const
	{
		return m_tbuf + m_mipTriOffset[nMipLevel][nTri];
	}

	const unsigned char* getVectorsTexels(int nTri) const
	{
		return &(m_vectorTbuf[nTri*knTexWidth*knTexHeight*4]);
	}

	unsigned char* getVectorsTexels(int nTri)
	{
		return &(m_vectorTbuf[nTri*knTexWidth*knTexHeight*4]);
	}

	const unsigned short* getTriStripIndices(int nTri) const
	{
		return (nTri || isHexagon()) ? m_triStripIndices : m_triStripIndicesPent;
	}

	const unsigned short* getTriStripInfo(int nTri) const
	{
		return (nTri || isHexagon()) ? m_triStripInfo : m_triStripInfoPent;
	}

	const unsigned short* getGrid0Indices(int nTri) const
	{
		return (nTri || isHexagon()) ? m_grid0Indices : m_grid0IndicesPent;
	}

	const unsigned short* getGrid0Info(int nTri) const
	{
		return (nTri || isHexagon()) ? m_grid0Info : m_grid0InfoPent;
	}

	const unsigned short* getGrid2Indices(int nTri) const
	{
		return (nTri || isHexagon()) ? m_grid2Indices : m_grid2IndicesPent;
	}

	const unsigned short* getGrid2Info(int nTri) const
	{
		return (nTri || isHexagon()) ? m_grid2Info : m_grid2InfoPent;
	}

	const unsigned short* getGrid4Indices(int nTri) const
	{
		return (nTri || isHexagon()) ? m_grid4Indices : m_grid4IndicesPent;
	}

	const unsigned short* getGrid4Info(int nTri) const
	{
		return (nTri || isHexagon()) ? m_grid4Info : m_grid4InfoPent;
	}

	const STILE_COORD_TYPE* getTexCoordPointer(int nTexMip, int nTri) const
	{
		return (nTri || isHexagon()) ? m_texCoords[nTexMip] : m_texCoordsPent[nTexMip];
	}

	int getMipLevels() const
	{
		return m_nMipLevels;
	}

public:
	bool isHexagon() const
	{
		return m_bIsHexagon;
	}

	int getTileCode() const
	{
		return ((m_index.getClass() - 1) * 4) + (isHexagon() ? m_index.isMinor() : 2 + m_index.isSouthern());
	}

	boost::filesystem::path getLUTFilename(int knMipLevel, 
		PYXIcosIndex indices[4],
		int indexDirections[4]);

public:

	bool hasOrigin() const
	{
		return m_mesh->hasOrigin();
	}

	vec3 getOrigin() const
	{
		return m_mesh->getOrigin();
	}

public: // state management

	//! True if this tile has all of its data loaded and ready to go.
	bool isValid() { return m_bRasterFilled && m_bElevationFilled && m_bVectorFilled && m_bIconFilled; }

	//! True if the raster imagery is in the STile.
	bool isRasterFilled() { return m_bRasterFilled; }

	bool isElevationFilled() { return m_bElevationFilled; }

	bool isVectorFilled() { return m_bVectorFilled; }

	bool isVectorComposited() { return m_bVectorComposited; }

	bool isIconFilled() { return m_bIconFilled; }

	bool isRasterReloading() { return m_bRasterReloading; }
	
	//! True if the STile has been changed and needs flushing to disk cache.
	bool needsWritingToDisk() { return m_bNeedsWritingToDisk; }

	bool vectorNeedsWritingToGL() { return m_bVectorNeedsWritingToGL; }

	bool rasterNeedsWritingToGL() { return m_bRasterNeedsWritingToGL; }

	//! True if the STile is one of the tiles that is currently being viewed.
	bool isOnScreen() { return m_bOnScreen; }

	bool needsReloading() { return m_bNeedsReloading; }

	bool hadNullDataTile() { return m_bHadNullDataTile; }

	//! Mark this tile as needing data refreshed.
	void invalidate()
	{
		// TODO: we need to deal with the cached version of this object at this stage
		// either we need to save the state information with the cached tile and write 
		// it out again here, or we need to wipe the file from disk.

		// TODO: Possibly we should separate the caching mechanism from the STile and then
		// have the cache watch for OnChanged events from the STile.

		// If the STile is in the middle of being worked on, signal that it will need reloading
		// otherwise, clear all the flags.
		if (isProcessing())
		{
			m_bNeedsReloading = true;
		}
		else
		{
			m_bRasterFilled = false;
			m_bElevationFilled = false;
			m_bVectorFilled = false;
			m_bVectorComposited = false;
			m_bIconFilled = false;
		}
	}

	//! Return true if this tile intersects the given geometry.
	bool intersects(PYXPointer<PYXGeometry> geometry);

	//! Mark this tile as completely up-to-date.
	void validate()
	{
		m_bRasterFilled = true;
		m_bElevationFilled = true;
		m_bVectorFilled = true;
		m_bIconFilled = true;
	}

	void setRasterFilled(bool bState) { m_bRasterFilled = bState; }

	void setElevationFilled(bool bState) { m_bElevationFilled = bState; }

	void setVectorFilled(bool bState) { m_bVectorFilled = bState; }

	void setVectorComposited(bool bState) { m_bVectorComposited = bState; }

	void setIconFilled(bool bState) { m_bIconFilled = bState; }

	//! True if the STile has been changed and needs flushing to disk cache.
	void setNeedsWritingToDisk(bool bState) { m_bNeedsWritingToDisk = bState; }

	void setVectorNeedsWritingToGL(bool bState) { m_bVectorNeedsWritingToGL = bState; }

	void setRasterNeedsWritingToGL(bool bState) { m_bRasterNeedsWritingToGL = bState; }

	void setHadNullDataTile(bool bState) { m_bHadNullDataTile = bState; }

	void setOnScreen(bool bState) 
	{
		// If we are transitioning from Off Screen to On Screen, and we had
		// Null Data last time we tried to fetch data, then mark the tile as
		// needs reloading so we can try and get rid of snow flakes.
		if (hadNullDataTile() && !m_bOnScreen && bState)
		{
			setNeedsReloading(true);
		}
		m_bOnScreen = bState; 
	}

	void setNeedsReloading(bool bState) { m_bNeedsReloading = bState; }
	
	void setRasterReloading(bool bState) { m_bRasterReloading = bState; }
	
	bool isProcessing();
	
	void incrementProcessing();
	
	void decrementProcessing();
	
	
private:
	//! ***** State information for the STile *****

	//! True if the raster data is up-to-date
	bool m_bRasterFilled;

	//! Keep track of the state of the elevation filling.
	bool m_bElevationFilled;

	//! Keep track of the state of the vector filling.
	bool m_bVectorFilled;

	//! Keep track of the icons beign processd.
	bool m_bIconFilled;

	//! True if this tile has been changed and needs to be updated on disk.
	bool m_bNeedsWritingToDisk;

	//! true if the GL system needs to update with new vector information.
	bool m_bVectorNeedsWritingToGL;

	//! true if the GL system needs to update with new raster (texture) information.
	bool m_bRasterNeedsWritingToGL;

	//! True if this tile is currently on screen.
	bool m_bOnScreen;

	//! Represents the number of job lines this STile is in in the FillManager processing.
	boost::detail::atomic_count m_nProcessingCount;

	//! True if this tile is currently reloading.
	bool m_bRasterReloading;

	//! True if this tile needs to have the data reloaded.
	bool m_bNeedsReloading;

	//! True if any one of the input data tiles was null.
	bool m_bHadNullDataTile;

	//! Lock for fillElev() method to make it thread safe.
	boost::recursive_mutex m_fillElevationMutex;

	//! Lock between fillElev() and getElevation()
	boost::recursive_mutex m_elevationDataMutex;

	void adjustMeshForOrigin();

	bool readMTFFile(const std::string& strProcessIdentity);

private:
	//! The indices which intersect this STile.
	TileIndexSet m_indices;

	//! Get tiles of data.  Used by getElevationData and getImageData.
	void getData(boost::intrusive_ptr<ICoverage> spCov, TileDataSet& data, int nDataResolution, int nFieldIndex);

	//! Generate the look up table for Pyxis tile to STile translation.
	void generateLUT(const TileDataSet &data, unsigned char* nTileNumber, unsigned int* nTileOffset, int nMipLevel);

	//! Load the look up table for Pyxis tile to STile translation from disk.
	bool loadLUT(boost::filesystem::path srcFilename, unsigned char* nTileNumber, unsigned int* nTileOffset);

	//! Save the look up table for Pyxis tile to STile translation to disk.
	void saveLUT(boost::filesystem::path dstFilename, unsigned char* nTileNumber, unsigned int* nTileOffset);

public:
	void fillElev(boost::intrusive_ptr<ICoverage> spCov);
	void getElevationData(boost::intrusive_ptr<ICoverage> spCov);
	void fillElevationData();
	void fill(const std::string& strProcessIdentity, boost::intrusive_ptr<ICoverage> spCov);
	void getImageData(boost::intrusive_ptr<ICoverage> spCov);
	void fillImageData();
	
	//return elevation (in meters) for a given point in LatLon (in geocentric and not in not WGS84). on succuess return true.
	bool getElevation(const CoordLatLon & ll,double & elevation);	

	// buffer is expected to be 3*244*244*4 bytes
	// returns true if any cells were set
	bool fillVector(boost::intrusive_ptr<ICoverage> spCov, unsigned char* buf, unsigned int rgba);

	// fills value tiles from a vtree node
	bool fillValueTilesFromVTreeNode(const PYXIcosIndex* roots, PYXPointer<PYXValueTile>* tiles, int nTileCount, VectorPYXTree* pVTree, VectorPYXTree* pNode, const PYXIcosIndex& nodeIndex);

	// buffer is expected to be 3*244*244*4 bytes
	// returns true if any cells were set
	bool fillVector(boost::shared_ptr<VectorPYXTree> spVTree, unsigned char* buf, unsigned int rgba);

	//! Returns true if can fill from cache, but does not do it.
	bool canFillFromCache(const std::string& strProcessIdentity, int nMipLevel) const;

	//! Returns true if can fill from cache, and does it.
	bool fillFromCache(const std::string& strProcessIdentity, int nMipLevel);

	//! Returns true if the file was properly written to disk.
	bool writeToFile(const std::string& strProcessIdentity);

private:

	//! Root index of the tile.
	PYXIcosIndex m_index;

	//! Snyder projected position of this STile.
	vec3 m_pos;

	//! A shortcut for remembering if we are a Hexagonal tile.
	bool m_bIsHexagon;

	//! Imagery data tiles used to fill the STile.
	TileDataSet m_imageData;

	//! Elevation data tiles used to fill the STile.
	TileDataSet m_elevationData;

	//! The number of mip levels loaded. This should only be set by the fill thread.
	int m_nMipLevels;

	//! Origin for the tile.
	vec3 m_origin;

	//! Vertex buffers.
	PYXPointer<STileMesh> m_mesh;

	// Texel buffers.
	unsigned char m_tbuf[knTBufSize];

	
	// Vector Texel buffers.
	unsigned char m_vectorTbuf[3*knTexWidth*knTexHeight*4];

	bool m_bVectorComposited;
	

	unsigned int m_nElevationFillCount;

//Picking and intersections
public:
	bool intersects(const Ray & ray,double & time);

public:
	std::vector<PYXPointer<VectorRGBData>> m_vectorsGenerated;

	PYXPointer<STileMesh> getMesh()
	{
		boost::recursive_mutex::scoped_lock lock(m_elevationDataMutex);
		return m_mesh;
	}

	void setMesh(const PYXPointer<STileMesh> & mesh)
	{
		boost::recursive_mutex::scoped_lock lock(m_elevationDataMutex);
		m_mesh = mesh;
	}
};

inline bool operator < (const STile& lhs, const STile& rhs)
{
	return lhs.getRootIndex() < rhs.getRootIndex();
}


class VIEW_MODEL_API WeakPointerSTile : public PYXObject
{
protected:
	ProcRef m_procRef;
	PYXIcosIndex m_index;

public:
	WeakPointerSTile(const ProcRef & procRef,const PYXIcosIndex & index);	
	static PYXPointer<WeakPointerSTile> create(const ProcRef & procRef,const PYXIcosIndex & index);

	PYXPointer<STile> get();

	const ProcRef & getProcRef() const { return m_procRef; }
	const PYXIcosIndex & getIndex() const { return m_index; }
};


#endif
