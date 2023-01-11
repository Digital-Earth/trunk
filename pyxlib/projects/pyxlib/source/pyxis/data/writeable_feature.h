#ifndef WRITEABLE_FEATURE_H
#define WRITEABLE_FEATURE_H
/******************************************************************************
writeable_feature.h

begin		: 2007-10-25
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "pyxlib.h"
#include "pyxis/utility/pyxcom.h"

#include "feature.h"

struct PYXLIB_DECL IWritableFeature : public IFeature
{
	PYXCOM_DECLARE_INTERFACE();

public:

	//! Set the id of this feature.
	virtual void STDMETHODCALLTYPE setID(const std::string & strID) = 0;

	//! Set the name for this named Geometry
	virtual void STDMETHODCALLTYPE setGeometryName(const std::string & strName) = 0;

	//! Set the geometry to be named.
	virtual void STDMETHODCALLTYPE setGeometry(const PYXPointer<PYXGeometry> & spGeom) = 0;

	//! Set whether this feature and it's data can be written to and changed.
	virtual void STDMETHODCALLTYPE setIsWritAble(bool bWritable) = 0;

	//! Set the style that this feature is supposed to be styled with.
	virtual void STDMETHODCALLTYPE setStyle(const std::string & style) = 0;

	//! Set the definition of the meta data for this feature.
	virtual void STDMETHODCALLTYPE setMetaDataDefinition(const PYXPointer<PYXTableDefinition> & spDef) = 0;

};
#endif 