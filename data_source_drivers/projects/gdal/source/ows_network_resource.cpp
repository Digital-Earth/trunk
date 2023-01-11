/******************************************************************************
ows_network_resource.h

begin      : 9/01/2011 9:57:18 AM
copyright  : (c) 2011 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_GDAL_SOURCE

// local includes
#include "ows_network_resource.h"
#include "gdal_pipe_builder.h"
#include "ogr_pipe_builder.h"

#include "pyxis/utility/http_utils.h"
#include "pyxis/pipe/process_identity_cache.h"
#include "pyxis/utility/checksum_calculator.h"

// {9C99E17D-7E87-4D3C-9970-4058F18C674F}
PYXCOM_DEFINE_CLSID(OwsCoverageNetworkResourceProcess, 
0x9c99e17d, 0x7e87, 0x4d3c, 0x99, 0x70, 0x40, 0x58, 0xf1, 0x8c, 0x67, 0x4f);
PYXCOM_CLASS_INTERFACES(OwsCoverageNetworkResourceProcess, IProcess::iid, IXYCoverage::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(OwsCoverageNetworkResourceProcess, "OWS Coverage Network Resource", "Coverage that can be downloaded from the network", "Reader",
					IXYCoverage::iid, IOWSReference::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END

// Tester class
Tester<OwsCoverageNetworkResourceProcess> gTester;

// Test method
void OwsCoverageNetworkResourceProcess::test()
{
	// TODO: add unit tests.
}


////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

/*! 
Serialize the process to a map of strings.

\return the attribute schema as a string.
*/
std::string OwsCoverageNetworkResourceProcess::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"OwsCoverageNetworkResourceProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"request_uri\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Request Url</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"request_method\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Request Method</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"request_body\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Request Body</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"request_headers\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Request Headers</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"response_mimeType\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Response MimeT ype</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"response_schema\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Response Schema</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"response_encoding\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Response Encoding</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"			  
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

/*!
Set the attributes for this process.
*/
void STDMETHODCALLTYPE OwsCoverageNetworkResourceProcess::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{
	m_initState = knNeedsInit;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"request_uri",m_requestUrl);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"request_method",m_method);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"request_body",m_body);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"request_headers",m_headers);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"response_mimeType",m_responseMimeType);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"response_schema",m_responseSchema);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"response_encoding",m_responseEncoding);
}

/*!
Get the attributes for this process.
*/
std::map<std::string, std::string> OwsCoverageNetworkResourceProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr["request_uri"] = m_requestUrl;
	mapAttr["request_method"] = m_method;
	mapAttr["request_body"] = m_body;
	mapAttr["request_headers"] = m_headers;
	mapAttr["response_mimeType"] = m_responseMimeType;
	mapAttr["response_schema"] = m_responseSchema;
	mapAttr["response_encoding"] = m_responseEncoding;
	
	return mapAttr;
}


boost::filesystem::path OwsCoverageNetworkResourceProcess::getLocalFile()
{
	boost::filesystem::path cacheDir = AppServices::getCacheDir("ProcessCache");

	ProcessIdentityCache cache(cacheDir);
	boost::filesystem::path pathCache = cache.getPath(getIdentity(), true);

	if (pathCache.empty())
	{
		PYXTHROW(PYXException, "Could not create cache directory.");
	}

	boost::filesystem::path localFilePath = pathCache / getFileNameFromMimeType();

	return localFilePath;
}

std::string OwsCoverageNetworkResourceProcess::getFileNameFromMimeType()
{
	if (m_responseMimeType == "image/tiff" ||
		m_responseMimeType == "image/geotiff" ||
		m_responseMimeType == "application/x-geotiff" ||
		m_responseMimeType == "application/geotiff")
	{
		return "result.tiff";
	}

	PYXTHROW(PYXException, "Could not detect file type from mime type");
}

bool OwsCoverageNetworkResourceProcess::downloadNetworkResource(const std::string & localFile)
{
	PYXPointer<HttpRequest> request = HttpRequest::create(m_requestUrl,m_method);

	if (m_body != "")
	{
		request->addRequestBody(m_body);
	}

	if (m_headers != "")
	{
		std::map<std::string,std::string> headers = getRequestHeaders();

		for(std::map<std::string,std::string>::iterator it = headers.begin();it != headers.end(); ++it)
		{
			request->addRequestHeader(it->first,it->second);
		}
	}

	return request->downloadResponse(localFile);
}

std::map<std::string,std::string> OwsCoverageNetworkResourceProcess::getRequestHeaders() const
{
	std::map<std::string,std::string> result;

	std::string left = m_headers;

	while(left.size()>0)
	{
		std::string header;
		size_t pos = left.find(';');
		if (pos == std::string::npos)
		{
			header = left;
			left.clear();
		}
		else {
			header = left.substr(0,pos);
			left = left.substr(pos+1);
		}

		pos = header.find(':');
		
		result.insert(std::make_pair(header.substr(0,pos),header.substr(pos+1)));
	}

	return result;
}

boost::intrusive_ptr<IProcess> OwsCoverageNetworkResourceProcess::createGdalProcess(const boost::filesystem::path& path) 
{
	boost::intrusive_ptr<GDALFileProcess> spGDalProc (new GDALFileProcess);

	boost::intrusive_ptr<IProcess> spPathProc; 
	PYXCOMCreateInstance(PathProcess::clsid, 0, IProcess::iid, (void**) &spPathProc);
	std::map<std::string, std::string> mapAttr;
	mapAttr["uri"] = FileUtils::pathToString(path);
	spPathProc->setAttributes(mapAttr);
	spPathProc->setProcName("File Process: " + FileUtils::pathToString(path.leaf()));

	spGDalProc->getParameter(0)->addValue(spPathProc);
	spGDalProc->setProcName(spGDalProc->getProcName() + " " + FileUtils::pathToString(path.leaf()));	

	return spGDalProc;
}

bool OwsCoverageNetworkResourceProcess::buildLocalPipeline()
{
	setData(""); //the local resource checksum
	m_spXYCoverage.reset();

	boost::filesystem::path localFile = getLocalFile();

	if (FileUtils::exists(localFile))
	{
		try
		{
			m_readerProcess = createGdalProcess(localFile);

			if (m_readerProcess->initProc(true) == IProcess::knInitialized)
			{
				m_spXYCoverage = m_readerProcess->getOutput()->QueryInterface<IXYCoverage>();
			}
		}
		catch (PYXException &ex)
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("GDAL is unable to open the downloaded network resouce: '" + ex.getFullErrorString() + "'.");
			return false;
		}
		catch (...)
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("GDAL is unable to open the downloaded network resouce due to an unknown exception.");
			return false;
		}
	}
	if (!m_spXYCoverage) 
	{
		if (downloadNetworkResource(FileUtils::pathToString(localFile)))
		{
			try
			{
				m_readerProcess = createGdalProcess(localFile);

				if (m_readerProcess->initProc(true) == IProcess::knInitialized)
				{
					m_spXYCoverage = m_readerProcess->getOutput()->QueryInterface<IXYCoverage>();
				}
			}
			catch (PYXException &ex)
			{
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError("GDAL is unable to open the downloaded network resouce: '" + ex.getFullErrorString() + "'.");
				return false;
			}
			catch (...)
			{
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError("GDAL is unable to open the downloaded network resouce due to an unknown exception.");
				return false;
			}
		} else {
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("failed to downloand network resouce.");
			return false;
		}
	}

	//if we manage to open the file, it's time to create a check sum for it (to make this pipeline unique on the file content and not the url we are requesting)
	if (m_spXYCoverage)
	{
		//ask for the check sum to be calculated.
		std::string manifest = CSharpFunctionProvider::getCSharpFunctionProvider()->getSerializedManifestForFile(FileUtils::pathToString(localFile));

		//if we got a check sum - add it to the identity as the process data.
		if (manifest.size()>0)
		{
			setData(CSharpFunctionProvider::getCSharpFunctionProvider()->getIdentity(manifest));
		}
	}

	return m_spXYCoverage;
}

/*!
Initialize the process so that it is able to provide data.
*/
IProcess::eInitStatus OwsCoverageNetworkResourceProcess::initImpl()
{
	if (m_requestUrl.empty())
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Can't initalize process because request url was not set.");
		return knFailedToInit;
	}

	if (m_method.empty())
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Can't initalize process because request method was not set.");
		return knFailedToInit;
	}

	if (m_responseMimeType.empty() && m_responseSchema.empty() )
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Can't initalize process because mimeType or schema was not set.");
		return knFailedToInit;
	}

	if (buildLocalPipeline())
	{
		return knInitialized;
	}
	else 
	{
		return knFailedToInit;
	}
}

PYXPointer<OWSFormat> OwsCoverageNetworkResourceProcess::getDefaultOutputFormat() const
{
	return OWSFormat::create(m_responseMimeType,m_responseSchema);
}

bool OwsCoverageNetworkResourceProcess::supportOutputType(const OWSFormat & format) const
{
	if (format.getMimeType() != "" && format.getMimeType() != m_responseMimeType)
		return false;

	if (format.getSchema() != "" && format.getSchema() != m_responseSchema)
		return false;

	return true;
}

std::string OwsCoverageNetworkResourceProcess::getOWSReference(IOWSReference::ReferenceType referenceType,const OWSFormat & format) const
{
	if (!supportOutputType(format))
	{
		PYXTHROW(PYXException, "mimeType and schema is not supported");
	}

	if (referenceType == IOWSReference::WpsReference)
	{
		std::string reference = "<wps:Reference ";

		if (m_responseSchema != "")
		{
			reference += "schema=\"" + m_responseSchema + "\" ";
		}

		if (m_responseMimeType != "")
		{
			reference += "mimeType=\"" + m_responseMimeType + "\" ";
		}

		if (m_responseEncoding != "")
		{
			reference += "encoding=\"" + m_responseEncoding + "\" ";
		}

		reference += "xlink:href=\""+ XMLUtils::toSafeXMLText(m_requestUrl) + "\" ";
		reference += "method=\""+ m_method + "\">";

		if (m_headers != "")
		{
			std::map<std::string,std::string> headers = getRequestHeaders();

			for(std::map<std::string,std::string>::iterator it = headers.begin();it != headers.end(); ++it)
			{
				reference += "<wps:Header key=\"" + XMLUtils::toSafeXMLText(it->first) + "\" value=\"" + XMLUtils::toSafeXMLText(it->second) + "\" />";
			}
		}

		if (m_body != "")
		{
			reference += "<wps:Body>" + m_body + "</wps:Body>";
		}

		reference += "</wps:Reference>";

		return reference ;
	}
	if (referenceType == IOWSReference::OwsContextReference)
	{
		PYXTHROW_NOT_IMPLEMENTED();
	}
	PYXTHROW(PYXException,"unsupported reference type");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OwsVectorNetworkResourceProcess
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// {428B2314-B45E-4B5D-B863-D69FD0C8A145}
PYXCOM_DEFINE_CLSID(OwsVectorNetworkResourceProcess, 
0x428b2314, 0xb45e, 0x4b5d, 0xb8, 0x63, 0xd6, 0x9f, 0xd0, 0xc8, 0xa1, 0x45);
PYXCOM_CLASS_INTERFACES(OwsVectorNetworkResourceProcess, IProcess::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(OwsVectorNetworkResourceProcess, "OWS Feature Network Resource", "Features collection that can be downloaded from the network", "Reader",
					IOWSReference::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ISRS::iid, 0, 1, "Spatial Reference System", "An external spatial reference system definition.")
IPROCESS_SPEC_END

// Tester class
Tester<OwsVectorNetworkResourceProcess> gVectorTester;

// Test method
void OwsVectorNetworkResourceProcess::test()
{
	// TODO: add unit tests.
}


////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

/*! 
Serialize the process to a map of strings.

\return the attribute schema as a string.
*/
std::string OwsVectorNetworkResourceProcess::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"OwsVectorNetworkResourceProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"request_uri\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Request Url</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"request_method\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Request Method</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"request_body\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Request Body</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"request_headers\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Request Headers</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"response_mimeType\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Response MimeT ype</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"response_schema\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Response Schema</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"response_encoding\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Response Encoding</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

/*!
Set the attributes for this process.
*/
void STDMETHODCALLTYPE OwsVectorNetworkResourceProcess::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{
	m_initState = knNeedsInit;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"request_uri",m_requestUrl);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"request_method",m_method);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"request_body",m_body);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"request_headers",m_headers);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"response_mimeType",m_responseMimeType);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"response_schema",m_responseSchema);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"response_encoding",m_responseEncoding);
}

/*!
Get the attributes for this process.
*/
std::map<std::string, std::string> OwsVectorNetworkResourceProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr["request_uri"] = m_requestUrl;
	mapAttr["request_method"] = m_method;
	mapAttr["request_body"] = m_body;
	mapAttr["request_headers"] = m_headers;
	mapAttr["response_mimeType"] = m_responseMimeType;
	mapAttr["response_schema"] = m_responseSchema;
	mapAttr["response_encoding"] = m_responseEncoding;

	return mapAttr;
}


boost::filesystem::path OwsVectorNetworkResourceProcess::getLocalFile()
{
	boost::filesystem::path cacheDir = AppServices::getCacheDir("ProcessCache");

	ProcessIdentityCache cache(cacheDir);
	boost::filesystem::path pathCache = cache.getPath(getIdentity(), true);

	if (pathCache.empty())
	{
		PYXTHROW(PYXException, "Could not create cache directory.");
	}

	boost::filesystem::path localFilePath = pathCache / getFileNameFromMimeType();

	return localFilePath;
}

std::string OwsVectorNetworkResourceProcess::getFileNameFromMimeType()
{
	if (m_responseMimeType.find("kml") != std::string::npos ||
		m_responseMimeType.find("KML") != std::string::npos ||
		m_responseSchema.find("kml") != std::string::npos ||
		m_responseSchema.find("KML") != std::string::npos)
	{
		return "result.kml";
	}

	if (m_responseMimeType.find("gml") != std::string::npos ||
		m_responseMimeType.find("GML") != std::string::npos ||
		m_responseSchema.find("gml") != std::string::npos ||
		m_responseSchema.find("GML") != std::string::npos)
	{
		return "result.gml";
	}

	PYXTHROW(PYXException, "Could not detect file type from mime type");
}

bool OwsVectorNetworkResourceProcess::downloadNetworkResource(const std::string & localFile)
{
	PYXPointer<HttpRequest> request = HttpRequest::create(m_requestUrl,m_method);

	if (m_body != "")
	{
		request->addRequestBody(m_body);
	}

	if (m_headers != "")
	{
		std::map<std::string,std::string> headers = getRequestHeaders();

		for(std::map<std::string,std::string>::iterator it = headers.begin();it != headers.end(); ++it)
		{
			request->addRequestHeader(it->first,it->second);
		}
	}

	return request->downloadResponse(localFile);
}

std::map<std::string,std::string> OwsVectorNetworkResourceProcess::getRequestHeaders() const
{
	std::map<std::string,std::string> result;

	std::string left = m_headers;

	while(left.size()>0)
	{
		std::string header;
		size_t pos = left.find(';');
		if (pos == std::string::npos)
		{
			header = left;
			left.clear();
		}
		else {
			header = left.substr(0,pos);
			left = left.substr(pos+1);
		}

		pos = header.find(':');

		result.insert(std::make_pair(header.substr(0,pos),header.substr(pos+1)));
	}

	return result;
}

boost::intrusive_ptr<IProcess> OwsVectorNetworkResourceProcess::createOgrProcess(const boost::filesystem::path& path) 
{
	boost::intrusive_ptr<OGRProcess> spOgrProc (new OGRProcess);

	boost::intrusive_ptr<IProcess> spPathProc; 
	PYXCOMCreateInstance(PathProcess::clsid, 0, IProcess::iid, (void**) &spPathProc);
	std::map<std::string, std::string> mapAttr;
	mapAttr["uri"] = FileUtils::pathToString(path);
	spPathProc->setAttributes(mapAttr);
	spPathProc->setProcName("File Process: " + FileUtils::pathToString(path.leaf()));

	spOgrProc->getParameter(0)->addValue(spPathProc);
	spOgrProc->setProcName(spOgrProc->getProcName() + " " + FileUtils::pathToString(path.leaf()));
	if (getParameter(0)->getValueCount()>0)
	{
		spOgrProc->getParameter(2)->addValue(getParameter(0)->getValue(0));
	}

	return spOgrProc;
}

bool OwsVectorNetworkResourceProcess::buildLocalPipeline()
{
	setData(""); //the local resource checksum
	m_spFC.reset();

	boost::filesystem::path localFile = getLocalFile();

	//step1: download the file
	if (!FileUtils::exists(localFile))
	{
		if (!downloadNetworkResource(FileUtils::pathToString(localFile)))
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("failed to downland network resource.");
			return false;
		}

		//make sure the file was downloaded...
		if (!FileUtils::exists(localFile))
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("failed to downland network resource.");
			return false;
		}
	}

	//step2: try to init the file
	try
	{
		m_readerProcess = createOgrProcess(localFile);

		if (m_readerProcess->initProc(true) != IProcess::knInitialized)
		{
			//we failed to init the process - check the reason why? 
			if (PipeUtils::findFirstError(m_readerProcess, guidToStr(GDALSRSInitError::clsid)))
			{
				//ogr process failed due to missing SRS, if so, pass it on...
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GDALSRSInitError());
			}
			else 
			{
				//some other error
				m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
				m_spInitError->setError(m_readerProcess->getInitError()->getError());
			}
			return false;
		}

		m_spFC = m_readerProcess->getOutput()->QueryInterface<IFeatureCollection>();

		if (!m_spFC)
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Failed to get IFeatureCollection output.");
			return false;
		}	
	}
	catch (PYXException &ex)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("OGR is unable to open the downloaded network resouce: '" + ex.getFullErrorString() + "'.");
		return false;
	}
	catch (...)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("OGR is unable to open the downloaded network resouce due to an unknown exception.");
		return false;
	}

	//Step3: make a unique indentity depend on the file content
	//if we manage to open the file, it's time to create a check sum for it (to make this pipeline unique on the file content and not the url we are requesting)
	if (m_spFC)
	{
		//ask for the check sum to be calculated.
		std::string manifest = CSharpFunctionProvider::getCSharpFunctionProvider()->getSerializedManifestForFile(FileUtils::pathToString(localFile));

		//if we got a check sum - add it to the identity as the process data.
		if (manifest.size()>0)
		{
			setData(CSharpFunctionProvider::getCSharpFunctionProvider()->getIdentity(manifest));
		}
	}

	return m_spFC;
}

/*!
Initialize the process so that it is able to provide data.
*/
IProcess::eInitStatus OwsVectorNetworkResourceProcess::initImpl()
{
	if (m_requestUrl.empty())
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Can't initalize process because request url was not set.");
		return knFailedToInit;
	}

	if (m_method.empty())
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Can't initalize process because request method was not set.");
		return knFailedToInit;
	}

	if (m_responseMimeType.empty() && m_responseSchema.empty() )
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Can't initalize process because mimeType or schema was not set.");
		return knFailedToInit;
	}

	if (buildLocalPipeline())
	{
		return knInitialized;
	}
	else 
	{
		return knFailedToInit;
	}
}

PYXPointer<OWSFormat> OwsVectorNetworkResourceProcess::getDefaultOutputFormat() const
{
	return OWSFormat::create(m_responseMimeType,m_responseSchema);
}

bool OwsVectorNetworkResourceProcess::supportOutputType(const OWSFormat & format) const
{
	if (format.getMimeType() != "" && format.getMimeType() != m_responseMimeType)
		return false;

	if (format.getSchema() != "" && format.getSchema() != m_responseSchema)
		return false;

	return true;
}

std::string OwsVectorNetworkResourceProcess::getOWSReference(IOWSReference::ReferenceType referenceType,const OWSFormat & format) const
{
	if (!supportOutputType(format))
	{
		PYXTHROW(PYXException, "mimeType and schema is not supported");
	}

	if (referenceType == IOWSReference::WpsReference)
	{
		std::string reference = "<wps:Reference ";

		if (m_responseSchema != "")
		{
			reference += "schema=\"" + m_responseSchema + "\" ";
		}

		if (m_responseMimeType != "")
		{
			reference += "mimeType=\"" + m_responseMimeType + "\" ";
		}

		if (m_responseEncoding != "")
		{
			reference += "encoding=\"" + m_responseEncoding + "\" ";
		}

		reference += "xlink:href=\""+ XMLUtils::toSafeXMLText(m_requestUrl) + "\" ";
		reference += "method=\""+ m_method + "\">";

		if (m_headers != "")
		{
			std::map<std::string,std::string> headers = getRequestHeaders();

			for(std::map<std::string,std::string>::iterator it = headers.begin();it != headers.end(); ++it)
			{
				reference += "<wps:Header key=\"" + XMLUtils::toSafeXMLText(it->first) + "\" value=\"" + XMLUtils::toSafeXMLText(it->second) + "\" />";
			}
		}

		if (m_body != "")
		{
			reference += "<wps:Body>" + m_body + "</wps:Body>";
		}

		reference += "</wps:Reference>";

		return reference;
	}
	if (referenceType == IOWSReference::OwsContextReference)
	{
		PYXTHROW_NOT_IMPLEMENTED();
	}
	PYXTHROW(PYXException,"unsupported reference type");
}