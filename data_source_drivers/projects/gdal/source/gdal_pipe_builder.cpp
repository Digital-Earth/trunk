/***************************************************************************
gdal_pipe_builder.cpp

begin		: 2007-12-03
copyright	: (C) 2007 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
***************************************************************************/
#include "stdafx.h"
#define MODULE_GDAL_SOURCE
#include "gdal_pipe_builder.h"

#include "exceptions.h"
#include "gdal_file_process.h"
#include "pyx_shared_gdal_data_set.h"

#include "pyxis/data/exceptions.h"
#include "pyxis/procs/path.h"
#include "pyxis/procs/srs.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/pipe/pipe_utils.h"

// boost includes
#include <boost/algorithm/string.hpp>

// standard includes
#include <numeric>

// {D3215B21-DA1A-4d8b-8689-D567D4C67918}
PYXCOM_DEFINE_CLSID(GDALPipeBuilder, 
					0xd3215b21, 0xda1a, 0x4d8b, 0x86, 0x89, 0xd5, 0x67, 0xd4, 0xc6, 0x79, 0x18);

PYXCOM_CLASS_INTERFACES(GDALPipeBuilder, IPipeBuilder::iid, PYXCOM_IUnknown::iid);


// Tester class
Tester<GDALPipeBuilder> gTester;

std::string vectorToCSV(const std::vector<int>& input);
std::string vectorToCSV(const std::vector<std::string>& input);

// Test method
void GDALPipeBuilder::test()
{
	static const std::string arr[] = {"16","2","77","29" };
	std::vector<std::string> vec (arr, arr + sizeof(arr) / sizeof(arr[0]) );
	auto csvStr = vectorToCSV(vec);
	std::cerr << csvStr << std::endl;
	TRACE_TEST(csvStr);
	TEST_ASSERT(csvStr== "16,2,77,29");
}

/*!
Default constructor initializes the GDAL pipebuilder with a list of file extensions that 
are supported by this reader.
*/
GDALPipeBuilder::GDALPipeBuilder()
{
	// Main data source file extensions
	{
		m_vecSupportedExtensions.push_back("asc");		// Arc/Info ASCII Grid
		m_vecSupportedExtensions.push_back("cdf");		// Network Common Data Form (NetCDF)
		m_vecSupportedExtensions.push_back("ddf");		// USGS SDTS DEM
		m_vecSupportedExtensions.push_back("dem");		// USGS ASCII DEM / CDED
		m_vecSupportedExtensions.push_back("dt0");		// Military Elevation Data
		m_vecSupportedExtensions.push_back("dt1");		// Military Elevation Data
		m_vecSupportedExtensions.push_back("dt2");		// Military Elevation Data
		m_vecSupportedExtensions.push_back("gif");		// Graphic Interchange Format
		m_vecSupportedExtensions.push_back("grb");		// World Meteorological Organization Gridded Binary (GRIB1)
		m_vecSupportedExtensions.push_back("grib");		// World Meteorological Organization Gridded Binary (GRIB1)
		m_vecSupportedExtensions.push_back("grd");		// Northwood/VerticalMapper Numeric Grid Format
		m_vecSupportedExtensions.push_back("grb2");		// World Meteorological Organization Gridded Binary (GRIB2)
		m_vecSupportedExtensions.push_back("grib2");	// World Meteorological Organization Gridded Binary (GRIB2)
		m_vecSupportedExtensions.push_back("img");		// Erdas Imagine
		m_vecSupportedExtensions.push_back("jpg");		// Joint Photographic Experts Group
		m_vecSupportedExtensions.push_back("nc");		// Network Common Data Form (NetCDF)
		m_vecSupportedExtensions.push_back("nc2");		// Network Common Data Form (NetCDF)
		m_vecSupportedExtensions.push_back("nc3");		// Network Common Data Form (NetCDF)
		m_vecSupportedExtensions.push_back("nc4");		// Network Common Data Form (NetCDF)
		m_vecSupportedExtensions.push_back("ntf");		// NITF
		m_vecSupportedExtensions.push_back("png");		// Portable Network Graphics
		m_vecSupportedExtensions.push_back("sid");		// Multi-resolution Seamless Image Database (MrSID)
		m_vecSupportedExtensions.push_back("tif");		// Tagged Image File Format (TIFF) files may include geospatial information
		m_vecSupportedExtensions.push_back("tiff");		// Tagged Image File Format (TIFF) files may include geospatial information
		m_vecSupportedExtensions.push_back("tl2");		// Raster Product Format/RPF (CADRG, CIB)
		//	m_vecSupportedExtensions.push_back("toc");	// ?
		//	m_vecSupportedExtensions.push_back("vrt");  // GDAL specific file that wraps up a dataset decription as XML
	}

	// Multi-file data sources with supporting files
	{
		std::vector<std::string> extensions;

		// GIF requires supporting files
		extensions.clear();
		extensions.push_back("gfw");
		extensions.push_back("gifw");
		extensions.push_back("wld");
		m_vecRequiredFilesByExtensionOneOf["gif"] = extensions;

		// JPG requires supporting files
		extensions.clear();
		extensions.push_back("jpw");
		extensions.push_back("jpgw");
		extensions.push_back("jpegw");
		extensions.push_back("wld");
		m_vecRequiredFilesByExtensionOneOf["jpg"] = extensions;

		// PNG requires supporting files
		extensions.clear();
		extensions.push_back("pgw");
		extensions.push_back("pngw");
		extensions.push_back("wld");
		m_vecRequiredFilesByExtensionOneOf["png"] = extensions;

		// TIF and TIFF may require supporting files
		extensions.clear();
		extensions.push_back("tfw");
		extensions.push_back("tifw");
		extensions.push_back("tiffw");
		extensions.push_back("wld");
		m_vecOptionalFilesByExtension["tif"] = extensions;
		m_vecOptionalFilesByExtension["tiff"] = extensions;
	}

	// Folder-based data sources with required files
	{
		// Arc/Info ASCII Grid (ADF) is a folder containing a "hdr.adf" file
	}
}

/*!
Determine if a specific data source can be read by this PipeBuilder.

The data source is supported if:

1. strPath is a single file and the data source consists of a single file
2. strPath is a single file, is the main file in a multi-file data source and
	2.1 all required files are present (options == knPermissive or options == knRestrictive) and
	2.2 all optional files are present (options == knRestrictive)
3. strPath is a folder representing a folder based data source

\param strPath	The path to the data source. May be a file, folder or url.
\param options	The type of checking to perform.

\return true if the data source is supported, otherwise false.
*/
bool GDALPipeBuilder::isDataSourceSupported(const std::string& strPath, eCheckOptions options) const
{
	// Cases 1 & 2: simple file extension checks
	if (PipeBuilderBase::isDataSourceSupported(strPath, options))
	{
		return true;
	}

	// An exception will be thrown if file or folder is deleted before isDirectory is called, protect with try/catch
	try
	{
		auto path = FileUtils::stringToPath(strPath);
		if (FileUtils::exists(path))
		{
			if (FileUtils::isDirectory(path))
			{
				// Case 3: check the contents of the folder for required file(s)
				if (FileUtils::exists(path / "hdr.adf"))
				{
					// folder is an ADF data source
					return true;
				}
			}
		}
	}
	catch (...)
	{
		// fall through
	}

	return false;
}

/*!
Open a data source, and traverse the metadata for subdatasets, parsing out the name and description
information for each matching subdataset returned by the original query.

\param strPath	Path to the data source

\return PYXDatasetCatalog describing the data sets in the data source or nullptr if no data sets.
*/
PYXPointer<const PYXCatalog> STDMETHODCALLTYPE  GDALPipeBuilder::buildCatalog(const std::string& strPath) const
{
	try
	{
		// create catalog
		auto pCatalog = PYXCatalog::create();
		pCatalog->setUri(strPath);
		auto strLeaf = FileUtils::pathToString(FileUtils::stringToPath(strPath).leaf());
		pCatalog->setName(strLeaf);

		// lenient is the minimum required to load a data set
		if (isDataSourceSupported(strPath, eCheckOptions::knLenient))
		{
			std::string gdalPath = strPath;

			if (boost::starts_with(strPath,"http") && boost::ends_with(strPath,".nc"))
			{
				gdalPath = "NETCDF:" + gdalPath;
			}
			// record a data set for each layer in the data source
			auto pGDALDataSet = PYXSharedGDALDataSet::createRaster(gdalPath);

			char** pMetadata = GDALGetMetadata(pGDALDataSet->get(), "SUBDATASETS");
			if (CSLCount(pMetadata) > 0)
			{
				for (int nIdx = 0; pMetadata[nIdx] != NULL; nIdx++)
				{
					std::string metadata = pMetadata[nIdx];
					int nPos = metadata.find('=');

					std::string lhs = metadata.substr(0,nPos);
					std::string rhs = metadata.substr(nPos+1);

					nPos = lhs.find_last_of('_');
					lhs = lhs.substr(nPos+1);

					if (lhs == "NAME")
					{
						try
						{
							// open the sub-dataset
							auto pSubGDALDataSet = PYXSharedGDALDataSet::createRaster(rhs);
							addDataSets(pCatalog, strPath, pSubGDALDataSet);
						}
						catch (PYXException& e)
						{
							TRACE_ERROR(e.getFullErrorString());

							// if we are unable to open the GDAL driver for the data set, just skip it,
							// while including the other layers that succeeded in the catalog
						}
					}
				}
			}
			else
			{
				// GDAL data source has no layers, create a data set for the top level
				addDataSets(pCatalog, strPath, pGDALDataSet);
			}
		}
		else if (isDataSourceSupported(strPath, eCheckOptions::knPartial))
		{
			// we failed the lenient test, return a data set with missing files if this is a partial data set
			auto pDataSet = PYXDataSet::create(strPath, strLeaf);

			// record missing files
			pDataSet->setMissingRequiredFilesAllOf(*findMissingRequiredFilesAllOf(strPath));
			pDataSet->setMissingRequiredFilesOneOf(*findMissingRequiredFilesOneOf(strPath));
			pDataSet->setMissingOptionalFiles(*findMissingOptionalFiles(strPath));

			pCatalog->addDataSet(pDataSet);
		}

		return (pCatalog->getDataSetCount() > 0) ? pCatalog : nullptr;
	}
	catch (PYXException&)
	{
		// fall through
	}

	return nullptr;
}

/*!
Add Pyxis data sets for a GDAL data set to a specified catalog.

\param pCatalog	The catalog
\param strPath	Path to the data source
\param pGDALDataSet	The GDAL data set (ownership retained by caller)
*/
void GDALPipeBuilder::addDataSets(
	PYXPointer<PYXCatalog> pCatalog,
	const std::string& strPath,
	PYXPointer<PYXSharedGDALDataSet> pGDALDataSet	) const
{
	assert(pCatalog != nullptr);
	assert(pGDALDataSet != NULL);

	auto pDataSet = PYXDataSet::create();
	pDataSet->setUri(strPath);

	auto strLeaf = FileUtils::pathToString(FileUtils::stringToPath(strPath).leaf());

	// netCDF layers are named as NETCDF:"filePath":layerName. Here we extract the layerName.
	auto strName = std::string (pGDALDataSet->get()->GetDescription());
	if (boost::iequals(strName.substr(0, 6), "netcdf"))
	{
		auto strLayerName = strName.substr(strName.find_last_of(':') + 1);

		pDataSet->setName(strLayerName + ": " + strLeaf);
		pDataSet->setLayer(strLayerName);
	}
	else
	{
		// in this case, GDALDataSet::GetDescription() returns the full filename with path
		// so we display only the filename
		pDataSet->setName(strLeaf);

		// leave layer name empty for single layer NetCDF files
		if (!pGDALDataSet->isNetCDF())
		{
			pDataSet->setLayer(strLeaf);
		}
	}

	try
	{
		std::string strWKT = pGDALDataSet->get()->GetProjectionRef();
		if (strWKT.empty())
		{
			strWKT = pGDALDataSet->get()->GetGCPProjection();
		}

		if (!strWKT.empty())
		{
			GDALMetaData metaDataGDAL;
			metaDataGDAL.initialize(*pGDALDataSet->get(),-1);

			PYXRect2DDouble bounds;
			bounds.setEmpty();
			bounds.expand(metaDataGDAL.getSouthEast());
			bounds.expand(metaDataGDAL.getSouthWest());
			bounds.expand(metaDataGDAL.getNorthEast());
			bounds.expand(metaDataGDAL.getNorthWest());	

			CoordConverterImpl coordConverter;
			coordConverter.initialize(strWKT);

			PYXXYBoundsGeometry bbox(bounds, coordConverter, 24);
			pDataSet->setBoundingBox(bbox);
		}
	}
	catch(...)
	{
		//failed to get bbox
	}

	try
	{
		// special handling for netCDF and GRIB data files, each band becomes a dataset
		if (pGDALDataSet->isNetCDF() || pGDALDataSet->isGRIB())
		{
			// GDAL bands go from 1..n
			auto numberOfBands = pGDALDataSet->get()->GetRasterCount();
			for (int i = 1; i <= numberOfBands; i++)
			{
				auto pBand = pGDALDataSet->get()->GetRasterBand(i);
				if (pBand != NULL)
				{
					auto pBandDataSet = pDataSet->clone();
					pBandDataSet->setName(std::to_string(i) + ": " + pBandDataSet->getName());

					std::vector<int> vecBands;
					vecBands.push_back(i);
					auto nContext = GDALXYCoverage::calcContext(pGDALDataSet, pBand, vecBands.size());

					GDALXYCoverage::addDataSetMetadata(pBandDataSet.get(), pGDALDataSet, pBand, nContext); 

					auto nGDALDataType = pBand->GetRasterDataType();
					GDALXYCoverage::addContentDefinition(
						pBandDataSet->getContentDefinition(),
						vecBands,
						vectorToCSV(vecBands),
						nContext,
						nGDALDataType);

					pCatalog->addDataSet(pBandDataSet);
				}
			}
		}
		else
		{
			// 1, 3 or 4 total bands are supported
			auto nBands = pGDALDataSet->get()->GetRasterCount();
			if (0 < nBands)
			{
				if (1 != nBands && 3 != nBands && 4 != nBands)
				{
					nBands = 1;
				}

				// GDAL bands go from 1..n
				std::vector<int> vecBands(nBands);
				std::iota(vecBands.begin(), vecBands.end(), 1);

				auto pBand = pGDALDataSet->get()->GetRasterBand(vecBands[0]);
				if (pBand != NULL)
				{
					auto nContext = GDALXYCoverage::calcContext(pGDALDataSet, pBand, vecBands.size());
					GDALXYCoverage::addDataSetMetadata(pDataSet.get(), pGDALDataSet, pBand, nContext);

					auto nGDALDataType = pBand->GetRasterDataType();
					GDALXYCoverage::addContentDefinition(
						pDataSet->getContentDefinition(),
						vecBands,
						vectorToCSV(vecBands),
						nContext,
						nGDALDataType);

					// record any missing optional files
					pDataSet->setMissingOptionalFiles(*findMissingOptionalFiles(strPath));

					pCatalog->addDataSet(pDataSet);
				}
			}
		}
	}
	catch (...)
	{
		// fall through
		TRACE_INFO("An error occurred getting number of bands or raster size for dataset:" << pDataSet->getName());
	}
}

/*!
Build a pipeline for the data source.

\param pDataSet  Describes the data set to be opened.

\return The head process of the pipeline or nullptr if the pipeline was unable to be built.
*/
PYXPointer<IProcess> GDALPipeBuilder::buildPipeline(PYXPointer<PYXDataSet> pDataSet) const 
{
	if (!isDataSourceSupported(pDataSet->getUri(), eCheckOptions::knLenient))
	{
		return nullptr;
	}

	boost::intrusive_ptr<IProcess> spGdalProcess = createGdalProcess(pDataSet);

	if (spGdalProcess)
	{
		boost::intrusive_ptr<IPath> spPath;
		spGdalProcess->getParameter(0)->getValue(0)->QueryInterface(
			IPath::iid, (void**) &spPath);

		boost::intrusive_ptr<PYXCOM_IUnknown> spIUnknown = spGdalProcess->getOutput();
		boost::intrusive_ptr<GDALXYCoverage> spGDalXY;
		spIUnknown->QueryInterface(IXYCoverage::iid, (void**) &spGDalXY);
		PYXFieldDefinition::eContextType eContext = spGDalXY->getContext();

		auto path = FileUtils::stringToPath(pDataSet->getUri());
		auto fileName = FileUtils::pathToString(path.leaf());
		bool isDEMFile = (boost::iequals(FileUtils::getExtensionNoDot(path), "dem"));

		if (isDEMFile && hasUnsignedValues(spGdalProcess))
		{
			boost::intrusive_ptr<IProcess> translateProcess = createUnsignedToSignedTranslationProcess(spGdalProcess);

			if (translateProcess)
			{
				translateProcess->getParameter(0)->addValue(spGdalProcess);
				translateProcess->setProcName(pDataSet->getName());

				//if we created the translation successfully, use it.
				if (translateProcess->initProc(false) == IProcess::knInitialized)
				{
					//make the spGDalProcess to point to the output of the translateProcess.
					spGdalProcess = translateProcess;
				}
			}
		}

		//Auto cast into float to make high resolution elevation mesh much nicer
		if (isDEMFile && !isFloatValues(spGdalProcess))
		{
			boost::intrusive_ptr<IProcess> castedDEM = addCastValuesToFloatProcess(spGdalProcess);

			if (castedDEM)
			{
				castedDEM->setProcName(pDataSet->getName());
				spGdalProcess = castedDEM;
			}
		}

		boost::intrusive_ptr<IProcess> spDefaultSampler = getDefaultSampler();
		spDefaultSampler->getParameter(0)->addValue(spGdalProcess);
		spDefaultSampler->setProcName(pDataSet->getName());

		boost::intrusive_ptr<IProcess> spCovCacheProc; 
		PYXCOMCreateInstance(strToGuid("{83F35C37-5D0A-41c9-A937-F8C9C1E86850}"), 0, IProcess::iid, (void**)&spCovCacheProc);
		spCovCacheProc->setProcName(pDataSet->getName());

		// plug the sampler into the cache
		spCovCacheProc->getParameter(0)->addValue(spDefaultSampler);

		if (hasSimpleGreyscaleValues(spGdalProcess) && eContext == PYXFieldDefinition::knContextGreyScale)
		{
			//add default style to the process.
			return ProcessInitHelper("{43FFAAE3-0A08-45f8-80DA-2E75B31EB96F}")
				.addInput(0,spCovCacheProc)
				.setProcName(pDataSet->getName())
				.borrowNameAndDescription(spCovCacheProc)
				.setAttribute("show_as_elevation","false")
				.setAttribute("palette","2  0 0 0 0 255  255 255 255 255 255")
				.getProcess(false);
		}

		// return the cache as the process
		return spCovCacheProc;
	}

	return nullptr;
}

/*!
Convert a vector of integers to a CSV string

\param input	A vector of integers

\return	CSV string
*/
std::string vectorToCSV(const std::vector<int>& input)
{
	std::string result;
	for (auto it = input.begin(); it != input.end(); ++it)
	{
		if (it != input.begin()) 
		{
			result += ",";
		}
		result += StringUtils::toString(*it);
	}
	return result;
}

/*!
Convert a vector of strings to a CSV string

\param input	A vector of strings

\return	CSV string
*/
std::string vectorToCSV(const std::vector<std::string>& input)
{
	std::string result;
	for (auto it = input.begin(); it != input.end(); ++it)
	{
		if (it != input.begin()) 
		{
			result += ",";
		}
		result += *it;
	}
	return result;
}

/*!
Creates a GDAL File reader process with the appropriate path. 

\param pDataSet	Describes the data set to be opened.
\param layer	The layer identification for the coverage (empty "" if there is only one layer) 

\return A boost intrusive pointer to a file GDALFile process to go into pipeline if successful
otherwise a null pointer.
*/
boost::intrusive_ptr<IProcess> GDALPipeBuilder::createGdalProcess(PYXPointer<PYXDataSet> pDataSet) const
{
	boost::intrusive_ptr<GDALFileProcess> spGDalProc (new GDALFileProcess);

	if (!spGDalProc)
	{
		TRACE_INFO("Unable to initialize a GDAL pipeline for: " << pDataSet->getUri());
		return boost::intrusive_ptr<IProcess>();
	}

	auto path = FileUtils::stringToPath(pDataSet->getUri());
	auto fileName = FileUtils::pathToString(path.leaf());
	bool isDEMFile = (boost::iequals(FileUtils::getExtensionNoDot(path), "dem"));

	// create and initialize a path process
	boost::intrusive_ptr<IProcess> spPathProc; 
	PYXCOMCreateInstance(PathProcess::clsid, 0, IProcess::iid, (void**) &spPathProc);
	std::map<std::string, std::string> mapAttr;
	mapAttr["uri"] = FileUtils::pathToString(path);
	if (FileUtils::isDirectory(path))
	{
		mapAttr["is_directory"] = "true";
	}
	spPathProc->setAttributes(mapAttr);
	spPathProc->setProcName("File Process: " + fileName);

	// add any required and optional files to the path proc
	addSupportingFiles(spPathProc, path);

	spGDalProc->getParameter(0)->addValue(spPathProc);
	std::map<std::string, std::string> spGDalProcMapAttr;
	spGDalProcMapAttr["LayerName"] = pDataSet->getLayer();
	spGDalProcMapAttr["Bands"] = vectorToCSV(pDataSet->getContentDefinition()->getFieldNames());
	spGDalProc->setAttributes(spGDalProcMapAttr);
	spGDalProc->setProcName(pDataSet->getName());

	//Add default no data value for dem files (case insenstive)
	if (isDEMFile)
	{
		mapAttr.clear();
		mapAttr["ForcedNullValue"] = "int16_t -9999";
		spGDalProc->setAttributes(mapAttr);
	}

	if (	spGDalProc->initProc(true) == IProcess::knInitialized ||
			PipeUtils::findFirstError(spGDalProc, guidToStr(GDALSRSInitError::clsid)) ||
			PipeUtils::findFirstError(spGDalProc, guidToStr(MissingGeometryInitError::clsid)) ||
			PipeUtils::findFirstError(spGDalProc, guidToStr(MissingWorldFileInitError::clsid))	)
	{
		return spGDalProc;
	}	

	TRACE_INFO("Unable to initialize a GDAL pipeline for: " << FileUtils::pathToString(path));
	return nullptr;	
}

bool GDALPipeBuilder::hasSimpleGreyscaleValues(boost::intrusive_ptr<IProcess> spGDalProcess) const
{
	if (!spGDalProcess->getOutput())
	{
		return false;
	}

	boost::intrusive_ptr<IXYCoverage> inputCoverage = spGDalProcess->getOutput()->QueryInterface<IXYCoverage>();
	return inputCoverage->getCoverageDefinition()->getFieldDefinition(0).getType() == PYXValue::knUInt8;
}


bool GDALPipeBuilder::hasUnsignedValues(boost::intrusive_ptr<IProcess> spGDalProcess) const
{	
	boost::intrusive_ptr<IXYCoverage> inputCoverage;
	spGDalProcess->getOutput()->QueryInterface(IXYCoverage::iid, (void**) &inputCoverage);

	if (inputCoverage)
	{
		switch (inputCoverage->getCoverageDefinition()->getFieldDefinition(0).getType())
		{
		case PYXValue::knUInt8:	
		case PYXValue::knUInt16:
		case PYXValue::knUInt32:
			return true;
		}
	}

	return false;
}

bool GDALPipeBuilder::isFloatValues(boost::intrusive_ptr<IProcess> spGDalProcess) const
{	
	boost::intrusive_ptr<IXYCoverage> inputCoverage;
	spGDalProcess->getOutput()->QueryInterface(IXYCoverage::iid, (void**) &inputCoverage);

	if (inputCoverage)
	{
		switch (inputCoverage->getCoverageDefinition()->getFieldDefinition(0).getType())
		{
		case PYXValue::knFloat:
		case PYXValue::knDouble:
			return true;
		}
	}

	return false;
}



boost::intrusive_ptr<IProcess> GDALPipeBuilder::createUnsignedToSignedTranslationProcess(boost::intrusive_ptr<IProcess> spGDalProcess) const
{
	boost::intrusive_ptr<IXYCoverage> inputCoverage;
	spGDalProcess->getOutput()->QueryInterface(IXYCoverage::iid, (void**) &inputCoverage);

	if (inputCoverage)
	{
		boost::intrusive_ptr<IProcess> translateProcess;
		//create XyCoverageTranslator process
		PYXCOMCreateInstance(strToGuid("{B26A382D-151C-4149-8F03-00C1740E56B4}"), 0, IProcess::iid, (void**)&translateProcess);

		std::map<std::string, std::string> mapAttr;
		mapAttr["Operation"] = "Reinterpret";

		PYXValue::eType coverageType = inputCoverage->getCoverageDefinition()->getFieldDefinition(0).getType();
		switch (coverageType)
		{
		case PYXValue::knUInt8:
			mapAttr["OutputType"] = "int8_t";
			break;
		case PYXValue::knUInt16:
			mapAttr["OutputType"] = "int16_t";
			break;
		case PYXValue::knUInt32:
			mapAttr["OutputType"] = "int32_t";
			break;
		default:
			PYXTHROW(PYXException,"Got unexpetected type for input coverage: " << PYXValue::getTypeAsString(coverageType));
		}

		translateProcess->setAttributes(mapAttr);

		return translateProcess;
	}

	return boost::intrusive_ptr<IProcess>();
}

boost::intrusive_ptr<IProcess> GDALPipeBuilder::addCastValuesToFloatProcess(boost::intrusive_ptr<IProcess> spGDalProcess) const
{
	boost::intrusive_ptr<IXYCoverage> inputCoverage;
	spGDalProcess->getOutput()->QueryInterface(IXYCoverage::iid, (void**) &inputCoverage);

	if (inputCoverage)
	{
		boost::intrusive_ptr<IProcess> translateProcess;
		//create XyCoverageTranslator process
		PYXCOMCreateInstance(strToGuid("{B26A382D-151C-4149-8F03-00C1740E56B4}"), 0, IProcess::iid, (void**)&translateProcess);

		std::map<std::string, std::string> mapAttr;
		mapAttr["Operation"] = "Cast";
		mapAttr["OutputType"] = "float";

		translateProcess->setAttributes(mapAttr);

		translateProcess->getParameter(0)->addValue(spGDalProcess);

		return translateProcess;
	}

	return boost::intrusive_ptr<IProcess>();
}
