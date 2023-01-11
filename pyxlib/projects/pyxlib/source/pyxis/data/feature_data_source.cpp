/******************************************************************************
feature_data_source.cpp

begin		: 2004-10-19
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE 
#include "pyxis/data/feature_data_source.h"

// local includes
#include "pyxis/sampling/spatial_reference_system.h"

/*!
Call this method after the data source is opened to determine if the data
source contains a spatial reference. If not, a spatial reference must be
supplied by calling setSpatialReference() before getFeatureIterator() is
called.

\return	true if the data source has a spatial reference, otherwise false
*/
bool PYXFeatureDataSource::hasSpatialReferenceSystem() const {return true;}

/*!
Specify the spatial reference for the data source. Call this method to set
the spatial reference if after the data source is opened
hasSpatialReference() returns false.

\param	spSRS	The spatial reference system.
*/
void PYXFeatureDataSource::setSpatialReferenceSystem(
	PYXPointer<PYXSpatialReferenceSystem> spSRS	)
{
	assert(false);
}

