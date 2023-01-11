/******************************************************************************
feature_collection_process.cpp

begin		: 2007-10-25
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_PYXIS_COVERAGES_SOURCE

#include "feature_collection_process.h"

// pyxlib includes
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/data/writeable_feature.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/pipe/pipe_utils.h"

// {DEF42C63-F377-4065-8C58-7215FBA45222}
PYXCOM_DEFINE_CLSID(FeatureCollectionProcess, 
0xdef42c63, 0xf377, 0x4065, 0x8c, 0x58, 0x72, 0x15, 0xfb, 0xa4, 0x52, 0x22);

PYXCOM_CLASS_INTERFACES(FeatureCollectionProcess, IProcess::iid, IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(FeatureCollectionProcess, "Feature Collection", "Combines multiple features into a feature collection", "Utility",
					IFeatureCollection::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_PARAMETER(IFeature::iid, 1, -1, "Feature", "Feature to add to the Feature Collection")
IPROCESS_SPEC_END

FeatureCollectionProcess::FeatureCollectionProcess()
{
}

IProcess::eInitStatus FeatureCollectionProcess::initImpl()
{
	boost::intrusive_ptr<IFeature> spFeature;
	if (!(getParameter(0) && getParameter(0)->getValue(0)) &&
		getParameter(0)->getValue(0)->getOutput())
	{
		PYXTHROW(PYXException, "Missing input.");
	}
	
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IFeature::iid, (void**) &spFeature);
	if (!spFeature)
	{
		PYXTHROW(PYXException, "Missing input.");
	}

	int nParamValCount = getParameter(0)->getValueCount();
	PYXPointer<Parameter> featParameter = getParameter(0);
	
	for (int nValue = 0; nValue < nParamValCount; ++nValue)
	{
		boost::intrusive_ptr<IProcess> paramValue = featParameter->getValue(nValue);

		boost::intrusive_ptr<IFeature> feature = paramValue->getOutput()->QueryInterface<IFeature>();

		/*
		boost::intrusive_ptr<IWritableFeature> wFeature;
		paramValue->QueryInterface(IWritableFeature::iid, (void**) &wFeature);

		if (wFeature)
		{
			if (!getFeature(wFeature->getID()))
			{
				if (m_vecFeatures.empty())
				{
					m_spTableDef = wFeature->getDefinition();
				}
				else if (m_spTableDef && *m_spTableDef != *(wFeature->getDefinition()))
				{
					m_spTableDef.reset();
				}
				m_vecFeatures.push_back(wFeature);
			}
		}
		else
		{
			boost::intrusive_ptr<IFeature> feature;
			paramValue->QueryInterface(IFeature::iid, (void**) &feature);
			*/
			if (!getFeature(feature->getID()))
			{
				if (m_vecFeatures.empty())
				{
					m_spTableDef = feature->getDefinition();
				}
				else if (m_spTableDef && *m_spTableDef != *(feature->getDefinition()))
				{
					m_spTableDef.reset();
				}
				m_vecFeatures.push_back(feature);
			}
		//}
	}

	// This geometry is the union of all features.
	std::vector<boost::intrusive_ptr<IFeature> >::const_iterator it = m_vecFeatures.begin();
	m_spGeom = (*it)->getGeometry()->clone();
	++it;
	while (it != m_vecFeatures.end())
	{
		m_spGeom = m_spGeom->disjunction(*((*it)->getGeometry().get()));
		++it;
	}

	m_strID = procRefToStr(ProcRef(this));
	return knInitialized;
}

/*!
Gets a feature from the feature collection based on a particular feature id. The 
feature is retrieved by iterating through all the features and comparing the feature 
ids in the feature collection to the id that is being asked for. When a match is found
that feature is returned. A null pointer to an IFeature is returned if the ID being 
requested does not exist in the feature collection.

\param strFeatId A string value containing the feature ID to get from the feature collection.

\return a Pointer to an IFeature if the feature was found or an empty pointer if it was not.
*/
boost::intrusive_ptr<IFeature> FeatureCollectionProcess::getFeature(const std::string& strFeatId) const
{
	std::vector<boost::intrusive_ptr<IFeature> >::const_iterator vecIt;
	boost::intrusive_ptr<IFeature> rtnFeat = 0;

	for (vecIt = m_vecFeatures.begin(); vecIt != m_vecFeatures.end(); ++vecIt)
	{
		if ((*vecIt)->getID().compare(strFeatId) == 0)
		{
			rtnFeat = (*vecIt);
			break;
		}
	}
	return rtnFeat;
}

/*!
Return an iterator to all of the features that intersect with the passed geometry.

\param geometry	Only features that intersect with this geometry are included in
				the iteration.

\return a PYXPointer to a feature iterator that only includes features that intersect
		with the specified geometry.
*/
PYXPointer<FeatureIterator> FeatureCollectionProcess::getIterator(const PYXGeometry &geometry) const
{
	std::vector<boost::intrusive_ptr<IFeature> > vecFeatureSet;
	for (std::vector<boost::intrusive_ptr<IFeature> >::const_iterator it = m_vecFeatures.begin();
		it != m_vecFeatures.end(); ++it)
	{
		PYXPointer<PYXGeometry> spGeomClone = (*it)->getGeometry()->clone();
		assert(spGeomClone);

		spGeomClone->setCellResolution(geometry.getCellResolution());
		if (geometry.intersects(*spGeomClone))
		{
			vecFeatureSet.push_back(*it);
		}
	}

	return FeatureCollectionIterator::create(vecFeatureSet);
}

PYXPointer<FeatureIterator> FeatureCollectionProcess::getIterator() const
{
	return FeatureCollectionIterator::create(m_vecFeatures);
}
