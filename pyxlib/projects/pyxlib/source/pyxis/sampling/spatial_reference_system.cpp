/******************************************************************************
spatial_reference_system.cpp

begin		: 2006-03-13
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "spatial_reference_system.h"

//! The UTM System has 60 Zones
const int PYXSpatialReferenceSystem::knMaxZone = 60;

// the scope
const std::string PYXSpatialReferenceSystem::kstrScope = "PYXSpatialReferenceSystem";

/*!
Constructor
*/
PYXSpatialReferenceSystem::PYXSpatialReferenceSystem()
{
	// initialize member variables
	setSystem(knSystemGeographical);
	setDatum(knDatumWGS84);
	setProjection(knProjectionUTM);
	setIsUTMNorth(true);
	setZone(19);
}

/*!
Destructor
*/
PYXSpatialReferenceSystem::~PYXSpatialReferenceSystem()
{
}

/*!
Set the system.

\param	nSystem	The system.
*/
void PYXSpatialReferenceSystem::setSystem(PYXSpatialReferenceSystem::eSystem nSystem)
{
	m_nSystem = nSystem;
}

/*!
Get the system.

\return	The system.
*/
PYXSpatialReferenceSystem::eSystem PYXSpatialReferenceSystem::getSystem() const
{
	return m_nSystem;
}

/*!
Set the datum.

\param	nDatum	The datum.
*/
void PYXSpatialReferenceSystem::setDatum(PYXSpatialReferenceSystem::eDatum nDatum)
{
	m_nDatum = nDatum;
}

/*!
Get the datum.

\return	The datum.
*/
PYXSpatialReferenceSystem::eDatum PYXSpatialReferenceSystem::getDatum() const
{
	return m_nDatum;
}

/*!
Set the projection.

\param	nProjection	The projection.
*/
void PYXSpatialReferenceSystem::setProjection(PYXSpatialReferenceSystem::eProjection nProjection)
{
	m_nProjection = nProjection;
}

/*!
Get the projection.

\return	The projection.
*/
PYXSpatialReferenceSystem::eProjection PYXSpatialReferenceSystem::getProjection() const
{
	return m_nProjection;
}

/*!
Set the hemisphere for the UTM system.

\param	bIsUTMNorth	true if the system is in the northern hemisphere, otherwise
					false.
*/
void PYXSpatialReferenceSystem::setIsUTMNorth(bool bIsUTMNorth)
{
	m_bIsUTMNorth = bIsUTMNorth;
}

/*!
Is the UTM system in the northern hemisphere.

\return	true if in the northern hemisphere, otherwise false.
*/
bool PYXSpatialReferenceSystem::getIsUTMNorth() const
{
	return m_bIsUTMNorth;
}

/*!
Set the zone.

\param	nZone	The zone.
*/
void PYXSpatialReferenceSystem::setZone(int nZone)
{
	m_nZone = nZone;
}

/*!
Get the zone.

\return	The zone.
*/
int PYXSpatialReferenceSystem::getZone() const
{
	return m_nZone;
}

/*!
Set the Well Known Text for the spatial reference system.

\param	strWKT	The well known text.
*/
void PYXSpatialReferenceSystem::setWKT(const std::string& strWKT)
{
	m_strWKT = strWKT;
}

/*!
Get the Well Known Text for the spatial reference system.

\return	The well known text.
*/
const std::string& PYXSpatialReferenceSystem::getWKT() const
{
	return m_strWKT;
}

/*!
Creates a spatial reference system identical to one passed in.

\returns A smart pointer to the new spatial reference system.
*/
PYXPointer<PYXSpatialReferenceSystem> PYXSpatialReferenceSystem::clone() const 
{	
	// create a new location
	PYXPointer<PYXSpatialReferenceSystem> spSRS(PYXSpatialReferenceSystem::create());

	// set up the location with the values here.
	spSRS->copy(*this);

	return spSRS;
}

/*!
Copies a spatial reference system.

\param sRS	The system to copy from.
*/
void PYXSpatialReferenceSystem::copy(const PYXSpatialReferenceSystem& sRS) 
{	
	this->setDatum(sRS.getDatum());
	this->setIsUTMNorth(sRS.getIsUTMNorth());
	this->setProjection(sRS.getProjection());
	this->setSystem(sRS.getSystem());
	this->setWKT(sRS.getWKT());
	this->setZone(sRS.getZone());
}
