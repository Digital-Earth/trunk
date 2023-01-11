#ifndef PYXIS__REGION__XY_BOUNDS_REGION_H
#define PYXIS__REGION__XY_BOUNDS_REGION_H
/******************************************************************************
xy_bounds_region.h

begin		: 2011-02-05
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/region/region.h"
#include "pyxis/derm/index.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/utility/rect_2d.h"
#include "pyxis/utility/bbox_lat_lon.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/wire_buffer.h"

#include <vector>

//! Represents a native XY bounding box projected onto the globe.
class PYXLIB_DECL PYXXYBoundsRegion : public PYXVectorRegion
{
//PYXRegion API
public:
	//! Make a copy of the region.
	virtual PYXPointer<IRegion> clone() const;

	//! Get the number of vertices of the region
	virtual int getVerticesCount() const
	{
		return 4;
	}

	virtual PYXPointer<IRegionVisitor> getVisitor() const
	{
		return PYXVectorRegionVisitor::create(this);
	}

	virtual void serializeVisitor(PYXWireBuffer & buffer,const PYXPointer<IRegionVisitor> & visitor) const
	{
		return;
	}

	virtual PYXPointer<IRegionVisitor> deserializeVisitor(PYXWireBuffer & buffer) const
	{
		return PYXVectorRegionVisitor::create(this);
	}

	//! Get the distance in radians from an xyz point on the unit sphere to the border of the bounding region.
	virtual double getDistanceToBorder(const PYXCoord3DDouble& location, double fErrorThreshold = 0.0) const;

	//! Determine if an xyz point on the unit sphere is contained within the bounding region.
	virtual bool isPointContained(const PYXCoord3DDouble& location, double fErrorThreshold = 0.0) const;
	
	//! Get the bounding circle of the region.
	virtual PYXBoundingCircle getBoundingCircle() const;

//members
private:
	//! The boundary of the geometry in native (non-PYXIS) coordinates
	PYXRect2DDouble m_nativeBounds;

	//! The estimated boundary of the geometry in geocentric lat lon coordinates - used as an optimization to avoid reverse projections
	BBoxLatLon m_latLonBounds;

	//! Converts between native and geocentric lat lon coordinates
	boost::intrusive_ptr<ICoordConverter> m_coordConverter;

	//! The boundary of the geometry as a circle
	PYXBoundingCircle m_boundingCircle;

public:

	/*!
	Create a new XYBounds region.

	\param	bounds			The bounding rectangle in native coordinates
	\param	coordConverter	Used to convert between native and geocentric lat lon coordinates

	\return	A new XYBounds region.
	*/
	static PYXPointer<PYXXYBoundsRegion> create(	const PYXRect2DDouble& bounds,
													const ICoordConverter& coordConverter	)
	{
		return PYXNEW(PYXXYBoundsRegion, bounds, coordConverter);
	}

	/*!
	Clone an XYBounds region.

	\param boundsRegion	The region to clone.

	\return a new XYBounds region.
	*/
	static PYXPointer<PYXXYBoundsRegion> create(const PYXXYBoundsRegion& boundsRegion)
	{
		return PYXNEW(PYXXYBoundsRegion, boundsRegion);
	}

	//! Constructor
	explicit PYXXYBoundsRegion(
		const PYXRect2DDouble& nativeBounds,
		const ICoordConverter& coordConverter);

	//! Get the bounds of the xy region in native coordinates.
	const PYXRect2DDouble& getBounds() const { return m_nativeBounds; }

	//! Get the converted used to convert between native and geocentric lat lon coordinates.
	const boost::intrusive_ptr<ICoordConverter> & getCoordConverter() const { return m_coordConverter; }

private:
	//! Create the bounding circle for the region.
	void createBoundingCircle();

	//! Create the bounding box for the region in geocentric lat lon coordinates.
	void createLatLonBoundingBox();

	//! Get the centre of the bounding region as an xyz coordinate on the unit sphere.
	PYXCoord3DDouble getBoundsCenter() const;
};

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXXYBoundsRegion & region);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXXYBoundsRegion> & region);

#endif // guard
