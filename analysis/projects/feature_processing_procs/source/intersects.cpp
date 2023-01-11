/******************************************************************************
intersects.cpp

begin		: 2008-01-23
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE

#include "intersects.h"

#include "feature_collection_process.h"

// Required by tests
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/procs/default_feature.h"

// {3952AB85-93AB-4918-8984-7FBC897804A8}
PYXCOM_DEFINE_CLSID(Intersects,
0x3952ab85, 0x93ab, 0x4918, 0x89, 0x84, 0x7f, 0xbc, 0x89, 0x78, 0x4, 0xa8);

PYXCOM_CLASS_INTERFACES(Intersects, IProcess::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(Intersects, "Intersects", "A feature collection comprised of features intersecting a reference feature.", "Analysis/Features",
					IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeature::iid, 1, 1, "Input AOI", "The area of interest that is the basis for comparison.")
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Features", "A collection of features to filter by the AOI.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<Intersects> gTester;

}

//! Constructor
Intersects::Intersects()
{
}

//! Destructor
Intersects::~Intersects()
{
}

void Intersects::test()
{
	// Create geometry for AOI.
	PYXPointer<PYXTileCollection> spAOIGeometry = PYXTileCollection::create();
	spAOIGeometry->addTile(PYXIcosIndex("1-02"), 15);
	spAOIGeometry->addTile(PYXIcosIndex("1-03"), 15);
	spAOIGeometry->addTile(PYXIcosIndex("3-02"), 15);
	spAOIGeometry->addTile(PYXIcosIndex("3-03"), 15);

	// Create AOI.
	boost::intrusive_ptr<DefaultFeature> spAOI(new DefaultFeature(spAOIGeometry));
	spAOI->addField("rgb", PYXFieldDefinition::knContextRGB, PYXValue::knUInt8, 3);
	unsigned char buf[3] = { 255, 128, 0 };
	PYXValue v(buf, 3);
	spAOI->setFieldValueByName(v, "rgb");

	// Create another geometry for data.
	PYXPointer<PYXTileCollection> spNonAOIGeometry1 = PYXTileCollection::create();
	spNonAOIGeometry1->addTile(PYXIcosIndex("2-0200"), 14);
	spNonAOIGeometry1->addTile(PYXIcosIndex("3-0200"), 14);
	spNonAOIGeometry1->addTile(PYXIcosIndex("3-0400"), 14);
	spNonAOIGeometry1->addTile(PYXIcosIndex("4-0200"), 14);

	// Create another geometry for data.
	PYXPointer<PYXTileCollection> spNonAOIGeometry2 = PYXTileCollection::create();
	spNonAOIGeometry2->addTile(PYXIcosIndex("1-040000"), 16);
	spNonAOIGeometry2->addTile(PYXIcosIndex("2-040000"), 16);
	spNonAOIGeometry2->addTile(PYXIcosIndex("3-040000"), 16);
	spNonAOIGeometry2->addTile(PYXIcosIndex("4-040000"), 16);

	// Create data feature collection, with some that intersect and some that don't.
	boost::intrusive_ptr<DefaultFeatureCollection> spFC(new DefaultFeatureCollection);
	boost::intrusive_ptr<IFeature> spAOIFeature1(new DefaultFeature(spAOIGeometry));
	boost::intrusive_ptr<IFeature> spAOIFeature2(new DefaultFeature(spAOIGeometry));
	boost::intrusive_ptr<IFeature> spFeatureNoGeometry(new DefaultFeature);
	boost::intrusive_ptr<IFeature> spNonAOI1Feature(new DefaultFeature(spNonAOIGeometry1));
	boost::intrusive_ptr<IFeature> spNonAOI2Feature(new DefaultFeature(spNonAOIGeometry2));
	spFC->addFeature(spAOIFeature1);
	spFC->addFeature(spAOIFeature2);
	spFC->addFeature(spFeatureNoGeometry);
	spFC->addFeature(spNonAOI1Feature);
	spFC->addFeature(spNonAOI2Feature);

	// Create the process.
	boost::intrusive_ptr<IProcess> spProcess;
	PYXCOMCreateInstance(Intersects::clsid, 0, IProcess::iid, (void**)&spProcess);

	// Set the AOI parameter.
	{
		// Create the parameter.
		PYXPointer<Parameter> spParameter = spProcess->getParameter(0);

		// Create the parameter process.
		boost::intrusive_ptr<IProcess> spAOIProcess;
		spAOI->QueryInterface(IProcess::iid, (void**)&spAOIProcess);

		// Add the process to the parameter.
		spParameter->addValue(spAOIProcess);
	}

	// Set the feature collection parameter.
	{
		// Create the parameter.
		PYXPointer<Parameter> spParameter = spProcess->getParameter(1);

		// Create the parameter process.
		boost::intrusive_ptr<IProcess> spDataProcess;
		spFC->QueryInterface(IProcess::iid, (void**)&spDataProcess);

		// Add the process to the parameter.
		spParameter->addValue(spDataProcess);
	}

	// Execute the process.
	spProcess->initProc();

	// Make sure that the collection contains the right ones.
	{
		// Get the output.
		boost::intrusive_ptr<PYXCOM_IUnknown> spOutput = spProcess->getOutput();
		assert(spOutput);

		// Cast it.
		boost::intrusive_ptr<IFeatureCollection> spFCOutput;
		spOutput->QueryInterface(IFeatureCollection::iid, (void**)&spFCOutput);
		assert(spFCOutput);

		// Check it.  Should contain spAOIFeature1, spAOIFeature2, spNonAOI1Feature.
		PYXPointer<FeatureIterator> spIt = spFCOutput->getIterator();
		TEST_ASSERT(!spIt->end());
		TEST_ASSERT(spIt->getFeature().get() == spAOIFeature1.get());
		spIt->next();
		TEST_ASSERT(!spIt->end());
		TEST_ASSERT(spIt->getFeature().get() == spAOIFeature2.get());
		spIt->next();
		TEST_ASSERT(!spIt->end());
		TEST_ASSERT(spIt->getFeature().get() == spNonAOI1Feature.get());
		spIt->next();
		TEST_ASSERT(spIt->end());
	}
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

//! Initialize the process.
IProcess::eInitStatus Intersects::initImpl()
{
	// Set the ID.
	m_strID = "Intersects" + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	// Get the AOI input.
	boost::intrusive_ptr<IFeature> spAOIInput;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IFeature::iid, (void**)&spAOIInput);
	assert(spAOIInput);

	// Get the AOI geometry.
	m_spAOIGeometry = spAOIInput->getGeometry();

	// Get the feature collection input.
	m_spFeaturesInput = getParameter(1)->getValue(0)->getOutput()->QueryInterface<IFeatureCollection>();
	assert(m_spFeaturesInput);
	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
////////////////////////////////////////////////////////////////////////////////

//! Get an iterator to all the features in this collection.
PYXPointer<FeatureIterator> Intersects::getIterator() const
{
	return FilteredFeatureIterator::create(m_spFeaturesInput->getIterator(), m_spAOIGeometry);
}

//! Get an iterator to all the features in this collection that intersect this geometry.
PYXPointer<FeatureIterator> Intersects::getIterator(const PYXGeometry& geometry) const
{
	return FilteredFeatureIterator::create(m_spFeaturesInput->getIterator(geometry), m_spAOIGeometry);
}

//! Get styles that determine how to visualize features in this collection.
std::vector<FeatureStyle> Intersects::getFeatureStyles() const
{
	return m_spFeaturesInput->getFeatureStyles();
}

//! Get the feature with the specified ID.
boost::intrusive_ptr<IFeature> Intersects::getFeature(const std::string& strFeatureID) const
{
	boost::intrusive_ptr<IFeature> spPossibleFeature = m_spFeaturesInput->getFeature(strFeatureID);
	if (spPossibleFeature && match(*m_spAOIGeometry, spPossibleFeature))
	{
		return spPossibleFeature;
	}
	return boost::intrusive_ptr<IFeature>();
}

//! Get the feature definition.
PYXPointer<const PYXTableDefinition> Intersects::getFeatureDefinition() const
{
	return m_spFeaturesInput->getFeatureDefinition();
}

//! Get the feature definition.
PYXPointer<PYXTableDefinition> Intersects::getFeatureDefinition()
{
	return m_spFeaturesInput->getFeatureDefinition();
}

bool Intersects::canRasterize() const
{
	return m_spFeaturesInput->canRasterize();
}
