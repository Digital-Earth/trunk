/******************************************************************************
library_feature.cpp

begin      : 15/11/2007 9:56:19 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#define LIBRARY_SOURCE

#include "library_feature.h"

// local includes
#include "exceptions.h"

// pyxis library includes
#include "pyxis/data/exceptions.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/geometry/curve.h"
#include "pyxis/geometry/multi_geometry.h"
#include "pyxis/geometry/polygon.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/notifier.h"
#include "pyxis/utility/rect_2d.h"

// standard includes
#include <cassert>
#include <memory>

//! Constructor
LibraryFeature::LibraryFeature(const ProcRef& procref, int nResolution) :
	m_nResolution(nResolution)
{
	LibraryItem item;
	if (!Library::existsItem(procref) || !Library::getItem(procref, &item))
	{
		assert(false);
		PYXTHROW(PYXDataException, "Could not create Library feature for procref " << procref);
	}
	
	m_spDefn = createFieldDefn();
	m_strID = procRefToStr(item.getProcRef());
	m_vecValue.push_back(PYXValue(item.getName()));
	m_vecValue.push_back(PYXValue(guidToStr(item.getProcRef().getProcID())));
	m_vecValue.push_back(PYXValue(item.getProcRef().getProcVersion()));
	std::string strLink = AppServices::getPyxisProtocol() + procRefToStr(item.getProcRef());
	m_vecValue.push_back(PYXValue(strLink));
}

// TODO needs documentation, proper home, and configurability for number of edge subdivisions
boost::intrusive_ptr<PYXPolygon> convertBoundsToPolygon(boost::intrusive_ptr<const ICoordConverter> spCC, const PYXRect2DDouble& r, int nRes)
{
	boost::intrusive_ptr<PYXPolygon> spPoly = PYXPolygon::create();
	PYXIcosIndex v;

	// TODO 2008-02-05 mlepage Assuming spCC is SnyderProjection
	// so x=lat y=lon when calling this function!
	PYXCoord2DDouble corners[4] =
	{
		PYXCoord2DDouble(r.yMin(), r.xMin()),
		PYXCoord2DDouble(r.yMin(), r.xMax()),
		PYXCoord2DDouble(r.yMax(), r.xMax()),
		PYXCoord2DDouble(r.yMax(), r.xMin())
	};

	PYXCoord2DDouble c;
	for (int nSide = 0; nSide != 4; ++nSide)
	{
		PYXCoord2DDouble& c1 = corners[nSide];
		PYXCoord2DDouble& c2 = corners[(nSide + 1) % 4];
		const int knSubCount = 16;
		double xd = (c2.x() - c1.x()) / knSubCount;
		double yd = (c2.y() - c1.y()) / knSubCount;
		for (int n = 0; n != knSubCount + 1; ++n)
		{
			c.setX(c1.x() + n * xd);
			c.setY(c1.y() + n * yd);
			spCC->nativeToPYXIS(c, &v, nRes);
			spPoly->addVertex(v);
		}
	}

	// TODO may need to set exterior point.
#if 0
	// TEMP just testing with north pole
	c.setX(90);
	c.setY(0);
	spCC->nativeToPYXIS(c, &v, nRes);
	spPoly->setExteriorPoint(v);
#endif

	// Assume if we don't get at least 4 vertexes, we aren't good to go.
	spPoly->closeRing();
	if (spPoly->getVertexCount(0) < 4)
	{
		spPoly = 0;
	}

	return spPoly;
}

// TODO needs documentation, proper home, and configurability for number of edge subdivisions
boost::intrusive_ptr<PYXCurve> convertBoundsToCurve(boost::intrusive_ptr<const ICoordConverter> spCC, const PYXRect2DDouble& r, int nRes)
{
	boost::intrusive_ptr<PYXCurve> spCurve = PYXCurve::create();
	PYXIcosIndex v;

	// TODO 2008-02-05 mlepage Assuming spCC is SnyderProjection
	// so x=lat y=lon when calling this function!
	PYXCoord2DDouble corners[4] =
	{
		PYXCoord2DDouble(r.yMin(), r.xMin()),
		PYXCoord2DDouble(r.yMin(), r.xMax()),
		PYXCoord2DDouble(r.yMax(), r.xMax()),
		PYXCoord2DDouble(r.yMax(), r.xMin())
	};

	PYXCoord2DDouble c;
	for (int nSide = 0; nSide != 4; ++nSide)
	{
		PYXCoord2DDouble& c1 = corners[nSide];
		PYXCoord2DDouble& c2 = corners[(nSide + 1) % 4];
		const int knSubCount = 16;
		double xd = (c2.x() - c1.x()) / knSubCount;
		double yd = (c2.y() - c1.y()) / knSubCount;
		for (int n = 0; n != knSubCount + 1; ++n)
		{
			c.setX(c1.x() + n * xd);
			c.setY(c1.y() + n * yd);
			spCC->nativeToPYXIS(c, &v, nRes);
			spCurve->addNode(v);
		}
	}

	// Assume if we don't get at least 4 vertexes, we aren't good to go.
	if (spCurve->getNodes().size() < 4)
	{
		spCurve = 0;
	}

	return spCurve;
}

PYXPointer<PYXGeometry> STDMETHODCALLTYPE LibraryFeature::getGeometry()
{
	// fetch the geometry from the library
	if (!m_spGeometry)
	{
#if 0
		// Just return the geometry.
		boost::intrusive_ptr<PYXGeometry> spGeom = Library::getProcGeometry(
			strToProcRef(m_strID), false);

		// But first attempt to cull global geometries.
		//if (!isPracticallyGlobal(spGeom))
		{
			spGeom->setCellResolution(m_nResolution);
			m_spGeometry = spGeom;
		}
#elif 0
		// Convert the geometry to a polygon using its bounds.
		boost::intrusive_ptr<PYXGeometry> tempGeom = 
			Library::getProcGeometry(strToProcRef(m_strID), false);
		if (tempGeom)
		{
			// Clone so we don't alter original.
			tempGeom = tempGeom->clone();
			tempGeom->setCellResolution(m_nResolution);

			// Get bounding rects.
			PYXRect2DDouble rect[2];
			tempGeom->getBoundingRects(SnyderProjection::getInstance(), &rect[0], &rect[1]);

			boost::intrusive_ptr<PYXPolygon> geom[2];
			PYXIcosIndex vertex;

			for (int n = 0; n != 2; ++n)
			{
				if (rect[n].empty())
				{
					continue;
				}

				geom[n] = convertBoundsToPolygon(SnyderProjection::getInstance(), rect[n], m_nResolution);
			}
			
			if (geom[0] && geom[1])
			{
				boost::intrusive_ptr<PYXMultiPolygon> spMG = PYXMultiPolygon::create();
				spMG->addGeometry(geom[0]);
				spMG->addGeometry(geom[1]);
				m_spGeometry = spMG;
			}
			else if (poly[0])
			{
				m_spGeometry = geom[0];
			}
			else if (geom[1])
			{
				m_spGeometry = geom[1];
			}
			else
			{
				m_spGeometry = PYXEmptyGeometry::create();
			}
		}
#else
		// Convert the geometry to a curve using its bounds.
		boost::intrusive_ptr<PYXGeometry> tempGeom = 
			Library::getProcGeometry(strToProcRef(m_strID), false);
		if (tempGeom)
		{
			// Clone so we don't alter original.
			tempGeom = tempGeom->clone();
			tempGeom->setCellResolution(m_nResolution);

			// Get bounding rects.
			PYXRect2DDouble rect[2];
			tempGeom->getBoundingRects(SnyderProjection::getInstance(), &rect[0], &rect[1]);

			boost::intrusive_ptr<PYXCurve> geom[2];
			PYXIcosIndex vertex;

			for (int n = 0; n != 2; ++n)
			{
				if (rect[n].empty())
				{
					continue;
				}

				geom[n] = convertBoundsToCurve(SnyderProjection::getInstance(), rect[n], m_nResolution);
			}
			
			if (geom[0] && geom[1])
			{
				boost::intrusive_ptr<PYXMultiCurve> spMG = PYXMultiCurve::create();
				spMG->addGeometry(geom[0]);
				spMG->addGeometry(geom[1]);
				m_spGeometry = spMG;
			}
			else if (geom[0])
			{
				m_spGeometry = geom[0];
			}
			else if (geom[1])
			{
				m_spGeometry = geom[1];
			}
			else
			{
				m_spGeometry = PYXEmptyGeometry::create();
			}
		}
#endif
	}
	return m_spGeometry;
}

PYXPointer<PYXTableDefinition> LibraryFeature::createFieldDefn()
{
	PYXPointer<PYXTableDefinition> spFeatureDefn = PYXTableDefinition::create();
	spFeatureDefn->addFieldDefinition(
		"Name", 
		PYXFieldDefinition::knContextNone, 
		PYXValue::knString);
	spFeatureDefn->addFieldDefinition(
		"Process ID", 
		PYXFieldDefinition::knContextNone, 
		PYXValue::knString);
	spFeatureDefn->addFieldDefinition(
		"Version", 
		PYXFieldDefinition::knContextNone, 
		PYXValue::knInt32);
	spFeatureDefn->addFieldDefinition(
		"Open Link", 
		PYXFieldDefinition::knContextNone, 
		PYXValue::knString);
	return spFeatureDefn;
}

