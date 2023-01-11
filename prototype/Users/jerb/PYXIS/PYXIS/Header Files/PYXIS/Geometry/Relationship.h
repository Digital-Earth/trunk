#pragma once

namespace PYXIS
{
	namespace Geometry
	{
		// Indicates a topological relationship of a geometry to another.
		enum Relationship
		{
			isDisjointFrom,
			contains,
			isInside,
			isEqualTo,
			meets,
			covers,
			isCoveredBy,
			overlaps
		};
	}
}
