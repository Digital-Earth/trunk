#ifndef PYXIS__PROCS__GEOMETRY_PROVIDER_H
#define PYXIS__PROCS__GEOMETRY_PROVIDER_H
/******************************************************************************
geometry_provider.h

begin		: 2013-5-30
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/


// pyxlib includes
#include "pyxlib.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/data/feature_collection.h"
#include "pyxis/data/record.h"


struct PYXLIB_DECL IGeometryProvider : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();

public:
	virtual PYXPointer<PYXGeometry> STDMETHODCALLTYPE getGeometry(boost::intrusive_ptr<IRecord> & record) const = 0;

};

#endif // guard
