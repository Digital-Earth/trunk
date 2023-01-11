#ifndef PYXILIZATION_DATA_SOURCE_ITERATOR_H
#define PYXILIZATION_DATA_SOURCE_ITERATOR_H

/******************************************************************************
pyxilization_data_source_iterator.h

begin		: 2007-02-11
copyright	: (C) 2007 by Stephen Scovil, Sopheap Hok, Dale Offord, Nick Lipson
web			: www.pyxisinnovation.com
******************************************************************************/


#include <boost/shared_ptr.hpp>

class PyxilizationFeature;
class PyxilizationDataSource;

class PyxilizationDataSourceIterator: public PYXFeatureIterator
{
private:
	int m_nPos;
	std::vector<boost::shared_ptr<PyxilizationFeature>> m_vecUnits;
	std::vector<boost::shared_ptr<PyxilizationFeature>>::iterator m_itUnits;
public:
	//! Move to the next feature.
	virtual void next(){m_itUnits++;}

	//! See all the features have been.
	virtual bool end() const
	{
		return (m_vecUnits.end() == m_itUnits);
	}

	//! Get the current PYXIS feature.
	virtual boost::shared_ptr<const PYXFeature> getFeature() const;
	virtual boost::shared_ptr<PYXFeature> getFeature();

	static PYXPointer<PyxilizationDataSourceIterator> create(const PyxilizationDataSource *pPDS);

	static PYXPointer<PyxilizationDataSourceIterator> create(const PyxilizationDataSource *pPDS, const PYXGeometry &spGeo);

	//! iterator for all features of datasource pPDS
	PyxilizationDataSourceIterator(const PyxilizationDataSource *pPDS);

	//! iterator that only fits a particular geometry for datasource pPDS
	PyxilizationDataSourceIterator(const PyxilizationDataSource *pPDS, const PYXGeometry &spGeo);
};

#endif