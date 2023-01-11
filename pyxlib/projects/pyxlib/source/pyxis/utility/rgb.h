#ifndef PYX_RGB_H
#define PYX_RGB_H
/******************************************************************************
pyx_rgb.h

begin		: 2004-11-11
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxlib.h"

// system includes
#include <cassert>
#include <limits>
#include <string>

//! Represents an RGBA colour value.
class PYXLIB_DECL PYXRGB
{
public:

	//! Default constructor sets RGB to 0 and alpha to 255.
	PYXRGB() { reset(); }

	/*!
	\param nR	The red component (0 to 255).
	\param nG	The green component (0 to 255).
	\param nB	The blue component (0 to 255).
	*/
	//! Constructor sets components as specified.
	PYXRGB(int nR, int nG, int nB) { set(nR, nG, nB); }

	/*!
	\param nR	The red component (0 to 255).
	\param nG	The green component (0 to 255).
	\param nB	The blue component (0 to 255).
	\param nA	The alpha component (0 to 255).
	*/
	//! Constructor sets components as specified.
	PYXRGB(int nR, int nG, int nB, int nA) { set(nR, nG, nB, nA); }

public:

	//! Get the red component (0 to 255).
	int red() const { return m_tuple[0]; }

	//! Get the red component (0.0 to 1.0).
	double redAsDouble() const
	{
		return red() / static_cast<double>(getMaxComponent());
	}

	//! Set the red component (0 to 255).
	void setRed(int nR)
	{
		checkComponent(nR);
		m_tuple[0] = nR;
	}

	//! Set the red component (0.0 to 1.0).
	void setRedAsDouble(double fR)
	{
		setRed(static_cast<int>(fR * getMaxComponent()));
	}

public:

	//! Get the green component (0 to 255).
	int green() const { return m_tuple[1]; }

	//! Get the green component (0.0 to 1.0).
	double greenAsDouble() const
	{
		return green() / static_cast<double>(getMaxComponent());
	}

	//! Set the green component (0 to 255).
	void setGreen(int nG)
	{
		checkComponent(nG);
		m_tuple[1] = nG;
	}

	//! Set the green component (0.0 to 1.0).
	void setGreenAsDouble(double fG)
	{
		setGreen(static_cast<int>(fG * getMaxComponent()));
	}

public:

	//! Get the blue component (0 to 255).
	int blue() const { return m_tuple[2]; }

	//! Get the blue component (0.0 to 1.0).
	double blueAsDouble() const
	{
		return blue() / static_cast<double>(getMaxComponent());
	}

	//! Set the blue component (0 to 255).
	void setBlue(int nB)
	{
		checkComponent(nB);
		m_tuple[2] = nB;
	}

	//! Set the blue component (0.0 to 1.0).
	void setBlueAsDouble(double fB)
	{
		setBlue(static_cast<int>(fB * getMaxComponent()));
	}

public:

	//! Get the alpha component (0 to 255).
	int alpha() const { return m_tuple[3]; }

	//! Get the alpha component (0.0 to 1.0).
	double alphaAsDouble() const
	{
		return alpha() / static_cast<double>(getMaxComponent());
	}

	//! Set the alpha component (0 to 255).
	void setAlpha(int nA)
	{
		checkComponent(nA);
		m_tuple[3] = nA;
	}

	//! Set the alpha component (0.0 to 1.0).
	void setAlphaAsDouble(double fA)
	{
		checkComponent(fA);
		setAlpha(static_cast<int>(fA * getMaxComponent()));
	}

public:

	//! Invert color of this RGB (change zero bits to 1 and 1 bits to zero)
	void invertColor()
	{
		// Set selected color (inverse of current color)
		int nMask = 0xFF;

		setRed((~red()) & nMask);
		setGreen((~green()) & nMask);
		setBlue((~blue()) & nMask);

		// We don't flip alpha as that controls transparency (normally).
	}

	//! Resets RGB to 0 and alpha to 255.
	void reset() { set(0, 0, 0, getMaxComponent()); }

	/*!
	\param nR	The red component (0 to 255).
	\param nG	The green component (0 to 255).
	\param nB	The blue component (0 to 255).
	*/
	//! Sets the components.
	void set(int nR, int nG, int nB)
	{
		setRed(nR);
		setGreen(nG);
		setBlue(nB);
	}

	/*!
	\param nR	The red component (0 to 255).
	\param nG	The green component (0 to 255).
	\param nB	The blue component (0 to 255).
	\param nA	The alpha component (0 to 255).
	*/
	//! Sets the components.
	void set(int nR, int nG, int nB, int nA)
	{
		setRed(nR);
		setGreen(nG);
		setBlue(nB);
		setAlpha(nA);
	}

	/*!
	\param fR	The red component (0.0 to 1.0).
	\param fG	The green component (0.0 to 1.0).
	\param fB	The blue component (0.0 to 1.0).
	*/
	//! Sets the components.
	void setAsDouble(double fR, double fG, double fB)
	{
		setRedAsDouble(fR);
		setGreenAsDouble(fG);
		setBlueAsDouble(fB);
	}

	/*!
	\param fR	The red component (0.0 to 1.0).
	\param fG	The green component (0.0 to 1.0).
	\param fB	The blue component (0.0 to 1.0).
	\param fA	The alpha component (0.0 to 1.0).
	*/
	//! Sets the components.
	void setAsDouble(double fR, double fG, double fB, double fA)
	{
		setRedAsDouble(fR);
		setGreenAsDouble(fG);
		setBlueAsDouble(fB);
		setAlphaAsDouble(fA);
	}

private:

	//! Return the maximum component value.
	int getMaxComponent() const
	{
		return 255; //std::numeric_limits<ComponentType>::max();
	}

	//! Check that component is in range.
	void checkComponent(int n)
	{
		assert(0 <= n && n <= getMaxComponent());
	}

	//! Check that component is in range.
	void checkComponent(double f)
	{
		assert(0.0 <= f && f <= 1.0);
	}

private:

	//! A colour entry has four components of this type.
	typedef unsigned char ComponentType;

	//! A colour entry has four components (red, green, blue, alpha).
	ComponentType m_tuple[4];
};

//! Write an RGB to a stream.
PYXLIB_DECL std::ostream& operator <<(std::ostream& out, const PYXRGB& rgb);

//! Read an RGB from a stream.
PYXLIB_DECL std::istream& operator >>(std::istream& in, PYXRGB& rgb);

#endif	// PYX_RGB_H
