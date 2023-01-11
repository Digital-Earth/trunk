/******************************************************************************
gdal_wcs_process.cpp

begin      : 10/4/2007 10:43:16 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_GDAL_SOURCE

// local includes
#include "gdal_wcs_process.h"
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/reference_sphere.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/derm/wgs84.h"
#include "pyxis/geometry/geometry.h"
#include "pyxis/pipe/process.h"
#include "pyxis/procs/srs.h"
#include "pyxis/sampling/xy_utils.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"

// GDAL includes
#include "cpl_string.h"
#include "gdal_priv.h"

// boost includes
#include <boost/filesystem/path.hpp>

// standard includes
#include <algorithm>
#include <cassert>

// #define WCS_LIST_OF_BANDS

// {77E0108B-BD6C-41ae-AD9A-BDD0AA303793}
PYXCOM_DEFINE_CLSID(GDALWCSProcess,
0x77e0108b, 0xbd6c, 0x41ae, 0xad, 0x9a, 0xbd, 0xd0, 0xaa, 0x30, 0x37, 0x93);

PYXCOM_CLASS_INTERFACES(GDALWCSProcess, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(GDALWCSProcess, "WCS Reader (Old)", "WCS geospatial data opened by the GDAL library", "Utility",
					IXYCoverage::iid, IFeatureCollection::iid, IFeature::iid, IOWSReference::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END

// Tester class
Tester<GDALWCSProcess> gTester;

// Test method
void GDALWCSProcess::test()
{
	TRACE_INFO("Testing GDALWCSProcess.");

	boost::intrusive_ptr<IProcess> spProc(new GDALWCSProcess);
	TEST_ASSERT(spProc);
	
	GDALWCSProcess* pCov = dynamic_cast<GDALWCSProcess*>(spProc.get());
	TEST_ASSERT(pCov != 0);

	// Test that the process is created with the correct attributes.
	std::map<std::string, std::string> mapAttr;
	mapAttr.clear();
	std::string server = "http://nsidc.org/cgi-bin/atlas_north?";
	mapAttr["server"] = server;
	std::string layer = "greenland_ice_thickness";
	mapAttr["layer"] = layer;
	spProc->setAttributes(mapAttr);

	std::map<std::string, std::string> mapReturned =
		spProc->getAttributes();

	TEST_ASSERT(mapReturned["server"] == mapAttr["server"]);
	TEST_ASSERT(mapReturned["layer"] == mapAttr["layer"]);

	// ****short circuited for Ticket #197****
	// TODO: when we have a datasource on our own WCS Server, do the test locally.
	//spProc->initProc();
	//// Build the XML to how it should be.
	//std::stringstream str;
	//str << "<WCS_GDAL>"
	//	<< "<ServiceURL>http://nsidc.org/cgi-bin/atlas_north?</ServiceURL>"
	//	<< "<CoverageName>greenland_ice_thickness</CoverageName>"
	//	<< "</WCS_GDAL>";
	//std::string strProcessWCSXML;
	//str >> strProcessWCSXML;

	//// Test that WCS XML was created properly.
	//TEST_ASSERT(strProcessWCSXML.compare(spProc->getData()) == 0);


	//Enable this test to test why specific dataset failed to import
	/*
	std::string wcs = "<WCS_GDAL><ServiceURL>http://geobrain.laits.gmu.edu/cgi-bin/ows8/wcseo</ServiceURL><CoverageName>TRMM_Daily_Accumulated_Precipitation_Year_2000</CoverageName><Version>2.0</Version><Timeout>200</Timeout></WCS_GDAL>";

	boost::intrusive_ptr<GDALXYCoverage> spXYCoverage = 0;
	PYXCOMCreateInstance(GDALXYCoverage::clsid, 0, IXYCoverage::iid, (void**) &spXYCoverage );

	TEST_ASSERT(spXYCoverage->open(wcs,0));
	*/

	//// TODO: Test that the process is read from file correctly.

	// Write the process to file.
	boost::filesystem::path tempPath = AppServices::makeTempFile(".ppl");
	TRACE_TEST("Writing Test GDALWCSProcess to file: " << FileUtils::pathToString(tempPath));
	PipeManager::writeProcessToFile(FileUtils::pathToString(tempPath), spProc);
}
////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

/*! 
Serialize the process to a map of strings.

\return the attribute schema as a string.
*/
std::string GDALWCSProcess::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"GDALWCSProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"server\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Server</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"layer\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Layer</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"wcs_version\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Version</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
#ifdef WCS_LIST_OF_BANDS
			  "<xs:element name=\"list_of_bands\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>List of Bands</friendlyName>"
					"<description>List of bands as comma-separated values.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"			  
#endif
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

/*!
Set the attributes for this process.
*/
void STDMETHODCALLTYPE GDALWCSProcess::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{
	m_initState = knNeedsInit;
	std::map<std::string, std::string>::const_iterator it;

	// Get the server URL attribute.
	it = mapAttr.find("server");
	if (it != mapAttr.end())
	{
		m_strServer = it->second;
	}

	// Get the server version attribute.
	it = mapAttr.find("wcs_version");
	if (it != mapAttr.end())
	{
		m_strVersion = it->second;
	} 
	else 
	{
		m_strVersion = "2.0.0";
	}

	// Get the layer (datasource name) attribute.
	it = mapAttr.find("layer");
	if (it != mapAttr.end())
	{
		m_strLayer = it->second;
	}

#ifdef WCS_LIST_OF_BANDS	
	it = mapAttr.find("list_of_bands");
	if (it != mapAttr.end())
	{
		m_strListOfBands = it->second.c_str();
	}
#endif	
}

/*!
Get the attributes for this process.
*/
std::map<std::string, std::string> GDALWCSProcess::getAttributes() const
{
	if (m_strServer == "" || m_strLayer == "" && getData() != "")
	{
		PYXPointer<CSharpXMLDoc> xml = CSharpXMLDoc::create(getData());

		std::string server = xml->getNodeText("/WCS_GDAL/ServiceURL");
		std::string coverageId = xml->getNodeText("/WCS_GDAL/CoverageName");
		std::string version = xml->getNodeText("/WCS_GDAL/Version");
	}

	std::map<std::string, std::string> mapAttr;
	mapAttr["server"] = m_strServer;
	mapAttr["layer"] = m_strLayer;
	mapAttr["wcs_version"] = m_strVersion;
#ifdef WCS_LIST_OF_BANDS
	mapAttr["list_of_bands"] = m_strListOfBands;
#endif

	return mapAttr;
}


/*!
Initialize the process so that it is able to provide data.
*/
IProcess::eInitStatus GDALWCSProcess::initImpl()
{
	m_spXYCoverage = 0;
	PYXCOMCreateInstance(GDALXYCoverage::clsid, 0, IXYCoverage::iid, (void**) &m_spXYCoverage);
	
	if  (!m_spXYCoverage)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("Unable to create instance of GDAL XY coverage. Fatal Error.");
		return knFailedToInit;
	}

	// Holds the XML for the WCS process.
	std::string strProcessWCSXML;

	// Indicates if this is a new process or not (ie: coming from library).
	bool bNewProcess = false;

	// Check if the data block has been set to the WCS XML (ie: process is in library already)
	if(getData() != "")
	{
		// If the data block has already been set, check if changes have been made.
		strProcessWCSXML = getData();

		// Case for importing PPL: m_str[Server,Layer] are both still ""
		if(m_strServer != "" && m_strLayer != "")
		{
			// Test for changes		
			std::string strNewXML;
			std::stringstream str;
			str << "<WCS_GDAL>"
				<< "<ServiceURL>" << m_strServer << "</ServiceURL>"
				<< "<CoverageName>" << m_strLayer << "</CoverageName>"
				<< "<Version>" << m_strVersion << "</Version>"
				<< "<Timeout>" << "200" << "</Timeout>";
				
#ifdef WCS_LIST_OF_BANDS
			if(m_strListOfBands != "")
			{
				str << "<GetCoverageExtra>&RangeSubset=" << m_strListOfBands << "</GetCoverageExtra>";
			}
#endif

			str << "</WCS_GDAL>";
			str >> strNewXML;			

			if(getData().compare(strNewXML) != 0)
			{				
				strProcessWCSXML = strNewXML;
				bNewProcess = true;
			}
		}
	}
	else
	{
		// Check to make sure that both server and layer are set.
		if(m_strServer == "" || m_strLayer == "")
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Server and/or layer not set for WCS process.");
			return knFailedToInit;
		}

		// Build our XML for the WCS request.
		std::stringstream str;
		str << "<WCS_GDAL>"
			<< "<ServiceURL>" << m_strServer << "</ServiceURL>"
			<< "<CoverageName>" << m_strLayer << "</CoverageName>"
			<< "<Version>" << m_strVersion << "</Version>"
			<< "<Timeout>" << "200" << "</Timeout>";

#ifdef WCS_LIST_OF_BANDS

		if(m_strListOfBands != "")
		{
			str << "<GetCoverageExtra>&RangeSubset=" << m_strListOfBands << "</GetCoverageExtra>";
		}

#endif

		str << "</WCS_GDAL>";
		str >> strProcessWCSXML;		

		bNewProcess = true;
	}

	// Open the WCS datasource.
	try
	{
		// Hack for EOX data servers...
		if ((m_strServer.find("ows.eox.at/") != std::string::npos) ||
			(strProcessWCSXML.find("ows.eox.at:80/") != std::string::npos))
		{
			m_spXYCoverage->setAxisLimit(1024);
		}

		if (!m_spXYCoverage->openAsRGB(strProcessWCSXML, 0))
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("GDAL is unable to open the WCS datasource: '" + strProcessWCSXML + "'.");
			return knFailedToInit;
		}
	}
	catch (PYXException &ex)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("GDAL is unable to open the WCS datasource: '" + ex.getFullErrorString() + "'.");
		return knFailedToInit;
	}
	catch (...)
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("GDAL is unable to open the WCS datasource due to an unknown exception.");
		return knFailedToInit;
	}

	// If this process is being created for the first, write it's XML to the data block.
	if(bNewProcess)
	{
		setData(strProcessWCSXML);
	}
	return knInitialized;
}

PYXPointer<OWSFormat> GDALWCSProcess::getDefaultOutputFormat() const
{
	return OWSFormat::create("application/geotiff");
}

bool GDALWCSProcess::supportOutputType(const OWSFormat & format) const
{
	return format.supportMimeType("application/geotiff") || format.supportMimeType("application/tiff");
}

std::string GDALWCSProcess::getOWSReference(IOWSReference::ReferenceType referenceType,const OWSFormat & format) const
{
	if (!supportOutputType(format))
	{
		PYXTHROW(PYXException, "mimeType and schema is not supported");
	}

	PYXPointer<CSharpXMLDoc> xml = CSharpXMLDoc::create(getData());

	std::string server = xml->getNodeText("/WCS_GDAL/ServiceURL");
	std::string coverageId = xml->getNodeText("/WCS_GDAL/CoverageName");
	std::string version = xml->getNodeText("/WCS_GDAL/Version");

	if (server.size() == 0)
	{
		PYXTHROW(PYXException, "Server was not set");
	}
	if (server.find('?') == std::string::npos)
	{
		server += "?";
	}
	else if (server[server.size()-1] != '&' && server[server.size()-1] != '?')
	{
		server += "&";
	}

	if (coverageId.size() == 0)
	{
		PYXTHROW(PYXException, "CoverageId was not set");
	}

	if (version.size() == 0 )
	{
		version = "2.0.0";
	}

	std::string outputType = "image/tiff";

	if (referenceType == IOWSReference::WpsReference)
	{
		return "<wps:Reference mimeType=\"" + format.getMimeType() + "\" "
			"xlink:href=\""+ XMLUtils::toSafeXMLText(server) + "SERVICE=WCS&amp;VERSION=" + version + "&amp;REQUEST=GetCoverage&amp;COVERAGEID=" + XMLUtils::toSafeXMLText(coverageId) + "&amp;FORMAT=" + outputType + "&amp;MEDIATYPE=" + outputType + "\" "
			"method=\"GET\"/>";
	}
	if (referenceType == IOWSReference::OwsContextReference)
	{
		PYXTHROW_NOT_IMPLEMENTED();
	}
	PYXTHROW(PYXException,"unsupported reference type");
}