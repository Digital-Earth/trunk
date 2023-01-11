/******************************************************************************
feature_serializer.cpp

begin		: 2006-10-19
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h" 
#include "pyxis/data/feature_serializer.h"

// pyxlib includes
#include "pyxis/utility/tester.h"

PYXPointer<PYXConstBufferSlice> FeatureSerializer::serializeFeature(const boost::intrusive_ptr<IFeature> & feature)
{
	PYXTHROW_NOT_IMPLEMENTED();
}

boost::intrusive_ptr<IFeature> FeatureSerializer::deserializeFeature(const PYXPointer<PYXTableDefinition> featureDefinition, const PYXConstBufferSlice & in)
{
	PYXTHROW_NOT_IMPLEMENTED();
}

void serializeFeatureCollection(const boost::intrusive_ptr<IFeatureCollection> & featureCollection, const std::string filename) 
{
	PYXTHROW_NOT_IMPLEMENTED();
}

const boost::intrusive_ptr<IFeatureCollection> deserializeFeatureCollection(const std::string filename)
{
	PYXTHROW_NOT_IMPLEMENTED();
}