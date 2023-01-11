/******************************************************************************
intersection.cpp

begin		: 2008-01-25
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE

#include "intersection.h"

#include "feature_collection_process.h"
#include "pyxis/procs/default_feature.h"

// Required by tests
#include "pyxis/geometry/tile_collection.h"

// {04B24097-E91E-4bf9-9970-7E59ECFB7C36}
PYXCOM_DEFINE_CLSID(Intersection, 
0x4b24097, 0xe91e, 0x4bf9, 0x99, 0x70, 0x7e, 0x59, 0xec, 0xfb, 0x7c, 0x36);

PYXCOM_CLASS_INTERFACES(Intersection, IProcess::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(Intersection, "Intersection", "A feature collection comprised of features intersecting a reference feature, and whose geometries include only the intersecting region.", "Development/Broken",
					IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IFeature::iid, 1, 1, "Input AOI", "The area of interest that is the basis for comparison.")
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, 1, "Input Features", "A collection of features to filter by, and intersect with, the AOI.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<Intersection> gTester;

}

//! Constructor
Intersection::Intersection() : m_bOutputPopulated(false)
{
}

//! Destructor
Intersection::~Intersection()
{
}

void Intersection::test()
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
	PYXCOMCreateInstance(Intersection::clsid, 0, IProcess::iid, (void**)&spProcess);
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
		// Create the parameter.
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

		// Check it.  Should contain spAOIFeature1, spAOIFeature2, spNonAOI1Feature.
		PYXPointer<FeatureIterator> spIt = spFCOutput->getIterator();
		TEST_ASSERT(!spIt->end());
		TEST_ASSERT(*(spIt->getFeature()->getGeometry()) == *spAOIGeometry);
		spIt->next();
		TEST_ASSERT(!spIt->end());
		TEST_ASSERT(*(spIt->getFeature()->getGeometry()) == *spAOIGeometry);
		spIt->next();
		TEST_ASSERT(!spIt->end());
		TEST_ASSERT(*(spIt->getFeature()->getGeometry()) == *(spAOIGeometry->intersection(*spNonAOIGeometry1)));
		spIt->next();
		TEST_ASSERT(spIt->end());
	}
}

//! Populate the output field.
void Intersection::populateOutput() const
{
	// This must be called after initProc() is called.
	assert(m_spFeaturesInput);
	assert(m_spAOIGeometry);

	// Get the process.
	boost::intrusive_ptr<IProcess> spOutputProcess;
	m_spOutput->QueryInterface(IProcess::iid, (void**)&spOutputProcess);
	assert(spOutputProcess);

	// Create the parameter.
	PYXPointer<Parameter> spParameter = spOutputProcess->getParameter(0);

	// Iterate through the input features; transform and, if result is non-empty geometry, add to result.
	for (PYXPointer<FeatureIterator> spIterator = m_spFeaturesInput->getIterator(); !spIterator->end(); spIterator->next())
	{
		// Get the feature.
		boost::intrusive_ptr<IFeature> spInputFeature = spIterator->getFeature();
		assert(spInputFeature);

		// Get the geometry.
		PYXPointer<const PYXGeometry> spInputGeometry = spInputFeature->getGeometry();
		if (spInputGeometry)
		{
			// Transform.
			PYXPointer<PYXGeometry> spOutputGeometry = m_spAOIGeometry->intersection(*spInputGeometry);

			// If non-empty, create output feature containing transformed geometry and original data, and add to parameter.
			if (!spOutputGeometry->isEmpty())
			{
				// Create output feature, using output geometry.
				boost::intrusive_ptr<IFeature> spOutputFeature(new DefaultFeature(spOutputGeometry));

				// Set the data from the input feature.
				spOutputFeature->setFieldValues(spInputFeature->getFieldValues());

				// Get output feature as process.
				boost::intrusive_ptr<IProcess> spOutputFeatureProcess;
				spOutputFeature->QueryInterface(IProcess::iid, (void**)&spOutputFeatureProcess);
				assert(spOutputFeatureProcess);

				// Initialize the process.
				spOutputFeatureProcess->initProc();

				// Add the value.
				spParameter->addValue(spOutputFeatureProcess);
			}
		}
	}

	// Initialize the process.  Otherwise, the inputs won't get added to the collection.
	spOutputProcess->initProc();

	m_bOutputPopulated = true;
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

//! Initialize the process.
IProcess::eInitStatus Intersection::initImpl()
{
	// Set the ID.
	m_strID = "Intersection" + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	// Get the AOI input.
	boost::intrusive_ptr<IFeature> spAOIInput = 0;
	assert(getParameter(0));
	assert(getParameter(0)->getValueCount() == 1);
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IFeature::iid, (void**)&spAOIInput);
	assert(spAOIInput);

	// Get the AOI geometry.
	m_spAOIGeometry = spAOIInput->getGeometry();

	// Get the feature collection input.
	m_spFeaturesInput = 0;
	assert(getParameter(1));
	assert(getParameter(1)->getValueCount() == 1);
	m_spFeaturesInput = getParameter(1)->getValue(0)->getOutput()->QueryInterface<IFeatureCollection>();
	assert(m_spFeaturesInput);

	// Create the output.
	m_spOutput = 0;
	PYXCOMCreateInstance(FeatureCollectionProcess::clsid, 0, IFeatureCollection::iid, (void**)&m_spOutput);
	assert(m_spOutput);
	m_bOutputPopulated = false;
	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
////////////////////////////////////////////////////////////////////////////////

//! Get an iterator to all the features in this collection.
PYXPointer<FeatureIterator> Intersection::getIterator() const
{
	if (!m_bOutputPopulated)
	{
		populateOutput();
	}
	return m_spOutput->getIterator();
}

//! Get an iterator to all the features in this collection that intersect this geometry.
PYXPointer<FeatureIterator> Intersection::getIterator(const PYXGeometry& geometry) const
{
	if (!m_bOutputPopulated)
	{
		populateOutput();
	}
	return m_spOutput->getIterator();
}

//! Get styles that determine how to visualize features in this collection.
std::vector<FeatureStyle> Intersection::getFeatureStyles() const
{
	return m_spOutput->getFeatureStyles();
}

//! Get the feature with the specified ID.
boost::intrusive_ptr<IFeature> Intersection::getFeature(const std::string& strFeatureID) const
{
	if (!m_bOutputPopulated)
	{
		populateOutput();
	}
	return m_spOutput->getFeature(strFeatureID);
}

//! Get the feature definition.
PYXPointer<const PYXTableDefinition> Intersection::getFeatureDefinition() const
{
	return m_spOutput->getFeatureDefinition();
}

//! Get the feature definition.
PYXPointer<PYXTableDefinition> Intersection::getFeatureDefinition()
{
	return m_spOutput->getFeatureDefinition();
}

bool Intersection::canRasterize() const
{
	return m_spOutput->canRasterize();
}
