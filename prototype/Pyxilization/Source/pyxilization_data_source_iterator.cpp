/******************************************************************************
pyxilization_data_source_iterator.cpp

begin		: 2007-02-11
copyright	: (C) 2007 by Stephen Scovil, Sopheap Hok, Dale Offord, Nick Lipson
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyx_feature_data_source.h"
#include "pyx_feature_iterator.h"
#include "pyxilization_data_source.h"
#include "pyxilization_data_source_iterator.h"
#include "pyx_edge_iterator.h"
#include "pyx_dir_edge_iterator.h"
#include "pyx_icos_iterator.h"

PYXPointer<PyxilizationDataSourceIterator> PyxilizationDataSourceIterator::create(const PyxilizationDataSource *pPDS)
{
	return PYXPointer<PyxilizationDataSourceIterator>(new PyxilizationDataSourceIterator(pPDS));
}

PYXPointer<PyxilizationDataSourceIterator> PyxilizationDataSourceIterator::create(const PyxilizationDataSource *pPDS, const PYXGeometry &spGeo)
{
	return PYXPointer<PyxilizationDataSourceIterator>(new PyxilizationDataSourceIterator(pPDS, spGeo));
}

PyxilizationDataSourceIterator::PyxilizationDataSourceIterator(const PyxilizationDataSource *pPDS, const PYXGeometry &spGeo)
{
	const std::vector<boost::shared_ptr<PyxilizationFeature>> *units = pPDS->getUnits();
	std::vector<boost::shared_ptr<PyxilizationFeature>>::const_iterator cit;

/*	std::vector<boost::shared_ptr<PyxilizationFeature>> *vecEdges = pPDS->getEdges();
	for(unsigned int n=0;n<vecEdges->size();n++)
	{
		boost::shared_ptr<PyxilizationFeature> spEdge = (*vecEdges)[n];
		if(PyxilizationStyleMapper::getDisplayResolution() > 6)
		{
			spEdge->setResolution(PyxilizationStyleMapper::getDisplayResolution());
			m_vecUnits.push_back(spEdge);
		}
	}*/

	cit = units->begin();
	for( ;cit != units->end(); cit++)
	{
		if((*cit)->getGeometry()->intersects(&spGeo))
		{
			m_vecUnits.push_back(*cit);
		}
	}
//	m_vecUnits = *units;
	m_itUnits = m_vecUnits.begin();
	m_nPos=0;
}

PyxilizationDataSourceIterator::PyxilizationDataSourceIterator(const PyxilizationDataSource *pPDS)
{
	const std::vector<boost::shared_ptr<PyxilizationFeature>> *units = pPDS->getUnits();
	m_vecUnits = *units;

	//done test stuff
	m_itUnits = m_vecUnits.begin();
	m_nPos=0;
}

boost::shared_ptr<const PYXFeature> PyxilizationDataSourceIterator::getFeature()const
{
	return *m_itUnits;
}

boost::shared_ptr<PYXFeature> PyxilizationDataSourceIterator::getFeature()
{
	return *m_itUnits; 
}