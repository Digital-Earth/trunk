#ifndef NAMED_GEOMETRY_PROC_H
#define NAMED_GEOMETRY_PROC_H
/******************************************************************************
named_geometry_proc.h

begin		: 2007-10-18
copyright	: (C) 2007 by the PYXIS innovation Inc.
web			: www.pyxisinnovation.com
******************************************************************************/

//local includes
#include "pyxlib.h"

//pyxlib includes
#include "pyxis/data/writeable_feature.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/pyxcom.h"

class PYXLIB_DECL NamedGeometryProc : 
	public ProcessImpl<NamedGeometryProc>, 
	public IWritableFeature
{
	PYXCOM_DECLARE_CLASS();

public:

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IWritableFeature)
	IUNKNOWN_QI_END
 
	IUNKNOWN_RC_IMPL_FINALIZE();

public: //IProcess

	IPROCESS_GETSPEC_IMPL();

	//! Get the output of this process.
	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IFeature*>(this);
	}

	//! Get the output of this process.
	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IFeature*>(this);
	}

	/*!
	Get the attributes associated with  this process. 

	\return a map of standard string - standard string containing the attributes to be serialized.
	*/
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;	

	//! Get the schema to describe how the attributes should be edited.
	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const
	{
		return "";
	}

	//! Set the attributes of this process. 
	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: //IFeature

	IFEATURE_IMPL();

public: // IRecord

	IRECORD_IMPL();
	
public: //IWritableFeature
	
	//! Set the id of this feature.
	virtual void STDMETHODCALLTYPE setID (const std::string & strID);

	//! Set the geometry to be named.
	virtual void  STDMETHODCALLTYPE setGeometry(const PYXPointer<PYXGeometry> & spGeom);

	//! Set the name of the geometry. 
	virtual void  STDMETHODCALLTYPE setGeometryName(const std::string & strName)
	{
		m_strName = strName;
	}

	//! Set whether this feature and it's data can be written to and changed.
	virtual void STDMETHODCALLTYPE setIsWritAble(bool bWritable);

	//! Set the style that this feature is supposed to be styled with.
	virtual void STDMETHODCALLTYPE setStyle(const std::string & style);

	//! Set the definition of the meta data for this feature.
	virtual void STDMETHODCALLTYPE setMetaDataDefinition(const PYXPointer<PYXTableDefinition> & spDef);

public: //NamedGeometry

	NamedGeometryProc(const PYXPointer<PYXGeometry> spGeom) : m_defaultID(true), m_bWritable(true), m_spDefn(PYXTableDefinition::create())
	{
		setGeometry(spGeom);
	}

	//! Default Constructor
	NamedGeometryProc() : m_defaultID(true), m_bWritable(true), m_spDefn(PYXTableDefinition::create()) {;}

	//Destructor
	~NamedGeometryProc(){;}

public: //testing
	static void test();

private:

	//! boost path containing the path to the serialized geometry.
	boost::filesystem::path m_path;
	
	//! The name of the file to serialize the geometry to.
	std::string m_strName;

	bool m_defaultID;
};

#endif //end guard