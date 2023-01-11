#pragma once
#ifndef VIEW_MODEL__TILES_CPU_RENDERER_H
#define VIEW_MODEL__TILES_CPU_RENDERER_H
/******************************************************************************
tiles_cpu_renderer.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "component.h"
#include "open_gl_texture.h"
#include "open_gl_resource_pool.h"
#include "stile.h"

#include "pyxis/derm/index.h"
#include "pyxis/pipe/process.h"

#include <boost/shared_array.hpp>

#include <map>

class STileTextures
{
public:
	STileTextures() : m_rasterTextures(3,OpenGLTexture::knTexture2D,OpenGLTexture::knTextureRGB),
					  m_vectorTextures(3,OpenGLTexture::knTexture2D,OpenGLTexture::knTextureRGBA),
					  m_vectorTextureComposing(false)
	{ };

public:
	OpenGLTextureArray m_rasterTextures;
	OpenGLTextureArray m_vectorTextures;	

	bool m_vectorTextureComposing;
};

typedef std::map<PYXIcosIndex, STileTextures> STilesTexturesMap;

/*!

TilesCPURenderer - render tiles on the screen - using raster, elevation and vector data

-- Description:
     - This renderer is strongly depend on View visible tiles implemetation and on STile structure.
     - Uses the original rendering code of the View class

-- OpenGL extentions:
	 - Multitexture

-- Limitations: 
     - None
	 
*/
//! TilesCPURenderer - render tiles on the screen - using raster, elevation and vector data
class TilesCPURenderer : public Component
{
public:
	TilesCPURenderer(ViewOpenGLThread & viewThread);
	static PYXPointer<TilesCPURenderer> create(ViewOpenGLThread & viewThread) { return PYXNEW(TilesCPURenderer,viewThread); }

	virtual ~TilesCPURenderer(void);

	virtual bool initialize();
	virtual void releaseOpenGLResources();
	virtual void render(); 
	
	//! schedule a post processing if needed
	void setNeedPostProcessing();	

private:
	bool m_postProcessingScheduled;
	void postProcessingTiles();

	//! make sure we are using the right textures
	void validateTilesTextureMap();

	static const int knTextureLowerLimit = (knMaxSTileCacheSize * 6);
	static const int knTextureUpperLimit = knTextureLowerLimit + 25;

	//! limit the active textures
	void limitActiveTextures(int upperLimit,int lowerLimit);


	OpenGLTexture m_checkerTxture;
	OpenGLTexture m_checkerPentagonTxture;

	//! the STile texture map
	STilesTexturesMap  m_tilesTextures;

	//! the current ProcRef that we are rendering
	ProcRef            m_currentProcRef;

	//! textures resource pool - to manage active textures
	OpenGLResourcePool m_texturePool;

	//! background worker thread for vector texture compositing
	Sync::WorkerThreadContext m_workerThread;

	//! called in background worker thread
	void compositeVectorTexture(boost::intrusive_ptr<IProcess> spViewPointProcess, PYXPointer<STile> spTile);
	void createVectorTextures(boost::intrusive_ptr<IProcess> spViewPointProcess, PYXPointer<STile> spTile, unsigned char * buf);

protected:
	bool m_showEarth;
	bool m_showVectors;

public:
	const bool & isShowingRaster() const { return m_showEarth; };
	const bool & isShowingVectors() const { return m_showVectors; };

	void setShowingRaster(const bool & visible) { m_showEarth = visible; setVisible(m_showEarth || m_showVectors); }
	void setShowingVectors(const bool & visible) { m_showVectors = visible; setVisible(m_showEarth || m_showVectors); }
};


#endif