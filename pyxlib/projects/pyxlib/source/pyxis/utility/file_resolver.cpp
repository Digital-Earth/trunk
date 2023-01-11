/******************************************************************************
file_resolver.cpp

begin		: 2007-3-29
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "file_resolver.h"

FileResolver::ResolverList FileResolver::m_registeredResolvers;
