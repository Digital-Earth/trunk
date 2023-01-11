#ifndef SERACH_RESULT_PROC_H
#define SEARCH_RESULT_PROC_H
/******************************************************************************
search_result_proc.h

begin		: 2008-06-09
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "module_gdal.h"

//pyxlib includes.
#include "pyxis/data/writable_search_feature.h"
#include "pyxis/pipe/process.h"

class MODULE_GDAL_DECL SearchResultProcess : 
	public ProcessImpl<SearchResultProcess>, 
	public IFeature
{
	PYXCOM_DECLARE_CLASS();

public:
	//! Default Constructor
	SearchResultProcess():m_fLat(0), m_fLon(0){;}

	//! Destructor
	~SearchResultProcess(){;}

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IRecord)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST(SearchResultProcess, IProcess);

public: // IRecord

	virtual PYXPointer<const PYXTableDefinition> STDMETHODCALLTYPE getDefinition() const;

	virtual PYXPointer<PYXTableDefinition> STDMETHODCALLTYPE getDefinition();

	virtual PYXValue STDMETHODCALLTYPE getFieldValue(int nFieldIndex) const;

	virtual void STDMETHODCALLTYPE setFieldValue(PYXValue value, int nFieldIndex);

	virtual PYXValue STDMETHODCALLTYPE getFieldValueByName(const std::string& strName) const;

	virtual void STDMETHODCALLTYPE setFieldValueByName(PYXValue value, const std::string& strName);

	virtual std::vector<PYXValue> STDMETHODCALLTYPE getFieldValues() const;

	virtual void STDMETHODCALLTYPE setFieldValues(const std::vector<PYXValue>& vecValues);

	virtual void STDMETHODCALLTYPE addField(const std::string& strName, 
		PYXFieldDefinition::eContextType nContext,
		PYXValue::eType nType,
		int nCount = 1,
		PYXValue value = PYXValue() 
		);

public: //IFeature

	IFEATURE_IMPL();

public:

	IPROCESS_GETSPEC_IMPL();

	//! Get the output of this process.
	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const;

	//! Get the output of this process.
	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput();

	//! Get the schema to edit this process.
	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	//! Get the attributes in this process.
	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

	//! Set the attributes in this process.
	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);	

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

private: 

	//! The IWritable feature this process wraps.
	boost::intrusive_ptr<WritableSearchFeature> m_spOutputFeature;

	//! The latitude.
	double m_fLat;
	
	//! The longitude
	double m_fLon;

	//! A Url to add to this feature
	std::string m_strUrl;

	//! The Description of this feature.
	std::string m_strDesc;

	//! The Name of this feature.
	std::string m_strName;

	//! Attribute to determine if the output feature is writable or not.
	bool m_writable;
	
};
#endif