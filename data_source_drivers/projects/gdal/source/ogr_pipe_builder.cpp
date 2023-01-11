/******************************************************************************
ogr_pipe_builder.cpp

begin      : 12/07/2007 2:31:48 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#include "stdafx.h"
#define MODULE_GDAL_SOURCE
#include "ogr_pipe_builder.h"

// local includes
#include "exceptions.h"
#include "ogr_process.h"
#include "pyx_shared_gdal_data_set.h"

// pyxlib includes
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/procs/path.h"
#include "pyxis/procs/srs.h"
#include "pyxis/utility/file_utils.h"

// GDAL includes
#include "ogr_feature.h"
#include "ogrsf_frmts.h"

// boost includes
#include "boost/algorithm/string/case_conv.hpp"
#include "boost/algorithm/string/predicate.hpp"

// {B71E87AD-BBA7-4611-95E1-68AB241619C7}
PYXCOM_DEFINE_CLSID(OgrPipeBuilder, 
0xb71e87ad, 0xbba7, 0x4611, 0x95, 0xe1, 0x68, 0xab, 0x24, 0x16, 0x19, 0xc7);

PYXCOM_CLASS_INTERFACES(OgrPipeBuilder, IPipeBuilder::iid, PYXCOM_IUnknown::iid);

OgrPipeBuilder::OgrPipeBuilder()
{
	// Data source main file extensions	

	//[shatzi]: renable TEIGHA driver to support AutoCARD
	//m_vecSupportedExtensions.push_back("dxf");		// AutoCAD Drawing Exchange Files
	//m_vecSupportedExtensions.push_back("dwg");		// AutoCAD Drawing Files

	m_vecSupportedExtensions.push_back("e00");		// Arc/Info .E00 (ASCII) Coverage
	m_vecSupportedExtensions.push_back("gdbtable");	// ESRI File Geodatabase (single table)
	m_vecSupportedExtensions.push_back("geojson");	// GeoJSON file
	m_vecSupportedExtensions.push_back("gml");		// OGC Geography Markup Language
	m_vecSupportedExtensions.push_back("json");		// JSON file (GeoJSON)
	m_vecSupportedExtensions.push_back("kml");		// Keyhole Markup Language
	m_vecSupportedExtensions.push_back("shp");		// ESRI Shapefile
	m_vecSupportedExtensions.push_back("tab");		// MapInfo file

	// Multi-file data sources with supporting files
	{
		std::vector<std::string> extensions;

		// Shapefile required files
		extensions.clear();
		extensions.push_back("shx");
		extensions.push_back("dbf");
		m_vecRequiredFilesByExtensionAllOf["shp"] = extensions;

		// Shapefile optional files
		extensions.clear();
		extensions.push_back("prj");
		m_vecOptionalFilesByExtension["shp"] = extensions;
	}

	// Support files needed for VMAP
	m_vmapNeededFiles.push_back("cat");
	m_vmapNeededFiles.push_back("dqt");
	m_vmapNeededFiles.push_back("dqx");
	m_vmapNeededFiles.push_back("grt");
	m_vmapNeededFiles.push_back("lht");
	m_vmapNeededFiles.push_back("lineage.doc");
	m_vmapNeededFiles.push_back("..\\dht");
	m_vmapNeededFiles.push_back("..\\lat");

	// GDB folders and zipped folders handled in isDataSourceSupported()
}

/*!
Determine if a specific data source can be read by this PipeBuilder. Check if a GeoJSON file is
too large and handle special cases for ESRI File Geodatabases and VMAP directories.

\param strPath	The path to the data source. May be a file, directory or url.
\param options	The type of checking to perform.

\return true if the data source is supported, otherwise false.
*/
bool OgrPipeBuilder::isDataSourceSupported(const std::string& strPath, eCheckOptions options) const
{
	// the GDAL GeoJSON driver is unable to handle large files as it parses the entire file at once
	// limit the size of GeoJSON files to avoid corrupting memory.
	auto path = FileUtils::stringToPath(strPath);
	const std::string strExt = FileUtils::getExtensionNoDot(path);
	if (FileUtils::exists(path) && !FileUtils::isDirectory(path) &&
		(boost::iequals(strExt, "geojson") || boost::iequals(strExt, "json")))
	{
		// limit file size to 100MB
		boost::uintmax_t knMaxFileSize = 100 * 1024 * 1024;
		if (boost::filesystem::file_size(path) > knMaxFileSize)
		{
			return false;
		}

	}

	// handle checks for simple extensions
	if (PipeBuilderBase::isDataSourceSupported(strPath, options))
	{
		return true;
	}

	// ESRI File Geodatabases
	if (isESRIFileGeodatabase(strPath))
	{
		return true;
	}

	// VMAP0 directory
	if (isVMAP0Directory(strPath))
	{
		return true;
	}

	return false;
}

/*!
Check if the path represents an ESRI File Geodatabase. This may be a zip file with the extension
.gdb.zip or a directory with the extension .gdb.

\param strPath	The path to the data source.

\return true if the resource is supported, otherwise false.
*/
bool OgrPipeBuilder::isESRIFileGeodatabase(const std::string& strPath) const
{
	// work in lower case
	auto strPathLower = strPath;
	boost::algorithm::to_lower(strPathLower);
	boost::filesystem::path path = FileUtils::stringToPath(strPathLower);

	if (FileUtils::exists(path))
	{
		// check for a folder named *.gdb or *_gdb
		if (FileUtils::isDirectory(path))
		{
			std::vector<std::string> vecExtensions;
			vecExtensions.push_back(".gdb");
			vecExtensions.push_back("_gdb");

			for (auto it = vecExtensions.begin(); it != vecExtensions.end(); ++it)
			{
				if (boost::algorithm::ends_with(FileUtils::pathToString(path), *it))
				{
					return true;
				}
			}
		}
		else // or a zip file named *.gdb.zip, *_gdb.zip, *.gdb.tar, *_gdb.tar
		{
			std::vector<std::string> vecExtensions;
			vecExtensions.push_back(".gdb.zip");
			vecExtensions.push_back("_gdb.zip");
			vecExtensions.push_back(".gdb.tar");
			vecExtensions.push_back("_gdb.tar");

			for (auto it = vecExtensions.begin(); it != vecExtensions.end(); ++it)
			{
				if (boost::algorithm::ends_with(FileUtils::pathToString(path), *it))
				{
					return true;
				}
			}
		}
	}

	return false;
}

/*!
Check if the path represents an VMAP0 data source. This will be a directory containing files with
specific names.

\param path	The path to the data source.
\param options	The type of checking to perform.

\return true if the resource is supported, otherwise false.
*/
bool OgrPipeBuilder::isVMAP0Directory(const std::string& strPath) const
{
	// work in lower case
	auto strPathLower = strPath;
	boost::algorithm::to_lower(strPathLower);
	boost::filesystem::path path = FileUtils::stringToPath(strPathLower);

	// make sure this is a directory
	if (!FileUtils::isDirectory(path))
	{
		return false;
	}

	// make sure the diectory has all needed files
	for (unsigned int i = 0; i < m_vmapNeededFiles.size(); i++)
	{
		if (!boost::filesystem::is_regular_file(path / m_vmapNeededFiles[i]))
		{
			return false;
		}
	}

	return true;
}

/*!
Build a catalog describing the data source. This includes all data sets within
the data source and any sub-catalogs.

The default implementation simply returns a single catalog with one data set.

\param strPath	The path to the data source. May be a file, directory or url.

\return The catalog or nullptr if the data source was not supported.
*/
PYXPointer<const PYXCatalog> STDMETHODCALLTYPE OgrPipeBuilder::buildCatalog(const std::string& strPath) const
{
	try
	{
		auto strLeaf = FileUtils::pathToString(FileUtils::stringToPath(strPath).leaf());
		auto pCatalog = PYXCatalog::create();
		pCatalog->setUri(strPath);
		pCatalog->setName(strLeaf);

		// lenient is the minimum required to load a data set
		if (isDataSourceSupported(strPath, eCheckOptions::knLenient))
		{
			// record a data set for each layer in the data source
			auto pOGRDataSource = PYXSharedGDALDataSet::createVector(strPath);
			pCatalog->setName(pOGRDataSource->get()->GetDescription());

			// lock OGR context
			boost::recursive_mutex::scoped_lock lock(PYXSharedGDALDataSet::s_gdalScope);

			if (pOGRDataSource->isAutoCAD())
			{
				addAutoCADDataSets(pOGRDataSource, pCatalog, strPath, strLeaf);
			}
			else
			{
				// create a data set for each layer
				int nLayers = pOGRDataSource->get()->GetLayerCount();
				for (int i = 0; i < nLayers; i++)
				{
					OGRLayer* pLayer = pOGRDataSource->get()->GetLayer(i);

					// only return layers that we can display on the globe, not pure attribute layers
					if (pLayer->GetGeomType() != wkbNone)
					{
						auto pDataSet = PYXDataSet::create();
						pDataSet->setUri(strPath);
						auto strLayerName = std::string(pLayer->GetDescription());
						if (strLayerName.empty())
						{
							strLayerName = StringUtils::toString(i);
						}

						if (boost::iequals(strLayerName, "OGRGeoJSON"))
						{
							// all GeoJSON files have one layer named "OGRGeoJSON"
							// it adds nothing to the name so it is omitted
							pDataSet->setName(strLeaf);
						}
						else
						{
							pDataSet->setName(strLayerName + ": " + strLeaf);
						}
						pDataSet->setLayer(StringUtils::toString(i));

						// add data set metadata
						PYXOGRDataSource::addDataSetMetadata(pDataSet.get(), pOGRDataSource, pLayer, pLayer->GetFeatureCount(FALSE));

						// add field definitions
						PYXOGRDataSource::addContentDefinition(pLayer, pDataSet->getContentDefinition());

						// add missing optional files
						pDataSet->setMissingOptionalFiles(*findMissingOptionalFiles(strPath));

						try
						{
							OGRSpatialReference* pSourceRef = pLayer->GetSpatialRef();
							if (pSourceRef)
							{
								OGREnvelope fExt;
								pLayer->GetExtent(&fExt, TRUE);
								PYXOGRDataSource::correctOGREnvelope(&fExt);

								PYXRect2DDouble bounds;
								bounds.setXMin(fExt.MinX);
								bounds.setXMax(fExt.MaxX);
								bounds.setYMin(fExt.MinY);
								bounds.setYMax(fExt.MaxY);

								char* pPtr = nullptr;
								pSourceRef->exportToWkt(&pPtr);
								if (pPtr && !bounds.degenerate())
								{
									CoordConverterImpl coordConverter;
									std::string strWKT(pPtr);
									coordConverter.initialize(strWKT);
									OGRFree(pPtr); 

									PYXXYBoundsGeometry bbox(bounds, coordConverter, 24);
									pDataSet->setBoundingBox(bbox);
								}
							}
						}
						catch(...)
						{
							
						}

						pCatalog->addDataSet(pDataSet);
					}
				}
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
		TRACE_ERROR("Unable to create catalog for " << strPath << ". ");

		// fall through
	}

	return nullptr;
}

/*!
Add the datasets for an AutoCAD data source to a catalog.

\param pDataSet	Describes the data set to be opened.
\param pCatalog	The catalog where the datasets are added.
\param strPath	The full path of the data source
\param strLeaf	The file name of the data source
*/
void OgrPipeBuilder::addAutoCADDataSets(
	PYXPointer<PYXSharedGDALDataSet> pOGRDataSource,
	PYXPointer<PYXCatalog> pCatalog,
	const std::string& strPath,
	const std::string& strLeaf	) const
{
	assert(nullptr != pOGRDataSource && "data source must not be null");
	assert(nullptr != pCatalog && "catalog must not be null");

	// GDAL puts all AutoCAD entities into a single layer called
	// "entities" with the AutoCAD layer identified by the "Layer" attribute

	// iterate through all features in the "entities" layer to get the layer names
	OGRLayer* pLayer = pOGRDataSource->get()->GetLayerByName("entities");
	if (pLayer != NULL)
	{
		OGRFeatureDefn* pLayerDefn = pLayer->GetLayerDefn();
		if (pLayerDefn != NULL)
		{
			// get the index of the layer attribute
			int nFieldIndex = pLayerDefn->GetFieldIndex("Layer");
			if (nFieldIndex >= 0)
			{
				// keep track of the unique layer names and number of features in each layer
				std::map<std::string, int> layers;

				// for each feature
				pLayer->SetAttributeFilter(NULL);
				pLayer->SetSpatialFilter(NULL);
				pLayer->ResetReading();

				OGRFeature* pFeature = pLayer->GetNextFeature();
				while (pFeature != NULL)
				{
					// get the layer name from the attribute
					const char* pstrLayerName = pFeature->GetFieldAsString(nFieldIndex);
					if (pstrLayerName != 0)
					{
						auto it = layers.find(pstrLayerName);
						if (it == layers.end())
						{
							layers.insert(std::pair<std::string, int>(pstrLayerName, 1));
						}
						else
						{
							it->second++;
						}
					}

					OGRFeature::DestroyFeature(pFeature);
					pFeature = pLayer->GetNextFeature();
				}

				// create a data set for each layer
				for (auto it = layers.begin(); it != layers.end(); ++it)
				{
					auto pDataSet = PYXDataSet::create();
					pDataSet->setUri(strPath);
					pDataSet->setName(it->first + ": " + strLeaf);
					pDataSet->setLayer(it->first);

					// add data set metadata
					PYXOGRDataSource::addDataSetMetadata(pDataSet.get(), pOGRDataSource, pLayer, it->second);

					// add content metadata
					PYXOGRDataSource::addContentDefinition(pLayer, pDataSet->getContentDefinition());

					pCatalog->addDataSet(pDataSet);
				}
			}
		}
	}
}
/*!
Create a process that reads a layer from an OGR data source.

\param pDataSet	Describes the data set to be opened.

\return The process or null if the process could not be created.
*/
boost::intrusive_ptr<OGRProcess> OgrPipeBuilder::createOGRProcess(PYXPointer<PYXDataSet> pDataSet) const
{
	try 
	{
		boost::intrusive_ptr<IProcess> spFilePathProc = PYXCOMCreateInstance<IProcess, PathProcess>();		
		
		if (spFilePathProc)
		{
			auto path = FileUtils::stringToPath(pDataSet->getUri());
			const std::string& strDataSetName = pDataSet->getName();
			const std::string& strLayerName = pDataSet->getLayer();

			// Special handling for VMAP data sources
			auto uri = pDataSet->getUri();
			if (isVMAP0Directory(uri))
			{
				uri = "gltp:/vrf/" + uri;
				std::replace(uri.begin(), uri.end(), '\\','/');
			}

			std::map<std::string, std::string> mapAttr;
			mapAttr["uri"] = uri;
			if (FileUtils::isDirectory(path))
			{
				mapAttr["is_directory"] = "true";
			}
			spFilePathProc->setAttributes(mapAttr);

			// add all supporting files to the path proc
			addSupportingFiles(spFilePathProc, path);

			boost::intrusive_ptr<OGRProcess> spOgrProcess = boost::intrusive_ptr<OGRProcess> (new OGRProcess());				
			if (spOgrProcess)
			{
				// initialize process variables
				spOgrProcess->getParameter(0)->addValue(spFilePathProc);
				spOgrProcess->setProcName(pDataSet->getName());

				std::map<std::string, std::string> mapAttr;
				mapAttr.clear();
				mapAttr["res"] = "24";					// arbitrary resolution picked for default.
				mapAttr["canRasterize"]= "false";		// we want to show icons, so no we can't rasterize.
				mapAttr["layer_name"] = strLayerName;	// set layer name
				mapAttr["layer"] = StringUtils::isNumeric(strLayerName) ? strLayerName : "0";	// set layer number
				spOgrProcess->setAttributes(mapAttr);

				// parent process must call OGRProcess::initProc(true) to ensure the OGRProcess is initialized.
				return spOgrProcess;
			}
		}
	}
	catch (PYXException&)
	{
		// fall through
	}

	TRACE_INFO("Unable to build an OGR pipeline for: " << pDataSet->getUri());
	return boost::intrusive_ptr<OGRProcess>();
}

/*!
Build a pipeline for the data source.

\param pDataSet	Describes the data set to be opened.

\return The head process of the pipeline or nullptr if the pipeline was unable to be built.
*/
PYXPointer<IProcess> OgrPipeBuilder::buildPipeline(PYXPointer<PYXDataSet> pDataSet) const
{
	try 
	{
		if (isDataSourceSupported(pDataSet->getUri(), eCheckOptions::knLenient))
		{
			boost::intrusive_ptr<OGRProcess> spOgrProcess = createOGRProcess(pDataSet);

			if (spOgrProcess)
			{
				boost::intrusive_ptr<IProcess> spFeatureSummaryProcess;
				PYXCOMCreateInstance(strToGuid("{E6C3802D-E7B3-431c-A41F-FBAB79E1CA2D}"),
					0, IProcess::iid, (void**) &spFeatureSummaryProcess);

				boost::intrusive_ptr<IProcess> spApplyStyleProcess;
				PYXCOMCreateInstance(strToGuid("{4F41A149-7EBF-41cb-B0F9-031D76EF81E0}"),
					0, IProcess::iid, (void**) &spApplyStyleProcess);

				if (spFeatureSummaryProcess && spApplyStyleProcess)
				{
					//[shatzi: 19 March 2012]
					//HACK!!!!
					//before we continue adding features summary - make sure the identity is current...
					PipeUtils::waitUntilPipelineIdentityStable(spOgrProcess);
				
					spFeatureSummaryProcess->getParameter(0)->addValue(spOgrProcess);
					spApplyStyleProcess->getParameter(0)->addValue(spFeatureSummaryProcess);
					spApplyStyleProcess->setProcName(pDataSet->getName());
					
					if (!pDataSet->getLayer().empty())
					{
						spApplyStyleProcess->setProcDescription("Information layer '" + pDataSet->getLayer() + "'");
					}

					spFeatureSummaryProcess->setProcName(pDataSet->getName());
					spFeatureSummaryProcess->setProcDescription(spApplyStyleProcess->getProcDescription());

					return spApplyStyleProcess;
				}
			}
		}
	}
	catch (PYXException&)
	{
		// fall through
	}

	TRACE_INFO("Unable to build a pipeline for: " << pDataSet->getUri());
	return nullptr;
}
