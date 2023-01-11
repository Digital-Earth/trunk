#pragma once

#include "PYXIS/Pointee.h"
#include "PYXIS/Geometry/IGeometry.h"

namespace PYXIS
{
	namespace Geometry
	{
		class Curve :
			public Pointee,
			public IGeometry
		{
		};
	}
}
