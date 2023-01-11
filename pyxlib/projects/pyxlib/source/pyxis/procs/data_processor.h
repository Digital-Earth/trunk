#ifndef PYXIS__PROCS__DATA_PROCESSOR_H
#define PYXIS__PROCS__DATA_PROCESSOR_H

/******************************************************************************
data_processor.h

begin      : 28/01/2008 2:18:40 PM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"

// pyxlib includes
#include "pyxis/pipe/process.h"

//! Interface for an element that processes data
struct PYXLIB_DECL IDataProcessor : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:

	virtual void STDMETHODCALLTYPE processData() = 0;
};

#endif // guard
