#ifndef PYXIS__DERM__SNYDER_PROJECTION_H
#define PYXIS__DERM__SNYDER_PROJECTION_H
/******************************************************************************
snyder_projection.h

begin		: 2004-02-11
copyright	: derived from DgProjSnyder by Kevin Sahr
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxlib.h"
#include "pyxis/derm/icosahedron.h"
#include "pyxis/derm/projection_method.h"
#include "pyxis/utility/coord_polar.h"

/*!
SnyderProjection implements the Snyder equal area projection from a latitude /
longitude on a sphere to a point on the icosahedral plane. The inverse
projection is also implemented. The Snyder projection is a modified Lambert equal
area projection.
*/
//! Implements the Snyder equal area projection.
class PYXLIB_DECL SnyderProjection : public ProjectionMethod
{
	PYXCOM_DECLARE_CLASS();

public:

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(PYXCOM_IUnknown)
		IUNKNOWN_QI_CASE(ICoordConverter)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public:

	// Global constant values
	static double kfUnitRadianDistance;
	static double kfUnitMetreDistance;

	//! Test method
	static void test();

	//! Get the singleton instance of the snyder projection
	static const SnyderProjection* getInstance();

	//! Clone. (unsupported)
	virtual boost::intrusive_ptr<ICoordConverter> clone() const;

	//! Convert geocentric lat lon values to PYXIS indices.
	virtual void nativeToPYXIS(	const PYXCoord2DDouble& native,
								PYXIcosIndex* pIndex,
								int nResolution	) const;

	//! Convert PYXIS indices to geocentric lat lon.
	virtual void pyxisToNative(	const PYXIcosIndex& index,
								PYXCoord2DDouble* pNative	) const;

	//! Convert PYXIS indices to geocentric lat lon.
	virtual bool tryPyxisToNative(	const PYXIcosIndex& index,
								PYXCoord2DDouble* pNative	) const;

	//! Convert geocentric lat lon values to PYXIS indices.
	virtual void nativeToPYXIS(	const CoordLatLon& ll,
								PYXIcosIndex* pIndex,
								int nResolution	) const;

	//! Convert PYXIS indices to geocentric lat lon.
	virtual void pyxisToNative(	const PYXIcosIndex& index,
								CoordLatLon* pll	) const;

	//! Convert a native coordinate to a geocentric LatLon coordinates (convert degree into radains)
	virtual void nativeToLatLon(const PYXCoord2DDouble& native,
								CoordLatLon * pLatLon) const;
	
	//! Convert a native coordinate to a geocentric LatLon coordinates (convert radains into degree)
	virtual void latLonToNative(const CoordLatLon & latLon,
								PYXCoord2DDouble * pNative) const;

	//! Is the native coordinate system (Geocentric Lat/Lon) projected?
	virtual bool isProjected() const { return false; }

	//! Convert 3d coordinates to PYXIS indices.
	virtual void xyzToPYXIS(	const PYXCoord3DDouble& coord,
								PYXIcosIndex* pIndex,
								int nResolution	) const;

	//! Convert PYXIS indices to x, y, z coordinates.
	virtual void pyxisToXYZ(	const PYXIcosIndex& index,
								PYXCoord3DDouble* pCoord	) const;

	//! Convert a precision specified in arc radians to a resolution
	virtual int precisionToResolution(double fPrecision) const;

	//! Convert a PYXIS resolution to a precision in arc radians.
	virtual double resolutionToPrecision(int nResolution) const;

	//! Calculate the pyxis resolution of a geodetic coordinate.
	int calculateResolution(	const std::string& strLatitude,
								const std::string& strLongitude	) const;

	//! Serialize. (unsupported)
	virtual std::basic_ostream<char>& serialize(std::basic_ostream<char>& out) const;

	//! Deserialize.  (unsupported)
	virtual std::basic_istream<char>& deserialize(std::basic_istream<char>& in);

	//! Serialize the COM object.
	virtual void serializeCOM(std::basic_ostream<char>& out) const;

	//! Calculates the area of a cell on the unit sphere.
	virtual double calcCellAreaOnUnitSphere(const PYXIcosIndex& index) const;

	//! Calculates the area of a cell on the reference sphere.
	virtual double calcCellAreaOnReferenceSphere(const PYXIcosIndex& index) const;

	/*!
	Calculates the approximate distance between the centers of 
	neighbouring cells.
	*/
	virtual double calcCellDistanceOnReferenceSphere(int nResolution) const;

private:

	//! Constructor.
	SnyderProjection();

	//! Disable copy constructor
	SnyderProjection(const SnyderProjection&);

	//! Destructor
	virtual ~SnyderProjection() {}

	//! Disable copy assignment
	void operator=(const SnyderProjection&);

	//! Initialize any static data
	static void initStaticData();

	//! Free singleton instance
	static void freeStaticData();
	
	//! Project a point on a sphere to a face on the icosahedron.
	void projectToFace(	const CoordLatLon& ll,
						PYXCoordPolar* pPolar,
						char* pcFace	) const;

	//! Project a point on an icosahedron face to a sphere.
	void projectToSphere(	const PYXCoordPolar& ra,
							char cFace,
							CoordLatLon* pll	) const;

	//! Perform the Snyder projection from the sphere to a plane.
	PYXCoordPolar sllra (const CoordLatLon& ll, int nFace) const;

	//! The singleton instance of the snyder projection
	static boost::intrusive_ptr<const SnyderProjection> m_spInstance;

	//! The icosahedron
	Icosahedron m_sphIcosa;

	//! Allows PYXLibInstance to initialize the static data.
	friend class PYXLibInstance;
};

#endif // guard
