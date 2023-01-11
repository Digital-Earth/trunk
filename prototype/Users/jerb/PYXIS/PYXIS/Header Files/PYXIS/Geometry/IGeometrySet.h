#pragma once

#include "PYXIS/Geometry/IGeometry.h"

namespace PYXIS
{
	namespace Geometry
	{
		// Exposes a heterogenous Geometry set collection interface.
		struct IGeometrySet : IGeometry
		{
			// Returns the union of all geometries within.
			virtual Pointer<IGeometry> spatialUnion() const = 0;
		};

		// Exposes a homogenous Geometry set collection interface.
		template <typename TGeometry> struct ITypedGeometrySet : IGeometrySet
		{
		};
	}
}
