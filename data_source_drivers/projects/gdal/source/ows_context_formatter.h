#ifndef OWS_CONTEXT_FORMATTER_H
#define OWS_CONTEXT_FORMATTER_H

/******************************************************************************
ows_context_formatter.h

begin      : 03/01/2013 9:57:18 AM
copyright  : (c) 2013 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "module_gdal.h"
#include "ows_reference.h"

// pyxlib includes
#include "pyxis/pipe/pipe_formater.h"
#include "pyxis/utility//xml_transform.h"

//! OWSContextFomrmater - format a pipeline into an OWS Context document
struct MODULE_GDAL_DECL OwsContextFormatter : public ProcessImpl<OwsContextFormatter >, public IPipeFormater
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Test method
	static void test();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IPipeFormater)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

	IUNKNOWN_DEFAULT_CAST( OwsContextFormatter , IProcess);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IPipeFormater *>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IPipeFormater *>(this);
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: //IPipeFormater

	//! check if the pipeline formatter can format 
	virtual bool canFormatPipeline(boost::intrusive_ptr<IProcess> pipeline) const;

	virtual std::string formatPipeline(boost::intrusive_ptr<IProcess> pipeline) const;

private:
	void examinPipeline(const PYXPointer<CSharpXMLDoc> & doc, const boost::intrusive_ptr<IProcess> & pipeline) const;

private:
	std::string m_document_title;
	std::string m_document_subtitle;

	std::string m_author_name;
	std::string m_author_email;
	std::string m_author_uri;

	std::string m_location;
};

#endif