/******************************************************************************
ogr_wfs_process.h

begin      : 9/01/2011 9:57:18 AM
copyright  : (c) 2011 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_GDAL_SOURCE

// local includes
#include "ogr_wfs_process.h"

#include "pyxis/procs/path.h"
#include "pyxis/utility/http_utils.h"
#include "pyxis/utility/xml_transform.h"
#include "pyxis/procs/user_credentials_provider.h"
#include "pyxis/utility/ssl_utils.h"

// {AA47A7D3-6749-4bd4-99B4-9B4B6CF3EF9C}
PYXCOM_DEFINE_CLSID(OGRWFSProcess, 
0xaa47a7d3, 0x6749, 0x4bd4, 0x99, 0xb4, 0x9b, 0x4b, 0x6c, 0xf3, 0xef, 0x9c);

PYXCOM_CLASS_INTERFACES(OGRWFSProcess, IProcess::iid, IOWSReference::iid, IWFSReference::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(OGRWFSProcess, "WFS Reader", "reading features from a WFS server.", "Reader",
					IOWSReference::iid, IWFSReference::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ISRS::iid, 0, 1, "Spatial Reference System", "An external spatial reference system definition.")
	IPROCESS_SPEC_PARAMETER(IUserCredentialsProvider::iid, 0, 1, "User Credentials", "User credentials to use.")
IPROCESS_SPEC_END

std::map<std::string, std::string> STDMETHODCALLTYPE OGRWFSProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	if (m_serverUri != "")
	{
		mapAttr["server"] = m_serverUri;
	}

	if (m_sLayerName != "")
	{
		mapAttr["layer_name"] = m_sLayerName;
	}

	if (m_axisFlip)
	{
		mapAttr["axis_flip"] = "1";
	}
	
	return mapAttr;
}

std::string STDMETHODCALLTYPE OGRWFSProcess::getAttributeSchema() const
{
	return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"OGRWFSProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"server\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>WFS Server uri</friendlyName>"
					"<description>WFS server uri</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"layer_name\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Layer Name</friendlyName>"
					"<description>OGR Layer Name to read from data source</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"axis_flip\" type=\"xs:boolean\" default=\"" + StringUtils::toString(m_axisFlip?1:0) + "\">"
			    "<xs:annotation>"
			      "<xs:appinfo>"
			        "<friendlyName>Axis Flip</friendlyName>"
			        "<description></description>"
			      "</xs:appinfo>"
			    "</xs:annotation>"
			  "</xs:element>"
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

void STDMETHODCALLTYPE OGRWFSProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	m_initState = knNeedsInit;
	std::map<std::string, std::string>::const_iterator it;

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"server",m_serverUri);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"layer_name",m_sLayerName);

	std::string axis_flip_value;
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"axis_flip",axis_flip_value);
	m_axisFlip = axis_flip_value=="bool 1" || axis_flip_value=="1" || axis_flip_value=="true";
}



IProcess::eInitStatus OGRWFSProcess::initImpl()
{
	// TODO: reset state
	// ...

	if (m_serverUri == "")
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Could not create WFS reader because server was not defined");
		return knFailedToInit;
	}

	if (m_sLayerName == "")
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Could not create WFS reader because layer was not defined");
		return knFailedToInit;
	}

	if (m_initState == knInitialized)
	{
		return knInitialized;
	}

	//Node, if they just try to init the same WFS, it good to keep a reference for the old process, so OGR should not reopen the WFS
	boost::intrusive_ptr<IProcess> oldProcess = m_ogrProcess;


	/*	
	PYXPointer<HttpRequest> request = HttpRequest::create(m_serverUri + "service=wfs&version=1.1.0&request=GetFeature&typeName="+m_sLayerName+"&outputFormat=SHAPE-ZIP","get");

	boost::filesystem::path tempFile = AppServices::makeTempFile(".zip");

	if (request->downloadResponse(FileUtils::pathToString(tempFile)))
	{
		//need to extract the zip files before we can use this - not fun. skiping...
		boost::intrusive_ptr<IProcess> spPathProc; 
		PYXCOMCreateInstance(PathProcess::clsid, 0, IProcess::iid, (void**) &spPathProc);
		std::map<std::string, std::string> mapAttr;
		mapAttr["uri"] = FileUtils::pathToString(tempFile);
		spPathProc->setAttributes(mapAttr);
		spPathProc->initProc();

		m_ogrProcess = new OGRProcess();

		m_ogrProcess->getParameter(0)->addValue(spPathProc);

		mapAttr.clear();		
		mapAttr["res"] = "24"; //Arbitrary Res picked for default.
		mapAttr["canRasterize"]= "false"; // We want to show Icons, so no we can't rasterize.

		m_ogrProcess->setAttributes(mapAttr);
	}
	else {
	*/
		m_sId = "WFS:"+m_sLayerName + "@" + m_serverUri;

		PYXPointer<UrlProcess> url = new UrlProcess();
		url->setUrl(m_serverUri);
		url->initProc();

		m_ogrProcess = new OGRProcess();

		m_ogrProcess->getParameter(0)->addValue(url);
		
		std::map<std::string, std::string> mapAttr;

		mapAttr["layer_name"] = m_sLayerName; //later name
		mapAttr["res"] = "24"; //Arbitrary Res picked for default.
		mapAttr["canRasterize"]= "false"; // We want to show Icons, so no we can't rasterize.
		mapAttr["axis_flip"] = m_axisFlip?"1":"0";

		m_ogrProcess->setAttributes(mapAttr);
	//}
	
	//if we were provided a SRS, pass it to the OGR process to use
	if (getParameter(0)->getValueCount()>0)
	{
		m_ogrProcess->getParameter(2)->addValue(getParameter(0)->getValue(0));
	}

	//if we were provided credentials, pass it to the OGR process to use
	if (getParameter(1)->getValueCount()>0)
	{
		m_ogrProcess->getParameter(3)->addValue(getParameter(1)->getValue(0));

		//TODO: this is wrong, this should come from the m_ogrProcess->getIdentity()...
		//this code is a code clone from OgrProcess...
		boost::intrusive_ptr<IUserCredentials> credentials =
			getParameter(1)->getValue(0)->getOutput()->QueryInterface<IUserCredentialsProvider>()->
				getCredentials(CSharpFunctionProvider::getCSharpFunctionProvider()->getUrlHost(m_serverUri),IUsernameAndPasswordCredentials::iid);

		if (credentials)
		{
			credentials = credentials->findFirstCredentialOfType(IUsernameAndPasswordCredentials::iid);

			if (credentials)
			{
				SSLUtils::Checksum checksum("SHA256");
				checksum.generate(credentials->QueryInterface<IUsernameAndPasswordCredentials>()->getUsername()+ "@" + m_serverUri); //there is not need to store the password because it could change over time.
				setData(checksum.toHexString());
			}
			else
			{
				setData("");
			}
		}
		else
		{
			setData("");
		}
	}
	else
	{
		setData("");
	}

	eInitStatus status = m_ogrProcess->initProc();

	if (status == knInitialized)
	{
		m_outputAsFeatureCollection = m_ogrProcess->getOutput()->QueryInterface<IFeatureCollection>();
		m_outputAsFeature = m_ogrProcess->getOutput()->QueryInterface<IFeature>();
		m_spDefn = m_outputAsFeature->getDefinition();
		m_vecValues = m_outputAsFeature->getFieldValues();
	}
	else if (PipeUtils::findFirstError(m_ogrProcess, guidToStr(GDALSRSInitError::clsid)))
	{
		// pass along missing SRS error
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GDALSRSInitError());
	}
	else if (PipeUtils::findFirstError(m_ogrProcess, guidToStr(UserCredentialsInitError::clsid)))
	{
		// pass along missing user credentials error
		boost::intrusive_ptr<const UserCredentialsInitError> error = boost::dynamic_pointer_cast<const UserCredentialsInitError>(
			PipeUtils::findFirstError(m_ogrProcess, guidToStr(UserCredentialsInitError::clsid))->getInitError());

		if (error)
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new UserCredentialsInitError(error->getNeededCredentialsType(),error->getCredentialsTarget()));
		}
	}
	else if (PipeUtils::findFirstError(m_ogrProcess, guidToStr(MissingGeometryInitError::clsid)))
	{
		// pass along missing or empty geometry error
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new MissingGeometryInitError());
		const boost::intrusive_ptr<const IProcessInitError> spError = m_ogrProcess->getInitError();
		if (spError)
		{
			m_spInitError->setError(spError->getError());
		}
	}
	else if (PipeUtils::findFirstError(m_ogrProcess, guidToStr(GenericProcInitError::clsid)))
	{
		// pass along generic error
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		const boost::intrusive_ptr<const IProcessInitError> spError = m_ogrProcess->getInitError();
		if (spError)
		{
			m_spInitError->setError(spError->getError());
		}	
	}

	return status;
}

void OGRWFSProcess::getSupportedOutputs() const
{
	if (m_supportedFormats.size()>0)
	{
		return;
	}

	try
	{
		std::string getCapabilitiesRequest = m_serverUri;
		getCapabilitiesRequest = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(getCapabilitiesRequest,"Service","WFS");
		getCapabilitiesRequest = CSharpFunctionProvider::getCSharpFunctionProvider()->setDefaultValueForUrlQueryParameter(getCapabilitiesRequest,"Version","1.1.0");
		getCapabilitiesRequest = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(getCapabilitiesRequest,"Request","GetCapabilities");

		PYXPointer<HttpRequest> request = HttpRequest::create(getCapabilitiesRequest,"GET");

		request->getResponse();

		PYXPointer<CSharpXMLDoc> doc = CSharpXMLDoc::createWithoutNamespaces(request->getResponseBody());

		std::string outputXpath = "/WFS_Capabilities/OperationsMetadata/Operation[@name = \"GetFeature\"]/Parameter[@name = \"outputFormat\"]/Value";
		int outputCount = doc->getNodesCount(outputXpath);
		for(int i=0;i<outputCount;i++)
		{
			std::string itemXpath = outputXpath + "[" + StringUtils::toString(i+1) + "]";

			std::string mimeType = doc->getNodeText(itemXpath);

			PYXPointer<OWSFormat> wellKnownFormat = OWSFormat::createFromWellKnownMimeType(mimeType);

			if (wellKnownFormat)
			{
				m_supportedFormats.push_back(wellKnownFormat);
			}
		}
	}
	catch(...)
	{
		TRACE_ERROR("Failed to get supported formats for WFS server: " << m_serverUri);
	}

	if (m_supportedFormats.size()==0)
	{
		TRACE_INFO("Found no supported format for WFS server: " << m_serverUri << ", using GML3 as default");

		m_supportedFormats.push_back(OWSFormat::createFromWellKnownMimeType("GML3"));
		return;
	}

}

PYXPointer<OWSFormat> OGRWFSProcess::getDefaultOutputFormat() const
{
	getSupportedOutputs();
	return m_supportedFormats.front()->clone();
}

bool OGRWFSProcess::supportOutputType(const OWSFormat & format) const
{
	getSupportedOutputs();

	for(std::list<PYXPointer<OWSFormat>>::iterator it = m_supportedFormats.begin(); it != m_supportedFormats.end(); ++it)
	{
		if ((**it).supportSchema(format.getSchema()))
		{
			return true;
		}
	}	
	if (format.getMimeType() == "application/WFS")
	{
		return true;
	}

	return false;
}

std::string OGRWFSProcess::getOWSReference(IOWSReference::ReferenceType referenceType,const OWSFormat & format) const 
{
	if (!supportOutputType(format))
		PYXTHROW(PYXException, "mimeType and schema is not supported");

	std::string outputType;
	std::string usedSchema;

	if (format.supportMimeType("application/WFS"))
	{
		getSupportedOutputs();

		outputType = m_supportedFormats.front()->getMimeType();
		usedSchema = m_supportedFormats.front()->getSchema();
	}
	else
	{
		outputType = format.getMimeType();
		usedSchema = format.getSchema();
	}

	CSharpFunctionProvider & csharpFunctionProvider = *CSharpFunctionProvider::getCSharpFunctionProvider();
	if (referenceType == IOWSReference::WpsReference)
	{
		std::string getFeatureRequest = m_serverUri;
		getFeatureRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getFeatureRequest,"Service","WFS");
		getFeatureRequest = csharpFunctionProvider.setDefaultValueForUrlQueryParameter(getFeatureRequest,"Version","1.1.0");
		getFeatureRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getFeatureRequest,"Request","GetFeature");
		getFeatureRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getFeatureRequest,"TypeName",m_sLayerName);
		getFeatureRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getFeatureRequest,"OutputFormat",outputType);

		return "<wps:Reference schema=\""+ usedSchema + "\" "
			"xlink:href=\"" + XMLUtils::toSafeXMLText(getFeatureRequest) + "\" "
			"method=\"GET\"/>";
	}
	if (referenceType == IOWSReference::OwsContextReference)
	{
		std::string getCapabilitiesRequest = m_serverUri;
		getCapabilitiesRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCapabilitiesRequest,"Service","WFS");
		getCapabilitiesRequest = csharpFunctionProvider.setDefaultValueForUrlQueryParameter(getCapabilitiesRequest,"Version","1.1.0");
		getCapabilitiesRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCapabilitiesRequest,"Request","GetCapabilities");

		std::string getFeatureRequest = getCapabilitiesRequest;
		getFeatureRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getFeatureRequest,"Request","GetFeature");
		getFeatureRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getFeatureRequest,"TypeName",m_sLayerName);
		getFeatureRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getFeatureRequest,"OutputFormat",outputType);

		return "<owc:offering code=\"http://www.opengis.net/spec/owc/1.0/req/atom/wfs\">"
			"<owc:operation method=\"GET\" code=\"GetCapabilities\" href=\""+XMLUtils::toSafeXMLText(getCapabilitiesRequest)+"\" type=\"text/xml\"/>"
			"<owc:operation method=\"GET\" code=\"GetFeature\" href=\""+XMLUtils::toSafeXMLText(getFeatureRequest)+"\"/>"
			"</owc:offering>";
	}
	PYXTHROW(PYXException,"unsupported reference type");
}
