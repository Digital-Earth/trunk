#ifndef PYXIS__GEOMETRY__GEOMETRY_SERIALIZER_H
#define PYXIS__GEOMETRY__GEOMETRY_SERIALIZER_H
/******************************************************************************
geometry_serializer.h

begin		: 2007-08-23
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/geometry/curve.h"
#include "pyxis/utility/wire_buffer.h"

// standard includes
#include <string>

// forward declarations
class PYXGeometry;
class PYXGlobalGeometry;
class PYXTileCollection;
class PYXXYBoundsGeometry;
class PYXCircleGeometry;
class PYXVectorGeometry2;
template <class T> class PYXMultiGeometry;

/*!
This class manages serialization of geometries.

NOTE: the serialized strings are not encoded in Utf8! it better to cover them with XMLUtils::toBase64() or writeHex.
*/
//! Serializes and deserializes a geometry.
class PYXLIB_DECL PYXGeometrySerializer
{
	// Serialization format.
	// TO DO: Add native support for other geometry formats.
	enum eFormat
	{
		knTileCollection = 1,
		knGlobalGeometry = 2,
		knXYBoundsGeometry = 3,
		knCircleGeometry = 4,
		knMultiGeometry = 5,
		knCurve = 6,
		knVectorGeometry=7
	};

public:

	//! Test method.
	static void test();

	//! Deserialize a geometry from stream.
	static PYXPointer<PYXGeometry> deserialize(std::basic_istream<char>& in);

	//! Deserialize a geometry from a string.
	static PYXPointer<PYXGeometry> deserialize(const std::string& strIn);

	//! Deserialize a geometry from a PYXConstBufferSlice.
	static PYXPointer<PYXGeometry> deserialize(const PYXConstBufferSlice& slice);

	//! Serialize a geometry to stream.
	static void serialize(const PYXPointer<PYXGeometry> & geometry, std::basic_ostream<char>& out);	

	static void serialize(const PYXTileCollection& geometry, std::basic_ostream<char>& out);
	static void serialize(const PYXGlobalGeometry& globalGeometry, std::basic_ostream<char>& out);
	static void serialize(const PYXXYBoundsGeometry& xyBoundsGeometry, std::basic_ostream<char>& out);
	static void serialize(const PYXGeometry& geometry, std::basic_ostream<char>& out);	
	static void serialize(const PYXCircleGeometry& geometry, std::basic_ostream<char>& out);
	static void serialize(const PYXMultiGeometry<PYXGeometry>& geometry, std::basic_ostream<char>& out);
	static void serialize(const PYXMultiGeometry<PYXCurve>& geometry, std::basic_ostream<char>& out);	
	static void serialize(const PYXCurve& geometry, std::basic_ostream<char>& out);
	static void serialize(const PYXVectorGeometry2& geometry, std::basic_ostream<char>& out);

	//! Serialize a tile colection to a string.
	static std::string serialize(const PYXTileCollection& geometry);

	//! Serialize a global geometry to a string.
	static std::string serialize(const PYXGlobalGeometry& globalGeometry);

	//! Serialize a circle geometry to a string.
	static std::string serialize(const PYXCircleGeometry& geometry);

	//! Serialize any other geometry to a string.
	static std::string serialize(const PYXGeometry& geometry);

	static std::string serialize(const PYXPointer<PYXGeometry> & geometry);	

	//! Serialize a MultiGeometry to a string.
	template<class T> static std::string serialize(const PYXMultiGeometry<T>& geometry);
};

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXGeometry & geom);
PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXGeometry> & geom);

#endif // guard
