#pragma once

#include "PYXIS/IPointee.h"
#include "PYXIS/Pointer.h"
#include "PYXIS/Geometry/Relationship.h"

namespace PYXIS
{
	namespace Geometry
	{
		class Raster;

		// The base class of all geometries.
		struct IGeometry : IPointee
		{
			// Returns a raster representation, at the specified resolution, of the geometry.
			virtual Pointer<Raster> rasterize(size_t resolution) const = 0;

			// Returns the spatial topological relationship between the two geometries.
			virtual Relationship relationship(const IGeometry& geometry) const = 0;

			// Returns the spatial intersection of the two geometries.
			virtual Pointer<IGeometry> spatialIntersection(const IGeometry& geometry) const = 0;

			// Returns the spatial intersection of the two geometries. rasterized at the given resolution,
			// allowing for an optimized alternative to computing the precise intersection in vector
			// space with the other form of intersection and then projecting onto a resolution via rasterGeometry. 
			virtual Pointer<Raster> spatialIntersection(const IGeometry& geometry, size_t resolution) const = 0;

			// Returns the spatial union of the two geometries.
			virtual Pointer<IGeometry> spatialUnion(const IGeometry& geometry) const = 0;
		};
	}
}
