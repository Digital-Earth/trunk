/******************************************************************************
icon_annotation_controller.cpp

begin		: 2011-01-17
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "icon_renderer.h"
#include "icon_annotation_controller.h"

#include "view_open_gl_thread.h"
#include "gl_utils.h"
#include "garbage_collector.h"
#include "performance_counter.h"

#include "pyxis/utility/xml_utils.h"
#include "pyxis/data/feature_group.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/geometry/vector_geometry.h"
#include "pyxis/geometry/vector_geometry2.h"
#include "pyxis/region/vector_point_region.h"
#include "pyxis/utility/thread_pool.h"
#include "pyxis/utility/profile.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <cassert>
#include <map>
#include <vector>

/////////////////////////////////////////////////////////////////////////////
// findFeatureCenter helper function
/////////////////////////////////////////////////////////////////////////////

PYXIcosIndex findFeatureCenter(PYXPointer<PYXGeometry> geometry)
{
	if (geometry->isEmpty())
	{
		//return null index
		return PYXIcosIndex();
	}

	// Only render an icon if a PYXCell OR a CircleGeometry OR a PYXTileCollection
	// TODO[kabiraman]: ATTENTION!  Adding a check for CircleGeometry and PYXTileCollection is a hack.  
	// The work on styles to associate icons with individual features (the style 
	// process could then be responsible for deciding which feature should icons be rendered for) and the work on 
	// point and multi-point data structure should give us an elegant path forward.  
	// Also, rendering just an icon for a file geo-referenced to a region doesn't make sense.

	if (PYXCircleGeometry * circle = dynamic_cast<PYXCircleGeometry*>(geometry.get()))
	{
		return circle->getCentre();
	}

	if (dynamic_cast<PYXVectorGeometry*>(geometry.get()) || dynamic_cast<PYXVectorGeometry2*>(geometry.get()))
	{
		PYXBoundingCircle circle = geometry->getBoundingCircle();

		int resolution = std::min(PYXMath::knMaxAbsResolution,PYXBoundingCircle::estimateResolutionFromRadius(circle.getRadius())+5);
		//int resolution = geometry->getCellResolution();
		return CmlConvertor::toPYXIcosIndex(circle.getCenter(),resolution);
	}

	if((dynamic_cast<PYXCell*>(geometry.get())) 
		|| (dynamic_cast<PYXTileCollection*>(geometry.get())))
	{
		PYXIcosIndex index = geometry->getIterator()->getIndex();
		return index;
	}

	//return null index - we can't find the feature center
	return PYXIcosIndex();
}

/////////////////////////////////////////////////////////////////////////////
// IconAnnotationsController::IconStyleSkeleton
/////////////////////////////////////////////////////////////////////////////
IconAnnotationsController::IconStyleSkeleton::IconStyleSkeleton(const std::string & style)
	:	m_style(style),
		m_scale(1.0,1.0),
		m_offset(0.0,0.0),
		m_textAlign(0.0,0.0),
		m_textAppearAlways(true)
{
	PYXPointer<CSharpXMLDoc> doc = CSharpXMLDoc::create(style);
	m_iconStyle = doc->getInnerXMLString("/style/Icon/Bitmap");
	m_textField = doc->getNodeText("/style/Icon/TextField");

	if (!m_textField.empty())
	{
		std::string attr = doc->getNodeText("/style/Icon/FontFamily");
		if (!attr.empty())
		{
			m_textAttributes += "Font='" + attr + "' ";
		}

		attr = doc->getNodeText("/style/Icon/FontSize");
		if (!attr.empty())
		{
			m_textAttributes += "Size='" + attr + "' ";
		}

		attr = doc->getNodeText("/style/Icon/FontStyle");
		if (!attr.empty())
		{
			m_textAttributes += "Style='" + attr + "' ";
		}

		attr = doc->getNodeText("/style/Icon/FontColor");
		if (!attr.empty())
		{
			m_textAttributes += "Color='" + attr + "' ";
		}
	}

	m_scale[0] = doc->getAttributeValue<double>("/style/Icon/Scale","Width",1.0);
	m_scale[1] = doc->getAttributeValue<double>("/style/Icon/Scale","Height",1.0);

	m_offset[0] = doc->getAttributeValue<double>("/style/Icon/Offset","X",0.0);
	m_offset[1] = doc->getAttributeValue<double>("/style/Icon/Offset","Y",0.0);

	std::string attrValue = doc->getAttributeValue("/style/Icon/TextAlign","Align");
	if (attrValue.size() > 0)
	{
		if (attrValue == "Left")
		{
			m_textAlign[0] = -1;
		}
		else if (attrValue == "Right")
		{
			m_textAlign[0] = 1;
		}
	}

	attrValue = doc->getAttributeValue("/style/Icon/TextAlign","VAlign");
	if (attrValue.size() > 0)
	{
		if (attrValue == "Top")
		{
			m_textAlign[1] = 1;
		}
		else if (attrValue == "Bottom")
		{
			m_textAlign[1] = -1;
		}
	}

	if (doc->getNodeText("/style/Icon/TextAppearanceMode") == "OnMouseOver")
	{
		m_textAppearAlways = false;
	}

	///Icon Coloring
	std::string defaultColor = doc->getNodeText("/style/Icon/Colour");
	
	if (defaultColor.empty())
	{
		memset(m_color,255,sizeof(m_color));
	}
	else
	{
		PYXValue value = StringUtils::fromString<PYXValue>(defaultColor);
		m_color[0] = value.getUInt8(0);
		m_color[1] = value.getUInt8(1);
		m_color[2] = value.getUInt8(2);
		if (value.getArraySize()==4)
		{
			m_color[3] = value.getUInt8(3);
		}
		else
		{
			m_color[3] = 255;
		}
	}
	m_colorField = doc->getNodeText("/style/Icon/ColourPaletteField");	
	if (!m_colorField.empty())
	{
		std::string paletteString = doc->getNodeText("/style/Icon/ColourPalette");		
		if (paletteString.empty())
		{
			//we don't have a palette, disable custom color.
			m_colorField = "";
		}
		else
		{
			m_colorPalette = PYXValueColorPalette::create(paletteString);
		}
	}
}

void IconAnnotationsController::IconStyleSkeleton::getCustomColor(const boost::intrusive_ptr<IFeature> & feature,unsigned char * color) const
{
	if (!m_colorPalette || !feature)
	{
		getColor(color);
		return;
	}

	boost::intrusive_ptr<IFeatureGroup> group = feature->QueryInterface<IFeatureGroup>();

	if (group)
	{
		PYXPointer<PYXHistogram> histogram = group->getFieldHistogram(feature->getDefinition()->getFieldIndex(m_colorField));

		if (!histogram)
		{
			getColor(color);
			return;
		}

		std::vector<PYXHistogramBin> bins(histogram->getBins());

		float finalColor[4];
		finalColor[0]=0;
		finalColor[1]=0;
		finalColor[2]=0;
		finalColor[3]=0;

		unsigned char minColor[4];
		unsigned char maxColor[4];
		int totalCount = 0;

		for(std::vector<PYXHistogramBin>::iterator it = bins.begin();it != bins.end();++it)
		{
			if (it->count.max == 0)
				continue;

			//TODO: think of something better then the middle of the range - how we deal with uncertainty ?
			int count = it->count.middle();
			totalCount += count;

			if (it->range.single())
			{
				m_colorPalette->convert(it->range.min,minColor,true);
				finalColor[0] += minColor[0]*count;
				finalColor[1] += minColor[1]*count;
				finalColor[2] += minColor[2]*count;
				finalColor[3] += minColor[3]*count;
			}
			else 
			{
				m_colorPalette->convert(it->range.min,minColor,true);
				m_colorPalette->convert(it->range.max,maxColor,true);
				finalColor[0] += (minColor[0]+maxColor[0])*count/2;
				finalColor[1] += (minColor[1]+maxColor[1])*count/2;
				finalColor[2] += (minColor[2]+maxColor[2])*count/2;
				finalColor[3] += (minColor[3]+maxColor[3])*count/2;
			}
		}

		if (totalCount>0)
		{
			color[0] = (unsigned char)(finalColor[0]/totalCount);
			color[1] = (unsigned char)(finalColor[1]/totalCount);
			color[2] = (unsigned char)(finalColor[2]/totalCount);
			color[3] = (unsigned char)(finalColor[3]/totalCount * m_color[3] / 255 );
		}
		else 
		{
			getColor(color);
			return;
		}
	}
	else
	{
		m_colorPalette->convert(feature->getFieldValueByName(m_colorField),color,true);
	}
}

/////////////////////////////////////////////////////////////////////////////
// IconAnnotationsController::StyledIcon
/////////////////////////////////////////////////////////////////////////////

IconAnnotationsController::StyledIcon::StyledIcon(IViewModel * view,const boost::intrusive_ptr<IProcess> & process,const PYXPointer<Surface::Patch> & patch,const std::string & style,const PYXPointer<IconAnnotationsController::IconAnnotationSkeleton> & skeleton,const PYXPointer<IconAnnotationsController::IconStyle> & iconStyle)
	:	Annotation(view,process,style,skeleton->getFeatureID(),skeleton->getGroupID()),
		m_iconStyle(iconStyle),
		m_skeleton(skeleton),
		m_patch(patch),
		m_location(skeleton->getLocation()),
		m_textGenerated(false),
		m_colorCalculated(false)
{
	if (m_iconStyle->hasText() && m_iconStyle->getTextAppearAlways())
	{
		generateText();
	}

	if (m_iconStyle->hasCustomColor())
	{
		m_colorBackgroundTask = PYXTaskWithContinuation::start(boost::bind(&IconAnnotationsController::StyledIcon::generateCustomColorFromFeature,PYXPointer<StyledIcon>(this)));
	}
	else
	{
		m_iconStyle->getColor(m_color);		
		m_colorCalculated = true;
	}
}

IconAnnotationsController::StyledIcon::~StyledIcon()
{
}

bool IconAnnotationsController::StyledIcon::generateText()
{
	if (m_textGenerated)
	{
		m_backgroundTask.reset();
		return true;
	}

	if (!m_backgroundTask)
	{
		m_backgroundTask = PYXTaskWithContinuation::start(boost::bind(&IconAnnotationsController::StyledIcon::generateTextFromFeature,PYXPointer<StyledIcon>(this)));
	}
	return false;
}

void IconAnnotationsController::StyledIcon::generateCustomColorFromFeature(PYXPointer<StyledIcon> icon)
{
	try
	{
		icon->m_iconStyle->getCustomColor(icon->getFeature(),icon->m_color);
		icon->m_colorCalculated = true;
	}
	catch(PYXException& e)
	{
		TRACE_ERROR("Failed to generate icon color for icon with feature ID: " << icon->m_skeleton->getFeatureID() << " : " << e.getLocalizedErrorString());
	}
	catch(...)
	{
		TRACE_ERROR("Failed to generate icon color for icon with feature ID: " << icon->m_skeleton->getFeatureID());
	}

	icon->m_colorBackgroundTask.reset();
}

void IconAnnotationsController::StyledIcon::generateTextFromFeature(PYXPointer<StyledIcon> icon)
{
	try
	{
		auto feature = icon->getFeature();
		auto group = feature->QueryInterface<IFeatureGroup>();

		if (!group)
		{
			icon->m_textStyle = icon->m_iconStyle->getTextBitmapDefinition(feature);
		}
		else
		{
			icon->m_textStyle = icon->m_iconStyle->getTextBitmapDefinitionForString(StringUtils::toString(group->getFeaturesCount().max)+" features");
		}

		icon->m_textGenerated = true;
	}
	catch(...)
	{
	}

	icon->m_backgroundTask.reset();
}

PYXPointer<PackedTextureItem> IconAnnotationsController::StyledIcon::getTextTextureItem()
{
	if (!m_textTexture)
	{
		if (!generateText())
		{
			return 0;
		}

		m_textTexture = m_iconStyle->getIconRenderer().getTextureFromBitmapDefinition(m_textStyle);
	}

	return m_textTexture;
}

void IconAnnotationsController::StyledIcon::onMouseEnter(PYXPointer<AnnotationMouseEvent> eventData)
{
	if (m_iconStyle->hasText() && ! m_iconStyle->getTextAppearAlways() )
	{
		generateText();
		setDynamicVisualization(true);
	}
}

void IconAnnotationsController::StyledIcon::onMouseLeave(PYXPointer<AnnotationMouseEvent> eventData)
{
	if (m_iconStyle->hasText() && ! m_iconStyle->getTextAppearAlways() )
	{
		setDynamicVisualization(false);
		if (m_textIcon)
		{
			m_iconStyle->removeIcon(m_textIcon);
			m_textIcon = NULL;
		}
		m_textTexture = NULL;
	}
}

bool IconAnnotationsController::StyledIcon::wasVisualizationGenerated()
{
	return m_icon || m_textIcon;
}

bool IconAnnotationsController::StyledIcon::canGenerateVisualization()
{
	if (m_iconStyle->hasIcon() )
	{
		if (!m_iconStyle->getIconTexture())
		{
			return false;
		}
	}

	if (m_iconStyle->hasText() && m_iconStyle->getTextAppearAlways() )
	{
		if (!getTextTextureItem())
		{
			return false;
		}
	}

	if (m_iconStyle->hasCustomColor() && !m_colorCalculated)
	{
		return false;
	}

	return true;
}

void IconAnnotationsController::StyledIcon::generateVisualization()
{
	try
	{
		vec2 textOffset(0.0,0.0);

		if (m_iconStyle->hasIcon())
		{
			PYXPointer<PackedTextureItem> iconTexture = m_iconStyle->getIconTexture();
			m_icon = m_iconStyle->addIcon(iconTexture,m_patch,m_location,m_iconStyle->getIconScale(),m_iconStyle->getIconOffset(),this);

			if (m_skeleton->isGroup())
			{
				m_icon->setScale(m_icon->getScale()*2);
			}

			//compute the text icon offset which is the icon size * textAlign(-1,0 or 1).
			textOffset[0] = iconTexture->getWidth()  * m_icon->getScale()[0] * m_iconStyle->getTextAlign()[0];
			textOffset[1] = iconTexture->getHeight() * m_icon->getScale()[1] * m_iconStyle->getTextAlign()[1];		
		
			m_icon->setColor(m_color);
		}

		if (m_iconStyle->hasText() && m_iconStyle->getTextAppearAlways() )
		{
			PYXPointer<PackedTextureItem> textTexture = getTextTextureItem();

			textOffset[0] += textTexture->getWidth() * m_iconStyle->getTextAlign()[0] / 2;
			textOffset[1] += textTexture->getHeight() * m_iconStyle->getTextAlign()[1] / 2;

			m_textIcon = m_iconStyle->addIcon(getTextTextureItem(),m_patch,m_location,vec2(1.0,1.0),textOffset,this);
		}

		validateElevation();
	}
	catch(...)
	{
		TRACE_INFO("not good");
	}
}

void IconAnnotationsController::StyledIcon::destroyVisualization()
{
	if (m_icon)
	{
		m_iconStyle->removeIcon(m_icon);
		m_icon = NULL;
	}

	if (m_textIcon)
	{
		m_iconStyle->removeIcon(m_textIcon);
		m_textIcon = NULL;
	}
	m_textTexture.reset();
}

void IconAnnotationsController::StyledIcon::updateVisualization()
{
	if (!m_textIcon)
	{
		PYXPointer<PackedTextureItem> textTexture = getTextTextureItem();

		if (textTexture)
		{
			vec2 textOffset(0.0,0.0);

			if (m_iconStyle->hasIcon())
			{
				PYXPointer<PackedTextureItem> iconTexture = m_iconStyle->getIconTexture();

				//compute the text icon offset which is the icon size * textAlign(-1,0 or 1).
				textOffset[0] = iconTexture->getWidth()  * m_iconStyle->getIconScale()[0] * m_iconStyle->getTextAlign()[0];
				textOffset[1] = iconTexture->getHeight() * m_iconStyle->getIconScale()[1] * m_iconStyle->getTextAlign()[1];
			}

			textOffset[0] += textTexture->getWidth() * m_iconStyle->getTextAlign()[0] / 2;
			textOffset[1] += textTexture->getHeight() * m_iconStyle->getTextAlign()[1] / 2;

			m_textIcon = m_iconStyle->addIcon(textTexture,m_patch,m_location,vec2(1.0,1.0),textOffset);

			setDynamicVisualization(false);
		}
	}
}

void IconAnnotationsController::StyledIcon::validateElevation()
{
	double elevation;

	if (wasVisualizationGenerated())
	{
		elevation = m_iconStyle->getIconRenderer().getViewThread().getMeshElevation(m_patch,m_skeleton->getUV());

		if (m_icon)
		{
			m_icon->setLocationElevation(elevation);
			m_location = m_icon->getLocation();
		}

		if (m_textIcon)
		{
			m_textIcon->setLocationElevation(elevation);
			m_location = m_textIcon->getLocation();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// IconAnnotationsController::LoadingThread
/////////////////////////////////////////////////////////////////////////////

IconAnnotationsController::LoadingThread::LoadingThread(const boost::intrusive_ptr<IProcess> & process, IconAnnotationsController * controller)
	:	m_process(process), 
		m_controller(controller),
		m_invalidateWhenReady(false)
{
	TRACE_INFO("IconAnnotationsController::LoadingThread created");
	m_selector = IconAnnotationsController::IFeaturesSelector::create(process);

	m_globalStyle = process->getOutput()->QueryInterface<IFeature>()->getStyle();

	m_lastInvalidateTime = boost::posix_time::microsec_clock::local_time();
}

IconAnnotationsController::LoadingThread::~LoadingThread()
{
	m_thread.stop();
	TRACE_INFO("IconAnnotationsController::LoadingThread deleted");
}

	
PYXPointer<IconAnnotationsController::LoadingThread> IconAnnotationsController::LoadingThread::create(	const boost::intrusive_ptr<IProcess> & process, 
																										IconAnnotationsController * controller)
{
	boost::intrusive_ptr<IFeatureCollection> fc = process->QueryInterface<IFeatureCollection>();

	if (!fc)
	{
		return PYXPointer<LoadingThread>();
	}

	//check that the feature collection has IconStyle applyed
	PYXPointer<CSharpXMLDoc> doc = CSharpXMLDoc::create(fc->getStyle());

	if (!doc->getNodeText("/style/Icon/Bitmap").empty() ||
		!doc->getNodeText("/style/Icon/TextField").empty())
	{
		return PYXNEW(LoadingThread,process,controller);
	}
	else
	{
		return PYXPointer<LoadingThread>();
	}
}

int IconAnnotationsController::LoadingThread::getWaitingJobsCount()
{
	return m_thread.getWaitingJobsCount();
}

void IconAnnotationsController::LoadingThread::addPatch(const PYXPointer<Surface::Patch> & patch)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_thread.addJob(patch,boost::bind(&LoadingThread::loadPatch,this,patch));	
	m_neededPatches.insert(patch);	
}

void IconAnnotationsController::LoadingThread::addPatches(const std::set<PYXPointer<Surface::Patch>> & patches)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	std::vector<std::pair<PYXPointer<Surface::Patch>,boost::function<void(void)>>> jobs;
	jobs.reserve(patches.size());

	for(auto & patch : patches)
	{
		m_neededPatches.insert(patch);
		jobs.push_back(std::make_pair(patch,boost::bind(&LoadingThread::loadPatch,this,patch)));
	}
	
	m_thread.addJobs(jobs);	
}


void IconAnnotationsController::LoadingThread::removePatch(const PYXPointer<Surface::Patch> & patch)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);		
	m_thread.cancelJobs(patch);
	m_neededPatches.erase(patch);
}

void IconAnnotationsController::LoadingThread::invalideAll()
{
	//TODO: maybe there is some efficent way to do so?
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_thread.cancelAllJobs();	
	for(NeededPatchesSet::iterator it = m_neededPatches.begin(); it != m_neededPatches.end(); ++it)
	{
		m_thread.addJob(*it,boost::bind(&LoadingThread::loadPatch,this,*it));
	}
}

void IconAnnotationsController::LoadingThread::loadPatch(PYXPointer<Surface::Patch> patch)
{
	try
	{		
		PYXPointer<IconAnnotationSkeletonPatch> annotations = generateAnnotations(patch);

		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);
			if (m_controller != 0)
			{
				m_controller->onNewAnnotations(patch,m_process,annotations);
			}
		}		
	}
	catch(PYXException&)
	{
		TRACE_ERROR("Failed to generate icon annotation for " << this->getProcess()->getProcName());
	}

	//reload all annotations if cache is not fully popolated yet
	if (m_thread.getWaitingJobsCount() == 0)
	{
		if (m_selector->isLoading())
		{
			boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();

			boost::posix_time::time_duration td = now - m_lastInvalidateTime;

			if (static_cast<int>(td.total_milliseconds()) > 1000)
			{
				invalideAll();
				m_lastInvalidateTime = now;
			}
		}
		else if (!m_invalidateWhenReady)
		{
			m_invalidateWhenReady = true;
			invalideAll();
		}
	}
}

PYXPointer<IconAnnotationsController::IconAnnotationSkeletonPatch> IconAnnotationsController::LoadingThread::generateAnnotations(const PYXPointer<Surface::Patch> & patch)
{
	PYXPointer<IconAnnotationSkeletonPatch> skeletons = IconAnnotationSkeletonPatch::create();

	//TRACE_INFO("Start generating patch: " << patch->getKey().toString());
	//int count = 0;

	for(int u=0;u<4;u++)
	{
		for (int v=0;v<4;v++)
		{
			PYXIcosIndex index = patch->getRhombus().getSubIndex(u,v,2);

			std::auto_ptr<IFeaturesSelector::SelectedFeatureInfo> spF = m_selector->getFeatureToVisualizeForCell(index);

			if (spF.get()!=0)
			{
				PYXPointer<IconAnnotationSkeleton> annotationSkeleton = createAnnotationSkeleton(patch,spF);

				if (annotationSkeleton)
				{
					//skeletons->addAnnotationSkeleton(u,v,annotationSkeleton);
					skeletons->addAnnotationSkeleton(u*3,v*3,annotationSkeleton);

					//count++;
				}
			}
		}
	}

	//TRACE_INFO("Done generating patch: " << patch->getKey().toString() << " with " << count << " icons");


	return skeletons;
}

PYXIcosIndex IconAnnotationsController::LoadingThread::findFeatureCenter(PYXPointer<PYXGeometry> geometry)
{
	return ::findFeatureCenter(geometry);
}

PYXPointer<IconAnnotationsController::IconAnnotationSkeleton> IconAnnotationsController::LoadingThread::createAnnotationSkeleton(
	const PYXPointer<Surface::Patch> & patch,
	const std::auto_ptr<IconAnnotationsController::IFeaturesSelector::SelectedFeatureInfo> & featureInfo)
{
	PYXIcosIndex index = CmlConvertor::toPYXIcosIndex(featureInfo->m_location,std::min(PYXMath::knMaxAbsResolution,patch->getRhombus().getIndex(0).getResolution()+2));

	if (!index.isNull())
	{
		if ((index.getResolution()-patch->getRhombus().getIndex(0).getResolution()) % 2 != 0)
		{
			index.incrementResolution();
		}

		double u,v;
		if (patch->getRhombus().isInside(index,&u,&v))
		{
			return IconAnnotationSkeleton::create(featureInfo->m_featureID,featureInfo->m_groupID,getStyleSkeleton(m_globalStyle),vec2(u,v),CmlConvertor::toVec3(featureInfo->m_location),featureInfo->m_isGroup);
		}
	}
	return PYXPointer<IconAnnotationSkeleton>();
}

PYXPointer<IconAnnotationsController::IconStyleSkeleton> IconAnnotationsController::LoadingThread::getStyleSkeleton(const std::string & style)
{
	StyleCache::iterator it = m_styleCache.find(style);
	if (it != m_styleCache.end())
	{
		return it->second;
	}
	else
	{
		return m_styleCache[style] = IconStyleSkeleton::create(style);
	}
}

void IconAnnotationsController::LoadingThread::attach(IconAnnotationsController & controller)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_controller = &controller;
}

void IconAnnotationsController::LoadingThread::detach(IconAnnotationsController & controller)
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	m_controller = 0;
}

/////////////////////////////////////////////////////////////////////////////
// IconAnnotationsController
/////////////////////////////////////////////////////////////////////////////

IconAnnotationsController::IconAnnotationsController(ViewOpenGLThread & viewThread) : Component(viewThread), m_iconRenderer(0)
{
	getViewThread().getViewPortProcessChangeNotifier().attach(this,&IconAnnotationsController::updateLoaders);
}

IconAnnotationsController::~IconAnnotationsController()
{
	//stop all threads before destorying the object
	for(LoadingThreadsMap::iterator it = m_loadingThreads.begin(); it != m_loadingThreads.end(); ++it)
	{
		it->second->detach(*this);
		GarbageCollector::getInstance()->collect(it->second);
	}
	m_loadingThreads.clear();
}

void IconAnnotationsController::setIconRenderer(const PYXPointer<IconRenderer> & iconRenderer)
{
	m_iconRenderer = iconRenderer;
}

void IconAnnotationsController::render()
{
	PerformanceCounter::getTimePerformanceCounter("Start IconAnnotationsController",0.5f,0.5f,1.0f)->makeMeasurement();

	for(IconAnnotationPatchMap::iterator it = m_visiblePatches.begin(); it != m_visiblePatches.end(); ++it)
	{
		if (!it->second)
		{
			continue;
		}

		PatchAnnotations & annotations = *(it->second->getAnnotations());

		if (annotations.getAnnotationsCount()>0)
		{
			annotations.generateAnnotations();

			annotations.updateAnnotations();
		}
	}	

	PerformanceCounter::getTimePerformanceCounter("start IconAnnotationsController::prepare tiles",0.5f,0.5f,1.0f)->makeMeasurement();

	startLoadersTasks();

	PerformanceCounter::getTimePerformanceCounter("End IconAnnotationsController",0.5f,0.5f,1.0f)->makeMeasurement();

	getViewThread().getView().setStreamingProgress("icons",getLoadingProgress());

	getViewThread().setFrameTimeMeasurement("setup-icons");
}

int IconAnnotationsController::getLoadingProgress() {
	// there is no loading threads - there is no icons to display
	if (m_loadingThreads.empty()) 
	{
		return 100;
	}

	//start from all loaded
	auto progress = 100.0;
	
	//find out the impact for every loading thread (in perecnt)
	auto threadImpact = 100.0/m_loadingThreads.size();
	
	//find out how much patches we need to load.
	auto neededPatches = std::max(m_visiblePatches.size(),1u);
	
	for (auto & loadingThread : m_loadingThreads)
	{
		//find how many waiting jobs each thread have...
		auto waitingJobs = loadingThread.second->getWaitingJobsCount();		

		if (waitingJobs > 0)
		{
			//remove this thread remaing effort from overall progress
			progress -= std::min(1.0,1.0*waitingJobs / neededPatches) * threadImpact;
		}
	}
	return (int)progress;
}

void IconAnnotationsController::startLoadersTasks()
{
	if (m_neededPatches.empty())
	{
		return;
	}

	for(auto & it : m_loadingThreads)
	{
		it.second->addPatches(m_neededPatches);	
	}
	m_neededPatches.clear();
}

void IconAnnotationsController::updateLoaders(PYXPointer<NotifierEvent> e)
{
	//the surface has been changed - reset everything...
	if (m_surface != getViewThread().getSurface())
	{
		if (m_surface)
		{
			m_surface->getPatchBecomeVisible().detach(this,&IconAnnotationsController::onPatchBecomeVisible);
			m_surface->getPatchBecomeNotVisible().detach(this,&IconAnnotationsController::onPatchBecomeHidden);
		}

		for(auto & it : m_loadingThreads)
		{
			it.second->detach(*this);
			GarbageCollector::getInstance()->collect(it.second);
		}
		m_loadingThreads.clear();
		m_visiblePatches.clear();

		m_surface = getViewThread().getSurface();

		m_surface->getPatchBecomeVisible().attach(this,&IconAnnotationsController::onPatchBecomeVisible);
		m_surface->getPatchBecomeNotVisible().attach(this,&IconAnnotationsController::onPatchBecomeHidden);
	}

	//check for vectors pipelines modifications...
	LoadingThreadsMap newLoaders;

	std::vector<boost::intrusive_ptr<IProcess>> vectors = getViewThread().getViewPointProcess()->QueryInterface<IViewPoint>()->getFeatureCollectionPipelines();

	for(std::vector<boost::intrusive_ptr<IProcess>>::iterator processIt = vectors.begin();processIt != vectors.end(); ++processIt)
	{
		if (m_loadingThreads.find(*processIt) != m_loadingThreads.end())
		{
			newLoaders[*processIt] = m_loadingThreads[*processIt];
		}
		else
		{
			//we have new process
			PYXPointer<LoadingThread> newThread = LoadingThread::create(*processIt,this);

			//if we found that the style is good enough for generating annoations
			if (newThread)
			{
				newLoaders[*processIt] = newThread;

				//make sure it will load everything...
				for(IconAnnotationPatchMap::iterator it = m_visiblePatches.begin(); it != m_visiblePatches.end(); ++it)
				{
					newThread->addPatch(it->first);
				}
			}
		}

		IconStyleCacheMap::iterator styleCacheIt = m_stylesCache.find(*processIt);

		if (styleCacheIt == m_stylesCache.end())
		{
			m_stylesCache.insert(std::make_pair(*processIt,IconStyleCache()));
		}
	}

	for(auto & it : m_loadingThreads)
	{
		if (newLoaders.find(it.first) == newLoaders.end())
		{
			GarbageCollector::getInstance()->collect(it.second);
			removeAllAnnotations(it.first);

			IconStyleCacheMap::iterator styleCacheIt = m_stylesCache.find(it.first);

			if (styleCacheIt != m_stylesCache.end())
			{
				for(auto & style : styleCacheIt->second)
				{
					style.second->clearOpenGLResources();
				}
				m_stylesCache.erase(styleCacheIt);
			}
		}
	}

	std::swap(m_loadingThreads,newLoaders);

	GarbageCollector::getInstance()->startDestroyObjects();
}

void IconAnnotationsController::onPatchBecomeVisible(PYXPointer<NotifierEvent> e)
{
	Surface::Event * surfaceEvent = dynamic_cast<Surface::Event*>(e.get());

	if (surfaceEvent == 0)
	{
		return;
	}

	PYXPointer<Surface::Patch> patch = surfaceEvent->getPatch();

	PYXPointer<IconAnnotationPatch> annotations = IconAnnotationPatch::create();
	m_visiblePatches[patch] = annotations;
	
	borrowIcons(patch);
	m_neededPatches.insert(patch);
}

void IconAnnotationsController::borrowIcons(const PYXPointer<Surface::Patch> & patch)
{
	//get parent patch annotations
	PYXPointer<IconAnnotationPatch> parentAnnotations;
	int uoffset;
	int voffset;
	if (patch->getParent())
	{
		parentAnnotations = m_visiblePatches[patch->getParent()];

		uoffset = (patch->getIndex() % 3);
		voffset = (patch->getIndex() / 3);
	}

	if (!parentAnnotations)
	{
		return;
	}

	for(auto & it : m_loadingThreads)
	{		
		IconAnnotationPatch::IconAnnotationSkeletonPatchMap::iterator skeletonIt = parentAnnotations->getSkeletons().find(ProcRef(it.first));

		if (skeletonIt != parentAnnotations->getSkeletons().end())
		{
			PYXPointer<IconAnnotationSkeletonPatch> borrowedSkeletons = IconAnnotationSkeletonPatch::create();
			int borrowedCount = 0;

			for(int u=0;u<4;u++)
			{
				for(int v=0;v<4;v++)
				{
					PYXPointer<IconAnnotationSkeleton> skeleton = skeletonIt->second->getAnnotationSkeleton(u+uoffset*3,v+voffset*3);

					if (skeleton)
					{
						vec2 uv = skeleton->getUV();
						uv[0] = uv[0]*3-uoffset;
						uv[1] = uv[1]*3-voffset;
						//if this annotation is inside the patch... borrow it
						if (uv[0]>=0 && uv[0]<=1 && uv[1] >=0 && uv[1]<=1)
						{
							skeleton = IconAnnotationSkeleton::create(skeleton->getFeatureID(),skeleton->getGroupID(),skeleton->getStyleSkeleton(),uv,skeleton->getLocation(),skeleton->isGroup());
							borrowedSkeletons->addAnnotationSkeleton(u*3,v*3,skeleton);
							borrowedCount++;
						}
					}
				}
			}
			if (borrowedCount>0)
			{
				addNewAnnotations(patch,it.first,borrowedSkeletons);
			}
		}
	}	
}

void IconAnnotationsController::onPatchBecomeHidden(PYXPointer<NotifierEvent> e)
{
	Surface::Event * surfaceEvent = dynamic_cast<Surface::Event*>(e.get());

	if (surfaceEvent == 0)
	{
		return;
	}

	for(auto & it : m_loadingThreads)
	{
		it.second->removePatch(surfaceEvent->getPatch());
	}

	IconAnnotationPatchMap::iterator it = m_visiblePatches.find(surfaceEvent->getPatch());

	if (it == m_visiblePatches.end())
	{
		//the patch is no longer exsits
		return;
	}

	if (it->second)
	{
		it->second->getAnnotations()->destroyAnnotations();
	}

	m_visiblePatches.erase(it);	
	m_neededPatches.erase(surfaceEvent->getPatch());	
}

void IconAnnotationsController::removeAllAnnotations(const boost::intrusive_ptr<IProcess> & process)
{
	ProcRef procRef(process);
	for(IconAnnotationPatchMap::iterator it = m_visiblePatches.begin(); it != m_visiblePatches.end(); ++it)
	{
		if (it->second)
		{
			it->second->getAnnotations()->removeAllAnnotationOfPipeline(procRef);
			it->second->getSkeletons().erase(procRef);
		}
	}
}

void IconAnnotationsController::onNewAnnotations(	const PYXPointer<Surface::Patch> & patch, 
													const boost::intrusive_ptr<IProcess> & process,
													const PYXPointer<IconAnnotationSkeletonPatch> & newProcessAnnotations)
{
	schedulePostProcessing(&IconAnnotationsController::addNewAnnotations,patch,process,newProcessAnnotations);
}

void IconAnnotationsController::addNewAnnotations(	PYXPointer<Surface::Patch> patch, 
													boost::intrusive_ptr<IProcess> process,
													PYXPointer<IconAnnotationSkeletonPatch> newProcessAnnotations)
{
	Performance::ScopedTimer funcTime("IconAnnotationsController::addNewAnnotations",0.10);

	IconAnnotationPatchMap::iterator it = m_visiblePatches.find(patch);

	if (it == m_visiblePatches.end() || !it->second)
	{
		//the patch is no longer exists
		return;
	}

	IconAnnotationPatch & annotations = *(it->second);

	ProcRef procRef(process);

	annotations.getAnnotations()->removeAllAnnotationOfPipeline(procRef);

	annotations.getSkeletons()[procRef] = newProcessAnnotations;

	for(int u=0;u<10;u++)
	{
		for(int v=0;v<10;v++)
		{
			PYXPointer<IconAnnotationSkeleton> skeleton = newProcessAnnotations->getAnnotationSkeleton(u,v);

			if (skeleton)
			{
				annotations.getAnnotations()->addAnnotation(createAnnotation(patch,process,skeleton));
			}
		}
	}
}

PYXPointer<Annotation> IconAnnotationsController::createAnnotation(	const PYXPointer<Surface::Patch> & patch, 
																	const boost::intrusive_ptr<IProcess> & process,
																	const PYXPointer<IconAnnotationSkeleton> & skeleton )
{
	IconStyleCache & cache = m_stylesCache[process];
	const std::string & style = skeleton->getStyleSkeleton()->getStyle();
	IconStyleCache::iterator cacheIt = cache.find(style);
	PYXPointer<IconStyle> iconStyle;
	if (cacheIt == cache.end())
	{
		iconStyle = cache[style] = IconStyle::create(*m_iconRenderer,skeleton->getStyleSkeleton());
	}
	else
	{
		iconStyle = cacheIt->second;
	}

	return StyledIcon::create(getViewThread().getViewHandle().get(),process,patch,style,skeleton,iconStyle);
}

/////////////////////////////////////////////////////////////////////////////
// GroupFeatureSelector
/////////////////////////////////////////////////////////////////////////////

class GroupFeatureSelector : public IconAnnotationsController::IFeaturesSelector
{
public:
	static PYXPointer<GroupFeatureSelector> create(const boost::intrusive_ptr<IFeatureGroup> & fg)
	{
		return PYXNEW(GroupFeatureSelector,fg);
	}

	GroupFeatureSelector(const boost::intrusive_ptr<IFeatureGroup> & fg) : m_group(fg), m_iconsCount(0)
	{
		TRACE_INFO("GroupFeatureSelector created");
		m_annotationCache = AnnotationCache::create();

		PYXPointer<CSharpXMLDoc> doc = CSharpXMLDoc::create(fg->getStyle());

		initPopulatedGroups();
	}

	virtual ~GroupFeatureSelector()
	{
		TRACE_INFO("GroupFeatureSelector deleted");
	}

	virtual std::auto_ptr<SelectedFeatureInfo> getFeatureToVisualizeForCell(const PYXIcosIndex & index)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		PYXCell cell(index);
		PYXBoundingCircle circle(CmlConvertor::toPYXCoord3D(index),PYXIcosMath::UnitSphere::calcCellCircumRadius(index));

		if (m_iconsCount > 20000)
		{
			m_annotationCache = AnnotationCache::create();
			m_iconsCount = 0;

			initPopulatedGroups(true);
		}

		popualteGroupsAsNeeded(m_populatedGroup,index,circle);

		const std::list<AnnotationCache::AnnotationInfo> & cachedAnnotations = m_annotationCache->getAnnotation(index);

		if (cachedAnnotations.size() > 0)
		{
			return std::auto_ptr<SelectedFeatureInfo>(new SelectedFeatureInfo(cachedAnnotations.front()));
		}

		return std::auto_ptr<SelectedFeatureInfo>();
	}

	virtual bool isLoading() const 
	{
		return false;
	}

private:
	struct PendingGroup
	{
		boost::intrusive_ptr<IFeatureGroup> group;
		PYXBoundingCircle circle;
		int resolution;
	};

	struct PopulatedGroup
	{
		boost::intrusive_ptr<IFeatureGroup> group;
		PYXBoundingCircle circle;
		int resolution;
		bool populated;
		std::list<PopulatedGroup> subGroups;
	};

private:
	void initPopulatedGroups(bool forcePopulate = false)
	{
		m_populatedGroup.resolution = 2;
		m_populatedGroup.circle = PYXBoundingCircle::global();
		m_populatedGroup.group = m_group;
		m_populatedGroup.populated = false;
		m_populatedGroup.subGroups.clear();

		if (forcePopulate)
		{
			populateGroup(m_populatedGroup);
		}
	}

	void popualteGroupsAsNeeded(PopulatedGroup & group,const PYXIcosIndex & index,PYXBoundingCircle & circle)
	{
		if (index.getResolution() < group.resolution || ! circle.intersects(group.circle))
		{
			return;
		}

		if (!group.populated)
		{
			populateGroup(group);
		}

		for(std::list<PopulatedGroup>::iterator it = group.subGroups.begin(); it != group.subGroups.end();  ++it)
		{
			popualteGroupsAsNeeded(*it,index,circle);
		}
	}

	void populateGroup(PopulatedGroup & popGroup)
	{
		int topResolution = popGroup.resolution;
		PYXPointer<FeatureIterator> iterator = popGroup.group->getGroupIterator();

		for(; !iterator->end(); iterator->next())
		{
			boost::intrusive_ptr<IFeature> feature = iterator->getFeature();

			boost::intrusive_ptr<IFeatureGroup> group = feature->QueryInterface<IFeatureGroup>();

			if (group)
			{
				popGroup.subGroups.push_back(PopulatedGroup());
				PopulatedGroup & pendingGroup = popGroup.subGroups.back();
				pendingGroup.group = group;
				pendingGroup.circle = group->getGeometry()->getBoundingCircle();
				pendingGroup.resolution = group->getGeometry()->getCellResolution()-3;
				pendingGroup.populated = false;

				PYXIcosIndex index = CmlConvertor::toPYXIcosIndex(pendingGroup.circle.getCenter(),pendingGroup.resolution);
				if (index.getResolution()>2)
				{
					index.setResolution(std::max(2,index.getResolution()-2));
					PYXIcosIndex topIndex = index;

					topIndex.setResolution(topResolution);
					m_annotationCache->addAnnotation(index,topIndex,AnnotationCache::AnnotationInfo("",group->getID(),pendingGroup.circle.getCenter(),group->getFeaturesCount().max,true));
					m_iconsCount++;
				}
			}
			else 
			{
				//should be nothing or small list of cells
				PYXPointer<PYXGeometry> annotationGeometry = preprocessFeature(feature);

				if (annotationGeometry)
				{
					boost::recursive_mutex::scoped_lock lock(m_mutex);

					PYXPointer<PYXIterator> iterator = annotationGeometry->getIterator();

					//add annotation to all cells - this allow the annotation to appear at several locations
					while(!iterator->end())
					{
						PYXIcosIndex topIndex = iterator->getIndex();
						topIndex.setResolution(topResolution);
						m_annotationCache->addAnnotation(iterator->getIndex(),topIndex,AnnotationCache::AnnotationInfo(feature->getID(),popGroup.group->getID(),CmlConvertor::toPYXCoord3D(iterator->getIndex()),1));
						iterator->next();
						m_iconsCount++;
					}
				}
			}
		}

		popGroup.populated = true;
	}

	PYXPointer<PYXGeometry> preprocessFeature(boost::intrusive_ptr<IFeature> feature)
	{
		PYXPointer<PYXGeometry> spGeom = feature->getGeometry();

		PYXIcosIndex index = findFeatureCenter(spGeom);

		if (!index.isNull())
		{
			return PYXCell::create(index);
		}

		return PYXPointer<PYXGeometry>();
	}

	PYXIcosIndex findFeatureCenter(PYXPointer<PYXGeometry> geometry)
	{
		return ::findFeatureCenter(geometry);
	}

private:
	//! AnnotationCache for aggregating the annotations (TODO: move this into a prcoess) 
	PYXPointer<AnnotationCache> m_annotationCache;

	boost::intrusive_ptr<IFeatureGroup> m_group;

	PopulatedGroup m_populatedGroup;

	//! list of all groups not populated yet..
	std::list<PendingGroup> m_pendingGroups;

	int m_iconsCount;

	boost::recursive_mutex m_mutex;
};


/////////////////////////////////////////////////////////////////////////////
// FeatureCollectionSelector
/////////////////////////////////////////////////////////////////////////////

class FeatureCollectionSelector : public IconAnnotationsController::IFeaturesSelector
{
public:
	static PYXPointer<FeatureCollectionSelector> create(const boost::intrusive_ptr<IFeatureCollection> & fc)
	{
		return PYXNEW(FeatureCollectionSelector,fc);
	}

	FeatureCollectionSelector(const boost::intrusive_ptr<IFeatureCollection> & fc) : m_spFeatures(fc), m_annotationCachePopolated(false)
	{
		m_annotationCache = AnnotationCache::create();
		m_spCacheFeatureIterator = m_spFeatures->getIterator();
	}

	virtual std::auto_ptr<SelectedFeatureInfo> getFeatureToVisualizeForCell(const PYXIcosIndex & index)
	{
		if (!m_annotationCachePopolated)
		{
			populateCache();
		}

		const std::list<AnnotationCache::AnnotationInfo> & cachedAnnotations = m_annotationCache->getAnnotation(index);

		if (cachedAnnotations.size() > 0)
		{
			return std::auto_ptr<SelectedFeatureInfo>(new SelectedFeatureInfo(cachedAnnotations.front()));
		}

		return std::auto_ptr<SelectedFeatureInfo> ();
	}

	virtual bool isLoading() const 
	{
		return !m_annotationCachePopolated;
	}


private:
	void populateCache()
	{
		int tryCount = 0;

		//try to add 1000 features every time...
		while(!m_spCacheFeatureIterator->end() && tryCount++ < 100 )
		{
			//should be nothing or small list of cells
			PYXPointer<PYXGeometry> annotationGeometry = preprocessFeature(m_spCacheFeatureIterator->getFeature());

			if (annotationGeometry)
			{
				PYXPointer<PYXIterator> iterator = annotationGeometry->getIterator();

				//add annotation to all cells - this allow the annotation to appear at several locations
				while(!iterator->end())
				{
					PYXIcosIndex topIndex = iterator->getIndex();
					//topIndex.setResolution(std::max(2,topIndex.getResolution()-5));
					topIndex.setResolution(2);
					m_annotationCache->addAnnotation(iterator->getIndex(),topIndex,AnnotationCache::AnnotationInfo(m_spCacheFeatureIterator->getFeature()->getID(),CmlConvertor::toPYXCoord3D(iterator->getIndex())));
					iterator->next();
				}
			}

			m_spCacheFeatureIterator->next();
		}

		if (m_spCacheFeatureIterator->end())
		{
			m_annotationCachePopolated = true;
		}
	}

	PYXPointer<PYXGeometry> preprocessFeature(boost::intrusive_ptr<IFeature> feature)
	{
		PYXPointer<PYXGeometry> spGeom = feature->getGeometry();

		PYXIcosIndex index = findFeatureCenter(spGeom);

		if (!index.isNull())
		{
			return PYXCell::create(index);
		}

		return PYXPointer<PYXGeometry>();
	}

	PYXIcosIndex findFeatureCenter(PYXPointer<PYXGeometry> geometry)
	{
		return ::findFeatureCenter(geometry);
	}

private:
	//! AnnotationCache for aggregating the annotations (TODO: move this into a prcoess) 
	PYXPointer<AnnotationCache> m_annotationCache;
	bool						m_annotationCachePopolated;

	//! used to popolate AnnotationCache
	boost::intrusive_ptr<IFeatureCollection>	m_spFeatures;

	//! used to popolate AnnotationCache
	PYXPointer<FeatureIterator>					m_spCacheFeatureIterator;
};


PYXPointer<IconAnnotationsController::IFeaturesSelector> IconAnnotationsController::IFeaturesSelector::create(const boost::intrusive_ptr<IProcess> & process)
{
	boost::intrusive_ptr<PYXCOM_IUnknown> output = process->getOutput();

	boost::intrusive_ptr<IFeatureGroup> group = output->QueryInterface<IFeatureGroup>();

	if (group)
	{
		return GroupFeatureSelector::create(group);
	}

	boost::intrusive_ptr<IFeatureCollection> collection = output->QueryInterface<IFeatureCollection>();

	if (collection)
	{
		return FeatureCollectionSelector::create(collection);
	}

	PYXTHROW(PYXException,"Can't find valid output for generating icon from");
};