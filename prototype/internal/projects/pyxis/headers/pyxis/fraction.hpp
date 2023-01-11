#if !defined(PYXIS__FRACTION)
#define PYXIS__FRACTION

#include "pyxis/pointee.hpp"
#include <boost/static_assert.hpp>
#include <stdexcept>

namespace Pyxis
{
	template < typename UnsignedInteger > class Fraction;
}

// An unsigned fraction between 0 and 1 inclusive.
template < typename UnsignedInteger >
class Pyxis::Fraction : public virtual Pointee
{
	// Statically assert that UnsignedInteger is unsigned integral type.
	typedef boost::integer_traits< UnsignedInteger > traits;
	BOOST_STATIC_ASSERT(traits::is_integral);
	BOOST_STATIC_ASSERT(0 == traits::const_min);

	UnsignedInteger m_numerator;
	UnsignedInteger m_denominator;

public:

	// Throws if denominator is 0, or the fraction is greater than 1.
	// Performs no normalization, i.e. to lowest common denominator.
	explicit Fraction(UnsignedInteger numerator, UnsignedInteger denominator) :
	m_numerator(numerator),
	m_denominator(denominator)
	{
		if (0 == m_denominator)
		{
			throw std::invalid_argument("The denominator cannot be zero.");
		}

		if (m_denominator < m_numerator)
		{
			throw std::invalid_argument("The numerator cannot exceed the denominator.");
		}
	}

	// Returns the numerator.
	UnsignedInteger getNumerator() const
	{
		return m_numerator;
	}

	// Returns the denominator.
	UnsignedInteger getDenominator() const
	{
		return m_denominator;
	}

	// Returns the value as a double.
	double getDouble() const
	{
		return m_numerator / m_denominator;
	}

	// Normalizes to lowest common denominator.
	void normalize() const;
};

#endif
