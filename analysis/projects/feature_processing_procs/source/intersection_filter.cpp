/******************************************************************************
intersection_filter.cpp

begin		: 2011-02-18
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE

#include "intersection_filter.h"
#include "feature_collection_process.h"

#include "pyxis/data/feature_collection_cache.h"

#include <pyxis/grid/dodecahedral/tree.hpp>

// {6FCF0836-E1E1-43ba-9597-9EAF80AB72F3}
PYXCOM_DEFINE_CLSID(IntersectionFilter,
0x6fcf0836, 0xe1e1, 0x43ba, 0x95, 0x97, 0x9e, 0xaf, 0x80, 0xab, 0x72, 0xf3);

PYXCOM_CLASS_INTERFACES(IntersectionFilter, IProcess::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(IntersectionFilter, "Intersection Filter", "A feature collection comprised of features intersecting a reference feature.", "Development/Broken",
	IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_PARAMETER(IFeature::iid, 1, 1, "Input Feature", "The feature whose region is the basis for intersection.")
IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Features", "A collection of features to filter by intersection with the feature region.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester< IntersectionFilter > gTester;

}

//! Constructor
IntersectionFilter::IntersectionFilter() :
m_spFeatureCollection(),
m_spFeatureCollectionCache(),
m_spAOIGeometry(),
m_defaultResolution(11),
m_resolution(m_defaultResolution)
{
}

//! Destructor
IntersectionFilter::~IntersectionFilter()
{
}

void IntersectionFilter::test()
{
	// TODO
}

////////////////////////////////////////////////////////////////////////////////
// IFeature
////////////////////////////////////////////////////////////////////////////////

bool IntersectionFilter::isWritable() const
{
	assert(m_spFeatureCollection);
	return m_spFeatureCollection->isWritable();
}

// TODO: Remove IFeature::getGeometry(); deprecated.
PYXPointer< PYXGeometry > IntersectionFilter::getGeometry()
{
	// This is a temporary hack so that we return something valid until 
	// PYXGeometry is removed from features.
	assert(m_spAOIGeometry);
	return PYXGlobalGeometry::create(m_spAOIGeometry->getResolution());
}

// TODO: Remove IFeature::getGeometry(); deprecated.
PYXPointer< PYXGeometry const > IntersectionFilter::getGeometry() const
{
	// This is a temporary hack so that we return something valid until 
	// PYXGeometry is removed from features.
	assert(m_spAOIGeometry);
	return PYXGlobalGeometry::create(m_spAOIGeometry->getResolution());
}

std::string IntersectionFilter::getStyle() const
{
	assert(m_spFeatureCollection);
	return m_spFeatureCollection->getStyle();
}

std::string IntersectionFilter::getStyle(std::string const & strStyleToGet) const
{
	assert(m_spFeatureCollection);
	return m_spFeatureCollection->getStyle(strStyleToGet);
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::string IntersectionFilter::getAttributeSchema() const
{
	std::string strSchema =
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
			"<xs:element name=\"IntersectionFilter\">"
				"<xs:complexType>"
					"<xs:sequence>"
						"<xs:element name=\"resolution\" type=\"xs:int\" default=\"";
	strSchema += StringUtils::toString< int >(m_defaultResolution);
	strSchema +=		"\">"
							"<xs:annotation>"
								"<xs:appinfo>"
									"<friendlyName>Resolution</friendlyName>"
									"<description></description>"
								"</xs:appinfo>"
							"</xs:annotation>"
						"</xs:element>"
					"</xs:sequence>"
				"</xs:complexType>"
			"</xs:element>"
		"</xs:schema>";

	return strSchema;
}

std::map< std::string, std::string > IntersectionFilter::getAttributes() const
{
	std::map< std::string, std::string > mapAttr;

	mapAttr["resolution"] = StringUtils::toString< int >(m_resolution);

	return mapAttr;
}

void IntersectionFilter::setAttributes(
	std::map< std::string, std::string > const & mapAttr)
{
	std::map< std::string, std::string >::const_iterator it;

	it = mapAttr.find("resolution");
	m_resolution = (it == mapAttr.end()) ? m_defaultResolution : StringUtils::fromString< int >(it->second);

	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;
}

////////////////////////////////////////////////////////////////////////////////
// ProcessImpl
////////////////////////////////////////////////////////////////////////////////

//! Initialize the process.
IProcess::eInitStatus IntersectionFilter::initImpl()
{
	// Set the ID.
	m_strID = "IntersectionFilter" + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	// Get the AOI feature.
	boost::intrusive_ptr< IFeature > spAOIFeature = getParameter(0)->getValue(0)->getOutput()->QueryInterface< IFeature >();
	assert(spAOIFeature);

	// Get the AOI region.
	PYXPointer< IRegion const > spRegion = spAOIFeature->getRegion();
	assert(spRegion);

	// Get the AOI resolution.
	Pyxis::Grid::Dodecahedral::Resolution resolution(m_resolution);

	// Get the AOI geometry.
	m_spAOIGeometry = new Geometry(resolution);
	assert(m_spAOIGeometry);
	Geometry fullGeometry(resolution);
	fullGeometry.setIsFull();
	spRegion->getIntersection(fullGeometry, m_spAOIGeometry.get(), m_spAOIGeometry.get());

	// Get the feature collection input.
	m_spFeatureCollection = getParameter(1)->getValue(0)->getOutput()->QueryInterface< IFeatureCollection >();
	assert(m_spFeatureCollection);

	// Set the feature collection cache.
	m_spFeatureCollectionCache = FeatureCollectionCache::create(m_spFeatureCollection);

	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
////////////////////////////////////////////////////////////////////////////////

//! Get an iterator to all the features in this collection.
PYXPointer< FeatureIterator > IntersectionFilter::getIterator() const
{
	assert(m_spFeatureCollectionCache);
	assert(m_spAOIGeometry);
	return FeatureCollectionCache::IntersectionIterator::create(m_spFeatureCollectionCache, m_spAOIGeometry);
}

//! Get an iterator to all the features in this collection that intersect this geometry.
PYXPointer< FeatureIterator > IntersectionFilter::getIterator(PYXGeometry const & geometry) const
{
	// This is a temporary fill-in until PYXGeometry gets removed from features (deprecated).
	return getIterator();
}

//! Get styles that determine how to visualize features in this collection.
std::vector< FeatureStyle > IntersectionFilter::getFeatureStyles() const
{
	assert(m_spFeatureCollection);
	return m_spFeatureCollection->getFeatureStyles();
}

//! Get the feature with the specified ID.
boost::intrusive_ptr< IFeature > IntersectionFilter::getFeature(std::string const & strFeatureID) const
{
	assert(m_spFeatureCollectionCache);
	assert(m_spAOIGeometry);
	return m_spFeatureCollectionCache->getFeature(strFeatureID, *m_spAOIGeometry);
}

//! Get the feature definition.
PYXPointer< PYXTableDefinition > IntersectionFilter::getFeatureDefinition()
{
	assert(m_spFeatureCollection);
	return m_spFeatureCollection->getFeatureDefinition();
}

//! Get the feature definition.
PYXPointer< PYXTableDefinition > IntersectionFilter::getFeatureDefinition() const
{
	assert(m_spFeatureCollection);
	return m_spFeatureCollection->getFeatureDefinition();
}

bool IntersectionFilter::canRasterize() const
{
	assert(m_spFeatureCollection);
	return m_spFeatureCollection->canRasterize();
}
