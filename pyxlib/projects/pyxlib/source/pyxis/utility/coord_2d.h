#ifndef PYXIS__UTILITY__COORD_2D_H
#define PYXIS__UTILITY__COORD_2D_H
/******************************************************************************
coord_2d.h

begin		: 2004-02-17
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/math_utils.h"

// standard includes
#include <iosfwd>

//! Represents a cartesian coordinate in 2D space.
template<class T> class PYXCoord2D
{
public:

	/*!
	Constructor initializes member variables.

	\param	x	The x value (default = 0).
	\param	y	The y value (default = 0).
	*/
	//! Default constructor.
	PYXCoord2D(T x = 0, T y = 0)
	{
		m_pCoord[0] = x;
		m_pCoord[1] = y;
	}

	/*!
	Copy constructor.

	\param	pt	The point to copy.
	*/
	//! Copy constructor.
	PYXCoord2D(const PYXCoord2D<T>& pt)
	{
		m_pCoord[0] = pt.m_pCoord[0];
		m_pCoord[1] = pt.m_pCoord[1];
	}

	/*!
	Copy assignment.

	\param	pt	The point to copy.
	*/
	//! Copy assignment.
	PYXCoord2D<T> operator=(const PYXCoord2D<T>& pt)
	{
		if (this != &pt)
		{
			m_pCoord[0] = pt.m_pCoord[0];
			m_pCoord[1] = pt.m_pCoord[1];
		}

		return *this;
	}

	/*!
	Equality operator.

	\param	pt	The point to compare with this one.

	\return	true if this coordinate is equal to the one passed in, otherwise false.
	*/
	//! Equality operator.
	bool operator==(const PYXCoord2D<T>& pt) const
	{
		return (	(m_pCoord[0] == pt.m_pCoord[0]) &&
					(m_pCoord[1] == pt.m_pCoord[1])	);
	}

	/*!
	Check if two points are equal within a the default precision for T. The
	default implementation simply calls the equality operator. A specialization
	exists for PYXCoord2D<double>.

	\param	pt			The point to compare with this one.

	\return	true if the points are equal, otherwise false.
	*/
	//! Check the equality of two values within a given precision.
	bool equal(const PYXCoord2D<T>& pt) const
	{
		return (*this == pt);
	}

	/*!
	Check if two points are equal within a given precision. The default
	implementation simply calls the equality operator. A specialization
	exists for PYXCoord2D<double>.

	\param	pt			The point to compare with this one.
	\param	precision	The precision with which to compare.

	\return	true if the points are equal, otherwise false.
	*/
	//! Determine if two points are equal within a given precision.
	bool equal(const PYXCoord2D<T>& pt, T precision) const
	{
		return (*this == pt);
	}

	/*!
	Less than operator provided to allow ordering in containers. Sorting is done
	first by x coordinate, then by the y coordinate.

	\param	pt	The point to compare with this one.

	\return	true if this coordinate is less than the one passed in otherwise false.
	*/
	//! Less than operator merely for sorting in container classes.
	bool operator<(const PYXCoord2D<T>& pt) const
	{
		bool bLess = (m_pCoord[0] < pt.m_pCoord[0]);

		if (m_pCoord[0] == pt.m_pCoord[0])
		{
			bLess = (m_pCoord[1] < pt.m_pCoord[1]);
		}

		return bLess;
	}

	/*!
	Return an element. For efficiency reasons the index is not range checked.

	\param	nIndex	The index.

	\return	The element.
	*/
	//! Access a specific value.
	const T & operator[](const int nIndex) const {return m_pCoord[nIndex];}
	T & operator[](const int nIndex) {return m_pCoord[nIndex];}

	/*!
	Convenience method for passing coordinates, should not be used
	directly.

	\return	Pointer to the element array.
	*/
	//! Method for passing pointers to external libraries.
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
	//! Set the x value.
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
	//! Return the y value.
	inline void setY(T y) {m_pCoord[1] = y;}

	/*!
	Calculate the distance between two points.

	\param	pt	The second point.
	
	\return	The distance.
	*/
	//! Calculate the distance between two points.
	double distance(const PYXCoord2D<T>& pt) const
	{
		T dX = m_pCoord[0] - pt.m_pCoord[0];
		T dY = m_pCoord[1] - pt.m_pCoord[1];

		return sqrt(	static_cast<double>(dX * dX) + 
						static_cast<double>(dY * dY)	);
	}

	/*!
	Calculate the location of a point half way between two others.

	\param pt	The second point.

	\return The midpoint.
	*/
	//! Determine the midpoint.
	PYXCoord2D<T> midpoint(const PYXCoord2D<T>& pt) const
	{
		PYXCoord2D<T> midpoint;
		midpoint.m_pCoord[0] = static_cast<T>(	(m_pCoord[0] + pt.m_pCoord[0])
												/ 2.0	);
		midpoint.m_pCoord[1] = static_cast<T>(	(m_pCoord[1] + pt.m_pCoord[1])
												/ 2.0	);
		return midpoint;
	}

	/*!
	Scale the point by the given value.

	\param	value	The value by which to scale.
	*/
	//! Treat as a vector from the origin and alter the scale by a given value.
	void scale(T value)
	{
		m_pCoord[0] *= value;
		m_pCoord[1] *= value;
	}

	/*!
	Treat the point as a vector from the origin and normalize so the vector has
	a length of 1 unit. This method only makes sense for coordinates based on
	float and double values.
	*/
	//! Treat as a vector from the origin and normalize to unit length.
	void normalize()
	{
		double fDivisor = sqrt(	static_cast<double>(m_pCoord[0] * m_pCoord[0]) +
								static_cast<double>(m_pCoord[1] * m_pCoord[1])	);

		// avoid divide by zero
		if (0.0 != fDivisor)
		{
			m_pCoord[0] /= fDivisor;
			m_pCoord[1] /= fDivisor;
		}
	}

	/*!
	Calculate the dot product of this vector and a specified vector.

	\param	v	The vector

	\return	this dot v
	*/
	//! Determine the dot product.
	double dot(const PYXCoord2D<T>& v) const
	{
		return (	(m_pCoord[0] * v.m_pCoord[0]) + 
					(m_pCoord[1] * v.m_pCoord[1])	);
	}

protected:

	//! The coordinates
	T m_pCoord[2];
};

//! Member function template specialization for int does nothing
template <>
void PYXCoord2D<int>::normalize()
{
	PYXTHROW(PYXException, "No meaningful implementation for int");
}

//! Typedef for integer 2D point.
typedef PYXCoord2D<int> PYXCoord2DInt;

//! Typedef for double 2D point.
typedef PYXCoord2D<double> PYXCoord2DDouble;

#ifndef DOXYGEN_IGNORE	// doxygen cannot handle template method specialization

//! Specialization for equal method.
template<>
bool PYXCoord2D<double>::equal(	const PYXCoord2D<double>& pt, double fPrecision) const
{
	return (	MathUtils::equal(m_pCoord[0], pt.m_pCoord[0], fPrecision) &&
				MathUtils::equal(m_pCoord[1], pt.m_pCoord[1], fPrecision)	);
}

//! Specialization for equal method.
template<>
bool PYXCoord2D<double>::equal(const PYXCoord2D<double>& pt) const
{
	return equal(pt, MathUtils::kfDefaultDoublePrecision);
}


//! Allows templated 2D values to be written to streams.
template <class T>
std::ostream& operator <<(std::ostream& out, const PYXCoord2D<T>& point)
{
	out << point.x() << " " << point.y();
	return out;
}

//! Allows templated 2D values to be read from streams.
template <class T>
std::istream& operator >>(std::istream& input, PYXCoord2D<T>& point)
{
	T x;
	T y;
	input >> x >> y;
	point.setX(x);
	point.setY(y);
	return input;
}

#endif // DOXYGEN_IGNORE

#endif // guard
