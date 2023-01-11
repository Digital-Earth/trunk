#ifndef PYXIS__DATA__GEOPACKET_SOURCE_H
#define PYXIS__DATA__GEOPACKET_SOURCE_H
/******************************************************************************
geopacket_source.h

begin		: 2014-06-04
copyright	: (C) 2014 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/pyxcom.h"

struct PYXLIB_DECL IGeoPacketSource : public PYXCOM_IUnknown
{
	PYXCOM_DECLARE_INTERFACE();
};
#endif // guard
