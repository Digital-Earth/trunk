/******************************************************************************
gdal_file_converter_process.cpp

begin      : 1/4/2008 2:44:24 PM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_GDAL_SOURCE

// local includes
#include "exceptions.h"
#include "gdal_file_converter_process.h"

// pyxlib includes
#include "pyxis/data/coverage.h"
#include "pyxis/derm/wgs84_coord_converter.h"
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/procs/path.h"
#include "pyxis/sampling/xy_coverage.h"
#include "pyxis/utility/app_services.h"

// standard includes
#include <iostream>
#include <fstream>

// {7732B135-8BB8-47f7-99E4-6E039FDF5067}
PYXCOM_DEFINE_CLSID(GDALFileConverterProcess,
0x7732b135, 0x8bb8, 0x47f7, 0x99, 0xe4, 0x6e, 0x3, 0x9f, 0xdf, 0x50, 0x67);
PYXCOM_CLASS_INTERFACES(GDALFileConverterProcess, IProcess::iid, PYXCOM_IUnknown::iid, IDataProcessor::iid);

IPROCESS_SPEC_BEGIN(GDALFileConverterProcess, "Export Process", "The converted coverage (to a file).", "Hidden",
					IDataProcessor::iid, PYXCOM_IUnknown::iid)
	IPROCESS_SPEC_PARAMETER(ICoverage::iid, 1, 1, "Coverage", "The process (root of a pipeline) who owns the coverage to convert and write to file.")
IPROCESS_SPEC_END

// Tester class
Tester<GDALFileConverterProcess> gTester;

// Test method
void GDALFileConverterProcess::test()
{
	/*
	[chowell] - Will uncomment this test once the FileConverter plugin is rebuilt again and the updated binaries
	are checked into the system. Until then this test is being disabled, because we know there's a problem with it 
	and it shouldn't stop the build from succeeding.
	*/
#if 0
	boost::intrusive_ptr<IProcess> spProc(new GDALFileConverterProcess);
	TEST_ASSERT(spProc);

	GDALFileConverterProcess* pConverter = dynamic_cast<GDALFileConverterProcess*>(spProc.get());
	TEST_ASSERT(pConverter != 0);

	// Use a checkerboard coverage to write to file
	boost::intrusive_ptr<IProcess> spCoverageProc;
	PYXCOMCreateInstance(
		strToGuid("{1716ED04-9238-42f6-9CC2-C0749A828CA8}"), 
		0, 
		IProcess::iid, 
		(void**) &spCoverageProc);

	// Set the process parameter
	TEST_ASSERT(spCoverageProc);
	pConverter->getParameter(0)->addValue(spCoverageProc);
	
	// Set the save path parameter
	boost::filesystem::path strImagePath = AppServices::makeTempFile(".jpg");
	std::map<std::string, std::string> mapAttr;
	mapAttr["savePath"] = FileUtils::pathToString(strImagePath);
	pConverter->setAttributes(mapAttr);

	TEST_ASSERT(pConverter->initProc(true) == IProcess::knInitialized);

	// Get the coverage
	boost::intrusive_ptr<ICoverage> spCoverage;
	pConverter->getParameter(0)->getValue(0)->getOutput()->QueryInterface(ICoverage::iid, (void**) &spCoverage);
	TEST_ASSERT(spCoverage != 0);

	// Get the bounds
	WGS84CoordConverter coordConverter;
	PYXRect2DDouble rect1, rect2;
	spCoverage->getGeometry()->getBoundingRects(&coordConverter, &rect1, &rect2); // rect2 is not used	

	// Create the PGD file
	int nXSize = 100;
	int nYSize = 50;
	boost::filesystem::path strPGDPath = pConverter->createPGDFile(rect1, nXSize, nYSize);

	TRACE_TEST("Writing coverage to file using the PYXIS GDAL driver: " << FileUtils::pathToString(strImagePath));

	// Open the PGD file with the PYXISGDAL driver
	GDALDataset* pPYXISDataset = (GDALDataset *) GDALOpen(FileUtils::pathToString(strPGDPath).c_str(), GA_ReadOnly);
	TEST_ASSERT(pPYXISDataset != 0);
	
	bool bResult = pConverter->writeToFile(pPYXISDataset);
	TEST_ASSERT(bResult);
	if (!bResult)
	{
		TRACE_ERROR("THIS FAILURE COULD MEAN THE PYXIS GDAL DRIVER IS OUT OF DATE!!");
	}
	GDALClose(pPYXISDataset);

// TODO: Re-enable this call when the size is configurable (ticket 473)
	// perform the conversion again through the public method, the test framework will catch exceptions
	//pConverter->processData();

	// TODO: binary compare of created image and \\aristotle\Pyxis_Data\TestData\ExportCloudsGoal.jpg
	// TODO: add forced exceptions tests
#endif
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

/*! 
Serialize the process to a map of strings.

\return the attribute schema as a string.
*/
std::string GDALFileConverterProcess::getAttributeSchema() const
{
	return 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
			"elementFormDefault=\"qualified\" "
			"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
			"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		  "<xs:element name=\"GDALFileConverterProcess\">"
		  "<xs:complexType>"
			"<xs:sequence>"
			  "<xs:element name=\"uri\" type=\"xs:anyURI\">"
				"<xs:annotation>"
				  "<xs:appinfo>"
					"<friendlyName>URI</friendlyName>"
					"<description></description>"
				  "</xs:appinfo>"
				"</xs:annotation>"			  
			"</xs:sequence>"
		  "</xs:complexType>"
		  "</xs:element>"
		"</xs:schema>";
}

/*!
Set the attributes for this process.
*/
void STDMETHODCALLTYPE GDALFileConverterProcess::setAttributes(
	const std::map<std::string, std::string>& mapAttr)
{
	m_initState = knNeedsInit;
	std::map<std::string, std::string>::const_iterator it;

	// Get the server URL attribute.
	it = mapAttr.find("savePath");
	if (it != mapAttr.end())
	{
		m_strSavePath = it->second;
	}
}

/*!
Get the attributes for this process.
*/
std::map<std::string, std::string> GDALFileConverterProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;	
	mapAttr["savePath"] = m_strSavePath;

	return mapAttr;
}

////////////////////////////////////////////////////////////////////////////////
// IDataProcessor
////////////////////////////////////////////////////////////////////////////////
/*!
Perform the actual conversion and file writing operations. Any errors in the conversion
process will throw exceptions.
*/
void GDALFileConverterProcess::processData()
{
	assert(getParameter(0)->getValue(0) && "not properly initialized");

	boost::intrusive_ptr<ICoverage> spCoverage;
	if (getParameter(0)->getValue(0)->getOutput())
	{
		getParameter(0)->getValue(0)->getOutput()->QueryInterface(ICoverage::iid, (void**) &spCoverage);
	}
	if(!spCoverage)
	{
		PYXTHROW(
			GDALFileConvertException, 
			"Could not retrieve the Coverage parameter for the GDAL File Converter Process");
	}

	// Get the bounds
	WGS84CoordConverter coordConverter;
	PYXRect2DDouble rect1, rect2;
	spCoverage->getGeometry()->getBoundingRects(&coordConverter, &rect1, &rect2); // rect2 is not used
		
	// Calculate output size (will change to be dynamic when both TODO's below are completed
	double xRatio = (rect1.xMax() - rect1.xMin()) / 360;
	double yRatio = (rect1.yMax() - rect1.yMin()) / 180;
	int nXSize = static_cast<int>(1024 * xRatio);
	int nYSize = static_cast<int>(512 * yRatio);

	boost::filesystem::path strPGDPath = createPGDFile(rect1, nXSize, nYSize);
	GDALDataset* pPYXISDataset = (GDALDataset *) GDALOpen(FileUtils::pathToString(strPGDPath).c_str(), GA_ReadOnly);

	if(!pPYXISDataset)
	{
		PYXTHROW(GDALFileConvertException, "PYXIS GDAL driver could not open the pipeline");
	}
	
	if(!writeToFile(pPYXISDataset))
	{
		PYXTHROW(GDALFileConvertException, "Could not convert the Coverage for the GDAL File Converter Process");
	}

	GDALClose(pPYXISDataset);
}

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

/*!
Creates the pgd file needed by the driver.
*/
boost::filesystem::path GDALFileConverterProcess::createPGDFile(PYXRect2DDouble rect1, int nXSize, int nYSize)
{
	// Write the process out to a temp file for the PYXIS GDAL driver input
	boost::filesystem::path strPPLPath = AppServices::makeTempFile(".ppl");	
	PipeManager::writePipelineToFile(FileUtils::pathToString(strPPLPath), getParameter(0)->getValue(0));

	// Make PGD file to hold the driver inputs. TODO: move to use XML
	boost::filesystem::path strPGDPath = AppServices::makeTempFile(".pgd");
	std::ofstream outFilePGD(FileUtils::pathToString(strPGDPath).c_str());

	outFilePGD << FileUtils::pathToString(AppServices::getWorkingPath()).c_str() << "\n";
	outFilePGD << FileUtils::pathToString(AppServices::getApplicationPath()).c_str() << "\n";
	outFilePGD << FileUtils::pathToString(strPPLPath) << "\n"; // TODO: get file name internally
	outFilePGD << nXSize << "\n"; // TODO: calculate from geometry
	outFilePGD << nYSize << "\n"; // TODO: calculate from geometry	
	outFilePGD << "10\n"; // TODO: use later: spCoverage->getGeometry()->getCellResolution();	
	outFilePGD << rect1.xMin() << "\n"; // TODO: PYXISGDAL driver has yMax yMin reversed. Will fix in next version.
	outFilePGD << rect1.yMax() << "\n";
	outFilePGD << rect1.xMax() << "\n";
	outFilePGD << rect1.yMin() << "\n";	
	outFilePGD.close();
	
	return strPGDPath;
}

/*!
Converts a GDALXYCoverage and saves the converted data.
*/
bool GDALFileConverterProcess::writeToFile(GDALDataset* pDS)
{
	const char* pFormat = "JPEG";
	GDALDriver* pDriver;
	char** ppMetadata;

	pDriver = GetGDALDriverManager()->GetDriverByName(pFormat);

	if(pDriver == NULL)
	{
		TRACE_ERROR("Could not load PYXIS GDAL Driver in GDALFileConverter");
		return false;
	}

	ppMetadata = pDriver->GetMetadata();

    if(!CSLFetchBoolean( ppMetadata, GDAL_DCAP_CREATECOPY, FALSE))
	{
		TRACE_ERROR("GDAL Driver does not support CreateCopy method.");
		return false;
	}

	// TODO: pConvertedDataset = ... as the process output

	GDALDatasetH pConvertedDataset = 0;
		pConvertedDataset = 
			pDriver->CreateCopy(m_strSavePath.c_str(), pDS, FALSE, NULL, NULL, NULL);
	if (pConvertedDataset == 0)
	{
		TRACE_ERROR("GDAL Create Copy failed for: " << getProcName());
		return false;
	}
	GDALClose(pConvertedDataset);

	// TODO: Verify the output file is not empty.
	return true;
}


