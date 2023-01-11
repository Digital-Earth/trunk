#ifndef PYXIS__UTILITY__RECT_2D_H
#define PYXIS__UTILITY__RECT_2D_H
/******************************************************************************
rect_2d.h

begin		: 2004-12-08
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/coord_2d.h"

//! Represents a rectangle in 2D space.
/*!
PYXRect2D represents a rectangle in 2D space. It avoids the use of terms like
top, bottom, left and right to avoid confusion between a coordinate systems
that have their positive y-axes pointing up and those that have them pointing
down. This class uses the more generic terms xMin, yMin, xMax and yMax instead.
*/
template<class T> class PYXRect2D
{
public:

	/*!
	Constructor initializes member variables.

	\param	xMin	The minimum x value (default = 1).
	\param	yMin	The minimum y value (default = 1).
	\param	xMax	The maximum x value (default = -1).
	\param	yMax	The maximum y value (default = -1).
	*/
	PYXRect2D(T xMin = 1, T yMin = 1, T xMax = -1, T yMax = -1)
	{
		m_xMin = xMin;
		m_yMin = yMin;
		m_xMax = xMax;
		m_yMax = yMax;
	}

	/*!
	Copy constructor.

	\param	r	The rectangle to copy.
	*/
	PYXRect2D(const PYXRect2D<T>& r)
	{
		m_xMin = r.m_xMin;
		m_yMin = r.m_yMin;
		m_xMax = r.m_xMax;
		m_yMax = r.m_yMax;
	}

	/*!
	Copy assignment.

	\param	r	The rectangle to copy.
	*/
	PYXRect2D<T> operator=(const PYXRect2D<T>& r)
	{
		if (this != &r)
		{
			m_xMin = r.m_xMin;
			m_yMin = r.m_yMin;
			m_xMax = r.m_xMax;
			m_yMax = r.m_yMax;
		}

		return *this;
	}

	/*!
	Equality operator.

	\param	r	The rectangle to compare with this one.

	\return	true if this rectangle is equal to the one passed in, otherwise false.
	*/
	bool operator==(const PYXRect2D<T>& r) const
	{
		return (	(m_xMin == r.m_xMin) &&
					(m_yMin == r.m_yMin) &&
					(m_xMax == r.m_xMax) &&
					(m_yMax == r.m_yMax)	);
	}

	/*!
	Check if two rectangles are equal within the default precision for T. The
	default implementation simply calls the equality operator. Specializations
	exist for PYXRect2D<float> and PYXRect2D<double>.

	\param	r			The rectangle to compare with this one.
	\param	precision	The precision with which to compare.

	\return	true if the points are equal, otherwise false.
	*/
	bool equal(const PYXRect2D<T>& r) const
	{
		return (*this == r);
	}

	/*!
	Check if two rectangles are equal within a given precision. The default
	implementation simply calls the equality operator. Specializations exist for
	PYXRect2D<float> and PYXRect2D<double>.

	\param	r			The rectangle to compare with this one.
	\param	precision	The precision with which to compare.

	\return	true if the points are equal, otherwise false.
	*/
	bool equal(const PYXRect2D<T>& r, T precision) const
	{
		return (*this == r);
	}

	bool contains(const PYXRect2D<T>& r) const
	{
		if (empty())
		{
			return r.empty();
		}

		PYXRect2D<T> test = r;
		test.clip(*this);

		return test == r;
	}

	bool touch(const PYXRect2D<T>& r) const
	{
		if (inside(PYXCoord2D<T>(r.xMin(),r.yMin())) ||
			inside(PYXCoord2D<T>(r.xMax(),r.yMin())) ||
			inside(PYXCoord2D<T>(r.xMin(),r.yMax())) ||
			inside(PYXCoord2D<T>(r.xMax(),r.yMax())) ||

			r.inside(PYXCoord2D<T>(xMin(),yMin())) ||
			r.inside(PYXCoord2D<T>(xMax(),yMin())) ||
			r.inside(PYXCoord2D<T>(xMin(),yMax())) ||
			r.inside(PYXCoord2D<T>(xMax(),yMax())))
		{
			return true;
		}

		return false;
	}

	/*!
	Get the minimum x value.

	\return	The minimum x value.
	*/
	inline T xMin() const {return m_xMin;}

	/*!
	Set the minimum x value.

	\param	xMin	The minimum x value.
	*/
	inline void setXMin(T xMin) {m_xMin = xMin;}

	/*!
	Get the minimum y value.

	\return	The minimum y value.
	*/
	inline T yMin() const {return m_yMin;}

	/*!
	Set the minimum y value.

	\param	yMin	The minimum y value.
	*/
	inline void setYMin(T yMin) {m_yMin = yMin;}

	/*!
	Get the maximum x value.

	\return	The maximum x value.
	*/
	inline T xMax() const {return m_xMax;}

	/*!
	Set the maximum x value.

	\param	xMax	The maximum x value.
	*/
	inline void setXMax(T xMax) {m_xMax = xMax;}

	/*!
	Get the maximum y value.

	\return	The maximum y value.
	*/
	inline T yMax() const {return m_yMax;}

	/*!
	Set the maximum y value.

	\param	yMax	The maximum y value.
	*/
	inline void setYMax(T yMax) {m_yMax = yMax;}

	/*!
	Get the width of the rectangle.

	\return	The width.
	*/
	T width() const {return (m_xMax - m_xMin);}

	/*!
	Get the height of the rectangle.

	\return	The height.
	*/
	T height() const {return (m_yMax - m_yMin);}

	/*!
	Is the rectangle degenerate? A rectangle is degenerate if its width or
	height is less than or equal to zero.

	\return true if the rectangle is degenerate, otherwise false.
	*/
	bool degenerate()
	{
		return ((width() <= 0) || (height() <= 0));
	}

	/*!
	Is the rectangle empty? A rectangle is empty if its width or height is less
	than zero.

	\return	true if the rectangle is empty, otherwise false.
	*/
	bool empty() const
	{
		return ((width() < 0) || (height() < 0));
	}

	/*!
	Set the rectangle to the empty rectangle.
	*/
	void setEmpty()
	{
		m_xMin = 1;
		m_yMin = 1;
		m_xMax = -1;
		m_yMax = -1;
	}

	/*!
	Does the specified point fall within or on the border of the rectangle.

	\param	pt	The point.

	\return	true if the point falls inside the rectangle, otherwise false.
	*/
	bool inside(const PYXCoord2D<T>& pt) const
	{
		return(	(pt.x() >= m_xMin) && (pt.x() <= m_xMax) &&
				(pt.y() >= m_yMin) && (pt.y() <= m_yMax)	);
	}

	/*!
	Expand the rectangle to include the specified x coordinate. If this rectangle
	is empty, there's no effect.

	\param	x	The x coordinate.
	*/
	void expandX(const T& x)
	{
		if (!empty())
		{
			m_xMin = std::min(m_xMin, x);
			m_xMax = std::max(m_xMax, x);
		}
	}

	/*!
	Expand the rectangle to include the specified y coordinate. If this rectangle
	is empty, there's no effect.

	\param	y	The y coordinate.
	*/
	void expandY(const T& y)
	{
		if (!empty())
		{
			m_yMin = std::min(m_yMin, y);
			m_yMax = std::max(m_yMax, y);
		}
	}

	/*!
	Expand the rectangle to include the specified point. If the rectangle is
	empty, it becomes the point.

	\param	pt	The point.
	*/
	void expand(const PYXCoord2D<T>& pt)
	{
		if (empty())
		{
			m_xMin = pt.x();
			m_yMin = pt.y();
			m_xMax = m_xMin;
			m_yMax = m_yMin;
		}
		else
		{
			m_xMin = std::min(m_xMin, pt.x());
			m_yMin = std::min(m_yMin, pt.y());
			m_xMax = std::max(m_xMax, pt.x());
			m_yMax = std::max(m_yMax, pt.y());
		}
	}

	/*!
	Expand the rectangle to include the specified rectangle. If the rectangle
	is empty, it becomes the specified rectangle.

	\param	r	The rectangle.
	*/
	void expand(const PYXRect2D<T>& r)
	{
		if (!r.empty())
		{
			if (empty())
			{
				*this = r;
			}
			else
			{
				m_xMin = std::min(m_xMin, r.m_xMin);
				m_yMin = std::min(m_yMin, r.m_yMin);
				m_xMax = std::max(m_xMax, r.m_xMax);
				m_yMax = std::max(m_yMax, r.m_yMax);
			}
		}
	}

	/*!
	Clips the rectangle to the other rectangle. If the other rectangle
	is empty, then this rectangle becomes empty.

	\param	r	The rectangle.
	*/
	void clip(const PYXRect2D<T>& r)
	{
		m_xMin = std::max(m_xMin, r.m_xMin);
		m_yMin = std::max(m_yMin, r.m_yMin);
		m_xMax = std::min(m_xMax, r.m_xMax);
		m_yMax = std::min(m_yMax, r.m_yMax);
	}

	/*!
	Returns the center of a rectangle
	*/
	PYXCoord2D<T> center() const
	{
		PYXCoord2D<T> ptOut;
		ptOut.setX ((xMax() + xMin()) / 2);
		ptOut.setY ((yMax() + yMin()) / 2);
		return ptOut;
	}

	/*!
	Scale the rectangle by the given value (from the origin, not the center).

	\param	value	The value by which to scale.
	*/
	void scale(T value)
	{
		m_xMin *= value;
		m_yMin *= value;
		m_xMax *= value;
		m_yMax *= value;
	}

	/*!
	Scale the rectangle by the given value (from the center).

	\param	value	The value by which to scale.
	*/
	void scaleInPlace(T value)
	{
		PYXCoord2D<T> centerPoint = center();
		centerPoint.scale(-1);
		translate(centerPoint);
		scale(value);
		centerPoint.scale(-1);
		translate(centerPoint);
	}

	/*!
	Scale the rectangle by the given value (from the center).

	\param	value	The value by which to scale.
	*/
	void flip()
	{
		T tmpXmin= xMin();
		T tmpXmax= xMax();
		//clip and flip it to match the min/max of bing
		
		setXMin(yMin());
		setYMin(tmpXmin);
		setXMax(yMax());
		setYMax(tmpXmax);
		
	}

	/*!
	Translates the rectangle by the given vector .

	\param	displacement	The vector by which to translate.
	*/
	void translate(PYXCoord2D<T> displacement)
	{
		m_xMin += displacement.x();
		m_yMin += displacement.y();
		m_xMax += displacement.x();
		m_yMax += displacement.y();
	}

	/*!
	Check if this rectangle's interior intersects with the specified rectangle's
	interior.

	\param	r	The rectangle to test.

	\return	true if the rectangles intersect, otherwise false.
	*/
	bool intersects(const PYXRect2D<T>& r) const
	{
		return (	(m_xMin < r.m_xMax) && (r.m_xMin < m_xMax) &&
					(m_yMin < r.m_yMax) && (r.m_yMin < m_yMax)	);
	}

	/*!
	Pin the point to the rectangle. If the x and/or y values lie outside
	the rectangle, they are pinned to the nearest border.

	\param	pt	The point

	\return	The pinned point.
	*/
	PYXCoord2D<T> pin(const PYXCoord2D<T>& pt) const
	{
		PYXCoord2D<T> ptOut;

		ptOut.setX(MathUtils::constrain(pt.x(), m_xMin, m_xMax));
		ptOut.setY(MathUtils::constrain(pt.y(), m_yMin, m_yMax));

		return ptOut;
	}

	/*!
	Make the aspect ratio of the rectangle match the specified aspect ratio by growing the width or
	height as appropriate and preserving the minimum x and y values. The rectangle is unchanged if
	it is empty.

	\param fAspect	The aspect ratio specified as width/height
	*/
	void fixAspect(double fAspect)
	{
		if (!empty())
		{
			if (width() / height() > fAspect)
			{
				T newHeight = width() / fAspect;
				m_yMax = m_yMin + newHeight;
			}
			else
			{
				T newWidth = height() * fAspect;
				m_xMax = m_xMin + newWidth;
			}
		}
	}

private:

	//! The minimum x value
	T m_xMin;

	//! The minimum y value
	T m_yMin;

	//! The maximum x value
	T m_xMax;

	//! The maximum y value
	T m_yMax;
};

//! Typedefs for commonly used rectangles
PYXLIB_DECL typedef PYXRect2D<int> PYXRect2DInt;
PYXLIB_DECL typedef PYXRect2D<float> PYXRect2DFloat;
PYXLIB_DECL typedef PYXRect2D<double> PYXRect2DDouble;

#ifndef DOXYGEN_IGNORE	// doxygen cannot handle template method specialization

//! Specialization for equal method
template<> bool PYXRect2DFloat::equal(const PYXRect2DFloat& r, float fPrecision) const
{
	return (	MathUtils::equal(m_xMin, r.m_xMin, fPrecision) &&
				MathUtils::equal(m_yMin, r.m_yMin, fPrecision) &&
				MathUtils::equal(m_xMax, r.m_xMax, fPrecision) &&
				MathUtils::equal(m_yMax, r.m_yMax, fPrecision)	);
}

//! Specialization for equal method
template<> bool PYXRect2DFloat::equal(const PYXRect2DFloat& r) const
{
	return equal(r, MathUtils::kfDefaultFloatPrecision);
}

//! Specialization for equal method
template<> bool PYXRect2DDouble::equal(const PYXRect2DDouble& r, double fPrecision) const
{
	return (	MathUtils::equal(m_xMin, r.m_xMin, fPrecision) &&
				MathUtils::equal(m_yMin, r.m_yMin, fPrecision) &&
				MathUtils::equal(m_xMax, r.m_xMax, fPrecision) &&
				MathUtils::equal(m_yMax, r.m_yMax, fPrecision)	);
}

//! Specialization for equal method
template<> bool PYXRect2DDouble::equal(const PYXRect2DDouble& r) const
{
	return equal(r, MathUtils::kfDefaultDoublePrecision);
}

#endif	// DOXYGEN_IGNORE

//! Allows templated 2D rects to be written to streams.
template <class T>
std::basic_ostream< char>& operator <<(std::basic_ostream< char>& out, const PYXRect2D<T>& rect)
{
	out << rect.xMin() << " " << rect.xMax() << " " << rect.yMin() << " " << rect.yMax();
	return out;
}

//! Allows templated 2D rects to be read from streams.
template <class T>
std::basic_istream< char>& operator >>(std::basic_istream< char>& input, PYXRect2D<T>& rect)
{
	T xMin;
	T xMax;
	T yMin;
	T yMax;
	input >> xMin >> xMax >> yMin >> yMax;
	rect.setXMin(xMin);
	rect.setXMax(xMax);
	rect.setYMin(yMin);
	rect.setYMax(yMax);
	return input;
}

////! Allows templated 2D rects to be written to streams.
//template <class T>
//std::ostream & operator <<(std::ostream & out, const PYXRect2D<T>& rect)
//{
//	out << rect.xMin() << " " << rect.xMax() << " " << rect.yMin() << " " << rect.yMax();
//	return out;
//}
//
////! Allows templated 2D rects to be read from streams.
//template <class T>
//std::ostream & operator >>(std::ostream & input, PYXRect2D<T>& rect)
//{
//	T xMin;
//	T xMax;
//	T yMin;
//	T yMax;
//	input >> xMin >> xMax >> yMin >> yMax;
//	rect.setXMin(xMin);
//	rect.setXMax(xMax);
//	rect.setYMin(yMin);
//	rect.setYMax(yMax);
//	return input;
//}

#endif // guard
