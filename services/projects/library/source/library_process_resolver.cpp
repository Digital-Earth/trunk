/******************************************************************************
library_process_resolver.cpp

begin		: October 8, 2009
copyright	: (C) 2009 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define LIBRARY_SOURCE
#include "library_process_resolver.h"

// local includes
#include "exceptions.h"

PYXPointer<LibraryProcessResolver> LibraryProcessResolver::k_resolver;

void LibraryProcessResolver::set(PYXPointer<LibraryProcessResolver> resolver)
{
	k_resolver = resolver;
}

void LibraryProcessResolver::reset()
{
	k_resolver = 0;
}

boost::intrusive_ptr<IProcess> LibraryProcessResolver::resolve(
	const ProcRef& procRef)
{
	if (k_resolver == 0)
	{
		PYXTHROW(LibraryException, "Director has not been set!");
	}
	return k_resolver->resolve(procRef);
}

boost::intrusive_ptr<IProcess> LibraryProcessResolver::notifyResolve(
	boost::intrusive_ptr<IProcess> spProcess)
{
	if (k_resolver == 0)
	{
		PYXTHROW(LibraryException, "Director has not been set!");
	}
	return k_resolver->notifyResolve(spProcess);
}