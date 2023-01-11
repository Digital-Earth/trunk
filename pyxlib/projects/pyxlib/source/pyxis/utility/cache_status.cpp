/******************************************************************************
cache_status.cpp

begin		: 2005-12-02
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/cache_status.h"

// standard includes
#include <cassert>

/*!
Default Constructor.
*/
CacheStatus::CacheStatus()	:
	m_nState(knUninitialized),
	m_nCreateTime(0),
	m_nAccessTime(0)
{
}

/*!
Destructor.
*/
CacheStatus::~CacheStatus()
{
}

/*!
Initialize the timer to be in the valid state with the current time.
*/
void CacheStatus::initialize()
{
	assert(m_nCreateTime == 0 && "Can't initialize the object more than once.");
	setAccessed();
	m_nCreateTime = m_nAccessTime;
	m_nState = knValid;
}

/*!
Set the state of the object.  The object must be initialized (setAccessed must
have been called) before this call is made.

\param nState	The new state for the object (can't be knUninitialized).
*/
void CacheStatus::setState(eTimeState nState)
{
	assert(m_nState != knUninitialized && "Object not initialized yet");
	assert(nState != knUninitialized && "Can't set object to that state");
	m_nState = nState;
}
