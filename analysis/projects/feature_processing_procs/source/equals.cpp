/******************************************************************************
equals.cpp

begin		: 2008-01-23
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE

#include "equals.h"

#include "feature_collection_process.h"

// Required by tests
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/procs/default_feature.h"

// {62EBF561-9A7D-43dc-8912-819B27C8FB71}
PYXCOM_DEFINE_CLSID(Equals,
0x62ebf561, 0x9a7d, 0x43dc, 0x89, 0x12, 0x81, 0x9b, 0x27, 0xc8, 0xfb, 0x71);

PYXCOM_CLASS_INTERFACES(Equals, IProcess::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(Equals, "Equals", "A feature collection comprised of features equal to a reference feature.", "Analysis/Features",
					IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeature::iid, 1, 1, "Input AOI", "The area of interest that is the basis for comparison.")
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Features", "A collection of features to filter by the AOI.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<Equals> gTester;

}

//! Constructor
Equals::Equals()
{
}

//! Destructor
Equals::~Equals()
{
}

void Equals::test()
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

	// Create a non-AOI geometry.
	PYXPointer<PYXTileCollection> spNonAOIGeometry = PYXTileCollection::create();
	spNonAOIGeometry->addTile(PYXIcosIndex("1-02"), 15);
	spNonAOIGeometry->addTile(PYXIcosIndex("1-03"), 15);
	spNonAOIGeometry->addTile(PYXIcosIndex("3-02"), 15);

	// Create data feature collection, with some that equal and some that don't.
	boost::intrusive_ptr<DefaultFeatureCollection> spFC(new DefaultFeatureCollection);
	boost::intrusive_ptr<IFeature> spFeatureMatch1(new DefaultFeature(spAOIGeometry));
	boost::intrusive_ptr<IFeature> spFeatureMatch2(new DefaultFeature(spAOIGeometry));
	boost::intrusive_ptr<IFeature> spFeatureNoGeometry(new DefaultFeature);
	boost::intrusive_ptr<IFeature> spFeatureMismatch(new DefaultFeature(spNonAOIGeometry));
	spFC->addFeature(spFeatureMatch1);
	spFC->addFeature(spFeatureMatch2);
	spFC->addFeature(spFeatureNoGeometry);
	spFC->addFeature(spFeatureMismatch);

	// Create the process.
	boost::intrusive_ptr<IProcess> spProcess;
	PYXCOMCreateInstance(Equals::clsid, 0, IProcess::iid, (void**)&spProcess);
	assert(spProcess);

	// Set the AOI parameter.
	{
		// Get the parameter.
		PYXPointer<Parameter> spParameter = spProcess->getParameter(0);
		assert(spParameter);

		// Create the parameter process.
		boost::intrusive_ptr<IProcess> spAOIProcess;
		spAOI->QueryInterface(IProcess::iid, (void**)&spAOIProcess);

		// Add the process to the parameter.
		spParameter->addValue(spAOIProcess);
	}

	// Set the feature collection parameter.
	{
		// Get the parameter.
		PYXPointer<Parameter> spParameter = spProcess->getParameter(1);
		assert(spParameter);

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

		// Check it.
		PYXPointer<FeatureIterator> spIt = spFCOutput->getIterator();
		TEST_ASSERT(!spIt->end());
		TEST_ASSERT(spIt->getFeature().get() == spFeatureMatch1.get());
		spIt->next();
		TEST_ASSERT(spIt->getFeature().get() == spFeatureMatch2.get());
		spIt->next();
		TEST_ASSERT(spIt->end());
	}

	// TODO: Run a different process, with a different AOI, and remove all the rest.  Ensure an empty geometry.
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

//! Initialize the process.
IProcess::eInitStatus Equals::initImpl()
{
	// Set the ID.
	m_strID = "Equals" + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	// Get the AOI input.
	boost::intrusive_ptr<IFeature> spAOIInput;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IFeature::iid, (void**)&spAOIInput);
	assert(spAOIInput);

	// Get the AOI geometry.
	m_spAOIGeometry = spAOIInput->getGeometry();

	// Get the feature collection input.
	getParameter(1)->getValue(0)->getOutput()->QueryInterface(IFeatureCollection::iid, (void**)&m_spFeaturesInput);
	assert(m_spFeaturesInput);
	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
////////////////////////////////////////////////////////////////////////////////

//! Get an iterator to all the features in this collection.
PYXPointer<FeatureIterator> Equals::getIterator() const
{
	return FilteredFeatureIterator::create(m_spFeaturesInput->getIterator(), m_spAOIGeometry);
}

//! Get an iterator to all the features in this collection that intersect this geometry.
PYXPointer<FeatureIterator> Equals::getIterator(const PYXGeometry& geometry) const
{
	return FilteredFeatureIterator::create(m_spFeaturesInput->getIterator(geometry), m_spAOIGeometry);
}

//! Get styles that determine how to visualize features in this collection.
std::vector<FeatureStyle> Equals::getFeatureStyles() const
{
	return m_spFeaturesInput->getFeatureStyles();
}

//! Get the feature with the specified ID.
boost::intrusive_ptr<IFeature> Equals::getFeature(const std::string& strFeatureID) const
{
	boost::intrusive_ptr<IFeature> spPossibleFeature = m_spFeaturesInput->getFeature(strFeatureID);
	if (spPossibleFeature && match(*m_spAOIGeometry, spPossibleFeature))
	{
		return spPossibleFeature;
	}
	return boost::intrusive_ptr<IFeature>();
}

//! Get the feature definition.
PYXPointer<const PYXTableDefinition> Equals::getFeatureDefinition() const
{
	return m_spFeaturesInput->getFeatureDefinition();
}

//! Get the feature definition.
PYXPointer<PYXTableDefinition> Equals::getFeatureDefinition()
{
	return m_spFeaturesInput->getFeatureDefinition();
}

bool Equals::canRasterize() const
{
	return m_spFeaturesInput->canRasterize();
}
