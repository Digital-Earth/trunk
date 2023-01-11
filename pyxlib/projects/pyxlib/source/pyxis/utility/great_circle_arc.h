#ifndef PYXIS__UTILITY__GREAT_CIRLE_ARC_H
#define PYXIS__UTILITY__GREAT_CIRLE_ARC_H
/******************************************************************************
great_circle_arc.h

begin		: 2005-04-04
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/coord_lat_lon.h"

/*!
GreatCircleArc represents a great circle arc on a sphere.
*/
//! Represents a great circle arc.
class PYXLIB_DECL GreatCircleArc
{
public:

	//! Test method
	static void test();

	//! Constructor
	GreatCircleArc(const CoordLatLon& pt1, const CoordLatLon& pt2);

	//! Destructor
	virtual ~GreatCircleArc() {;}

	//! Calculate the distance in radians.
	static double calcDistance(	const CoordLatLon& pt1,
								const CoordLatLon& pt2,
								double fRadius = 1.0	);

	//! Get the start location
	const CoordLatLon& getPoint1() const {return m_pt1;}

	//! Get the end location
	const CoordLatLon& getPoint2() const {return m_pt2;}

	//! Get the distance in radians of the great circle arc.
	double getDistance() const {return m_fDistance;}

	//! Get a point a fraction of the way along the great circle arc.
	CoordLatLon getPoint(double fFraction) const;

protected:

private:
	
	//! Disable default constructor
	GreatCircleArc();

	//! The first point
	CoordLatLon m_pt1;

	//! The second point
	CoordLatLon m_pt2;

	//! The distance in radians
	double m_fDistance;

	//! The sin of the distance
	double m_fSinDistance;
};

#endif // guard
