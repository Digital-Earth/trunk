/******************************************************************************
ogr_process.cpp

begin		: 2007-06-14
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_GDAL_SOURCE
#include "ogr_process.h"

#include "exceptions.h"
#include "pyx_shared_gdal_data_set.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/reference_sphere.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/derm/wgs84.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/pipe/process.h"
#include "pyxis/pipe/process_local_storage.h"
#include "pyxis/procs/path.h"
#include "pyxis/procs/srs.h"
#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/sampling/xy_utils.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/ssl_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"
#include "pyxis/utility/ssl_utils.h"
#include "pyxis/pipe/process_identity_cache.h"

// windows includes
#include <direct.h>

// ogr includes
#include "ogrsf_frmts.h"

// gdal includes
#include "cpl_string.h"
#include "gdal_priv.h"

// boost includes
#include <boost/filesystem/path.hpp>
#include <boost/algorithm/string/predicate.hpp>

// standard includes
#include <algorithm>
#include <cassert>

// {C621458A-9E1D-41eb-B01E-C0569743C0B8}
PYXCOM_DEFINE_CLSID(OGRProcess, 
0xc621458a, 0x9e1d, 0x41eb, 0xb0, 0x1e, 0xc0, 0x56, 0x97, 0x43, 0xc0, 0xb8);
PYXCOM_CLASS_INTERFACES(OGRProcess, IProcess::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(OGRProcess, "OGR File Reader", "A feature extension to the GDAL library.", "Reader",
					IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IUrl::iid, 1, 1, "Geospatial File Path", "The path of the file to open");
	IPROCESS_SPEC_PARAMETER(IPath::iid, 0, 1, "Style Path", "The path of the style specification file");
	IPROCESS_SPEC_PARAMETER(ISRS::iid, 0, 1, "Spatial Reference System", "An external spatial reference system definition.")
	IPROCESS_SPEC_PARAMETER(IUserCredentialsProvider::iid, 0, 1, "User Credentials", "User credentials to use.")
IPROCESS_SPEC_END

int OGRProcess::knUninitializedRes = -1;

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::map<std::string, std::string> STDMETHODCALLTYPE OGRProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	if (m_nRes != -1)
	{
		mapAttr["res"] = StringUtils::toString(m_nRes);
	}
	if (!m_strStyle.empty())
	{
		mapAttr["style"] = m_strStyle;
	}
	mapAttr["canRasterize"] = m_bCanRasterize ? "true" : "false";

	if (m_nLayerIndex != 0)
	{
		mapAttr["layer"] = StringUtils::toString(m_nLayerIndex);
	}

	if (m_strLayerName != "")
	{
		mapAttr["layer_name"] = m_strLayerName;
	}

	if (m_axisFlip)
	{
		mapAttr["axis_flip"] = "1";
	}
	
	return mapAttr;
}

std::string STDMETHODCALLTYPE OGRProcess::getAttributeSchema() const
{
	return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"OGRProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"res\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Resolution</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"			  
			  "<xs:element name=\"layer\" type=\"xs:int\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Layer Index</friendlyName>"
					"<description>OGR Layer Index to read from data source</description>"
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

void STDMETHODCALLTYPE OGRProcess::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{
	m_initState = knNeedsInit;
	std::map<std::string, std::string>::const_iterator it;

	m_nRes = -1;
	UPDATE_PROCESS_ATTRIBUTE(mapAttr,"res", int, m_nRes);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr, "style", m_strStyle);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr, "layer_name", m_strLayerName);
	UPDATE_PROCESS_ATTRIBUTE(mapAttr, "layer", int, m_nLayerIndex);

	std::string axis_flip_value;
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr, "axis_flip", axis_flip_value);
	m_axisFlip = axis_flip_value=="bool 1" || axis_flip_value=="1" || axis_flip_value=="true";

	it = mapAttr.find("canRasterize");
	if (it != mapAttr.end())
	{
		std::string strRast = it->second;

		if (strRast.compare("true") == 0)
		{
			m_bCanRasterize = true;
		}
		else
		{
			m_bCanRasterize = false;
		}
		
	}
}

IProcess::eInitStatus OGRProcess::initImpl()
{
	// TODO: reset state
	// ...

	// Get the file process.
	assert (getParameter(0)->getValueCount() && getParameter(0)->getValue(0)->getOutput());
	boost::intrusive_ptr<IUrl> spUri = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IUrl>();
	boost::intrusive_ptr<IPath> spPath = spUri->QueryInterface<IPath>();

	assert(spPath || spUri);
	m_strUrl.clear();	
	if (spPath)
	{
		m_path = FileUtils::stringToPath(spPath->getLocallyResolvedPath());
	}
	else 
	{
		m_strUrl = spUri->getUrl();
	}

	// get the style path
	if (getParameter(1)->getValueCount() && getParameter(1)->getValue(0)->getOutput())
	{
		spPath = getParameter(1)->getValue(0)->getOutput()->QueryInterface<IPath>();
		if (!spPath)
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Could not retrieve style path of the OGR File process.");
			return knFailedToInit;
		}
		m_styleURI = FileUtils::stringToPath(spPath->getLocallyResolvedPath());
	}
	else
	{
		m_styleURI = boost::filesystem::path();
	}

	// initialize the Conversion object (get the SRS if one was provided)
	boost::intrusive_ptr<ISRS> spSRS;
	if (getParameter(2)->getValueCount() == 1)
	{
		assert(getParameter(2)->getValue(0)->getOutput());
		spSRS = getParameter(2)->getValue(0)->getOutput()->QueryInterface<ISRS>();
	}

	m_credentials.reset();

	if (getParameter(3)->getValueCount() == 1 && !m_strUrl.empty())
	{
		boost::intrusive_ptr<IUserCredentials> credentials =
			getParameter(3)->getValue(0)->getOutput()->QueryInterface<IUserCredentialsProvider>()->
			getCredentials(CSharpFunctionProvider::getCSharpFunctionProvider()->getUrlHost(m_strUrl), IUsernameAndPasswordCredentials::iid);

		if (credentials)
		{
			credentials = credentials->findFirstCredentialOfType(IUsernameAndPasswordCredentials::iid);

			if (credentials)
			{
				m_credentials = credentials->QueryInterface<IUsernameAndPasswordCredentials>();
			}
		}
	}

	try
	{
		open(spSRS);
	}
	catch (MissingGeometryException&)
	{
		m_spInitError = new MissingGeometryInitError();
		return knFailedToInit;	
	}
	catch (MissingSRSException&)
	{
		m_spInitError = new GDALSRSInitError();
		return knFailedToInit;	
	}
	catch (MissingUserCredentialsException&)
	{
		m_spInitError = new UserCredentialsInitError(IUsernameAndPasswordCredentials::iid, CSharpFunctionProvider::getCSharpFunctionProvider()->getUrlHost(m_strUrl));
		return knFailedToInit;	
	}
	catch (PYXException& e)
	{
		m_spInitError = new GenericProcInitError();
		m_spInitError->setError(e.getFullErrorString());
		return knFailedToInit;
	}
	return knInitialized;
}

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

std::string OGRProcess::buildDatasetURIforWFS()
{
	PYXPointer<CSharpXMLDoc> wfsDoc = CSharpXMLDoc::create("<OGRWFSDataSource />");

	wfsDoc->addChild("OGRWFSDataSource","URL");
	wfsDoc->setNodeText("OGRWFSDataSource/URL", m_strUrl);

	if (m_credentials)
	{
		wfsDoc->addChild("OGRWFSDataSource","UserPwd");
		wfsDoc->setNodeText("OGRWFSDataSource/UserPwd", m_credentials->getUsername() + ":" + m_credentials->getPassword());
		wfsDoc->addChild("OGRWFSDataSource","HttpAuth");
		wfsDoc->setNodeText("OGRWFSDataSource/HttpAuth","BASIC");

		SSLUtils::Checksum checksum("SHA256");
		checksum.generate(m_credentials->getUsername()+ "@" + m_strUrl); //there is no need to store the password because it could change over time.
		setData(checksum.toHexString());
	}
	else
	{
		setData("");
	}

	boost::filesystem::path cacheDir = AppServices::getCacheDir("ProcessCache");
	const ProcessIdentityCache cache(cacheDir);
	boost::filesystem::path processPath = cache.getPath(getIdentity(),true);

	boost::filesystem::path path = processPath / "server.wfs";

	wfsDoc->saveToFile(FileUtils::pathToString(path));

	return FileUtils::pathToString(path);
}

void OGRProcess::open(boost::intrusive_ptr<ISRS> spSRS)
{	
	std::string strDataSourceURI;
	
	if (m_strUrl.empty())
	{
		strDataSourceURI = FileUtils::pathToString(m_path);
	}
	else 
	{
		if ((m_strUrl.find("/rest/services") != std::string::npos) &&
			(m_strUrl.find("service=WFS") == std::string::npos))
		{
			// this is a GeoServices URL. Pass as-is to GDAL.
			strDataSourceURI = m_strUrl;
		}
		else
		{
			// this is an OGC Web Feature Service URL
			strDataSourceURI = buildDatasetURIforWFS();
		}
	}
	
	// lock OGR context
	{
		boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

		try
		{
			// get the data source, if one was already created for this data source it would be shared
			auto pOGRDataSource = PYXSharedGDALDataSet::createVector(strDataSourceURI);

			m_spDS = new PYXOGRDataSource();
			if (!m_strUrl.empty())
			{
				//TOOD: this is not how it should be done, it should happen inside the PYXOGRDataSource;
				m_spDS->m_addExtentToMetadata = false;
			}
			m_spDS->open(pOGRDataSource, spSRS, m_axisFlip, m_nLayerIndex, m_strLayerName, PYXProcessLocalStorage::create(getIdentity()));
			m_spDS->setResolution(m_nRes);

			// TODO[kabiraman]: remove this hack
			if (!m_strStyle.empty())
			{
				m_spDS->setStyle(m_strStyle);
			}		
			int nCellRes = m_spDS->getGeometry()->getCellResolution();
			m_nRes = nCellRes > PYXMath::knMaxAbsResolution ? PYXMath::knMaxAbsResolution : nCellRes;
		}
		catch (MissingUserCredentialsException& e)
		{
			if (!m_strUrl.empty())
			{
				PYXRETHROW(e, MissingUserCredentialsException, "Unable to open web server due to missing or invalid user credentials: " << m_strUrl);
			}
			else
			{
				PYXRETHROW(e, MissingUserCredentialsException, "Unable to open data source due to missing or invalid user credentials: " << m_path);
			}
		}
		catch (MissingSRSException & e)
		{
			PYXRETHROW(e, MissingSRSException, "Unable to open data source: " << strDataSourceURI << " due to missing SRS");
		}
		catch (PYXException& e)
		{
			PYXRETHROW(e, GDALOGRException, "Unable to open data source: " << strDataSourceURI);
		}
	}
}

bool OGRProcess::close()
{
	// TODO
	return true;
}
