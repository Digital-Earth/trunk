#pragma once

#include "PYXIS/Pointee.h"
#include "PYXIS/Geometry/IGeometrySet.h"

namespace PYXIS
{
	namespace Geometry
	{
		class Cell;

		// A set of cells at a single resolution.
		class Raster :
			public Pointee,
			public ITypedGeometrySet<Cell>
		{
		};
	}
}
