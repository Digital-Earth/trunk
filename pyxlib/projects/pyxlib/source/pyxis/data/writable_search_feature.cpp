/******************************************************************************
writable_search_feature.cpp

begin		: June 20, 2008
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "writable_search_feature.h"


// {166013DE-0C68-4f06-A974-F3D71B8418D5}
PYXCOM_DEFINE_CLSID(WritableSearchFeature, 
0x166013de, 0xc68, 0x4f06, 0xa9, 0x74, 0xf3, 0xd7, 0x1b, 0x84, 0x18, 0xd5);


PYXCOM_CLASS_INTERFACES(
						WritableSearchFeature, 
						IWritableFeature::iid, 
						IFeature::iid, 
						IRecord::iid, 
						PYXCOM_IUnknown::iid );


WritableSearchFeature::WritableSearchFeature(
	PYXPointer<PYXGeometry> spGeom, 
	std::string strId, std::string strStyle, bool isWritable, 
	PYXPointer<PYXTableDefinition> spTableDef,
	std::string strGeomName)
:	PYXFeature(spGeom, strId, strStyle, isWritable, spTableDef, strGeomName)
{
}