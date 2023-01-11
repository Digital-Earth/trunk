/******************************************************************************
tiles_cpu_renderer.cpp

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "tiles_cpu_renderer.h"
#include "performance_counter.h"

#ifdef false

// view model includes
#include "view_open_gl_thread.h"
#include "gl_utils.h"
#include "exceptions.h"
#include "fill_utils.h"
#include "stile.h"
#include "vector_utils.h"

// pyxlib includes
#include "pyxis/procs/viewpoint.h"

#include <cassert>
#include <map>
#include <vector>

TilesCPURenderer::TilesCPURenderer(ViewOpenGLThread & viewThread) : Component(viewThread), 
	m_checkerTxture(OpenGLTexture::knTexture2D,OpenGLTexture::knTextureRGB), 
	m_checkerPentagonTxture(OpenGLTexture::knTexture2D,OpenGLTexture::knTextureRGB),
	m_postProcessingScheduled(false),
	m_showEarth(true),
	m_showVectors(true)
{
}

TilesCPURenderer::~TilesCPURenderer(void)
{
}

bool TilesCPURenderer::initialize() 
{

	m_checkerTxture.setMinFilter(OpenGLTexture::knTextureMinLinear);
	m_checkerTxture.setSize(256,256,OpenGLTexture::knDataRGB,OpenGLTexture::knTextelUnsignedByte);
	m_checkerTxture.setDataRegion(0,0,STile::knTexWidth, STile::knTexHeight,OpenGLTexture::knDataRGB,STile::getCheckerTexels());

	m_checkerPentagonTxture.setMinFilter(OpenGLTexture::knTextureMinLinear);
	m_checkerPentagonTxture.setSize(256,256,OpenGLTexture::knDataRGB,OpenGLTexture::knTextelUnsignedByte);
	m_checkerPentagonTxture.setDataRegion(0,0,STile::knTexWidth, STile::knTexHeight,OpenGLTexture::knDataRGB,STile::getCheckerTexelsPent());

	return m_checkerTxture.isValid() && m_checkerPentagonTxture.isValid();
}

void TilesCPURenderer::releaseOpenGLResources()
{
	//clear the textures vector - we don't use them any more...
	m_tilesTextures.clear();
}


void TilesCPURenderer::setNeedPostProcessing()
{
	if (!m_postProcessingScheduled )
	{
		schedulePostProcessing(&TilesCPURenderer::postProcessingTiles);
		m_postProcessingScheduled = true;
	}
}

void TilesCPURenderer::validateTilesTextureMap()
{
	ProcRef procref(getViewThread().getViewPointProcess());

	if (procref != m_currentProcRef)
	{
		m_currentProcRef = procref;

		//clear the textures verctor - we don't use them any more...
		m_tilesTextures.clear();
	}
}

void TilesCPURenderer::limitActiveTextures(int upperLimit,int lowerLimit)
{
	int validCount = m_texturePool.getValidCount();
	//if we passed the upperLimit
	if (validCount > upperLimit)
	{
		//try to rleases unused resource...
		m_texturePool.forceRelease(validCount-lowerLimit);			
	}
}

void TilesCPURenderer::render()
{	
	getViewThread().setState("render STiles");
	//make sure our map is validate.
	validateTilesTextureMap();

	glEnableClientState(GL_VERTEX_ARRAY);
	if (m_showEarth)
	{
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	if (m_showVectors)
	{
		glClientActiveTexture(GL_TEXTURE1);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glClientActiveTexture(GL_TEXTURE0);
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 5);

	getViewThread().applyCamera();
	double altitudeFactor = 1 + getViewThread().getCamera().getOrbitalAltitude()/SphereMath::knEarthRadius;
	
	// Use this to control local coordinate transformations
	glPushMatrix();
	vec3 localOrigin(0, 0, 0); // default local origin
	glTranslated(-0, -0, -altitudeFactor ); // default base origin

	// Use this matrix to rotate local origin
	mat4 m;
	cml::matrix_rotation_quaternion(m, getViewThread().getCamera().getOrbitalRotation());
	glMultMatrixd(m.data());

	glColor3d(1, 1, 1);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	bool bTex1Enabled = false;

	for (ViewOpenGLThread::VisibleTilesVector::iterator it =  getViewThread().getVisibleSTiles().begin(); it != getViewThread().getVisibleSTiles().end(); ++it)
	{
		PYXPointer<STile> pSTile = *it;

		STileTextures & textures = m_tilesTextures[pSTile->getRootIndex()];

		if (pSTile->getOrigin() != localOrigin)
		{
			// Local origin has changed so redo the transformations
			glPopMatrix();
			glPushMatrix();
			localOrigin = pSTile->getOrigin();
			vec3 v = localOrigin;
			v = cml::transform_vector(m, v);
			v -= vec3(0, 0, altitudeFactor );
			glTranslated(v[0], v[1], v[2]);
			glMultMatrixd(m.data());
		}

		if (m_showVectors && pSTile->isVectorFilled() && (pSTile->vectorNeedsWritingToGL() || !textures.m_vectorTextures.isAllValid()))
		{
			setNeedPostProcessing();
		}

		//! Note - we are using current textures. if pSTile->vectorNeedsWritingToGL() is true it would be updated on the next frame...
		if (m_showVectors && pSTile->isVectorFilled() && textures.m_vectorTextures.isAllValid())
		{
			if (!bTex1Enabled)
			{
				glActiveTexture(GL_TEXTURE1);
				glEnable(GL_TEXTURE_2D);
#if 1
				// basic decal
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
#else
				// psychedelic blend
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE1);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE0);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
				float rgba[4] = { (float)rand()/RAND_MAX, (float)rand()/RAND_MAX, (float)rand()/RAND_MAX, (float)rand()/RAND_MAX };
				glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, rgba);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);
#endif
				glActiveTexture(GL_TEXTURE0);
				bTex1Enabled = true;
			}
		}
		else
		{
			if (bTex1Enabled)
			{
				glActiveTexture(GL_TEXTURE1);
				glDisable(GL_TEXTURE_2D);
				glActiveTexture(GL_TEXTURE0);
				bTex1Enabled = false;
			}
		}

		const int knMipLevel = pSTile->getMipLevels() - 1;

		for (int nTri = 0; nTri != 3; ++nTri)
		{
			if (m_showEarth)
			{
				glTexCoordPointer(2, GL_FLOAT, 0, pSTile->getTexCoordPointer(knMipLevel, nTri));
			}

			if (m_showVectors)
			{
				glClientActiveTexture(GL_TEXTURE1);
				glTexCoordPointer(2, GL_FLOAT, 0, pSTile->getTexCoordPointer(knMipLevel, nTri));
				glClientActiveTexture(GL_TEXTURE0);
			}

			if (m_showEarth)
			{
				if ( pSTile->isRasterFilled() && (pSTile->rasterNeedsWritingToGL() || !textures.m_rasterTextures.isAllValid()) )				
				{
					setNeedPostProcessing();
				}

				//! Note - we are using current textures. if pSTile->rasterNeedsWritingToGL() is true it would be updated on the next frame...
				if ( pSTile->isRasterFilled() && textures.m_rasterTextures.isAllValid())				
				{

					textures.m_rasterTextures[nTri]->startUsing();					
				}
				else
				{
					// Draw checker pattern
					glTexCoordPointer(2, GL_FLOAT, 0, pSTile->getTexCoordPointer(STile::knMipLevels - 1, nTri));

					if (nTri || pSTile->isHexagon())
					{
						m_checkerTxture.startUsing();
					}
					else
					{
						m_checkerPentagonTxture.startUsing();
					}					
				}
			}

			if (m_showVectors && pSTile->isVectorFilled() && (! pSTile->vectorNeedsWritingToGL()) && textures.m_vectorTextures.isAllValid())			
			{
				glActiveTexture(GL_TEXTURE1);
				textures.m_vectorTextures[nTri]->startUsing();				
				glActiveTexture(GL_TEXTURE0);
			}

			// glVertex* methods should be called after glCoord* methods
			glVertexPointer(3, GL_FLOAT, 0, pSTile->getVertexPointer(nTri));
			const unsigned short* pInfo = pSTile->getTriStripInfo(nTri);
			const unsigned short* pIndices = pSTile->getTriStripIndices(nTri);
			const int nDrawCount = *pInfo++;

			// Draw the triangle strips.
			for (int nDraw = 0; nDraw != nDrawCount; ++nDraw)
			{
				const int nCount = *pInfo++;
				glDrawElements(GL_TRIANGLE_STRIP, nCount, GL_UNSIGNED_SHORT, pIndices);
				pIndices += nCount;
			}
		}
	}

	glPopMatrix();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_2D);
	glClientActiveTexture(GL_TEXTURE1);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTexture(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_POLYGON_OFFSET_FILL);

	if (bTex1Enabled)
	{
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
	}	
}


void TilesCPURenderer::postProcessingTiles()
{
	MEASURE_FUNC_CALL("postProcessingTiles");

	m_postProcessingScheduled = false;
	
	// wait until we have a ViewPointProcess
	if (getViewThread().getViewPointProcess().get() == NULL)
	{	
		return;
	}

	std::vector<boost::intrusive_ptr< IProcess > > vecProc;
	boost::intrusive_ptr<IViewPoint> spViewPoint = 
		boost::dynamic_pointer_cast<IViewPoint>(getViewThread().getViewPointProcess());
	vecProc = spViewPoint->getFeatureCollectionPipelines();

	//search for raster need to fill
	for (ViewOpenGLThread::VisibleTilesVector::iterator it =  getViewThread().getVisibleSTiles().begin(); it != getViewThread().getVisibleSTiles().end(); ++it)
	{
		PYXPointer<STile> pSTile = *it;		
		STileTextures & textures = m_tilesTextures[pSTile->getRootIndex()];

		if (m_showEarth)
		{
			if (pSTile->isRasterFilled() && (pSTile->rasterNeedsWritingToGL() || !textures.m_rasterTextures.isAllValid()))
			{
				const int knMipLevel = pSTile->getMipLevels() - 1;

				for (int nTri = 0; nTri != 3; ++nTri)
				{			
					const int knSize = STile::getMipPO3(knMipLevel) + 1;						
					if (! textures.m_rasterTextures[nTri]->isValid())
					{
						textures.m_rasterTextures[nTri]->setSize(256,256,OpenGLTexture::knDataRGB, OpenGLTexture::knTextelUnsignedByte);
						textures.m_rasterTextures[nTri]->setMinFilter(OpenGLTexture::knTextureMinLinear);
					}
					textures.m_rasterTextures[nTri]->setDataRegion(0, 0, knSize, knSize, OpenGLTexture::knDataRGB, OpenGLTexture::knTextelUnsignedByte, pSTile->getTexels(knMipLevel, nTri));
					textures.m_rasterTextures[nTri]->stopUsing();

					m_texturePool.attach(*textures.m_rasterTextures[nTri]);
				}
				
				pSTile->setRasterNeedsWritingToGL(false);

				setNeedPostProcessing();
				//done something - go back to render loop
				return;
			}			
		}
	}

	//search for vector need to fill
	for (ViewOpenGLThread::VisibleTilesVector::iterator it =  getViewThread().getVisibleSTiles().begin(); it != getViewThread().getVisibleSTiles().end(); ++it)
	{
		PYXPointer<STile> pSTile = *it;		
		
		STileTextures & textures = m_tilesTextures[pSTile->getRootIndex()];

		if (m_showVectors && pSTile->isVectorFilled() && (! textures.m_vectorTextures.isAllValid() || pSTile->vectorNeedsWritingToGL()))
		{				
			if (pSTile->isVectorComposited())
			{
				createVectorTextures(getViewThread().getViewPointProcess(), pSTile, pSTile->getVectorsTexels(0));
				
				//done something - go back to render loop
				setNeedPostProcessing();
				return;	
			}
			else if (!textures.m_vectorTextureComposing)
			{
				m_workerThread.normal.beginInvokeOn(*this, 
					&TilesCPURenderer::compositeVectorTexture, getViewThread().getViewPointProcess(), pSTile);
				textures.m_vectorTextureComposing = true;

				//done something - go back to render loop
				setNeedPostProcessing();
				return;		
			}			
		}		
	}

	//if we got here - no more texture to process - then free unsder textures if needed
	limitActiveTextures(knTextureUpperLimit,knTextureLowerLimit);
}


void TilesCPURenderer::compositeVectorTexture(boost::intrusive_ptr<IProcess> spViewPointProcess, PYXPointer<STile> spSTile)
{
	std::vector<boost::intrusive_ptr< IProcess > > vecProc;	
	boost::intrusive_ptr<IViewPoint> spViewPoint = 
		boost::dynamic_pointer_cast<IViewPoint>(spViewPointProcess);
	vecProc = spViewPoint->getFeatureCollectionPipelines();	

	// Here we composite the vector data and shove it into texture objects.				
	unsigned char * buf = const_cast<unsigned char *>(spSTile->getVectorsTexels(0));
	memset(&buf[0], 0, 3*STile::knTexWidth*STile::knTexHeight*4);
	std::vector<unsigned char*> vecBuf;
	for (std::vector<boost::intrusive_ptr<IProcess> >::iterator it = vecProc.begin();
		it != vecProc.end(); ++it)
	{
		unsigned char* vectorBuf = VectorRGBData::get(ProcRef(*it), spSTile->getRootIndex())->getRGB();
		if (vectorBuf)
		{
			vecBuf.push_back(vectorBuf);
		}
	}
	// Composite vecBuf into buf
	const int nVecCount = static_cast<int>(vecBuf.size());
	const int nCount = 3*STile::knTexWidth*STile::knTexHeight*4;
	for (int n = 0; n != nCount; n += 4)
	{
		float rgba[4] = { 0, 0, 0, 0 };
		for (int n2 = 0; n2 != nVecCount; ++n2)
		{
			unsigned char* src = vecBuf[n2];
			float srcalpha = src[n+3]/255.0f;
			float oneminussrcalpha = 1 - srcalpha;
			buf[n+0] = static_cast<unsigned char>(src[n+0] * srcalpha + buf[n+0] * oneminussrcalpha);
			buf[n+1] = static_cast<unsigned char>(src[n+1] * srcalpha + buf[n+1] * oneminussrcalpha);
			buf[n+2] = static_cast<unsigned char>(src[n+2] * srcalpha + buf[n+2] * oneminussrcalpha);
			buf[n+3] = static_cast<unsigned char>(src[n+3] * srcalpha + buf[n+3] * oneminussrcalpha);
		}
	}

	schedulePostProcessing(&TilesCPURenderer::createVectorTextures, spViewPointProcess, spSTile, buf);
}

void TilesCPURenderer::createVectorTextures(boost::intrusive_ptr<IProcess> spViewPointProcess, PYXPointer<STile> spSTile, unsigned char * buf)
{

	//check if we still need this STile...
	if (ProcRef(spViewPointProcess) != m_currentProcRef)
	{
		return;
	}

	STileTextures & textures = m_tilesTextures[spSTile->getRootIndex()];

	for (int nTri = 0; nTri != 3; ++nTri)
	{							
		if (! textures.m_vectorTextures[nTri]->isValid())
		{
			textures.m_vectorTextures[nTri]->setSize(256,256,OpenGLTexture::knDataRGB, OpenGLTexture::knTextelUnsignedByte);
			textures.m_vectorTextures[nTri]->setMinFilter(OpenGLTexture::knTextureMinLinear);
		}
		textures.m_vectorTextures[nTri]->setDataRegion(0, 0, STile::knTexWidth, STile::knTexHeight, OpenGLTexture::knDataRGBA, OpenGLTexture::knTextelUnsignedByte, &buf[nTri*STile::knTexWidth*STile::knTexHeight*4]);
		textures.m_vectorTextures[nTri]->stopUsing();

		m_texturePool.attach(*textures.m_vectorTextures[nTri]);
	}			


	//remove all generated vectors
	spSTile->m_vectorsGenerated.clear();
	spSTile->setVectorComposited(true);

	textures.m_vectorTextureComposing = false;
	spSTile->setVectorNeedsWritingToGL(false);
}

#endif