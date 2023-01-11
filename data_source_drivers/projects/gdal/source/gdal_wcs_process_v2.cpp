/******************************************************************************
gdal_wcs_process_v2.cpp

begin      : 10/4/2011 10:43:16 AM
copyright  : (c) 2011 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_GDAL_SOURCE

// local includes
#include "gdal_wcs_process_v2.h"
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/reference_sphere.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/derm/wgs84.h"
#include "pyxis/derm/wgs84_coord_converter.h"
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

// {3C11ACF7-AD4B-4bf5-A7FA-98ADCD454FDC}
PYXCOM_DEFINE_CLSID(GDALWCSProcessV2,
0x3c11acf7, 0xad4b, 0x4bf5, 0xa7, 0xfa, 0x98, 0xad, 0xcd, 0x45, 0x4f, 0xdc);

PYXCOM_CLASS_INTERFACES(GDALWCSProcessV2, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(GDALWCSProcessV2, "WCS Reader", "WCS geospatial data opened by the GDAL library", "Reader",
					ICoverage::iid, IFeatureCollection::iid, IFeature::iid, IOWSReference::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END

// Tester class
Tester<GDALWCSProcessV2> gTester;

// Test method
void GDALWCSProcessV2::test()
{
	TRACE_INFO("Testing GDALWCSProcessV2.");

	boost::intrusive_ptr<IProcess> spProc(new GDALWCSProcessV2);
	TEST_ASSERT(spProc);
	
	GDALWCSProcessV2* pCov = dynamic_cast<GDALWCSProcessV2*>(spProc.get());
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


	//// TODO: Test that the process is read from file correctly.

	// Write the process to file.
	boost::filesystem::path tempPath = AppServices::makeTempFile(".ppl");
	TRACE_TEST("Writing Test GDALWCSProcessV2 to file: " << FileUtils::pathToString(tempPath));
	PipeManager::writeProcessToFile(FileUtils::pathToString(tempPath), spProc);
}
////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

/*! 
Serialize the process to a map of strings.

\return the attribute schema as a string.
*/
std::string GDALWCSProcessV2::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:simpleType name=\"SamplingMethod\">"
			"<xs:restriction base=\"xs:string\">"
				"<xs:enumeration value=\"Nearest Neighbor\" />"
				"<xs:enumeration value=\"Bilinear\" />"
				"<xs:enumeration value=\"Bicubic\" />"
			"</xs:restriction>"
		  "</xs:simpleType>"
		  "<xs:element name=\"GDALWCSProcessV2\">"
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
			  "<xs:element name=\"over_sampling\" type=\"SamplingMethod\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Sampling</friendlyName>"
					"<description>Sampling method for higher resolution DERM cells</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"list_of_bands\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>List of Bands</friendlyName>"
					"<description>List of bands as comma-separated values.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
				"<xs:element name=\"format\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Format</friendlyName>"
					"<description>prefered format type</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"
			  "<xs:element name=\"subset\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Subset</friendlyName>"
					"<description>subset axes to reduce coverage to 2d. format: axis(value),axis(value) </description>"
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
void STDMETHODCALLTYPE GDALWCSProcessV2::setAttributes(
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

	// Get over sampling method
	m_overSampling = "Nearest Neighbor";
	it = mapAttr.find("over_sampling");
	if (it != mapAttr.end())
	{
		m_overSampling = it->second;
	}

	it = mapAttr.find("list_of_bands");
	if (it != mapAttr.end())
	{
		m_strListOfBands = it->second;
	}

	it = mapAttr.find("subset");
	if (it != mapAttr.end())
	{
		std::string subsetString = it->second;

		m_subsets.clear();

		while(!subsetString.empty())
		{
			auto startIndex = subsetString[0] == ',' ? 1 : 0;
			auto nameEnd = subsetString.find('(');
			auto valueEnd = subsetString.find(')');
			
			//fail to parse string, aborting
			if (nameEnd == std::string::npos || valueEnd == std::string::npos)
			{
				break;
			}

			auto key = subsetString.substr(startIndex, nameEnd - startIndex);
			auto value = subsetString.substr(nameEnd + 1, valueEnd - nameEnd - 1);

			m_subsets[key] = value;

			subsetString = subsetString.substr(valueEnd + 1);
		}
	}

	it = mapAttr.find("format");
	if (it != mapAttr.end())
	{
		m_strFormat = it->second;
	}
}

/*!
Get the attributes for this process.
*/
std::map<std::string, std::string> GDALWCSProcessV2::getAttributes() const
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
	mapAttr["over_sampling"] = m_overSampling;
	if (m_strListOfBands != "")
	{
		mapAttr["list_of_bands"] = m_strListOfBands;
	}

	if (!m_subsets.empty())
	{
		std::string subsetString;

		for(auto domain: m_subsets)
		{
			if (!subsetString.empty())
			{
				subsetString += ",";
			}
			subsetString += domain.first + "(" + domain.second + ")";
		}

		mapAttr["subset"] = subsetString;
	}

	if (m_strFormat != "")
	{
		mapAttr["format"] = m_strFormat;
	}

	return mapAttr;
}


/*!
Initialize the process so that it is able to provide data.
*/
IProcess::eInitStatus GDALWCSProcessV2::initImpl()
{
	if (m_initState == knInitialized)
		return knInitialized;

	m_spGeom.reset();
	m_strID = "WCS " + procRefToStr(ProcRef(getProcID(), getProcVersion()));

	boost::intrusive_ptr<GDALXYCoverage> m_wcsCoverage;	
	PYXCOMCreateInstance(GDALXYCoverage::clsid, 0, IXYCoverage::iid, (void**) &m_wcsCoverage);
	
	if  (!m_wcsCoverage)
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
		if (m_strServer != "" && m_strLayer != "")
		{
			// Test for changes		
			std::string strNewXML = buildGdalXml();
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

		strProcessWCSXML = buildGdalXml();	

		bNewProcess = true;
	}

	// Open the WCS datasource.
	try
	{
		// Hack for EOX data servers...
		if ((m_strServer.find("ows.eox.at/") != std::string::npos) ||
			(strProcessWCSXML.find("ows.eox.at:80/") != std::string::npos))
		{
			m_wcsCoverage->setAxisLimit(1024);
		}

		// open with default bands until we properly support WCS bands
		if (!m_wcsCoverage->openAsDefault(strProcessWCSXML, 0))
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

	std::list<PYXPointer<GDALXYCoverage>> overviews;
	std::list<int> overviewsResolution;
	overviews.push_back(m_wcsCoverage);
	overviewsResolution.push_back(m_wcsCoverage->getGeometry()->getCellResolution());

	m_spCovDefn = m_wcsCoverage->getCoverageDefinition();

	while (m_wcsCoverage->hasOverview())
	{
		m_wcsCoverage = m_wcsCoverage->openOverview();
		int resolution = m_wcsCoverage->getGeometry()->getCellResolution();

		if (resolution<overviewsResolution.front())
		{
			overviews.push_front(m_wcsCoverage);
			overviewsResolution.push_front(resolution);	
		}
		if (resolution < 5)
		{
			break;
		}
	}

	std::list<int>::iterator resIt = overviewsResolution.begin();

	m_overviewPipelines = std::vector<boost::intrusive_ptr<IProcess>>(overviews.size());
	m_overviewCoverages = std::vector<boost::intrusive_ptr<ICoverage>>(overviews.size());
	m_overviewCoveragesResolutions = std::vector<int>(overviews.size());

	int index = 0;

	for(std::list<PYXPointer<GDALXYCoverage>>::iterator it=overviews.begin(); it != overviews.end(); ++it, index++,	resIt++)
	{
		boost::intrusive_ptr<IProcess> coveragePipeline;
		if (*it!=overviews.back())
		{
			//this is an overview resolution - do nearest neighbor...
			coveragePipeline = createCoverageFromOverview(*it,"Nearest Neighbor");
		}
		else {
			//this is the real data - then use the over sampling method
			coveragePipeline = createCoverageFromOverview(*it,m_overSampling);
		}

		m_overviewPipelines[index] = coveragePipeline;

		if (coveragePipeline->initProc(true) != knInitialized)
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
			m_spInitError->setError("Error when initalizing WCS overviews pipeline.");
			return knFailedToInit;
		}

		m_overviewCoverages[index] = coveragePipeline->getOutput()->QueryInterface<ICoverage>();
		m_overviewCoveragesResolutions[index] = *resIt;

	}

	// If this process is being created for the first, write it's XML to the data block.
	if(bNewProcess)
	{
		setData(strProcessWCSXML);
	}

	m_spDefn = PYXTableDefinition::create();

	addField("LibraryCategory",PYXFieldDefinition::knContextNone,PYXValue::knString,1,PYXValue(m_strServer));
	addField("Service",PYXFieldDefinition::knContextNone,PYXValue::knString,1,PYXValue("WCS"));
	addField("Version",PYXFieldDefinition::knContextNone,PYXValue::knString,1,PYXValue(m_strVersion));
	addField("Layer",PYXFieldDefinition::knContextNone,PYXValue::knString,1,PYXValue(m_strLayer));

	return knInitialized;
}

std::string GDALWCSProcessV2::buildGdalXml() const 
{
	std::stringstream str;
	str << "<WCS_GDAL>"
		<< "<ServiceURL>" << XMLUtils::toSafeXMLText(m_strServer) << "</ServiceURL>"
		<< "<CoverageName>" << XMLUtils::toSafeXMLText(m_strLayer) << "</CoverageName>"
		<< "<Version>" << XMLUtils::toSafeXMLText(m_strVersion) << "</Version>"
		<< "<Timeout>" << "200" << "</Timeout>";
	if (m_strListOfBands != "" && m_strVersion != "1.0.0" )
	{
		str << "<FieldName>" << XMLUtils::toSafeXMLText(m_strListOfBands) << "</FieldName>";
	}

	if (!m_subsets.empty())
	{
		str << "<Subset>";

		for(auto axis : m_subsets)
		{
			str << "<Axis label=\"" << XMLUtils::toSafeXMLText(axis.first) << "\">" <<  XMLUtils::toSafeXMLText(axis.second) << "</Axis>";
		}

		str << "</Subset>";
	}
	if (!m_strFormat.empty())
	{
		str << "<PreferredFormat>" << XMLUtils::toSafeXMLText(m_strFormat) << "</PreferredFormat>";	
	}
	str << "</WCS_GDAL>";
	return str.str();
}

void GDALWCSProcessV2::createGeometry() const
{
	m_spGeom = m_overviewCoverages.back()->getGeometry();
}

PYXValue STDMETHODCALLTYPE GDALWCSProcessV2::getCoverageValue(	const PYXIcosIndex& index,
															int nFieldIndex	) const
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);
	return getCoverageForResolution(index.getResolution())->getCoverageValue(index,nFieldIndex);
}

PYXPointer<PYXValueTile> GDALWCSProcessV2::getCoverageTile(const PYXTile& tile) const
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);
	return getCoverageForResolution(tile.getCellResolution())->getCoverageTile(tile);
}

PYXPointer<PYXValueTile> STDMETHODCALLTYPE GDALWCSProcessV2::getFieldTile(		const PYXIcosIndex& index,
																		int nRes,
																		int nFieldIndex ) const
{
	boost::recursive_mutex::scoped_lock lock(m_procMutex);
	return getCoverageForResolution(nRes)->getFieldTile(index,nRes,nFieldIndex);
}

PYXCost STDMETHODCALLTYPE GDALWCSProcessV2::getFieldTileCost(	const PYXIcosIndex& index,
													int nRes,
													int nFieldIndex ) const
{
	return getCoverageForResolution(nRes)->getFieldTileCost(index,nRes,nFieldIndex);
}

const boost::intrusive_ptr<ICoverage> & GDALWCSProcessV2::getCoverageForResolution(int resolution) const
{
	int i=m_overviewCoveragesResolutions.size()-1;
	for(;i>0;i--)
	{
		if (m_overviewCoveragesResolutions[i] <= resolution)
			break;
	}

	return m_overviewCoverages[i];
}

boost::intrusive_ptr<IProcess> GDALWCSProcessV2::createCoverageFromOverview(boost::intrusive_ptr<GDALXYCoverage> coverage,const std::string & samplingType)
{
	boost::intrusive_ptr<IProcess> sampler;

	if (samplingType == "Bicubic")
	{
		sampler = PYXCOMCreateInstance<IProcess>(strToGuid("{175AB4E6-BDE1-4c30-9C80-BC9A31E45058}"));
	}
	else if (samplingType == "Bilinear")
	{
		sampler = PYXCOMCreateInstance<IProcess>(strToGuid("{46C8C829-9CFE-469e-93DF-F9688B83BB09}"));
	}
	else //nereast neighbor
	{
		sampler = PYXCOMCreateInstance<IProcess>(strToGuid("{D612004E-FC51-449a-B0D6-1860E59F3B0D}"));
	}

	boost::intrusive_ptr<IProcess> coverageWrapper = GDALCoverageProcessWrapper::create(coverage);

	sampler->getParameter(0)->addValue(coverageWrapper);

	return sampler;
}

boost::intrusive_ptr<IProcess> GDALWCSProcessV2::addResolutionFilter(boost::intrusive_ptr<IProcess> pipeline,int minRes,int maxRes)
{
	boost::intrusive_ptr<IProcess> filter = PYXCOMCreateInstance<IProcess>(strToGuid("{FE0A6DAD-AA1E-4489-9AAB-24564CF39A2C}"));

	std::map<std::string,std::string> attr;
	attr["min_abs_res"] = StringUtils::toString(minRes);
	attr["max_abs_res"] = StringUtils::toString(maxRes);
	filter->setAttributes(attr);
	filter->getParameter(0)->addValue(pipeline);

	return filter;
}

PYXPointer<OWSFormat> GDALWCSProcessV2::getDefaultOutputFormat() const
{
	return OWSFormat::create("application/geotiff");
}

bool GDALWCSProcessV2::supportOutputType(const OWSFormat & format) const
{
	return format.supportMimeType("application/geotiff") || format.supportMimeType("application/tiff");
}

std::string GDALWCSProcessV2::getOWSReference(IOWSReference::ReferenceType referenceType,const OWSFormat & format) const
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

	std::string coverageIdName = "COVERAGEID";
	std::string boundingBox = "BOUNDINGBOX";
	if (version != "2.0.0")
	{
		if (version == "1.0.0")
		{
			coverageIdName = "COVERAGE";
			boundingBox = "BBOX";
		}
		else {
			coverageIdName = "IDENTIFIER";
		}
	}
	
	std::string outputType = "image/tiff";
	
	CSharpFunctionProvider & csharpFunctionProvider = *CSharpFunctionProvider::getCSharpFunctionProvider();
	if (referenceType == IOWSReference::WpsReference)
	{
		std::string getCoverageRequest = server;
		getCoverageRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCoverageRequest,"Service","WCS");
		getCoverageRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCoverageRequest,"Version",version);
		getCoverageRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCoverageRequest,"Request","GetCoverage");
		getCoverageRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCoverageRequest,coverageIdName,coverageId);
		getCoverageRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCoverageRequest,"Format",outputType);
		getCoverageRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCoverageRequest,"MediaType",outputType);

		return "<wps:Reference mimeType=\"" + format.getMimeType() + "\" "
			"xlink:href=\"" + XMLUtils::toSafeXMLText(getCoverageRequest) + "\" "
			"method=\"GET\"/>";
	}
	if (referenceType == IOWSReference::OwsContextReference)
	{
		std::string getCapabilitiesRequest = server;
		getCapabilitiesRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCapabilitiesRequest,"Service","WCS");
		getCapabilitiesRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCapabilitiesRequest,"Version",version);
		getCapabilitiesRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCapabilitiesRequest,"Request","GetCapabilities");

		std::string getCoverageRequest = getCapabilitiesRequest;
		getCoverageRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCoverageRequest,"Request","GetCoverage");
		getCoverageRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCoverageRequest,coverageIdName,coverageId);
		getCoverageRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCoverageRequest,"Format",outputType);
		getCoverageRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCoverageRequest,"MediaType",outputType);

		//We have no sense of screen size, so half HD is a random guess
		getCoverageRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCoverageRequest,"Width","1280");
		getCoverageRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCoverageRequest,"Height","720");

		//Assuming that CSW support WGS84 coordinates... we can add a nice bbox
		//TODO: support other conversions as well
		WGS84CoordConverter convreter;
		PYXRect2DDouble rect1,rect2;
		getGeometry()->getBoundingRects(&convreter,&rect1,&rect2);

		if (!rect1.empty() && rect2.empty())
		{
			if (boundingBox == "BBOX")
			{
				getCoverageRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCoverageRequest,boundingBox,
					StringUtils::toString(rect1.yMin()) + "," + 
					StringUtils::toString(rect1.xMin()) + "," + 
					StringUtils::toString(rect1.yMax()) + "," + 
					StringUtils::toString(rect1.xMax()) );
				getCoverageRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCoverageRequest,"CRS","urn:ogc:def:crs:EPSG::4326");
			}
			else 
			{
				getCoverageRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCoverageRequest,boundingBox,
					StringUtils::toString(rect1.yMin()) + "," + 
					StringUtils::toString(rect1.xMin()) + "," + 
					StringUtils::toString(rect1.yMax()) + "," + 
					StringUtils::toString(rect1.xMax()) + ",urn:ogc:def:crs:EPSG::4326");
			}
		}

		return "<owc:offering code=\"http://www.opengis.net/spec/owc/1.0/req/atom/wcs\">" 
			"<owc:operation method=\"GET\" code=\"GetCapabilities\" href=\""+XMLUtils::toSafeXMLText(getCapabilitiesRequest)+"\" type=\"text/xml\"/>" 
			"<owc:operation method=\"GET\" code=\"GetCoverage\" type=\"" + XMLUtils::toSafeXMLText(outputType) + "\" href=\""+XMLUtils::toSafeXMLText(getCoverageRequest)+"\"/>" 
			"</owc:offering>";
	}

	PYXTHROW(PYXException,"unsupported reference type");
}


// {04D5F518-E694-42c3-AD82-7DCF513DCC1E}
PYXCOM_DEFINE_CLSID(GDALCoverageProcessWrapper,
0x4d5f518, 0xe694, 0x42c3, 0xad, 0x82, 0x7d, 0xcf, 0x51, 0x3d, 0xcc, 0x1e);

PYXCOM_CLASS_INTERFACES(GDALCoverageProcessWrapper, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(GDALCoverageProcessWrapper, "WCS Reader wrapper", "UtilitProcess", "Hidden",
					IXYCoverage::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid)
IPROCESS_SPEC_END


GDALCoverageProcessWrapper::GDALCoverageProcessWrapper(const boost::intrusive_ptr<IXYCoverage> & coverage)
	: m_spXYCoverage(coverage)
{	
}

IProcess::eInitStatus GDALCoverageProcessWrapper::initImpl()
{
	if (m_spXYCoverage)
		return knInitialized;
	else
		return knFailedToInit;
}

std::string GDALCoverageProcessWrapper::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"GDALWCSProcessV2\">"
		  "<xs:complexType>"
			"<xs:sequence>"	
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

/*!
Set the attributes for this process.
*/
void STDMETHODCALLTYPE GDALCoverageProcessWrapper::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{	
}

/*!
Get the attributes for this process.
*/
std::map<std::string, std::string> GDALCoverageProcessWrapper::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	return mapAttr;
}
