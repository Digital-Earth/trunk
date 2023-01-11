/******************************************************************************
region_builder.cpp

begin		: 2014-02-09
copyright	: (C) 2012 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "region_builder.h"

#include "multi_polygon_region.h"
#include "multi_curve_region.h"


PYXRegionBuilder::PYXRegionBuilder() : m_nearestVerticesDistance(0)
{
}

void PYXRegionBuilder::addVertex(const PointLocation & location)
{
	addVertex(location.asXYZ());
}

void PYXRegionBuilder::addVertex(const PYXCoord3DDouble & coordinate)
{
	m_currentVertices.push_back(coordinate);

	if (m_currentVertices.size()>1) {
		size_t lastIndex = m_currentVertices.size()-1;
		double distance = SphereMath::distanceBetween(m_currentVertices[lastIndex-1], m_currentVertices[lastIndex]);
		if (m_nearestVerticesDistance == 0 || distance < m_nearestVerticesDistance )
		{
			m_nearestVerticesDistance = distance;
		}
	}
}

void PYXRegionBuilder::closeCurve()
{
	if (!m_currentVertices.empty())
	{
		m_currentCurves.push_back(PYXCurveRegion::create(m_currentVertices,true));
		m_currentVertices.clear();
	}
}

void PYXRegionBuilder::endCurve()
{
	if (!m_currentVertices.empty())
	{
		m_currentCurves.push_back(PYXCurveRegion::create(m_currentVertices,false));
		m_currentVertices.clear();
	}
}



int PYXRegionBuilder::suggestResolution() {
	return std::max(11,PYXBoundingCircle::estimateResolutionFromRadius(m_nearestVerticesDistance/2));
}

PYXPointer<IRegion> PYXRegionBuilder::createCurveRegion()
{
	endCurve();
	return PYXMultiCurveRegion::create(m_currentCurves);
}

PYXPointer<IRegion> PYXRegionBuilder::createPolygonRegion()
{
	closeCurve();
	return PYXMultiPolygonRegion::create(m_currentCurves);
}

PYXPointer<PYXGeometry> PYXRegionBuilder::createCurveGeometry(int resolution)
{
	return PYXVectorGeometry2::create(createCurveRegion(),resolution);
}

PYXPointer<PYXGeometry> PYXRegionBuilder::createPolyonGeometry(int resolution)
{
	return PYXVectorGeometry2::create(createPolygonRegion(),resolution);
}
