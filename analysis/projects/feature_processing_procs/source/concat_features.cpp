/******************************************************************************
concat_features.cpp

begin		: 2012-06-11
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_FEATURE_PROCESSING_PROCS_SOURCE

#include "concat_features.h"

#include "feature_collection_process.h"
#include "pyxis/procs/default_feature.h"

// Required by tests
#include "pyxis/geometry/tile_collection.h"

// {BBDCA91A-083E-4a86-B694-6E808A62DC07}
PYXCOM_DEFINE_CLSID(ConcatFeatures, 
0xbbdca91a, 0x83e, 0x4a86, 0xb6, 0x94, 0x6e, 0x80, 0x8a, 0x62, 0xdc, 0x7);

PYXCOM_CLASS_INTERFACES(ConcatFeatures, IProcess::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(ConcatFeatures, "Concat Features", "A feature collection comprised of all features of its input feature collections.", "Development",
					IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)	
	IPROCESS_SPEC_PARAMETER(IFeatureCollection::iid, 1, -1, "Input Features Collection", "A collection of features to merge.")
IPROCESS_SPEC_END

namespace
{

//! Tester class
Tester<ConcatFeatures> gTester;

}

//! Constructor
ConcatFeatures::ConcatFeatures()
{
}

//! Destructor
ConcatFeatures::~ConcatFeatures()
{
}

void ConcatFeatures::test()
{
}

/*
//! Populate the output field.
void ConcatFeatures::populateOutput() const
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
*/

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

//! Initialize the process.
IProcess::eInitStatus ConcatFeatures::initImpl()
{
	// Set the ID.
	m_strID = "ConcatFeatures" + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	boost::shared_ptr<std::vector<boost::intrusive_ptr<IFeatureCollection>>> inputs(new std::vector<boost::intrusive_ptr<IFeatureCollection>>());

	PYXPointer<PYXTileCollection> finalGeometry = PYXTileCollection::create();

	PYXPointer<PYXTableDefinition> definition;

	int finalResolution = 0;

	for(int i = 0; i < getParameter(0)->getValueCount(); ++i)
	{
		boost::intrusive_ptr<IFeatureCollection> fc = getParameter(0)->getValue(i)->getOutput()->QueryInterface<IFeatureCollection>();
		assert(fc);
		inputs->push_back(fc);

		if (!definition)
		{
			definition = fc->getFeatureDefinition();
		}
		else 
		{
			if (*definition != *fc->getFeatureDefinition())
			{
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError("Input feature collections don't have the same definition");
				return knFailedToInit;
			}
		}
		
		if (finalResolution == 0)
		{
			finalResolution = fc->getGeometry()->getCellResolution();
		}

		PYXTileCollection geometry;
		fc->getGeometry()->copyTo(&geometry,finalResolution);
		geometry.limitCellsCountTo(10000);
		finalGeometry->addGeometry(geometry);
		finalGeometry->limitCellsCountTo(10000);
		finalResolution = finalGeometry->getCellResolution();
	}

	m_inputFCs = inputs;
	m_spGeometry = finalGeometry;
	m_featuresDefinition = definition;

	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// IFeatureCollection
////////////////////////////////////////////////////////////////////////////////

//! Get an iterator to all the features in this collection.
PYXPointer<FeatureIterator> ConcatFeatures::getIterator() const
{
	return Iterator::create(m_inputFCs,0);
}

//! Get an iterator to all the features in this collection that intersect this geometry.
PYXPointer<FeatureIterator> ConcatFeatures::getIterator(const PYXGeometry& geometry) const
{
	return Iterator::create(m_inputFCs,geometry.clone());
}

//! Get styles that determine how to visualize features in this collection.
std::vector<FeatureStyle> ConcatFeatures::getFeatureStyles() const
{
	return std::vector<FeatureStyle>();
}

//! Get the feature with the specified ID.
boost::intrusive_ptr<IFeature> ConcatFeatures::getFeature(const std::string& strFeatureID) const
{
	boost::shared_ptr<std::vector<boost::intrusive_ptr<IFeatureCollection>>> inputs = m_inputFCs;

	size_t pos = strFeatureID.find("-");

	int dataLocation = StringUtils::fromString<int>(strFeatureID.substr(0,pos));
	return (*inputs)[dataLocation]->getFeature(strFeatureID.substr(pos+1));
}

//! Get the feature definition.
PYXPointer<const PYXTableDefinition> ConcatFeatures::getFeatureDefinition() const
{
	return m_featuresDefinition;
}

//! Get the feature definition.
PYXPointer<PYXTableDefinition> ConcatFeatures::getFeatureDefinition()
{
	return m_featuresDefinition;
}

bool ConcatFeatures::canRasterize() const
{
	return true;
}


////////////////////////////////////////////////////////////////////////////
// ConcatFeatures::Iterator
////////////////////////////////////////////////////////////////////////////

ConcatFeatures::Iterator::Iterator(const boost::shared_ptr<FeatureCollectionVector> inputs,const PYXPointer<PYXGeometry> & geometry) 
	: m_inputs(inputs), m_geometry(geometry), m_curentDatasetIndex(0)
{
	m_currentDataset =  m_inputs->begin();

	findNextNonEmptyDataset();
}

void ConcatFeatures::Iterator::findNextNonEmptyDataset()
{
	while (m_currentDataset != m_inputs->end())
	{
		if (m_geometry)
		{
			m_currentDatasetIterator = (*m_currentDataset)->getIterator(*m_geometry);
		}
		else 
		{
			m_currentDatasetIterator = (*m_currentDataset)->getIterator();
		}

		if (!m_currentDatasetIterator->end())
		{
			break;
		}

		++m_curentDatasetIndex;
		++m_currentDataset;
	}
}

bool ConcatFeatures::Iterator::end() const
{
	return m_currentDataset == m_inputs->end() && m_currentDatasetIterator->end();
}

void ConcatFeatures::Iterator::next()
{
	if (end())
	{
		return;
	}

	if (!m_currentDatasetIterator->end())
	{
		m_currentDatasetIterator->next();

		if (!m_currentDatasetIterator->end())
		{
			return;
		}
	}

	++m_curentDatasetIndex;
	++m_currentDataset;

	findNextNonEmptyDataset();
}

boost::intrusive_ptr<IFeature> ConcatFeatures::Iterator::getFeature() const
{
	if (end())
	{
		return 0;
	}
	return Feature::create(m_currentDatasetIterator->getFeature(),m_curentDatasetIndex);
}