#pragma once

#include "PYXIS/Pointee.h"
#include "PYXIS/Geometry/Polygon/IPolygon.h"

namespace PYXIS
{
	namespace Geometry
	{
		namespace Polygon
		{
			class Cell :
				public Pointee,
				public IPolygon
			{
			};
		}
	}
}
