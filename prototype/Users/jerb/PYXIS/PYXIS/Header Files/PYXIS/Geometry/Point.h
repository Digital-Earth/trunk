#pragma once

#include "PYXIS/Pointee.h"
#include "PYXIS/Geometry/IGeometry.h"

namespace PYXIS
{
	namespace Geometry
	{
		class Point :
			public Pointee,
			public IGeometry
		{
		};
	}
}
