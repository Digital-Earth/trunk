#pragma once

#include "PYXIS/IPointee.h"

namespace PYXIS
{
	namespace Geometry
	{
		struct IGeometry;
	}

	namespace Feature
	{
		// The base class of all features.
		struct IFeature : IPointee
		{
			virtual Geometry::IGeometry const & getGeometry() const = 0;
		};

		template <typename TGeometry> struct ITypedFeature : IFeature
		{
			virtual TGeometry const & getGeometry() const = 0;
		};
	}
}
