/******************************************************************************
parameter.cpp

begin		: 2006-12-04
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/pipe/parameter.h"

/*!
Finds the location of the given pipeline inside the value list of the parameter.
if the given value wasn't found, the function return -1.
*/
//! Finds the location of the given pipeline inside the value list of the parameter.
int Parameter::findValue(boost::intrusive_ptr<IProcess> spProc)
{
	for(unsigned int i=0;i<m_vecValue.size();i++)
	{
		if (m_vecValue[i] == spProc)
		{
			return static_cast<int>(i);
		}
	}
	return -1;
}

/*!
Finds the location of the given pipeline inside the value list of the parameter.
if the given value wasn't found, the function return -1.
*/
//! Finds the location of the given pipeline inside the value list of the parameter.
int Parameter::findValue(const ProcRef & procRef)
{
	for(unsigned int i=0;i<m_vecValue.size();i++)
	{
		if (ProcRef(m_vecValue[i]) == procRef)
		{
			return static_cast<int>(i);
		}
	}
	return -1;
}
