#pragma once

#include "PYXIS/Pointee.h"
#include "PYXIS/Feature/IFeatureList.h"

namespace PYXIS
{
	namespace Geometry
	{
		class Curve;
		class Cell;
		class Raster;
		class Point;

		namespace Polygon
		{
			struct IPolygon;
			class Polygon;
		}
	}

	namespace Feature
	{
		// A general feature list.
		template <typename TGeometry> class FeatureList :
			public Pointee,
			public ITypedFeatureList<TGeometry>
		{
		};

		// A curve feature list.
		template <> class FeatureList<Geometry::Curve> :
			public Pointee,
			public ITypedFeatureList<Geometry::Curve>
		{
		};

		// A polygon feature list.
		template <> class FeatureList<Geometry::Polygon::Polygon> :
			public Pointee,
			public ITypedFeatureList<Geometry::Polygon::Polygon>
		{
		};

		// A cell feature list.
		template <> class FeatureList<Geometry::Cell> :
			public Pointee,
			public ITypedFeatureList<Geometry::Cell>
		{
		};

		// A raster feature list.
		template <> class FeatureList<Geometry::Raster> :
			public Pointee,
			public ITypedFeatureList<Geometry::Raster>
		{
		};

		// A point feature list.
		template <> class FeatureList<Geometry::Point> :
			public Pointee,
			public ITypedFeatureList<Geometry::Point>
		{
		};
	}
}
