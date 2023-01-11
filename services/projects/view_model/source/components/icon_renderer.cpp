/******************************************************************************
icon_renderer.cpp

begin		: 2009-11-01
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "icon_gpu_renderer.h"

#include "view_open_gl_thread.h"
#include "gl_utils.h"
#include "performance_renderer.h"
#include "garbage_collector.h"

#include "pyxis/utility/bitmap_server_provider.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/geometry/circle_geometry.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/procs/viewpoint.h"

#include <boost/scoped_array.hpp>

#include <cassert>
#include <map>
#include <vector>



IconRenderer::IconRenderer(ViewOpenGLThread & viewThread) : 
	Component(viewThread), 	
	m_postProcessingScheduled(false),
	m_iconTextures(PYXNEW(TexturePacker))
{
	m_iconsMemento = IconsMemento::create(getViewThread().getSurface(),PatchIconsCreator::create());
	getViewThread().getViewPortProcessChangeNotifier().attach(this,&IconRenderer::newViewPointPorcess);
}

IconRenderer::~IconRenderer(void)
{
	getViewThread().getViewPortProcessChangeNotifier().detach(this,&IconRenderer::newViewPointPorcess);

	//make sure pending request are ignored.
	forgetPendingRequests(*m_iconTextures);

	//make sure we release all textures - this would could very late when someone reference PackedTexturesItems. therefore, we make sure the openGL sources get released now.
	m_iconTextures->clearOpenGLResources();
}


float IconRenderer::getIconScaling()
{
	static const std::string kstrScope = "IconTile";
	static const std::string kstrIconScaling = "IconScaling";
	static const std::string kstrIconScalingDescription = "Scale to apply to feature icons (typically use 0.1 to 1.0, default of 0.25).";

	static float iconScale = getAppProperty(kstrScope, kstrIconScaling, 1.0f, kstrIconScalingDescription);
	
	return iconScale;
}

void IconRenderer::setNeedPostProcessing()
{
	if (!m_postProcessingScheduled )
	{
		schedulePostProcessing(&IconRenderer::postProcessingIcons);
		m_postProcessingScheduled = true;
	}
}

void IconRenderer::newViewPointPorcess(PYXPointer<NotifierEvent> e)
{
	//create new memento
	if (!m_iconsMemento || m_iconsMemento->getSurface() != getViewThread().getSurface())
	{
		if (m_iconsMemento)
		{
			m_iconsMemento->clearAllMementos();
			GarbageCollector::getInstance()->collect(m_iconsMemento);
		}

		m_iconsMemento = IconsMemento::create(getViewThread().getSurface(),PatchIconsCreator::create());
	}
}


void IconRenderer::render()
{	
	PerformanceCounter::getTimePerformanceCounter("Start IconRender",0.5f,0.5f,0.5f)->makeMeasurement();

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//Do depth check
	glEnable(GL_DEPTH_TEST);

	//Risky - that why icons must be last :)
	//We must do it to make the icons printed infront of the globe
	glClear(GL_DEPTH_BUFFER_BIT);

	//will not update depth buffer if alpha channel is less than 0.10
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GEQUAL, 0.10f);	
	

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	getViewThread().applyCamera();
	m_altitudeFactor = 1 + getViewThread().getCamera().getOrbitalAltitude()/SphereMath::knEarthRadius;

	// Use this to control local coordinate transformations
	glPushMatrix();
	m_localOrigin = vec3(0.0, 0.0, 0.0); // default local origin
	glTranslated(-0, -0, - m_altitudeFactor); // default base origin

	// Use this matrix to rotate local origin
	cml::matrix_rotation_quaternion(m_localMatrix, getViewThread().getCamera().getOrbitalRotation());
	glMultMatrixd(m_localMatrix.data());
	
	PYXPointer<Surface> surface = getViewThread().getSurface();

	for(Surface::PatchVector::const_iterator it = surface->getVisiblePatches().begin(); it != surface->getVisiblePatches().end(); ++it)
	{
		PYXPointer<PatchIcons> icons = m_iconsMemento->getIfExists(*it);

		if (icons)
		{
			renderPatch(icons,*it);
		}
	}

	glPopMatrix();

	PerformanceCounter::getTimePerformanceCounter("End IconRender",0.5f,0.5f,0.5f)->makeMeasurement();

	getViewThread().setFrameTimeMeasurement("render-icons");
}

bool IconRenderer::renderPatch(const PYXPointer<PatchIcons> & icons,const PYXPointer<Surface::Patch> & patch)
{
	//check the blend level
	if (icons->blendLevel > 0.1)
	{
		IconsOpenGLData & openGLData = icons->getIconsData();

		openGLData.validate();

		if (openGLData.hasIcons())
		{
			if (openGLData.getOrigin() != m_localOrigin)
			{
				// Local origin has changed so redo the transformations
				glPopMatrix();
				glPushMatrix();
				m_localOrigin  = openGLData.getOrigin();
				vec3 v = m_localOrigin ;
				v = cml::transform_vector(m_localMatrix,v);
				v -= vec3(0, 0, m_altitudeFactor );
				glTranslated(v[0], v[1], v[2]);
				glMultMatrixd(m_localMatrix.data());
			}

			glColor4f(1.0f,1.0f,1.0f,static_cast<float>(icons->blendLevel));

			renderOpenGLData(openGLData);
		}
	}

	return true;
}

void IconRenderer::pickAnnotations(const Ray & ray,PickedIAnnotationVector & resultVector)
{
	struct PickingState : public PYXObject
	{
		IconRenderer & iconRenderer;
		vec3 direction;
		vec3 origin;
		double pixelDistanceFactor;
		PickedIAnnotationVector & resultVector;

		PickingState(IconRenderer & theIconRenderer,const Ray & ray,PickedIAnnotationVector & theResultVector) : resultVector(theResultVector), iconRenderer(theIconRenderer)
		{
			//get the ray direction
			direction = ray.getDirection();
			origin = ray.getOrigin();

			//and aspect ratio to screen pixel distance ratio.
			pixelDistanceFactor = iconRenderer.getViewThread().getViewportHeight()/cml::rad(iconRenderer.getViewThread().getCamera().getFovy());
		}

		void pick(const PYXPointer<Surface::Patch> & patch)
		{
			PYXPointer<PatchIcons> icons = iconRenderer.m_iconsMemento->get(patch);

			if (!icons)
			{
				return;
			}

			pick(icons,patch);
		}

		void pick(const PYXPointer<PatchIcons> & icons,const PYXPointer<Surface::Patch> & patch)
		{
			IconsOpenGLData & openGLData = icons->getIconsData();
			IconsOpenGLData::iterator ItOpenGl = openGLData.begin();

			while(ItOpenGl != openGLData.end())
			{
				//go over all icons inside this openGLData stucture...
				for(unsigned int i=0; i< ItOpenGl->second.size();i++)
				{
					//the icons are keys in the openGLData map
					PYXPointer<Icon> icon = ItOpenGl ->second.getKey(i);

					//we check for intersection only if the icon has Annotation attached to it. 
					if (icon->hasAnnotation() && icon->isAttachedToTile())
					{
						vec3 location(icon->getLocation()-origin);

						location.normalize();

						double pixelDistance = pixelDistanceFactor*acos(cml::dot(location,direction));

						PYXPointer<PackedTextureItem> item = icon->getTextureItem();

						vec2 scale = icon->getScale();

						if (pixelDistance < std::max(item->getWidth()*scale[0]*iconRenderer.getIconScaling(),item->getHeight()*scale[1]*iconRenderer.getIconScaling())/2 )
						{
							Annotation* annotation = nullptr;
							
							try
							{
								annotation = dynamic_cast<Annotation*>(icon->getAnnotation());
							}
							catch(...)
							{
								TRACE_ERROR("failed to dynamic cast an annotation while picking");
							}
							
							if (annotation != nullptr)
							{
								resultVector.push_back(std::pair<PickRange,PYXPointer<IAnnotation>>((icon->getLocation()-origin).length(),icon->getAnnotation()));
							}
						}
					}
				}

				++ItOpenGl;
			}
		}
	};

	//get the ray direction
	vec3 direction(ray.getDirection());
	//and aspect ratio to screen pixel distance ratio.
	double pixelDistanceFactor = getViewThread().getViewportHeight()/cml::rad(getViewThread().getCamera().getFovy());

	PickingState state(*this,ray,resultVector);

	const Surface::PatchVector & visiblePatches = getViewThread().getSurface()->getVisiblePatches();
	
	for(Surface::PatchVector::const_iterator it = visiblePatches.begin(); it != visiblePatches.end(); ++it)
	{
		PYXPointer<PatchIcons> icons = m_iconsMemento->getIfExists(*it);

		if (icons)
		{
			state.pick(icons,*it);
		}
	}	
}


void IconRenderer::postProcessingIcons()
{
}

PYXPointer<PatchIcons> IconRenderer::getIconsPatch(const PYXPointer<Surface::Patch> & patch)
{
	return m_iconsMemento->get(patch);
}

////////////////////////////////////////////////////////////////////////////////////

IconsOpenGLData::IconsOpenGLData() : m_origin(0,0,0), m_totalIcons(0)
{
}

IconsOpenGLData::~IconsOpenGLData()
{
	//detach all icons
	for(std::set<PYXPointer<Icon>>::iterator it = m_iconsToAdd.begin();it != m_iconsToAdd.end();++it)
	{
		(*it)->m_tile = NULL;
	}
	m_iconsToAdd.clear();

	for(std::set<PYXPointer<Icon>>::iterator it = m_updatedIcons.begin();it != m_updatedIcons.end();++it)
	{
		(*it)->m_tile = NULL;
	}
	
	for(iterator it = m_iconsOpenGLData.begin();it != m_iconsOpenGLData.end(); ++it)
	{
		for(size_t i=0;i<it->second.size();i++)
		{
			it->second.getKey(i)->m_tile = NULL;
		}
	}
	releaseMemory(m_totalIcons*sizeof(IconOpenGLData));
}

void IconsOpenGLData::add(const PYXPointer<Icon> & icon)
{
	if (icon->m_tile != this)
	{
		if (icon->m_hasOpenGLData)
		{
			icon->remove();
		}

		icon->m_tile = this;
	}
	else
	{
		if (! icon->m_hasOpenGLData)
		{
			m_iconsToAdd.insert(icon);
			m_iconsToRemove.erase(icon);
		}
		else
		{
			m_updatedIcons.insert(icon);
			m_iconsToRemove.erase(icon);
		}
	}
}

void IconsOpenGLData::remove(const PYXPointer<Icon> & icon)
{
	if (icon->m_hasOpenGLData)
	{
		m_iconsToRemove.insert(icon);
	}
	m_iconsToAdd.erase(icon);
	m_updatedIcons.erase(icon);
}

void IconsOpenGLData::validate()
{
	int oldSize = m_totalIcons;
	for(std::set<PYXPointer<Icon>>::iterator it = m_iconsToAdd.begin();it != m_iconsToAdd.end();++it)
	{
		m_iconsOpenGLData[(*it)->m_textureItem->getTexture()].insert(*it,IconOpenGLData());
		(*it)->m_hasOpenGLData = true;
		m_updatedIcons.insert(*it);
		m_totalIcons++;
	}
	m_iconsToAdd.clear();

	for(std::set<PYXPointer<Icon>>::iterator it = m_iconsToRemove.begin();it != m_iconsToRemove.end();++it)
	{
		m_iconsOpenGLData[(*it)->m_textureItem->getTexture()].erase(*it);

		//if this is the last icon, remove the map
		if (m_iconsOpenGLData[(*it)->m_textureItem->getTexture()].size()==0)
		{
			m_iconsOpenGLData.erase((*it)->m_textureItem->getTexture());
			m_totalIcons--;
		}

		(*it)->m_hasOpenGLData = false;
	}
	m_iconsToRemove.clear();

	for(std::set<PYXPointer<Icon>>::iterator it = m_updatedIcons.begin();it != m_updatedIcons.end();++it)
	{
		updateIcon(*it,m_iconsOpenGLData[(*it)->m_textureItem->getTexture()][*it]);
	}
	m_updatedIcons.clear();

	if (m_totalIcons != oldSize)
	{
		consumeMemory((m_totalIcons-oldSize)*sizeof(IconOpenGLData));
	}
}

void IconsOpenGLData::rotateIcons(const vec3 & left,const vec3 & up,const float & scale)
{
	for(auto & data : m_iconsOpenGLData)
	{
		for(auto & openGLData : data.second)
		{
			float scaleX = scale*openGLData.vertices[0].scale[0];
			float scaleY = scale*openGLData.vertices[0].scale[1];

			for (int j=0; j<4; j++)
			{
				openGLData.vertices[j].vertex[0] = openGLData.vertices[j].pos[0]
					+static_cast<float>(left[0])*openGLData.vertices[j].pixelOffset[0]*scaleX
					+static_cast<float>(up[0])*openGLData.vertices[j].pixelOffset[1]*scaleY;
				
				openGLData.vertices[j].vertex[1] = openGLData.vertices[j].pos[1]
					+static_cast<float>(left[1])*openGLData.vertices[j].pixelOffset[0]*scaleX
					+static_cast<float>(up[1])*openGLData.vertices[j].pixelOffset[1]*scaleY;

				openGLData.vertices[j].vertex[2] = openGLData.vertices[j].pos[2]
					+static_cast<float>(left[2])*openGLData.vertices[j].pixelOffset[0]*scaleX
					+static_cast<float>(up[2])*openGLData.vertices[j].pixelOffset[1]*scaleY;
			}
		}
	}
}

void IconsOpenGLData::updateIcon(const PYXPointer<Icon> & icon, IconOpenGLData & data)
{
	//make a safe origin transformation
	float pos[3];		
	CmlConvertor::fromVec3WithOrigin(icon->getLocation(),getOrigin(),pos);	

	//vertex #1 of 4
	memcpy(&data.vertices[0].pos,pos,sizeof(pos));
	memcpy(&data.vertices[0].vertex,pos,sizeof(pos));
	memcpy(&data.vertices[0].color,icon->m_color,sizeof(icon->m_color));
	data.vertices[0].pixelOffset[0] = static_cast<float>(-icon->getTextureItem()->getWidth()/2 + icon->getOffset()[0]);
	data.vertices[0].pixelOffset[1] = static_cast<float>(-icon->getTextureItem()->getHeight()/2 + icon->getOffset()[1]);
	data.vertices[0].textureCoord[0] = (icon->getTextureItem()->getLeftTextureCoord()+0.5f/512);
	data.vertices[0].textureCoord[1] = (icon->getTextureItem()->getTopTextureCoord()-0.5f/512);
	data.vertices[0].scale[0] = static_cast<float>(icon->getScale()[0]);
	data.vertices[0].scale[1] = static_cast<float>(icon->getScale()[1]);

	//vertex #2 of 4
	memcpy(&data.vertices[1].pos,pos,sizeof(pos));
	memcpy(&data.vertices[1].vertex,pos,sizeof(pos));
	memcpy(&data.vertices[1].color,icon->m_color,sizeof(icon->m_color));
	data.vertices[1].pixelOffset[0] = static_cast<float>(icon->getTextureItem()->getWidth()/2 + icon->getOffset()[0]);
	data.vertices[1].pixelOffset[1] = static_cast<float>(-icon->getTextureItem()->getHeight()/2 + icon->getOffset()[1]);
	data.vertices[1].textureCoord[0] = (icon->getTextureItem()->getRightTextureCoord()-0.5f/512);
	data.vertices[1].textureCoord[1] = (icon->getTextureItem()->getTopTextureCoord()-0.5f/512);
	data.vertices[1].scale[0] = static_cast<float>(icon->getScale()[0]);
	data.vertices[1].scale[1] = static_cast<float>(icon->getScale()[1]);

	//vertex #3 of 4
	memcpy(&data.vertices[2].pos,pos,sizeof(pos));
	memcpy(&data.vertices[2].vertex,pos,sizeof(pos));
	memcpy(&data.vertices[2].color,icon->m_color,sizeof(icon->m_color));
	data.vertices[2].pixelOffset[0] = static_cast<float>(icon->getTextureItem()->getWidth()/2 + icon->getOffset()[0]);
	data.vertices[2].pixelOffset[1] = static_cast<float>(icon->getTextureItem()->getHeight()/2 + icon->getOffset()[1]);
	data.vertices[2].textureCoord[0] = (icon->getTextureItem()->getRightTextureCoord()-0.5f/512);
	data.vertices[2].textureCoord[1] = (icon->getTextureItem()->getBottomTextureCoord()+0.5f/512);
	data.vertices[2].scale[0] = static_cast<float>(icon->getScale()[0]);
	data.vertices[2].scale[1] = static_cast<float>(icon->getScale()[1]);

	//vertex #4 of 4
	memcpy(&data.vertices[3].pos,pos,sizeof(pos));
	memcpy(&data.vertices[3].vertex,pos,sizeof(pos));
	memcpy(&data.vertices[3].color,icon->m_color,sizeof(icon->m_color));
	data.vertices[3].pixelOffset[0] = static_cast<float>(-icon->getTextureItem()->getWidth()/2 + icon->getOffset()[0]);
	data.vertices[3].pixelOffset[1] = static_cast<float>(icon->getTextureItem()->getHeight()/2 + icon->getOffset()[1]);
	data.vertices[3].textureCoord[0] = (icon->getTextureItem()->getLeftTextureCoord()+0.5f/512);
	data.vertices[3].textureCoord[1] = (icon->getTextureItem()->getBottomTextureCoord()+0.5f/512);
	data.vertices[3].scale[0] = static_cast<float>(icon->getScale()[0]);
	data.vertices[3].scale[1] = static_cast<float>(icon->getScale()[1]);	
}

////////////////////////////////////////////////////////////////////////////////////

Icon::Icon(IconsOpenGLData * tile,
	PYXPointer<PackedTextureItem> textureItem,
	const vec2 & scale,
	const vec2 & offset,
	const vec3 & location,
	IAnnotation * annotation)
	:
	m_tile(tile),
	m_textureItem(textureItem),
	m_scale(scale),
	m_offset(offset),
	m_location(location),
	m_hasOpenGLData(false),
	m_annotation(annotation)
{
	memset(&m_color[0],255,sizeof(m_color));
}

Icon::~Icon()
{
	remove();
}

void Icon::remove()
{
	if (m_tile != NULL)
	{
		m_tile->remove(this);
	}
	m_tile = NULL;
}

void Icon::setScale(const vec2 & scale)
{
	m_scale = scale;
	if (m_tile != NULL)
	{
		m_tile->m_updatedIcons.insert(this);
	}
}

void Icon::setOffset(const vec2 & offset)
{
	m_offset = offset;
	if (m_tile != NULL)
	{
		m_tile->m_updatedIcons.insert(this);
	}
}

void Icon::setLocation(const vec3 & location)
{
	m_location = location;
	if (m_tile != NULL)
	{
		m_tile->m_updatedIcons.insert(this);
	}
}

void Icon::setColor(const unsigned char color[4])
{
	memcpy(m_color,color,sizeof(m_color));	
	if (m_tile != NULL)
	{
		m_tile->m_updatedIcons.insert(this);
	}
}


void Icon::setLocationElevation(const double & elevation)
{
	m_location.normalize();
	m_location *= elevation / SphereMath::knEarthRadius + 1.0;
	if (m_tile != NULL)
	{
		m_tile->m_updatedIcons.insert(this);
	}
}

PYXPointer<Icon> PatchIcons::addIcon(PYXPointer<PackedTextureItem> textureItem,
									 const vec2 & scale,
									 const vec2 & offset,
									 const vec3 & location,
									 IAnnotation * annotation)
{
	PYXPointer<Icon> icon = Icon::create(m_openGLData.get(),textureItem,scale,offset,location,annotation);
	m_openGLData->add(icon);
	return icon;
}