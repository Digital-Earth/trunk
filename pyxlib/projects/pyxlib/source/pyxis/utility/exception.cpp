/******************************************************************************
exception.cpp

begin		: 2005-11-15
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/exception.h"

//! global static trace output variable, disabled by default.
bool PYXException::m_bTrace = false;

/*! 
Disable exception tracing to the trace log.
*/
void PYXException::disableTrace()
{
	TRACE_INFO("Global exception tracing disabled.");
	m_bTrace = false;
}

//! Enable exception tracing
void PYXException::enableTrace()
{
	TRACE_INFO("Global exception tracing enabled.");
	m_bTrace = true;
}
