/******************************************************************************
geotag_record_collection.cpp

begin		: 2013-06-11
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE

#include "geotag_record_collection.h"

#include "feature_collection_process.h"
#include "pyxis/procs/default_feature.h"
#include "pyxis/pipe/process_local_storage.h"
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/data/feature_iterator_linq.h"
#include "pyxis/geometry/tile_collection.h"
#include "boost/bind.hpp"


// {11E2EBE8-71D3-4D9B-B01A-308BFBB76F71}
PYXCOM_DEFINE_CLSID(GeotagRecordCollection, 
0x11e2ebe8, 0x71d3, 0x4d9b, 0xb0, 0x1a, 0x30, 0x8b, 0xfb, 0xb7, 0x6f, 0x71);


PYXCOM_CLASS_INTERFACES(GeotagRecordCollection, IProcess::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(GeotagRecordCollection, "Geotag Record Collection", "Add geo-spatial tag to non geo-satcial data", "Analysis/Features/Geotagging",
					IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)	
	IPROCESS_SPEC_PARAMETER(IRecordCollection::iid, 1, 1, "Input Record Collection", "A collection of records to be tagged.")
	IPROCESS_SPEC_PARAMETER(IGeometryProvider::iid, 1, 1, "Input Geometry Provider", "A geometry provider based on which the records are geotagged.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<GeotagRecordCollection> gTester;

}

//! Constructor
GeotagRecordCollection::GeotagRecordCollection()
{
}

//! Destructor
GeotagRecordCollection::~GeotagRecordCollection()
{
}

void GeotagRecordCollection::test()
{
}


////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

//! Initialize the process.
IProcess::eInitStatus GeotagRecordCollection::initImpl()
{
	//set the ID.
	m_strID = "Geotagged-RecordCollection:" + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	//set geometry provider
	m_inputRC = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IRecordCollection>();
	if(!m_inputRC)
	{
		setInitProcError<GenericProcInitError> ("Invalid record collection");
		return knFailedToInit;
	}	

	//set Geometry Provider
	m_geometryProvider = getParameter(1)->getValue(0)->getOutput()->QueryInterface<IGeometryProvider>();
	if(!m_geometryProvider)
	{
		setInitProcError<GenericProcInitError> ("Invalid geometry provider");
		return knFailedToInit;
	}	

	//set output feature definition
	m_featuresDefinition = m_inputRC->getRecordDefinition();

	PipeUtils::waitUntilPipelineIdentityStable(this);
	PYXPointer<PYXLocalStorage> storage = PYXProcessLocalStorage::create(this);

	auto buffer = storage->get("geometry");

	if (buffer.get()!=NULL)
	{
		*buffer >> m_spGeometry;
	}
	else
	{
		// Creating the geometry of the output feature collection
		PYXPointer<PYXTileCollection> finalGeometry = PYXTileCollection::create();
		int finalResolution = 0;
		int newCellCount = 0;

		const int GEOMETRY_CELL_LIMIT = 10000;

		for (auto it = getIterator();!it->end();it->next())
		{
			auto recordGeometry = it->getFeature()->getGeometry();
			if (finalResolution == 0)
			{
				finalResolution = std::min(10, recordGeometry->getCellResolution());
			}
		
			PYXTileCollection geometry;
			recordGeometry->copyTo(&geometry,finalResolution);
			auto cellCount = geometry.getGeometryCount();
			if (cellCount > GEOMETRY_CELL_LIMIT) 
			{
				geometry.limitCellsCountTo(GEOMETRY_CELL_LIMIT);
				cellCount = geometry.getGeometryCount();
			}
			finalGeometry->addGeometry(geometry);
			newCellCount += cellCount;

			if (newCellCount > GEOMETRY_CELL_LIMIT) 
			{
				finalGeometry->limitCellsCountTo(GEOMETRY_CELL_LIMIT);
				newCellCount = 0;
			}
			
			finalResolution = finalGeometry->getCellResolution();
		}
		
		finalGeometry->limitCellsCountTo(GEOMETRY_CELL_LIMIT);
		finalGeometry->setCellResolution(24);
		m_spGeometry = finalGeometry;

		PYXStringWireBuffer geomBuffer;
		geomBuffer << *m_spGeometry;
		storage->set("geometry",geomBuffer);
	}	
	
	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
////////////////////////////////////////////////////////////////////////////////

bool goemetryNotNull (const boost::intrusive_ptr<IFeature> & feature)
{
	return feature->getGeometry();
}
bool goemetryIntersects (const boost::intrusive_ptr<IFeature> & feature, PYXPointer<PYXGeometry> geometry)
{
	return goemetryNotNull(feature) && geometry->intersects(*feature->getGeometry());
}

//! Get an iterator to all the features in this collection.
PYXPointer<FeatureIterator> GeotagRecordCollection::getIterator() const
{
	//Filter the null geometries.
	return PYXFeatureIteratorLinq(Iterator::create(m_inputRC,m_geometryProvider)).filter(boost::bind(goemetryNotNull,_1)) ;
}

//! Get an iterator to all the features in this collection that intersect this geometry.
PYXPointer<FeatureIterator> GeotagRecordCollection::getIterator(const PYXGeometry& geometry) const
{
	//filter null goemetries and geometries that do not intersect
	return PYXFeatureIteratorLinq(Iterator::create(m_inputRC,m_geometryProvider)).filter(boost::bind(goemetryIntersects,_1,geometry.clone())) ;
}



//! Get styles that determine how to visualize features in this collection.
std::vector<FeatureStyle> GeotagRecordCollection::getFeatureStyles() const
{
	return std::vector<FeatureStyle>();
}

//! Get the feature with the specified ID.
boost::intrusive_ptr<IFeature> GeotagRecordCollection::getFeature(const std::string& strFeatureID) const
{
	auto it = getIterator();
	
	while (!it->end())
	{
		if (it->getFeature()->getID() == strFeatureID)
		{
			return it->getFeature();
		}
		it->next();
	}

	return 0;
}

//! Get the feature definition.
PYXPointer<const PYXTableDefinition> GeotagRecordCollection::getFeatureDefinition() const
{
	return m_featuresDefinition;
}

//! Get the feature definition.
PYXPointer<PYXTableDefinition> GeotagRecordCollection::getFeatureDefinition()
{
	return m_featuresDefinition;
}

bool GeotagRecordCollection::canRasterize() const
{
	return true;
}


////////////////////////////////////////////////////////////////////////////
// GeotagRecordCollection::Iterator
////////////////////////////////////////////////////////////////////////////

GeotagRecordCollection::Iterator::Iterator(const boost::intrusive_ptr<IRecordCollection> & inputs,
											const boost::intrusive_ptr<IGeometryProvider> & geometryProvider) 
	: m_geometryProvider(geometryProvider), m_currentID(0)
{
	m_recordIterator = inputs->getIterator();
}


bool GeotagRecordCollection::Iterator::end() const
{
	return m_recordIterator->end();
}

void GeotagRecordCollection::Iterator::next()
{
	m_currentFeature.reset();

	if(!m_recordIterator->end())
	{
		m_recordIterator->next();
		m_currentID++;
	}
}

boost::intrusive_ptr<IFeature> GeotagRecordCollection::Iterator::getFeature() const
{
	if (!m_currentFeature && !end())
	{
		auto geometry =	m_geometryProvider->getGeometry(m_recordIterator->getRecord());
		m_currentFeature = Feature::create(m_recordIterator->getRecord(), geometry, StringUtils::toString(m_currentID));
	}
	return m_currentFeature;
}