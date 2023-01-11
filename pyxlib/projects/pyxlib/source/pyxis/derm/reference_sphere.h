#ifndef PYXIS__DERM__REFERENCE_SPHERE_H
#define PYXIS__DERM__REFERENCE_SPHERE_H
/******************************************************************************
reference_sphere.h

begin		: 2004-04-01
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/derm/horizontal_datum.h"

/*!
ReferenceSphere provides calculations based on the GRS 1980 Authalic Sphere.
*/
//! Represents an earth reference sphere.
class PYXLIB_DECL ReferenceSphere : public HorizontalDatum
{
public:

	// Static constant values.
	static const double kfRadius;

	//! Get the instance of the datum.
	static ReferenceSphere const * getInstance();

	/*!
	Get the name of this datum.

	\return	The name of this datum.
	*/
	virtual std::string getName() const {return "Reference Sphere";}

	//! Calculate the distance in metres between two points.
	virtual double calcDistance(const CoordLatLon& pt1, const CoordLatLon& pt2) const;

	/*!
	Convert a point in datum lat/lon coordinates to geocentric lat/lon
	coordinates.

	\param	pt	The point in datum lat/lon coordinates.

	\return	The point in geocentric lat/lon coordinates.
	*/
	virtual CoordLatLon toGeocentric(const CoordLatLon& pt) const {return pt;}

	/*!
	Convert a point in geocentric lat/lon coordinates to datum lat/lon
	coordinates.

	\param	pt	The point in geocentric lat/lon coordinates.

	\return	The point in datum lat/lon coordinates.
	*/
	virtual CoordLatLon toDatum(const CoordLatLon& pt) const {return pt;}

private:

	//! Hide Constructor
	ReferenceSphere() {}

	//! Disable copy constructor
	ReferenceSphere(const ReferenceSphere&);

	//! Initialize any static data.
	static void initStaticData();

	//! Free singleton instance
	static void freeStaticData();

	//! Disable copy assignment
	void operator=(const ReferenceSphere&);

	//! Singleton instance
	static ReferenceSphere* m_pInstance;

	//! Allows PYXLibInstance to initialize the static data.
	friend class PYXLibInstance;
};

#endif // guard
