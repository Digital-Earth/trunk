#ifndef LIBRARY_PROCESS_RESOLVER_H
#define LIBRARY_PROCESS_RESOLVER_H

/******************************************************************************
library_process_resolver.h

begin		: October 8, 2009
copyright	: (C) 2009 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "library_config.h"

// pyxlib includes
#include "pyxis/pipe/pipe_manager.h"

/*!
Resolves pipelines from the database.  Uses SWIG directors to call into a 
derived C# LibraryProcessResolver to interact with the database.
*/
class LIBRARY_DECL LibraryProcessResolver : public ProcessResolver
{

private: 

	static boost::intrusive_ptr<IProcess> wrapResolved(boost::intrusive_ptr<IProcess> spProcess);

protected:

	LibraryProcessResolver()
	{
	}

// SWIG doesn't know about addRef and release, since they are defined in 
// the opaque PYXObject.  Add them here so they get director'ed.
public:

	virtual long release() const
	{
		return PYXObject::release();
	}

	virtual long addRef() const
	{
		return PYXObject::addRef();
	}

public:

	//! Virtual destructor.
	virtual ~LibraryProcessResolver()
	{
		k_resolver = 0;
	}

public:

	static PYXPointer<LibraryProcessResolver> create()
	{
		return PYXNEW(LibraryProcessResolver);
	}

	virtual boost::intrusive_ptr<IProcess> resolve(const ProcRef& procRef);

	virtual boost::intrusive_ptr<IProcess> notifyResolve(boost::intrusive_ptr<IProcess> spProcess);

public:

	//! Sets the pointer to the derived C# LibraryProcessResolver. 
	static void set(PYXPointer<LibraryProcessResolver> resolver);
	
	//! Resets the pointer.
	static void reset();

private:

	static PYXPointer<LibraryProcessResolver> k_resolver;
};

#endif // guard