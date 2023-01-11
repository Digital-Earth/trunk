#ifndef PYX_FEATURE_H
#define PYX_FEATURE_H

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/data/writeable_feature.h"
#include "pyxis/data/feature.h"
#include "pyxis/pipe/process.h"

class PYXLIB_DECL PYXFeature : public IWritableFeature
{
	PYXCOM_DECLARE_CLASS();

public:
	
	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IWritableFeature)
		IUNKNOWN_QI_CASE(IRecord)
	IUNKNOWN_QI_END
	
	IUNKNOWN_RC_IMPL();

	IUNKNOWN_DEFAULT_CAST(PYXFeature, IWritableFeature);

public: //IFeature
	
	IFEATURE_IMPL()

public: //IRecord

	IRECORD_IMPL();

public: //IWritableFeature

	virtual void STDMETHODCALLTYPE setID (const std::string & strID);

	virtual void STDMETHODCALLTYPE setGeometryName(const std::string & strName);

	virtual void STDMETHODCALLTYPE setGeometry(const PYXPointer<PYXGeometry> & spGeom);

	virtual void STDMETHODCALLTYPE setIsWritAble(bool bWritable);

	virtual void STDMETHODCALLTYPE setStyle(const std::string & style);

	virtual void STDMETHODCALLTYPE setMetaDataDefinition(const PYXPointer<PYXTableDefinition> & spDef);

public: 

	//! Default constructor
	PYXFeature();

	//! Alternate constructor to construct a feature by passing in all the data required to initalize it with.
	PYXFeature(PYXPointer<PYXGeometry> spGeom, std::string strId, std::string strStyle, bool isWritable, 
		PYXPointer<PYXTableDefinition> spTableDef, std::string strGeomName="");

	//! Destructor
	~PYXFeature(){;}

private:

	//! The named to associate the geometry with this feature.
	std::string m_strGeomName;

};

#endif