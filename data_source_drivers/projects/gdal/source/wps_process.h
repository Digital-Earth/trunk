#ifndef WPS_PROCESS_H
#define WPS_PROCESS_H

/******************************************************************************
wps_process.h

begin      : 9/01/2011 9:57:18 AM
copyright  : (c) 2011 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

// local includes
#include "ows_reference.h"
#include "ows_network_resource.h"
#include "module_gdal.h"

// pyxlib includes
#include "pyxis/derm/horizontal_datum.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/path.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/sampling/xy_iterator.h"

// GDAL includes
#include "ogr_spatialref.h"

// boost includes
#include <boost/filesystem/path.hpp>
#include <boost/thread/recursive_mutex.hpp>

// local forward declarations
class ProjectionMethod;
class PYXGeometry;
class GDALXYCoverage;
class PYXSpatialReferenceSystem;



class WPSAttributeSpec : public PYXObject
{
public:
	static PYXPointer<WPSAttributeSpec> create(
		const std::string & identifier,
		const std::string & title, 
		const std::string & type,
		int minOccurs,int maxOccurs)
	{
		return PYXNEW(WPSAttributeSpec,identifier,title,type,minOccurs,maxOccurs);
	}

	virtual ~WPSAttributeSpec()
	{
	}

	WPSAttributeSpec(const std::string & identifier,const std::string & title, const std::string & type,int minOccurs,int maxOccurs) 
		: m_identifier(identifier), m_title(title), m_type(type), m_minOccurs(minOccurs), m_maxOccurs(maxOccurs)
	{
	}

	const std::string & getIdentifier() const { return m_identifier; };
	const std::string & getTitle() const { return m_title; };
	const std::string & getType() const { return m_type; };
	int getMinOccurs() const { return m_minOccurs; };
	int getMaxOccurs() const { return m_maxOccurs; };

private:
	std::string m_identifier;
	std::string m_title;
	std::string m_type;
	int m_minOccurs;
	int m_maxOccurs;
};

class WPSProcessMetadata : public PYXObject
{
public:

	static PYXPointer<WPSProcessMetadata> create(const std::string & fromString)
	{
		return PYXNEW(WPSProcessMetadata,fromString);
	}

	virtual ~WPSProcessMetadata()
	{
	}

private:
	WPSProcessMetadata(const std::string & fromString); 

public:
	typedef std::map<std::string,PYXPointer<WPSAttributeSpec>> AttributesSpecMap;
	typedef std::list<PYXPointer<OWSFormat>> ParameterFormatsList;

	PYXPointer<ProcessSpec> getSpec() const;

	ParameterFormatsList getParamterFormats(int paramterIndex) const;

	ParameterFormatsList getOutputFormats() const;

	const AttributesSpecMap & getAttributesSpec() const { return m_attributes; }

	std::list<std::string> getAttributeNames() const;

	PYXPointer<WPSAttributeSpec> getAttributeSpec(const std::string & attributeId) const;

	const std::string & getOutputIdentifier() const { return m_outputId; }

	const std::string & getProcessIdentifier() const { return m_processId; }

	const std::string & getServer() const { return m_server; }

private:
	PYXPointer<ProcessSpec> m_spec;
	
	AttributesSpecMap m_attributes;
	std::list<ParameterFormatsList> m_paramterFormats;
	ParameterFormatsList m_outputFormats;

	std::string m_server;
	std::string m_version;
	std::string m_processId;
	std::string m_processDesc;
	std::string m_outputId;
};

/*!
Provides access to WMS data through the GDAL library.
*/
class MODULE_GDAL_DECL WPSProcess : public ProcessImpl<WPSProcess>
{
	PYXCOM_DECLARE_CLASS();

public:

	//! Test method
	static void test();

public: // PYXCOM_IUnknown

	IUNKNOWN_QI_BEGIN
		IUNKNOWN_QI_CASE(IProcess)
	IUNKNOWN_QI_END

	IUNKNOWN_RC_IMPL_FINALIZE();

public: //IProcess
	
	//HACK: to make ProcessImpl to work
	static PYXPointer<ProcessSpec> getSpecStatic() 
	{ 
		static PYXPointer<ProcessSpec> spProcSpec(ProcessSpec::create( 
			clsid, 
			std::vector<IID>(), 
			std::vector<PYXPointer<ParameterSpec> >(), 
			"WPS Process", 
			"WPS Process" )); 
		spProcSpec->setCategory("Hidden");
		return spProcSpec; 
	}; 

	//! The specification that define inputs and outputs for the process.
	virtual PYXPointer<ProcessSpec> STDMETHODCALLTYPE getSpec() const;

	virtual void STDMETHODCALLTYPE setData(const std::string& strData);

	virtual boost::intrusive_ptr<const PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput() const
	{
		if (m_coverageNetworkResource)
		{
			return m_coverageNetworkResource->getOutput();
		}
		if (m_vectorNetworkResource)
		{
			return m_vectorNetworkResource->getOutput();
		}
		return NULL;
	}

	virtual boost::intrusive_ptr<PYXCOM_IUnknown> STDMETHODCALLTYPE getOutput()
	{
		if (m_coverageNetworkResource)
		{
			return m_coverageNetworkResource->getOutput();
		}
		if (m_vectorNetworkResource)
		{
			return m_vectorNetworkResource->getOutput();
		}
		return NULL;
	}

	virtual std::string STDMETHODCALLTYPE getAttributeSchema() const;

	virtual void STDMETHODCALLTYPE setAttributes(const std::map<std::string, std::string>& mapAttr);

	virtual std::map<std::string, std::string> STDMETHODCALLTYPE getAttributes() const;

protected: // ProcessImpl

	virtual IProcess::eInitStatus initImpl();


private:
	static const std::string kstrRequestFileName;
	static const std::string kstrStatusResponseFileName;
	static const std::string kstrResultUrlFileName;

	void createCacheState();

	void storeNewRequestInCache(const std::string & requestDetails);
	void storeNewStatusResponseInCache(const std::string & responseDetails);
	void storeNewResultUrlInCache(const std::string & url);

	void parseRepsonse();

private:
	//! the WPS process metadata
	PYXPointer<WPSProcessMetadata> m_wpsMetadata;

	std::map<std::string, std::string> m_attributesValue;

	//the last request performed (as XML doc)
	std::string m_lastRequestDetails;

	//the time when last request was perform
	time_t m_lastRequestSent;

	//the last response got from wps
	std::string m_lastStatusResponseDetails;

	//the time when the last response was answered
	time_t m_lastStatusResponseSent;

	//url for the next status request
	std::string m_nextStatusUrl;

	//string describing the error returned from the wps.
	std::string m_wpsProcessError;

	enum eExecuteStatus
	{
		knNotRequested,
		knFailedToRequest,
		knRequestedSuccessfully,
		knExectueFailed,
		knExectueCompleted
	};	

	eExecuteStatus m_executeStatus;

	//the result url
	std::string m_resultUrl;

	//processs identity cache directory - this is where we store the process to start, files downloaded etc
	std::string m_strCacheDir;

	//the network resource process that downloads and read the result file (if output is an coverage)
	boost::intrusive_ptr<OwsCoverageNetworkResourceProcess> m_coverageNetworkResource;

	//the network resource process that downloads and read the result file (if output is a vector file)
	boost::intrusive_ptr<OwsVectorNetworkResourceProcess> m_vectorNetworkResource;

	//! Mutex for thread safety.
	mutable boost::recursive_mutex m_mutex;
};

#endif