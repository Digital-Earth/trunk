/******************************************************************************
gdal_wms_process.cpp

begin      : 1/25/2008 9:57:00 AM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_GDAL_SOURCE

// local includes
#include "gdal_wms_process.h"
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
#include "pyxis/sampling/xy_bounds_geometry.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/value.h"
#include "pyxis/geometry/bounding_rects_calculator.h"
#include "pyxis/utility/http_utils.h"
#include "pyxis/utility/bitmap_server_provider.h"

// GDAL includes
#include "cpl_string.h"
#include "gdal_priv.h"

// boost includes
#include <boost/filesystem/path.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <boost/algorithm/string.hpp>
#include "pyxis/procs/user_credentials_provider.h"

// standard includes
#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>

// {F5E595F7-B58B-4d23-AC2B-865829306E10}
PYXCOM_DEFINE_CLSID(GDALWMSProcess,
					0xf5e595f7, 0xb58b, 0x4d23, 0xac, 0x2b, 0x86, 0x58, 0x29, 0x30, 0x6e, 0x10);
PYXCOM_CLASS_INTERFACES(GDALWMSProcess, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(GDALWMSProcess, "WMS Reader", "WMS geospatial data opened by the GDAL library", "Reader",
					IXYCoverage::iid, IOWSReference::iid,  IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid)
					IPROCESS_SPEC_PARAMETER(IUserCredentialsProvider::iid, 0, 1, "User Credentials", "User credentials to use.")
					IPROCESS_SPEC_END

//! The default resolution for geographic coordinate systems
const double GDALWMSProcess::kfDefaultGeographicResolutionInDegrees = 0.00004;

//! The default PYXIS resolution when calculating units/pixel
const int GDALWMSProcess::knDefaultPyxisResolution = 12;

//! The maximum number of pixels in the width or height dimension for an overview
const int GDALWMSProcess::knMaxOverviewWidthOrHeightPixels = 500000000;

					// Tester class
					Tester<GDALWMSProcess> gTester;

// Test method
void GDALWMSProcess::test()
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
std::string GDALWMSProcess::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
		"elementFormDefault=\"qualified\" "
		"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		"<xs:element name=\"GDALWMSProcess\">"
		"<xs:complexType>"
		"<xs:sequence>"
		"<xs:element name=\"serviceName\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Service Name</friendlyName>"
		"<description></description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"
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
		"<xs:element name=\"format\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Image format</friendlyName>"
		"<description>image/jepg;image/png</description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"
		"<xs:element name=\"srs\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Spatial reference system</friendlyName>"
		"<description></description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"
		"<xs:element name=\"styles\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Styles</friendlyName>"
		"<description></description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"
		"<xs:element name=\"minRes\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Minimum Resolution</friendlyName>"
		"<description></description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"
		"<xs:element name=\"maxRes\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Maximum Resolution</friendlyName>"
		"<description></description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"
		"<xs:element name=\"minLat\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Minimum Latitude</friendlyName>"
		"<description></description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"
		"<xs:element name=\"maxLat\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Maximum Latitude</friendlyName>"
		"<description></description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"
		"<xs:element name=\"minLon\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Minimum Longitude</friendlyName>"
		"<description></description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"
		"<xs:element name=\"maxLon\" type=\"xs:string\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Maximum Longitude</friendlyName>"
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
void STDMETHODCALLTYPE GDALWMSProcess::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{
	m_initState = knNeedsInit;
	std::map<std::string, std::string>::const_iterator it;

	// Get the service name attribute
	it = mapAttr.find("serviceName");
	if (it != mapAttr.end())
	{
		m_strServiceName = it->second;
	}
	else
	{
		// for backwards compatibility
		m_strServiceName = "WMS";
	}

	// Get the server URL attribute.
	it = mapAttr.find("server");
	if (it != mapAttr.end())
	{
		m_strServer = it->second;
	}

	// Get the layer (datasource name) attribute.
	it = mapAttr.find("layer");
	if (it != mapAttr.end())
	{
		m_strLayer = it->second;
	}

	it = mapAttr.find("format");
	if (it != mapAttr.end())
	{
		m_strFormat = it->second;
	}

	it = mapAttr.find("srs");
	if (it != mapAttr.end())
	{
		if (it->second[0] == '{')
		{
			// SRS is in JSON format, do not change to uppercase
			m_strSrs = it->second;
		}
		else
		{
			m_strSrs = boost::to_upper_copy(it->second);
		}
	}

	it = mapAttr.find("styles");
	if (it != mapAttr.end())
	{
		m_strStyles = it->second;
	}

	it = mapAttr.find("minRes");
	if (it != mapAttr.end())
	{
		m_strMinResolution = it->second;
	}

	it = mapAttr.find("maxRes");
	if (it != mapAttr.end())
	{
		m_strMaxResolution = it->second;
	}

	it = mapAttr.find("minLat");
	if (it != mapAttr.end())
	{
		m_strMinLat = it->second.c_str();
	}

	it = mapAttr.find("maxLat");
	if (it != mapAttr.end())
	{
		m_strMaxLat = it->second.c_str();
	}

	it = mapAttr.find("minLon");
	if (it != mapAttr.end())
	{
		m_strMinLon = it->second.c_str();
	}

	it = mapAttr.find("maxLon");
	if (it != mapAttr.end())
	{
		m_strMaxLon = it->second.c_str();
	}
}

/*!
Get the attributes for this process.
*/
std::map<std::string, std::string> GDALWMSProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	// for WMS we do not write out the service name to preserve the identity of old pipelines
	if (m_strServiceName != "WMS")
	{
		mapAttr["serviceName"] = m_strServiceName;
	}
	mapAttr["server"] = m_strServer;
	mapAttr["layer"] = m_strLayer;
	mapAttr["format"] = m_strFormat;
	mapAttr["srs"] = m_strSrs;
	mapAttr["styles"] = m_strStyles;
	mapAttr["minRes"] = m_strMinResolution;
	mapAttr["maxRes"] = m_strMaxResolution;
	mapAttr["minLat"] = m_strMinLat;
	mapAttr["maxLat"] = m_strMaxLat;
	mapAttr["minLon"] = m_strMinLon;
	mapAttr["maxLon"] = m_strMaxLon;

	return mapAttr;
}

/*!
Initialize the process so that it is able to provide data.
*/
IProcess::eInitStatus GDALWMSProcess::initImpl()
{
	m_spParentXYCoverage = 0;

	PYXCOMCreateInstance(GDALXYCoverage::clsid, 0, IXYCoverage::iid, (void**) &m_spParentXYCoverage);

	if (m_strFormat.empty())
	{
		m_strFormat = "image/jpeg";
	}

	if (m_strSrs.empty())
	{
		m_strSrs = "EPSG:4326";
	}

	//if we were provided credentials, use it or fail to init	
	if (getParameter(0)->getValueCount() > 0)
	{
		auto credentialProvider = getParameter(0)->getValue(0)->getOutput()->QueryInterface<IUserCredentialsProvider>();
		auto credentials = credentialProvider->getCredentials(
			CSharpFunctionProvider::getCSharpFunctionProvider()->getUrlHost(m_strServer),
			IUsernameAndPasswordCredentials::iid);

		if (!credentials)
		{
			m_spInitError = boost::intrusive_ptr<IProcessInitError>(
				new UserCredentialsInitError(IUsernameAndPasswordCredentials::iid,
											 CSharpFunctionProvider::getCSharpFunctionProvider()->getUrlHost(m_strServer)));
			return knFailedToInit;
		}
		else
		{
			m_credentials = credentials->
				findFirstCredentialOfType(IUsernameAndPasswordCredentials::iid)->
				QueryInterface<IUsernameAndPasswordCredentials>();
		}
	}

	std::string strProcessWMSXML = createWMSXML();

	if (!m_spParentXYCoverage->openAsRGB(strProcessWMSXML, 0)) 
	{
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GenericProcInitError());
		m_spInitError->setError("GDAL is unable to open the WMS datasource: '" + strProcessWMSXML + "'.");
		return knFailedToInit;
	}

	m_spGeom = m_spParentXYCoverage->getGeometry();

	if (m_strSrs == "EPSG:4326" || m_strSrs == "4326")
	{
		// geographic coordinate systems display better with the request per tile implementation
		m_implementation = WmsRequestPerTileImplementation::create(*this);
	}
	else
	{
		m_implementation = WmsGdalOverviewImplementation::create(m_spParentXYCoverage);
	}

	return knInitialized;
}

/*!
Creates the xml request for one of the WMS grid tiles.
*/
std::string GDALWMSProcess::createWMSXML() const
{
	std::string version;

	if (m_strServiceName == "AGS" || m_strServiceName == "AGSI")
	{
		// versions not used by ArcGIS MapServer
		version = "0";
	}
	else
	{
		auto requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->setDefaultValueForUrlQueryParameter(m_strServer,"version","1.1.1");
		version = CSharpFunctionProvider::getCSharpFunctionProvider()->getUrlQueryParameter(requestUrl,"version");
	}

	PYXRect2DDouble rect;

	// If it's WMS v1.3.0 , we need to flip lat and lon for SRS from a particular set
	// TODO: we need to maintain a list of the SRS to which this applies SRS (so far found a couple)
	if (version == "1.3.0" && (boost::iequals(m_strSrs, "EPSG:4326") || boost::iequals(m_strSrs, "EPSG:4267")))
	{
		//flip lat/lon
		rect = PYXRect2DDouble(	StringUtils::fromString<double>(m_strMinLon),
			StringUtils::fromString<double>(m_strMinLat),
			StringUtils::fromString<double>(m_strMaxLon),
			StringUtils::fromString<double>(m_strMaxLat));
	}
	else
	{
		rect = PYXRect2DDouble(	StringUtils::fromString<double>(m_strMinLat),
			StringUtils::fromString<double>(m_strMinLon),
			StringUtils::fromString<double>(m_strMaxLat),
			StringUtils::fromString<double>(m_strMaxLon));
	}

	// get the minimum image resolution
	double fMinRes = -1.0;
	if (!m_strMinResolution.empty() && StringUtils::isNumeric(m_strMinResolution))
	{
		fMinRes = StringUtils::fromString<double>(m_strMinResolution);
	}

	// no minimum resolution specified, we need to calculate it
	if (fMinRes < 0.0)
	{
		fMinRes = calculateMinimumResolution(rect);
	}

	// using the minimum resolution, calculate the image size in pixels while maintaining the aspect ratio
	double fTotalImageWidthPixels;
	double fTotalImageHeightPixels;
	if (rect.width() > rect.height())
	{
		fTotalImageWidthPixels = rect.width() / fMinRes;
		fTotalImageWidthPixels = std::min(fTotalImageWidthPixels, (double) knMaxOverviewWidthOrHeightPixels);
		fTotalImageHeightPixels = fTotalImageWidthPixels * rect.height() / rect.width();
	}
	else
	{
		fTotalImageHeightPixels = rect.height() / fMinRes;
		fTotalImageHeightPixels = std::min(fTotalImageHeightPixels, (double) knMaxOverviewWidthOrHeightPixels);
		fTotalImageWidthPixels = fTotalImageHeightPixels * rect.width() / rect.height();
	}

	int totalImageWidthPixels = MathUtils::round(fTotalImageWidthPixels);
	int totalImageHeightPixels = MathUtils::round(fTotalImageHeightPixels);

	std::stringstream str;
	std::string fixedServiceName = m_strServiceName;

	if (m_strServiceName == "AGSI")
	{
		fixedServiceName = "AGS";
	}
	
	str	<< "<GDAL_WMS><Service name=\"" << fixedServiceName << "\">";	
	

	if (version != "0")
	{
		str << "<Version>" << XMLUtils::toSafeXMLText(version) << "</Version>";
	}

	str	<< "<ServerUrl>" <<  XMLUtils::toSafeXMLText(m_strServer) << "</ServerUrl>"		
		<< "<ImageFormat>" << XMLUtils::toSafeXMLText(m_strFormat) << "</ImageFormat>"
		<< "<Layers>" << XMLUtils::toSafeXMLText(m_strLayer) << "</Layers>"
		<< "<Styles>" << XMLUtils::toSafeXMLText(m_strStyles) << "</Styles>";

	if (version == "1.3.0")
	{
		//want CRS tag
		str << "<CRS>" << XMLUtils::toSafeXMLText(m_strSrs) << "</CRS>";
	}
	else
	{
		if (m_strServiceName == "AGS" || m_strServiceName == "AGSI")
		{
			str << "<BBoxOrder>xyXY</BBoxOrder>";
		}

		str << "<SRS>" << XMLUtils::toSafeXMLText(m_strSrs) << "</SRS>";
	}

	str	<< "</Service>";	

	str	<< "<BandsCount>3</BandsCount>";

	str << "<DataWindow>"
		<< "<UpperLeftX>" << rect.xMin() << "</UpperLeftX><UpperLeftY>" << rect.yMax() << "</UpperLeftY>"
		<< "<LowerRightX>" << rect.xMax() << "</LowerRightX><LowerRightY>" << rect.yMin() << "</LowerRightY>"
		<< "<SizeX>" << totalImageWidthPixels << "</SizeX><SizeY>" << totalImageHeightPixels << "</SizeY>"
		<< "</DataWindow>";

	//add username and password if needed
	if (m_credentials)
	{
		str	<< "<UserPwd>" + XMLUtils::toSafeXMLText(m_credentials->getUsername() + ":" + m_credentials->getPassword()) + "</UserPwd>";
	}

	// required for servers that use self-signed certificates
	str << "<UnsafeSSL>true</UnsafeSSL>";

	str	<< "</GDAL_WMS>";

	return str.str();
}

/*!
Calculate the minimum resolution in native units/pixel.

\param rect	The rectangle in native coordinates.

\return	The minimum resolution.
*/
double GDALWMSProcess::calculateMinimumResolution(const PYXRect2DDouble& rect) const
{
	double fMinRes;

	// ArcGIS typically does not provide the prefix for the SRS. We add it here
	auto strSrs = m_strSrs;
	if ((m_strServiceName == "AGS" || m_strServiceName == "AGSI") && StringUtils::isNumeric(m_strSrs))
	{
		strSrs = "EPSG:" + m_strSrs;
	}

	// If this is not WGS84 SRS, we most likely need to perform the coordinate conversion,
	// in order to get lat/lon boundaries and calculate the image size at highest resolution
	if (!boost::iequals(strSrs, "EPSG:4326"))
	{
		// Use two coordinate converters to get boundaries in WGS84 degrees
		CoordConverterImpl coordConverter;
		coordConverter.initialize(strSrs);
		CoordConverterImpl wgs84;
		std::string defaultSrs = "EPSG:4326";
		wgs84.initialize(defaultSrs);
		PYXXYBoundsGeometry bounds(rect, coordConverter, knDefaultPyxisResolution);
		PYXRect2DDouble rect1;
		PYXRect2DDouble rect2;
		bounds.getBoundingRects(&wgs84, &rect1, &rect2);
		
		// Handle the case when rect2 is informative
		//		         90
		//			     o
		//  XXXX         |           XX
		//  XXXX         |           XX
		//  o------------+------------o
		//-180           |           180
		//               |
		//   		     o
		//		       -90
		double fEffectiveWidthDegrees = rect1.width();
		if (!rect2.empty())
		{
			fEffectiveWidthDegrees += rect2.width();
		}

		// calculate native units/pixel
		double fTotalPixels = fEffectiveWidthDegrees / kfDefaultGeographicResolutionInDegrees;
		fMinRes = rect.width() / fTotalPixels;
	}
	else
	{
		// a geographic coordinate system in degrees
		fMinRes = kfDefaultGeographicResolutionInDegrees;
	}

	return fMinRes;
}

////////////////////////////////////////////////////////////////////////////////
// IXYCoverage
////////////////////////////////////////////////////////////////////////////////
/*!
Returns a constant coverage definition.

\return		The coverage definition, as a constant.
*/
PYXPointer<const PYXTableDefinition> GDALWMSProcess::getCoverageDefinition() const
{
	return m_spParentXYCoverage->getCoverageDefinition();
}

/*!
Returns the coverage definition.

\return		The coverage definition.
*/
PYXPointer<PYXTableDefinition> GDALWMSProcess::getCoverageDefinition()
{
	return m_spParentXYCoverage->getCoverageDefinition();
}

/*!
Get the coverage value at the specified native coordinate.

\param	native		The native coordinate.
\param	pValue		The value being retrieved.

\return	True if successful, false otherwise.
*/
bool GDALWMSProcess::getCoverageValue(const PYXCoord2DDouble& native,
									  PYXValue* pValue) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	if(!m_spParentXYCoverage->getBounds().inside(native))
	{
		return false;
	}

	auto implementationCoverage = m_implementation->getCoverage();
	if(implementationCoverage)
	{
		return implementationCoverage->getCoverageValue(native, pValue);
	}
	else
	{
		return m_spParentXYCoverage->getCoverageValue(native, pValue);
	}

}


PYXPointer<XYAsyncValueGetter> GDALWMSProcess::getAsyncCoverageValueGetter(
	const XYAsyncValueConsumer & consumer,
	int matrixWidth,
	int matrixHeight
	) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	auto implementationCoverage = m_implementation->getCoverage();
	if(!implementationCoverage)
	{
		return m_spParentXYCoverage->getAsyncCoverageValueGetter(consumer,matrixWidth,matrixHeight);
	}

	return implementationCoverage->getAsyncCoverageValueGetter(consumer,matrixWidth,matrixHeight);
}

/*!
Get the field values around the specified native coordinate.
The origin of the matrix of returned values will be at the grid point 
in the mesh that is lower or equal to the nativeCentre point and adjusted 
left (or down) by trunc().  

If the values to be returned fall outside of the current data set, then 
the edges of the data set will be duplicated and returned.

\param	nativeCentre	The native coordinate.
\param  pValues         An array of PYXValue to be filled in with field values
that are centered on the point requested.
\param  sizeX           Width of PYXValue array.
\param  sizeY           Height of PYXValue array.
*/
void GDALWMSProcess::getMatrixOfValues(const PYXCoord2DDouble& nativeCentre,
									   PYXValue* pValues,
									   int nSizeX,
									   int nSizeY) const
{
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	auto implementationCoverage = m_implementation->getCoverage();
	if(implementationCoverage)
	{
		implementationCoverage->getMatrixOfValues(nativeCentre, pValues, nSizeX, nSizeY);		
	}
	else
	{
		m_spParentXYCoverage->getMatrixOfValues(nativeCentre, pValues, nSizeX, nSizeY);
	}	
}

/*!
Indicates if the coverage has a spatial reference system.

\return True if the coverage has a spatial reference system. False otherwise.
*/
bool GDALWMSProcess::hasSpatialReferenceSystem() const
{
	return m_spParentXYCoverage->hasSpatialReferenceSystem();
}

/*!
Sets the coverage's spatial reference system.

\param	The SRS we are setting to.
*/
void GDALWMSProcess::setSpatialReferenceSystem(boost::intrusive_ptr<ISRS> spSRS)
{
	m_spParentXYCoverage->setSpatialReferenceSystem(spSRS);
}

/*!
Get the coordinate converter.

\return	The coordinate converter.
*/
const ICoordConverter* GDALWMSProcess::getCoordConverter() const
{
	return m_spParentXYCoverage->getCoordConverter();
}

/*!
Get the spatial precision.

\return	The spatial precision in metres or -1.0 if not known.
*/
double GDALWMSProcess::getSpatialPrecision() const
{
	return m_spParentXYCoverage->getSpatialPrecision();
}

/*!
Get the bounds of the data set in native coordinate.

\return	The bounds of the coverage.
*/
const PYXRect2DDouble& GDALWMSProcess::getBounds() const
{
	return m_spParentXYCoverage->getBounds();
}

/*!
Get the distance between data points in this coverage.

\return The step size.
*/
PYXCoord2DDouble GDALWMSProcess::getStepSize() const
{
	return m_spParentXYCoverage->getStepSize();
}

PYXCoord2DDouble GDALWMSProcess::nativeToRasterSubPixel(const PYXCoord2DDouble & native) const
{
	return m_spParentXYCoverage->nativeToRasterSubPixel(native);
}

GDALWMSProcess::WmsRequestPerTileImplementation::WmsRequestPerTileImplementation(GDALWMSProcess & parent)
{
	m_strServiceName = parent.m_strServiceName;
	m_credentials = parent.m_credentials;			
	m_coordConverter = parent.m_spParentXYCoverage->getCoordConverter();

	if (m_strServiceName == "AGS" || m_strServiceName == "AGSI")
	{
		std::stringstream ss;

		ss << parent.m_strServer;
		if (m_strServiceName == "AGSI")
		{
			ss << "/exportImage?f=image";
		} 
		else
		{
			ss << "/export?f=image";	
		}
		
		ss << "&bbox=0,0,0,0";	// placeholder
		ss << "&size=0,0";		// placeholder
		ss << "&imageSR=" << parent.m_strSrs;
		ss << "&bboxSR=" << parent.m_strSrs;
		ss << "&format=" << parent.m_strFormat;
		ss << "&layers=" << parent.m_strLayer;
		ss << "&transparent=false";

		if (m_strServiceName == "AGS")
		{
			ss << "&dpi=";
			ss << "&layerdefs=";		
			ss << "&time=";
			ss << "&layerTimeOptions=";
			ss << "&dynamicLayers=";
		}

		m_baseUrl = ss.str();

		m_fileExtension = ".jpeg";

		m_boundary = PYXRect2DDouble(
			StringUtils::fromString<double>(parent.m_strMinLat),
			StringUtils::fromString<double>(parent.m_strMinLon),
			StringUtils::fromString<double>(parent.m_strMaxLat),
			StringUtils::fromString<double>(parent.m_strMaxLon));
	}
	else
	{
		auto requestUrl = parent.m_strServer;
		requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(requestUrl,"service","WMS");
		requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(requestUrl,"request","GetMap");
		requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->setDefaultValueForUrlQueryParameter(requestUrl,"version","1.1.1");
		requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(requestUrl,"layers",parent.m_strLayer);	
		requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(requestUrl,"format",parent.m_strFormat);
		requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(requestUrl,"styles",parent.m_strStyles);

		auto version = CSharpFunctionProvider::getCSharpFunctionProvider()->getUrlQueryParameter(requestUrl,"version");
		if (version == "1.3.0")
		{
			// WMS 1.3.0 new rules: flip coordinates on bbox coordinates + change srs to crs!		
			requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(requestUrl,"crs",parent.m_strSrs);
		}
		else
		{
			requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(requestUrl,"srs",parent.m_strSrs);
		}

		m_fileExtension = ".jpeg";

		if (parent.m_strFormat == "image/png")
		{
			m_fileExtension = ".png";
		} 
		else if (parent.m_strFormat == "image/gif")
		{
			m_fileExtension = ".gif";
		} 
		else if (parent.m_strFormat == "image/tiff" || parent.m_strFormat == "image/tif" )
		{
			m_fileExtension = ".tif";
		}

		m_baseUrl = requestUrl; 

		m_boundary = (version == "1.3.0") ? 
		PYXRect2DDouble(	StringUtils::fromString<double>(parent.m_strMinLon),
		StringUtils::fromString<double>(parent.m_strMinLat),
		StringUtils::fromString<double>(parent.m_strMaxLon),
		StringUtils::fromString<double>(parent.m_strMaxLat)) :
		PYXRect2DDouble(	StringUtils::fromString<double>(parent.m_strMinLat),
		StringUtils::fromString<double>(parent.m_strMinLon),
		StringUtils::fromString<double>(parent.m_strMaxLat),
		StringUtils::fromString<double>(parent.m_strMaxLon));
	}
}

std::list<PYXPointer<GDALWMSProcess::WmsRequestPerTileImplementation::WmsRequestInfo>> GDALWMSProcess::WmsRequestPerTileImplementation::createRequests(const PYXTile & tile) const
{
	std::list<PYXPointer<WmsRequestInfo>> result;

	auto requestUrl = m_baseUrl;	
	std::string version;
	if (m_strServiceName == "AGS" || m_strServiceName == "AGSI")
	{
		// no version in ArcGIS MapServer
		version = "0";
	}
	else
	{
		version = CSharpFunctionProvider::getCSharpFunctionProvider()->getUrlQueryParameter(requestUrl,"version");
	}

	// get the bounding rect for (an approximation of) the tile
	PYXPointer<PYXCell> spTestCell = PYXCell::create(tile.getRootIndex());
	WGS84CoordConverter coordConverter;
	PYXBoundingRectsCalculator calc(&coordConverter, *spTestCell);

	if (spTestCell->getIndex().hasVertexChildren())
	{
		for (PYXChildIterator it(spTestCell->getIndex()); !it.end(); it.next())
		{
			calc.addCell(PYXCell(it.getIndex()));
		}
	}
	else
	{		
		calc.addCell(*(spTestCell.get()));
	}

	// determine where to split the bounding rectangle, this handles data sets with boundaries
	// that extend slightly beyond the international dateline
	double fSplitLon = -180.0;
	if (m_boundary.xMin() < -180.0)
	{
		fSplitLon = m_boundary.xMin();
	}
	else if (m_boundary.xMax() > 180.0)
	{
		fSplitLon = m_boundary.xMax() - 360.0;
	}

	PYXRect2DDouble rect1; 
	PYXRect2DDouble rect2; 
	calc.getBoundingRects(&rect1, &rect2, fSplitLon);

	rect1.scaleInPlace(1.1);
	rect1.clip(m_boundary);

	if (!rect2.empty())
	{
		rect2.scaleInPlace(1.1);
		rect2.clip(m_boundary);		
		//the clip could make it empty...
	}

	// get the bounding rectangle(s) for the cell at the centre of the tile
	auto tileCell = tile.getRootIndex();
	tileCell.setResolution(tile.getCellResolution());
	PYXRect2DDouble cellRect1; 
	PYXRect2DDouble cellRect2; 	
	PYXCell(tileCell).getBoundingRects(&coordConverter, &cellRect1, &cellRect2);

	// make sure that rect1 is not empty... so we need only check rect2.empty() in the future
	if (rect1.empty())
	{
		if (rect2.empty())
		{
			//special case when both regions are outside. make a small request 1x1 pixel
			rect2 = PYXRect2DDouble(m_boundary.xMin(),m_boundary.yMin(),m_boundary.xMin()+cellRect1.width(),m_boundary.yMin()+cellRect1.height());			
		}

		std::swap(rect1,rect2);
	}

	if (!rect2.empty())
	{
		// make both rects the same height
		rect2.expandY(rect1.yMin());
		rect2.expandY(rect1.yMax());

		rect1.expandY(rect2.yMin());
		rect1.expandY(rect2.yMax());
	}	

	int imageWidth1;
	int imageHeight1;
	int imageWidth2;
	int imageHeight2;

	if (m_strServiceName == "AGS" || m_strServiceName == "AGSI")
	{
		// the cell size in native units
		double fCellSize = cellRect1.height();

		// make the image aspect ratio match the bounding box aspect ratio to avoid distortion
		imageWidth1 = (int) ceil(rect1.width() / fCellSize);
		imageHeight1 = (int) ceil(imageWidth1 * rect1.height() / rect1.width());
		imageWidth2 = (int) ceil(rect2.width() / fCellSize);
		imageHeight2 = (int) ceil(imageWidth2 * rect2.height() / rect2.width());
	}
	else
	{
		imageWidth1 = (int) ceil(rect1.width() / cellRect1.width());
		imageHeight1 = (int) ceil(rect1.height() / cellRect1.height());
		imageWidth2 = (int) ceil(rect2.width() / cellRect1.width());
		imageHeight2 = imageHeight1;
	}

	if (version == "1.3.0")
	{
		std::stringstream bbox;
		bbox << std::setprecision(11) << rect1.yMin() << "," << rect1.xMin() << "," << rect1.yMax() << "," << rect1.xMax();
		requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(requestUrl,"bbox",bbox.str());	
	}
	else
	{
		std::stringstream bbox;
		bbox << std::setprecision(11) << rect1.xMin() <<"," << rect1.yMin() << "," << rect1.xMax() << "," << rect1.yMax();
		requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(requestUrl,"bbox",bbox.str());	
	}

	if (m_strServiceName == "AGS" || m_strServiceName == "AGSI")
	{
		std::stringstream ss;
		ss << imageWidth1 << "," << imageHeight1;
		requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(requestUrl,"size",ss.str());
	}
	else
	{
		requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(requestUrl,"width",StringUtils::toString(imageWidth1));
		requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(requestUrl,"height",StringUtils::toString(imageHeight1));
	}

	auto request1 = WmsRequestInfo::create();
	request1->url = requestUrl;
	request1->area = rect1;
	request1->imageSize = PYXCoord2DInt(imageWidth1, imageHeight1);
	request1->imagesPath = AppServices::makeTempFile(m_fileExtension);

	result.push_back(request1);

	if (!rect2.empty())
	{
		if (m_strServiceName == "AGS" || m_strServiceName == "AGSI")
		{
			std::stringstream ss;
			ss << imageWidth2 << "," << imageHeight2;
			requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(requestUrl,"size",ss.str());
		}
		else
		{
			requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(requestUrl,"width",StringUtils::toString(imageWidth2));
			requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(requestUrl,"height",StringUtils::toString(imageHeight2));
		}

		if (version == "1.3.0")
		{
			//WMS 1.3.0 new rules: flip coordinates on bbox for EPSG:4326 coordinates + change src to crs!		
			std::stringstream bbox;
			bbox << std::setprecision(11) << rect2.yMin() << "," << rect2.xMin() << "," << rect2.yMax() << "," << rect2.xMax() ;
			requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(requestUrl,"bbox",bbox.str());	
		}
		else
		{
			std::stringstream bbox;
			bbox << std::setprecision(11) << rect2.xMin() <<"," << rect2.yMin() << "," << rect2.xMax() << "," << rect2.yMax();
			requestUrl = CSharpFunctionProvider::getCSharpFunctionProvider()->overwriteUrlQueryParameter(requestUrl,"bbox",bbox.str());	
		}

		auto request2 = WmsRequestInfo::create();
		request2->url = requestUrl;
		request2->area = rect2;
		request2->imageSize = PYXCoord2DInt(imageWidth2, imageHeight2);
		request2->imagesPath = AppServices::makeTempFile(m_fileExtension);

		result.push_back(request2);
	}

	return result;
}

std::string GDALWMSProcess::WmsRequestPerTileImplementation::createSingleFileVrt(const PYXPointer<GDALWMSProcess::WmsRequestPerTileImplementation::WmsRequestInfo> & request) const
{
	std::stringstream vrtDefinition;

	int imageWidth = request->imageSize.x() ;
	int imageHeight = request->imageSize.y();

	std::string path = XMLUtils::toSafeXMLText(FileUtils::pathToString(request->imagesPath));

	vrtDefinition 
		<< std::setprecision(11)
		<< "<VRTDataset rasterXSize=\""<< imageWidth <<"\" rasterYSize=\""<< imageHeight <<"\">"
		<< "<SRS>GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]]</SRS>"		
		<< "<GeoTransform>"<< request->area.xMin() <<","<< (request->area.width())/imageWidth <<","<< 0.0 <<","<< request->area.yMax() <<","<< 0.0 <<","<< (-request->area.height())/imageHeight << "</GeoTransform>"

		<< "<VRTRasterBand dataType=\"Byte\" band=\"1\">"
		<< "<ColorInterp>"<< "Red" << "</ColorInterp>"
		<< "<SimpleSource>"
		<< "<SourceFilename relativeToVRT=\"0\">" << path << "</SourceFilename>"
		<< "<SourceBand>"<<1<<"</SourceBand>"
		<< "<SrcRect xOff=\""<< 0 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "<DstRect xOff=\""<< 0 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "</SimpleSource>"
		<< "</VRTRasterBand>"

		<< "<VRTRasterBand dataType=\"Byte\" band=\"2\">"
		<< "<ColorInterp>"<< "Green" << "</ColorInterp>"
		<< "<SimpleSource>"
		<< "<SourceFilename relativeToVRT=\"0\">" << path << "</SourceFilename>"
		<< "<SourceBand>"<<2<<"</SourceBand>"
		<< "<SrcRect xOff=\""<< 0 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "<DstRect xOff=\""<< 0 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "</SimpleSource>"
		<< "</VRTRasterBand>"

		<< "<VRTRasterBand dataType=\"Byte\" band=\"3\">"
		<< "<ColorInterp>"<< "Blue" << "</ColorInterp>"
		<< "<SimpleSource>"
		<< "<SourceFilename relativeToVRT=\"0\">" << path << "</SourceFilename>"
		<< "<SourceBand>"<<3<<"</SourceBand>"
		<< "<SrcRect xOff=\""<< 0 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "<DstRect xOff=\""<< 0 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "</SimpleSource>"
		<< "</VRTRasterBand>"
		<< "</VRTDataset>";

	return vrtDefinition.str();
}

std::string GDALWMSProcess::WmsRequestPerTileImplementation::createTwoFilesVrt(const PYXPointer<GDALWMSProcess::WmsRequestPerTileImplementation::WmsRequestInfo> & request1,const PYXPointer<GDALWMSProcess::WmsRequestPerTileImplementation::WmsRequestInfo> & request2) const
{
	std::stringstream vrtDefinition;

	int imageWidth1 = request1->imageSize.x();
	int imageWidth2 = request2->imageSize.x();
	int imageHeight = request1->imageSize.y();

	int totalWidth = (int)ceil(360 / (request1->area.width()) * imageWidth1);
	int offset1 = (int)((180+request1->area.xMin())/360*totalWidth);
	int offset2 = (int)((180+request2->area.xMin())/360*totalWidth);

	std::string path1 = XMLUtils::toSafeXMLText(FileUtils::pathToString(request1->imagesPath));
	std::string path2 = XMLUtils::toSafeXMLText(FileUtils::pathToString(request2->imagesPath));

	vrtDefinition 
		<< std::setprecision(11)
		<< "<VRTDataset rasterXSize=\""<< totalWidth <<"\" rasterYSize=\""<< imageHeight <<"\">"		
		<< "<SRS>GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]]</SRS>"
		<< "<GeoTransform>"<< -180 <<","<< 360.0/totalWidth <<","<< 0.0 <<","<< request1->area.yMax() <<","<< 0.0 <<","<< (-request1->area.height())/imageHeight << "</GeoTransform>"

		<< "<VRTRasterBand dataType=\"Byte\" band=\"1\">"
		<< "<ColorInterp>"<< "Red" << "</ColorInterp>"

		<< "<SimpleSource>"
		<< "<SourceFilename relativeToVRT=\"0\">" << path1 << "</SourceFilename>"
		<< "<SourceBand>"<<1<<"</SourceBand>"
		<< "<SrcRect xOff=\""<< 0 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth1 <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "<DstRect xOff=\""<< offset1 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth1 <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "</SimpleSource>"

		<< "<SimpleSource>"
		<< "<SourceFilename relativeToVRT=\"0\">" << path2 << "</SourceFilename>"
		<< "<SourceBand>"<<1<<"</SourceBand>"
		<< "<SrcRect xOff=\""<< 0 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth2 <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "<DstRect xOff=\""<< offset2 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth2 <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "</SimpleSource>"

		<< "</VRTRasterBand>"

		<< "<VRTRasterBand dataType=\"Byte\" band=\"2\">"
		<< "<ColorInterp>"<< "Green" << "</ColorInterp>"

		<< "<SimpleSource>"
		<< "<SourceFilename relativeToVRT=\"0\">" << path1 << "</SourceFilename>"
		<< "<SourceBand>"<<2<<"</SourceBand>"
		<< "<SrcRect xOff=\""<< 0 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth1 <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "<DstRect xOff=\""<< offset1 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth1 <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "</SimpleSource>"

		<< "<SimpleSource>"
		<< "<SourceFilename relativeToVRT=\"0\">" << path2 << "</SourceFilename>"
		<< "<SourceBand>"<<2<<"</SourceBand>"
		<< "<SrcRect xOff=\""<< 0 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth2 <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "<DstRect xOff=\""<< offset2 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth2 <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "</SimpleSource>"

		<< "</VRTRasterBand>"

		<< "<VRTRasterBand dataType=\"Byte\" band=\"3\">"
		<< "<ColorInterp>"<< "Blue" << "</ColorInterp>"

		<< "<SimpleSource>"
		<< "<SourceFilename relativeToVRT=\"0\">" << path1 << "</SourceFilename>"
		<< "<SourceBand>"<<3<<"</SourceBand>"
		<< "<SrcRect xOff=\""<< 0 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth1 <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "<DstRect xOff=\""<< offset1 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth1 <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "</SimpleSource>"

		<< "<SimpleSource>"
		<< "<SourceFilename relativeToVRT=\"0\">" << path2 << "</SourceFilename>"
		<< "<SourceBand>"<<3<<"</SourceBand>"
		<< "<SrcRect xOff=\""<< 0 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth2 <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "<DstRect xOff=\""<< offset2 <<"\" yOff=\""<< 0 <<"\" xSize=\"" << imageWidth2 <<"\" ySize=\""<< imageHeight <<"\"/>"
		<< "</SimpleSource>"

		<< "</VRTRasterBand>"
		<< "</VRTDataset>";

	return vrtDefinition.str();
}

void GDALWMSProcess::WmsRequestPerTileImplementation::tileLoadHint(const PYXTile& tile) const
{
	//web requests for downloading the tile data from the server
	std::list<PYXPointer<WmsRequestInfo>> requests = createRequests(tile);
	//information holders to use for loading downloaded and stored data on the globe
	std::list<PYXPointer<WmsRequestInfo>> successfulRequests;

	for(auto & request : requests)
	{	
		auto httpRequest = HttpRequest::create(request->url,"GET",m_credentials);
		auto path = FileUtils::pathToString(request->imagesPath);

		if(!httpRequest->downloadResponse(path))
		{
			PYXTHROW (PYXException, "Failed to download wms image: " << request->url);
		}

		auto extension = FileUtils::getExtensionNoDot(path);
		if (boost::iequals(extension, "png") || boost::iequals(extension, "gif")) 
		{
			//for the specified formats, try to load a bitmap from the downloaded data
			try
			{
				auto newPath = BitmapServerProvider::getBitmapServerProvider()->forceRGB(path);

				if (newPath != path)
				{
					request->oldImagesPath = request->imagesPath;
					request->imagesPath = newPath;
				}

				successfulRequests.push_back(request);
			}
			catch (...)
			{
				TRACE_ERROR("Failed to force RGB for the file: " << path.c_str());
			}
		}
		else
		{
			// for all other formats, consider the request successful as long as we received a response
			successfulRequests.push_back(request);
		}

		m_requests.push_back(request);
	}

	//don't proceed unless we have data to load
	if (successfulRequests.empty())
	{
		PYXTHROW(PYXException, "Failed to load data for a tile.");
	}

	std::string vrtDefinition;
	if (successfulRequests.size() == 1)
	{
		vrtDefinition = createSingleFileVrt(successfulRequests.front());
	}
	else
	{
		vrtDefinition = createTwoFilesVrt(successfulRequests.front(),successfulRequests.back());
	}

	m_spCurrentDataSrc = PYXCOMCreateInstance<IXYCoverage,GDALXYCoverage>();
	auto xyCoverage = boost::dynamic_pointer_cast<GDALXYCoverage>(m_spCurrentDataSrc);

	if (!xyCoverage->openAsRGB(vrtDefinition,0))
	{
		PYXTHROW(PYXException,"GDAL is unable to open the WMS datasource");
	}
}

void GDALWMSProcess::WmsRequestPerTileImplementation::tileLoadHintDone(const PYXTile& tile) const
{
	if (m_spCurrentDataSrc)
	{
		m_spCurrentDataSrc->tileLoadDoneHint(tile);
		m_spCurrentDataSrc.reset();
	}

	std::list<PYXPointer<WmsRequestInfo>> failedToDeleteRequests;

	for(auto & request : m_requests)
	{
		try
		{
			boost::filesystem::remove(request->imagesPath);

			if (!request->oldImagesPath.empty())
			{
				boost::filesystem::remove(request->oldImagesPath);
			}
		}
		catch(...)
		{
			failedToDeleteRequests.push_back(request);
		}
	}

	std::swap(m_requests,failedToDeleteRequests);
}

boost::intrusive_ptr<IXYCoverage> GDALWMSProcess::WmsRequestPerTileImplementation::getCoverage() const
{
	return m_spCurrentDataSrc;
}

GDALWMSProcess::WmsGdalOverviewImplementation::WmsGdalOverviewImplementation(const boost::intrusive_ptr<GDALXYCoverage> & parent)
{
	m_spOverviews.push_back(parent);
}

void GDALWMSProcess::WmsGdalOverviewImplementation::tileLoadHint(const PYXTile& tile) const
{
	unsigned int index = 0;
	boost::intrusive_ptr<GDALXYCoverage> currentDataSource = m_spOverviews[index];

	m_spCurrentDataSrc = currentDataSource;	
	int resolution = currentDataSource->getGeometry()->getCellResolution();

	while (currentDataSource->hasOverview() && tile.getCellResolution() <= resolution)
	{
		m_spCurrentDataSrc = currentDataSource;
		index++;
		if (! (index < m_spOverviews.size()) )
		{
			m_spOverviews.push_back(currentDataSource->openOverview());
		}
		currentDataSource = m_spOverviews[index];
		resolution = currentDataSource->getGeometry()->getCellResolution();
	}
}

void GDALWMSProcess::WmsGdalOverviewImplementation::tileLoadHintDone(const PYXTile& tile) const
{	
	if (m_spCurrentDataSrc)
	{
		m_spCurrentDataSrc->tileLoadDoneHint(tile);
		m_spCurrentDataSrc.reset();
	}
}

boost::intrusive_ptr<IXYCoverage> GDALWMSProcess::WmsGdalOverviewImplementation::getCoverage() const
{
	return m_spCurrentDataSrc;
}


/*!
Loads a hint for tile data access.
*/
void GDALWMSProcess::tileLoadHint(const PYXTile& tile) const
{	
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	m_implementation->tileLoadHint(tile);
}

/*!
Indicates when tile hint is no longer needed.
*/
void GDALWMSProcess::tileLoadDoneHint(const PYXTile& tile) const
{	
	boost::recursive_mutex::scoped_lock lock(m_mutex);

	m_implementation->tileLoadHintDone(tile);
}

PYXPointer<OWSFormat> GDALWMSProcess::getDefaultOutputFormat() const
{	
	if (m_strServiceName == "AGS" || m_strServiceName == "AGSI")
	{
		// OWS not supported for ArcGIS MapServers
		PYXTHROW_NOT_SUPPORTED();
	}

	if (m_strFormat.empty())
	{
		return OWSFormat::create("image/jpeg");
	}

	return OWSFormat::create(m_strFormat);
}

bool GDALWMSProcess::supportOutputType( const OWSFormat & format ) const
{
	if (m_strServiceName == "AGS" || m_strServiceName == "AGSI")
	{
		PYXTHROW_NOT_SUPPORTED();
	}

	return GDALWMSProcess::getDefaultOutputFormat()->supportMimeType(format.getMimeType());	
}

std::string GDALWMSProcess::getOWSReference( ReferenceType referenceType, const OWSFormat & format ) const
{
	if (m_strServiceName == "AGS" || m_strServiceName == "AGSI")
	{
		PYXTHROW_NOT_SUPPORTED();
	}

	if (!supportOutputType(format))
	{
		PYXTHROW(PYXException, "mimeType and schema is not supported");
	}

	std::string server = m_strServer;
	std::string layerName = m_strLayer;
	std::string version = CSharpFunctionProvider::getCSharpFunctionProvider()->getUrlQueryParameter(server,"version");
	if (version.empty())
	{
		version = "1.0.0";
	}
	std::string srs = m_strSrs;

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

	if (layerName.empty())
	{
		PYXTHROW(PYXException, "CoverageId was not set");
	}

	if (version.size() == 0 )
	{
		version = "1.0.0";
	}

	std::string outputType = m_strFormat;
	std::string styles = m_strStyles;

	CSharpFunctionProvider & csharpFunctionProvider = *CSharpFunctionProvider::getCSharpFunctionProvider();
	if (referenceType == IOWSReference::WpsReference)
	{
		std::string getMapRequest = server;
		getMapRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getMapRequest,"Service","WMS");
		getMapRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getMapRequest,"Version",version);
		getMapRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getMapRequest,"Request","GetMap");
		getMapRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getMapRequest,"Layers",layerName);
		getMapRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getMapRequest,"Format",outputType);
		getMapRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getMapRequest,"Styles",styles);

		return "<wps:Reference mimeType=\"" + XMLUtils::toSafeXMLText(format.getMimeType()) + "\" "
			"xlink:href=\"" + XMLUtils::toSafeXMLText(getMapRequest) + "\" "
			"method=\"GET\"/>";
	}
	if (referenceType == IOWSReference::OwsContextReference)
	{
		std::string getCapabilitiesRequest = server;
		getCapabilitiesRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCapabilitiesRequest,"Service","WMS");
		getCapabilitiesRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCapabilitiesRequest,"Version",version);
		getCapabilitiesRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getCapabilitiesRequest,"Request","GetCapabilities");

		std::string getMapRequest = getCapabilitiesRequest;
		getMapRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getMapRequest,"Request","GetMap");
		getMapRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getMapRequest,"Layers",layerName);
		getMapRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getMapRequest,"Format",outputType);
		getMapRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getMapRequest,"Styles",styles);

		//We have no sense of screen size, so half HD is a random guess
		getMapRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getMapRequest,"Width","1280");
		getMapRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getMapRequest,"Height","720");

		if (version >= "1.3.0")
		{
			getMapRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getMapRequest,"CRS",srs);
			csharpFunctionProvider.overwriteUrlQueryParameter(getMapRequest,"BBOX",m_strMinLon + "," + m_strMinLat + "," + m_strMaxLon + "," + m_strMaxLat);
		}
		else
		{
			getMapRequest = csharpFunctionProvider.overwriteUrlQueryParameter(getMapRequest,"SRS",srs);
			csharpFunctionProvider.overwriteUrlQueryParameter(getMapRequest,"BBOX", m_strMinLat + "," + m_strMinLon + "," + m_strMaxLat + "," + m_strMaxLon);
		}

		return "<owc:offering code=\"http://www.opengis.net/spec/owc/1.0/req/atom/wms\">" 
			"<owc:operation method=\"GET\" code=\"GetCapabilities\" href=\""+XMLUtils::toSafeXMLText(getCapabilitiesRequest)+"\" type=\"text/xml\"/>" 
			"<owc:operation method=\"GET\" code=\"GetMap\" type=\"" + XMLUtils::toSafeXMLText(outputType) + "\" href=\""+XMLUtils::toSafeXMLText(getMapRequest)+"\"/>" 
			"</owc:offering>";
	}

	PYXTHROW(PYXException,"unsupported reference type");
}


