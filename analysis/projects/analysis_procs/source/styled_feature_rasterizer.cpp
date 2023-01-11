/******************************************************************************
styled_feature_rasterizer.cpp

begin		: 2010-11-17
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_ANALYSIS_PROCS_SOURCE

//Enable this to trace diagnostic profing on this module speed
//#define TRACE_STYLED_FEATURE_RASTERIZER
#include "styled_feature_rasterizer.h"

// local includes
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/value_tile.h"
#include "pyxis/data/feature_iterator_with_prefetch.h"
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/child_iterator.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/geometry/cell.h"

#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

#include "pyxis/region/circle_region.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/xtime.hpp>

// standard includes
#include <cassert>

// {E82F5DB8-3BF9-48b5-B529-64BB7819DD38}
PYXCOM_DEFINE_CLSID(StyledFeatureRasterizer, 
0xe82f5db8, 0x3bf9, 0x48b5, 0xb5, 0x29, 0x64, 0xbb, 0x78, 0x19, 0xdd, 0x38);
PYXCOM_CLASS_INTERFACES(StyledFeatureRasterizer, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(StyledFeatureRasterizer, "Styled Feature to Coverage", "A coverage that rasterizes its input features ", "Analysis/Features",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, -1, "Input Features", "A feature collection to rasterize")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<StyledFeatureRasterizer> gTester;

}

StyledFeatureRasterizer::StyledFeatureRasterizer() : m_useAlpha(false)
{
}

StyledFeatureRasterizer::~StyledFeatureRasterizer()
{
}

void StyledFeatureRasterizer::test()
{
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::string STDMETHODCALLTYPE StyledFeatureRasterizer::getAttributeSchema() const
{
	std::string strXSD = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">";

	strXSD += 
		"<xs:element name=\"StyledFeatureRasterizer\">"
		  "<xs:complexType>"
			  "<xs:sequence>"

				  "<xs:element name=\"UseAlpha\" type=\"xs:int\">"
					"<xs:annotation>"
					  "<xs:appinfo>"
						"<friendlyName>Use Alpha channel</friendlyName>"
						"<description>Specify if to generate an RGB coverage or RGB + Alpha channel coverage</description>"
					  "</xs:appinfo>"
					"</xs:annotation>"
				  "</xs:element>"

			  "</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";

	return strXSD;
}

std::map<std::string, std::string> STDMETHODCALLTYPE StyledFeatureRasterizer::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	mapAttr["UseAlpha"] = m_useAlpha?"1":"0";

	return mapAttr;
}

void STDMETHODCALLTYPE StyledFeatureRasterizer::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	std::string alpha;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"UseAlpha",alpha);

	if (alpha.empty() || alpha == "0" || alpha == "false" || alpha == "False")
	{
		m_useAlpha = false;
	}
	else
	{
		m_useAlpha = true;
	}
}

IProcess::eInitStatus StyledFeatureRasterizer::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	setData("style-version:1.0.0.1");

	// Set up coverage definition
	m_spCovDefn = PYXTableDefinition::create();
	
	if (m_useAlpha)
	{
		m_spCovDefn->addFieldDefinition("RGB", PYXFieldDefinition::knContextRGB, PYXValue::knUInt8, 4);
	}
	else 
	{
		m_spCovDefn->addFieldDefinition("RGB", PYXFieldDefinition::knContextRGB, PYXValue::knUInt8, 3);
	}

	boost::shared_ptr<RasterStateVector> new_states(new RasterStateVector());
	for(int i=0;i<getParameter(0)->getValueCount();++i)
	{
		boost::intrusive_ptr<IFeatureCollection> fc = getParameter(0)->getValue(i)->getOutput()->QueryInterface<IFeatureCollection>();

		if (!fc)
		{
			PYXTHROW(NullFeatureCollectionException, "Failed to get the feature \
									collection to rasterize from the parameter.");
		}

		if (fc->canRasterize())
		{
			m_FCs.push_back(fc);
			new_states->push_back(RasterState::create(fc,ProcRef(getParameter(0)->getValue(i)),getCoverageDefinition()));
		}
	}

	m_state = new_states;
	
	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue StyledFeatureRasterizer::getCoverageValue(	const PYXIcosIndex& index,
												int nFieldIndex	) const
{
	boost::shared_ptr<RasterStateVector> state;
	{
		boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

		if (m_FCs.empty())
		{
			return PYXValue();
		}
		state = m_state;
	}
	
	RasterContext rasterContext(state,index,index.getResolution());

	rasterContext.raster();

	if (rasterContext.getResultValueTile())
	{
		bool init;
		return rasterContext.getResultValueTile()->getValue(0,nFieldIndex,&init);
	}
	else
	{
		return PYXValue();
	}
}

PYXPointer<PYXValueTile> StyledFeatureRasterizer::getFieldTile(	const PYXIcosIndex& index,
															int nRes,
															int nFieldIndex	) const
{
	try
	{
		boost::shared_ptr<RasterStateVector> state;
		{
			boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

			if (m_FCs.empty())
			{
				return PYXPointer<PYXValueTile>();
			}
			state = m_state;
		}
	
		RasterContext rasterContext(state,index,nRes);

		rasterContext.raster();

		return rasterContext.getResultValueTile();
	}
	CATCH_AND_RETHROW("Failed to raster tile " << PYXTile(index,nRes));	
}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

void StyledFeatureRasterizer::createGeometry() const
{
	// TODO[idans] should probably do something more sensible

	if (m_FCs.size() == 1)
	{
		m_spGeom = m_FCs[0]->getGeometry()->clone();
	}
	else
	{
		m_spGeom = PYXGlobalGeometry::create(10);
	}	
}

////////////////////////////////////////////////////////////////////////////////
// StyledFeatureRasterizer::RasterState
////////////////////////////////////////////////////////////////////////////////

StyledFeatureRasterizer::RasterState::RasterState(
				const boost::intrusive_ptr<IFeatureCollection> & spFC,
				const ProcRef & procRef,
				const PYXPointer<const PYXTableDefinition> & definition)
	: 	m_spFC(spFC),
		m_procRef(procRef),
		m_definition(definition),
		m_hasFillColor(false),
		m_hasLineColor(false),
		m_paletteFieldIndex(-1)
{
	loadStyleParameters();
	if (!m_paletteField.empty())
	{
		m_paletteFieldIndex = m_spFC->getFeatureDefinition()->getFieldIndex(m_paletteField);
	}
}

PYXValue StyledFeatureRasterizer::RasterState::getColor(const std::string & styleNode) 
{
	std::string colorString = m_spFC->getStyle(styleNode);

	if (colorString.size() == 0)
	{
		if (styleNode == "Line/Colour")
		{
			m_hasLineColor = false;
		}
		else
		{
			m_hasFillColor = false;
		}

		return StringUtils::fromString<PYXValue>("uint8_t[3] 127 127 255");
	}

	if (styleNode == "Line/Colour")
	{
		m_hasLineColor = true;
	}
	else
	{
		m_hasFillColor = true;
	}
	return StringUtils::fromString<PYXValue>(colorString);
}

PYXValue StyledFeatureRasterizer::RasterState::createRandomColor() const
{
	const unsigned char colours[][3] =
	{
		// http://en.wikipedia.org/wiki/Web_colors
		{ 0x00, 0xff, 0xff }, // aqua
		{ 0x00, 0x00, 0x00 }, // black
		{ 0x00, 0x00, 0xff }, // blue
		{ 0xff, 0x00, 0xff }, // fuschia
		{ 0x00, 0x80, 0x00 }, // green
		{ 0x80, 0x80, 0x80 }, // grey
		{ 0x00, 0xff, 0x00 }, // lime
		{ 0x80, 0x00, 0x00 }, // maroon
		{ 0x00, 0x00, 0x80 }, // navy
		{ 0x80, 0x80, 0x00 }, // olive
		{ 0x80, 0x00, 0x80 }, // purple
		{ 0xff, 0x00, 0x00 }, // red
		{ 0xc0, 0xc0, 0xc0 }, // silver
		{ 0x00, 0x80, 0x80 }, // teal
		{ 0xff, 0xff, 0xff }, // white
		{ 0xff, 0xff, 0x00 }  // yellow
	};

	int nHash = 0;
	std::string strProcrefFC(procRefToStr(m_procRef));
	for (std::string::iterator it = strProcrefFC.begin();
		it != strProcrefFC.end(); ++it)
	{
		nHash += *it;
	}

	return PYXValue((const unsigned char *)&colours[nHash % 16],3);
}

double StyledFeatureRasterizer::RasterState::getLineWidth(const std::string & styleNode) const
{
	std::string widthString = m_spFC->getStyle(styleNode);

	if (widthString.size() == 0)
	{
		return 1.0;
	}
	return StringUtils::fromString<double>(widthString);
}

int StyledFeatureRasterizer::RasterState::getFillOpacity(const std::string & styleNode) const
{
	std::string transparencyString = m_spFC->getStyle(styleNode);

	if (transparencyString.size() == 0)
	{
		return 255;
	}
	return StringUtils::fromString<int>(transparencyString);
}


bool StyledFeatureRasterizer::RasterState::hasAlphaChannel()
{
	return (m_definition->getFieldDefinition(0).getCount()==4);
}

void StyledFeatureRasterizer::RasterState::addAlphaChannel(PYXValue & value,int alphaValue)
{
	PYXValue finalColor = m_definition->getFieldDefinition(0).getTypeCompatibleValue();
	for(int i=0;i<3;i++)
	{
		finalColor.setUInt8(i,value.getUInt8(i));
	}
	finalColor.setUInt8(3,alphaValue);

	value = finalColor;
}

void StyledFeatureRasterizer::RasterState::loadStyleParameters()
{
	m_lineColor = getColor("Line/Colour");
	m_fillColor = getColor("Area/Colour");

	//in case there is no style set at all.. make sure we use fill
	if (m_spFC->getStyle("Line/Colour").empty() && m_spFC->getStyle("Line/Colour").empty())
	{
		m_hasFillColor = true;
	}

	m_lineWidth = getLineWidth("Line/Width");
	m_paletteString = m_spFC->getStyle("Area/Palette");
	m_paletteField = m_spFC->getStyle("Area/PaletteField");
	m_lineOpacity = getFillOpacity("Line/Opacity");
	m_fillOpacity = getFillOpacity("Area/Opacity");

	if (hasAlphaChannel())
	{
		addAlphaChannel(m_lineColor,(255*m_lineOpacity)/100);
		addAlphaChannel(m_fillColor,(255*m_fillOpacity)/100);

		if (m_fillColor.getUInt8(3)==0)
		{
			m_hasFillColor = false;
		}
	}

	if (!m_paletteString.empty() && !m_paletteField.empty())
	{
		try
		{
			m_palette = PYXValueColorPalette::create(m_paletteString);
		}
		catch (...)
		{
			TRACE_ERROR("failed to parse palette string: " << m_paletteString);
		}
	}

#ifdef TRACE_STYLED_FEATURE_RASTERIZER
	TRACE_INFO("settings: line: " << m_lineColor << " area: " << m_fillColor << " lineWidth : " << m_lineWidth );
#endif
}

bool StyledFeatureRasterizer::RasterState::doesStyleRequireToDrilIntoGroup(const PYXPointer<IFeatureGroup> & group) const
{
	return m_palette || (!m_hasFillColor && m_hasLineColor);
}

void StyledFeatureRasterizer::RasterState::getStyleForFeature(const PYXPointer<IFeature> & feature,
															  const PYXPointer<const PYXGeometry> & geom,
															  int resolution,
															  unsigned char lineColor[4],
															  unsigned char fillColor[4],
															  bool & hasFillColor)
{

	//vector geometries support borders - so, we can use line and borders...
	lineColor[0] = m_lineColor.getUInt8(0);
	lineColor[1] = m_lineColor.getUInt8(1);
	lineColor[2] = m_lineColor.getUInt8(2);

	fillColor[0] = m_fillColor.getUInt8(0);
	fillColor[1] = m_fillColor.getUInt8(1);
	fillColor[2] = m_fillColor.getUInt8(2);

	if (hasAlphaChannel())
	{
		fillColor[3] = m_fillColor.getUInt8(3);
		lineColor[3] = m_lineColor.getUInt8(3);
	}
	else 
	{
		lineColor[3] = 255;
		fillColor[3] = 255;
	}

	if (!m_hasFillColor)
	{
		fillColor[3] = 0;
	}

	if (!m_hasLineColor)
	{
		lineColor[3] = 0;
	}

	if (m_palette)
	{
		PYXPointer<IFeatureGroup> group = feature->QueryInterface<IFeatureGroup>();

		if (group)
		{
			PYXPointer<PYXHistogram> histogram = group->getFieldHistogram(m_paletteFieldIndex);

			if (histogram)
			{
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
						m_palette->convert(it->range.min,minColor,true);
						finalColor[0] += minColor[0]*count;
						finalColor[1] += minColor[1]*count;
						finalColor[2] += minColor[2]*count;
						finalColor[3] += minColor[3]*count;
					}
					else 
					{
						m_palette->convert(it->range.min,minColor,true);
						m_palette->convert(it->range.max,maxColor,true);
						finalColor[0] += (minColor[0]+maxColor[0])*count/2;
						finalColor[1] += (minColor[1]+maxColor[1])*count/2;
						finalColor[2] += (minColor[2]+maxColor[2])*count/2;
						finalColor[3] += (minColor[3]+maxColor[3])*count/2;
					}
				}

				if (totalCount>0)
				{
					fillColor[0] = (unsigned char)(finalColor[0]/totalCount);
					fillColor[1] = (unsigned char)(finalColor[1]/totalCount);
					fillColor[2] = (unsigned char)(finalColor[2]/totalCount);
					fillColor[3] = (unsigned char)(finalColor[3]/totalCount * m_fillColor.getUInt8(3) / 255 );
				}
				else 
				{
					memset(fillColor,0,4);
				}
			}
		}
		else 
		{
			m_palette->convert(feature->getFieldValue(m_paletteFieldIndex),fillColor,true);
			fillColor[3] = (unsigned char)(fillColor[3] * m_fillColor.getUInt8(3) / 255 );
		}
	}

	hasFillColor = fillColor[3]>0;

	//set the line color...
	bool overwriteLineColor = hasFillColor;

	const PYXVectorGeometry * vectorGeometry = dynamic_cast<const PYXVectorGeometry *>(geom.get());
	const PYXVectorGeometry2 * vectorGeometry2 = dynamic_cast<const PYXVectorGeometry2 *>(geom.get());

	const int useLineColorOffset = 15-4;
	const int startBlendLineColor = 7-4;
	const float blendLength = useLineColorOffset-startBlendLineColor;

	if ((vectorGeometry != 0 || vectorGeometry2 != 0) && overwriteLineColor && m_hasLineColor)
	{
		PYXBoundingCircle circle = geom->getBoundingCircle();
		int featureResolution = PYXBoundingCircle::estimateResolutionFromRadius(circle.getRadius());

		if (featureResolution + useLineColorOffset < resolution)
		{
			overwriteLineColor = false;
		}
		else if (featureResolution + startBlendLineColor < resolution)
		{
			float blend = std::min((resolution - featureResolution - startBlendLineColor)/blendLength,1.0f);

			//TRACE_INFO("resolution = " << resolution << ", featureResolution = " << featureResolution << " ==> blend " << blend);

			overwriteLineColor = false;

			lineColor[0] = (unsigned char)(lineColor[0]*blend+fillColor[0]*(1-blend));
			lineColor[1] = (unsigned char)(lineColor[1]*blend+fillColor[1]*(1-blend));
			lineColor[2] = (unsigned char)(lineColor[2]*blend+fillColor[2]*(1-blend));
			lineColor[3] = (unsigned char)(lineColor[3]*blend+fillColor[3]*(1-blend));
		}
	}

	if (overwriteLineColor)
	{
		//TODO: this is a hack!
		lineColor[0] = fillColor[0];
		lineColor[1] = fillColor[1];
		lineColor[2] = fillColor[2];
		lineColor[3] = fillColor[3];
	}
}

////////////////////////////////////////////////////////////////////////////////
// StyledFeatureRasterizer::RasterContext
////////////////////////////////////////////////////////////////////////////////

StyledFeatureRasterizer::RasterContext::RasterContext(const boost::shared_ptr<RasterStateVector> & state,
													  const PYXIcosIndex & root,
													  int resoluton) 
	:	m_states(state),
		m_rootIndex(root),
		m_nResolution(resoluton),
		m_getDistanceCount(0),
		m_getPointContainedCount(0),
		m_setValueCount(0)
{
	//scale for actual distance on earth.
	m_lineWidth = (*m_states)[0]->getLineWidth()*PYXIcosMath::UnitSphere::calcCellCircumRadius(m_nResolution);
}


void StyledFeatureRasterizer::RasterContext::fillCell(int nPos,unsigned char color[4])
{
	//using 16 looks to minimize looking delays
	boost::recursive_mutex::scoped_lock lock(m_cacheMutex[nPos % 16]);

	ColorState & colorState = m_colorCache[nPos];
	
	colorState.colorSetCount ++;
	colorState.color[0] += color[0]*color[3];
	colorState.color[1] += color[1]*color[3];
	colorState.color[2] += color[2]*color[3];
	colorState.color[3] += color[3];
}

void StyledFeatureRasterizer::RasterContext::fillCells(int nPos,int nLength,unsigned char color[4])
{
	int max = std::min(16,nLength);

	for(int i=0;i<max;++i)
	{
		//using 16 locks to minimize locking delays
		boost::recursive_mutex::scoped_lock lock(m_cacheMutex[(nPos+i) % 16]);

		for(int pos = nPos+i;pos<nPos+nLength;pos+=16)
		{
			ColorState & colorState = m_colorCache[pos];

			colorState.colorSetCount ++;
			colorState.color[0] += color[0]*color[3];
			colorState.color[1] += color[1]*color[3];
			colorState.color[2] += color[2]*color[3];
			colorState.color[3] += color[3];
		}
	}
}


void StyledFeatureRasterizer::RasterContext::raster()
{
#ifdef TRACE_STYLED_FEATURE_RASTERIZER
	boost::posix_time::ptime startTime = boost::posix_time::microsec_clock::local_time();

	TRACE_INFO(	"Start Raster :" << m_rootIndex << " resolution : " << m_nResolution );
#endif

	m_getDistanceOptimizedCount = 0;
	m_makeOptimizedCount = 0;

	//good enough resolution for rastering stuff
	m_errorThreshold = PYXIcosMath::UnitSphere::calcCellCircumRadius(m_nResolution)/5;

	PYXCoord3DDouble opmtimizationCenter;
	SnyderProjection::getInstance()->pyxisToXYZ(m_rootIndex,&opmtimizationCenter);
	double optimizationRadius = PYXIcosMath::UnitSphere::calcTileCircumRadius(m_rootIndex) + m_lineWidth;

	int cellCount = PYXIcosMath::getCellCount(m_rootIndex,m_nResolution);
	try
	{
		m_colorCache.reset(new ColorState[cellCount]);
	}
	CATCH_AND_RETHROW("Failed to alloc colorCache");
	

	m_tileVectorGeom = PYXVectorGeometry::create(PYXCircleRegion::create(m_rootIndex,true),m_rootIndex.getResolution()+11);
	m_tileBoundCircle = PYXCircleRegion(m_rootIndex,true).getBoundingCircle();


	//start rasterize all pipelines...
	std::vector<boost::shared_ptr<SinglePipelineContext>> m_pipelinesContexts;

	try
	{
		for(RasterStateVector::iterator state = m_states->begin(); state != m_states->end(); ++state )
		{
			m_pipelinesContexts.push_back(boost::shared_ptr<SinglePipelineContext>(new SinglePipelineContext(*this,*state)));
			m_pipelinesContexts.back()->raster();
		}
	}
	CATCH_AND_RETHROW("Failed to initialize rasters");	

	//wait for everything to complete
	m_rasterTasks.joinAll();

	m_pipelinesContexts.clear();

	try
	{
		if (m_resultValueTile)
		{
			PYXValue colorValue = (*m_states)[0]->getFillColor();
			unsigned char * color = colorValue.getUInt8Ptr(0);

			if ((*m_states)[0]->hasAlphaChannel())
			{
				for(int i=0;i<cellCount;i++)
				{
					ColorState & colorState = m_colorCache[i];
					if (colorState.colorSetCount>0 && colorState.color[3]>0)
					{
						color[0] = (unsigned char)(colorState.color[0]/colorState.color[3]);
						color[1] = (unsigned char)(colorState.color[1]/colorState.color[3]);
						color[2] = (unsigned char)(colorState.color[2]/colorState.color[3]);
						color[3] = (unsigned char)(colorState.color[3]/colorState.colorSetCount);
						m_resultValueTile->setValue(i, 0, colorValue);
					}
				}
			}
			else
			{
				for(int i=0;i<cellCount;i++)
				{
					ColorState & colorState = m_colorCache[i];
					if (colorState.colorSetCount>0 && colorState.color[3]>0)
					{
						color[0] = (unsigned char)(colorState.color[0]/colorState.color[3]);
						color[1] = (unsigned char)(colorState.color[1]/colorState.color[3]);
						color[2] = (unsigned char)(colorState.color[2]/colorState.color[3]);
						m_resultValueTile->setValue(i, 0, colorValue);
					}
				}
			}
		}
	}
	CATCH_AND_RETHROW("Failed to calculate final color values ");
}

StyledFeatureRasterizer::RasterContext::SinglePipelineContext::SinglePipelineContext(RasterContext & context,const PYXPointer<RasterState> & state) 
	: m_context(context),m_state(state)
{
}

void StyledFeatureRasterizer::RasterContext::SinglePipelineContext::raster()
{
#ifdef TRACE_STYLED_FEATURE_RASTERIZER
	int featureCount = 0;
	int regionCount = 0; 
	int unionRegionCount = 0;
	int geometryCount = 0;
#endif

	PYXPointer<IFeatureGroup> group = m_state->getFeatureCollection()->QueryInterface<IFeatureGroup>();

	if (group)
	{
		if (!m_context.m_resultValueTile)
		{
			try
			{
				m_context.m_resultValueTile = PYXValueTile::create(m_context.m_rootIndex, m_context.m_nResolution, m_state->getDefinition() );
			}
			CATCH_AND_RETHROW("Failed to allocate result PYXValueTile");
		}

		rasterizeGroup(group);
	}
	else 
	{

		PYXPointer<FeatureIterator> spFit = m_state->getFeatureCollection()->getIterator(PYXTile(m_context.m_rootIndex, m_context.m_nResolution));

		if (!spFit->end() && ! m_context.m_resultValueTile)
		{
			try
			{
				m_context.m_resultValueTile = PYXValueTile::create(m_context.m_rootIndex, m_context.m_nResolution, m_state->getDefinition() );
			}
			CATCH_AND_RETHROW("Failed to allocate result PYXValueTile");
		}

		for (; !spFit->end(); spFit->next())
		{
#ifdef TRACE_STYLED_FEATURE_RASTERIZER
			featureCount++;
#endif

			boost::intrusive_ptr<IFeature> spF = spFit->getFeature();

			m_context.m_rasterTasks.addTask(boost::bind(&StyledFeatureRasterizer::RasterContext::SinglePipelineContext::rasterizeFeature,this,spF));
		}
#ifdef TRACE_STYLED_FEATURE_RASTERIZER
		boost::posix_time::time_duration td = boost::posix_time::microsec_clock::local_time() - startTime;
		double totalTime = static_cast<int>(td.total_milliseconds())/1000.0;

		TRACE_INFO(	"Raster Tile summary: Index: " << m_rootIndex << " Depth: " << (m_nResolution- m_rootIndex.getResolution()) << 
					" Features: " << featureCount << "(Regions: " << regionCount << "(Union: " << unionRegionCount << "), Geometries: " << geometryCount << ")" <<
					" dist: " << m_getDistanceCount << " opt-dist: " << m_getDistanceOptimizedCount <<  " opt: " << m_makeOptimizedCount << 
					" inside: " << m_getPointContainedCount << " set: " << m_setValueCount <<
					" TotalTime: " << totalTime );
#endif
	}
}

void StyledFeatureRasterizer::RasterContext::SinglePipelineContext::rasterizeFeature(PYXPointer<IFeature> spF)
{
	PYXPointer<const PYXGeometry> spGeom = spF->getGeometry();
	assert(spGeom);

	const PYXVectorGeometry * vectorGeometry = dynamic_cast<const PYXVectorGeometry *>(spGeom.get());

	unsigned char lineColor[4];
	unsigned char fillColor[4];
	bool hasFillColor;

	try
	{
		m_state->getStyleForFeature(spF,spGeom,m_context.m_nResolution,lineColor,fillColor,hasFillColor);
	}
	CATCH_AND_RETHROW("Failed to get style for feature " << spF->getID());	

	try
	{
		if (vectorGeometry)
		{
			PYXCollectionVectorRegion * unionRegion = dynamic_cast<PYXCollectionVectorRegion*>(vectorGeometry->getRegion().get());
		
			if (unionRegion != 0)
			{
				for(int i=0;i<unionRegion->getRegionCount();i++)
				{
					rasterizeRegion(unionRegion->getRegion(i),lineColor,fillColor,hasFillColor);
				}
			}
			else
			{
				rasterizeRegion(vectorGeometry->getRegion(),lineColor,fillColor,hasFillColor);
			}
		}
		else
		{
			const PYXVectorGeometry2 * vectorGeometry2 = dynamic_cast<const PYXVectorGeometry2 *>(spGeom.get());
			if(vectorGeometry2)
			{
				rasterizeVectorGeometry(*vectorGeometry2,lineColor,fillColor,hasFillColor);
			}
			else
			{
				if (hasFillColor)
				{
					rasterizeGeometry(*spGeom,fillColor);
				}
				else
				{
					rasterizeGeometry(*spGeom,lineColor);
				}
			}
		}
	}
	CATCH_AND_RETHROW("Failed to raster feature " << spF->getID());	
}


void StyledFeatureRasterizer::RasterContext::SinglePipelineContext::rasterizeGroup(PYXPointer<IFeatureGroup> group)
{
	//if the group is empty - there is nothing to raster
	if (group->getFeaturesCount().max == 0)
	{
		return;
	}

	//if we have no more details available - raster that group as we get it.
	if (!group->moreDetailsAvailable())
	{
		m_context.m_rasterTasks.addTask(boost::bind(&StyledFeatureRasterizer::RasterContext::SinglePipelineContext::rasterizeFeature,this,group));
		return;
	}

	std::vector<boost::intrusive_ptr<IFeature>> subFeaturesToRasterize;
	std::vector<boost::intrusive_ptr<IFeatureGroup>> subGroupToRasterize;

	try
	{
		//split between groups and features.
		for (PYXPointer<FeatureIterator> spFit = group->getGroupIterator(*m_context.m_tileVectorGeom); !spFit->end(); spFit->next())
		{
			boost::intrusive_ptr<IFeature> spF = spFit->getFeature();

			boost::intrusive_ptr<IFeatureGroup> subGroup = spF->QueryInterface<IFeatureGroup>();

			if (subGroup)
			{
				subGroupToRasterize.push_back(subGroup);
			}
			else
			{
				subFeaturesToRasterize.push_back(spF);
			}
		}
	}
	CATCH_AND_RETHROW("Failed to iterate over feature in group " << group->getID());
	
	try
	{
		//rasterize groups...
		for(auto & subGroup : subGroupToRasterize)
		{	
			PYXPointer<const PYXGeometry> spGeom = subGroup->getGeometry();

			int resOffset = 0;
			
			if (m_state->doesStyleRequireToDrilIntoGroup(subGroup))
			{
				resOffset = 3;
			}

			if (spGeom->getCellResolution()-resOffset >= m_context.m_nResolution)
			{
				//treat this group as a single feature.
				subFeaturesToRasterize.push_back(subGroup);
			}
			else
			{
				m_context.m_rasterTasks.addTask(boost::bind(&StyledFeatureRasterizer::RasterContext::SinglePipelineContext::rasterizeGroup,this,subGroup));
			}		
		}	
	}
	CATCH_AND_RETHROW("Failed to start subgroup tasks for group " << group->getID());
	
	try
	{
		//put all features at the back - this should improve the speed and memory consumption as the thead pool do tasks in lifo order.
		//so, we would like to get read of all the features ASAP.
		for(auto & feature : subFeaturesToRasterize)
		{
			m_context.m_rasterTasks.addTask(boost::bind(&StyledFeatureRasterizer::RasterContext::SinglePipelineContext::rasterizeFeature,this,feature));
		}
	}
	CATCH_AND_RETHROW("Failed to start features raster tasks for group " << group->getID());	
}

void StyledFeatureRasterizer::RasterContext::SinglePipelineContext::rasterizeRegion(const PYXPointer<PYXVectorRegion> & region,unsigned char lineColor[4],unsigned char fillColor[4],bool useFillColor)
{
	PYXBoundingCircle regionCircle = region->getBoundingCircle();
	if (regionCircle.getRadius() < PYXIcosMath::UnitSphere::calcCellCircumRadius(m_context.m_nResolution))
	{
		//this is more or less a single cell - just rasterize it.

		PYXIcosIndex index;
		SnyderProjection::getInstance()->nativeToPYXIS(SphereMath::xyzll(regionCircle.getCenter()),&index,m_context.m_nResolution);

		if (index.isDescendantOf(m_context.m_rootIndex))
		{
			int nPos = PYXIcosMath::calcCellPosition(m_context.m_rootIndex, index);
			m_context.fillCell(nPos,lineColor);
		}
		return;
	}
	
	int nCellCount = PYXIcosMath::getCellCount(m_context.m_rootIndex,m_context.m_nResolution);

	for(int nPos = 0;nPos<nCellCount;nPos++)
	{
		PYXIcosIndex index = PYXIcosMath::calcIndexFromOffset(m_context.m_rootIndex,m_context.m_nResolution,nPos);
		PYXCoord3DDouble location;
		SnyderProjection::getInstance()->pyxisToXYZ(index,&location);

		double distance = 0;
		
		distance = region->getDistanceToBorder(location,m_context.m_errorThreshold);

		bool isBorder = distance < m_context.m_lineWidth;

		while(!isBorder && index.hasVertexChildren() && index.getResolution()>m_context.m_rootIndex.getResolution() )
		{
			index.decrementResolution();

			isBorder = distance < PYXIcosMath::UnitSphere::calcTileCircumRadius(index) + m_context.m_lineWidth; //radius of the boundary
		}

		if (index.getResolution()==m_context.m_nResolution)
		{
			if (isBorder)
			{
				m_context.fillCell(nPos,lineColor);
			}
			else if (useFillColor)
			{
				if (region->isPointContained(location,m_context.m_errorThreshold))
				{
					m_context.fillCell(nPos,fillColor);
				}
			}
		}
		else
		{
			if (isBorder)
			{
				index.incrementResolution();
			}

			//amount of cells in our tile
			int tileCellCount = PYXIcosMath::getCellCount(index,m_context.m_nResolution);

			if (useFillColor)
			{
				if (region->isPointContained(location,m_context.m_errorThreshold))
				{
					m_context.fillCells(nPos,tileCellCount,fillColor);
				}
			}

			nPos += tileCellCount-1;
		}
	}
}

void StyledFeatureRasterizer::RasterContext::SinglePipelineContext::rasterizeGeometry(const PYXGeometry & geom,unsigned char color[4])
{
	// Intersect geometry with tile
	PYXTile tile(m_context.m_rootIndex, m_context.m_nResolution);

	PYXPointer<PYXGeometry> intersectionGeom = geom.intersection(tile);

	if (!intersectionGeom->isEmpty())
	{
		PYXTile * intersectionTile = dynamic_cast<PYXTile*>(intersectionGeom.get());
		if (intersectionTile != NULL && intersectionTile->getRootIndex().getResolution() > m_context.m_nResolution)
		{
			PYXIcosIndex i(intersectionTile->getRootIndex());
			i.setResolution(m_context.m_nResolution);
			intersectionGeom = PYXTile::create(i,m_context.m_nResolution);
		}

		intersectionGeom->setCellResolution(m_context.m_nResolution);
	}
	else
	{
		//it's empty - lets move to the next feature.
		return;
	}

	if (m_context.m_rootIndex.getResolution() == m_context.m_nResolution)
	{
		if (!intersectionGeom->isEmpty())
		{
			m_context.fillCell(0,color);
		}
	}
	else
	{
		PYXPointer<PYXIterator> spIt = intersectionGeom->getIterator();
		for (; !spIt->end(); spIt->next())
		{
			const PYXIcosIndex& index2 = spIt->getIndex();

			int nPos = PYXIcosMath::calcCellPosition(m_context.m_rootIndex, index2);

			m_context.fillCell(nPos,color);
		}
	}
}

void StyledFeatureRasterizer::RasterContext::SinglePipelineContext::rasterizeVectorGeometry(const PYXVectorGeometry2 & geom, unsigned char lineColor[4],unsigned char fillColor[4],bool useFillColor)
{
	int resolution = m_context.m_nResolution;
	std::vector<PYXInnerTile> innerTiles = PYXInnerTile::createInnerTiles(PYXTile(m_context.m_rootIndex, m_context.m_nResolution));

	int cellCount = PYXIcosMath::getCellCount(m_context.m_rootIndex,resolution);

	for(std::vector<PYXInnerTile>::iterator it = innerTiles.begin(); it != innerTiles.end(); ++it)
	{
		for(PYXPointer<PYXInnerTileIntersectionIterator> iterator = geom.getInnerTileIterator(*it);
			!iterator->end();
			iterator->next())
		{
			PYXInnerTileIntersection intersection = iterator->getIntersection();
			if(intersection != knIntersectionNone)
			{
				if (intersection == knIntersectionComplete && !useFillColor)
				{
					continue;
				}
				PYXIcosIndex index = iterator->getTile().asTile().getRootIndex();
				unsigned char *color = (intersection==knIntersectionComplete)?  fillColor:lineColor;
				if(index.getResolution() == resolution)
				{
					int nPos = PYXIcosMath::calcCellPosition(m_context.m_rootIndex, index);
					if(nPos < cellCount)
					{
						m_context.fillCell(nPos, color);
					}
					else
					{
						assert(0 && "Should never get here");
					}
				}
				else if (index.getResolution() < resolution)
				{
					int tileCellCount = PYXIcosMath::getCellCount(index,resolution);

					index.setResolution(resolution);
					int nPos = PYXIcosMath::calcCellPosition(m_context.m_rootIndex, index);
				
					m_context.fillCells(nPos,tileCellCount,color);
				}
			}
		}
	}
}

PYXPointer<PYXValueTile> StyledFeatureRasterizer::RasterContext::getResultValueTile()
{
	return m_resultValueTile;
}

