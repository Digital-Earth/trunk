#pragma once

#include "PYXIS/Pointee.h"
#include "PYXIS/Geometry/Polygon/IPolygon.h"

namespace PYXIS
{
	namespace Geometry
	{
		namespace Polygon
		{
			class Polygon :
				public Pointee,
				public IPolygon
			{
			};
		}
	}
}
