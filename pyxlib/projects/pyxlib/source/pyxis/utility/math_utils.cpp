/******************************************************************************
math_utils.cpp

begin		: 2004-04-20
copyright	: (C) 2004 by the PYXIS innovation inc.
web			: www.igotechnologies.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/math_utils.h"

//! An approximation of PI
const double MathUtils::kfPI = 3.1415926535897932;

//! Conversion factor from degrees to radians
const double MathUtils::kfDegreesToRadians = kfPI / 180.0;

//! Pre-calculated radian values.
const double MathUtils::kf30Rad = degreesToRadians(30.0);
const double MathUtils::kf60Rad = degreesToRadians(60.0);
const double MathUtils::kf90Rad = degreesToRadians(90.0);
const double MathUtils::kf120Rad = degreesToRadians(120.0);
const double MathUtils::kf150Rad = degreesToRadians(150.0);
const double MathUtils::kf180Rad = degreesToRadians(180.0);
const double MathUtils::kf240Rad = degreesToRadians(240.0);
const double MathUtils::kf300Rad = degreesToRadians(300.0);
const double MathUtils::kf360Rad = degreesToRadians(360.0);

//! Pre-calculated trig values.
const double MathUtils::kfSin30 = sin(kf30Rad);
const double MathUtils::kfCos30 = cos(kf30Rad);
const double MathUtils::kfSin60 = sin(kf60Rad);
const double MathUtils::kfCos60 = cos(kf60Rad);
const double MathUtils::kfTan60 = tan(kf60Rad);


//! The default float precision
const float MathUtils::kfDefaultFloatPrecision = 1.0e-5f;

//! The default double precision
const double MathUtils::kfDefaultDoublePrecision = 1.0e-10;

//! The golden ratio
const double MathUtils::kfPHI = (sqrt(5.0) + 1.0) / 2.0;

//! The square root of three
const double MathUtils::kfSqrt3 = sqrt(3.0);
