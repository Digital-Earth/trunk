#ifndef COORD_CONVERTER_IMPL_H
#define COORD_CONVERTER_IMPL_H
/******************************************************************************
coord_converter_impl.h

begin		: 2004-10-15
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "module_driver_utility.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/utility/coord_2d.h"
#include "pyxis/utility/coord_lat_lon.h"
#include "pyxis/utility/rect_2d.h"
#include "pyxis/utility/app_services.h"

// GDAL includes
#include "ogr_spatialref.h"

/////////////////////////////////////////////////////////////////////
//GDAL THREAD SAFETY [shatzi:29/10/2013]
//when upgrading from GDAL1.8 with Proj4.5 to GDAL1.10 with Proj4.8,
//this class causes heap corruptions as it not thread safe.
//We are testing this class thread safety by:
//creating 10K of those objects on 8 threads. (see the test on cpp file)
//
//remove this when we think GDAL calls to PROJ4 are thread safe.
#define OGR_PROJ_IS_NOT_THREAD_SAFE
/////////////////////////////////////////////////////////////////////


// local forward declarations
class PYXIcosIndex;

// OGR forward declarations
class OGRSpatialReference;

//! An implementation of a coordinate converter based on OGR utility classes.
/*!
CoordConverterImpl provides methods for converting from the specified spatial
reference system to geocentric and PYXIS coordinates and back again.
*/
class MODULE_DRIVER_UTILITY_DECL CoordConverterImpl : public PYXCoordConverter
{
	PYXCOM_DECLARE_CLASS();

public:

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(PYXCOM_IUnknown)
		IUNKNOWN_QI_CASE(ICoordConverter)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public:
	
	//! Test method
	static void test();

	//! Constructor.
	CoordConverterImpl();

	//! Destructor.
	virtual ~CoordConverterImpl();

	//! Serialize.
	std::basic_ostream</*unsigned*/ char>& serialize(std::basic_ostream< char>& out) const;

	//! Deserialize.
	std::basic_istream</*unsigned*/ char>& deserialize(std::basic_istream< char>& in);

	//! Serialize the COM object.
	virtual void serializeCOM(std::basic_ostream</*unsigned*/ char>& out) const;

	//! Clone.
	virtual boost::intrusive_ptr<ICoordConverter> clone() const { return new CoordConverterImpl(*this); }

	//! Initialize with a PYXSpatialReferenceSystem
	void initialize(const PYXSpatialReferenceSystem& srs);

	//! Initialize with Well Known Text
	void initialize(const std::string& strWKT);

	//! Initialize with an OGRSpatialReference
	void initialize(const OGRSpatialReference& osrs);

	//! return the Well Known Text (WKT) for this coord convertor
	std::string exportToWellKnownText();

	/*!
	Set the bounding rectangle in native coordinates. For geographic and geocentric spatial
	reference systems, the native lon coordinate will be adjusted to fit within the range
	of the native bounds if possible.

	\param rNativeBounds	The bounding rectangle in native coordinates.
	*/
	void setNativeBounds(const PYXRect2DDouble& rNativeBounds) { m_rNativeBounds = rNativeBounds; }

	//! Convert native coordinates to WGS84 coordinates.
	void nativeToWGS84(	const PYXCoord2DDouble& native,
						CoordLatLon* pLatLon	) const;

	//! Convert WGS84 coordinates to native coordinates.
	void wgs84ToNative(	const CoordLatLon& latLon,
						PYXCoord2DDouble* pNative	) const;

	//! Convert WGS84 coordinates to native coordinates.
	bool tryWgs84ToNative(	const CoordLatLon& latLon,
							PYXCoord2DDouble* pNative	) const;

	//! Convert WGS84 coordinates to a PYXIS index.
	void wgs84ToPYXIS(	const CoordLatLon& ll,
						PYXIcosIndex* pIndex,
						int nResolution	) const;

	//! Convert a PYXIS index to WGS84 coordinates.
	void pyxisToWGS84(	const PYXIcosIndex& index,	
						CoordLatLon* pll	) const;

	//! Convert native coordinates to a PYXIS index.
	virtual void nativeToPYXIS(	const PYXCoord2DDouble& native,
								PYXIcosIndex* pIndex,
								int nResolution	) const;

	//! Convert a PYXIS index to native coordinates.
	virtual void pyxisToNative(	const PYXIcosIndex& index,
								PYXCoord2DDouble* pNative	) const;

	//! Convert a PYXIS index to native coordinates.
	virtual bool tryPyxisToNative(	const PYXIcosIndex& index,
									PYXCoord2DDouble* pNative	) const;

	//! Convert a native coordinate to a geocentric LatLon coordinates.
	virtual void nativeToLatLon(const PYXCoord2DDouble& native,
								CoordLatLon * pLatLon) const;

	//! Convert a native coordinate to a geocentric LatLon coordinates.
	virtual void latLonToNative(const CoordLatLon & latLon,
								PYXCoord2DDouble * pNative) const;

	/*!
	Determine if the native coordinate system is projected.

	\return	true if projected, otherwise false.
	*/
	bool isProjected() const {return m_bNativeIsProjected;}

	//! Equality operator.
	bool operator==(const CoordConverterImpl& conversion) const;

private:

	//! Copy constructor.
	CoordConverterImpl(const CoordConverterImpl&);

	//! Copy assignment not implemented.
	CoordConverterImpl& operator=(const CoordConverterImpl&);

	//! Initialize the transforms.
	void initializeTransforms();

	//! Get the system datum as string.
	std::string getDatumString(PYXSpatialReferenceSystem::eDatum nDatum);

	//! Get the system projection as string.
	std::string getProjectionString(PYXSpatialReferenceSystem::eProjection nProjection);

	//! Serialize OGR spatial reference.
	static std::basic_ostream</*unsigned*/ char>& serialize(std::basic_ostream</*unsigned*/ char>& out, const OGRSpatialReference& ogr);

	//! Deserialize OGR spatial reference.
	static std::basic_istream</*unsigned*/ char>& deserialize(std::basic_istream</*unsigned*/ char>& in, OGRSpatialReference& ogr);

private:

	//! The spatial reference system for the native data source.
	OGRSpatialReference m_SRSNative;

	//! Flag to differentiate between normal WGS84 with one that has lon_wrap
	bool m_containsLonWrap;

	//! The spatial reference system for WGS84
	OGRSpatialReference m_SRSWGS84;
	
	//! The transform that converts from native to WGS84 coordinates.
	OGRCoordinateTransformation* m_pNativeToWGS84;

	//! The transform that converts from WGS84 to native coordinates.
	OGRCoordinateTransformation* m_pWGS84ToNative;

	//! Is the native spatial reference system projected?
	bool m_bNativeIsProjected;

	//! The boundary of the coordinate conversion in WGS84 coordinates. (note: currently set to [-180 -> 180, -90 -> 90 ])
	PYXRect2DDouble m_rWGS84Bounds;

	//! The boundary of the coordinate conversion in native coordinates.
	PYXRect2DDouble m_rNativeBounds;

	//! The longitude adjustment to apply to Native coordinates if out of range
	double m_fNativeLonAdjustment;

#ifdef OGR_PROJ_IS_NOT_THREAD_SAFE
	static AppExecutionScope s_gdalScope;
#endif
};

class CoordConverterImplFactory : public ICoordConverterFromSrsFactory
{
	PYXCOM_DECLARE_CLASS();

public:

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(PYXCOM_IUnknown)
		IUNKNOWN_QI_CASE(ICoordConverterFromSrsFactory)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public:
	virtual boost::intrusive_ptr<ICoordConverter> createFromSRS(const PYXPointer<PYXSpatialReferenceSystem> & srs);

	virtual boost::intrusive_ptr<ICoordConverter> createFromWKT(const std::string & wellKnownText);

	virtual PYXPointer<PYXSpatialReferenceSystem> convertToSRS(const boost::intrusive_ptr<ICoordConverter> & coordConverter);
};

#endif	// COORD_CONVERTER_IMPL_H
