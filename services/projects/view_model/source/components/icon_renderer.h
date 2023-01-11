#pragma once
#ifndef VIEW_MODEL__ICON_RENDERER_H
#define VIEW_MODEL__ICON_RENDERER_H
/******************************************************************************
icon_renderer.h

begin		: 2009-11-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "component.h"
#include "animation.h"
#include "texture_packer.h"
#include "cml_utils.h"
#include "view_open_gl_thread.h"
#include "stile.h"
#include "animation.h"
#include "continues_data_map.h"
#include "surface_memento.h"

#include "pyxis/derm/index.h"
#include "pyxis/pipe/process.h"

#include "pyxis/geometry/circle_geometry.h"

#include <map>
#include <set>

class IconsOpenGLData;
class Icon;
class PatchIcons;

/*!

IconRenderer - render icons on the screen using a vertex shader

-- Description:
     - This renderer is strongly depend on View visible tiles implemetation and on STile structure and IconTile class.
     - Uses camera range property and Fovy to detemine icons scaling
     - Uses IconCPURenderer (CPU) to initialize icons bitmap - hack

-- OpenGL extentions:
	 - Vertex Shaders

-- Limitations: 
     - must have shaders support
     - the FPS would be less then 30 for more than 10,000 Icons on screen
	 - only use demo icons at the momement
	 - doesn't support depth test - so Icons far away could be printeded infornt of closer icons
	 
*/
//! IconGPURenderer - render icons on the screen using a vertex shader
class IconRenderer : public Component
{
public:
	IconRenderer(ViewOpenGLThread & viewThread);
	virtual ~IconRenderer(void);

	//virtual bool initialize(); - implemented by derived classes
	virtual void render(); 

	//! Perfrom a Ray Picking
	virtual void pickAnnotations(const Ray & ray,PickedIAnnotationVector & resultVector);

	//! schedule a post processing if needed
	void setNeedPostProcessing();	

protected:
	bool m_postProcessingScheduled;
	void postProcessingIcons();
	
	void newViewPointPorcess(PYXPointer<NotifierEvent> e);

	bool renderPatch(const PYXPointer<PatchIcons> & icons,const PYXPointer<Surface::Patch> & patch);

	virtual void renderOpenGLData(IconsOpenGLData & openGLData) = 0;

	//! texture packer for icons bitmaps
	PYXPointer<TexturePacker> m_iconTextures;

	typedef SurfaceMemento<PatchIcons> IconsMemento;

	PYXPointer<IconsMemento> m_iconsMemento;
	
	double m_altitudeFactor;
	vec3 m_localOrigin;
	mat4 m_localMatrix;

	//Icon scaling backward compatability
public:
	static float getIconScaling();


	//generic Icon addition and removal
public:
	PYXPointer<PatchIcons> getIconsPatch(const PYXPointer<Surface::Patch> & patch);

	PYXPointer<PackedTextureItem> getTextureFromBitmapDefinition(const std::string &bitmapDefinition)
	{
		try
		{
			return Component::getTextureFromBitmapDefinition(*m_iconTextures,bitmapDefinition);
		}
		catch(...)
		{
			TRACE_ERROR("Failed to load texture from bitmap definition " << bitmapDefinition);
		}
		return PYXPointer<PackedTextureItem>();
	}
};

class Icon : public PYXObject, ObjectMemoryUsageCounter<Icon>
{
	friend class IconsOpenGLData;

protected:
	IconsOpenGLData * m_tile;

	PYXPointer<PackedTextureItem> m_textureItem;
	vec2 m_scale;
	vec2 m_offset;
	vec3 m_location;
	unsigned char m_color[4];
	bool m_hasOpenGLData;
	IAnnotation * m_annotation;

public:
	Icon(IconsOpenGLData * tile,
		 PYXPointer<PackedTextureItem> textureItem,
		 const vec2 & scale,
		 const vec2 & offset,
		 const vec3 & location,
		 IAnnotation * annotation = NULL);

	static PYXPointer<Icon> create(IconsOpenGLData * tile,
		 PYXPointer<PackedTextureItem> textureItem,
		 const vec2 & scale,
		 const vec2 & offset,
		 const vec3 & location,
		 IAnnotation * annotation = NULL)
	{
		return PYXNEW(Icon,tile,textureItem,scale,offset,location,annotation);
	}

	virtual ~Icon();

public:
	void remove();

	void setScale(const vec2 & scale);	

	void setOffset(const vec2 & offset);	

	void setLocation(const vec3 & location);	

	void setLocationElevation(const double & elevation);

	void setColor(const unsigned char color[4]);

	const vec3 & getLocation() const
	{
		return m_location;
	}

	const vec2 & getScale() const
	{
		return m_scale;
	}

	const vec2 & getOffset() const
	{
		return m_offset;
	}

	const unsigned char * getColor() const
	{
		return m_color;
	}

	const PYXPointer<PackedTextureItem> & getTextureItem() const
	{
		return m_textureItem;
	}

	bool isAttachedToTile() const
	{
		return m_tile != nullptr;
	}

	bool hasAnnotation() const
	{
		return m_annotation != nullptr;
	}

	IAnnotation * getAnnotation() const
	{
		return m_annotation;
	}

	void setAnnotation(IAnnotation * annotation)
	{
		m_annotation = annotation;
	}
};

struct IconOpenGLVertexData
{
	float pos[3];
	float pixelOffset[2];
	float scale[2];
	float vertex[3];
	float textureCoord[2];
	unsigned char color[4];
};

struct IconOpenGLData
{
	IconOpenGLVertexData vertices[4];
};

class IconsOpenGLData : public PYXObject, ObjectMemoryUsageCounter<IconsOpenGLData>
{
	friend class Icon;

public:
	typedef std::map<PYXPointer<OpenGLTexture>,ContinuesDataMap<PYXPointer<Icon>,IconOpenGLData>> OpenGLDataMap;
	typedef OpenGLDataMap::iterator iterator;

protected:
	vec3 m_origin;
	int m_totalIcons;

	std::set<PYXPointer<Icon>> m_iconsToAdd;
	std::set<PYXPointer<Icon>> m_iconsToRemove;
	std::set<PYXPointer<Icon>> m_updatedIcons;

	OpenGLDataMap m_iconsOpenGLData;

public:
	IconsOpenGLData();

	static PYXPointer<IconsOpenGLData> create()
	{
		return PYXNEW(IconsOpenGLData);
	}

	virtual ~IconsOpenGLData();	

public:
	void add(const PYXPointer<Icon> & icon);
	void remove(const PYXPointer<Icon> & icon);		
	void validate();
	
	const vec3 & getOrigin()
	{
		return m_origin;
	}

	void setOrigin(const vec3 & origin)
	{
		m_origin = origin;
	}

	void rotateIcons(const vec3 & left,const vec3 & up,const float & scale);
	void updateIcon(const PYXPointer<Icon> & icon,IconOpenGLData & data);
	
public:
	iterator begin()
	{
		return m_iconsOpenGLData.begin();
	}

	iterator end()
	{
		return m_iconsOpenGLData.end();
	}

	bool hasIcons() const
	{
		return m_iconsOpenGLData.size() != 0;
	}
};

class PatchIcons : public PYXObject, ObjectMemoryUsageCounter<PatchIcons>
{
protected:
	PYXPointer<Surface::Patch> m_patch;
	PYXPointer<IconsOpenGLData> m_openGLData;

	PatchIcons(const PYXPointer<Surface::Patch> & patch,const vec3 & origin) : m_patch(patch),blendLevel(1.0)
	{
		m_openGLData = IconsOpenGLData::create();
		m_openGLData->setOrigin(origin);
	};

public:
	static PYXPointer<PatchIcons> create(const PYXPointer<Surface::Patch> & patch,const vec3 & origin)
	{
		return PYXNEW(PatchIcons,patch,origin);
	}

public:
	PYXPointer<Icon> addIcon(PYXPointer<PackedTextureItem> textureItem,
							 const vec2 & scale,
							 const vec2 & offset,
							 const vec3 & location,
							 IAnnotation * annotation = NULL);

	IconsOpenGLData & getIconsData() { return *m_openGLData; }

public:
	double blendLevel;
};


class PatchIconsCreator : public MementoCreator<PatchIcons>
{
public:
	static PYXPointer<PatchIconsCreator> create()
	{
		return PYXNEW(PatchIconsCreator);
	}

	PatchIconsCreator()
	{
	}

	virtual PYXPointer<PatchIcons> createMemento(const PYXPointer<Surface::Patch> & patch)
	{
		PYXPointer<Surface::Patch::VertexBuffer> elevation = patch->getVertices();
		if (elevation)
		{
			return PatchIcons::create(patch,elevation->zero);
		}
		else
		{
			return PYXPointer<PatchIcons>();
		}
	}

	virtual void destroyMemento(const PYXPointer<Surface::Patch> & patch,PYXPointer<PatchIcons> & memento)
	{
	}

	virtual PYXPointer<PatchIcons> validateMemento(const PYXPointer<Surface::Patch> & patch,const PYXPointer<PatchIcons> & memento)
	{
		return memento;
	}
};

#endif