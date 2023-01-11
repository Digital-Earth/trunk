#pragma once

#include "PYXIS/Pointee.h"
#include "PYXIS/Feature/IBoundedPointFeatureList.h"

namespace PYXIS
{
	namespace Feature
	{
		class PointFeatureCoverage : 
			public Pointee,
			public IBoundedPointFeatureList
		{
		};
	}
}
