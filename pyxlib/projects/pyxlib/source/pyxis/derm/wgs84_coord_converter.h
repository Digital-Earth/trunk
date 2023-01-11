#ifndef WGS84_COORD_CONVERTER_H
#define WGS84_COORD_CONVERTER_H
/******************************************************************************
wgs84_coord_converter.h

begin		: 2005-09-28
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxlib.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/utility/coord_2d.h"

// standard includes

// local forward declarations
class PYXIcosIndex;

/*!
	This class employs a scaling factor, common values for which constants are supplied.
	For example, if the native coordinates are in geodetic seconds, use the constant
	knSecondsPerDegree. The default is 1 (i.e. geodetic degrees).
*/
//! Provides coordinate conversion facilities for WGS84.
class PYXLIB_DECL WGS84CoordConverter : public PYXCoordConverter
{
	PYXCOM_DECLARE_CLASS();

public:

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(PYXCOM_IUnknown)
		IUNKNOWN_QI_CASE(ICoordConverter)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public:

	enum
	{
		//! Use this if the native coordinates are geodetic degrees.
		knDegreesPerDegree = 1,
		//! Use this if the native coordinates are geodetic minutes.
		knMinutesPerDegree = 60,
		//! Use this if the native coordinates are geodetic seconds.
		knSecondsPerDegree = knMinutesPerDegree * 60,
		//! Use this if the native coordinates are geodetic tenths of seconds.
		knTenthsOfSecondsPerDegree = knSecondsPerDegree * 10
	};

	//! Test method
	static void test();

	//! Constructor.
	explicit WGS84CoordConverter(int nUnitsPerDegree = knDegreesPerDegree) : m_fUnitsPerDegree(nUnitsPerDegree) {}

	//! Deserialization constructor.
	explicit WGS84CoordConverter(std::basic_istream<char>& in);

	//! Destructor.
	virtual ~WGS84CoordConverter() {}

	//! Clone.
	virtual boost::intrusive_ptr<ICoordConverter> clone() const { return new WGS84CoordConverter(*this); }

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

	//! Are the native coordinates (WGS84) projected?
	virtual bool isProjected() const { return false; }


	//! Serialize.
	virtual std::basic_ostream<char>& serialize(std::basic_ostream<char>& out) const;

	//! Deserialize.
	virtual std::basic_istream<char>& deserialize(std::basic_istream<char>& in);

	//! Serialize the COM object.
	virtual void serializeCOM(std::basic_ostream<char>& out) const;

private:

	//! Copy constructor.
	WGS84CoordConverter(const WGS84CoordConverter& other) : m_fUnitsPerDegree(other.m_fUnitsPerDegree) {}

	//! Copy assignment not implemented.
	WGS84CoordConverter& operator=(const WGS84CoordConverter&);

	//! The number of units per degree. (It's a double to avoid type promotion upon use.)
	double m_fUnitsPerDegree;

};

#endif	// WGS84_COORD_CONVERTER_H
