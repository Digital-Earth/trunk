/******************************************************************************
snyder_projection.cpp

begin		: 2004-02-11
copyright	: derived from DgProjSnyder by Kevin Sahr
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/derm/snyder_projection.h"

// local includes
#include "pyxis/derm/exceptions.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/reference_sphere.h"
#include "pyxis/derm/spiral_iterator.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/math_utils.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <cmath>

// {7E112214-C4B4-47d6-AD32-5D64548728A0}
PYXCOM_DEFINE_CLSID(SnyderProjection, 
0x7e112214, 0xc4b4, 0x47d6, 0xad, 0x32, 0x5d, 0x64, 0x54, 0x87, 0x28, 0xa0);

PYXCOM_CLASS_INTERFACES(SnyderProjection, ICoordConverter::iid, PYXCOM_IUnknown::iid);

//! The best precision we can get from the Snyder Projection
static const double kfPrecision = 5.0e-8;

//! Distance in radians between resolution 0 PYXIS cells.
double SnyderProjection::kfUnitRadianDistance;

//! Distance in metres between resolution 0 PYXIS cells on the suface of the sphere.
double SnyderProjection::kfUnitMetreDistance;

/*
The spherical angle in degrees between a radius vector to the centre and the
adjacent edge of a spherical polygon on the globe.
*/
static double kfGH;			// = MathUtils::degreesToRadians(36.0);
static double kfSinGH;		// = sin(kfGH);
static double kfCosGH;		// = cos(kfGH);

static double kfCot30;		// = 1.0 / tan(MathUtils::kf30Rad);

/*
The spherical distance in radians from the centre of the polygon face to any of
its vertices on the globe. Approximately 37.3773681406498 degrees.
*/
static double kfDH;			// = asin(Icosahedron::kfSideLength / MathUtils::kfSqrt3);
static double kfCosDH;		// = cos(kfDH);
static double kfTanDH;		// = tan(kfDH);
static double kfTanDHSqr;	// = tan(kfDH) * tan(kfDH)

static double kfSinGHCosDH;	// = sin(kfGH) * cos(kfDH);

/*
The radius of an icosahedron relative to its enclosing sphere for equal area
projection. Approximately 0.9103832815095.
*/
static double kfR1;
static double kfR1Sqr; // = kfR1 * kfR1;

/*
The formula for kfOriginXOffset was derived from the Snyder forward projection
method by specifying a lat/lon that results in the maximum x value.
*/
static double kfOriginXOffset;	// = kfR1 * kfTanDH * sin(MathUtils::kf120Rad);
static double kfOriginYOffset;	// = kfOriginXOffset * tan(MathUtils::kf30Rad);
static double kfEdgeScale;		// = 2.0 * kfOriginXOffset;

//! The singleton instance of the snyder projection
boost::intrusive_ptr<const SnyderProjection> SnyderProjection::m_spInstance = 0;

//! Tester class
Tester<SnyderProjection> gTester;

//! Test method
void SnyderProjection::test()
{
	const SnyderProjection* pProjection = getInstance();

	PYXIcosIterator it(6);

	for (; !it.end(); it.next())
	{
		const PYXIcosIndex& index = it.getIndex();

		CoordLatLon ll;
		pProjection->pyxisToNative(index, &ll);

		PYXIcosIndex indexResult;
		pProjection->nativeToPYXIS(	ll,
									&indexResult,
									index.getResolution()	);

		TEST_ASSERT(indexResult == index);

		// test the convenience methods
		PYXCoord3DDouble coord;
		pProjection->pyxisToXYZ(index, &coord);
		pProjection->xyzToPYXIS(coord, &indexResult, index.getResolution());
		TEST_ASSERT(indexResult == index);

	}

	// test forward and reverse projection with lat/lon for precision
	for (double fLat = -90.0; fLat <= 90.0; fLat += 2.5)
	{
		for (double fLon = -180.0; fLon < 180.0; fLon += 2.5)
		{
			CoordLatLon ll;
			ll.setInDegrees(fLat, fLon);

			PYXCoordPolar ra;
			char cFace;
			pProjection->projectToFace(ll, &ra, &cFace);
			CoordLatLon llResult;
			pProjection->projectToSphere(ra, cFace, &llResult);

			// the best precision we can achieve through a forward and reverse projection
			const double kfTestPrecision = 1.0e-10;
			TEST_ASSERT(ll.equal(llResult, kfTestPrecision));
		}
	}

	//Test Area Calculations.
	{
		PYXIcosIndex hexIndex = "A-0";
		PYXIcosIndex pentIndex = "5-0";

		double fCalcHexAreaOnUnitSphere = 
			pProjection->calcCellAreaOnUnitSphere(hexIndex);
		double fCalcHexAreaOnRefSphere = 
			pProjection->calcCellAreaOnReferenceSphere(hexIndex);

		double fCalcPentAreaOnUnitSphere = 
			pProjection->calcCellAreaOnUnitSphere(pentIndex);
		double fCalcPentAreaOnRefSphere = 
			pProjection->calcCellAreaOnReferenceSphere(pentIndex);

		double fPIRefSphereCalc = fCalcHexAreaOnRefSphere * (30 * 
			pow(3.0, hexIndex.getResolution())) / 4 * (ReferenceSphere::kfRadius, 2.0);

		double fPIUnitSphereCalc = (fCalcHexAreaOnUnitSphere * (30 * (pow(3.0, hexIndex.getResolution() - 1)))) / 4.0;
	
		TEST_ASSERT(MathUtils::equal(fPIUnitSphereCalc, MathUtils::kfPI));

		TEST_ASSERT(MathUtils::equal(((5.0/6.0) * fCalcHexAreaOnUnitSphere), 
			fCalcPentAreaOnUnitSphere));

		TEST_ASSERT(MathUtils::equal(((5.0/6.0) * fCalcHexAreaOnRefSphere),  fCalcPentAreaOnRefSphere));

	}

	{
		// randomized polar to index
		for (int count = 0; count < 10; ++count)
		{
			// randomize a class 2 resolution 
			int nResolution = rand() % 28 + 2;

			// Testing conversion over the full contained area of a PYXIS cell.
			PYXIcosIndex testIndex;
			testIndex.randomize(nResolution);

			// spiral iterate over it 7 resolutions down, convert to xyx. Convert back
			// to PYXIS and verify every index converts to testIndex
			PYXPointer<PYXSpiralIterator> spIt = PYXSpiralIterator::create(testIndex, 7, -1);
			for (; !spIt->end(); spIt->next())
			{
				PYXCoord3DDouble coord;
				PYXIcosIndex startIndex = spIt->getIndex();
				pProjection->pyxisToXYZ(spIt->getIndex(), &coord);
				PYXIcosIndex newIndex;
				pProjection->xyzToPYXIS(coord, &newIndex, testIndex.getResolution());
				if (!(newIndex == testIndex))
				{
					std::string strTestIndex = testIndex.toString();
					TRACE_ERROR("Test failed converting index " + strTestIndex);
					TEST_ASSERT(newIndex == testIndex);
				}
			}
		}
	}

	{
		// This is an exhaustive verison of the test above. The full test would take over 2,000,000
		// years to run.  So, we are going to only test every 100,000,000th case and hopefully the test 
		// will run in under 2 weeks.

		// WARNING:  do not uncomment this test unless you want to run for two weeks or more!

		//long counter = 0;
		//for (int nResolution = 2; nResolution < 30; ++nResolution)
		//{
		//	PYXPointer<PYXIcosIterator> spIcosIt = PYXIcosIterator::create(nResolution);
		//	// Testing conversion over the full contained area of a PYXIS cell.
		//	while (!spIcosIt->end())
		//	{
		//		PYXIcosIndex testIndex = spIcosIt->getIndex();

		//		// spiral iterate over it 7 resolutions down, convert to xyz. Convert back
		//		// to PYXIS and verify every index converts to testIndex
		//		PYXPointer<PYXSpiralIterator> spIt = PYXSpiralIterator::create(testIndex, 7, -1);
		//		for (; !spIt->end(); spIt->next())
		//		{
		//			counter += 1;
		//			if (counter >= 100000000)
		//			{
		//				PYXCoord3DDouble coord;
		//				PYXIcosIndex startIndex = spIt->getIndex();
		//				pProjection->pyxisToXYZ(spIt->getIndex(), &coord);
		//				PYXIcosIndex newIndex;
		//				pProjection->xyzToPYXIS(coord, &newIndex, testIndex.getResolution());
		//				if (!(newIndex == testIndex))
		//				{
		//					std::string strTestIndex = testIndex.toString();
		//					TRACE_ERROR("Test failed converting index " + strTestIndex);
		//					TEST_ASSERT(newIndex == testIndex);
		//				}
		//				counter = 0;
		//			}
		//		}
		//	    spIcosIt->next();
		//	}
		//}
	}
}

/*!
Get the singleton instance of the Snyder projection.

\return	The singleton instance of the Snyder projection.
*/
const SnyderProjection* SnyderProjection::getInstance()
{
	// the static instance is stored as an intrusive pointer but is being 
	// returned as a raw pointer
	assert(m_spInstance);
	return m_spInstance.get();
}

//! Clone. (unsupported)
boost::intrusive_ptr<ICoordConverter> SnyderProjection::clone() const
{
	assert(false && "SnyderProjection cannot be cloned.");

	return 0;
}

/*!
Free singleton instance.
*/
void SnyderProjection::freeStaticData()
{
	// cleanup memory
	m_spInstance = 0;
}

/*!
Initialize the static data.
*/
void SnyderProjection::initStaticData()
{
	kfCot30 = 1.0 / tan(MathUtils::kf30Rad);

	/*
	The spherical angle in degrees between a radius vector to the centre and the
	adjacent edge of a spherical polygon on the globe.
	*/
	kfGH = MathUtils::degreesToRadians(36.0);
	kfSinGH = sin(kfGH);
	kfCosGH = cos(kfGH);

	/*
	The spherical distance in radians from the centre of the polygon face to any of
	its vertices on the globe. Approximately 37.3773681406498 degrees.
	*/
	kfDH = asin(Icosahedron::kfSideLength / MathUtils::kfSqrt3);
	kfCosDH = cos(kfDH);
	kfTanDH = tan(kfDH);
	kfTanDHSqr = kfTanDH * kfTanDH;

	kfSinGHCosDH = kfSinGH * kfCosDH;

	/*
	The radius of an icosahedron relative to its enclosing sphere for equal area
	projection. Approximately 0.9103832815095.
	*/
	kfR1 = sqrt(MathUtils::kfPI / (	15 * 
									kfTanDHSqr * 
									MathUtils::kfSin30 * 
									MathUtils::kfCos30		));

	kfR1Sqr = kfR1 * kfR1;

	/*
	The formula for kfOriginXOffset was derived from the Snyder forward projection
	method by specifying a lat/lon that results in the maximum x value.
	*/
	kfOriginXOffset = kfR1 * kfTanDH * sin(MathUtils::kf120Rad);
	kfOriginYOffset = kfOriginXOffset * tan(MathUtils::kf30Rad);
	kfEdgeScale = 2.0 * kfOriginXOffset;

	/*
	This section determines the distance between resolution 0 PYXIS cells in both
	metres and radians.
	*/
	double sqrt5 = sqrt(5.0);
	double phi = (1 + sqrt5) / 2;              // golden ratio
	double fR = sqrt(phi * sqrt5) / 2;         // circumradius of unit icosahedron
	kfUnitRadianDistance = asin(0.5 / fR) * 2; // radians between icosahedron vertices
	kfUnitMetreDistance = ReferenceSphere::kfRadius * kfUnitRadianDistance;

	m_spInstance = new SnyderProjection();
}

/*!
Constructs the Snyder projection for the default orientation of the icosahedron.
*/
SnyderProjection::SnyderProjection() : m_sphIcosa()
{
}

/*!
Convert a point on the sphere to a PYXIS index at a given resolution.

\param	native		The point on the sphere as a geocentric lat lon.
\param	pIndex		The index (out)
\param	nResolution	The resolution of the resulting index.
*/
void SnyderProjection::nativeToPYXIS(	const PYXCoord2DDouble& native,
										PYXIcosIndex* pIndex,
										int nResolution	) const
{
	CoordLatLon coord;
	coord.setInDegrees(native[0], native[1]);
	nativeToPYXIS(coord, pIndex, nResolution);
}

/*! 
Convert PYXIS indices to geocentric lat lon.
*/
void SnyderProjection::pyxisToNative(	const PYXIcosIndex& index,
										PYXCoord2DDouble* pNative	) const
{
	CoordLatLon coord;
	pyxisToNative(index, &coord);
	pNative->setX(coord.latInDegrees());
	pNative->setY(coord.lonInDegrees());
}

/*! 
Convert PYXIS indices to geocentric lat lon.
*/
bool SnyderProjection::tryPyxisToNative(	const PYXIcosIndex& index,
											PYXCoord2DDouble* pNative	) const
{
	//this will always work
	pyxisToNative(index,pNative);
	return true;
}

/*!
Convert a point on the sphere to a PYXIS index at a given resolution.

\param	ll			The point on the sphere in lat/lon coordinates
\param	pIndex		The index (out)
\param	nResolution	The resolution of the resulting index.
*/
void SnyderProjection::nativeToPYXIS(	const CoordLatLon& ll,
										PYXIcosIndex* pIndex,
										int nResolution	) const
{
	assert(1 <= nResolution);

	// calculate the polar coordinate from the centre of the face
	PYXCoordPolar ra;
	char cFace;
	projectToFace(ll, &ra, &cFace);

	PYXIcosMath::polarToIndex(ra, nResolution, cFace, pIndex);
}

/*!
Convert a PYXIS index to a point on the sphere.

\param	index	The PYXIS index.
\param	pll		The point on the sphere in lat/lon coordinates (out)

\return	The point on the sphere in lat/lon coordinates
*/
void SnyderProjection::pyxisToNative(	const PYXIcosIndex& index,
										CoordLatLon* pll	) const
{
	assert(!index.isNull());
	
	char cFace;
	PYXCoordPolar pt;
	PYXIcosMath::indexToPolar(index, &pt, &cFace);

	projectToSphere(pt, cFace, pll);
}

/*!
This is a convenience method to encapsulate the conversion between x,y,z 
coordinates and geocentric lat lon.

\param coord		The x, y, z coordinate to convert to a PYXIS index.
\param pIndex		The index to accept the new PYXIS index (out).
\param nResolution	The resolution of the resulting index.
*/
void SnyderProjection::xyzToPYXIS(	const PYXCoord3DDouble& coord,
									PYXIcosIndex* pIndex,
									int nResolution	) const
{
	assert(1 <= nResolution);
	assert(pIndex != 0);

	CoordLatLon ll = SphereMath::xyzll(coord);
	nativeToPYXIS(ll, pIndex, nResolution);
}

/*!
This is a convenience method to encapsulate the conversion between x,y,z 
coordinates and geocentric lat lon.

\param index	The index to convert to an x, y, z 
\param pCoord	The point on the surface of the sphere that the index represents.
*/
void SnyderProjection::pyxisToXYZ(	const PYXIcosIndex& index,
									PYXCoord3DDouble* pCoord	) const
{
	assert(pCoord != 0);
	assert(!index.isNull());

	CoordLatLon ll;
	pyxisToNative(index, &ll);
	SphereMath::llxyz(ll, pCoord);
}


//! Convert a native coordinate to a geocentric LatLon coordinates (convert degree into radains)
void SnyderProjection::nativeToLatLon(const PYXCoord2DDouble& native,
								CoordLatLon * pLatLon) const
{
	assert(pLatLon);
	pLatLon->setInDegrees(native.x(),native.y());
}

//! Convert a native coordinate to a geocentric LatLon coordinates (convert radains into degree)
void SnyderProjection::latLonToNative(const CoordLatLon & latLon,
								PYXCoord2DDouble * pNative) const
{
	assert(pNative);
	pNative->setX(latLon.latInDegrees());
	pNative->setY(latLon.lonInDegrees());
}

/*!
Project a point on a sphere to a face on the icosahedron.

\param	ll		The point on the sphere in lat/lon coordinates.
\param	pPolar	The polar coordinates relative to the centre of the triangle. The angle
				is measured counter-clockwise from the base of the triangle and the
				radius is specified such that each side of the face has a length of one
				unit.
\param	pcFace	The face of the icosahedron (out)
*/
void SnyderProjection::projectToFace(	const CoordLatLon& ll,
										PYXCoordPolar* pPolar,
										char* pcFace	) const
{
	if (0 != pPolar)
	{
		char nFace;

		// find the face (triangle) in which the point lies
		nFace = m_sphIcosa.findFace(ll);
		if (nFace < 0)
		{
			// no triangle found
			assert(false);

			pPolar->setRadius(0.0);
			pPolar->setAngle(0.0);
		}
		else
		{
			/*
			Perform the Snyder forward projection to determine the point in the
			triangle.
			*/
			*pPolar = sllra(ll, nFace);
		}

		if (0 != pcFace)
		{
			*pcFace = PYXIcosIndex::kcFaceFirstChar + nFace;
		}
	}
}

/*!
Using the Snyder projection, project a point specified in lat/lon coordinates
onto a face on the icosahedron. Return the distance and angle from the centre
of the face where the angle is measured counter-clockwise from the base of the
triangle and each side of the face has a length of one unit.

\param	ll		The point in lat/lon coordinates
\param	nFace	The face in which the point lies

\return	The polar coordinate relative to the centre of the face.
*/
PYXCoordPolar SnyderProjection::sllra (	const CoordLatLon& ll,
										int nFace	) const
{
	// get the precomputed values for the face from the icosahedron
	const Icosahedron::Face& face = m_sphIcosa.getFace(nFace);
	const PreCompLatLon& centre = face.sphTriCentre();

	// precompute some values
	double fCosLat = cos(ll.lat());
	double fSinLat = sin(ll.lat());
	double fDeltaLon = ll.lon() - centre.point().lon();

	/*
	Calculate the great circle distance between the point and the triangle's
	centre point. Note for longitude differences < 90 degrees, it is not
	necessary to take the absolute value of the longitude difference.
	*/
	// formula 13 from Snyder paper
	double fZ = acos(	(centre.sinLat() * fSinLat) +
						(centre.cosLat() * fCosLat * cos(fDeltaLon))	);

	// sanity check, must be less than distance from centre to any vertex
	if (fZ > kfDH + kfPrecision)
	{
		// the point is located on another polygon
		assert(false);
	}

	/*
	Calculate the azimuth of the vector from the centre to the point relative
	to the triangle's azimuth.
	*/
	// formula 14 from Snyder paper
	double fAZH = atan2(	fCosLat * sin(ll.lon() - centre.point().lon()), 
							centre.cosLat() * fSinLat -
							centre.sinLat() * fCosLat * cos(fDeltaLon)) -
							face.azimuth();

	// convert to a value in the range [0, 360)
	if (fAZH < 0.0)
	{
		fAZH = fAZH + 2.0 * MathUtils::kfPI;
	}

	/*
	Divide the triangle into three sub-triangles each formed by two vertices
	and the triangle's centre point. Rotate the azimuth so it falls in the
	range [0, 120).
	*/
	double fRotate;
	if (fAZH < MathUtils::kf120Rad)
	{
		fRotate = 0.0;
	}
	else if (fAZH <= MathUtils::kf240Rad)
	{
		fRotate = MathUtils::kf120Rad;
	}
	else if (fAZH > MathUtils::kf240Rad)
	{
		fRotate = MathUtils::kf240Rad;
	}

	fAZH -= fRotate;

	// precompute some more values
	double fSinAZH = sin(fAZH);
	double fCosAZH = cos(fAZH);

	/*
	Calculate the great circle distance from the centre of the face to the edge
	of the sub-triangle along the rotated azimuth.
	*/
	double fDZ = atan2(kfTanDH, fCosAZH + kfCot30 * fSinAZH);

	// sanity check, distance from point to centre must be less than this
	if (fZ > fDZ + kfPrecision)
	{
		// the point is located on another polygon
		assert(false);
	}

	// formula 6 from Snyder paper
	double fH = acos(fSinAZH * kfSinGHCosDH - fCosAZH * kfCosGH);

	// calculate spherical excess
	double fAG = fAZH + kfGH + fH - MathUtils::kf180Rad;

	// formula 8 from Snyder paper
	double fAZH1 = atan2(	2.0 * fAG,
							kfR1Sqr * kfTanDHSqr - 2.0 * fAG * kfCot30	);

	// formulae 10, 11 from Snyder paper
	double fFH = kfTanDH / (2.0 * (cos(fAZH1) + kfCot30 * 
								sin(fAZH1)) * sin(fDZ / 2.0));

	// formula 12 from Snyder paper
	double fPH = 2.0 * kfR1 * fFH * sin(fZ / 2.0);

	// un-rotate the azimuth
	fAZH1 += fRotate;

	// measure angle CCW from base of triangle
	double fAngle = MathUtils::kf90Rad - fAZH1;

	return PYXCoordPolar(fPH / kfEdgeScale, fAngle);
}

/*!
Project a point on an icosahedron face to a sphere. The point must be specified
in polar coordinates relative to the centre of the face where each side of the
face has a length of one unit and the angle is measured counter-clockwise from
the base of the triangle.

\param	ra		The point on the icosahedron face in polar coordinates.
\param	cFace	The icosahedron face.
\param	pll		The point on the sphere in lat/lon coordinates (out).
*/
void SnyderProjection::projectToSphere(	const PYXCoordPolar& ra,
										char cFace,
										CoordLatLon* pll	) const
{
	if (0 != pll)
	{
		const Icosahedron::Face& face = m_sphIcosa.getFace(cFace - PYXIcosIndex::kcFaceFirstChar);
		const PreCompLatLon& centre = face.sphTriCentre();

		if (MathUtils::equal(ra.radius(), 0.0))
		{
			// we are at the centre of the triangle, use the centre point
			*pll = centre.point();
		}
		else
		{
			/*
			Formula 17 from Snyder paper. Convert to angle measured CW from top of
			triangle.
			*/
			double fAZH1 = MathUtils::kf90Rad - ra.angle();

			// convert to value in the range [0, 360)
			if (fAZH1 < 0.0)
			{
				fAZH1 = fAZH1 + 2 * MathUtils::kfPI;
			}

			/*
			Divide the triangle into three sub-triangles each formed by two vertices
			and the triangle's centre point. Rotate the azimuth so it falls in the
			range [0, 120).
			*/
			double fRotate;
			if (fAZH1 <= MathUtils::kf120Rad)
			{
				fRotate = 0.0;
			}
			else if (fAZH1 <= MathUtils::kf240Rad)
			{
				fRotate = MathUtils::kf120Rad;
			}
			else if (fAZH1 > MathUtils::kf240Rad)
			{
				fRotate = MathUtils::kf240Rad;
			}

			fAZH1 -= fRotate;
			double fAZH = fAZH1;

			// for a non-zero azimuth
			if (!MathUtils::equal(fAZH1, 0.0)) 
			{
				// formula 19 from Snyder paper
				double agh =	kfR1Sqr * kfTanDHSqr /
								(2.0 * (1.0 / tan(fAZH1) + kfCot30));

				// iterate to determine azimuth
				double fDAZH = 1.0; 
				while (!MathUtils::equal(fDAZH, 0.0))
				{
					double fH = acos(sin(fAZH) * kfSinGHCosDH - cos(fAZH) * kfCosGH);

					// formula 20 from Snyder paper
					double fFAZH = agh - fAZH - kfGH - fH + MathUtils::kfPI;

					// formula 21 from Snyder paper
					double fFLAZH = ((cos(fAZH) * kfSinGHCosDH + sin(fAZH) * kfCosGH) / sin(fH)) - 1.0;      

					// formula 22 from Snyder paper
					fDAZH = -fFAZH / fFLAZH;
					fAZH = fAZH + fDAZH;
				}   
			}
			else
			{
				fAZH = 0.0;
				fAZH1 = 0.0;
			}

			// formula 9 from Snyder paper
			double fDZ = atan2(kfTanDH, cos(fAZH) + kfCot30 * sin(fAZH));

			// formulae 10, 11 from Snyder paper
			double fFH = kfTanDH / (2.0 * 
									(cos(fAZH1) + kfCot30 * 
									 sin(fAZH1)) * sin(fDZ / 2.0));

			// formula 18 from Snyder paper
			double fPH = ra.radius() * kfEdgeScale;

			// formula 23 from Snyder paper
			double fZ = 2.0 * asin(fPH / (2.0 * kfR1 * fFH));
			
			// un-rotate the azimuth
			fAZH += fRotate;
			fAZH += face.azimuth();

			// convert to value in the range (-180, 180]
			while (fAZH <= -MathUtils::kf180Rad)
			{
				fAZH += MathUtils::kf360Rad;
			}

			while (fAZH > MathUtils::kf180Rad)
			{
				fAZH -= MathUtils::kf360Rad;
			}

			// calculate latitude
			double fSinLat = centre.sinLat() * cos(fZ) + centre.cosLat() * sin(fZ) * cos(fAZH);

			fSinLat = std::min(fSinLat, 1.0);
			fSinLat = std::max(fSinLat, -1.0);

			pll->setLat(asin(fSinLat));

			if (pll->isNorthPole() || pll->isSouthPole())
			{
				pll->setLon(0.0);
			}
			else
			{
				// calculate longitude
				double fSinLon = sin(fAZH) * sin(fZ) / cos(pll->lat());
				fSinLon = std::min(fSinLon, 1.0);
				fSinLon = std::max(fSinLon, -1.0);

				double fCosLon =	(cos(fZ) - centre.sinLat() * sin(pll->lat())) /
									centre.cosLat() / cos(pll->lat());
				fCosLon = std::min(fCosLon, 1.0);
				fCosLon = std::max(fCosLon, -1.0);

				pll->setLon(centre.point().lon() + atan2(fSinLon, fCosLon));
			}
		}
	}
}

/*!
Convert a precision specified in arc radians to a resolution. This method
returns the resolution with cell areas most closely fits the area of influence
without exceeding it.

\param	fPrecision	The precision in arc radians. Must be between 0 (exclusive)
					and Icosahedron::kfCentralAngle (inclusive).

\return	The resolution.
*/
int SnyderProjection::precisionToResolution(double fPrecision) const
{
	// convert precision from a radius to a diameter
	fPrecision *= 2.0;

	if (!(0 < fPrecision && fPrecision <= Icosahedron::kfCentralAngle))
	{
		PYXTHROW(	PYXSnyderException,
					"Invalid precision: '" << fPrecision << "'."	);
	}

	int nResolution = 0;
	double fAngle = Icosahedron::kfCentralAngle;

	while (fPrecision < fAngle)
	{
		fAngle /= MathUtils::kfSqrt3;
		++nResolution;
	}

	return nResolution;
}

/*!
Convert a resolution to a precision specified in arc radians.

\param	nResolution	The PYXIS resolution. Must be between 0 (inclusive)
		and PYXMath::knMaxAbsResolution (inclusive).

\return	The precision in arc radians.
*/
double SnyderProjection::resolutionToPrecision(int nResolution) const
{
	if (!(0 <= nResolution && nResolution <= PYXMath::knMaxAbsResolution))
	{
		PYXTHROW(	PYXSnyderException,
					"Invalid resolution: '" << nResolution << "'."	);
	}

	double fAngle = Icosahedron::kfCentralAngle;

	while (0 < nResolution)
	{
		fAngle /= MathUtils::kfSqrt3;
		--nResolution;
	}

	// convert precision from a diameter to a radius
	fAngle /= 2.0;

	return fAngle;
}

/* 
Based on the precision of the entries, calculate the PYXIS resolution.

\param strLatitude The latitude.
\param strLongitude The longitude.

return The PYXIS resolution.
*/
int SnyderProjection::calculateResolution
									(	const std::string& strLatitude,
										const std::string& strLongitude	) const
{
	// get the resolution in radians
	double fRadians = CoordLatLon::calculateResolutionRadians
												(	strLatitude, 
													strLongitude	);

	// calculate the precision in metres
	double fPrecision = fRadians * ReferenceSphere::kfRadius;

	// convert to a PYXIS resolution
	int nPYXISResolution = precisionToResolution(fRadians);

	// return the resolution
	return nPYXISResolution;
}

//! Serialize. (unsupported)
std::basic_ostream< char>& SnyderProjection::serialize(std::basic_ostream< char>& out) const
{
	assert(false && "Serialization not implemented.");
	return out;
}

//! Deserialize.  (unsupported)
std::basic_istream< char>& SnyderProjection::deserialize(std::basic_istream< char>& in)
{
	assert(false && "Deserialization not implemented.");
	return in;
}

//! Serialize the COM object.
void SnyderProjection::serializeCOM(std::basic_ostream< char>& out) const
{
	out << clsid;
	serialize(out);
}

/*!
Calculates the area of a cell at a given resolution 
on the unit sphere. 
Forumula:
Area(hexagon) = 4 x PI /30 * (3^(resolution - 1).
Area(pentagon) = (5/6) * (4 x PI /30 * (3^(resolution - 1)).

\param index	The icos index of the cell to calculate the area for.

\return The area of a cell on the unit sphere,
		represented by the icos index on the unit sphere.
*/
double SnyderProjection::calcCellAreaOnUnitSphere(
	const PYXIcosIndex &index) const
{
	assert(!index.isNull() && "Invalid index for area calculation.");
	
	double fHexArea = (4.0 * MathUtils::kfPI) / 
			(30 * pow(3.0, (index.getResolution() - 1.0)));

	if (index.isPentagon())
	{
		return (5.0/6.0) * fHexArea;
	}
	else 
	{
		return fHexArea;
	}
}

/*!
Calculates the area of a cell at a given resolution on the reference 
sphere.
Forumula:
Area(hexagon) = 4 x PI * r^2/30 * (3^(resolution - 1).
Area(pentagon) = (5/6) * (4 x PI * r^2 /30 * (3^(resolution - 1)).

\param index	The icos index of the cell to calculate an area of.

\return The area of a cell on the reference sphere represented by the icos index.
*/
double SnyderProjection::calcCellAreaOnReferenceSphere(
	const PYXIcosIndex &index) const
{
	assert(!index.isNull() && "Invalid index for area calculation.");

	return pow(ReferenceSphere::kfRadius, 2) * 
		calcCellAreaOnUnitSphere(index);
}

/*!
Calculates the approximate distance from the center of one cell 
to the center of a neighbouring cell on the reference sphere. Can 
also be thought of as the width of a cell. 

\param nResolution	The resolution of the cells to calculate a distance for.

\return The distance in meters between the centers of neighbouring cells at a 
        given resolution.
*/
double SnyderProjection::calcCellDistanceOnReferenceSphere(
	int nResolution) const
{
	assert(nResolution > 0 && "Invalid resolution.");
	double fApproxDistanceRadians = 
		Icosahedron::kfCentralAngle * 
		PYXMath::calcInterCellDistance(nResolution);
	return (fApproxDistanceRadians * 
		ReferenceSphere::getInstance()->kfRadius);
}
