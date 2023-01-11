/******************************************************************************
coord_converter.cpp

begin		: 2004-10-15
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h" 
#include "pyxis/derm/coord_converter.h"

// {84F6342F-F18F-4434-8517-F73E9ED807DA}
PYXCOM_DEFINE_IID(ICoordConverter, 
0x84f6342f, 0xf18f, 0x4434, 0x85, 0x17, 0xf7, 0x3e, 0x9e, 0xd8, 0x7, 0xda);

//! Deserialize the COM object.
boost::intrusive_ptr<ICoordConverter> PYXCoordConverter::deserializeCOM(std::basic_istream<char>& in)
{
	// Read the class ID from the stream.
	IID clsID;
	in >> clsID;

	// Create the COM instance.
	boost::intrusive_ptr<ICoordConverter> spCoordConverter;
	PYXCOMCreateInstance(clsID, 0, ICoordConverter::iid, (void**)&spCoordConverter);

	if (spCoordConverter.get() == 0)
	{
		PYXTHROW(PYXException, "Could not create the coordinate converter.");
	}

	// Call deserialize on it.
	spCoordConverter->deserialize(in);
	
	// Return the COM instance.
	return spCoordConverter;
}

// {18741B03-78D8-405E-9DDF-1A4C5764EF18}
PYXCOM_DEFINE_IID(ICoordConverterFromSrsFactory, 
0x18741b03, 0x78d8, 0x405e, 0x9d, 0xdf, 0x1a, 0x4c, 0x57, 0x64, 0xef, 0x18);


// {1F1D8531-F594-4379-BFAD-0156D0AFC105}
PYXCOM_DEFINE_CLSID(PYXAxisFlipCoordConverter, 
0x1f1d8531, 0xf594, 0x4379, 0xbf, 0xad, 0x1, 0x56, 0xd0, 0xaf, 0xc1, 0x5);

PYXCOM_CLASS_INTERFACES(PYXAxisFlipCoordConverter, ICoordConverter::iid, PYXCOM_IUnknown::iid);


PYXAxisFlipCoordConverter::PYXAxisFlipCoordConverter()
{

}

PYXAxisFlipCoordConverter::PYXAxisFlipCoordConverter(const boost::intrusive_ptr<ICoordConverter> & coordConverter) : m_spCoordConverter(coordConverter)
{
}

PYXAxisFlipCoordConverter::~PYXAxisFlipCoordConverter()
{
}

void PYXAxisFlipCoordConverter::setInternalCoordConverter(const boost::intrusive_ptr<ICoordConverter> & coordConverter)
{
	m_spCoordConverter = coordConverter->clone();
}

std::basic_ostream<char>& PYXAxisFlipCoordConverter::serialize( std::basic_ostream<char>& out ) const
{
	m_spCoordConverter->serializeCOM(out);
	return out;
}

std::basic_istream<char>& PYXAxisFlipCoordConverter::deserialize( std::basic_istream<char>& in )
{
	m_spCoordConverter = PYXCoordConverter::deserializeCOM(in);
	return in;
}

void PYXAxisFlipCoordConverter::serializeCOM( std::basic_ostream<char>& out ) const
{
	out << clsid;
	serialize(out);
}

boost::intrusive_ptr<ICoordConverter> PYXAxisFlipCoordConverter::clone() const
{
	return new PYXAxisFlipCoordConverter(m_spCoordConverter);
}

void PYXAxisFlipCoordConverter::nativeToPYXIS( const PYXCoord2DDouble& native, PYXIcosIndex* pIndex, int nResolution ) const
{
	PYXCoord2DDouble flipedNative(native.y(),native.x());
	m_spCoordConverter->nativeToPYXIS(flipedNative,pIndex,nResolution);
}

void PYXAxisFlipCoordConverter::pyxisToNative( const PYXIcosIndex& index, PYXCoord2DDouble* pNative ) const
{
	if (!tryPyxisToNative(index,pNative))
	{
		PYXTHROW(PYXException,"Failed to convert pyxis index into valid native coordinate");
	}
}

bool PYXAxisFlipCoordConverter::tryPyxisToNative( const PYXIcosIndex& index, PYXCoord2DDouble* pNative ) const
{
	PYXCoord2DDouble flipedNative;
	if (m_spCoordConverter->tryPyxisToNative(index,&flipedNative))
	{
		pNative->setX(flipedNative.y());
		pNative->setY(flipedNative.x());
		return true;
	}
	return false;
}

void PYXAxisFlipCoordConverter::nativeToLatLon( const PYXCoord2DDouble& native, CoordLatLon * pLatLon ) const
{
	PYXCoord2DDouble flipedNative(native.y(),native.x());
	m_spCoordConverter->nativeToLatLon(flipedNative,pLatLon);
}

void PYXAxisFlipCoordConverter::latLonToNative( const CoordLatLon & latLon, PYXCoord2DDouble * pNative ) const
{
	PYXCoord2DDouble flippedNative;
	m_spCoordConverter->latLonToNative(latLon, &flippedNative);
	pNative->setX(flippedNative.y());
	pNative->setY(flippedNative.x());
}
