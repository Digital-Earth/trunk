/******************************************************************************
stile.cpp

begin		: 2007-08-08
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#include "stile.h"

// view model includes
#include "addr_utils.h"
#include "pyxtree_utils.h"
#include "view.h"
#include "view_open_gl_thread.h"

#include "performance_counter.h"
#include "tuv.h"
#include "ray.h"

// pyxlib includes
#include "pyxis/derm/cursor.h"
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/geometry/curve.h"
#include "pyxis/pipe/pipe_manager.h"
#include "pyxis/pipe/process_identity_cache.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/sphere_math.h"

// boost includes
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/thread/thread.hpp>

// third party includes
#include "zlib.h"

// standard includes
#include <fstream>
#include <map>
#include <set>
#include <sstream>

namespace
{

const SnyderProjection* pSnyder = SnyderProjection::getInstance();

unsigned char checkerTexels[] =
{
	#include "checker_data.txt"
};

unsigned char checkerTexelsPent[] =
{
	#include "checker_data_pent.txt"
};

// This is where we store all STiles.
// TODO: make an object that manages these statics

typedef std::map< PYXIcosIndex, PYXPointer<STile> > STileSet;
typedef std::map<ProcRef, STileSet> STileSetMap;
STileSetMap stilesetmap;
std::map<ProcRef, int> stilesetmaplock;

boost::recursive_mutex moduleMutex;

// TODO For performance, if we are doing a lot of work with a tile set for a particular procref,
// it may make sense to make it active so we don't have to keep looking in the stilesetmap.

// The visualization cache.
const ProcessIdentityCache & getCacheVis()
{
	static const ProcessIdentityCache cacheViz(AppServices::getCacheDir("viz"));	
	return cacheViz;
}

std::string getCacheFileName(const PYXIcosIndex& index)
{
	return index.toString() + ".mtf";
}

}

////////////////////////////////////////////////////////////////////////////////

const unsigned short STile::m_triStripIndices[(knMeshHeight-1)*(knMeshWidth*2)] =
{
	10,  0, 11,  1, 12,  2, 13,  3, 14,  4, 15,  5, 16,  6, 17,  7, 18,  8, 19,  9,
	20, 10, 21, 11, 22, 12, 23, 13, 24, 14, 25, 15, 26, 16, 27, 17, 28, 18, 29, 19,
	30, 20, 31, 21, 32, 22, 33, 23, 34, 24, 35, 25, 36, 26, 37, 27, 38, 28, 39, 29,
	40, 30, 41, 31, 42, 32, 43, 33, 44, 34, 45, 35, 46, 36, 47, 37, 48, 38, 49, 39,
	50, 40, 51, 41, 52, 42, 53, 43, 54, 44, 55, 45, 56, 46, 57, 47, 58, 48, 59, 49,
	60, 50, 61, 51, 62, 52, 63, 53, 64, 54, 65, 55, 66, 56, 67, 57, 68, 58, 69, 59,
	70, 60, 71, 61, 72, 62, 73, 63, 74, 64, 75, 65, 76, 66, 77, 67, 78, 68, 79, 69,
	80, 70, 81, 71, 82, 72, 83, 73, 84, 74, 85, 75, 86, 76, 87, 77, 88, 78, 89, 79,
	90, 80, 91, 81, 92, 82, 93, 83, 94, 84, 95, 85, 96, 86, 97, 87, 98, 88, 99, 89
};

const unsigned short STile::m_triStripIndicesPent[knMeshHeight*knMeshWidth - 1] =
{
	 0,  1, 10, 11, 19, 20, 27, 28, 34, 35, 40, 41, 45, 46, 49, 50, 52, 53, 54,
	 1,  2, 11, 12, 20, 21, 28, 29, 35, 36, 41, 42, 46, 47, 50, 51, 53,
     2,  3, 12, 13, 21, 22, 29, 30, 36, 37, 42, 43, 47, 48, 51,
	 3,  4, 13, 14, 22, 23, 30, 31, 37, 38, 43, 44, 48,
	 4,  5, 14, 15, 23, 24, 31, 32, 38, 39, 44,
	 5,  6, 15, 16, 24, 25, 32, 33, 39,
	 6,  7, 16, 17, 25, 26, 33,
	 7,  8, 17, 18, 26,
	 8,  9, 18
};

const unsigned short STile::m_triStripInfo[knMeshHeight] =
{
	9, 20, 20, 20, 20, 20, 20, 20, 20, 20
};

const unsigned short STile::m_triStripInfoPent[knMeshHeight] =
{
	9, 19, 17, 15, 13, 11, 9, 7, 5, 3
};

const unsigned short STile::m_grid0Indices[19] =
{
	 9, 19, 29, 39, 49, 59, 69, 79, 89, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90
};

const unsigned short STile::m_grid0Info[2] =
{
	1, 19
};

const unsigned short STile::m_grid0IndicesPent[10] =
{
	 9, 18, 26, 33, 39, 44, 48, 51, 53, 54
};

const unsigned short STile::m_grid0InfoPent[2] =
{
	1, 10
};

const unsigned short STile::m_grid2Indices[34] =
{
	33, 23, 13,  3,  4,  5,  6, 17, 28, 39, 49, 59, 69, 68, 67, 66, 76, 86, 96, 95, 94, 93, 82, 71, 60, 50, 40, 30, 31, 32, 33, 44, 55, 66
};

const unsigned short STile::m_grid2Info[2] =
{
	1, 34
};

const unsigned short STile::m_grid2IndicesPent[19] =
{
	3, 4, 5, 6, 16, 25, 33, 39, 44, 48, 47, 46, 45, 40, 34, 27, 20, 12, 3
};

const unsigned short STile::m_grid2InfoPent[2] =
{
	1, 19
};

const unsigned short STile::m_grid4Indices[108] =
{
	44, 34, 35, 25, 26, 16, 17,  7,  8, 19, 29, 28, 38, 37, 47, 46, 56, 55, 65, 64, 74, 73, 83, 82, 92, 91,	80, 70, 71, 61, 62, 52, 53, 43, 44, 55,
	22, 23, 13, 14,  4,  5, 16, 17, 28, 38, 49, 59, 58, 68, 67, 77, 76, 86, 85, 95, 94, 83, 82, 71, 61, 50, 40, 41, 31, 32,
	11,  1,  2, 13, 14, 25, 26, 37, 47, 58, 68, 79, 89, 88, 77, 67, 56, 46, 35, 34, 23,
	88, 98, 97, 86, 85, 74, 73, 62, 52, 41, 31, 20, 10, 11, 22, 32, 43, 53, 64, 65, 76
};

const unsigned short STile::m_grid4Info[5] =
{
	4, 36, 30, 21, 21
};

const unsigned short STile::m_grid4IndicesPent[58] =
{
	13, 22, 23, 31, 32, 39, 44, 43, 37, 36, 29, 28, 20, 19, 10,  1,  2, 12,
	20, 12, 13,  4,  5, 15, 23, 22, 29, 36, 41, 40, 34, 28,
	15, 16,  7,  8, 18, 26, 25, 32, 39, 44, 43, 47, 51, 53, 52, 49, 46, 41,
	16, 25, 32, 31, 37, 43, 47, 46
};

const unsigned short STile::m_grid4InfoPent[5] =
{
	4, 18, 14, 18, 8
};

////////////////////////////////////////////////////////////////////////////////

//! For a mip level, the number of PYXIS resolutions to add to the base.
const int STile::m_mipResIncr[knMipLevels] =
{
	1, 3, 5, 7, 9, 11
};

//! For a mip level, the power of three. Same as pow(3, (mipResIncr-1)/2)
const int STile::m_mipPO3[knMipLevels] =
{
	1, 3, 9, 27, 81, 243
};

//! For a mip level, the lengths of the tris.
const int STile::m_mipTriLength[knMipLevels] =
{
	2*2*3, 4*4*3, 10*10*3, 28*28*3, 82*82*3, 244*244*3
};

//! For a mip level, the offsets of the tris.
const int STile::m_mipTriOffset[knMipLevels][3] =
{
	{ 0, 12, 24 },
	{ 36, 84, 132 },
	{ 180, 480, 780 },
	{ 1080, 3432, 5784 },
	{ 8136, 28308, 48480 },
	{ 68652, 247260, 425868 },
};

//! For a mip level, the total length of all mip data for this and previous levels.
const int STile::m_mipTotalLength[knMipLevels] =
{
	3*2*2*3,
	3*2*2*3 + 3*4*4*3,
	3*2*2*3 + 3*4*4*3 + 3*10*10*3,
	3*2*2*3 + 3*4*4*3 + 3*10*10*3 + 3*28*28*3,
	3*2*2*3 + 3*4*4*3 + 3*10*10*3 + 3*28*28*3 + 3*82*82*3,
	3*2*2*3 + 3*4*4*3 + 3*10*10*3 + 3*28*28*3 + 3*82*82*3 + 3*244*244*3
};

//! For a mip level, the uv origin in the texture.
const int STile::m_mipOriginUV[knMipLevels][2] =
{
	{ 244, 14 }, { 244, 10 }, { 244, 0 }, { 0, 0 }, { 82, 0 }, { 0, 0 }
};

////////////////////////////////////////////////////////////////////////////////

STILE_COORD_TYPE STile::m_texCoords[knMipLevels][2 * knMeshHeight*knMeshWidth];

STILE_COORD_TYPE STile::m_texCoordsPent[knMipLevels][2 * ((knMeshHeight/2)*(knMeshWidth+1))];

////////////////////////////////////////////////////////////////////////////////

static struct AutoInit { AutoInit() { STile::init(); } } autoInit;

////////////////////////////////////////////////////////////////////////////////

void STile::dumpTexels(const PYXIcosIndex& index, const unsigned char* pTexels)
{
	boost::filesystem::path texelsdir = "texels";
	if (!FileUtils::exists(texelsdir))
	{
		create_directory(texelsdir);
	}

	boost::filesystem::path rgbfile = texelsdir / (index.toString() + ".rgb");
	boost::filesystem::path pngfile = texelsdir / (index.toString() + ".png");

	std::ofstream file(FileUtils::pathToString(rgbfile).c_str(), std::ios::binary);
	file.write(reinterpret_cast<const char*>(pTexels), getTextureSize());
	file.close();
	std::string cmd = std::string("convert -size 32x") + StringUtils::toString(getTextureHeight()) + " -depth 8 " + 
		FileUtils::pathToString(rgbfile) + " " + FileUtils::pathToString(pngfile);
	system(cmd.c_str());
	remove(rgbfile);
}

void STile::init()
{
	// Calculate the texture coordinates.
	for (int nMipLevel = 0; nMipLevel != knMipLevels; ++nMipLevel)
	{
		STILE_COORD_TYPE* ph = m_texCoords[nMipLevel];
		STILE_COORD_TYPE* pp = m_texCoordsPent[nMipLevel];
		const float fPO3 = static_cast<float>(m_mipPO3[nMipLevel]);
		const float uscale = fPO3 / (knMeshHeight - 1);
		const float vscale = fPO3 / (knMeshHeight - 1);
		for (int v = 0; v != knMeshHeight; ++v)
		{
			STILE_COORD_TYPE t = (m_mipOriginUV[nMipLevel][1] + 0.5f + v*vscale) / knTexHeightPOT;
			const int nPentCount = knMeshWidth - v;
			for (int u = 0; u != knMeshWidth; ++u)
			{
				STILE_COORD_TYPE s = (m_mipOriginUV[nMipLevel][0] + 0.5f + u*uscale) / knTexWidthPOT;
				*ph++ = s;
				*ph++ = t;
				if (u < nPentCount)
				{
					*pp++ = s;
					*pp++ = t;
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Manager functions

/*!
\param pipe		The pipeline's procref.
\return The number of stiles for that pipeline (0 if the pipeline is unknown).
*/
int STile::getCount(const ProcRef& pipe)
{
	boost::recursive_mutex::scoped_lock lock(moduleMutex);

	int nCount = 0;
	STileSetMap::iterator it = stilesetmap.find(pipe);
	if (it != stilesetmap.end())
	{
		return static_cast<int>(it->second.size());
	}
	return nCount;
}

int STile::getTotalCount()
{
	boost::recursive_mutex::scoped_lock lock(moduleMutex);

	int nCount = 0;
	STileSetMap::iterator it = stilesetmap.begin();
	while (it != stilesetmap.end())
	{
		nCount += static_cast<int>(it->second.size());
		++it;
	}
	return nCount;
}

// Static 
bool STile::exists(const ProcRef& pipe, const PYXIcosIndex& index)
{
	boost::recursive_mutex::scoped_lock lock(moduleMutex);

	bool bExists = false;
	STileSetMap::iterator it = stilesetmap.find(pipe);
	if (it != stilesetmap.end())
	{
		bExists = it->second.find(index) != it->second.end();
	}
	return bExists;
}

// Static 
PYXPointer<STile> STile::getIfExists(const ProcRef& pipe, const PYXIcosIndex& index)
{
	boost::recursive_mutex::scoped_lock lock(moduleMutex);
	
	STileSetMap::iterator it = stilesetmap.find(pipe);
	if (it != stilesetmap.end())
	{
		STileSet::iterator it2 = it->second.find(index);
		if (it2 != it->second.end())
		{
			return it2->second;		
		}
	}
	return NULL;
}

// Static 
PYXPointer<STile> STile::get(const ProcRef& pipe, const PYXIcosIndex& index)
{
	boost::recursive_mutex::scoped_lock lock(moduleMutex);

	PYXPointer<STile> spTile = stilesetmap[pipe][index];
	if (!spTile)
	{
		try
		{
			spTile = STile::create(index);
		}
		catch (...)
		{
			PYXTHROW(PYXException,"failed to create STile for " << pipe << " and " << index);
		}
		stilesetmap[pipe][index] = spTile;
	}
	return spTile;
}

// Static 
void STile::erase(const ProcRef& pipe, const PYXIcosIndex& index)
{
	boost::recursive_mutex::scoped_lock lock(moduleMutex);
	stilesetmap[pipe].erase(index);
}

// Static 
void STile::invalidate(const ProcRef& pipe, PYXPointer<PYXGeometry> geometry)
{
	boost::recursive_mutex::scoped_lock lock(moduleMutex);

	STileSet& stileset = stilesetmap[pipe];
	for (STileSet::iterator it = stileset.begin(); it != stileset.end(); ++it)
	{
		if (it->second->intersects(geometry))
		{
			it->second->setNeedsReloading(true);
		}
	}
}

void STile::lockPipe(const ProcRef& pipe)
{
	boost::recursive_mutex::scoped_lock lock(moduleMutex);

	++stilesetmaplock[pipe];
}

void STile::unlockPipe(const ProcRef& pipe)
{
	boost::recursive_mutex::scoped_lock lock(moduleMutex);

	assert(stilesetmaplock.find(pipe) != stilesetmaplock.end());
	if (!--stilesetmaplock[pipe])
	{
		stilesetmaplock.erase(stilesetmaplock.find(pipe));
		stilesetmap.erase(stilesetmap.find(pipe));
	}
}

////////////////////////////////////////////////////////////////////////////////
// Data retrieval utility functions.

// Utility function for getting a value from a set of PyxValueTiles by PYXIcosIndex. 
// Returns false if the retrieved value should be treated as a Null value.
bool getValue(PYXPointer<PYXValueTile>* tiles, int nTileCount, const PYXIcosIndex& index, 
			  int nField, PYXValue& val)
{
	for (int nTile = 0; nTile < nTileCount; ++nTile)
	{
		if (tiles[nTile] && tiles[nTile]->getTile().getRootIndex().isAncestorOf(index))
		{
			return tiles[nTile]->getValue(index, nField, &val);
		}
	}
	return false;
}

// Utility function for getting a value from a set of PyxValueTiles by tile number and offset into 
// that tile.  Returns false if the retrieved value should be treated as a Null value.
bool getValue(PYXPointer<PYXValueTile>* tiles, unsigned char nTileNum, unsigned int nTileOffset, 
			  int nField, PYXValue& val)
{
	if (tiles[nTileNum])
	{
		return tiles[nTileNum]->getValue(nTileOffset, nField, &val);
	}
	return false;
}

// Utility function used to generate the look up tables.  nTileNum and nTileOffset are set
// and used as output vlaues from this function.  They are set to the tile number and the
// offset into the ValueTile that is the location of the index that is passed in in the set
// of tiles root indices taht is passed in.
void whereValue(const PYXIcosIndex rootIndices[4], int nTileCount, const PYXIcosIndex& index, 
				unsigned char *nTileNum, unsigned int *nTileOffset)
{
	for (int nTile = 0; nTile < nTileCount; ++nTile)
	{
		if (rootIndices[nTile].isAncestorOf(index))
		{
			*nTileNum = nTile;
			*nTileOffset = PYXIcosMath::calcCellPosition(rootIndices[nTile], index);
			return;
		}
	}
	PYXTHROW(PYXException, "Data Error");
}

////////////////////////////////////////////////////////////////////////////////
// ??? functions


STile::STile(const PYXIcosIndex& index) :
	MemoryUsed(sizeof(STile)),
	m_index(index),
	m_nMipLevels(0),
	m_bElevationFilled(false),
	m_bRasterFilled(false),
	m_bVectorFilled(false),
	m_bVectorComposited(false),
	m_bIconFilled(false),
	m_bNeedsWritingToDisk(false),
	m_bVectorNeedsWritingToGL(false),
	m_bRasterNeedsWritingToGL(false),
	m_bNeedsReloading(false),
	m_bOnScreen(false),
	m_bHadNullDataTile(false),
	m_indices(index),
	m_nProcessingCount(0)
{	
	m_bIsHexagon = m_index.isHexagon();
	m_nElevationFillCount = 0;
	
	m_pos  = CmlConvertor::toVec3(m_index);
	m_mesh = STileMesh::create(m_index);

	PerformanceCounter::getValuePerformanceCounter("STile count",1.0f,0.0f,0.0f)->addToMeasurement(1);
}

STile::~STile()
{
	PerformanceCounter::getValuePerformanceCounter("STile count",1.0f,0.0f,0.0f)->addToMeasurement(-1);
}

bool STile::isProcessing() 
{ 
	return (m_nProcessingCount > 0); 
}

void STile::incrementProcessing() 
{  
	++m_nProcessingCount; 
}

void STile::decrementProcessing() 
{  
	--m_nProcessingCount; 
}

const int STile::getMipPO3(int nMipLevel)
{
	return m_mipPO3[nMipLevel];
}

const int* STile::getMipOriginUV(int nMipLevel)
{
	return m_mipOriginUV[nMipLevel];
}

const unsigned char* STile::getCheckerTexels()
{
	return checkerTexels;
}

const unsigned char* STile::getCheckerTexelsPent()
{
	return checkerTexelsPent;
}

void STile::getData(boost::intrusive_ptr<ICoverage> spCov, TileDataSet& data, int nDataResolution, int nFieldIndex)
{
	assert(spCov);

	data.clear();
	PYXPointer<PYXValueTile> spVT;

	for (int n = 0; n != m_indices.m_nIndexCount; ++n)
	{
		try
		{
			spVT = spCov->getFieldTile(m_indices.m_indices[n], nDataResolution, nFieldIndex);
			if (spVT)
			{
				data.m_tiles[n] = spVT;
				++(data.m_nTileCount);
			}
		}
		catch (PYXException& ex)
		{
			TRACE_ERROR(ex.getFullErrorString());
			invalidate();
		}
		catch (...)
		{
			TRACE_ERROR("Exception in fill thread while attempting to get a PYXValueTile!");
			invalidate();
		}
	}
}

void STile::fillElev(boost::intrusive_ptr<ICoverage> spCov)
{
	boost::recursive_mutex::scoped_lock lock(m_fillElevationMutex);
	if (!m_bElevationFilled)
	{
		getElevationData(spCov);
		fillElevationData();
		m_bElevationFilled = true;
	}
}

bool STile::intersects(PYXPointer<PYXGeometry> geometry)
{
	for (int n = 0; n != m_indices.m_nIndexCount; ++n)
	{
		if (geometry->intersects(PYXTile(m_indices.m_indices[n], m_indices.m_indices[n].getResolution()+1)))
		{
			// Short circut return if we find any intersection.
			return true;
		}
	}
	// we found no intersect.
	return false;
}

void STile::getElevationData(boost::intrusive_ptr<ICoverage> spCov)
{
	getData(spCov, m_elevationData, m_index.getResolution() + 5, 1);
}

void STile::fillElevationData()
{
	boost::recursive_mutex::scoped_lock lock(m_fillElevationMutex);

	PYXPointer<STileMesh> newMesh = STileMesh::create(m_index);

	if (m_elevationData.m_nTileCount != 0)
	{
		STILE_COORD_TYPE* p[3] = { newMesh->getFirstVertex(0), newMesh->getFirstVertex(1), newMesh->getFirstVertex(2) };

		int nValidIndex = 0;
		while (!m_elevationData.m_tiles[nValidIndex])
		{
			++nValidIndex;
		}
		PYXValue val = m_elevationData.m_tiles[nValidIndex]->getTypeCompatibleValue(0);

		const int elevationMipLevel = 2;

		// the look up tables
		boost::scoped_array<unsigned char> nTileNumber(new unsigned char[knTBufSize/3]);
		boost::scoped_array<unsigned int> nTileOffset(new unsigned int[knTBufSize/3]);
		// These were defined using the following syntax, but having local
		// variables this large introduced problems.  Specifically this
		// introduced the "Reentrant call in managed code" bug when doing a TRACE.
		//unsigned char* nTileNumber = new unsigned char[knTBufSize/3];
		//unsigned int* nTileOffset = new unsigned int[knTBufSize/3];

		const int knSize = m_mipPO3[elevationMipLevel] + 1;

		// find out if we have the look up table on disk
		boost::filesystem::path fname = getLUTFilename(elevationMipLevel, m_indices.m_indices, m_indices.m_indexDirections);
		if (!loadLUT(fname, nTileNumber.get(), nTileOffset.get()))
		{
			generateLUT(m_elevationData, nTileNumber.get(), nTileOffset.get(), elevationMipLevel);
			saveLUT(fname, nTileNumber.get(), nTileOffset.get());
		}

		// Control constants for the vertical scale exaggeration
		const int highClampResolution = 28;
		const double highResolutionScaleFactor = 1.0;
		const int lowClampResolution = 18;
		const double lowResolutionScaleFactor = 10.0;

		// Elevation exaggeration factor
		double scaleFactor = lowResolutionScaleFactor;
		int displayResolution = m_index.getResolution() + 11;
		if (displayResolution >= highClampResolution)
		{
			scaleFactor = highResolutionScaleFactor;
		}
		else
		{
			if (displayResolution > lowClampResolution)
			{
#if FALSE
				// Linear scaling... jumps too much at low resolutions...
				scaleFactor = highResolutionScaleFactor +
					((lowResolutionScaleFactor - highResolutionScaleFactor) * 
					 ((highClampResolution - displayResolution) / 
					  (double) (highClampResolution - lowClampResolution)));
#else
				// Logarithmic scaling.  Smoother visually.
				// Res Scale
				//	18	10
				//	19	9
				//	20	6.68
				//	21	5.33
				//	22	4.37
				//	23	3.63
				//	24	3.02
				//	25	2.50
				//	26	2.06
				//	27	1.67
				//	28	1
				scaleFactor = highResolutionScaleFactor +
					((lowResolutionScaleFactor - 2.0 * highResolutionScaleFactor) * 
					(1.0 - log((double)displayResolution-lowClampResolution) / 
					log((double)highClampResolution - lowClampResolution + 1.0)));
#endif
			}
		}

		// Copy the data from the Value tiles to the sTile
		int nTableIndex = 0;
		for (int v = 0; v != knSize; ++v)
		{
			// Number of cells on this row if a pentagon.
			const int nPentCount = knSize - v;

			for (int u = 0; u != knSize; ++u)
			{
				if (isHexagon() || u < nPentCount)
				{
					if (getValue(m_elevationData.m_tiles, nTileNumber.get()[nTableIndex], nTileOffset.get()[nTableIndex], 0, val))
					{
						newMesh->scaleVertex(p[0],(SphereMath::knEarthRadius+val.getDouble()*scaleFactor)/SphereMath::knEarthRadius);
					} 
					p[0]+=3;

					nTableIndex++;
				}

				if (getValue(m_elevationData.m_tiles, nTileNumber.get()[nTableIndex], nTileOffset.get()[nTableIndex], 0, val))
				{
					newMesh->scaleVertex(p[1],(SphereMath::knEarthRadius+val.getDouble()*scaleFactor)/SphereMath::knEarthRadius);
				}
				p[1]+=3;
				nTableIndex++;

				if (getValue(m_elevationData.m_tiles, nTileNumber.get()[nTableIndex], nTileOffset.get()[nTableIndex], 0, val))
				{
					newMesh->scaleVertex(p[2],(SphereMath::knEarthRadius+val.getDouble()*scaleFactor)/SphereMath::knEarthRadius);
				}
				p[2]+=3;
				nTableIndex++;
			}
		}
	}

	++m_nElevationFillCount;
	{
		boost::recursive_mutex::scoped_lock lock(m_elevationDataMutex);

		m_mesh = newMesh;
	}
	setElevationFilled(true);
	// TODO: if we ever cache the elevation, we should pay attention to 
	// if we had a null tile.
	
	m_elevationData.clear();
}

bool STile::getElevation(const CoordLatLon & ll,double & elevation)
{
	boost::recursive_mutex::scoped_lock lock(m_elevationDataMutex);

	return m_mesh->getElevation(ll,elevation);	
}

void STile::fill(const std::string& strProcessIdentity, boost::intrusive_ptr<ICoverage> spCov)
{
	assert(!strProcessIdentity.empty());
	assert(spCov);

	getImageData(spCov);
	fillImageData();

	if (getMipLevels() == knMipLevels)
	{
		// Only write a complete file so we don't have issues reading partial files
		writeToFile(strProcessIdentity);
	}
}

bool STile::canFillFromCache(const std::string& strProcessIdentity, int nMipLevel) const
{
	assert(!strProcessIdentity.empty());

	boost::filesystem::path fname = getCacheVis().getPath(strProcessIdentity, false);
	if (fname.empty())
	{
		return false;
	}
	fname /= getCacheFileName(m_index);

	// Optimistically assume if the file exists, we can read it.
	return FileUtils::exists(fname);
}

bool STile::fillFromCache(const std::string& strProcessIdentity, int nMipLevel)
{
	assert(!strProcessIdentity.empty());

	if (readMTFFile(strProcessIdentity))
	{		
		return true;
	}

	return false;
}

// Load a Look Up Table from disk (LUT)
// Returns true if the file was successfully loaded.
bool STile::loadLUT(boost::filesystem::path srcFilename, unsigned char* nTileNumber, unsigned int* nTileOffset)
{
	bool bLUTLoaded = false;

	try
	{
		if (FileUtils::exists(srcFilename))
		{
			// load the table
			std::ifstream file(FileUtils::pathToString(srcFilename).c_str(), std::ios::binary);

			const char* const magic = "lut"; // note this is 4 bytes not 3!
			char buf[8] = { 0 }; // init buffer to zero

			file.read(buf, 8);
			if (memcmp(magic, buf, 4) == 0)
			{
				int& nVersion = *reinterpret_cast<int*>(buf + 4);
				if (nVersion == 1)
				{
					file.read(reinterpret_cast<char*>(nTileNumber), STile::knTBufSize/3);
					file.read(reinterpret_cast<char*>(nTileOffset), STile::knTBufSize/3 * sizeof(unsigned int));
					file.close();
					bLUTLoaded = file.good();
				}
				if (nVersion == 2)
				{
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
					memIn.read(reinterpret_cast<char*>(nTileNumber), STile::knTBufSize/3);
					memIn.read(reinterpret_cast<char*>(nTileOffset), STile::knTBufSize/3 * sizeof(unsigned int));
					bLUTLoaded = memIn.good();
				}
			}
		}
	}
	catch (PYXException& ex)
	{
		TRACE_ERROR("Failed lo load lut file : " + ex.getFullErrorString());		
	}
	catch (...)
	{
		TRACE_ERROR("Failed lo load lut file : unknown error");		
	}

	return bLUTLoaded;
}

// Save a Look Up Table (LUT) to disk.
void STile::saveLUT(boost::filesystem::path dstFilename, unsigned char* nTileNumber, unsigned int* nTileOffset)
{
	// create a memory stream for the tile so we can compress it before it goes on disk.
	std::ostringstream memOut;
	
	memOut.write(reinterpret_cast<char*>(nTileNumber), STile::knTBufSize/3);
	memOut.write(reinterpret_cast<char*>(nTileOffset), STile::knTBufSize/3 * sizeof(unsigned int));

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

	// create the output file
	std::ofstream file(FileUtils::pathToString(dstFilename).c_str(), std::ios::binary);

	// Write magic number.
	file.write("lut", 4);

	// Write version.
	int nVersion = 2;
	file.write(reinterpret_cast<const char*>(&nVersion), 4);

	// write the compresed data
	file.write((char*)&nUnCompressedLength,sizeof(unsigned long));
	file.write((char*)&nCompressedBufferLength,sizeof(unsigned long));
	file.write(pCompressedDataBuffer.get(), nCompressedBufferLength);

	file.close();
}

/* Utility function for generating the name of the file that holds the look-up table.

This function must ensure when it generates a file name for the look-up table that
all look-up tabels that have the same file name will have the same contents.

For all tiles the MipLevel, class, if it is pentagonal or haxagonal, and which 
hemisphere are used to identify the file name.

For major tiles, the general case is enough to identify them.

For minor tiles, the naming depends on the relative direction of each contributing tile, and 
the relative rotation of these tiles with respect to the main tile, and if the contributing
tile is a hexagon or a pentagon. And in the case that the neighbour is a pentagon, the hemisphere
of the pentagon is important.

*/
boost::filesystem::path STile::getLUTFilename(int knMipLevel, 
											  PYXIcosIndex indices[4],
											  int indexDirections[4])
{
	boost::filesystem::path fname = AppServices::getCacheDir("lut");

	// put in the common naming traits.
	std::string name = StringUtils::toString(knMipLevel) + "_" + StringUtils::toString(m_index.getClass());
	if (m_index.isNorthern())
	{
		name += "N";
	}
	else
	{
		name += "S";
	}
	if (isHexagon())
	{
		name += "H";
	}
	else
	{
		name += "P";
	}

	// 
	if (m_index.isMajor())
	{
		name += "maj";

	}
	else
	{
		name += "min";

		// naming here will be three pairs of digits made up of direction of 
		// contributing tile, and relative rotation of contributing tile, and
		// the hex or pentagon of the contributing tile. And in the case that 
		// the neighbour is a pentagon, the hemisphere of the pentagon is added.

		// first look to see if all indicies are in the same tile and on a face (hex):
		bool allInOneTile = 
			(indices[0].getPrimaryResolution() == indices[1].getPrimaryResolution()) &&
			(indices[0].getPrimaryResolution() == indices[2].getPrimaryResolution()) &&
			(indices[0].getPrimaryResolution() == indices[3].getPrimaryResolution());
		if (allInOneTile && indices[0].isFace())
		{
			for (int index = 1; index <= 3; index++)
			{
				name += StringUtils::toString(indexDirections[index]) + "0";
				if (indices[index].isHexagon())
				{
					name += "H";
				}
				else
				{
					name += "P";
				}
			}
		}
		else
		{
			for (int index = 1; index <= 3; index++)
			{
				name += StringUtils::toString(indexDirections[index]);
				PYXIcosIndex movedIndex(indices[0]);
				int rotation = 0;
				PYXIcosMath::move(&movedIndex, (PYXMath::eHexDirection) indexDirections[index],
					&rotation);
				name += StringUtils::toString(rotation);
				if (indices[index].isHexagon())
				{
					name += "H";
				}
				else
				{
					name += "P";
					if (indices[index].isNorthern())
					{
						name += "N";
					}
					else
					{
						name += "S";
					}
				}
			}
		}
	}

	name += ".lut";

	fname /= name;
	return fname;
}

// will fill one mip level
void STile::getImageData(boost::intrusive_ptr<ICoverage> spCov)
{
	// We're going to be filling this mip level. For now, skip all but 2 and 5.
	const int knMipLevel =
		(m_nMipLevels == 0 && View::lodEnabled())
			? 2
			: (knMipLevels - 1);

	getData(spCov, m_imageData, m_index.getResolution() + m_mipResIncr[knMipLevel], 0);
}

void STile::generateLUT(const TileDataSet &data, unsigned char* nTileNumber, unsigned int* nTileOffset, int nMipLevel)
{
	const int nSize = m_mipPO3[nMipLevel] + 1;

	// clear the buffers so that any unused buffer at the end will compress well.
	memset(nTileNumber, 0, knTBufSize/3);
	memset(nTileOffset, 0, knTBufSize/3 * sizeof(unsigned int));

	// if no disk cache of tables, generate the tables.
	int nTableIndex = 0;

	// Set up cursors in v dir.
	PYXCursor cv[3];
	cv[0].reset(m_index, static_cast<PYXMath::eHexDirection>(m_index.isNorthern() ? 2 : 5));
	{
		int nResIncr = m_mipResIncr[nMipLevel];
		while (nResIncr--)
		{
			cv[0].zoomIn();
		}
	}
	cv[1] = cv[0];
	cv[1].left();
	cv[1].left();
	cv[2] = cv[1];
	cv[2].left();
	cv[2].left();

	for (int v = 0; v != nSize; ++v)
	{
		// Set up cursors in u dir.
		PYXCursor cu[3] = { cv[0], cv[1], cv[2] };
		cu[0].right();
		if (isHexagon())
		{
			cu[0].right();
		}
		cu[1].right();
		cu[1].right();
		cu[2].right();
		cu[2].right();

		// Number of cells on this row if a pentagon.
		const int nPentCount = nSize - v;

		for (int u = 0; u != nSize; ++u)
		{
			if (isHexagon() || u < nPentCount)
			{
				whereValue(m_indices.m_indices, m_indices.m_nIndexCount, cu[0].getIndex(), &nTileNumber[nTableIndex], &nTileOffset[nTableIndex]);
				nTableIndex++;
				cu[0].forward();
			}

			whereValue(m_indices.m_indices, m_indices.m_nIndexCount, cu[1].getIndex(), &nTileNumber[nTableIndex], &nTileOffset[nTableIndex]);
			nTableIndex++;
			cu[1].forward();

			whereValue(m_indices.m_indices, m_indices.m_nIndexCount, cu[2].getIndex(), &nTileNumber[nTableIndex], &nTileOffset[nTableIndex]);
			nTableIndex++;
			cu[2].forward();
		}

		cv[0].forward();
		cv[1].forward();
		cv[2].forward();
	}
}

void STile::fillImageData()
{
	const int knMipLevel =
		(m_nMipLevels == 0 && View::lodEnabled())
			? 2
			: (knMipLevels - 1);

	PYXValue val((uint8_t*)0, 3);
	unsigned char* pv = val.getUInt8Ptr(0);

	unsigned char* p[3] =
	{
		m_tbuf + m_mipTriOffset[knMipLevel][0],
		m_tbuf + m_mipTriOffset[knMipLevel][1],
		m_tbuf + m_mipTriOffset[knMipLevel][2]
	};

	// Start with just solid colour for nodata cells
	memset(p[0], 0xe0, 3 * m_mipTriLength[knMipLevel]);

	if (m_imageData.m_nTileCount != 0)
	{
		// the look up tables
		boost::scoped_array<unsigned char> nTileNumber(new unsigned char[knTBufSize/3]);
		boost::scoped_array<unsigned int> nTileOffset(new unsigned int[knTBufSize/3]);
		// These were defined using the following syntax, but having local
		// variables this large introduced problems.  Specifically this
		// introduced the "Reentrant call in managed code" bug when doing a TRACE.
		//unsigned char* nTileNumber = new unsigned char[knTBufSize/3];
		//unsigned int* nTileOffset = new unsigned int[knTBufSize/3];

		const int knSize = m_mipPO3[knMipLevel] + 1;

		// find out if we have the look up table on disk
		boost::filesystem::path fname = getLUTFilename(knMipLevel, m_indices.m_indices, m_indices.m_indexDirections);
		if (!loadLUT(fname, nTileNumber.get(), nTileOffset.get()))
		{
			generateLUT(m_imageData, nTileNumber.get(), nTileOffset.get(), knMipLevel);
			saveLUT(fname, nTileNumber.get(), nTileOffset.get());
		}

		// Copy the data from the Value tiles to the sTile
		int nTableIndex = 0;
		for (int v = 0; v != knSize; ++v)
		{
			// Number of cells on this row if a pentagon.
			const int nPentCount = knSize - v;

			for (int u = 0; u != knSize; ++u)
			{
				if (isHexagon() || u < nPentCount)
				{
					if (getValue(m_imageData.m_tiles, nTileNumber.get()[nTableIndex], nTileOffset.get()[nTableIndex], 0, val))
					{
						memcpy(p[0], pv, 3);
					}
					nTableIndex++;
					p[0] += 3;
				}
				else if (u == nPentCount)
				{
					// Pentagon diagonal edge blending.
					// We can't get this perfect but we can get it pretty damn close.
					unsigned char* pu = p[0] - 3;                // prev u
					unsigned char* pv = p[0] - 3*knSize;         // prev v
					unsigned char* puv = pv - 3;                 // prev u and v
					int q;
					q = (*pu++ + *pv++) - *puv++;                // cancel
					if (q < 0) q = 0; else if (255 < q) q = 255; // clamp
					*p[0]++ = q;
					q = (*pu++ + *pv++) - *puv++;                // cancel
					if (q < 0) q = 0; else if (255 < q) q = 255; // clamp
					*p[0]++ = q;
					q = (*pu++ + *pv++) - *puv++;                // cancel
					if (q < 0) q = 0; else if (255 < q) q = 255; // clamp
					*p[0]++ = q;
				}
				else
				{
					p[0] += 3;
				}

				if (getValue(m_imageData.m_tiles,  nTileNumber.get()[nTableIndex], nTileOffset.get()[nTableIndex], 0, val))
				{
					memcpy(p[1], pv, 3);
				}
				nTableIndex++;
				p[1] += 3;

				if (getValue(m_imageData.m_tiles,  nTileNumber.get()[nTableIndex], nTileOffset.get()[nTableIndex], 0, val))
				{
					memcpy(p[2], pv, 3);
				}
				nTableIndex++;
				p[2] += 3;
			}
		}
	}

	//TRACE_INFO("FILLED " << m_index.toString() << " " << knMipLevel);

	m_nMipLevels = knMipLevel + 1;
	setRasterFilled(true);
	setRasterReloading(false);
	setRasterNeedsWritingToGL(true);
	if (m_imageData.hasNullTile(m_indices.m_nIndexCount))
	{
		setHadNullDataTile(true);
	}
	else
	{
		setHadNullDataTile(false);
		setNeedsWritingToDisk(true);
	}
	m_imageData.clear();
}

bool STile::fillVector(boost::intrusive_ptr<ICoverage> spCov, unsigned char* buf, unsigned int rgba)
{
	assert(spCov);
	bool bRet = false;

	TileDataSet vectorData;
	getData(spCov, vectorData, m_index.getResolution() + 11, 0);

	boost::intrusive_ptr<PYXTableDefinition const> spCoverageDefinition =
		spCov->getCoverageDefinition(); // Can throw.
	PYXValue val = spCoverageDefinition->getFieldDefinition(0).getTypeCompatibleValue();

	unsigned char* p[3] = { &buf[0*244*244*4], &buf[1*244*244*4], &buf[2*244*244*4] };

	// Start with RGBA(0, 0, 0, 0)
	memset(buf, 0, 3*244*244*4);

	if (vectorData.m_nTileCount == 0)
	{
		return true;
	}

	const int vectorMipLevel = 5;

	// the look up tables
	boost::scoped_array<unsigned char> nTileNumber(new unsigned char[knTBufSize/3]);
	boost::scoped_array<unsigned int> nTileOffset(new unsigned int[knTBufSize/3]);
	// These were defined using the following syntax, but having local
	// variables this large introduced problems.  Specifically this
	// introduced the "Reentrant call in managed code" bug when doing a TRACE.
	//unsigned char* nTileNumber = new unsigned char[knTBufSize/3];
	//unsigned int* nTileOffset = new unsigned int[knTBufSize/3];

	const int knSize = m_mipPO3[vectorMipLevel] + 1;

	// find out if we have the look up table on disk
	boost::filesystem::path fname = getLUTFilename(vectorMipLevel, m_indices.m_indices, m_indices.m_indexDirections);
	if (!loadLUT(fname, nTileNumber.get(), nTileOffset.get()))
	{
		generateLUT(vectorData, nTileNumber.get(), nTileOffset.get(), vectorMipLevel);
		saveLUT(fname, nTileNumber.get(), nTileOffset.get());
	}

	// For any cell that has data fill in the RGBA to the sTile
	int nTableIndex = 0;
	for (int v = 0; v != knSize; ++v)
	{
		// Number of cells on this row if a pentagon.
		const int nPentCount = knSize - v;

		for (int u = 0; u != knSize; ++u)
		{
			if (isHexagon() || u < nPentCount)
			{
				if (getValue(vectorData.m_tiles, nTileNumber.get()[nTableIndex], nTileOffset.get()[nTableIndex], 0, val))
				{
					memcpy(p[0], &rgba, 4);
					bRet = true;
				}
				nTableIndex++;
			}
			p[0] += 4;

			if (getValue(vectorData.m_tiles,  nTileNumber.get()[nTableIndex], nTileOffset.get()[nTableIndex], 0, val))
			{
				memcpy(p[1], &rgba, 4);
				bRet = true;
			}
			nTableIndex++;
			p[1] += 4;

			if (getValue(vectorData.m_tiles,  nTileNumber.get()[nTableIndex], nTileOffset.get()[nTableIndex], 0, val))
			{
				memcpy(p[2], &rgba, 4);
				bRet = true;
			}
			nTableIndex++;
			p[2] += 4;
		}
	}

	//setVectorNeedsWritingToGL(true);
	return bRet;
}

bool STile::fillValueTilesFromVTreeNode(const PYXIcosIndex* roots, PYXPointer<PYXValueTile>* tiles, int nTileCount, VectorPYXTree* pVTree, VectorPYXTree* pNode, const PYXIcosIndex& nodeIndex)
{
	bool bRet = false;

	PYXValue trueValue(true);

	for (std::vector<VectorFeat>::const_iterator itFeat = pNode->data().begin();
		itFeat != pNode->data().end(); ++itFeat)
	{
		for (std::vector<VectorPart>::const_iterator itPart = itFeat->m_parts.begin();
			itPart != itFeat->m_parts.end(); ++itPart)
		{
			PYXCurve curve;

			if (itPart->m_link[1].m_nDir)
			{
				// Find previous node with vertexes and add last one to curve.
				PYXIcosIndex index(nodeIndex);
				int nDir = itPart->m_link[1].m_nDir;
				int nPartID = itPart->m_link[1].m_nPartID;
				VectorPYXTree* pFindNode = pNode;
				VectorPart* pFindPart;
				do
				{
					PYXCursor c(index, static_cast<PYXMath::eHexDirection>(nDir));
					c.forward();
					index = c.getIndex();
					pFindNode = queryPTree(pVTree, indexToAddr(index));
					VectorFeat* pFindFeat = &vdataInsertFeat(pFindNode->data(), itFeat->m_nFeatID);
					pFindPart = &pFindFeat->m_parts[nPartID];
					nDir = pFindPart->m_link[1].m_nDir;
					nPartID = pFindPart->m_link[1].m_nPartID;
				} while (pFindPart->m_verts.empty());
				VertCode nVertCode = pFindPart->m_verts.back();
				int nVertDir = vertCodeGetDir(nVertCode);
				int nVertOffset = vertCodeGetOffset(nVertCode);
				if (nVertDir)
				{
					PYXIcosMath::move(&index, static_cast<PYXMath::eHexDirection>(nVertDir));
				}
				PYXIcosIndex vertIndex(PYXIcosMath::calcIndexFromOffset(index, index.getResolution() + 11, nVertOffset));
				vertIndex.setResolution(m_index.getResolution() + 11);
				curve.addNode(vertIndex);
			}

			// Current node vertexes.
			PYXIcosIndex moveIndex;
			for (std::vector<VertCode>::const_iterator itVertCode = itPart->m_verts.begin();
				itVertCode != itPart->m_verts.end(); ++itVertCode)
			{
				const PYXIcosIndex* pIndex = &nodeIndex;
				VertCode nVertCode = *itVertCode;
				int nVertDir = vertCodeGetDir(nVertCode);
				int nVertOffset = vertCodeGetOffset(nVertCode);
				if (nVertDir)
				{
					moveIndex = nodeIndex;
					PYXIcosMath::move(&moveIndex, static_cast<PYXMath::eHexDirection>(nVertDir));
					pIndex = &moveIndex;
				}
				PYXIcosIndex vertIndex(PYXIcosMath::calcIndexFromOffset(*pIndex, pIndex->getResolution() + 11, nVertOffset));
				vertIndex.setResolution(m_index.getResolution() + 11);
				curve.addNode(vertIndex);
			}

			if (itPart->m_link[0].m_nDir)
			{
				// Find next node with vertexes and add first one to curve.
				PYXIcosIndex index(nodeIndex);
				int nDir = itPart->m_link[0].m_nDir;
				int nPartID = itPart->m_link[0].m_nPartID;
				VectorPYXTree* pFindNode = pNode;
				VectorPart* pFindPart;
				do
				{
					PYXCursor c(index, static_cast<PYXMath::eHexDirection>(nDir));
					c.forward();
					index = c.getIndex();
					pFindNode = queryPTree(pVTree, indexToAddr(index));
					VectorFeat* pFindFeat = &vdataInsertFeat(pFindNode->data(), itFeat->m_nFeatID);
					pFindPart = &pFindFeat->m_parts[nPartID];
					nDir = pFindPart->m_link[0].m_nDir;
					nPartID = pFindPart->m_link[0].m_nPartID;
				} while (pFindPart->m_verts.empty());
				VertCode nVertCode = pFindPart->m_verts.front();
				int nVertDir = vertCodeGetDir(nVertCode);
				int nVertOffset = vertCodeGetOffset(nVertCode);
				if (nVertDir)
				{
					PYXIcosMath::move(&index, static_cast<PYXMath::eHexDirection>(nVertDir));
				}
				PYXIcosIndex vertIndex(PYXIcosMath::calcIndexFromOffset(index, index.getResolution() + 11, nVertOffset));
				vertIndex.setResolution(m_index.getResolution() + 11);
				curve.addNode(vertIndex);
			}

#define USE_NEW_CODE 0
#if USE_NEW_CODE
			// Go over curve and add its values to memory buffers.
			for (PYXPointer<PYXIterator> spIt = curve.getIterator();
				!spIt->end(); spIt->next())
			{
				int t, u, v;
				tuvFromIndex(m_index, spIt->getIndex(), t, u, v);
				// TODO skip some TUVs until they are handled
				if (0 <= u && u < 244 && 0 <= v && v < 244)
				{
					unsigned char* p = buf + t * 244 * 244 * 4;
					p += v * 244 * 4;
					p += u * 4;
					p[0] = 255;
					p[3] = 255;
				}
			}
#else
			// Go over curve and add its values to value tiles.
			for (PYXPointer<PYXIterator> spIt = curve.getIterator();
				!spIt->end(); spIt->next())
			{
				for (int n = 0; n != nTileCount; ++n)
				{
					if (roots[n].isAncestorOf(spIt->getIndex()))
					{
						if (!tiles[n])
						{
							std::vector<PYXValue::eType> vecType(1, PYXValue::knBool);
							tiles[n] = PYXValueTile::create(PYXTile(roots[n], m_index.getResolution() + 11), vecType);
						}
						tiles[n]->setValue(spIt->getIndex(), 0, trueValue);
						bRet = true;
						break;
					}
				}
			}
#endif
		}
	}

	return bRet;
}

bool STile::fillVector(boost::shared_ptr<VectorPYXTree> spVTree, unsigned char* buf, unsigned int rgba)
{
	assert(spVTree);
	bool bRet = false;

	// TODO This whole chunk of code should go into a utility function when we also do this for elevation
	// Get some data per tile first. We'll then use cursors to pull data from these tiles.
	PYXIcosIndex roots[4];
	PYXPointer<PYXValueTile> tiles[4];
	int nTileCount = 0;
	roots[nTileCount++] = m_index;
	if (m_index.isMinor())
	{
		// We're a minor tile so we have 3 major neighbours.
		std::vector<PYXIcosIndex> vec;
		PYXIcosMath::getNeighbours(m_index, &vec);
		for (int n = 0; n != 6; ++n)
		{
			if (!vec[n].isNull() && vec[n].isMajor())
			{
				roots[nTileCount++] = PYXIcosMath::move(m_index, static_cast<PYXMath::eHexDirection>(n + 1));
			}
		}
	}

	// Fill value tiles from vtree
	for (int n = 0; n != nTileCount; ++n)
	{
		Addr desiredAddr = indexToAddr(roots[n]);
		Addr nodeAddr;
		VectorPYXTree* pNode = descendPTree(spVTree.get(), desiredAddr, &nodeAddr);
		if (nodeAddr[0] != 0xf && nodeAddr[1] != 0xf && nodeAddr[2] != 0xf)
		{
			bRet |= fillValueTilesFromVTreeNode(roots, tiles, nTileCount, spVTree.get(), pNode, addrToIndex(nodeAddr));
		}
		if (n == 0 && nodeAddr == desiredAddr)
		{
			// No need to do overlapping tiles if we got exactly what we want.
			break;
		}
	}

	if (!bRet)
	{
		return false;
	}

	// Cull unused tiles.
	for (int n = 0; n != nTileCount;)
	{
		if (!tiles[n])
		{
			for (int n2 = n + 1; n2 != nTileCount; ++n2)
			{
				tiles[n2 - 1] = tiles[n2];
			}
			--nTileCount;
		}
		else
		{
			++n;
		}
	}

	PYXValue val(false);

	// Start with RGBA(0, 0, 0, 0)
	memset(buf, 0, 3*244*244*4);
	unsigned char* p[3] = { &buf[0*244*244*4], &buf[1*244*244*4], &buf[2*244*244*4] };

	// Set up cursors in v dir.
	PYXCursor cv[3];
	cv[0].reset(m_index, static_cast<PYXMath::eHexDirection>(m_index.isNorthern() ? 2 : 5));
	cv[0].zoomIn();
	cv[0].zoomIn();
	cv[0].zoomIn();
	cv[0].zoomIn();
	cv[0].zoomIn();
	cv[0].zoomIn();
	cv[0].zoomIn();
	cv[0].zoomIn();
	cv[0].zoomIn();
	cv[0].zoomIn();
	cv[0].zoomIn();
	cv[1] = cv[0];
	cv[1].left();
	cv[1].left();
	cv[2] = cv[1];
	cv[2].left();
	cv[2].left();

	for (int v = 0; v != knTexHeight; ++v)
	{
		// Set up cursors in u dir.
		PYXCursor cu[3] = { cv[0], cv[1], cv[2] };
		cu[0].right();
		if (isHexagon())
		{
			cu[0].right();
		}
		cu[1].right();
		cu[1].right();
		cu[2].right();
		cu[2].right();

		const int nPentCount = knTexWidth - v;

		for (int u = 0; u != knTexWidth; ++u)
		{
			if (isHexagon() || u < nPentCount)
			{
				if (getValue(tiles, nTileCount, cu[0].getIndex(), 0, val))
				{
					*reinterpret_cast<unsigned int*>(p[0]) = rgba;
					p[0] += 4;
					bRet = true;
				}
				else
				{
					p[0] += 4;
				}
				cu[0].forward();
			}
			else
			{
				p[0] += 4;
			}
			if (getValue(tiles, nTileCount, cu[1].getIndex(), 0, val))
			{
				*reinterpret_cast<unsigned int*>(p[1]) = rgba;
				p[1] += 4;
				bRet = true;
			}
			else
			{
				p[1] += 4;
			}
			cu[1].forward();
			if (getValue(tiles, nTileCount, cu[2].getIndex(), 0, val))
			{
				*reinterpret_cast<unsigned int*>(p[2]) = rgba;
				p[2] += 4;
				bRet = true;
			}
			else
			{
				p[2] += 4;
			}
			cu[2].forward();
		}

		cv[0].forward();
		cv[1].forward();
		cv[2].forward();
	}

	return true;
}

/*!
Writes an MTF file. Doesn't check if it exists beforehand.
\return True if successful, false otherwise.
*/
bool STile::writeToFile(const std::string& strProcessIdentity)
{
	if (strProcessIdentity.length() == 0)
	{
		return false;
	}

	int nVersion = View::lodEnabled() ? 5 : 4;

	boost::filesystem::path fname = getCacheVis().getPath(strProcessIdentity, true);
	if (fname.empty())
	{
		return false;
	}
	fname /= getCacheFileName(m_index);

	// create a memory stream for the tile so we can compress it before it goes on disk.
	std::ostringstream memOut;
	
	if (nVersion == 4)
	{
		memOut.write(reinterpret_cast<char*>(m_tbuf + m_mipTriOffset[5][0]), 3 * m_mipTriLength[5]);
	}
	else if (nVersion == 5)
	{
		memOut.write(reinterpret_cast<char*>(m_tbuf + m_mipTriOffset[2][0]), 3 * m_mipTriLength[2]);
		memOut.write(reinterpret_cast<char*>(m_tbuf + m_mipTriOffset[5][0]), 3 * m_mipTriLength[5]);
	}

	unsigned long nUnCompressedLength = static_cast<unsigned long>(memOut.str().length());
	unsigned long nCompressedBufferLength = compressBound(nUnCompressedLength);
	boost::scoped_array<char> pCompressedDataBuffer(new char[nCompressedBufferLength]);
	int returnCode = compress((Bytef *)pCompressedDataBuffer.get(), &nCompressedBufferLength,
                                (const Bytef *)memOut.str().c_str(), nUnCompressedLength);

	// Check to see if the compress was successful.
	if (returnCode != Z_OK)
	{
		PYXTHROW(PYXException, "Compression failed while saving an mtf file.");
	}

	// create the output file
	std::ofstream file(FileUtils::pathToString(fname).c_str(), std::ios::binary);

	// Write magic number.
	file.write("mtf", 4);

	// Write version.
	file.write(reinterpret_cast<const char*>(&nVersion), 4);

	// TODO:  this needs to be the reload flag...
	// write the reload flag
	bool bReload = false;
	file.write(reinterpret_cast<const char*>(&bReload), sizeof(bool));

	// write the compresed data
	file.write((char*)&nUnCompressedLength,sizeof(unsigned long));
	file.write((char*)&nCompressedBufferLength,sizeof(unsigned long));
	file.write(pCompressedDataBuffer.get(), nCompressedBufferLength);
	file.close();
	setNeedsWritingToDisk(false);
	return true;
}

/*!
Reads an MTF file.
\return True if successful, false otherwise.
*/
bool STile::readMTFFile(const std::string& strProcessIdentity)
{
	assert(!strProcessIdentity.empty());

	boost::filesystem::path fname = getCacheVis().getPath(strProcessIdentity, false);
	if (fname.empty())
	{
		return false;
	}
	fname /= getCacheFileName(m_index);

	if (!FileUtils::exists(fname))
	{
		return false;
	}

	std::ifstream file(FileUtils::pathToString(fname).c_str(), std::ios::binary);

	const char* const magic = "mtf"; // note this is 4 bytes not 3!
	char buf[8] = { 0 }; // init buffer to zero

	file.read(buf, 8);
	if (memcmp(magic, buf, 4))
	{
		// has no magic number, predates versioning, delete the file
		file.close();
		boost::filesystem::remove(fname);
		return false;
	}

	// versions are not up converted, old version are thrown away.
	int& nVersion = *reinterpret_cast<int*>(buf + 4);
	if ((nVersion == 4 && !View::lodEnabled()) || (nVersion == 5))
	{
		// read the reload flag -- means that the data is old and out of date, but might as well show
		// this old data until we get new data.
		bool bReload;
		file.read((char*)&bReload, sizeof(bool));

		// TODO:  this will turn into the Reload needed flag.
		// do nothing with it now.

		// read the data
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

		if (nVersion == 4)
		{
			memIn.read(reinterpret_cast<char*>(m_tbuf + m_mipTriOffset[5][0]), 3 * m_mipTriLength[5]);
		}
		else
		{
			memIn.read(reinterpret_cast<char*>(m_tbuf + m_mipTriOffset[2][0]), 3 * m_mipTriLength[2]);
			memIn.read(reinterpret_cast<char*>(m_tbuf + m_mipTriOffset[5][0]), 3 * m_mipTriLength[5]);
		}
		bool bReadOK = memIn.good();
		if (bReadOK)
		{
			m_nMipLevels = 6;
			setRasterFilled(true);
			setRasterNeedsWritingToGL(true);
			setNeedsWritingToDisk(false);
		}
		return bReadOK;
	}
	else
	{
		TRACE_INFO("unsupported MTF file version " << nVersion << " for current settings. Removing file: " << FileUtils::pathToString(fname));
		file.close();
		boost::filesystem::remove(fname);
		return false;
	}

	return true;
}

bool STile::intersects(const Ray & ray,double & resultTime)
{
	boost::recursive_mutex::scoped_lock lock(m_elevationDataMutex);

	return m_mesh->intersects(ray,resultTime);
}




WeakPointerSTile::WeakPointerSTile(const ProcRef & procRef,const PYXIcosIndex & index) : m_procRef(procRef), m_index(index)
{
}

PYXPointer<WeakPointerSTile> WeakPointerSTile::create(const ProcRef & procRef,const PYXIcosIndex & index)
{
	return PYXNEW(WeakPointerSTile,procRef,index);
}

PYXPointer<STile> WeakPointerSTile::get()
{
	return STile::getIfExists(m_procRef,m_index);	
}

