#pragma once
#ifndef VIEW_MODEL__ICON_ANNOTATION_CONTROLLER_H
#define VIEW_MODEL__ICON_ANNOTATION_CONTROLLER_H
/******************************************************************************
icon_annotation_controller.h

begin		: 2011-01-17
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "component.h"

#include "pyxis/utility/thread_pool.h"

/*!
IconAnnotationsController - responsible to generate Icons from input Pipelines

-- Description:
     - Generate IconSkeletons from pipelines for each patch with background threads
     - On render loop, call PatchIconAnnotation->update() to make annotatation update themselfs
	 - This Controller doesn't deal with picking annotation, it happen in IconRenderer

-- OpenGL extentions:
	 - None

-- Limitations: 
	 - Too many icons can slow down the FPS
	 - Too many piplines can slow dwon the FPS
	 
*/
//! IconAnnotationsController - responsible to generate Icons from input Pipelines
class IconAnnotationsController : public Component
{
public:

	/*!
	IconStyleSkeleton represent the needed data to generate the needed OpenGL resources for a given style.	
	*/
	class IconStyleSkeleton : public PYXObject, ObjectMemoryUsageCounter<IconStyleSkeleton>
	{
	protected:
		std::string m_style;

		vec2 m_scale;
		vec2 m_offset;
		vec2 m_textAlign; //! values: -1 - top/left, 0 - center, 1 - bottom/right

		std::string m_iconStyle; //icon bitmap definition
		std::string m_textField; //icon text field
		std::string m_textAttributes; //text field font styling attributes		

		bool m_textAppearAlways;

		std::string m_colorField; //color field font styling attributes
		PYXPointer<PYXValueColorPalette> m_colorPalette;
		bool m_hasCustomColor;
		unsigned char m_color[4];

	public:
		const std::string & getStyle() const 
		{
			return m_style;
		}

		const std::string & getIconBitmapDefinition() const 
		{
			return m_iconStyle;
		}

		std::string getTextBitmapDefinitionForString(const std::string & text)
		{
			return "<Text "+ m_textAttributes + ">" + XMLUtils::toSafeXMLText(text) + "</Text>";
		}

		std::string getTextBitmapDefinition(const boost::intrusive_ptr<IFeature> & feature) const 
		{
			if (feature)
			{
				PYXValue value = feature->getFieldValueByName(m_textField);

				if (value.isFloat() || value.isDouble())
				{
					std::string str = StringUtils::toString(value.getDouble(),4);
					return "<Text "+ m_textAttributes + ">" + str + "</Text>";
				}
				else
				{
					//encode the feaild value into UTF8 while removeing all xml specific markers to make it a complete string value insde the xml document
					return "<Text "+ m_textAttributes + ">" + XMLUtils::toSafeXMLText(feature->getFieldValueByName(m_textField).getString()) + "</Text>";
				}
			}
			else
			{
				//empty text...
				return "<Text "+ m_textAttributes + "></Text>";
			}
		}

		const vec2 & getIconScale() const { return m_scale; }
		const vec2 & getIconOffset() const { return m_offset; }
		const vec2 & getTextAlign() const { return m_textAlign; }
		bool getTextAppearAlways() const { return m_textAppearAlways; } 
		bool hasIcon() const { return !m_iconStyle.empty(); }
		bool hasText() const { return !m_textField.empty(); }
		bool hasCustomColor() const { return !m_colorField.empty(); }

		void getColor(OUT unsigned char * color) const { memcpy(color,m_color,sizeof(m_color)); }
		void getCustomColor(const boost::intrusive_ptr<IFeature> & feature,OUT unsigned char * color) const;

	public:
		IconStyleSkeleton(const std::string & style);

		static PYXPointer<IconStyleSkeleton> create(const std::string & style)
		{
			return PYXNEW(IconStyleSkeleton,style);
		}
	};

	/*!
	IconStyle storer all needed OpenGL resources for a given style. 
	IconStyle is generated from IconStyleSkeleton and pointer to IconRenderer.

	this class doesn't store feature specific OpenGL resources, such as the Text Texture 
	*/
	class IconStyle : public PYXObject, ObjectMemoryUsageCounter<IconStyle>
	{
	private:
		PYXPointer<IconStyleSkeleton> m_skeleton;
		IconRenderer & m_iconRenderer;
		PYXPointer<PackedTextureItem> m_iconTexture;

	public:
		IconStyle(IconRenderer & iconRenderer,const PYXPointer<IconStyleSkeleton> & skeleton) 
			:	m_skeleton(skeleton),
				m_iconRenderer(iconRenderer)
		{
		}

		static PYXPointer<IconStyle> create(IconRenderer & iconRenderer,const PYXPointer<IconStyleSkeleton> & skeleton)
		{
			return PYXNEW(IconStyle,iconRenderer,skeleton);
		}

	public:

		std::string getTextBitmapDefinitionForString(const std::string & text) const 
		{
			return m_skeleton->getTextBitmapDefinitionForString(text);
		}

		std::string getTextBitmapDefinition(boost::intrusive_ptr<IFeature> feature) const 
		{
			return m_skeleton->getTextBitmapDefinition(feature);
		}

		const vec2 & getIconScale() const { return m_skeleton->getIconScale(); }
		const vec2 & getIconOffset() const { return m_skeleton->getIconOffset(); }
		const vec2 & getTextAlign() const { return m_skeleton->getTextAlign(); }
		bool getTextAppearAlways() const { return m_skeleton->getTextAppearAlways(); }
		
		bool hasIcon() const { return m_skeleton->hasIcon(); }
		bool hasText() const { return m_skeleton->hasText(); }
		bool hasCustomColor() const { return m_skeleton->hasCustomColor(); }

		void getColor(/*out*/unsigned char * color) const { m_skeleton->getColor(color); }
		void getCustomColor(boost::intrusive_ptr<IFeature> feature,/*out*/unsigned char * color) const { m_skeleton->getCustomColor(feature,color); }

		IconRenderer & getIconRenderer() { return m_iconRenderer; }

		PYXPointer<PackedTextureItem> getIconTexture()
		{
			if (! m_iconTexture)
			{
				m_iconTexture = m_iconRenderer.getTextureFromBitmapDefinition(m_skeleton->getIconBitmapDefinition());
			}

			return m_iconTexture;
		}

		PYXPointer<Icon> addIcon(const PYXPointer<PackedTextureItem> texture, const PYXPointer<Surface::Patch> & patch,const vec3 & location,const vec2 & scale, const vec2 & offset,IAnnotation * annotation = NULL)
		{
			assert(texture && "can't add icon becasue there is no texture yet");

			PYXPointer<PatchIcons> icons = m_iconRenderer.getIconsPatch(patch);
			if (icons)
			{
				return icons->addIcon(texture,scale,offset,location,annotation);
			}
			else
			{
				return PYXPointer<Icon>();
			}
		}

		void removeIcon(PYXPointer<Icon> icon)
		{
			return icon->remove();
		}

		void clearOpenGLResources()
		{
			m_iconTexture.reset();
		}
	};

	/*!
	IconAnnotationSkeleton represent the needed data to generate an IconAnnotation. this class has no OpenGL resources attached to it.
	*/
	class IconAnnotationSkeleton : public PYXObject, ObjectMemoryUsageCounter<IconAnnotationSkeleton>
	{
	protected:
		std::string m_featureID;
		std::string m_groupID;
		bool m_isGroup;
		vec2 m_uv;
		vec3 m_location;
		PYXPointer<IconStyleSkeleton> m_styleSkeleton;

	public:
		const std::string & getFeatureID() { return m_featureID; }
		const std::string & getGroupID() { return m_groupID; }
		const PYXPointer<IconStyleSkeleton> & getStyleSkeleton() { return m_styleSkeleton; }
		const vec2 & getUV() const { return m_uv; }
		const vec3 & getLocation() const { return m_location; }
		bool isGroup() const { return m_isGroup; }

	public:
		IconAnnotationSkeleton(const std::string & featureID,const std::string & groupID,const PYXPointer<IconStyleSkeleton> & styleSkeleton,const vec2 & uv,const vec3 & location,bool group)
			:	m_featureID(featureID),
				m_groupID(groupID),
				m_styleSkeleton(styleSkeleton),
				m_uv(uv),
				m_location(location),
				m_isGroup(group)
		{
		}

		static PYXPointer<IconAnnotationSkeleton> create(const std::string & featureID,const std::string & groupID,const PYXPointer<IconStyleSkeleton> & styleSkeleton,const vec2 & uv,const vec3 & location,bool group)
		{
			return PYXNEW(IconAnnotationSkeleton,featureID,groupID,styleSkeleton,uv,location,group);
		}
	};

	/*!
	StyledIcon is the IconAnnotation that is generated from the pipelines.

	TOOD:
		- make this IconAnnotation to represent a collection of icons.
	*/
	class StyledIcon : public Annotation, ObjectMemoryUsageCounter<StyledIcon>
	{
	public:
		StyledIcon(IViewModel * view,const boost::intrusive_ptr<IProcess> & process,const PYXPointer<Surface::Patch> & patch,const std::string & style,const PYXPointer<IconAnnotationSkeleton> & skeleton,const PYXPointer<IconStyle> & iconStyle);

		static PYXPointer<StyledIcon> create(IViewModel * view,const boost::intrusive_ptr<IProcess> & process,const PYXPointer<Surface::Patch> & patch,const std::string & style,const PYXPointer<IconAnnotationSkeleton> & skeleton,const PYXPointer<IconStyle> & iconStyle)
		{
			return PYXNEW(StyledIcon,view,process,patch,style,skeleton,iconStyle);
		}

		virtual ~StyledIcon();

	protected:
		PYXPointer<IconStyle> m_iconStyle;
		PYXPointer<IconAnnotationSkeleton> m_skeleton;

		PYXPointer<Surface::Patch> m_patch;
		PYXPointer<Icon> m_icon;

		bool m_textGenerated;
		std::string m_textStyle;
		PYXPointer<Icon> m_textIcon;
		PYXPointer<PackedTextureItem> m_textTexture;
		PYXPointer<PYXTaskWithContinuation> m_backgroundTask;

		vec3 m_location;
		
		bool m_colorCalculated;
		unsigned char m_color[4];
		PYXPointer<PYXTaskWithContinuation> m_colorBackgroundTask;

		bool generateText();

		PYXPointer<PackedTextureItem> getTextTextureItem();

	private:
		static void generateTextFromFeature(PYXPointer<StyledIcon> icon);
		static void generateCustomColorFromFeature(PYXPointer<StyledIcon> icon);

	public:
		virtual void onMouseEnter(PYXPointer<AnnotationMouseEvent> eventData);
		virtual void onMouseLeave(PYXPointer<AnnotationMouseEvent> eventData);

	//UI creation;
	public:
		virtual bool wasVisualizationGenerated();
		virtual bool canGenerateVisualization();
		virtual void generateVisualization();
		virtual void destroyVisualization();
		virtual void updateVisualization();
		virtual void validateElevation();
	};

	class IconAnnotationSkeletonPatch : public PYXObject
	{
	protected:
		typedef std::list<PYXPointer<IconAnnotationSkeleton>> IconAnnotationSkeletonList;
		std::list<PYXPointer<IconAnnotationSkeleton>> m_annotations;

		PYXPointer<IconAnnotationSkeleton> m_annotationsLocation[10][10];
	public:
		IconAnnotationSkeletonPatch()
		{
		};

		static PYXPointer<IconAnnotationSkeletonPatch> create()
		{
			return PYXNEW(IconAnnotationSkeletonPatch);
		}

	public:
		void addAnnotationSkeleton(int u,int v,PYXPointer<IconAnnotationSkeleton> annotation)
		{
			assert(u>=0 && u<=9 && v>=0 && v<=9 && "UV is out of range");
			assert(!m_annotationsLocation[u][v] && "location is already occupied by another annotation");

			m_annotationsLocation[u][v] = annotation;
			m_annotations.push_back(annotation);
		}

		PYXPointer<IconAnnotationSkeleton> getAnnotationSkeleton(int u,int v)
		{
			assert(u>=0 && u<=9 && v>=0 && v<=9 && "UV is out of range");
			return m_annotationsLocation[u][v];
		}
	};

	class IconAnnotationPatch : public PYXObject
	{
	public:
		typedef std::map<ProcRef,PYXPointer<IconAnnotationSkeletonPatch>> IconAnnotationSkeletonPatchMap;

	protected:
		IconAnnotationSkeletonPatchMap m_skeletons;
		PYXPointer<PatchAnnotations> m_annotations;

	public:
		const PYXPointer<PatchAnnotations> & getAnnotations() { return m_annotations; }

		IconAnnotationSkeletonPatchMap & getSkeletons() { return m_skeletons; }

	public:
		IconAnnotationPatch() : m_annotations(PatchAnnotations::create())
		{
		}

		static PYXPointer<IconAnnotationPatch> create()
		{
			return PYXNEW(IconAnnotationPatch);
		}
	};

public:

	class IFeaturesSelector : public PYXObject
	{
	public:
		struct SelectedFeatureInfo
		{
			std::string m_featureID;
			std::string m_groupID;
			PYXCoord3DDouble m_location;
			bool m_isGroup;

			SelectedFeatureInfo(const AnnotationCache::AnnotationInfo & info) : m_featureID(info.m_featureID), m_groupID(info.m_groupID), m_location(info.m_location), m_isGroup(info.m_isGroup)
			{
			}

			SelectedFeatureInfo(const std::string & featureId,const std::string & groupID,const PYXCoord3DDouble & location, bool group) : m_featureID(featureId), m_groupID(groupID), m_location(location), m_isGroup(group)
			{
			}
		};

		static PYXPointer<IFeaturesSelector> create(const boost::intrusive_ptr<IProcess> & process);

		virtual std::auto_ptr<SelectedFeatureInfo> getFeatureToVisualizeForCell(const PYXIcosIndex & index) = 0;

		virtual bool isLoading() const = 0;

	protected:
		IFeaturesSelector() {};
	};

	//! set of all patches need to load
	typedef std::set<PYXPointer<Surface::Patch>> NeededPatchesSet;

	/*!
	LoadingThread class is responsible to generate IconAnnotationSkeletons for a given Pipeline.

	When a new patch is ready, the LoadingThread will notify the IconAnnotationsController that will generate all the OpenGL data for the skeletons.
	IconAnnotationsController generate the OpenGL Data on the main thread (using Sync::beginInvoke)
	*/
	class LoadingThread : public PYXObject
	{
	private:
		typedef WorkerThreadWithPriorities<Surface::Patch,Surface::Patch::PriorityCompare> WorkingThread;
	
		//! the background thread
		WorkingThread m_thread;
		boost::recursive_mutex m_mutex;

		boost::posix_time::ptime m_lastInvalidateTime;
		bool m_invalidateWhenReady;

		//! the process to load from
		boost::intrusive_ptr<IProcess> m_process;

		//! all the visible patches that needed to have loading job 
		NeededPatchesSet m_neededPatches;

		PYXPointer<IFeaturesSelector> m_selector;

		//! the IconAnnotationsController to notify about loaded sekeletons
		IconAnnotationsController * m_controller;

		//! all the genereated styles needed for this pipline
		typedef std::map<std::string,PYXPointer<IconStyleSkeleton>> StyleCache;
		StyleCache m_styleCache;

		//! the default style for the pipline
		std::string m_globalStyle;

	protected:
		LoadingThread(const boost::intrusive_ptr<IProcess> & process, IconAnnotationsController * controller = 0);

		virtual ~LoadingThread();

	public:
		static PYXPointer<LoadingThread> create(const boost::intrusive_ptr<IProcess> & process, IconAnnotationsController * controller = 0);

	public:
		int getWaitingJobsCount();

		const boost::intrusive_ptr<IProcess> getProcess() { return m_process; }

		void addPatch(const PYXPointer<Surface::Patch> & patch);
		void addPatches(const std::set<PYXPointer<Surface::Patch>> & patches);
		
		void removePatch(const PYXPointer<Surface::Patch> & patch);

		void invalideAll();

	protected:

		void loadPatch(PYXPointer<Surface::Patch> patch);

		PYXPointer<IconAnnotationSkeletonPatch> generateAnnotations(const PYXPointer<Surface::Patch> & patch);

		PYXPointer<IconAnnotationSkeleton> createAnnotationSkeleton(const PYXPointer<Surface::Patch> & patch,const std::auto_ptr<IFeaturesSelector::SelectedFeatureInfo> & feature);

		PYXIcosIndex findFeatureCenter(PYXPointer<PYXGeometry> geometry);

		PYXPointer<IconStyleSkeleton> getStyleSkeleton(const std::string & style);

	public:
		void attach(IconAnnotationsController & controller);
		void detach(IconAnnotationsController & controller);
	};

protected:
	PYXPointer<IconRenderer> m_iconRenderer;
	PYXPointer<Surface> m_surface;

	typedef std::map<PYXPointer<Surface::Patch>,PYXPointer<IconAnnotationPatch>> IconAnnotationPatchMap;
	IconAnnotationPatchMap m_visiblePatches;
	NeededPatchesSet m_neededPatches;
	
	typedef std::map<boost::intrusive_ptr<IProcess>,PYXPointer<LoadingThread>> LoadingThreadsMap;
	LoadingThreadsMap m_loadingThreads;

	typedef std::map<std::string,PYXPointer<IconStyle>> IconStyleCache;
	typedef std::map<boost::intrusive_ptr<IProcess>,IconStyleCache> IconStyleCacheMap;
	IconStyleCacheMap m_stylesCache;

public:
	IconAnnotationsController(ViewOpenGLThread & viewThread);

	virtual ~IconAnnotationsController();

	static PYXPointer<IconAnnotationsController> create(ViewOpenGLThread & viewThread)
	{
		return PYXNEW(IconAnnotationsController,viewThread);
	}

	void setIconRenderer(const PYXPointer<IconRenderer> & iconRenderer);
	
public:
	virtual void render();

protected:

	void updateLoaders(PYXPointer<NotifierEvent> e);
	void onPatchBecomeVisible(PYXPointer<NotifierEvent> e);
	void onPatchBecomeHidden(PYXPointer<NotifierEvent> e);

	void removeAllAnnotations(const boost::intrusive_ptr<IProcess> & process);
	
public:
	void borrowIcons(const PYXPointer<Surface::Patch> & patch);
	void startLoadersTasks();

	void onNewAnnotations(	const PYXPointer<Surface::Patch> & patch, 
							const boost::intrusive_ptr<IProcess> & process,
							const PYXPointer<IconAnnotationSkeletonPatch> & newProcessAnnotations);	

protected:

	void addNewAnnotations(	PYXPointer<Surface::Patch> patch, 
							boost::intrusive_ptr<IProcess> process,
							PYXPointer<IconAnnotationSkeletonPatch> newProcessAnnotations);

	PYXPointer<Annotation> createAnnotation(	const PYXPointer<Surface::Patch> & patch, 
												const boost::intrusive_ptr<IProcess> & process,
												const PYXPointer<IconAnnotationSkeleton> & skeleton );


	int getLoadingProgress();
};


#endif