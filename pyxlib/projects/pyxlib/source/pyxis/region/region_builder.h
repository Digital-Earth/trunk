#ifndef PYXIS__REGION__REGION_BUILDER_H
#define PYXIS__REGION__REGION_BUILDER_H
/******************************************************************************
region_builder.h

begin		: 2014-02-09
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/region/region.h"
#include "pyxis/region/curve_region.h"
#include "pyxis/geometry/vector_geometry2.h"
#include "pyxis/derm/point_location.h"


class PYXLIB_DECL PYXRegionBuilder
{
private:
	double m_nearestVerticesDistance;
	std::vector<PYXCoord3DDouble> m_currentVertices;
	std::vector< PYXPointer<PYXCurveRegion> > m_currentCurves;

public:
	PYXRegionBuilder();

	void addVertex(const PointLocation & location);
	void addVertex(const PYXCoord3DDouble & coordinate);
	void closeCurve();
	void endCurve();

	//sugguest a resolution that is calculated based on the distance of the nearest 2 verticies
	int suggestResolution();

	PYXPointer<IRegion> createCurveRegion();
	PYXPointer<IRegion> createPolygonRegion();
	
	PYXPointer<PYXGeometry> createCurveGeometry(int resolution);
	PYXPointer<PYXGeometry> createPolyonGeometry(int resolution);
};


#endif // guard
