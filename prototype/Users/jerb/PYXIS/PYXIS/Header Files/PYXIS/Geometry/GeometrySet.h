#pragma once

#include "PYXIS/Pointee.h"
#include "PYXIS/Geometry/IGeometrySet.h"

namespace PYXIS
{
	namespace Geometry
	{
		class Cell;

		// A general set of geometries.
		template <typename TGeometry> class GeometrySet : 
			public Pointee,
			public ITypedGeometrySet<TGeometry>
		{
		};

		// A set of cells at any resolution.
		template <> class GeometrySet<Cell> :
			public Pointee,
			public ITypedGeometrySet<Cell>
		{
		};
	}
}
