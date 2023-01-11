/******************************************************************************
coord_converter_impl.cpp

begin		: 2004-10-15
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_DRIVER_UTILITY_SOURCE
#include "coord_converter_impl.h"

// pyxlib includes
#include "pyxis/derm/exceptions.h"
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/derm/horizontal_datum.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/wgs84.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/math_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/thread_pool.h"
#include "pyxis/geometry/geometry_serializer.h"

// gdal includes
#include "cpl_error.h"

// boost includes
#include <boost/scoped_array.hpp>

// standard includes
#include <cassert>
#include <memory>
#include "pyxis/utility/string_utils.h"

// {372BCAF8-4FEB-4ff2-A1CF-CE049A738110}
PYXCOM_DEFINE_CLSID(CoordConverterImpl, 
					0x372bcaf8, 0x4feb, 0x4ff2, 0xa1, 0xcf, 0xce, 0x04, 0x9a, 0x73, 0x81, 0x10);

PYXCOM_CLASS_INTERFACES(CoordConverterImpl, ICoordConverter::iid, PYXCOM_IUnknown::iid);

#ifdef OGR_PROJ_IS_NOT_THREAD_SAFE
AppExecutionScope CoordConverterImpl::s_gdalScope("gdal");
#endif

//! Tester class.
Tester<CoordConverterImpl> gTester;

//search for OGR_PROJ_IS_NOT_THREAD_SAFE
/*!
This function creates an XYBoundGeometry(code=03) geometry that creates a new CoordConverterImpl. It
is used by the test method to create many CoordConverterImpl in parallel.
*/
void createGeometryFromStringTest()
{
	try
	{
		std::string strCalgaryGeometry(" 21 575825 825825 5.54863e+006 5.77363e+006{372BCAF8-4FEB-4FF2-A1CF-CE049A738110}PROJCS[\"NAD83 / UTM zone 11N\",GEOGCS[\"NAD83\",DATUM[\"North_American_Datum_1983\",SPHEROID[\"GRS 1980\",6378137,298.2572221010002,AUTHORITY[\"EPSG\",\"7019\"]],AUTHORITY[\"EPSG\",\"6269\"]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4269\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-117],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AUTHORITY[\"EPSG\",\"26911\"]]");
		strCalgaryGeometry[0] = 03;
		auto geom = PYXGeometrySerializer::deserialize(strCalgaryGeometry);
	}
	catch(...)
	{
		TRACE_INFO("failed");
	}
}

//! Test method.
void CoordConverterImpl::test()
{
	PYXPointer<PYXSpatialReferenceSystem> spSRS(PYXSpatialReferenceSystem::create());
	spSRS->setSystem(PYXSpatialReferenceSystem::knSystemGeographical);
	spSRS->setDatum(PYXSpatialReferenceSystem::knDatumWGS84);

	PYXPointer<PYXSpatialReferenceSystem> spSRS2(PYXSpatialReferenceSystem::create());
	spSRS2->setSystem(PYXSpatialReferenceSystem::knSystemGeographical);
	spSRS2->setDatum(PYXSpatialReferenceSystem::knDatumNAD27);

	// Test copy constructor
	{
		// Create coord converter.
		std::auto_ptr<CoordConverterImpl> apConv(new CoordConverterImpl);
		apConv->initialize(*spSRS);

		// Do some conversions.
		PYXCoord2DDouble coord1;
		{
			PYXIcosIndex index("A-010203040506");
			apConv->pyxisToNative(index, &coord1);
		}
		PYXIcosIndex index1;
		{
			PYXCoord2DDouble coord(0.5, 0.75);
			apConv->nativeToPYXIS(coord, &index1, 15);
		}

		// Copy coord converter.
		CoordConverterImpl conv(*apConv);

		// Change the original.
		apConv->initialize(*spSRS2);

		// Redo the conversion and compare results.
		{
			PYXCoord2DDouble coord2;
			PYXIcosIndex index("A-010203040506");
			conv.pyxisToNative(index, &coord2);
			TEST_ASSERT(coord1 == coord2);
		}
		{
			PYXIcosIndex index2;
			PYXCoord2DDouble coord(0.5, 0.75);
			conv.nativeToPYXIS(coord, &index2, 15);
			TEST_ASSERT(index1 == index2);
		}

		// Delete the original.
		apConv.reset(0);

		// Redo the conversion and compare results.
		{
			PYXCoord2DDouble coord2;
			PYXIcosIndex index("A-010203040506");
			conv.pyxisToNative(index, &coord2);
			TEST_ASSERT(coord1 == coord2);
		}
		{
			PYXIcosIndex index2;
			PYXCoord2DDouble coord(0.5, 0.75);
			conv.nativeToPYXIS(coord, &index2, 15);
			TEST_ASSERT(index1 == index2);
		}
	}

	// Test clone
	{
		// Create coord converter.
		std::auto_ptr<CoordConverterImpl> apConv(new CoordConverterImpl);
		apConv->initialize(*spSRS);

		// Do some conversions.
		PYXCoord2DDouble coord1;
		{
			PYXIcosIndex index("A-010203040506");
			apConv->pyxisToNative(index, &coord1);
		}
		PYXIcosIndex index1;
		{
			PYXCoord2DDouble coord(0.5, 0.75);
			apConv->nativeToPYXIS(coord, &index1, 15);
		}

		// Clone coord converter.
		boost::intrusive_ptr<ICoordConverter> apConv2(apConv->clone());

		// Change the original.
		apConv->initialize(*spSRS2);

		// Redo the conversion and compare results.
		{
			PYXCoord2DDouble coord2;
			PYXIcosIndex index("A-010203040506");
			apConv2->pyxisToNative(index, &coord2);
			TEST_ASSERT(coord1 == coord2);
		}
		{
			PYXIcosIndex index2;
			PYXCoord2DDouble coord(0.5, 0.75);
			apConv2->nativeToPYXIS(coord, &index2, 15);
			TEST_ASSERT(index1 == index2);
		}

		// Delete the original.
		apConv.reset(0);

		// Redo the conversion and compare results.
		{
			PYXCoord2DDouble coord2;
			PYXIcosIndex index("A-010203040506");
			apConv2->pyxisToNative(index, &coord2);
			TEST_ASSERT(coord1 == coord2);
		}
		{
			PYXIcosIndex index2;
			PYXCoord2DDouble coord(0.5, 0.75);
			apConv2->nativeToPYXIS(coord, &index2, 15);
			TEST_ASSERT(index1 == index2);
		}
	}

	// Test an offset coordinate system, lon values should fall within the native bounds.
	{
		// Create coord converter.
		CoordConverterImpl conv;
		conv.initialize(*spSRS);
		PYXRect2DDouble rNativeBounds(0.0, -90.0, 360.0, 90.0);
		conv.setNativeBounds(rNativeBounds);

		PYXCoord2DDouble native;
		CoordLatLon ll;
		ll.setLatInDegrees(0.0);

		ll.setLonInDegrees(-179.0);
		conv.latLonToNative(ll, &native);
		TEST_ASSERT(MathUtils::equal(native[0], 181.0));

		ll.setLonInDegrees(0.0);
		conv.latLonToNative(ll, &native);
		TEST_ASSERT(MathUtils::equal(native[0], 0.0));

		ll.setLonInDegrees(179.0);
		conv.latLonToNative(ll, &native);
		TEST_ASSERT(MathUtils::equal(native[0], 179.0));

		ll.setLonInDegrees(359.0);
		conv.latLonToNative(ll, &native);
		TEST_ASSERT(MathUtils::equal(native[0], 359.0));

		ll.setLonInDegrees(539.0);
		conv.latLonToNative(ll, &native);
		TEST_ASSERT(MathUtils::equal(native[0], 179.0));
	}

	// Test serialization
	{
		// Create coord converter.
		CoordConverterImpl conv;
		conv.initialize(*spSRS);

		// Do some conversions.
		PYXCoord2DDouble coord1;
		{
			PYXIcosIndex index("A-010203040506");
			conv.pyxisToNative(index, &coord1);
		}
		PYXIcosIndex index1;
		{
			PYXCoord2DDouble coord(0.5, 0.75);
			conv.nativeToPYXIS(coord, &index1, 15);
		}

		// Serialize coord converter.
		std::string strPath = FileUtils::pathToString(AppServices::makeTempFile(".test"));
		{
			std::basic_ofstream< char> out(strPath.c_str(), std::ios_base::binary);
			conv.serialize(out);
		}

		// Deserialize coord converter.
		CoordConverterImpl conv2;
		{
			std::basic_ifstream< char> in(strPath.c_str(), std::ios_base::binary);
			conv2.deserialize(in);
		}

		// Redo the conversion and compare results.
		{
			PYXCoord2DDouble coord2;
			PYXIcosIndex index("A-010203040506");
			conv2.pyxisToNative(index, &coord2);
			TEST_ASSERT(coord1 == coord2);
		}
		{
			PYXIcosIndex index2;
			PYXCoord2DDouble coord(0.5, 0.75);
			conv2.nativeToPYXIS(coord, &index2, 15);
			TEST_ASSERT(index1 == index2);
		}

		TEST_ASSERT(conv == conv2);
	}

	// Test various datums for consistency.
	{
		PYXSpatialReferenceSystem::eDatum datumArray[] =
		{
			PYXSpatialReferenceSystem::eDatum::knDatumNAD27,
			PYXSpatialReferenceSystem::eDatum::knDatumNAD83,
			PYXSpatialReferenceSystem::eDatum::knDatumWGS72,
			PYXSpatialReferenceSystem::eDatum::knDatumWGS84
		};

		PYXPointer<PYXSpatialReferenceSystem> spSRS(PYXSpatialReferenceSystem::create());
		spSRS->setSystem(PYXSpatialReferenceSystem::knSystemGeographical);

		for (int nDatum = 0; nDatum != sizeof(datumArray) / sizeof(datumArray[0]); ++nDatum)
		{
			spSRS->setDatum(datumArray[nDatum]);
			CoordConverterImpl conv;
			conv.initialize(*spSRS);

			for (int n = 0; n != 10000; ++n)
			{
				for (int nResolution = 30; nResolution != 40; ++nResolution)
				{
					for (int nTestType = 0; nTestType < 5; ++nTestType)
					{
						PYXCoord2DDouble coordOrig;

						switch (nTestType)
						{
						case 0:	// north pole
							coordOrig.setX(static_cast<double>(rand()) / RAND_MAX * 358.0 - 179.0);
							coordOrig.setY(90.0);
							break;

						case 1:	// south pole
							coordOrig.setX(static_cast<double>(rand()) / RAND_MAX * 358.0 - 179.0);
							coordOrig.setY(-90.0);
							break;

						case 2: // random position on the international date line + 180 degrees
							coordOrig.setX(180.0);
							coordOrig.setY(static_cast<double>(rand()) / RAND_MAX * 178.0 - 89.0);
							break;

						case 3:	// random position on the international date line -180 degrees
							coordOrig.setX(-180.0);
							coordOrig.setY(static_cast<double>(rand()) / RAND_MAX * 178.0 - 89.0);
							break;

						case 4:	// random position on the globe avoiding the poles and international date line
						default:
							coordOrig.setX(static_cast<double>(rand()) / RAND_MAX * 358.0 - 179.0);
							coordOrig.setY(static_cast<double>(rand()) / RAND_MAX * 178.0 - 89.0);
							break;
						}

						// Convert to PYXIS.
						PYXIcosIndex index;
						conv.nativeToPYXIS(coordOrig, &index, nResolution);

						// Convert back.
						PYXCoord2DDouble coordNew;
						conv.pyxisToNative(index, &coordNew);

						// Compare to original.
						switch (nTestType)
						{
						case 0:	// north pole
							TEST_ASSERT(MathUtils::equal(coordOrig.y(), coordNew.y(), 0.1));
							break;

						case 1:	// south pole
							TEST_ASSERT(MathUtils::equal(coordOrig.y(), coordNew.y(), 0.1));
							break;

						case 2: // random position on the international date line + 180 degrees
							TEST_ASSERT(
								(	MathUtils::equal(180.0, coordNew.x(), 0.1) ||
								MathUtils::equal(-180.0, coordNew.x(), 0.1)	) &&
								MathUtils::equal(coordOrig.y(), coordNew.y(), 0.1)	);
							break;

						case 3:	// random position on the international date line -180 degrees
							TEST_ASSERT(
								(	MathUtils::equal(180.0, coordNew.x(), 0.1) ||
								MathUtils::equal(-180.0, coordNew.x(), 0.1)	) &&
								MathUtils::equal(coordOrig.y(), coordNew.y(), 0.1)	);
							break;

						case 4:	// random position on the globe avoiding the poles and international date line
						default:
							TEST_ASSERT(
								MathUtils::equal(coordOrig.x(), coordNew.x(), 0.1) &&
								MathUtils::equal(coordOrig.y(), coordNew.y(), 0.1)	);
							break;
						}
					}
				}
			}
		}
	}


	//search for: OGR_PROJ_IS_NOT_THREAD_SAFE
	//[shatzi]not a perfect test I must say - as if this does't work - it will destroy the heap
	PYXTaskGroup tasks;

	for (int i=0; i<10000; i++)
	{
		tasks.addTask(boost::bind(createGeometryFromStringTest));
	}

	tasks.joinAll();
}

/*!
Constructor.
*/
CoordConverterImpl::CoordConverterImpl() :
	m_pNativeToWGS84(0),
	m_pWGS84ToNative(0),
	m_bNativeIsProjected(false),
	m_containsLonWrap(false),
	m_rWGS84Bounds(-180.0, -90.0, 180.0, 90.0),
	m_rNativeBounds(1.0, 1.0, -1.0, -1.0),
	m_fNativeLonAdjustment(0.0)
{
}

/*!
Copy constructor.
*/
CoordConverterImpl::CoordConverterImpl(const CoordConverterImpl& other) :
	m_SRSNative(other.m_SRSNative),
	m_SRSWGS84(),
	m_pNativeToWGS84(0),
	m_pWGS84ToNative(0),
	m_bNativeIsProjected(false),
	m_containsLonWrap(other.m_containsLonWrap),
	m_rWGS84Bounds(-180.0, -90.0, 180.0, 90.0),
	m_rNativeBounds(other.m_rNativeBounds),
	m_fNativeLonAdjustment(other.m_fNativeLonAdjustment)
{
	// This will set m_SRSWGS84 and create any necessary transforms.
	initializeTransforms();
}

/*!
Destructor. Delete the GeospatialCoordTransforms
*/
CoordConverterImpl::~CoordConverterImpl()
{
#ifdef OGR_PROJ_IS_NOT_THREAD_SAFE
	boost::recursive_mutex::scoped_lock lock(s_gdalScope);
#endif
	delete m_pNativeToWGS84;
	delete m_pWGS84ToNative;
}

std::string CoordConverterImpl::exportToWellKnownText() 
{
	char* pWkt = 0;
	{
#ifdef OGR_PROJ_IS_NOT_THREAD_SAFE
		boost::recursive_mutex::scoped_lock lock(s_gdalScope);
#endif
		m_SRSNative.exportToWkt(&pWkt);
	}
	assert(pWkt);
	//copy to std::string
	auto result = std::string(pWkt);

	OGRFree(pWkt);

	return result;
}

//! Serialize OGR spatial reference.
std::basic_ostream</*unsigned*/ char>& CoordConverterImpl::serialize(std::basic_ostream</*unsigned*/ char>& out, const OGRSpatialReference& ogr)
{
	char* pWkt = 0;
	{
#ifdef OGR_PROJ_IS_NOT_THREAD_SAFE
		boost::recursive_mutex::scoped_lock lock(s_gdalScope);
#endif
		ogr.exportToWkt(&pWkt);
	}
	assert(pWkt);
	try
	{
		for (char* pWktIter = pWkt; *pWktIter; ++pWktIter)
		{
			unsigned char c = *pWktIter;
			out.put(c);
		}
	}
	catch (...)
	{
		assert(false && "Exception caught when serializing OGR spatial reference.");
	}
	out.put(0);
	OGRFree(pWkt);

	return out;
}

//! Deserialize OGR spatial reference.
std::basic_istream</*unsigned*/ char>& CoordConverterImpl::deserialize(std::basic_istream</*unsigned*/ char>& in, OGRSpatialReference& ogr)
{
	std::string str;
	size_t nSize = 0;
	for (; in.good(); ++nSize)
	{
		char c;
		in.get(c);
		if (0 == c)
		{
			break;
		}
		str.push_back(c);
	}

	boost::scoped_array<char> saChars(new char[nSize + 1]);
	assert(saChars.get());
	str.copy(saChars.get(), nSize);
	saChars[nSize] = 0;

	char * pWktImporter = saChars.get();
	{
#ifdef OGR_PROJ_IS_NOT_THREAD_SAFE
		boost::recursive_mutex::scoped_lock lock(s_gdalScope);
#endif
		ogr.importFromWkt(&pWktImporter);
	}

	return in;
}

//! Serialize.
std::basic_ostream</*unsigned*/ char>& CoordConverterImpl::serialize(std::basic_ostream< char>& out) const
{
	if(m_containsLonWrap)
	{
		out << "version=1.1" << std::endl;
		out << m_containsLonWrap << std::endl;
	}
	serialize(out, m_SRSNative);
	return out;
}

//! Deserialize.
std::basic_istream</*unsigned*/ char>& CoordConverterImpl::deserialize(std::basic_istream< char>& in)
{
	auto pos = in.tellg();
	std::string version;
	in >> version;
	if(version == "version=1.1")
	{
		in >> m_containsLonWrap;
	}
	else
	{
		m_containsLonWrap = false;
		in.seekg(pos);
	}
	OGRSpatialReference srsNative;
	deserialize(in, srsNative);
	initialize(srsNative);

	return in;
}

//! Serialize COM object.
void CoordConverterImpl::serializeCOM(std::basic_ostream</*unsigned*/ char>& out) const
{
	out << clsid;
	serialize(out);
}

/*!
Specify the native spatial reference system with a PYXSpatialReferenceSystem.

\param srs			The native spatial reference system
*/
void CoordConverterImpl::initialize(const PYXSpatialReferenceSystem& srs)
{
#ifdef OGR_PROJ_IS_NOT_THREAD_SAFE
	boost::recursive_mutex::scoped_lock lock(s_gdalScope);
#endif

	std::string strDatum = getDatumString(srs.getDatum());

	switch (srs.getSystem())
	{
		// if it is a projected system
	case PYXSpatialReferenceSystem::knSystemProjected:
		{
			// NOTE: Documentation says that for now the order of the calls
			// SetProjCS, SetWellKnownGeogCS, and SetUTM must be in that order.

			// set the description for the projection system
			m_SRSNative.SetProjCS("Projection");

			// set the underlying geographic datum for the projection
			m_SRSNative.SetWellKnownGeogCS(strDatum.c_str());

			m_containsLonWrap = srs.getWKT().find("+lon_wrap") != -1;

			PYXSpatialReferenceSystem::eProjection nProjection = srs.getProjection();

			// set the actual projection information
			switch (nProjection)
			{
				// set the spatial reference to a UTM projection
			case PYXSpatialReferenceSystem::knProjectionUTM:
				{
					m_SRSNative.SetUTM(srs.getZone(), srs.getIsUTMNorth());
					break;
				}

			case PYXSpatialReferenceSystem::knCustomProjection:
				{
					auto err = m_SRSNative.SetFromUserInput(srs.getWKT().c_str());
					if(err==0)
					{
						break;
					}
				}

			case PYXSpatialReferenceSystem::knProjectionTM:
			case PYXSpatialReferenceSystem::knProjectionLCC:
			default:
				{
					PYXTHROW(	PYXCoordConversionException,
						"Unsupported projection: '"	<< getProjectionString(nProjection) << "'."	);
					break;
				}
			}

			break;
		}

	case PYXSpatialReferenceSystem::knSystemGeographical:
		{
			m_SRSNative.SetWellKnownGeogCS(strDatum.c_str());
			break;
		}

	default:
		{
			PYXTHROW(	PYXCoordConversionException,
				"Unsupported spatial reference system."	);
			break;
		}
	}

	initializeTransforms();
}

/*!
Specify the native spatial reference system as a Well Known Text string.

\param strWKT		The well known text string.
*/
void CoordConverterImpl::initialize(const std::string& strWKT)
{
#ifdef OGR_PROJ_IS_NOT_THREAD_SAFE
	boost::recursive_mutex::scoped_lock lock(s_gdalScope);
#endif

	assert(0 < strWKT.length());

	OGRErr nError;
	
	if (StringUtils::isNumeric(strWKT))
	{
		int wikd = 0;
		StringUtils::fromString<int>(strWKT,&wikd);
		nError = m_SRSNative.importFromEPSG(wikd);
	}
	else
	{
		nError = m_SRSNative.SetFromUserInput(strWKT.c_str());	
	}
	

	if (nError != OGRERR_NONE)
	{
		PYXTHROW(	PYXCoordConversionException,
			"Invalid spatial reference: '" << strWKT << "'."	);
	}

	initializeTransforms();
}

/*!
Specify the native spatial reference system as an OGRSpatialReference.

\param osrs			The OGR spatial reference.
*/
void CoordConverterImpl::initialize(const OGRSpatialReference& osrs)
{
	m_SRSNative = osrs;

	initializeTransforms();
}

/*!
Initialize the WGS84 spatial reference system and the transforms for converting
between the native spatial reference system and the WGS 84 spatial reference
system.
*/
void CoordConverterImpl::initializeTransforms()
{
#ifdef OGR_PROJ_IS_NOT_THREAD_SAFE
	boost::recursive_mutex::scoped_lock lock(s_gdalScope);
#endif

	// record if the native coordinate system is projected
	m_bNativeIsProjected = (0 != m_SRSNative.IsProjected());

	// create the WGS84 spatial reference system
	int nError = m_SRSWGS84.SetWellKnownGeogCS("WGS84");
	if (0 != nError)
	{
		PYXTHROW(	PYXCoordConversionException,
			"Unable to create WGS84 coordinate system.");
	}

	/*
	Only need to create transforms if the spatial reference systems are
	the different.
	*/
	if (0 == m_SRSNative.IsSame(&m_SRSWGS84))
	{
		/*
		Create the Coordinate Transformation object by passing in the source
		spatialreference object and the destination spatial reference object
		*/
		delete m_pNativeToWGS84;
		m_pNativeToWGS84 = OGRCreateCoordinateTransformation(	&m_SRSNative,
			&m_SRSWGS84 );
		if (m_pNativeToWGS84 == 0)
		{
			TRACE_ERROR(CPLGetLastErrorMsg());
			PYXTHROW(	PYXCoordConversionException,
				"Unable to create coordinate transformation."	);
		}

		/*
		Create the Coordinate Transformation object by passing in the source
		spatialreference object and the destination spatial reference object
		*/
		delete m_pWGS84ToNative;
		m_pWGS84ToNative = OGRCreateCoordinateTransformation(	&m_SRSWGS84,
			&m_SRSNative	);
		if (m_pWGS84ToNative == 0)
		{
			TRACE_ERROR(CPLGetLastErrorMsg());
			PYXTHROW(	PYXCoordConversionException,
				"Unable to create coordinate transformation."	);
		}
	}

	/*
	Handle the case where the native coordinates are in angular (degrees, radians etc.) units.
	When converting from WGS84 to native coordinates, ensure that the longitude coordinate
	falls into the correct range. The adjustment is applied in wgs84ToNative.
	*/
	if (m_SRSNative.IsGeographic() || m_SRSNative.IsGeocentric())
	{
		// determine the angular units for the native coordinates
		auto fUnitsToRadians = m_SRSNative.GetAngularUnits();
		if (MathUtils::equal(fUnitsToRadians, 1.0))
		{
			m_fNativeLonAdjustment = MathUtils::kf360Rad;
		}
		else if (MathUtils::equal(fUnitsToRadians, MathUtils::kfDegreesToRadians))
		{
			m_fNativeLonAdjustment = 360.0;
		}
	}
}

/*!
Convert native coordinates to WGS84 coordinates.

\param	native	The position in native coordinates.
\param	pll		The position in geodetic lat lon coordinates (out)
*/
void CoordConverterImpl::nativeToWGS84(	const PYXCoord2DDouble& native,
									   CoordLatLon* pll	) const
{
	assert(pll != 0);

	double fX = native.x();
	double fY = native.y();

	// convert to WGS84 coordinates
	if (m_pNativeToWGS84 != 0)
	{
		int nSuccess = m_pNativeToWGS84->Transform(1, &fX, &fY);
		if (nSuccess == 0)
		{
			PYXTHROW(
				PYXCoordConversionException,
				"Unable to perform coordinate transformation."	);
		}
	}

	const double MaxValue = 1000;
	
	if (abs(fX) > MaxValue|| abs(fY) > MaxValue)
	{
		PYXTHROW(
				PYXCoordConversionException,
				"Conversion to WGS84 return values outside of expected limits."	);
	}

	pll->setLonInDegrees(fX);
	pll->setLatInDegrees(fY);
}

/*!
Convert WGS84 coordinates to native coordinates. If they are outside the
bounding rectangle, coordinates are pinned to just outside the bounding
rectangle.

\param	ll		The position in geodetic lat lon coordinates.
\param	pNative	The position in native coordinates (out)
*/
void CoordConverterImpl::wgs84ToNative(	const CoordLatLon& ll,
									   PYXCoord2DDouble* pNative	) const
{
	if (!tryWgs84ToNative(ll,pNative))
	{
		PYXTHROW(
			PYXCoordConversionException,
			"Unable to perform coordinate transformation."	);
	}
}

/*!
Convert WGS84 coordinates to native coordinates. If they are outside the
bounding rectangle, coordinates are pinned to just outside the bounding
rectangle.

\param	ll		The position in geodetic lat lon coordinates.
\param	pNative	The position in native coordinates (out)
*/
bool CoordConverterImpl::tryWgs84ToNative(	const CoordLatLon& ll,
										  PYXCoord2DDouble* pNative	) const
{
	assert(pNative != 0);

	if (m_containsLonWrap && ll.lonInDegrees() < 0)
	{
		pNative->setX(ll.lonInDegrees() + 360.0);
	}
	else
	{
		pNative->setX(ll.lonInDegrees());
	}
	pNative->setY(ll.latInDegrees());

	// convert to native coordinates
	if (0 != m_pWGS84ToNative)
	{
		/*
		We have experienced problems with some coordinate transformations when the
		coordinates to be transformed lie way outside the range of the target
		coordinate system. If the point is outside the (expanded) bounding rectangle,
		pin it to the boundary.
		*/
		*pNative = m_rWGS84Bounds.pin(*pNative);

		double* pCoord = *pNative;
		int nSuccess = m_pWGS84ToNative->Transform(1, &(pCoord[0]), &(pCoord[1]), 0);
		if (nSuccess == 0)
		{
			return false;
		}
	}

	/*
	If the native coordinates are angular, ensure the longitude coordinate falls within the expected
	range.
	*/
	if (m_fNativeLonAdjustment > 0.0 && !m_rNativeBounds.empty())
	{
		if ((*pNative)[0] < m_rNativeBounds.xMin())
		{
			(*pNative)[0] += m_fNativeLonAdjustment;
		}
		else if ((*pNative)[0] > m_rNativeBounds.xMax())
		{
			(*pNative)[0] -= m_fNativeLonAdjustment;
		}
	}

	return true;
}


/*!
Convert WGS84 coordinates to a PYXIS index.

\param	ll			The position in geodetic lat lon coordinates.
\param	pIndex		The PYXIS index (out)
\param	nResolution	The resolution of the resulting index.
*/
void CoordConverterImpl::wgs84ToPYXIS(	const CoordLatLon& ll,
									  PYXIcosIndex* pIndex,
									  int nResolution	) const
{
	assert(pIndex != 0);
	assert(1 < nResolution);

	// convert to geocentric coordinates
	CoordLatLon llGeocentric;
	llGeocentric = WGS84::getInstance()->toGeocentric(ll);

	// convert to a PYXIS index
	SnyderProjection::getInstance()->nativeToPYXIS(llGeocentric, pIndex, nResolution);
}

/*!
Convert a PYXIS index to WGS84 coordinates.

\param	index	The PYXIS index
\param	pll		The position in geodetic lat lon coordinates (out)
*/
void CoordConverterImpl::pyxisToWGS84(	const PYXIcosIndex& index,	
									  CoordLatLon* pll	) const
{
	assert(pll != 0);

	CoordLatLon llGeocentric;
	SnyderProjection::getInstance()->pyxisToNative(index, &llGeocentric);

	// convert to WGS84 coordinates
	*pll = WGS84::getInstance()->toDatum(llGeocentric);
}

/*!
Convert native coordinates to a PYXIS index.

\param	native		The native coordinates.
\param	pIndex		The PYXIS index (out)
\param	nResolution	The resolution of the resulting index.
*/
void CoordConverterImpl::nativeToPYXIS(	const PYXCoord2DDouble& native,
									   PYXIcosIndex* pIndex,
									   int nResolution	) const
{
	assert(pIndex != 0);
	assert(1 < nResolution);

	CoordLatLon ll;

	// convert to WGS84 coordinates
	nativeToWGS84(native, &ll);

	// convert to a PYXIS index
	wgs84ToPYXIS(ll, pIndex, nResolution);
}

/*!
Convert a PYXIS index to native coordinates.

\param	index	The PYXIS index.
\param	pNative	The native coordinates (out)
*/
void CoordConverterImpl::pyxisToNative(	const PYXIcosIndex& index,
									   PYXCoord2DDouble* pNative	) const
{
	assert(pNative != 0);

	CoordLatLon ll;

	// convert to WGS84 coordinates
	pyxisToWGS84(index, &ll);

	// convert to native coordinates
	wgs84ToNative(ll, pNative);
}

bool CoordConverterImpl::tryPyxisToNative(	const PYXIcosIndex& index,
										  PYXCoord2DDouble* pNative	) const
{
	assert(pNative != 0);

	CoordLatLon ll;

	// convert to WGS84 coordinates
	pyxisToWGS84(index, &ll);

	// convert to native coordinates
	return tryWgs84ToNative(ll, pNative);
}


//! Convert a native coordinate to a geocentric LatLon coordinates.
void CoordConverterImpl::nativeToLatLon(const PYXCoord2DDouble& native,
										CoordLatLon * pLatLon) const
{
	assert(pLatLon);

	CoordLatLon wgs84latlon;	

	nativeToWGS84(native,&wgs84latlon);

	*pLatLon = WGS84::getInstance()->toGeocentric(wgs84latlon);
}

//! Convert a geocentric LatLon coordinate to a native coordinate.
void CoordConverterImpl::latLonToNative(const CoordLatLon & latLon,
										PYXCoord2DDouble * pNative) const
{
	assert(pNative);

	CoordLatLon wgs84latlon;

	wgs84latlon = WGS84::getInstance()->toDatum(latLon);

	wgs84ToNative(wgs84latlon,pNative);
}


/*!
Get the Datum as a string

\param	nDatum	The datum.

\return	The Datum as a string.
*/
std::string CoordConverterImpl::getDatumString(PYXSpatialReferenceSystem::eDatum nDatum) 
{
	std::string strGeogCS;

	switch (nDatum)
	{
	case PYXSpatialReferenceSystem::knDatumNAD27:
		{
			strGeogCS = "NAD27";		
			break;
		}

	case PYXSpatialReferenceSystem::knDatumNAD83:
		{
			strGeogCS = "NAD83";		
			break;
		}

	case PYXSpatialReferenceSystem::knDatumWGS72:
		{
			strGeogCS = "WGS72";		
			break;
		}

	case PYXSpatialReferenceSystem::knDatumWGS84:
		{
			strGeogCS = "WGS84";		
			break;
		}

	default:
		{
			strGeogCS = "None";
		}
	}

	return strGeogCS;
}

/*!
Get the projection as a string.

\param	nProjection	The projection.

\return	The Projection type as a string.
*/
std::string CoordConverterImpl::getProjectionString(PYXSpatialReferenceSystem::eProjection nProjection)
{
	std::string strProjection;

	switch (nProjection)
	{
	case PYXSpatialReferenceSystem::knProjectionUTM:
		{
			strProjection = "UTM";		
			break;
		}

	case PYXSpatialReferenceSystem::knProjectionTM:
		{
			strProjection = "TM";		
			break;
		}

	case PYXSpatialReferenceSystem::knProjectionLCC:
		{
			strProjection = "LCC";		
			break;
		}

	case PYXSpatialReferenceSystem::knCustomProjection:
		{
			strProjection = "Custom";		
			break;
		}

	default:
		{
			strProjection = "None";
		}
	}

	return strProjection;
}

/*!
Determine if two converter objects represent the same conversion.

\param	converter	The CoordConverterImpl object to be compared with this one.

\return	true if they are the same, otherwise false.
*/
bool CoordConverterImpl::operator==(const CoordConverterImpl& converter) const
{
	bool bEqual = false;

	bEqual = (1 == m_SRSNative.IsSame(&(converter.m_SRSNative)));

	return bEqual;
}


// {9FFF0792-6B6B-47B6-89BC-739D0F1137C7}
PYXCOM_DEFINE_CLSID(CoordConverterImplFactory, 
					0x9fff0792, 0x6b6b, 0x47b6, 0x89, 0xbc, 0x73, 0x9d, 0xf, 0x11, 0x37, 0xc7);

PYXCOM_CLASS_INTERFACES(CoordConverterImplFactory, ICoordConverterFromSrsFactory::iid, PYXCOM_IUnknown::iid);


boost::intrusive_ptr<ICoordConverter> CoordConverterImplFactory::createFromSRS(const PYXPointer<PYXSpatialReferenceSystem> & srs)
{
	boost::intrusive_ptr<CoordConverterImpl> coordConverter(new CoordConverterImpl());

	coordConverter->initialize(*srs);

	return coordConverter;
}

boost::intrusive_ptr<ICoordConverter> CoordConverterImplFactory::createFromWKT(const std::string & wellKnownText)
{
	boost::intrusive_ptr<CoordConverterImpl> coordConverter(new CoordConverterImpl());

	coordConverter->initialize(wellKnownText);

	return coordConverter;
}


PYXPointer<PYXSpatialReferenceSystem> CoordConverterImplFactory::convertToSRS(const boost::intrusive_ptr<ICoordConverter> & coordConverter)
{
	auto convertor = dynamic_cast<CoordConverterImpl*>(coordConverter.get());

	if (convertor == nullptr)
	{
		return nullptr;
	}

	auto result = PYXSpatialReferenceSystem::create();
	result->setWKT(convertor->exportToWellKnownText());	
	return result;
}
