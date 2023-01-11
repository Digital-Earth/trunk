/******************************************************************************
ui_events.cpp

begin		: 2010-01-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "StdAfx.h"
#include "ui_events.h"
#include "cml_utils.h"

double UIMouseEvent::mouseDistanceFrom(const double & X,const double & Y) const
{
	return (cml::vector2d(X-m_mouseX,Y-m_mouseY)).length();
}

bool UIMouseEvent::isInsideRect(const double & Xmin,const double & Xmax,const double & Ymin,const double & Ymax) const
{
	return (m_mouseX >= Xmin && m_mouseX <= Xmax && m_mouseY >= Xmin && m_mouseY <= Xmax);
}