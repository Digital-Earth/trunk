#pragma once

#include "PYXIS/Feature/IFeatureList.h"
#include "PYXIS/Geometry/Polygon/IPolygon.h"

namespace PYXIS
{
	namespace Geometry
	{
		class Point;
	}

	namespace Feature
	{
		// A point feature list, bounded by a polygon.
		struct IBoundedPointFeatureList : ITypedFeatureList<Geometry::Point>
		{
			// Returns the bounding polygon.
			virtual Geometry::Polygon::IPolygon const & getGeometry() const = 0;
		};
	}
}
