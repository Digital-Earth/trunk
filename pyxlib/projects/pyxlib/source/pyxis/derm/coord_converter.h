#ifndef PYXIS__DERM__COORD_CONVERTER_H
#define PYXIS__DERM__COORD_CONVERTER_H
/******************************************************************************
coord_converter.h

begin		: 2004-10-15
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/coord_2d.h"
#include "pyxis/utility/coord_lat_lon.h"
#include "pyxis/utility/pyxcom.h"
#include "pyxis/sampling/spatial_reference_system.h"

// forward declarations
class PYXIcosIndex;

// boost includes
#include <boost/intrusive_ptr.hpp>

/*!
ICoordConverter is an abstract base interface that provides methods for
converting from a native coordinate system to PYXIS coordinates and
vice versa. It should be as fast as possible.

Ideas for making this interface faster (mlepage):

1) Add to the API the capability to pre-specify a resolution for subsequent
   operations. This avoids passing it as a parameter in most cases, since we
   usually work at a single resolution for a significant amount of calls.

2) Add to the API converters that operate on a series of coordinates at a
   time. This avoids multiple calls for several coordinates, and implementations
   can optimize for the common case where subsequent coordinates are close to
   each other spatially.
*/
//! Interface for classes that perform coordinate conversion.
class PYXLIB_DECL ICoordConverter : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! Clone.
	virtual boost::intrusive_ptr<ICoordConverter> clone() const = 0;

	/*!
	This is generally a much slower operation than pyxisToNative.

	\param	native		The native coordinate.
	\param	pIndex		The PYXIS index. (out)
	\param	nResolution	The desired PYXIS resolution.
	*/
	//! Convert a native coordinate to a PYXIS index.
	virtual void nativeToPYXIS(	const PYXCoord2DDouble& native,
								PYXIcosIndex* pIndex,
								int nResolution	) const = 0;

	/*!
	This is generally a much faster operation than nativeToPYXIS.
	If conversion is not possible, an expection would be thrown

	\param	index	The PYXIS index.
	\param	pNative	The native coordinate. (out)
	*/
	//! Convert a PYXIS index to a native coordinate.
	virtual void pyxisToNative(
		const PYXIcosIndex& index, 
		PYXCoord2DDouble* pNative	) const = 0;

	/*!
	This is generally a much faster operation than nativeToPYXIS.
	If conversion was successful, function returns true, else returns false.

	\param	index	The PYXIS index.
	\param	pNative	The native coordinate. (out)
	\return	bool	True if conversion was successful
	*/
	//! Convert a PYXIS index to a native coordinate.
	virtual bool tryPyxisToNative(
		const PYXIcosIndex& index, 
		PYXCoord2DDouble* pNative	) const = 0;

	/*!	
	\param	native		The native coordinate.
	\param	platLon		The geocentric latlon. (out)
	*/
	//! Convert a native coordinate to a geocentric LatLon coordinates.
	virtual void nativeToLatLon(const PYXCoord2DDouble& native,
								CoordLatLon * pLatLon) const = 0;

	/*!	
	\param	native		The native coordinate.
	\param	platLon		The geocentric latlon. (out)
	*/
	//! Convert a native coordinate to a geocentric LatLon coordinates.
	virtual void latLonToNative(const CoordLatLon & latLon,
								PYXCoord2DDouble * pNative) const = 0;

	//! Is the native coordinate system projected?
	virtual bool isProjected() const = 0;

	//! Serialize natively.
	virtual std::basic_ostream< char>& serialize(std::basic_ostream< char>& out) const = 0;

	//! Deserialize natively.
	virtual std::basic_istream< char>& deserialize(std::basic_istream< char>& in) = 0;

	//! Serialize the COM object.
	virtual void serializeCOM(std::basic_ostream< char>& out) const = 0;
};

class PYXLIB_DECL PYXCoordConverter : public ICoordConverter
{
public:

	//! Deserialize the COM object.
	static boost::intrusive_ptr<ICoordConverter> deserializeCOM(std::basic_istream< char>& in);	
};


/*!
ICoordConverterFromSrsFactory is an abstract base interface that provides implementations
to help users create ICoordConverter from SRS
*/
class PYXLIB_DECL ICoordConverterFromSrsFactory : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:
	virtual boost::intrusive_ptr<ICoordConverter> createFromSRS(const PYXPointer<PYXSpatialReferenceSystem> & srs) = 0;

	virtual boost::intrusive_ptr<ICoordConverter> createFromWKT(const std::string & wellKnownText) = 0;

	virtual PYXPointer<PYXSpatialReferenceSystem> convertToSRS(const boost::intrusive_ptr<ICoordConverter> & coordConverter) = 0;
};


/*!	
PYXAxisFlipCoordConverter - utility class for help GDAL drivers to flip coordinate when needed.

this class take the native coordinates and flip them before sending them to coord converter.
So, this what would happen for each of the translations:
(flip) - flip native coordinates.
(convert) - converting using the given coord converter.

nativeToPYXIS:  native ->  (flip)  -> fliped-native -> (convert) -> PYXIS
pyxisToNative:  PYXIS -> (convert) -> fliped-native ->  (flip)  -> native 

nativeToLatLon:  native ->  (flip)   -> fliped-native -> (convert) -> LatLon
latLonToNative:  LatLon -> (convert) -> fliped-native ->  (flip)   -> native 

As you can see, the AxisFlip will make sure that the coord converter will get and output flip-axis native results 
*/
class PYXLIB_DECL PYXAxisFlipCoordConverter : public PYXCoordConverter
{
	PYXCOM_DECLARE_CLASS();

public:
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(PYXCOM_IUnknown)
		IUNKNOWN_QI_CASE(ICoordConverter)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

public:
	//! Constructor.
	PYXAxisFlipCoordConverter();

	//! Constructor.
	PYXAxisFlipCoordConverter(const boost::intrusive_ptr<ICoordConverter> & coordConverter);


	//! Destructor.
	virtual ~PYXAxisFlipCoordConverter();

	void setInternalCoordConverter(const boost::intrusive_ptr<ICoordConverter> & coordConverter);

public:
	//! Serialize.
	virtual std::basic_ostream<char>& serialize(std::basic_ostream<char>& out) const;

	//! Deserialize.
	virtual std::basic_istream<char>& deserialize(std::basic_istream<char>& in);

	//! Serialize the COM object.
	virtual void serializeCOM(std::basic_ostream<char>& out) const;

	//! Clone.
	virtual boost::intrusive_ptr<ICoordConverter> clone() const;

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

	//! Is the native coordinate system projected?
	virtual bool isProjected() const { return m_spCoordConverter->isProjected(); }

private:
	boost::intrusive_ptr<ICoordConverter> m_spCoordConverter;
};


#endif	//guard
