/******************************************************************************
geometry_collection.cpp

begin		: 2004-10-15
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/geometry_collection.h"

// pyxlib includes
#include "pyxis/geometry/geometry_iterator.h"

inline void PYXGeometryCollection::setCellResolution(int nCellResolution)
{
	// change the resolution of each geometry in the collection
	for (	PYXPointer<PYXGeometryIterator> spIt(getGeometryIterator());
			!spIt->end();
			spIt->next()	)
	{
		spIt->getGeometry()->setCellResolution(nCellResolution);
	}
}

inline void PYXGeometryCollection::PYXGeometryCollectionIterator::next()
{
	while (!end())
	{
		if (m_spCellIt.get() == nullptr)
		{
			//we need to keep the m_spCurrentGeometry in scope while we use it
			m_spCurrentGeometry = m_spGeometryIt->getGeometry();
			m_spCellIt = m_spCurrentGeometry->getIterator();
		}
		else
		{
			m_spCellIt->next();
		}

		if (!m_spCellIt->end())
		{
			break;
		}

		m_spCellIt.reset();
		m_spCurrentGeometry.reset();

		m_spGeometryIt->next();
	}
}

inline bool PYXGeometryCollection::PYXGeometryCollectionIterator::end() const
{
	return m_spGeometryIt->end();
}

//! Get the bounding box for this geometry.
void PYXGeometryCollection::getBoundingRects(const ICoordConverter* coordConvertor,
	PYXRect2DDouble* pRect1,
	PYXRect2DDouble* pRect2) const
{
	// Add each geometry into the pair of rectangles
	for (PYXPointer<PYXGeometryIterator> it = getGeometryIterator(); !it->end(); it->next())
	{
		PYXRect2DDouble rect1;
		PYXRect2DDouble rect2;
		it->getGeometry()->getBoundingRects(coordConvertor, &rect1, &rect2);
		pRect1->expand(rect1);
		pRect2->expand(rect2);
	}

	// if the resulting rects overlap east to west then amalgamate them for a final result
	if (pRect1->xMin() < pRect2->xMax())
	{
		pRect1->expand(*pRect2);
		pRect2->setEmpty();
	}
	else
	{
		// make both rects have the same Y dimensions.
		pRect1->expandY(pRect2->yMax());
		pRect1->expandY(pRect2->yMin());
		pRect2->setYMax(pRect1->yMax());
		pRect2->setYMin(pRect1->yMin());
	}
}


PYXBoundingCircle PYXGeometryCollection::getBoundingCircle() const
{
	PYXBoundingCircle circle;
	// Add each geometry into the pair of rectangles
	for (PYXPointer<PYXGeometryIterator> it = getGeometryIterator(); !it->end(); it->next())
	{
		circle += it->getGeometry()->getBoundingCircle();
	}
	return circle;
}