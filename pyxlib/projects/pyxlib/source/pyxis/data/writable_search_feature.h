#ifndef WRITABLE_SEARCH_FEATURE_H
#define WRITABLE_SEARCH_FEATURE_H
/******************************************************************************
writable_search_feature.h

begin		: June 20, 2008
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/data/pyx_feature.h"

class PYXLIB_DECL WritableSearchFeature : public PYXFeature
{
	PYXCOM_DECLARE_CLASS()

public: //PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IWritableFeature)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IRecord)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL();

	IUNKNOWN_DEFAULT_CAST(WritableSearchFeature, IWritableFeature);

public:

	//! Default Constructor.
	WritableSearchFeature(){;}

	//! Alternate Constructor.
	WritableSearchFeature(PYXPointer<PYXGeometry> spGeom, std::string strId, std::string strStyle, bool isWritable, 
		PYXPointer<PYXTableDefinition> spTableDef, std::string strGeomName="");

	//! Destructor
	~WritableSearchFeature(){;}
};


#endif