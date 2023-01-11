#ifndef PYXIS__DATA__FEATURE_SERIALIZER_H
#define PYXIS__DATA__FEATURE_SERIALIZER_H
/******************************************************************************
feature_serializer.h

begin		: 2019-05-16
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

#include "pyxis/data/feature.h"
#include "pyxis/data/feature_collection.h"
#include "pyxis/utility/wire_buffer.h"

// standard includes
#include <string>

/*!
A simple feature with geometry and fields.
*/
//! A feature.
class PYXLIB_DECL FeatureSerializer 
{
public: 
	static PYXPointer<PYXConstBufferSlice> serializeFeature(
		const boost::intrusive_ptr<IFeature> & feature);

	static boost::intrusive_ptr<IFeature> deserializeFeature(
		const PYXPointer<PYXTableDefinition> featureDefinition,
		const PYXConstBufferSlice & in);

public:
	static void serializeFeatureCollection(const boost::intrusive_ptr<IFeatureCollection> & featureCollection, const std::string filename);

	static const boost::intrusive_ptr<IFeatureCollection> deserializeFeatureCollection(const std::string filename);
};

#endif // guard
