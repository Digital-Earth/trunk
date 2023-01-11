#ifndef CIRCLE_GEOMETRY_H
#define CIRCLE_GEOMETRY_H
/******************************************************************************
circle_geometry.h

begin		: 2007-11-19
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/
#include "pyxlib.h"

//local includes.
#include "geometry_collection.h"
#include "pyxis/derm/index.h"
#include "tile_collection.h"

// boost includes.
#include <boost/thread/recursive_mutex.hpp>

/*!
Calculates a circular geometry from a given centre point and radius.
The geometry is comprised of all of the cells that intersect a circle 
which can be constructed from a given centre index and a radius at a 
particular cell resolution.
*/
//! Conatins all the cells that intersect a circle given centre point and a radius.
class PYXLIB_DECL PYXCircleGeometry : public PYXGeometryCollection
{
public:

	//! Unit test method
	static void test();
	
	//! Dynamic Creator
	static PYXPointer<PYXCircleGeometry> create(const PYXIcosIndex& centreIndex, double fRadius)
	{
		return PYXNEW(PYXCircleGeometry, centreIndex, fRadius);
	}

	//! Deserialization creator
	static PYXPointer<PYXCircleGeometry> create(std::basic_istream<char>& in)
	{
		return PYXNEW(PYXCircleGeometry, in);
	}

	//! Destructor
	virtual ~PYXCircleGeometry() {}

	/*! 
	Determine if this geometry collection is empty.

	\return bool indicating whether the collection is empty or not.
	*/
	virtual bool isEmpty() const 
	{
		return false;
	}

	//! Set the radius of the circle in metres.
	virtual void setRadius(double fRadius);

	//! Get the radius of the circle.
	virtual double getRadius() const
	{
		return m_fRadius;
	}

	//! Get the centre point of the circle.
	virtual PYXIcosIndex getCentre() const
	{
		return m_centre;
	}

	//! Clear all the cells in this collection.
	virtual void clear()
	{
		m_spGeometry.reset();
	}

	/*!
	Get the number of geometries in this collection.

	\return	The number of geometries in this geometry.
	*/
	//! Return the number of geometries in the collection.
	virtual int getGeometryCount() const
	{
		boost::recursive_mutex::scoped_lock lock (m_mutex);
		if (!m_spGeometry)
		{
			calcGeometry();
		}
		return m_spGeometry->getGeometryCount();
	}
	
	//! Set the cell resolution of this geometry.
	virtual inline void setCellResolution(int nCellResolution);

	//! Get the cell resolution for this geometry.
	virtual int getCellResolution() const;

	//! Get an iterator to all the geometries in this collection.
	virtual PYXPointer<PYXGeometryIterator> getGeometryIterator() const;

	//! Calculate the perimeter of this geometry collection.
	virtual void calcPerimeter(std::vector<PYXIcosIndex>* pVecIndex) const;
	
	//! Clone this geometry.
	virtual PYXPointer<PYXGeometry> clone() const;
	
	//! Copy to a tile collection.
	virtual void copyTo(PYXTileCollection* pTileCollection) const;

	//! Copy to a tile collection at the specified resolution.
	virtual void copyTo(PYXTileCollection* pTileCollection, int nTargetResolution) const;

	//! Set the centre point of this circle geometry.
	virtual void setCentrePoint(const PYXIcosIndex& centreIndex);

	//! Serialize.
	virtual void serialize(std::basic_ostream< char>& out) const;

	//! Deserialize.
	virtual void deserialize(std::basic_istream< char>& in);

	//! Return a boolean indication of intersection.
	virtual bool intersects(const PYXGeometry& geometry, bool bCommutative = true) const;

	//! Determine the intersecting geometry.
	virtual PYXPointer<PYXGeometry> intersection(const PYXGeometry& geometry, bool bCommutative = true) const;

	virtual PYXBoundingCircle getBoundingCircle() const;

protected:

	//! Default Constructor
	PYXCircleGeometry(const PYXIcosIndex& centreIndex, double fRadius);

	PYXCircleGeometry(std::basic_istream< char>& in)
	{
		deserialize(in);
	}

private: 

	//! Calculate the circle geometry.
	void calcGeometry() const;

private:

	//! The index which is at the centre of the circle.
	PYXIcosIndex m_centre;
	
	//! The radius of the circle in metres.
	double m_fRadius;
	
	//! The geometry that was last calculated.
	mutable PYXPointer<PYXTileCollection> m_spGeometry;

	//! Mutex for thread safety.
	mutable boost::recursive_mutex m_mutex;

};

//! The equality operator.
inline
PYXLIB_DECL bool operator ==(	const PYXCircleGeometry& lhs,
								const PYXCircleGeometry& rhs	)
{
	return lhs.getRadius() == rhs.getRadius() && lhs.getCentre() == rhs.getCentre();
}

#endif