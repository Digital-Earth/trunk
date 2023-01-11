/******************************************************************************
ogr_feature_server_process.h

begin      : 2016-02-09
copyright  : (c) 2011 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_GDAL_SOURCE

// local includes
#include "ogr_feature_server_process.h"

#include "pyxis/procs/path.h"
#include "pyxis/utility/http_utils.h"
#include "pyxis/utility/xml_transform.h"
#include "pyxis/procs/user_credentials_provider.h"
#include "pyxis/utility/ssl_utils.h"

// {88C80CEE-5EBE-4472-8E5F-FF0D6DD0A935}
PYXCOM_DEFINE_CLSID(OGRFeatureServerProcess, 
0x88c80cee, 0x5ebe, 0x4472, 0x8e, 0x5f, 0xff, 0x0d, 0x6d, 0xd0, 0xa9, 0x35);

PYXCOM_CLASS_INTERFACES(OGRFeatureServerProcess, IProcess::iid, IGeoServicesReference::iid, IGeoServicesFeatureServerReference::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(OGRFeatureServerProcess, "GeoServices Feature Server Reader", "reading features from a GeoServices feature server.", "Reader",
					IGeoServicesReference::iid, IGeoServicesFeatureServerReference::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ISRS::iid, 0, 1, "Spatial Reference System", "An external spatial reference system definition.")
	IPROCESS_SPEC_PARAMETER(IUserCredentialsProvider::iid, 0, 1, "User Credentials", "User credentials to use.")
IPROCESS_SPEC_END

std::map<std::string, std::string> STDMETHODCALLTYPE OGRFeatureServerProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	if (m_strUri != "")
	{
		mapAttr["uri"] = m_strUri;
	}

	return mapAttr;
}

std::string STDMETHODCALLTYPE OGRFeatureServerProcess::getAttributeSchema() const
{
	return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"OGRFeatureServerProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"uri\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>GeoServices FeatureServer URI</friendlyName>"
					"<description>GeoServices FeatureServer URI</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

void STDMETHODCALLTYPE OGRFeatureServerProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	m_initState = knNeedsInit;
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr, "uri", m_strUri);
}



IProcess::eInitStatus OGRFeatureServerProcess::initImpl()
{
	// TODO: reset state
	// ...

	if (m_strUri.empty())
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Could not create GeoServices FeatureServer reader because uri was not defined");
		return knFailedToInit;
	}

	if (m_initState == knInitialized)
	{
		return knInitialized;
	}

	// if they just try to init the same feature server, it is good to keep a reference for the old process, so OGR doesn't reopen the feature server
	boost::intrusive_ptr<IProcess> oldProcess = m_ogrProcess;

	PYXPointer<UrlProcess> url = new UrlProcess();
	url->setUrl(m_strUri);
	url->initProc();

	m_ogrProcess = new OGRProcess();

	m_ogrProcess->getParameter(0)->addValue(url);
		
	std::map<std::string, std::string> mapAttr;

	mapAttr["layer_name"] = ""; // layer name is included in URI
	mapAttr["res"] = "24";		// arbitrary resolution picked for default
	mapAttr["canRasterize"]= "false"; // we want to show Icons, so no we can't rasterize
	mapAttr["axis_flip"] = "0";	// no axis flipping for FeatureServers

	m_ogrProcess->setAttributes(mapAttr);
	
	// if we were provided a SRS, pass it to the OGR process to use
	if (getParameter(0)->getValueCount() > 0)
	{
		m_ogrProcess->getParameter(2)->addValue(getParameter(0)->getValue(0));
	}

	//if we were provided credentials, pass it to the OGR process to use
	if (getParameter(1)->getValueCount() > 0)
	{
		m_ogrProcess->getParameter(3)->addValue(getParameter(1)->getValue(0));

		// TODO: this is wrong, this should come from the m_ogrProcess->getIdentity()...
		// this code is a code clone from OgrProcess...
		boost::intrusive_ptr<IUserCredentials> credentials =
			getParameter(1)->getValue(0)->getOutput()->QueryInterface<IUserCredentialsProvider>()->
				getCredentials(CSharpFunctionProvider::getCSharpFunctionProvider()->getUrlHost(m_strUri),IUsernameAndPasswordCredentials::iid);

		if (credentials)
		{
			credentials = credentials->findFirstCredentialOfType(IUsernameAndPasswordCredentials::iid);

			if (credentials)
			{
				SSLUtils::Checksum checksum("SHA256");
				checksum.generate(credentials->QueryInterface<IUsernameAndPasswordCredentials>()->getUsername()+ "@" + m_strUri); // there is not need to store the password because it could change over time.
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
		// pass long missing credentials error
		boost::intrusive_ptr<const UserCredentialsInitError> error = boost::dynamic_pointer_cast<const UserCredentialsInitError>(
			PipeUtils::findFirstError(m_ogrProcess, guidToStr(UserCredentialsInitError::clsid))->getInitError());

		if (error)
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new UserCredentialsInitError(error->getNeededCredentialsType(), error->getCredentialsTarget()));
		}
	}
	else if (PipeUtils::findFirstError(m_ogrProcess, guidToStr(MissingGeometryInitError::clsid)))
	{
		// pass along missing geometry error
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
