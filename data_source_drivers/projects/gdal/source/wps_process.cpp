/******************************************************************************
wps_process.h

begin      : 9/01/2011 9:57:18 AM
copyright  : (c) 2011 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_GDAL_SOURCE

// local includes
#include "wps_process.h"
#include "ows_reference.h"
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/derm/wgs84.h"
#include "pyxis/pipe/process.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"
#include "pyxis/utility/http_utils.h"
#include "pyxis/utility/xml_transform.h"
#include "pyxis/pipe/process_identity_cache.h"

// boost includes
#include <boost/filesystem/path.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/algorithm/string.hpp>

// standard includes
#include <ctime>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>

class WpsExcuteRequestBuilder : public PYXObject
{
public:	
	static PYXPointer<WpsExcuteRequestBuilder> create(const PYXPointer<WPSProcessMetadata> & processMetadata)
	{
		return PYXNEW(WpsExcuteRequestBuilder,processMetadata);
	}

	WpsExcuteRequestBuilder(const PYXPointer<WPSProcessMetadata> & processMetadata ) : m_metadata(processMetadata), m_uri(processMetadata->getServer())
	{
		m_method = "POST";

		m_bodyFullHeader = 
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?><wps:Execute service=\"WPS\" version=\"1.0.0\" "
			"xmlns:wps=\"http://www.opengis.net/wps/1.0.0\" "
			"xmlns:ows=\"http://www.opengis.net/ows/1.1\" "
			"xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
			"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
			"xsi:schemaLocation=\"http://www.opengis.net/wps/1.0.0 http://schemas.opengis.net/wps/1.0.0/wpsAll.xsd\">"
			"<ows:Identifier>" + m_metadata->getProcessIdentifier() + "</ows:Identifier>";

		m_bodyReferenceHeader = "<wps:Execute service=\"WPS\" version=\"1.0.0\">"
			"<ows:Identifier>" + m_metadata->getProcessIdentifier() + "</ows:Identifier>";
	}

	void addInputAsReference(const std::string & inputId, const std::string inputReference)
	{
		m_inputs += "<wps:Input>"
			 "<ows:Identifier>" + inputId + "</ows:Identifier>" +
			 inputReference +
			 "</wps:Input>";
	}

	void addInputAsLiteralValue(const std::string & inputId, const std::string value)
	{
		m_inputs += "<wps:Input>"
			 "<ows:Identifier>" + inputId + "</ows:Identifier>" +
			 "<wps:Data>" +
			 "<wps:LiteralData>" + XMLUtils::toSafeXMLText(value) + "</wps:LiteralData>" +
			 "</wps:Data>" +
			 "</wps:Input>";
	}

	void addRawOutput(const std::string & outputId, const std::string & mime, const std::string & schema)
	{
		m_outputs += 
			"<wps:RawDataOutput mimeType=\""+ mime +"\" schema=\""+ schema+"\">"
			"<ows:Identifier>"+outputId+"</ows:Identifier>"
			"</wps:RawDataOutput>";

		m_outputMimeType = mime;
	}

	void addReferenceOutput(const std::string & outputId, const std::string & mime, const std::string & schema, const std::string & title)
	{
		m_outputs += 
			"<wps:ResponseDocument>"
			"<wps:Output asReference=\"true\" mimeType=\""+ mime +"\" schema=\""+ schema+"\">"
			"<ows:Identifier>"+outputId+"</ows:Identifier>"
			"<ows:Title>"+XMLUtils::toSafeXMLText(title)+"</ows:Title>"
			"</wps:Output>"
			"</wps:ResponseDocument>";

		m_outputMimeType = mime;
	}

	PYXPointer<HttpRequest> getRequest()
	{
		PYXPointer<HttpRequest> request = HttpRequest::create(m_uri,m_method);
		request->addRequestBody(buildBody(false));

		return request;
	}

	boost::intrusive_ptr<OwsCoverageNetworkResourceProcess> createNetworkResource()
	{
		boost::intrusive_ptr<OwsCoverageNetworkResourceProcess> process = boost::intrusive_ptr<OwsCoverageNetworkResourceProcess>(new OwsCoverageNetworkResourceProcess());

		std::map<std::string,std::string> attributes;

		attributes["request_uri"] = m_uri;
		attributes["request_method"] = m_method;
		attributes["request_headers"] = "";
		attributes["request_body"] = buildBody(false);
		attributes["response_mimeType"] = m_outputMimeType;
		attributes["response_schema"] = "";
		attributes["response_encoding"] = "";

		process->setAttributes(attributes);

		return process;
	}

	std::string getOWSReference()
	{
		std::string result = "<wps:Reference mimeType=\"" + m_outputMimeType + "\" xlink:href=\"" + m_uri + "\" method=\"" + boost::to_upper_copy(m_method) + "\">";
		result += "<wps:Body>";
		result += buildBody(true);
		result += "</wps:Body>";
		result += "</wps:Reference>";
		return result;
	}

protected:
	std::string buildBody(bool asReference)
	{
		return 
			(asReference?m_bodyReferenceHeader:m_bodyFullHeader) +
			"<wps:DataInputs>" +
			m_inputs +
			"</wps:DataInputs><wps:ResponseForm>" +
			m_outputs +
			"</wps:ResponseForm></wps:Execute>";
	}

private:
	PYXPointer<WPSProcessMetadata> m_metadata;
	std::string m_uri;
	std::string m_body;
	std::string m_bodyFullHeader;
	std::string m_bodyReferenceHeader;
	std::string m_inputs;
	std::string m_outputs;
	std::string m_outputMimeType;

	std::string m_method;
};


// {1DF6EEB4-7429-481d-BEC8-106546DD0EFA}
PYXCOM_DEFINE_CLSID(WPSProcess,
0x1df6eeb4, 0x7429, 0x481d, 0xbe, 0xc8, 0x10, 0x65, 0x46, 0xdd, 0xe, 0xfa);

PYXCOM_CLASS_INTERFACES(WPSProcess, IProcess::iid, PYXCOM_IUnknown::iid);

const std::string WPSProcess::kstrRequestFileName = "request.xml"; 
const std::string WPSProcess::kstrStatusResponseFileName = "status.xml";
const std::string WPSProcess::kstrResultUrlFileName = "result.url";


// Tester class
Tester<WPSProcess> gTester;

// Test method
void WPSProcess::test()
{
	// TODO: add unit tests.
	std::string schema = 
		"<WPSProcess>"
		"	<Server>serverUrl</Server>"
		"	<Version>1.0.0</Version>"
		"	<Process>"
		"		<Identifier>process.algorithm</Identifier>"
		"		<Title>alogrithm title</Title>"
		"	</Process>"
		"	<Input minOccurs=\"0\" maxOccurs=\"2\">"
		"		<Identifier>coverages</Identifier>"
		"		<Title>0 to 2 coverages</Title>"
		"		<Interface>" + guidToStr(IXYCoverage::iid) + "</Interface>"
		"	</Input>"
		"	<Input minOccurs=\"1\" maxOccurs=\"1\">"
		"		<Identifier>features</Identifier>"
		"		<Title>set of features</Title>"
		"		<Interface>" + guidToStr(IFeatureCollection::iid) + "</Interface>"
		"	</Input>"
		"	<Attribute minOccurs=\"1\" maxOccurs=\"1\">"
		"		<Identifier>attribute1</Identifier>"
		"		<Title>Title</Title>"
		"		<Type>double</Type>"
		"	</Attribute>"
		"	<Output>"
		"		<Identifier>result</Identifier>"
		"		<Interface>" + guidToStr(IXYCoverage::iid) + "</Interface>"
		"	</Output>"
		"</WPSProcess>";

	boost::intrusive_ptr<IProcess> spProc(new WPSProcess());
	TEST_ASSERT(spProc);

	WPSProcess* pCov = dynamic_cast<WPSProcess*>(spProc.get());
	TEST_ASSERT(pCov != 0);

	spProc->setData(schema);

	TEST_ASSERT(schema == spProc->getData());

	PYXPointer<ProcessSpec> spec = spProc->getSpec();
	TEST_ASSERT(spec != 0);
	TEST_ASSERT(IsEqualIID(spec->getClass(),WPSProcess::clsid) != 0);
	TEST_ASSERT(spec->getParameterCount() == 2 + 1 /* ISRS is added always*/);
	TEST_ASSERT(spec->getName() == "process.algorithm");
	TEST_ASSERT(spec->getDescription() == "alogrithm title");
	TEST_ASSERT(spec->getCategory() == "Hidden");

	TEST_ASSERT(spec->getParameterCount() == spProc->getParameterCount());

	TEST_ASSERT(spec->providesOutputType(IXYCoverage::iid));

}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////


/*! 
Serialize the process to a map of strings.

\return the attribute schema as a string.
*/
std::string WPSProcess::getAttributeSchema() const
{
	// default returns a schema of string attributes
	std::string strXSD = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\""
		" elementFormDefault=\"qualified\""
		" xmlns=\"http://tempuri.org/XMLSchema.xsd\""
		" xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\""
		" xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
			"<xs:element name=\"WPSProcess\">"
				"<xs:complexType>"
					"<xs:sequence>";

	std::list<std::string> attrNames = m_wpsMetadata->getAttributeNames();
	
	for (std::list<std::string>::iterator it = attrNames.begin(); it != attrNames.end(); ++it)
	{
		PYXPointer<WPSAttributeSpec> attr = m_wpsMetadata->getAttributeSpec((*it));

		strXSD += "<xs:element name=\"" + attr->getIdentifier() + "\" type=\"xs:"+attr->getType()+"\">";
		strXSD += "<xs:annotation>"
			  "<xs:appinfo>"
				"<friendlyName>" + attr->getIdentifier() + "</friendlyName>"
				"<description>" + attr->getTitle() + "</description>"
			  "</xs:appinfo>"
			"</xs:annotation>"
		  "</xs:element>";
	}
	strXSD += "</xs:sequence></xs:complexType></xs:element></xs:schema>";
	return strXSD;	
}

/*!
Set the attributes for this process.
*/
void STDMETHODCALLTYPE WPSProcess::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{
	m_initState = knNeedsInit;	
	
	m_attributesValue = mapAttr;
}

/*!
Get the attributes for this process.
*/
std::map<std::string, std::string> WPSProcess::getAttributes() const
{
	return m_attributesValue;
}

/*!
Initialize the process so that it is able to provide data.
*/
IProcess::eInitStatus WPSProcess::initImpl()
{
	//TODO: Not sure this need to happen here. parsing the metadata might heppen before the init...
	if (m_initState == knInitialized)
		return knInitialized;

	// Check if the data block has been set to the WMS XML (ie: process is in library already)
	if(getData() == "")
	{	
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("No WPS process metadata was provided.");
		return knFailedToInit;
	}

	WpsExcuteRequestBuilder requestBuilder(m_wpsMetadata);

	for(int p=0;p<getParameterCount();p++)
	{
		PYXPointer<Parameter> param = getParameter(p);
		PYXPointer<ParameterSpec> paramSpec = getSpec()->getParameter(p);

		//ISRS parameter - nothing to check here...
		if (paramSpec->getInterface() == ISRS::iid)
		{
			continue;
		}

		//IFeature used as geometry or bounding box
		if (paramSpec->getInterface() == IFeature::iid)
		{
			continue;
		}

		//this is an OWS reference input - make sure we can find a valid OWSForamt...
		std::list<PYXPointer<OWSFormat>> formats = m_wpsMetadata->getParamterFormats(p);

		for(int v=0;v<param->getValueCount();v++)
		{
			PYXPointer<IProcess> value = param->getValue(v);

			PYXPointer<IOWSReference> reference = value->getOutput()->QueryInterface<IOWSReference>();

			if (!reference)
			{
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError("Once of the inputs doesn't support IOWSReference.");
				return knFailedToInit;
			}

			PYXPointer<OWSFormat> usedFormat;

			for(std::list<PYXPointer<OWSFormat>>::iterator it = formats.begin();it!= formats.end();++it)
			{
				if (reference->supportOutputType(**it))
				{
					usedFormat = *it;
					break;
				}
			}

			if (!usedFormat)
			{
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError("Couldn't find match mimeType between output and input (input name: " + paramSpec->getName()+ ")");
				return knFailedToInit;
			}

			requestBuilder.addInputAsReference(paramSpec->getName(),reference->getOWSReference(IOWSReference::WpsReference, *usedFormat));
		}
	}

	for(WPSProcessMetadata::AttributesSpecMap::const_iterator it = m_wpsMetadata->getAttributesSpec().begin();
		it != m_wpsMetadata->getAttributesSpec().end();
		++it)
	{
		std::map<std::string,std::string>::iterator valueIt = m_attributesValue.find(it->first);

		if (valueIt != m_attributesValue.end())
		{
			requestBuilder.addInputAsLiteralValue(valueIt->first,valueIt->second);
		}
		else {
			if (it->second->getMinOccurs() > 0)
			{
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError("Mandatory attribute was not set. Attribute name: " + it->second->getIdentifier());
				return knFailedToInit;
			}
		}
	}

	PYXPointer<OWSFormat> formatOutput = m_wpsMetadata->getOutputFormats().front();

	requestBuilder.addReferenceOutput(m_wpsMetadata->getOutputIdentifier(),formatOutput->getMimeType(),formatOutput->getSchema(),getProcDescription());

	std::string newRequestDetails = requestBuilder.getOWSReference();

	createCacheState();

	if (newRequestDetails != m_lastRequestDetails)
	{
		TRACE_INFO("New Request" << newRequestDetails);

		storeNewRequestInCache(newRequestDetails);		
	}

	time_t now = time(0);

	if (m_executeStatus == knNotRequested || 
		m_executeStatus != knExectueCompleted && now-m_lastStatusResponseSent > 2*60 ||
		m_executeStatus == knExectueCompleted && now-m_lastStatusResponseSent > 24*60*60)
	{
		PYXPointer<HttpRequest> request = requestBuilder.getRequest();

		if (! request->getResponse())
		{
			m_executeStatus = knFailedToRequest;
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Failed to send request to WPS server");
			return knFailedToInit;
		}
		std::string response = request->getResponseBody();
		storeNewStatusResponseInCache(response);
		parseRepsonse();
	}

	if (m_executeStatus == knRequestedSuccessfully || m_executeStatus == knExectueCompleted)
	{
		if (getSpec()->providesOutputType(IXYCoverage::iid))
		{
			//create a coverage output
			m_coverageNetworkResource = boost::intrusive_ptr<OwsCoverageNetworkResourceProcess>(new OwsCoverageNetworkResourceProcess());

			std::map<std::string,std::string> attributes;

			attributes["request_uri"] = m_resultUrl;
			attributes["request_method"] = "GET";
			attributes["request_headers"] = "";
			attributes["request_body"] = "";
			attributes["response_mimeType"] = formatOutput->getMimeType();
			attributes["response_schema"] = formatOutput->getSchema();;
			attributes["response_encoding"] = "";

			m_coverageNetworkResource->setAttributes(attributes);

			if (m_coverageNetworkResource->initProc() != knInitialized)
			{
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError("Failed to get result with the following error:" + m_coverageNetworkResource->getInitError()->getError());
				return knFailedToInit;
			}
		}
		else
		{
			//create a feature collection output
			m_vectorNetworkResource = boost::intrusive_ptr<OwsVectorNetworkResourceProcess>(new OwsVectorNetworkResourceProcess());

			std::map<std::string,std::string> attributes;

			attributes["request_uri"] = m_resultUrl;
			attributes["request_method"] = "GET";
			attributes["request_headers"] = "";
			attributes["request_body"] = "";
			attributes["response_mimeType"] = formatOutput->getMimeType();
			attributes["response_schema"] = formatOutput->getSchema();;
			attributes["response_encoding"] = "";

			m_vectorNetworkResource->setAttributes(attributes);

			//check if we have ISRS specified
			if (getParameter(getParameterCount()-1)->getValueCount()>0)
			{
				m_vectorNetworkResource->getParameter(0)->addValue(getParameter(getParameterCount()-1)->getValue(0));
			}

			if (m_vectorNetworkResource->initProc() != knInitialized)
			{
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError("Failed to get result with the following error:" + m_vectorNetworkResource->getInitError()->getError());
				return knFailedToInit;
			}
		}
	}
	else 
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Failed to execute process on WPS server: " + m_wpsProcessError);
		return knFailedToInit;
	}

	return knInitialized;
}

void STDMETHODCALLTYPE WPSProcess::setData(const std::string& strData)
{
	m_wpsMetadata = WPSProcessMetadata::create(strData);

	//create inputs as needed
	initalizeParamertersFromSpec(getSpec());	

	ProcessImpl<WPSProcess>::setData(strData);
}

/*
IPROCESS_SPEC_BEGIN(WPSProcess, "WPS Process", "WMS geospatial data opened by the GDAL library", "Reader",
					IXYCoverage::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END
*/

PYXPointer<ProcessSpec> STDMETHODCALLTYPE WPSProcess::getSpec() const
{
	if (m_wpsMetadata)
	{
		return m_wpsMetadata->getSpec();
	}
	else
	{
		return getSpecStatic();
	}
}

void WPSProcess::createCacheState()
{
	boost::filesystem::path cacheDir = AppServices::getCacheDir("ProcessCache");

	ProcessIdentityCache cache(cacheDir);
	boost::filesystem::path pathCache = cache.getPath(getIdentity(), true);

	if (pathCache.empty())
	{
		PYXTHROW(PYXException, "Could not create cache directory.");
	}

	m_strCacheDir = FileUtils::pathToString(pathCache);

	boost::filesystem::path requestPath = pathCache / kstrRequestFileName;

	if (FileUtils::exists(requestPath))
	{
		std::ifstream in(FileUtils::pathToString(requestPath).c_str());
		std::ostringstream oss;
		oss << in.rdbuf();
		m_lastRequestDetails = oss.str();

		m_lastRequestSent = boost::filesystem::last_write_time(requestPath );
	}
	else
	{
		m_lastRequestDetails = "";
		m_lastRequestSent = 0;
		m_executeStatus = knNotRequested;
	}

	boost::filesystem::path responsePath = pathCache / kstrStatusResponseFileName;

	if (FileUtils::exists(responsePath))
	{
		std::ifstream in(FileUtils::pathToString(responsePath).c_str());
		std::ostringstream oss;
		oss << in.rdbuf();
		m_lastStatusResponseDetails = oss.str();

		m_lastStatusResponseSent = boost::filesystem::last_write_time(requestPath );

		parseRepsonse();
	}
	else 
	{
		m_lastStatusResponseDetails = "";
		m_lastStatusResponseSent = 0;
		m_executeStatus = knNotRequested;
	}

	//TODO: is this code is needed? we should get that from parseReponse...
	
	boost::filesystem::path resultPath = pathCache / kstrResultUrlFileName;

	if (FileUtils::exists(resultPath ))
	{
		std::ifstream in(FileUtils::pathToString(resultPath).c_str());
		std::ostringstream oss;
		oss << in.rdbuf();
		m_resultUrl = oss.str();
	}
	else
	{
		m_resultUrl = "";
	}
}


void WPSProcess::storeNewRequestInCache(const std::string & requestDetails)
{
	boost::filesystem::path pathCache = FileUtils::stringToPath(m_strCacheDir);

	boost::filesystem::path requestPath = pathCache / kstrRequestFileName;

	std::ofstream out(FileUtils::pathToString(requestPath).c_str());

	out << requestDetails;

	boost::filesystem::path responsePath = pathCache / kstrStatusResponseFileName;

	if (FileUtils::exists(responsePath))
	{
		boost::filesystem::remove(responsePath);
	}

	boost::filesystem::path resultPath = pathCache / kstrResultUrlFileName;

	if (FileUtils::exists(resultPath))
	{
		boost::filesystem::remove(resultPath);
	}

	m_lastRequestDetails = requestDetails;
	m_lastRequestSent = time(NULL);
	m_lastStatusResponseDetails = "";
	m_lastStatusResponseSent = 0;
	m_resultUrl = "";
	m_executeStatus = knNotRequested;
}

void WPSProcess::storeNewStatusResponseInCache(const std::string & responseDetails)
{
	boost::filesystem::path pathCache = FileUtils::stringToPath(m_strCacheDir);

	boost::filesystem::path responsePath = pathCache / kstrStatusResponseFileName;

	std::ofstream out(FileUtils::pathToString(responsePath).c_str());

	out << responseDetails;
	
	boost::filesystem::path resultPath = pathCache / kstrResultUrlFileName;

	if (FileUtils::exists(resultPath))
	{
		boost::filesystem::remove(resultPath);
	}
	
	m_lastStatusResponseDetails = responseDetails;
	m_lastStatusResponseSent = time(NULL);	

	m_resultUrl = "";
}

void WPSProcess::storeNewResultUrlInCache(const std::string & url)
{
	boost::filesystem::path pathCache = FileUtils::stringToPath(m_strCacheDir);

	boost::filesystem::path resultPath = pathCache / kstrResultUrlFileName;

	std::ofstream out(FileUtils::pathToString(resultPath).c_str());

	out << url;
	
	m_resultUrl = url;
}

void WPSProcess::parseRepsonse()
{
	if (m_lastStatusResponseDetails == "")
	{
		m_executeStatus = knNotRequested;
		return;
	}

	m_wpsProcessError = "";	

	//TODO: HACK: remove all namespaces to make it easy to parse... lets assume the server is ok... which is have to be true.
	PYXPointer<CSharpXMLDoc> doc = CSharpXMLDoc::createWithoutNamespaces(m_lastStatusResponseDetails);

	if (doc->hasNode("/ExecuteResponse"))
	{
		m_nextStatusUrl = doc->getAttributeValue("/ExecuteResponse","statusLocation");

		if (doc->hasNode("/ExecuteResponse/Status"))
		{
			if (doc->hasNode("/ExecuteResponse/Status/ProcessSucceeded"))
			{
				m_executeStatus = knExectueCompleted;
				
				if (doc->hasNode("/ExecuteResponse/ProcessOutputs/Output/Reference"))
				{
					storeNewResultUrlInCache(doc->getAttributeValue("/ExecuteResponse/ProcessOutputs/Output/Reference","href"));
				} 
				else 
				{
					m_wpsProcessError = "Failed to find Output Reference";
					m_executeStatus = knExectueFailed;
				}
			}

			if (doc->hasNode("/ExecuteResponse/Status/ProcessFailed"))
			{
				m_executeStatus = knExectueFailed;
				m_wpsProcessError = "Process Failed"; //generic error.

				if (doc->hasNode("/ExecuteResponse/Status/ProcessFailed/ExceptionReport"))
				{
					m_executeStatus = knExectueFailed;
					m_wpsProcessError = doc->getNodeText("/ExecuteResponse/Status/ProcessFailed/ExceptionReport/Exception/ExceptionText");

					if (m_wpsProcessError == "")
					{
						m_wpsProcessError = doc->getAttributeValue("/ExecuteResponse/Status/ProcessFailed/ExceptionReport/Exception","exceptionCode");
					}
				}
			}
		}
		return;
	}

	if (doc->hasNode("/ExceptionReport"))
	{
		m_executeStatus = knFailedToRequest;
		m_wpsProcessError = doc->getNodeText("/ExceptionReport/Exception/ExceptionText");

		if (m_wpsProcessError == "")
		{
			m_wpsProcessError = doc->getAttributeValue("/ExceptionReport/Exception","exceptionCode");
		}
		return;
	}

	m_executeStatus = knFailedToRequest;
	m_wpsProcessError = "Process Failed: Got unknown response"; //generic error.
}

////////////////////////////////////////////////////////////////////////////////////////
// WPSProcessMetadata
////////////////////////////////////////////////////////////////////////////////////////

WPSProcessMetadata::WPSProcessMetadata(const std::string & fromString)
{
	PYXPointer<CSharpXMLDoc> doc = CSharpXMLDoc::create(fromString);

	m_server = doc->getNodeText("/WPSProcess/Server");
	m_version = doc->getNodeText("/WPSProcess/Version");
	m_processId = doc->getNodeText("/WPSProcess/Process/Identifier");
	m_processDesc = doc->getNodeText("/WPSProcess/Process/Title");

	m_outputId = doc->getNodeText("/WPSProcess/Output/Identifier");

	std::vector<IID> outputsIID;
	std::vector<PYXPointer<ParameterSpec>> inputSpec;

	int inputCount = doc->getNodesCount("/WPSProcess/Input");
	for(int i=0;i<inputCount;i++)
	{
		std::string inputRef = "/WPSProcess/Input[" + StringUtils::toString(i+1) + "]";
		
		inputSpec.push_back(ParameterSpec::create(strToGuid(doc->getNodeText(inputRef+"/Interface")),
			doc->getAttributeValue<int>(inputRef,"minOccurs",1),
			doc->getAttributeValue<int>(inputRef,"maxOccurs",1),
			doc->getNodeText(inputRef+"/Identifier"),
			doc->getNodeText(inputRef+"/Title")));

		std::list<PYXPointer<OWSFormat>> formats;
		int formatCount = doc->getNodesCount(inputRef+"/Format");
		for(int f=0;f<formatCount;f++)
		{
			std::string formatRef = inputRef+"/Format["+ StringUtils::toString(f+1) + "]";

			formats.push_back(OWSFormat::create(
											doc->getAttributeValue(formatRef,"mimeType"),
											doc->getAttributeValue(formatRef,"schema"),
											doc->getAttributeValue(formatRef,"encoding")));
		}

		m_paramterFormats.push_back(formats);
	}

	int attrCount = doc->getNodesCount("/WPSProcess/Attribute");
	for(int i=0;i<attrCount;i++)
	{
		std::string inputRef = "/WPSProcess/Attribute[" + StringUtils::toString(i+1) + "]";
		
		m_attributes[doc->getNodeText(inputRef+"/Identifier")] = 
			WPSAttributeSpec::create(		
				doc->getNodeText(inputRef+"/Identifier"),
				doc->getNodeText(inputRef+"/Title"),
				doc->getNodeText(inputRef+"/Type"),
				doc->getAttributeValue<int>(inputRef,"minOccurs",1),
				doc->getAttributeValue<int>(inputRef,"maxOccurs",1)
				);
	}

	outputsIID.push_back(IFeatureCollection::iid);
	outputsIID.push_back(IFeature::iid);
	outputsIID.push_back(PYXCOM_IUnknown::iid);

	int outputIIDCount = doc->getNodesCount("/WPSProcess/Output/Interface");
	for(int i=0;i<outputIIDCount;i++)
	{
		std::string iidRef = "/WPSProcess/Output/Interface[" + StringUtils::toString(i+1) + "]";
		outputsIID.push_back(strToGuid(doc->getNodeText(iidRef)));
	}

	int formatCount = doc->getNodesCount("/WPSProcess/Output/Format");
	for(int f=0;f<formatCount;f++)
	{
		std::string formatRef = "/WPSProcess/Output/Format["+ StringUtils::toString(f+1) + "]";

		m_outputFormats.push_back(OWSFormat::create(
										doc->getAttributeValue(formatRef,"mimeType"),
										doc->getAttributeValue(formatRef,"schema"),
										doc->getAttributeValue(formatRef,"encoding")));
	}

	//add srs parameter if needed
	inputSpec.push_back(ParameterSpec::create(ISRS::iid,0,1,"Spatial Reference System", "An external spatial reference system definition."));
	m_paramterFormats.push_back(std::list<PYXPointer<OWSFormat>>());

	m_spec = ProcessSpec::create(WPSProcess::clsid,outputsIID,inputSpec,m_processId,m_processDesc);
	m_spec->setCategory("Hidden"); //don't let the tool box show this process...
}

PYXPointer<ProcessSpec> WPSProcessMetadata::getSpec() const
{
	return m_spec;
}

std::list<std::string> WPSProcessMetadata::getAttributeNames() const
{
	std::list<std::string> result;

	for(AttributesSpecMap::const_iterator it=m_attributes.begin();it!=m_attributes.end();++it)
	{
		result.push_back(it->first);
	}

	return result;
}

PYXPointer<WPSAttributeSpec> WPSProcessMetadata::getAttributeSpec(const std::string & attributeId) const
{
	AttributesSpecMap::const_iterator it = m_attributes.find(attributeId);
	if (it == m_attributes.end())
		return 0;
	return it->second;
}

WPSProcessMetadata::ParameterFormatsList WPSProcessMetadata::getParamterFormats(int paramterIndex) const
{
	std::list<ParameterFormatsList>::const_iterator it = m_paramterFormats.begin();
	std::advance(it,paramterIndex);
	return *it;
}

WPSProcessMetadata::ParameterFormatsList WPSProcessMetadata::getOutputFormats() const
{
	return m_outputFormats;
}