#ifndef PYXIS__SAMPLING__SPATIAL_REFERENCE_SYSTEM_H
#define PYXIS__SAMPLING__SPATIAL_REFERENCE_SYSTEM_H
/******************************************************************************
spatial_reference_system.h

begin		: 2006-03-13
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/object.h"

// standard includes
#include <string>

/*!
Holds spatial reference information for a data source.
*/
class PYXLIB_DECL PYXSpatialReferenceSystem : public PYXObject
{
public:

	//! System enum
	enum eSystem {knSystemProjected = 0, knSystemGeographical, knSystemNone};

	//! Datum enum
	enum eDatum {knDatumNAD27 = 0, knDatumNAD83, knDatumWGS72, knDatumWGS84, knDatumNone};

	//! Projection enum
	enum eProjection {knProjectionUTM = 0, knProjectionTM, knProjectionLCC, knProjectionNone, knCustomProjection};

	//! Clone a new PYXSpatialReferenceSystem.
	virtual PYXPointer<PYXSpatialReferenceSystem> clone() const;

	//! The number of zones available for UTM.
	static const int knMaxZone;

	// Create an instance of this class.
	static PYXPointer<PYXSpatialReferenceSystem> create()
	{
		return PYXNEW(PYXSpatialReferenceSystem);
	}

	// Constructor
	PYXSpatialReferenceSystem();

	// Destructor
	virtual ~PYXSpatialReferenceSystem();

	// Get scope (class name) of this class.
	static const std::string kstrScope;
	virtual std::string getScope() const {return kstrScope;}

	// Set System
	void setSystem(eSystem nSystem);

	// Get System
	eSystem getSystem() const;

	// Set Datum
	void setDatum(eDatum nDatum);

	// Get Datum
	eDatum getDatum() const;

	// Set Projection
	void setProjection(eProjection nProjection);

	// Get Projection
	eProjection getProjection() const;

	// Set IsUTMNorth
	void setIsUTMNorth(bool bIsUTMNorth);

	// Get IsUTMNorth
	bool getIsUTMNorth() const;

	// Set Zone
	void setZone(int nZone);

	// Get Zone
	int getZone() const;

	// Set WKT
	void setWKT(const std::string& strWKT);

	// Get WKT
	const std::string& getWKT() const;

private:

	//! Copies a spatial reference system.
	void copy(const PYXSpatialReferenceSystem& sRS); 

private:

	//! Type of the data source.
	eSystem m_nSystem;

	//! Type of the data source.
	eDatum m_nDatum;

	//! Type of the data source.
	eProjection m_nProjection;

	//! true if the utm system is in the northern hemisphere.
	bool m_bIsUTMNorth;

	//! true when item has never been opened.
	int m_nZone;

	//! The well known text format for the system
	std::string m_strWKT;
};

#endif // guard
