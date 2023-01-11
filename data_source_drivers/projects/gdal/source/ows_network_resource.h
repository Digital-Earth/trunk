#ifndef OWS_NETWORK_RESOURCE_H
#define OWS_NETWORK_RESOURCE_H

/******************************************************************************
ows_network_resource.h

begin      : 9/01/2011 9:57:18 AM
copyright  : (c) 2011 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "coord_converter_impl.h"
#include "gdal_metadata.h"
#include "ows_reference.h"
#include "gdal_xy_coverage.h"
#include "module_gdal.h"
#include "pyx_rtree.h"

// pyxlib includes
#include "pyxis/derm/horizontal_datum.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/path.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_iterator.h"


class OwsCoverageNetworkResourceProcess : public ProcessImpl<OwsCoverageNetworkResourceProcess>, public IXYCoverage, public IOWSReference
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Test method
	static void test();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IXYCoverage)
		IUNKNOWN_QI_CASE(IOWSReference)
	IUNKNOWN_QI_END

	IUNKNOWN_DEFAULT_CAST(OwsCoverageNetworkResourceProcess, IProcess);

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IRecord
	IRECORD_IMPL_PROXY(*m_spXYCoverage);

public: // IFeature

	IFEATURE_IMPL_PROXY(*m_spXYCoverage);

public: // IFeatureCollection

	IFEATURECOLLECTION_IMPL_PROXY(*m_spXYCoverage);

public: //IXYCoverage

	IXYCOVERAGE_IMPL_PROXY(*m_spXYCoverage);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IXYCoverage*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IXYCoverage*>(this);
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: //IOWSReference
	virtual PYXPointer<OWSFormat> getDefaultOutputFormat() const;

	virtual bool supportOutputType(const OWSFormat & format) const;

	virtual std::string getOWSReference(IOWSReference::ReferenceType referenceType,const OWSFormat & format) const;


protected: // request downloading
	boost::filesystem::path getLocalFile();

	std::string getFileNameFromMimeType();

	bool downloadNetworkResource(const std::string & localFile);

	boost::intrusive_ptr<IProcess> createGdalProcess(const boost::filesystem::path& path);

	bool buildLocalPipeline();

	std::map<std::string,std::string> getRequestHeaders() const;

private:
	boost::intrusive_ptr<IProcess> m_readerProcess;

	//! The GDAL(?) XY Coverage.
	boost::intrusive_ptr<IXYCoverage> m_spXYCoverage;

	//! request Url.
	std::string m_requestUrl;

	//! request method: POST, GET
	std::string m_method;

	//! request body
	std::string m_body;

	//! request headers
	std::string m_headers;

	//! exptected mimetype
	std::string m_responseMimeType;

	//! exptected schema
	std::string m_responseSchema;

	//! exptected encoding
	std::string m_responseEncoding;
};


class OwsVectorNetworkResourceProcess : public ProcessImpl<OwsVectorNetworkResourceProcess>, public IFeatureCollection, public IOWSReference
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Test method
	static void test();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
		IUNKNOWN_QI_CASE(IFeature)
		IUNKNOWN_QI_CASE(IFeatureCollection)
		IUNKNOWN_QI_CASE(IOWSReference)
	IUNKNOWN_QI_END

	IUNKNOWN_DEFAULT_CAST(OwsVectorNetworkResourceProcess, IProcess);

	IUNKNOWN_RC_IMPL_FINALIZE();

public: // IRecord
	IRECORD_IMPL_PROXY(*m_spFC);

public: // IFeature

	IFEATURE_IMPL_PROXY(*m_spFC);

public: // IFeatureCollection

	IFEATURECOLLECTION_IMPL_PROXY(*m_spFC);

public: // IProcess

	IPROCESS_GETSPEC_IMPL();

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		return static_cast<const IFeatureCollection*>(this);
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		return static_cast<IFeatureCollection*>(this);
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();

public: //IOWSReference
	virtual PYXPointer<OWSFormat> getDefaultOutputFormat() const;

	virtual bool supportOutputType(const OWSFormat & format) const;

	virtual std::string getOWSReference(IOWSReference::ReferenceType referenceType,const OWSFormat & format) const;


protected: // request downloading
	boost::filesystem::path getLocalFile();

	std::string getFileNameFromMimeType();

	bool downloadNetworkResource(const std::string & localFile);

	boost::intrusive_ptr<IProcess> createOgrProcess(const boost::filesystem::path& path);

	bool buildLocalPipeline();

	std::map<std::string,std::string> getRequestHeaders() const;

private:
	boost::intrusive_ptr<IProcess> m_readerProcess;

	//! The GDAL(?) XY Coverage.
	boost::intrusive_ptr<IFeatureCollection> m_spFC;

	//! request Url.
	std::string m_requestUrl;

	//! request method: POST, GET
	std::string m_method;

	//! request body
	std::string m_body;

	//! request headers
	std::string m_headers;

	//! exptected mimetype
	std::string m_responseMimeType;

	//! exptected schema
	std::string m_responseSchema;

	//! exptected encoding
	std::string m_responseEncoding;
};

#endif