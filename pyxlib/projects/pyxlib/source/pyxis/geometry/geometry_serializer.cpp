/******************************************************************************
geometry_serializer.cpp

begin		: 2007-08-23
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/geometry/geometry_serializer.h"

#include "pyxis/derm/wgs84_coord_converter.h"

#include "pyxis/geometry/circle_geometry.h"
#include "pyxis/geometry/multi_geometry.h"
#include "pyxis/geometry/polygon.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/geometry/vector_geometry2.h"

#include "pyxis/sampling/xy_bounds_geometry.h"

#include "pyxis/utility/app_services.h"
#include "pyxis/utility/rect_2d.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/file_utils.h"

// standard includes
#include <fstream>

//! Tester class
Tester<PYXGeometrySerializer> gTester(knRunTestFirst);

//! Test method
void PYXGeometrySerializer::test()
{
	// string to test string serialization methods
	std::string strGeometry;

	// Save and load a tile collection.
	{
		// Construct a tile collection.
		PYXTileCollection original;
		original.addTile(PYXIcosIndex("1-02"), 15);
		original.addTile(PYXIcosIndex("1-03"), 15);
		original.addTile(PYXIcosIndex("3-02"), 15);
		original.addTile(PYXIcosIndex("3-03"), 15);

		// Serialize.
		strGeometry = PYXGeometrySerializer::serialize(original);
		std::string strPath = FileUtils::pathToString(AppServices::makeTempFile(".test"));
		{
			std::basic_ofstream< char> out(strPath.c_str(), std::ios_base::binary);
			PYXGeometrySerializer::serialize(original, out);
		}

		// Deserialize from string.
		PYXPointer<PYXGeometry> spFromString = PYXGeometrySerializer::deserialize(strGeometry);
		TEST_ASSERT(spFromString);
		PYXTileCollection* pTc = dynamic_cast<PYXTileCollection*>(spFromString.get());
		TEST_ASSERT(pTc != 0);
		TEST_ASSERT(pTc->isEqual(original));

		// Cast it back to a tile collection and compare to the original.
		try
		{
			PYXTileCollection& loaded = dynamic_cast<PYXTileCollection&>(*spFromString);
			TEST_ASSERT(loaded == original);
		}
		catch (...)
		{
			TEST_ASSERT(0 && "The deserialized geometry from string is of the wrong type (should be PYXTileCollection).");
		}

		// deserialize from stream.
		PYXPointer<PYXGeometry> spLoaded;
		{
			std::basic_ifstream< char> in(strPath.c_str(), std::ios_base::binary);
			spLoaded = PYXGeometrySerializer::deserialize(in);
		}
		TEST_ASSERT(spLoaded);

		// Cast it back to a tile collection and compare to the original.
		try
		{
			PYXTileCollection& loaded = dynamic_cast<PYXTileCollection&>(*spLoaded);
			TEST_ASSERT(loaded == original);
		}
		catch (...)
		{
			TEST_ASSERT(0 && "The deserialized geometry is of the wrong type (should be PYXTileCollection).");
		}
	}

	// Save and load a PYXGlobalGeometry (supported type).
	{
		PYXGlobalGeometry original(10);

		// Serialize.
		strGeometry = PYXGeometrySerializer::serialize(original);
		std::string strPath = FileUtils::pathToString(AppServices::makeTempFile(".test"));
		{
			std::basic_ofstream< char> out(strPath.c_str(), std::ios_base::binary);
			PYXGeometrySerializer::serialize(original, out);
		}

		// Deserialize from string.
		PYXPointer<PYXGeometry> spFromString = PYXGeometrySerializer::deserialize(strGeometry);
		TEST_ASSERT(spFromString);

		// Cast it back to a global geometry and compare to the original.
		try
		{
			PYXGlobalGeometry& loaded = dynamic_cast<PYXGlobalGeometry&>(*spFromString);
			TEST_ASSERT(loaded == original);
		}
		catch (...)
		{
			TEST_ASSERT(0 && "The deserialized geometry from string is of the wrong type (should be PYXGlobalGeometry).");
		}

		// deserialize from stream.
		PYXPointer<PYXGeometry> spLoaded;
		{
			std::basic_ifstream< char> in(strPath.c_str(), std::ios_base::binary);
			spLoaded = PYXGeometrySerializer::deserialize(in);
		}
		TEST_ASSERT(spLoaded);

		// Cast it back to a global geometry and compare to the original.
		try
		{
			PYXGlobalGeometry& loaded = dynamic_cast<PYXGlobalGeometry&>(*spLoaded);
			TEST_ASSERT(loaded == original);
		}
		catch (...)
		{
			TEST_ASSERT(0 && "The deserialized geometry is of the wrong type (should be PYXGlobalGeometry).");
		}
	}

	// Save and load a PYXXYBoundsGeometry (supported type).
	{
		PYXRect2DDouble bounds(-180, -90, 180, 90);
		WGS84CoordConverter converter;
		PYXXYBoundsGeometry original(bounds, converter, 12);

		// Serialize.
		strGeometry = PYXGeometrySerializer::serialize(original);
		std::string strPath = FileUtils::pathToString(AppServices::makeTempFile(".test"));
		{
			std::basic_ofstream< char> out(strPath.c_str(), std::ios_base::binary);
			PYXGeometrySerializer::serialize(original, out);
		}

		// Deserialize from string.
		PYXPointer<PYXGeometry> spFromString = PYXGeometrySerializer::deserialize(strGeometry);
		TEST_ASSERT(spFromString);

		// Get the tile collection and compare to original.
		PYXTileCollection saved;
		original.copyTo(&saved);
		{
			PYXTileCollection loaded;
			spFromString->copyTo(&loaded);
			TEST_ASSERT(loaded == saved);
		}

		// deserialize from stream.
		PYXPointer<PYXGeometry> spLoaded;
		{
			std::basic_ifstream< char> in(strPath.c_str(), std::ios_base::binary);
			spLoaded = PYXGeometrySerializer::deserialize(in);
		}
		TEST_ASSERT(spLoaded);

		// Get the tile collection and compare to original.
		{
			PYXTileCollection loaded;
			spLoaded->copyTo(&loaded);
			TEST_ASSERT(loaded == saved);
		}
	}

	// Save and load an unsupported type.
	{
		// Spherical polygon around hex A.
		PYXPolygon polygon;
		polygon.addVertex(PYXIcosIndex("1-30"));
		polygon.addVertex(PYXIcosIndex("3-20"));
		polygon.addVertex(PYXIcosIndex("3-30"));
		polygon.addVertex(PYXIcosIndex("2-60"));
		polygon.addVertex(PYXIcosIndex("2-20"));
		polygon.addVertex(PYXIcosIndex("1-20"));

		// Get original tile collection for verification later.
		PYXTileCollection original;
		polygon.copyTo(&original);

		// Serialize the polygon.
		strGeometry = PYXGeometrySerializer::serialize(original);
		std::string strPath = FileUtils::pathToString(AppServices::makeTempFile(".test"));
		{
			std::basic_ofstream< char> out(strPath.c_str(), std::ios_base::binary);
			PYXGeometrySerializer::serialize(polygon, out);
		}

		// Deserialize from string.
		PYXPointer<PYXGeometry> spFromString = PYXGeometrySerializer::deserialize(strGeometry);
		TEST_ASSERT(spFromString);

		// Get the tile collection and compare to original.
		{
			PYXTileCollection loaded;
			spFromString->copyTo(&loaded);
			TEST_ASSERT(loaded == original);
		}

		// deserialize from stream.
		PYXPointer<PYXGeometry> spLoaded;
		{
			std::basic_ifstream< char> in(strPath.c_str(), std::ios_base::binary);
			spLoaded = PYXGeometrySerializer::deserialize(in);
		}
		TEST_ASSERT(spLoaded);

		// Get the tile collection and compare to original.
		{
			PYXTileCollection loaded;
			spLoaded->copyTo(&loaded);
			TEST_ASSERT(loaded == original);
		}
	}

	{//Test the serialization of a circular Geometry.
		PYXPointer<PYXCircleGeometry> spCircGeometry = PYXCircleGeometry::create(PYXIcosIndex("A-00000000000000"), 10000);
		{ 
			// Serialize To a file
			std::string strPath = FileUtils::pathToString(AppServices::makeTempFile(".test"));
			{
				std::basic_ofstream< char> out(strPath.c_str(), std::ios_base::binary);
				PYXGeometrySerializer::serialize(*spCircGeometry, out);
			}
			std::basic_ifstream< char> in(strPath.c_str(), std::ios_base::binary);
			PYXPointer<PYXGeometry> spGeom = PYXGeometrySerializer::deserialize(in);
			TEST_ASSERT(spGeom);
			PYXPointer<PYXCircleGeometry> spDeserializedCircleGeometry = boost::dynamic_pointer_cast<PYXCircleGeometry, PYXGeometry>(spGeom);
			TEST_ASSERT(spDeserializedCircleGeometry);
			TEST_ASSERT(*spDeserializedCircleGeometry == *spCircGeometry);
		}

		{ //Serialize to a String.
			std::string strGeometry = PYXGeometrySerializer::serialize(*spCircGeometry);
			PYXPointer<PYXGeometry> spGeom = PYXGeometrySerializer::deserialize(strGeometry);
			TEST_ASSERT(spGeom);
			PYXPointer<PYXCircleGeometry> spDeserializedCircleGeometry = boost::dynamic_pointer_cast<PYXCircleGeometry, PYXGeometry>(spGeom);
			TEST_ASSERT(spDeserializedCircleGeometry);
			TEST_ASSERT(*spDeserializedCircleGeometry == *spCircGeometry);
		}
	}

	// Test the (de)serialization of PYXCurve.
	{
		PYXCurve original;
		original.addNode(PYXIcosIndex("8-060500000"));
		original.addNode(PYXIcosIndex("8-060500002"));
		original.addNode(PYXIcosIndex("8-060500020"));
		
		// Test the (de)serialization to and from a string.
		{
			strGeometry = PYXGeometrySerializer::serialize(original);
			TEST_ASSERT(strGeometry.length() > 0 && "Unable to serialize PYXCurve to a string!");
			
			PYXPointer<PYXGeometry> spDeserializedGeometry = 
				PYXGeometrySerializer::deserialize(strGeometry);
			TEST_ASSERT(spDeserializedGeometry && "Unable to deserialize PYXCurve!");

			PYXPointer<PYXCurve> spDeserializedCurve = boost::dynamic_pointer_cast<PYXCurve>(spDeserializedGeometry);
			TEST_ASSERT(spDeserializedCurve && "Unable to cast deserialized geometry to PYXCurve!");

			TEST_ASSERT(original == *(spDeserializedCurve.get()) && "PYXCurves are not the same!");
		}

		// Test the (de)serialization to and from a stream.
		{
			std::string strPath = FileUtils::pathToString(AppServices::makeTempFile(".test"));
			{
				std::basic_ofstream< char> out(strPath.c_str(), std::ios_base::binary);
				PYXGeometrySerializer::serialize(original, out);
			}

			std::basic_ifstream< char> in(strPath.c_str(), std::ios_base::binary);
			PYXPointer<PYXGeometry> spDeserializedGeometry = PYXGeometrySerializer::deserialize(in);
			TEST_ASSERT(spDeserializedGeometry && "Unable to deserialize PYXCurve!");

			PYXPointer<PYXCurve> spDeserializedCurve = boost::dynamic_pointer_cast<PYXCurve>(spDeserializedGeometry);
			TEST_ASSERT(spDeserializedCurve && "Unable to cast deserialized geometry to PYXCurve!");

			TEST_ASSERT(original == *(spDeserializedCurve.get()) && "PYXCurves are not the same!");
		}
	}

	// Test the (de)serialization of PYXMultiGeometry of curves.
	{
		PYXPointer<PYXCurve> spCurve1 = PYXCurve::create();
		spCurve1->addNode(PYXIcosIndex("8-060500000"));
		spCurve1->addNode(PYXIcosIndex("8-060500002"));
		spCurve1->addNode(PYXIcosIndex("8-060500020"));
		
		PYXPointer<PYXCurve> spCurve2 = PYXCurve::create();
		spCurve2->addNode(PYXIcosIndex("E-004000002"));
		spCurve2->addNode(PYXIcosIndex("E-004000000"));		
		
		PYXPointer<PYXCurve> spCurve3 = PYXCurve::create();
		spCurve3->addNode(PYXIcosIndex("6-600010300"));
		spCurve3->addNode(PYXIcosIndex("6-600010302"));
		spCurve3->addNode(PYXIcosIndex("6-600104050"));
		spCurve3->addNode(PYXIcosIndex("6-600104006"));

		PYXPointer<PYXMultiGeometry<PYXCurve> > spMultiGeometry = PYXMultiGeometry<PYXCurve>::create();
		spMultiGeometry->addGeometry(spCurve1);
		spMultiGeometry->addGeometry(spCurve2);
		spMultiGeometry->addGeometry(spCurve3);

		PYXPointer<PYXGeometry> spOriginal = boost::dynamic_pointer_cast<PYXGeometry>(spMultiGeometry);
		
		// Test the (de)serialization to and from a string.
		{
			strGeometry = PYXGeometrySerializer::serialize(*(spOriginal.get()));
			TEST_ASSERT(strGeometry.length() > 0 && "Unable to serialize PYXMultiGeometry to a string!");
			
			PYXPointer<PYXGeometry> spDeserializedGeometry = 
				PYXGeometrySerializer::deserialize(strGeometry);
			TEST_ASSERT(spDeserializedGeometry && "Unable to deserialize PYXMultiGeometry!");

			PYXPointer<PYXMultiGeometry<PYXGeometry> > spDeserializedMultiGeometry = 
				boost::dynamic_pointer_cast<PYXMultiGeometry<PYXGeometry> >(spDeserializedGeometry);
			TEST_ASSERT(spDeserializedMultiGeometry && "Unable to cast deserialized geometry to PYXMultiGeometry<PYXGeometry>!");

			TEST_ASSERT(*(spOriginal.get()) == *(spDeserializedMultiGeometry.get()) && "PYXMultiGeometries are not the same!");
		}

		// Test the (de)serialization to and from a stream.
		{
			std::string strPath = FileUtils::pathToString(AppServices::makeTempFile(".test"));
			{
				std::basic_ofstream< char> out(strPath.c_str(), std::ios_base::binary);
				PYXGeometrySerializer::serialize(*(spOriginal.get()), out);
			}

			std::basic_ifstream<char> in(strPath.c_str(), std::ios_base::binary);
			PYXPointer<PYXGeometry> spDeserializedGeometry = PYXGeometrySerializer::deserialize(in);
			TEST_ASSERT(spDeserializedGeometry && "Unable to deserialize PYXMultiGeometry!");

			PYXPointer<PYXMultiGeometry<PYXGeometry> > spDeserializedMultiGeometry = 
				boost::dynamic_pointer_cast<PYXMultiGeometry<PYXGeometry> >(spDeserializedGeometry);
			TEST_ASSERT(spDeserializedMultiGeometry && "Unable to cast deserialized geometry to PYXMultiGeometry<PYXGeometry>!");

			TEST_ASSERT(*(spOriginal.get()) == *(spDeserializedMultiGeometry.get()) && "PYXMultiGeometries are not the same!");
		}
	}
}

/*!
Deserialize a geometry from the stream.

\return	Null if geometry could not be deserialized.
*/
PYXPointer<PYXGeometry> PYXGeometrySerializer::deserialize(std::basic_istream<char>& in)
{
	char nFormat = knTileCollection;
	if (in.get(nFormat))
	{
		switch (nFormat)
		{
		case knTileCollection:
			return PYXTileCollection::create(in);
		case knGlobalGeometry:
			return PYXGlobalGeometry::create(in);
		case knXYBoundsGeometry:
			return PYXXYBoundsGeometry::create(in);
		case knCircleGeometry:
			return PYXCircleGeometry::create(in);
		case knMultiGeometry:
			return PYXMultiGeometry<PYXGeometry>::create(in);
		case knCurve:
			return PYXCurve::create(in);
		case knVectorGeometry:
			return PYXVectorGeometry2::create(in);
		default:
			// Invalid type.
			// TO DO: Possibly throw exception.
			assert(false && "Invalid geometry serializer format.");
		}
	}
	return PYXPointer<PYXGeometry>();
}

PYXPointer<PYXGeometry> PYXGeometrySerializer::deserialize(const std::string& strIn)
{
	//std::basic_string<unsigned char> strCompatible(reinterpret_cast<const unsigned char*>(strIn.c_str()),
	//	strIn.length());
	std::istringstream inStream(strIn, std::istringstream::binary | std::istringstream::in );
	return deserialize(inStream);
}

PYXPointer<PYXGeometry> PYXGeometrySerializer::deserialize(const PYXConstBufferSlice& slice)
{
	PYXConstWireBuffer buffer(slice);
	unsigned char nFormat;
	buffer.read(nFormat);
	if (nFormat == knVectorGeometry)
	{
		int length;
		buffer >> length;
		return PYXVectorGeometry2::create(PYXConstWireBuffer(buffer.read(length)));
	}
	return deserialize((std::string)slice);
}


//! Serialize a Vector Geometry II to the stream.
void PYXGeometrySerializer::serialize(const PYXVectorGeometry2& vectorGeometry, std::basic_ostream<char>& out)
{
	const unsigned char nFormat = knVectorGeometry;
	out.put(nFormat);
	vectorGeometry.serialize(out);
}

//! Serialize a tile collection to the stream.
void PYXGeometrySerializer::serialize(const PYXTileCollection& tileCollection, std::basic_ostream<char>& out)
{
	const unsigned char nFormat = knTileCollection;
	out.put(nFormat);
	tileCollection.serialize(out);
}

//! Serialize a global geometry to the stream.
void PYXGeometrySerializer::serialize(const PYXGlobalGeometry& globalGeometry, std::basic_ostream<char>& out)
{
	const unsigned char nFormat = knGlobalGeometry;
	out.put(nFormat);
	globalGeometry.serialize(out);
}

//! Serialize a global geometry to the stream.
void PYXGeometrySerializer::serialize(const PYXXYBoundsGeometry& xyBoundsGeometry, std::basic_ostream<char>& out)
{
	const unsigned char nFormat = knXYBoundsGeometry;
	out.put(nFormat);
	xyBoundsGeometry.serialize(out);
}

/*!
Serialize a circular geometry to the stream. Serialization is achieved by serializing the type of the geomety to
the stream for deserialization purposes. Then we delegate to the circular geometry to serialize itself to the 
same stream.

\param geometry	The geometry to serialize to the stream. 
\param out	    The stream to serialize the geometry to.

*/
void PYXGeometrySerializer::serialize(const PYXCircleGeometry &geometry, std::basic_ostream<char> &out)
{
	const unsigned char nFormat = knCircleGeometry;
	out.put(nFormat);
	geometry.serialize(out);
}

/*!
Serialize a multi-geometry to the stream. Serialization is achieved by serializing the type of the geomety to
the stream for deserialization purposes. Then we delegate to the multi-geometry to serialize itself to the 
same stream.

\param geometry	The geometry to serialize to the stream. 
\param out	    The stream to serialize the geometry to.

*/
void PYXGeometrySerializer::serialize(const PYXMultiGeometry<PYXGeometry> &geometry, std::basic_ostream<char> &out)
{
	const unsigned char nFormat = knMultiGeometry;
	out.put(nFormat);
	geometry.serialize(out);
}

/*!
Serialize a multi-geometry to the stream. Serialization is achieved by serializing the type of the geomety to
the stream for deserialization purposes. Then we delegate to the multi-geometry to serialize itself to the 
same stream.

\param geometry	The geometry to serialize to the stream. 
\param out	    The stream to serialize the geometry to.

*/
void PYXGeometrySerializer::serialize(const PYXMultiGeometry<PYXCurve> &geometry, std::basic_ostream<char> &out)
{
	const unsigned char nFormat = knMultiGeometry;
	out.put(nFormat);
	geometry.serialize(out);
}

/*!
Serialize a curve to the stream. Serialization is achieved by serializing the type of the geomety to
the stream for deserialization purposes. Then we delegate to the curve to serialize itself to the 
same stream.

\param geometry	The geometry to serialize to the stream. 
\param out	    The stream to serialize the geometry to.

*/
void PYXGeometrySerializer::serialize(const PYXCurve &geometry, std::basic_ostream<char> &out)
{
	const unsigned char nFormat = knCurve;
	out.put(nFormat);
	geometry.serialize(out);
}

//! Serialize a geometry to the stream.
void PYXGeometrySerializer::serialize(const PYXGeometry& geometry, std::basic_ostream<char>& out)
{
	//TODO: Someday this should be handled better by making the serialize method virtual in all geometries.
	PYXPointer<PYXGeometry> spGeometry = geometry.clone();
	serialize(spGeometry,out);
}
//! Serialize a geometry to the stream.
void PYXGeometrySerializer::serialize(const PYXPointer<PYXGeometry> & spGeometry, std::basic_ostream<char>& out)
{	
	VALIDATE_ARGUMENT_NOT_NULL(spGeometry);	

	if (boost::dynamic_pointer_cast<PYXTileCollection, PYXGeometry> (spGeometry))
	{
		serialize(*(boost::dynamic_pointer_cast<PYXTileCollection, PYXGeometry>(spGeometry)), out);
	}
	else if (boost::dynamic_pointer_cast<PYXGlobalGeometry, PYXGeometry> (spGeometry))
	{
		serialize(*(boost::dynamic_pointer_cast<PYXGlobalGeometry, PYXGeometry> (spGeometry)), out);
	}
	else if (boost::dynamic_pointer_cast<PYXXYBoundsGeometry, PYXGeometry> (spGeometry))
	{
		serialize(*(boost::dynamic_pointer_cast<PYXXYBoundsGeometry, PYXGeometry> (spGeometry)), out);
	}	
	else if (boost::dynamic_pointer_cast<PYXCircleGeometry, PYXGeometry> (spGeometry))
	{
		serialize(*(boost::dynamic_pointer_cast<PYXCircleGeometry, PYXGeometry> (spGeometry)), out);
	}
	else if (boost::dynamic_pointer_cast<PYXMultiGeometry<PYXGeometry> >(spGeometry))
	{
		serialize(*(boost::dynamic_pointer_cast<PYXMultiGeometry<PYXGeometry> >(spGeometry)), out);
	}
	else if (boost::dynamic_pointer_cast<PYXMultiGeometry<PYXCurve> >(spGeometry))
	{
		serialize(*(boost::dynamic_pointer_cast<PYXMultiGeometry<PYXCurve> >(spGeometry)), out);
	}
	else if (boost::dynamic_pointer_cast<PYXCurve>(spGeometry))
	{
		serialize(*(boost::dynamic_pointer_cast<PYXCurve>(spGeometry)), out);
	}
	else if (boost::dynamic_pointer_cast<PYXVectorGeometry2>(spGeometry))
	{
		serialize(*(boost::dynamic_pointer_cast<PYXVectorGeometry2>(spGeometry)), out);
	}
	else
	{
		PYXTileCollection tileCollection;
		spGeometry->copyTo(&tileCollection);
		serialize(tileCollection, out);
	}
}

std::string PYXGeometrySerializer::serialize(const PYXTileCollection& geometry)
{
	std::basic_string< char> strOut;
	std::basic_ostringstream<char> outStream(strOut);
	serialize(geometry, outStream);
	return outStream.str();
}

std::string PYXGeometrySerializer::serialize(const PYXGlobalGeometry& globalGeometry)
{
	std::basic_string< char> strOut;
	std::basic_ostringstream<char> outStream(strOut);
	serialize(globalGeometry, outStream);
	return outStream.str();
}

std::string PYXGeometrySerializer::serialize(const PYXGeometry& geometry)
{
	//TODO: Someday this should be handled better by making the serialize method virtual in all geometries.	
	return serialize(geometry.clone());	
}

std::string PYXGeometrySerializer::serialize(const PYXPointer<PYXGeometry> & geometry)
{
	std::basic_string< char> strOut;
	std::basic_ostringstream<char> outStream(strOut);
	serialize(geometry, outStream);
	outStream << std::ends;	
	return outStream.str();
}

std::string PYXGeometrySerializer::serialize(const PYXCircleGeometry &geometry)
{
	std::basic_string<char> strOut;
	std::basic_ostringstream<char> outStream(strOut);
	serialize(geometry, outStream);
	return outStream.str();
}


///////////////////////////////////////////////////////////////////////////////
// PYXWireBuffer
///////////////////////////////////////////////////////////////////////////////

PYXLIB_DECL PYXWireBuffer & operator <<(PYXWireBuffer & buffer,const PYXGeometry & geom)
{
	std::string str = PYXGeometrySerializer::serialize(geom);
	buffer << str;

	return buffer;
}

PYXLIB_DECL PYXWireBuffer & operator >>(PYXWireBuffer & buffer,PYXPointer<PYXGeometry> & geom)
{
	PYXConstBufferSlice slice;
	buffer >> slice;
	geom = PYXGeometrySerializer::deserialize(slice);
	return buffer;
}

