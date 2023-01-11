/******************************************************************************
gdal_file_process.cpp

begin      : 9/25/2007 2:31:48 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#include "stdafx.h"
#define MODULE_GDAL_SOURCE

// local includes
#include "gdal_file_process.h"
#include "exceptions.h"

// pyxlib includes
#include "pyxis/data/exceptions.h"
#include "pyxis/procs/path.h"
#include "pyxis/procs/srs.h"
#include "pyxis/sampling/xy_utils.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/tester.h"

// boost includes
#include <boost/filesystem/path.hpp>

// {73B180AB-100B-4742-BE18-045F1DEF4B95}
PYXCOM_DEFINE_CLSID(GDALFileProcess,
0x73b180ab, 0x100b, 0x4742, 0xbe, 0x18, 0x4, 0x5f, 0x1d, 0xef, 0x4b, 0x95);
PYXCOM_CLASS_INTERFACES(GDALFileProcess, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(GDALFileProcess, "GDAL File Reader", "A Geospatial file opened by the GDAL library", "Reader",
					IXYCoverage::iid, IFeatureCollection::iid, IFeature::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(IPath::iid, 1, 1, "Geospatial File Path", "A Process containing the path of the GDAL file to open");
	IPROCESS_SPEC_PARAMETER(ISRS::iid, 0, 1, "Spatial Reference System", "An external spatial reference system definition.")
IPROCESS_SPEC_END

// Tester class
Tester<GDALFileProcess> gTester;

// Test method
void GDALFileProcess::test()
{
	boost::intrusive_ptr<IProcess> spProc(new GDALFileProcess);
	TEST_ASSERT(spProc);

	GDALFileProcess* pCov = dynamic_cast<GDALFileProcess*>(spProc.get());
	TEST_ASSERT(pCov != 0);

	boost::filesystem::path tempPath = AppServices::makeTempFile(".ppl");
	TRACE_TEST("Writing GDALFileProcess to file: " << FileUtils::pathToString(tempPath));
	PipeManager::writeProcessToFile(FileUtils::pathToString(tempPath), spProc);

	// TODO: Test that the process is read from file correctly.
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

//! Get the attributes in this process.
std::map<std::string, std::string> STDMETHODCALLTYPE GDALFileProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;

	if (! m_forcedNullValue.isNull())
	{
		mapAttr["ForcedNullValue"] = StringUtils::toString<PYXValue>(m_forcedNullValue);
	}

	if (! m_layerName.empty())
	{
		mapAttr["LayerName"] = m_layerName;
	}

	if (! m_selectedBands.empty())
	{
		mapAttr["Bands"] = m_selectedBands;
	}

	return mapAttr;
}

//! Set the attributes in this process.
void STDMETHODCALLTYPE GDALFileProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	std::string forceNullValueString;
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr,"ForcedNullValue",forceNullValueString);

	try
	{
		m_forcedNullValue = StringUtils::fromString<PYXValue>(forceNullValueString);
	}
	catch(...)
	{
		//make null value
		m_forcedNullValue = PYXValue();
	}

	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr, "LayerName", m_layerName);
	UPDATE_PROCESS_ATTRIBUTE_STRING(mapAttr, "Bands", m_selectedBands);
}

/*! 
Serialize the process to a map of strings.

\return the attribute schema as a string.
*/
std::string GDALFileProcess::getAttributeSchema() const
{
	return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"GDALFileProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"ForcedNullValue\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>No Data Value</friendlyName>"
					"<description>Value that represent that there is no data for that location.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"

			  "<xs:element name=\"LayerName\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Layer's name</friendlyName>"
					"<description>Address or index of the layer in the GDAL File. Empty if there is only one layer.</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"

			  "<xs:element name=\"Bands\" type=\"xs:string\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>Selected Bands</friendlyName>"
					"<description>CSV list of bands</description>"
				  "</xs:appinfo>"
				"</xs:annotation>"
			  "</xs:element>"

			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

/*!
Prepare the process so that it is able to provide data.

*/
IProcess::eInitStatus GDALFileProcess::initImpl()
{
	m_spXYCoverage = 0;
	PYXCOMCreateInstance(GDALXYCoverage::clsid, 0, IXYCoverage::iid, (void**) &m_spXYCoverage);
	if  (!m_spXYCoverage)
	{
		setInitProcError<GenericProcInitError>("Unable to create an instance of a GDALXYCoverage. Fatal Error.");
		return knFailedToInit;
	}	

	// Get the file path.
	boost::intrusive_ptr<IPath> spPath;
	getParameter(0)->getValue(0)->getOutput()->QueryInterface(IPath::iid, (void**) &spPath);
	if (!spPath)
	{
		setInitProcError<GenericProcInitError>("Could not retrieve the Path input parameter for the GDAL File process");
		return knFailedToInit;
	}

	// initialize the Conversion object (get the SRS if one was provided)
	boost::intrusive_ptr<ISRS> spSRS;
	if (getParameter(1)->getValueCount() == 1)
	{
		getParameter(1)->getValue(0)->getOutput()->QueryInterface(ISRS::iid, (void**) &spSRS);
		if (!spSRS)
		{
			setInitProcError<GenericProcInitError>("Invalid SRS used as an input.");
			return knFailedToInit;
		}
	}

	auto bands = std::vector<int>();
	try
	{
		std::stringstream ss(m_selectedBands);
		std::string item;
		while (std::getline(ss, item, ',')) 
		{
			bands.push_back(std::stoi(item));
		}
	}
	catch(...)
	{
		TRACE_ERROR("Failed to parse selected bands: " << m_selectedBands);
		m_spInitError = boost::intrusive_ptr<IProcessInitError>();
		return knFailedToInit;
	}

	// if a non-standard set of bands are selected, change the proc identity to generate new cache data
	// this fixes a problem where netCDF and GRIB files were always returning band 1 regardless of the
	// selected band
	setData("");
	for (std::vector<int>::size_type i = 0; i < bands.size(); i++)
	{
		// GDAL band numbering goes from 1..n
		if (bands[i] != (i + 1))
		{
			setData("V2");
			break;
		}
	}

	try
	{
		if (!m_spXYCoverage->open(spPath->getLocallyResolvedPath(), spSRS, bands, m_layerName))
		{
			setInitProcError<GenericProcInitError>("Unable to open GDAL file '" + spPath->getLocallyResolvedPath() + "'.");
			return knFailedToInit;
		}
	}
	catch (MissingGeometryException& e)
	{
		TRACE_ERROR("Caught MissingGeometryException during file open: " << e.getFullErrorString());
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new MissingGeometryInitError());
		return knFailedToInit;
	}
	catch (MissingWorldFileException& e)
	{
		TRACE_ERROR("Caught MissingWorldFileException during file open: " << e.getFullErrorString());
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new MissingWorldFileInitError());
		return knFailedToInit;
	}
	catch (MissingSRSException& e)
	{
		TRACE_ERROR("Caught MissingSRSException during file open: " << e.getFullErrorString());
		m_spInitError = boost::intrusive_ptr<IProcessInitError>(new GDALSRSInitError());
		return knFailedToInit;
	}

	//forced used defined null value
	m_spXYCoverage->forceNullValue(m_forcedNullValue);

	return knInitialized;
}
