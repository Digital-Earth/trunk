#ifndef PYXIS__DATA_SOURCE__FEATURE_ITERATOR_LINQ_H
#define PYXIS__DATA_SOURCE__FEATURE_ITERATOR_LINQ_H
/******************************************************************************
feature_iterator_linq.h

begin		: 2013-01-09
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/data/feature_collection.h"
#include "pyxis/utility/abstract_iterator.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/coord_3d.h"
#include "pyxis/data/feature_group.h"

#include "boost/function.hpp"
//! Abstract base for classes that iterate over PYXIS features.
/*!
The PYXFeatureIterator iterates over a collection of features.
*/
class PYXLIB_DECL PYXFeatureIteratorLinq
{
public:
	PYXFeatureIteratorLinq(const boost::intrusive_ptr<IFeature> & feature);

	PYXFeatureIteratorLinq(PYXPointer<FeatureIterator> iterator)
	{
		m_iterator = iterator;
	}

	PYXFeatureIteratorLinq single() const;
	PYXFeatureIteratorLinq take(int amount) const;
	PYXFeatureIteratorLinq skip(int amount) const;
	PYXFeatureIteratorLinq orderBy(boost::function< bool (PYXPointer<IFeature>,PYXPointer<IFeature>) > orderFunction) const;
	PYXFeatureIteratorLinq filter(boost::function< bool (PYXPointer<IFeature>) > filterFunction) const;
	PYXFeatureIteratorLinq filter(const PYXGeometry & geometry) const;
	PYXFeatureIteratorLinq filter(const PYXPointer<PYXGeometry> & geometry) const;
	PYXFeatureIteratorLinq selectMany(boost::function< PYXPointer<FeatureIterator> (const boost::intrusive_ptr<IFeature> &) > populateFunction) const;
	static PYXFeatureIteratorLinq expandGroupToGeometry(PYXPointer<IFeature> feature, const PYXPointer<PYXGeometry> & geometry);
	PYXFeatureIteratorLinq orderForTrip(PYXCoord3DDouble startLocation) const;

public:
	operator const PYXPointer<FeatureIterator> & () const
	{
		return m_iterator;
	}

private:
	PYXPointer<FeatureIterator> m_iterator;
};

#endif // guard
