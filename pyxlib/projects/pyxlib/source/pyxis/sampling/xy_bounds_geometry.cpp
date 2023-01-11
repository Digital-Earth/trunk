/*****************************************************************************
xy_bounds_geometry.cpp

begin		: 2005-08-12
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "xy_bounds_geometry.h"

// pyxlib includes
#include "pyxis/derm/coord_converter.h"
#include "pyxis/derm/neighbour_iterator.h"
#include "pyxis/derm/wgs84_coord_converter.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/geometry/bounding_rects_calculator.h"
#include "pyxis/geometry/icos_test_traverser.h"
#include "pyxis/geometry/multi_cell.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/sampling/xy_intersection_test.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/file_utils.h"


//! Tester class
Tester<PYXXYBoundsGeometry> gTester;

//! Test method
void PYXXYBoundsGeometry::test()
{
	// Save and load a PYXXYBoundsGeometry.
	{
		PYXRect2DDouble bounds(-180, -90, 180, 90);
		WGS84CoordConverter converter;
		PYXXYBoundsGeometry original(bounds, converter, 12);

		// Serialize.
		std::string strPath = FileUtils::pathToString(AppServices::makeTempFile(".test"));
		{
			std::basic_ofstream< char> out(strPath.c_str(), std::ios_base::binary);
			original.serialize(out);
		}

		// Deserialize and compare.
		{
			std::basic_ifstream< char> in(strPath.c_str(), std::ios_base::binary);
			PYXXYBoundsGeometry loaded(in);

			TEST_ASSERT(loaded.getBounds() == original.getBounds());
			TEST_ASSERT(loaded.getCellResolution() == original.getCellResolution());

			// Get the tile collection and compare to original.
			PYXTileCollection tcSaved;
			original.copyTo(&tcSaved);
			PYXTileCollection tcLoaded;
			loaded.copyTo(&tcLoaded);
			TEST_ASSERT(tcLoaded == tcSaved);
		}
	}

	{
		std::string strCalgaryGeometry("21 575825 825825 5.54863e+006 5.77363e+006{372BCAF8-4FEB-4FF2-A1CF-CE049A738110}PROJCS[\"NAD83 / UTM zone 11N\",GEOGCS[\"NAD83\",DATUM[\"North_American_Datum_1983\",SPHEROID[\"GRS 1980\",6378137,298.2572221010002,AUTHORITY[\"EPSG\",\"7019\"]],AUTHORITY[\"EPSG\",\"6269\"]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4269\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-117],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AUTHORITY[\"EPSG\",\"26911\"]]");
		std::basic_istringstream< char> inStream(strCalgaryGeometry, std::ios_base::binary);
		PYXXYBoundsGeometry original(inStream);
		original.setCellResolution(10);
		TEST_ASSERT(!original.getIterator()->end());
	}

	{
		//Checking the montana case
		PYXRect2DDouble bounds(-116, 44, -104, 49);
		WGS84CoordConverter converter;

		PYXXYBoundsGeometry montana(bounds, converter, 12);

		PYXPointer<PYXIterator> it = montana.getIterator();
		int count = 0;

		while(!it->end())
		{
			count ++;
			it->next();
		}

		TRACE_TEST("XYBound of montana at resolution " << montana.getCellResolution() << " was " << count << "cells");

		while (count > 1 && montana.getCellResolution() > 2)
		{
			montana.setCellResolution(montana.getCellResolution()-1);

			it = montana.getIterator();
			count = 0;

			while(!it->end())
			{
				count ++;
				it->next();
			}

			TRACE_TEST("XYBound of montana at resolution " << montana.getCellResolution() << " was " << count << "cells");
		}

		if (count == 1)
		{
			it = montana.getIterator();

			TRACE_TEST("XYBound of montana at resolution " << montana.getCellResolution() << " is " << it->getIndex());
		}
	}

	//testing north pool projection case
	{
		std::string strNorthPoolGeometry("15 -2.625e+006 6.6225e+006 -2.62e+006 6.625e+006{372BCAF8-4FEB-4FF2-A1CF-CE049A738110}PROJCS[\"unnamed\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Polar_Stereographic\"],PARAMETER[\"latitude_of_origin\",90],PARAMETER[\"central_meridian\",0],PARAMETER[\"scale_factor\",0.994],PARAMETER[\"false_easting\",2000000],PARAMETER[\"false_northing\",2000000],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]]]");	
		std::basic_istringstream< char> inStream(strNorthPoolGeometry, std::ios_base::binary);
		PYXXYBoundsGeometry geometry(inStream);		

		//north pool test
		TEST_ASSERT(geometry.intersects(PYXCell(PYXIcosIndex("2-2020200202"))));

		//inside the geometry
		TEST_ASSERT(geometry.intersects(PYXCell(PYXIcosIndex("1-3000400020"))));
		TEST_ASSERT(geometry.intersects(PYXCell(PYXIcosIndex("E-0506040605"))));
		TEST_ASSERT(geometry.intersects(PYXCell(PYXIcosIndex("2-0206010102"))));

		//outside the geometry

		TEST_ASSERT(!geometry.intersects(PYXCell(PYXIcosIndex("B-0000200103"))));
		TEST_ASSERT(!geometry.intersects(PYXCell(PYXIcosIndex("4-3001000206"))));
		TEST_ASSERT(!geometry.intersects(PYXCell(PYXIcosIndex("7-0500200000"))));		
	}

	{
		//Checking the safe-bounding bbox issues
		PYXRect2DDouble bounds(-180, -90, 180, 90);
		WGS84CoordConverter converter;

		PYXXYBoundsGeometry geometry(bounds, converter, 12);

		TEST_ASSERT(geometry.intersects(PYXTile(PYXIcosIndex("2-3"),11)));
	}
}

//! Constructor.
PYXXYBoundsGeometry::PYXXYBoundsGeometry(	const PYXRect2DDouble& bounds,
											const ICoordConverter& coordConverter,
											int nResolution	) :
	PYXVectorGeometry(PYXXYBoundsRegion::create(bounds,coordConverter),nResolution)	
{
}

//! Copy constructor.
PYXXYBoundsGeometry::PYXXYBoundsGeometry(const PYXXYBoundsGeometry& other) :
	PYXVectorGeometry(other)
{
}

//! Deserialization Constructor.
PYXXYBoundsGeometry::PYXXYBoundsGeometry(std::basic_istream< char>& in)
{
	deserialize(in);
}


/*!
Expand the bounds of the geometry to include the passed point. 

\param pt	The point to include in geometry. Must be in the same
			coordinate system as the bounds.
*/
void PYXXYBoundsGeometry::expand(const PYXCoord2DDouble& pt)
{
	PYXXYBoundsRegion* xyBoundsRegion = dynamic_cast<PYXXYBoundsRegion*>(getRegion().get());
	PYXRect2DDouble bounds = xyBoundsRegion->getBounds();
	bounds.expand(pt);

	setRegion(PYXXYBoundsRegion::create(bounds,*(xyBoundsRegion->getCoordConverter())));
}

//! Clone.
PYXPointer<PYXGeometry> PYXXYBoundsGeometry::clone() const
{
	return create(*this);
}

//! Serialize.
void PYXXYBoundsGeometry::serialize(std::basic_ostream< char>& out) const
{
	PYXXYBoundsRegion* xyBoundsRegion = dynamic_cast<PYXXYBoundsRegion*>(getRegion().get());
	out << getCellResolution() << " " << xyBoundsRegion->getBounds();

	assert(xyBoundsRegion->getCoordConverter());
	xyBoundsRegion->getCoordConverter()->serializeCOM(out);
}

//! Deserialize.
void PYXXYBoundsGeometry::deserialize(std::basic_istream< char>& in)
{
	int resolution;
	PYXRect2DDouble bounds;
	in >> resolution >> bounds;
	boost::intrusive_ptr<ICoordConverter> coordConverter = PYXCoordConverter::deserializeCOM(in);

	setRegion(PYXXYBoundsRegion::create(bounds,*coordConverter));
	setCellResolution(resolution);
}

/*!
Get the bounds of this geometry.
\return	The bounds in native coordinates.
*/
const PYXRect2DDouble& PYXXYBoundsGeometry::getBounds() const
{
	PYXXYBoundsRegion* xyBoundsRegion = dynamic_cast<PYXXYBoundsRegion*>(getRegion().get());
	return xyBoundsRegion->getBounds();
}

/*!
Get the coord converter of this geometry.
\return	The coord converter (ownership retained).
*/
const boost::intrusive_ptr<ICoordConverter> & PYXXYBoundsGeometry::getCoordConverter() const
{
	PYXXYBoundsRegion* xyBoundsRegion = dynamic_cast<PYXXYBoundsRegion*>(getRegion().get());
	return xyBoundsRegion->getCoordConverter();
}

//! Get the bounding box for this geometry.
void PYXXYBoundsGeometry::getBoundingRects(const ICoordConverter* coordConvertor,
	PYXRect2DDouble* pRect1,
	PYXRect2DDouble* pRect2) const
{
	assert((pRect1 != 0) && "Invalid argument.");
	assert((pRect2 != 0) && "Invalid argument.");

	// Clear incoming rectangles.
	pRect1->setEmpty();
	pRect2->setEmpty();

	// If we are using the same coord convertor each way then 
	// skip the conversion and keep more precision in the answer.
	if (getCoordConverter().get() == coordConvertor)
	{
		pRect1->expand(getBounds());
	}
	else
	{
		// Convert the rect to Pyxis with this object's coord converter
		// and then put those cells into a bounding rect calculator and get
		// the answer from there.
		PYXIcosIndex index;
		const PYXRect2DDouble & bounds = getBounds();
		PYXBoundingRectsCalculator calculator(coordConvertor, *this);

		//Sample the bounds and convert them into a set of cells
		//we are not doing linear but sample more around the edges of the bounding box
		double steps[15] = {0.0,0.001,0.01,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,0.99,0.999,1.0};
		auto sampleSize = 15;
		for (auto x=0;x<sampleSize;x++) 
		{
			for (auto y=0;y<sampleSize;y++) 
			{
				auto cx = bounds.xMin()+steps[x]*bounds.width();
				auto cy = bounds.yMin()+steps[y]*bounds.height();
				getCoordConverter()->nativeToPYXIS(PYXCoord2DDouble(cx, cy), &index, getCellResolution());
				calculator.addCell(PYXCell(index));
			}
		}

		calculator.getBoundingRects(pRect1, pRect2);
	}
}
