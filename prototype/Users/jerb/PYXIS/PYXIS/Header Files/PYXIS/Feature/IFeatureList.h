#pragma once

#include "PYXIS/Pointer.h"
#include "PYXIS/Feature/IFeature.h"
#include <memory>

namespace PYXIS
{
	namespace Geometry
	{
		struct IGeometry;
		class Raster;
	}

	namespace Feature
	{
		template <typename TGeometry> struct ITypedFeatureList;

		struct IFeatureList : IFeature
		{
			// Returns a geometry comprised of all geometries within.
			virtual Geometry::IGeometry const & getGeometry() const = 0;

			virtual void insert(std::auto_ptr<IFeature> feature) = 0;

			virtual bool contains(IFeature const & feature) const = 0;

			virtual std::auto_ptr<IFeature> remove(IFeature const & feature) = 0;

			virtual size_t count() const = 0;

			virtual Pointer< ITypedFeatureList<Geometry::Raster> > rasterize(size_t resolution) const = 0;
		};

		// The base class of all feature lists.
		template <typename TGeometry> struct ITypedFeatureList : IFeatureList
		{
			// Returns a geometry comprised of all geometries within.
			virtual Geometry::IGeometry const & getGeometry() const = 0;

			virtual void insert(std::auto_ptr< ITypedFeature<TGeometry> > feature) = 0;

			virtual bool contains(ITypedFeature<TGeometry> const & feature) const = 0;

			virtual std::auto_ptr< ITypedFeature<TGeometry> > remove(ITypedFeature<TGeometry> const & feature) = 0;

			virtual size_t count() const = 0;

			virtual Pointer< ITypedFeatureList<Geometry::Raster> > rasterize(size_t resolution) const = 0;
		};
	}
}
