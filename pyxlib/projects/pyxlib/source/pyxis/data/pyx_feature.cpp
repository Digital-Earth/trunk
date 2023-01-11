#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyx_feature.h"


// {EB2AB67B-0EAA-423c-AEAA-8268D25394FB}
PYXCOM_DEFINE_CLSID(PYXFeature, 
0xeb2ab67b, 0xeaa, 0x423c, 0xae, 0xaa, 0x82, 0x68, 0xd2, 0x53, 0x94, 0xfb);

PYXCOM_CLASS_INTERFACES(PYXFeature, IWritableFeature::iid, IFeature::iid, IRecord::iid, PYXCOM_IUnknown::iid);

//! Default Constructor
PYXFeature::PYXFeature()
{
}

/*!
Constructor to make a PYXFeature with.

\param spGeom	The geometry of this feature.
\param strId	The ID of this feature.
\param strStyle		The Style to be used to display this feature.
\param isWritable	Boolean indicating if the values of this feature can be changed.
\param spTableDef	The meta data definition of this feature.
\param strGeomName	The name associated with the geometry of this feature.
*/
PYXFeature::PYXFeature(
	PYXPointer<PYXGeometry> spGeom, std::string strId, std::string strStyle, bool isWritable, 
	PYXPointer<PYXTableDefinition> spTableDef, std::string strGeomName)
:	m_spGeom(spGeom),
	m_strID(strId),
	m_strStyle(strStyle),
	m_bWritable(isWritable),
	m_spDefn(spTableDef),
	m_strGeomName(strGeomName)
{
}

/*!
Accessor method to set the id that represents this feature.

\param strId The new id to be used to represent this feature.
*/
void PYXFeature::setID(const std::string & strId)
{
	m_strID = strId;
}

/*!
Accessor method to set the geometry of this feature.

\param spGeom	The new geometry to of this particular feature.
*/
void PYXFeature::setGeometry(const PYXPointer<PYXGeometry> & spGeom)
{
	m_spGeom = spGeom;
}

/*!
Sets whether values in this feature can be written or not.

\param bWritable The value indicating whether this feature can be written to.
*/
void PYXFeature::setIsWritAble(bool bWritable)
{
	m_bWritable = bWritable;
}

/*!
Sets the style that is used to represent the feature.

\param style The style to represent this feature with.
*/
void PYXFeature::setStyle(const std::string & style)
{
	m_strStyle = style;
}

/*!
Sets the name of the geometry to use in the geometry of the name.

\param strName  The name of to associate with the geometry.
*/
void PYXFeature::setGeometryName(const std::string & strName)
{
	m_strGeomName = strName;
}

/*!
Sets the meta data definition for this process. 

\param spDef	The meta data definition to set for this particular process.
*/
void PYXFeature::setMetaDataDefinition(const PYXPointer<PYXTableDefinition> & spDef)
{
	m_spDefn = spDef;
}