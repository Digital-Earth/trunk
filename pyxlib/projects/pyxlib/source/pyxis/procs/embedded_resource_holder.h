#ifndef PYXIS__PROCS__EMBEDDED_RESOURCE_HOLDER_H
#define PYXIS__PROCS__EMBEDDED_RESOURCE_HOLDER_H
/******************************************************************************
embedded_resource_holder.h

begin		: Apr 27, 2010
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "pyxlib.h"

// pyxlib includes
#include "pyxis/pipe/process.h"
#include "pyxis/utility/object.h"

/*!

IEmbeddedResourceHolder - interface to allow process to hold embedded resource inside itself.

A process may implemenet this interface to be able to add more local resources when published.
A publisher would publish all embedded resources when the process is published.

*/
//! IEmbeddedResourceHolder - interface to allow process to hold embedded resource inside iself.
struct PYXLIB_DECL IEmbeddedResourceHolder : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! Returns the number of resources embedded inside.
	virtual int getEmbeddedResourceCount() const = 0;

	//! Returns a pointer to the IProcess interface of the embedded resrouce
	virtual boost::intrusive_ptr<IProcess> getEmbeddedResource(int index) const = 0;

};



#endif // guard
