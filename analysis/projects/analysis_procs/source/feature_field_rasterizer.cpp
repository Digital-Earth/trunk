/******************************************************************************
feature_field_rasterizer.cpp

begin		: 2011-11-17
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_ANALYSIS_PROCS_SOURCE

//Enable this to trace diagnostic profing on this module speed
//#define TRACE_FEATURE_FIELD_RASTERIZER
#include "feature_field_rasterizer.h"

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
#include "pyxis/geometry/vector_geometry.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/xtime.hpp>

// standard includes
#include <cassert>

// {4C99372D-E391-4ca2-A834-725BB9013079}
PYXCOM_DEFINE_CLSID(FeatureFieldRasterizer, 
0x4c99372d, 0xe391, 0x4ca2, 0xa8, 0x34, 0x72, 0x5b, 0xb9, 0x1, 0x30, 0x79);

PYXCOM_CLASS_INTERFACES(FeatureFieldRasterizer, IProcess::iid, ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(FeatureFieldRasterizer, "Feature Field to Coverage", "A coverage that rasterizes its input features", "Analysis/Features",
					ICoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Features", "A feature collection to rasterize")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<FeatureFieldRasterizer> gTester;

}

FeatureFieldRasterizer::FeatureFieldRasterizer() 
{
	m_fieldName = "[FeatureCount]";
	m_aggregate = "max";
	m_buffer = 0;
}

FeatureFieldRasterizer::~FeatureFieldRasterizer()
{
}

void FeatureFieldRasterizer::test()
{
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE FeatureFieldRasterizer::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	mapAttr["field_name"] = m_fieldName;
	mapAttr["aggregate"] = m_aggregate;
	mapAttr["buffer"] = StringUtils::toString(m_buffer);
	
	return mapAttr;
}

void STDMETHODCALLTYPE FeatureFieldRasterizer::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"field_name",m_fieldName);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"aggregate",m_aggregate);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"buffer",double,m_buffer);
}


std::string STDMETHODCALLTYPE FeatureFieldRasterizer::getAttributeSchema() const
{
	std::string xsd = 
	"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:simpleType name=\"AggregateType\">"
			"<xs:restriction base=\"xs:string\">"
			    "<xs:enumeration value=\"min\" />"
				"<xs:enumeration value=\"max\" />"
			"</xs:restriction>"
		  "</xs:simpleType>"
		  "<xs:simpleType name=\"FieldName\">"
			"<xs:restriction base=\"xs:string\">"
			    "<xs:enumeration value=\"[FeatureCount]\" />"
				"<xs:enumeration value=\"[Intesection]\" />";

	if (!m_spFC)
	{
		if (getParameter(0)->getValue(0))
		{
			m_spFC = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureCollection>();
		}
	}

	if (m_spFC)
	{
		PYXPointer<PYXTableDefinition> definition = m_spFC->getFeatureDefinition();
		if (definition)
		{
			for(int i=0;i<definition->getFieldCount();++i)
			{
				if (definition->getFieldDefinition(i).isNumeric())
				{
					xsd +=	"<xs:enumeration value=\"" + definition->getFieldDefinition(i).getName()  + "\" />";
				}
			}
		}
	}

	xsd +=
		    "</xs:restriction>"
		  "</xs:simpleType>"
		  "<xs:element name=\"FeatureFieldRasterizer\">"
		  "<xs:complexType>"
			"<xs:sequence>"			
			  "<xs:element name=\"field_name\" type=\"FieldName\" >"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Field Name</friendlyName>"
					"<description>Filed to use for rasterization</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"aggregate\" type=\"AggregateType\" >"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Aggregate</friendlyName>"
					"<description>How to aggregate multiple features values</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"buffer\" type=\"xs:double\" >"
				"<xs:annotation>"
				  "<xs:appinfo>"
				    "<friendlyName>Buffer [m]</friendlyName>"
					"<description>Buffer to apply on features in metters</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";

	return xsd;
}

IProcess::eInitStatus FeatureFieldRasterizer::initImpl()
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);

	m_spFC = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IFeatureCollection>();

	if (!m_spFC)
	{
		PYXTHROW(NullFeatureCollectionException, "Failed to get the feature \
								collection to rasterize from the parameter.");
	}

	m_state = RasterState::create(m_spFC,ProcRef(getParameter(0)->getValue(0)),m_fieldName,m_aggregate,m_buffer);

	// Set up coverage definition
	m_spCovDefn = m_state->getOutputDefinition();

	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// ICoverage
////////////////////////////////////////////////////////////////////////////////

PYXValue FeatureFieldRasterizer::getCoverageValue(	const PYXIcosIndex& index,
												int nFieldIndex	) const
{
	PYXPointer<RasterState> state;
	{
		boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

		if (!m_spFC->canRasterize())
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

PYXPointer<PYXValueTile> FeatureFieldRasterizer::getFieldTile(	const PYXIcosIndex& index,
															int nRes,
															int nFieldIndex	) const
{
	PYXPointer<RasterState> state;
	{
		boost::recursive_mutex::scoped_lock lock(m_coverageMutex);

		if (!m_spFC->canRasterize())
		{
			return PYXPointer<PYXValueTile>();
		}
		state = m_state;
	}
	
	RasterContext rasterContext(state,index,nRes);

	rasterContext.raster();

	return rasterContext.getResultValueTile();

}

////////////////////////////////////////////////////////////////////////////////
// Misc
////////////////////////////////////////////////////////////////////////////////

void FeatureFieldRasterizer::createGeometry() const
{
	// TODO[mlepage] should probably do something more sensible

	if (m_spFC && m_spFC->getGeometry())
	{
		m_spGeom = m_spFC->getGeometry()->clone();
	}
	else
	{
		m_spGeom = PYXGlobalGeometry::create(10);
	}	
}

////////////////////////////////////////////////////////////////////////////////
// FeatureFieldRasterizer::RasterState
////////////////////////////////////////////////////////////////////////////////


FeatureFieldRasterizer::RasterState::RasterState(
				const boost::intrusive_ptr<IFeatureCollection> & spFC,
				const ProcRef & procRef,
				const std::string & fieldName,
				const std::string & aggregate,
				double buffer
				)
	: 	m_spFC(spFC),
		m_procRef(procRef),
		m_definition(PYXTableDefinition::create()),
		m_fieldIndex(knIntesection),
		m_max(true)
{
	if (fieldName == "[FeatureCount]")
		m_fieldIndex = knFieldCount;
	else if (fieldName == "Intesection")
		m_fieldIndex = knIntesection;
	else 
	{
		m_fieldIndex = spFC->getFeatureDefinition()->getFieldIndex(fieldName);
	}

	switch(m_fieldIndex)
	{
	case knFieldCount:
		m_definition->addFieldDefinition("Count",PYXFieldDefinition::knContextNone,PYXValue::knUInt32);
		break;
	case knIntesection:
		m_definition->addFieldDefinition("Intesection",PYXFieldDefinition::knContextNone,PYXValue::knBool);
		break;
	default:
		m_definition->addFieldDefinition(spFC->getFeatureDefinition()->getFieldDefinition(m_fieldIndex));
		break;
	}

	m_max = aggregate != "min";

	m_buffer = buffer/SphereMath::knEarthRadius; //in radians...
}


PYXPointer<PYXTableDefinition> FeatureFieldRasterizer::RasterState::getOutputDefinition()
{
	return m_definition;
}

PYXValue FeatureFieldRasterizer::RasterState::getValue(const PYXPointer<IFeature> & feature) const
{
	switch(m_fieldIndex)
	{
	case knFieldCount:
		return PYXValue((uint32_t)1);
	case knIntesection:
		return PYXValue((bool)true);
	default:
		return feature->getFieldValue(m_fieldIndex);
	}
}

PYXValue FeatureFieldRasterizer::RasterState::aggregate(const PYXValue & a,const PYXValue & b) const
{
	if (m_fieldIndex == knFieldCount)
	{
		return PYXValue(a.getUInt32()+b.getUInt32());
	}

	if (a.isNull())
		return b;
	if (b.isNull())
		return a;

	if (m_max)
	{
		if (a.getDouble() > b.getDouble())
			return a;
		else
			return b;
	}
	else 
	{
		if (a.getDouble() < b.getDouble())
			return a;
		else
			return b;
	}
}

////////////////////////////////////////////////////////////////////////////////
// FeatureFieldRasterizer::RasterContext
////////////////////////////////////////////////////////////////////////////////

FeatureFieldRasterizer::RasterContext::RasterContext(const PYXPointer<RasterState> & state,													  
													  const PYXIcosIndex & root,
													  int resoluton) 
	:	m_state(state),
		m_rootIndex(root),
		m_nResolution(resoluton),
		m_getDistanceCount(0),
		m_getPointContainedCount(0),
		m_setValueCount(0)
{
	//scale for actual distance on earth.
	m_lineWidth = PYXIcosMath::UnitSphere::calcCellCircumRadius(m_nResolution);
}


const FeatureFieldRasterizer::RasterContext::Location & FeatureFieldRasterizer::RasterContext::getLocation(int nPos)
{
	Location & location = m_locationCache[nPos];

	if (location.index.isNull())
	{
		location.index = PYXIcosMath::calcIndexFromOffset(m_rootIndex,m_nResolution,nPos);
		SnyderProjection::getInstance()->pyxisToXYZ(location.index,&location.location);
	}
	return location;
}

void FeatureFieldRasterizer::RasterContext::raster()
{
#ifdef TRACE_FEATURE_FIELD_RASTERIZER
	boost::posix_time::ptime startTime = boost::posix_time::microsec_clock::local_time();

	TRACE_INFO(	"Start Raster :" << m_rootIndex << " resolution : " << m_nResolution );
#endif

	int featureCount = 0;
	int regionCount = 0; 
	int unionRegionCount = 0;
	int geometryCount = 0;
	m_getDistanceOptimizedCount = 0;
	m_makeOptimizedCount = 0;

	double buffer = m_state->getBuffer();

	//good enough resolution for rastering stuff
	m_errorThreshold = PYXIcosMath::UnitSphere::calcCellCircumRadius(m_nResolution)/5;

	PYXIcosIndex tileRoot(m_rootIndex);

	while (buffer > PYXIcosMath::UnitSphere::calcTileCircumRadius(tileRoot)/10 && tileRoot.getResolution() > 2)
	{
		tileRoot.decrementResolution();
	}

	PYXPointer<FeatureIterator> spFit;

	if (buffer > PYXIcosMath::UnitSphere::calcTileCircumRadius(tileRoot)/10)
	{
		//get a global iterator...
		spFit = m_state->getFeatureCollection()->getIterator();
	}
	else
	{
		spFit = m_state->getFeatureCollection()->getIterator(PYXTile(tileRoot, m_nResolution));
	}

	if (!spFit->end())
	{
		m_resultValueTile = PYXValueTile::create(m_rootIndex, m_nResolution, m_state->getDefinition() );

		//inishitate the snyder cache as well
		m_locationCache.reset(new Location[PYXIcosMath::getCellCount(m_rootIndex,m_nResolution)]);
	}

	for (; !spFit->end(); spFit->next())
	{
		featureCount++;

		if (featureCount == 1000)
		{
			//we have alot of features. start to speed up the reading
			spFit = FeatureIteratorWithPrefetch::create(spFit);
		}
		if (featureCount % 10000 == 0)
		{
			TRACE_INFO("featureCount : " << featureCount);
		}

		boost::intrusive_ptr<IFeature> spF = spFit->getFeature();
		const std::string& strFID = spF->getID();

		PYXValue featureValue = m_state->getValue(spF);

		PYXPointer<const PYXGeometry> spGeom = spF->getGeometry();
		assert(spGeom);

		const PYXVectorGeometry * vectorGeometry = dynamic_cast<const PYXVectorGeometry *>(spGeom.get());

		if (vectorGeometry)
		{
			PYXCollectionVectorRegion * unionRegion = dynamic_cast<PYXCollectionVectorRegion*>(vectorGeometry->getRegion().get());
			if (unionRegion != 0 && ! m_state->isFeatureCount())
			{
				unionRegionCount++;
				for(int i=0;i<unionRegion->getRegionCount();i++)
				{
					regionCount++;
					if (buffer != 0)
					{
						rasterizeRegion(PYXVectorBufferRegion::create(unionRegion->getRegion(i),buffer),featureValue);
					}
					else
					{
						rasterizeRegion(unionRegion->getRegion(i),featureValue);
					}
				}
			}
			else
			{
				regionCount++;
				if (buffer != 0)
				{
					rasterizeRegion(PYXVectorBufferRegion::create(vectorGeometry->getRegion(),buffer),featureValue);
				}
				else 
				{
					rasterizeRegion(vectorGeometry->getRegion(),featureValue);
				}
			}
		}
		else
		{
			rasterizeGeometry(*spGeom,featureValue);
			geometryCount++;
		}
	}

#ifdef TRACE_FEATURE_FIELD_RASTERIZER
	boost::posix_time::time_duration td = boost::posix_time::microsec_clock::local_time() - startTime;
	double totalTime = static_cast<int>(td.total_milliseconds())/1000.0;

	TRACE_INFO(	"Raster Tile summary: Index: " << m_rootIndex << " Depth: " << (m_nResolution- m_rootIndex.getResolution()) << 
				" Features: " << featureCount << "(Regions: " << regionCount << "(Union: " << unionRegionCount << "), Geometries: " << geometryCount << ")" <<
				" dist: " << m_getDistanceCount << " opt-dist: " << m_getDistanceOptimizedCount <<  " opt: " << m_makeOptimizedCount << 
				" inside: " << m_getPointContainedCount << " set: " << m_setValueCount <<
				" TotalTime: " << totalTime );
#endif
}


void FeatureFieldRasterizer::RasterContext::rasterizeRegion(const PYXPointer<PYXVectorRegion> & region,const PYXValue & value)
{
	int nCellCount = PYXIcosMath::getCellCount(m_rootIndex,m_nResolution);

	PYXValue oldValue = value;

	for(int nPos = 0;nPos<nCellCount;nPos++)
	{
		const Location & loc = getLocation(nPos);

		PYXIcosIndex index(loc.index);

		const PYXCoord3DDouble & location = loc.location; 

		m_getDistanceCount++;

		double distance = 0;

		distance = region->getDistanceToBorder(location,m_errorThreshold);

		bool isBorder = distance < m_lineWidth;

		while(!isBorder && index.hasVertexChildren() && index.getResolution()>m_rootIndex.getResolution() )
		{
			index.decrementResolution();

			isBorder = distance < PYXIcosMath::UnitSphere::calcTileCircumRadius(index) + m_lineWidth; //radius of the boundary
		}

		if (index.getResolution()==m_nResolution)
		{
			if (isBorder)
			{
				if (m_resultValueTile->getValue(nPos, 0, &oldValue))
				{
					m_resultValueTile->setValue(nPos, 0, m_state->aggregate(value,oldValue));
				}
				else
				{
					m_resultValueTile->setValue(nPos, 0, value);
				}
				m_setValueCount++;
			}
			else
			{
				m_getPointContainedCount++;
				if (region->isPointContained(location,m_errorThreshold))
				{
					if (m_resultValueTile->getValue(nPos, 0, &oldValue))
					{
						m_resultValueTile->setValue(nPos, 0, m_state->aggregate(value,oldValue));
					}
					else 
					{
						m_resultValueTile->setValue(nPos, 0, value);
					}
					m_setValueCount++;
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
			int tileCellCount = PYXIcosMath::getCellCount(index,m_nResolution);

			m_getPointContainedCount++;

			if (region->isPointContained(location,m_errorThreshold))
			{
				for(int i=0;i<tileCellCount;i++)
				{
					if (m_resultValueTile->getValue(nPos+i, 0, &oldValue))
					{
						m_resultValueTile->setValue(nPos+i, 0, m_state->aggregate(value,oldValue));
					}
					else 
					{
						m_resultValueTile->setValue(nPos+i, 0, value);
					}
					m_setValueCount++;
				}
			}

			nPos += tileCellCount-1;
		}
	}
}

void FeatureFieldRasterizer::RasterContext::rasterizeGeometry(const PYXGeometry & geom, const PYXValue & value)
{
	// Intersect geometry with tile
	PYXTile tile(m_rootIndex, m_nResolution);

	PYXValue oldValue = value;

	PYXPointer<PYXGeometry> intersectionGeom = geom.intersection(tile);

	if (!intersectionGeom->isEmpty())
	{
		intersectionGeom->setCellResolution(m_nResolution);
	}
	else
	{
		//it's empty - lets move to the next feature.
		return;
	}

	if (m_rootIndex.getResolution() == m_nResolution)
	{
		if (!intersectionGeom->isEmpty())
		{
			if (m_resultValueTile->getValue(0, 0, &oldValue))
			{
				m_resultValueTile->setValue(0, 0, m_state->aggregate(value,oldValue));
			}
			else 
			{
				m_resultValueTile->setValue(0, 0, value);
			}
		}
	}
	else
	{
		PYXPointer<PYXIterator> spIt = intersectionGeom->getIterator();
		for (; !spIt->end(); spIt->next())
		{
			const PYXIcosIndex& index2 = spIt->getIndex();

			int nPos = PYXIcosMath::calcCellPosition(m_rootIndex, index2);

			if (m_resultValueTile->getValue(nPos, 0, &oldValue))
			{
				m_resultValueTile->setValue(nPos, 0, m_state->aggregate(value,oldValue));
			}
			else 
			{
				m_resultValueTile->setValue(nPos, 0, value);
			}
			m_setValueCount++;
		}
	}
}

PYXPointer<PYXValueTile> FeatureFieldRasterizer::RasterContext::getResultValueTile()
{
	return m_resultValueTile;
}
