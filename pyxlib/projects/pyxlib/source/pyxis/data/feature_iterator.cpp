/******************************************************************************
feature_iterator.cpp

begin		: 2006-11-21
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE 
#include "pyxis/data/feature_iterator.h"

/*!
Get the current feature.

\return	An empty shared pointer.
*/
PYXPointer<const PYXFeature> PYXEmptyFeatureIterator::getFeature() const
{
	return 0;
}
