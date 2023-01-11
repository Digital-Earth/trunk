#ifndef GRIB_COORD_CONVERTER_H
#define GRIB_COORD_CONVERTER_H
/******************************************************************************
grib_coord_converter.h

begin		: 2006-03-29
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "grib.h"

// pyxlib includes
#include "pyxis/derm/coord_converter.h"
#include "pyxis/utility/coord_lat_lon.h"
#include "pyxis/utility/coord_2d.h"
#include "pyxis/sampling/spatial_reference_system.h"

// local forward declarations
class PYXIcosIndex;

//! Provides coordinate conversion facilities for GDAL and OGR data sources.
/*!
GRIBCoordConverter provides methods for converting from the specified spatial
reference system to geocentric and PYXIS coordinate and back again.
*/
class GRIB_DECL GRIBCoordConverter : public PYXCoordConverter
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
	GRIBCoordConverter();

	//! Deserialization constructor.
	explicit GRIBCoordConverter(std::istream& in);

	//! Destructor.
	virtual ~GRIBCoordConverter();

	//! Clone.
	virtual boost::intrusive_ptr<ICoordConverter> clone() const { return new GRIBCoordConverter(*this); }

	//! Initialize with a PYXSpatialReferenceSystem
	void initialize(double fLonStart, double fLatStart,
					double fLonStep, double fLatStep);

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
	bool isProjected() const {return false;}

	//! Equality operator.
	bool operator==(const GRIBCoordConverter& conversion) const;

	//! Serialize the COM object.
	virtual void serializeCOM(std::basic_ostream</*unsigned*/ char>& out) const;

	//! Deserialize.
	virtual std::basic_istream</*unsigned*/ char>& deserialize(std::basic_istream</*unsigned*/ char>& in);

	//! Serialize.
	virtual std::basic_ostream</*unsigned*/ char>& serialize(std::basic_ostream</*unsigned*/ char>& out) const;

private:

	//! Copy constructor.
	GRIBCoordConverter(const GRIBCoordConverter&);

	//! Copy assignment not implemented.
	GRIBCoordConverter& operator=(const GRIBCoordConverter&);

	//! Convert native coordinates to WGS84 coordinates.
	void nativeToWGS84(	const PYXCoord2DDouble& native,
						CoordLatLon* pLatLon	) const;

	//! Convert WGS84 coordinates to native coordinates.
	void wgs84ToNative(	const CoordLatLon& latLon,
						PYXCoord2DDouble* pNative	) const;

	//! Convert WGS84 coordinates to a PYXIS index.
	void wgs84ToPYXIS(	const CoordLatLon& ll,
						PYXIcosIndex* pIndex,
						int nResolution	) const;

	//! Convert a PYXIS index to WGS84 coordinates.
	void pyxisToWGS84(	const PYXIcosIndex& index,	
						CoordLatLon* pll	) const;

	//! Initialize the transforms.
	void initializeTransforms();

	//! Get the system datum as string.
	std::string getDatumString(PYXSpatialReferenceSystem::eDatum nDatum);

	//! Get the system projection as string.
	std::string getProjectionString(PYXSpatialReferenceSystem::eProjection nProjection);

	//! Start of longitude for first coordinate of bounds.
	double	m_fLonStart;

	//! Start of latitude for first coordinate of bounds.
	double	m_fLatStart;

	//! Longitude step.
	double m_fLonStep;

	//! Latitude step.
	double m_fLatStep;
};

#endif	// GDAL_COORD_CONVERTER_H
