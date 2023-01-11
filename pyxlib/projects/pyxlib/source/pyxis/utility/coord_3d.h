#ifndef PYXIS__UTILITY__COORD_3D_H
#define PYXIS__UTILITY__COORD_3D_H
/******************************************************************************
coord_3d.h

begin		: 2004-12-10
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/math_utils.h"

// standard includes
#include <cmath>
#include <iosfwd>

//! Represents a cartesian coordinate in 3D space.
template<class T> class PYXCoord3D
{
public:

	/*!
	Constructor initializes member variables.

	\param	x	The x value (default = 0).
	\param	y	The y value (default = 0).
	\param	z	The z value (default = 0).
	*/
	//! Initializing constructor.
	PYXCoord3D(T x = 0, T y = 0, T z = 0)
	{
		m_pCoord[0] = x;
		m_pCoord[1] = y;
		m_pCoord[2] = z;
	}

	/*!
	Copy constructor.

	\param	pt	The point to copy.
	*/
	//! Copy constructor.
	PYXCoord3D(const PYXCoord3D<T>& pt)
	{
		m_pCoord[0] = pt.m_pCoord[0];
		m_pCoord[1] = pt.m_pCoord[1];
		m_pCoord[2] = pt.m_pCoord[2];
	}

	/*!
	Copy assignment.

	\param	pt	The point to copy.
	*/
	//! Copy Assignment.
	PYXCoord3D<T> operator=(const PYXCoord3D<T>& pt)
	{
		if (this != &pt)
		{
			m_pCoord[0] = pt.m_pCoord[0];
			m_pCoord[1] = pt.m_pCoord[1];
			m_pCoord[2] = pt.m_pCoord[2];
		}

		return *this;
	}

	/*! 
	Set all of the values in the in the coordinate

	\param x	The value to be assigned to x.
	\param y	The value to be assigned to y.
	\param z	The value to be assigned to z.
	*/
	//! Set all of the values.
	void set(T x, T y, T z)
	{
		m_pCoord[0] = x;
		m_pCoord[1] = y;
		m_pCoord[2] = z;
	}

	/*!
	Equality operator.

	\param	pt	The point to compare with this one.

	\return	true if this coordinate is equal to the one passed in, otherwise false.
	*/
	//! Equality operator.
	bool operator==(const PYXCoord3D<T>& pt) const
	{
		return (	(m_pCoord[0] == pt.m_pCoord[0]) &&
					(m_pCoord[1] == pt.m_pCoord[1]) &&
					(m_pCoord[2] ==	pt.m_pCoord[2])	);
	}

	/*!
	Check if two points are equal within a the default precision for T. The
	default implementation simply calls the equality operator. A specialization
	exists for PYXCoord3D<double>.

	\param	pt			The point to compare with this one.

	\return	true if the points are equal, otherwise false.
	*/
	//! Check the equality of two values within a given precision.
	bool equal(const PYXCoord3D<T>& pt) const
	{
		return (*this == pt);
	}

	/*!
	Check if two points are equal within a given precision. The default
	implementation simply calls the equality operator. A specialization
	exists for PYXCoord3D<double>.

	\param	pt			The point to compare with this one.
	\param	precision	The precision with which to compare.

	\return	true if the points are equal, otherwise false.
	*/
	//! Check the equality of two values within a given precision.
	bool equal(const PYXCoord3D<T>& pt, T precision) const
	{
		return (*this == pt);
	}

	/*!
	Less than operator provided to allow ordering in containers. Sorting is done
	first by x coordinate, then by the y coordinate.

	\param	pt	The point to compare with this one.

	\return	true if this coordinate is less than the one passed in otherwise false.
	*/
	//! Less than operator.
	bool operator<(const PYXCoord3D<T>& pt) const
	{
		bool bLess = (m_pCoord[0] < pt.m_pCoord[0]);

		if (m_pCoord[0] == pt.m_pCoord[0])
		{
			bLess = (m_pCoord[1] < pt.m_pCoord[1]);

			if (m_pCoord[1] == pt.m_pCoord[1])
			{
				bLess = (m_pCoord[2] < pt.m_pCoord[2]);
			}
		}

		return bLess;
	}

	/*!
	Return an element. For efficiency reasons the index is not range checked.

	\param	nIndex	The index.

	\return	The element.
	*/
	// Element access operator.
	const T & operator[](const int nIndex) const {return m_pCoord[nIndex];}
	T & operator[](const int nIndex) {return m_pCoord[nIndex];}
	
	/*!
	Convenience method for passing coordinates, should not be used
	directly.

	\return	Pointer to the element array.
	*/
	//! Direct data access method.
	operator T*() {return m_pCoord;}

	/*!
	Get the x value.

	\return	The x value.
	*/
	//! Return the x value.
	inline T x() const {return m_pCoord[0];}

	/*!
	Set the x value.

	\param	x	The x value.
	*/
	//! Set the y value.
	inline void setX(T x) {m_pCoord[0] = x;}

	/*!
	Get the y value.

	\return	The y value.
	*/
	//! Return the y value.
	inline T y() const {return m_pCoord[1];}

	/*!
	Set the y value.

	\param	y	The y value.
	*/
	//! Set the y value.
	inline void setY(T y) {m_pCoord[1] = y;}

	/*!
	Get the z value.

	\return	The z value.
	*/
	//! Return the z value.
	inline T z() const {return m_pCoord[2];}

	/*!
	Set the z value.

	\param	z	The z value.
	*/
	//! Set the z value.
	inline void setZ(T z) {m_pCoord[2] = z;}

	/*!
	Calculate the distance between two points.

	\param	pt	The second point.
	
	\return	The distance.
	*/
	//! Calculate the distance between two points.
	double distance(const PYXCoord3D<T>& pt) const
	{
		T dX = m_pCoord[0] - pt.m_pCoord[0];
		T dY = m_pCoord[1] - pt.m_pCoord[1];
		T dZ = m_pCoord[2] - pt.m_pCoord[2];

		return sqrt(	static_cast<double>(dX * dX) +
						static_cast<double>(dY * dY) +
						static_cast<double>(dZ * dZ)	);
	}

	/*!
	Scale the point by the given value.

	\param	value	The value by which to scale.
	*/
	//! Scale the value by a given value.
	void scale(T value)
	{
		m_pCoord[0] *= value;
		m_pCoord[1] *= value;
		m_pCoord[2] *= value;
	}

	/*!
	return the length of the vector
	*/
	double length() const
	{
		return sqrt(	static_cast<double>(m_pCoord[0] * m_pCoord[0]) +
						static_cast<double>(m_pCoord[1] * m_pCoord[1]) +
						static_cast<double>(m_pCoord[2] * m_pCoord[2])	);
	}

	double squareLength() const
	{
		return static_cast<double>(m_pCoord[0] * m_pCoord[0]) +
			   static_cast<double>(m_pCoord[1] * m_pCoord[1]) +
			   static_cast<double>(m_pCoord[2] * m_pCoord[2]);		
	}

	/*!
	Treat the point as a vector from the origin and normalize so the vector has
	a length of 1 unit. This method only makes sense for coordinates based on
	float and double values.
	*/
	//! Normalize the vector to unit length.
	void normalize()
	{
		double fDivisor = length();

		// avoid divide by zero
		if (0.0 != fDivisor)
		{
			m_pCoord[0] /= fDivisor;
			m_pCoord[1] /= fDivisor;
			m_pCoord[2] /= fDivisor;
		}
	}

	/*!
	Invert each of the three components of the coordinate.
	This operation places the coordinate on the opposite side of the origin for each
	of the three axis.
	*/
	//! Negate the coordinate moving it to the opposite side of the origin.
	void negate()
	{
		m_pCoord[0] = -m_pCoord[0];
		m_pCoord[1] = -m_pCoord[1];
		m_pCoord[2] = -m_pCoord[2];
	}

	/*!
	Calculate the dot product of this vector and a specified vector.

	\param	v	The vector

	\return	this dot v
	*/
	//! Determine the dot product.
	double dot(const PYXCoord3D<T>& v) const
	{
		return (	(m_pCoord[0] * v.m_pCoord[0]) + 
					(m_pCoord[1] * v.m_pCoord[1]) + 
					(m_pCoord[2] * v.m_pCoord[2])	);
	}

	/*!
	Translate the point by an amount defined by another coordinate

	\param value	The value to translate by.
	*/
	//! Translate a point.
	void translate(const PYXCoord3D<T>& value)
	{
		m_pCoord[0] += value.m_pCoord[0];
		m_pCoord[1] += value.m_pCoord[1];
		m_pCoord[2] += value.m_pCoord[2];
	}

	/*!
	Find the difference between two coordinates.

	\param value	The value to subtact from the object.
	*/
	//! Subtract one coordinate from another.
	void subtract(const PYXCoord3D<T>& value)
	{
		m_pCoord[0] -= value.m_pCoord[0];
		m_pCoord[1] -= value.m_pCoord[1];
		m_pCoord[2] -= value.m_pCoord[2];
	}

	/*!
	Calculate the cross product of this vector and a specified vector.

	\param	v	The vector

	\return	this x v
	*/
	//! Calculate the cross product.
	PYXCoord3D cross(const PYXCoord3D<T>& v) const
	{
		PYXCoord3D result;

		result.setX((y() * v.z()) - (z() * v.y()));
		result.setY((z() * v.x()) - (x() * v.z()));
		result.setZ((x() * v.y()) - (y() * v.x()));

		return result;
	}

	/*!
	Determine the point at the given parametric distance along the line defined by
	two points. This method only makes sense for coordinates based on float and
	double values.

	\param	pt1			The first point.
	\param	pt2			The second point.
	\param	fDistance	The parametric distance in the range [0, 1].

	\return	The point at the given parametric distance.
	*/
	//! Determine the point along a line.
	PYXCoord3D<T> pointAlongLine(	const PYXCoord3D<T>& pt1,
									const PYXCoord3D<T>& pt2,
									double fDistance	)
	{
		// get deltas
		double fDeltaX = pt2.x() - pt1.x();
		double fDeltaY = pt2.y() - pt1.y();
		double fDeltaZ = pt2.z() - pt1.z();

		return PYXCoord3D(	pt1.x() + fDeltaX * fDistance,
							pt1.y() + fDeltaY * fDistance,
							pt1.z() + fDeltaZ * fDistance	);
	}

	//! Reset all of the values to 0.
	void reset()
	{
		m_pCoord[0] = 0;
		m_pCoord[1] = 0;
		m_pCoord[2] = 0;
	}

protected:

private:

	//! The coordinates
	T m_pCoord[3];
};

//! Member function template specialization for int does nothing
template <>
inline
void PYXCoord3D<int>::normalize()
{
	PYXTHROW(PYXException, "No meaningful implementation for int");
}

//! Member function template specialization for int does nothing
template <>
inline
PYXCoord3D<int> PYXCoord3D<int>::pointAlongLine(	const PYXCoord3D<int>& pt1,
													const PYXCoord3D<int>& pt2,
													double fDistance	)
{
	PYXTHROW(PYXException, "No meaningful implementation for int");
}

//! Typedef for integer 3D point.
typedef PYXCoord3D<int> PYXCoord3DInt;

//! Typedef for double 3D point.
typedef PYXCoord3D<double> PYXCoord3DDouble;

#ifndef DOXYGEN_IGNORE	// doxygen cannot handle template method specialization

//! Specialization for equal method.
template<> 
bool PYXCoord3DDouble::equal(const PYXCoord3DDouble& pt, double fPrecision) const
{
	return (	MathUtils::equal(m_pCoord[0], pt.m_pCoord[0], fPrecision) &&
				MathUtils::equal(m_pCoord[1], pt.m_pCoord[1], fPrecision) &&
				MathUtils::equal(m_pCoord[2], pt.m_pCoord[2], fPrecision)	);
}

//! Specialization for equal method.
template<> 
bool PYXCoord3DDouble::equal(const PYXCoord3DDouble& pt) const
{
	return equal(pt, MathUtils::kfDefaultDoublePrecision);
}

//! Allows templated 3D values to be written to streams.
template <class T>
std::ostream& operator <<(std::ostream& out, const PYXCoord3D<T>& point)
{
	out << point.x() << " " 
		<< point.y() << " "
		<< point.z();

	return out;
}

//! Allows templated 3D values to be read from streams.
template <class T>
std::istream& operator >>(std::istream& input, PYXCoord3D<T>& point)
{
	T x;
	T y;
	T z;

	input >> x >> y >> z;
	point.set(x, y, z);
	return input;
}

#endif // DOXYGEN_IGNORE

#endif // guard
