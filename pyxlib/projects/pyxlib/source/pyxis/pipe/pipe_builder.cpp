/******************************************************************************
pipe_builder.cpp

begin      : 08/04/2007 1:07:01 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pipe_builder.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"

// boost includes
#include <boost/algorithm/string.hpp>

// {6B9172C1-0847-47a2-9A79-DFDADD2A740D}
PYXCOM_DEFINE_IID(IPipeBuilder, 
				  0x6b9172c1, 0x847, 0x47a2, 0x9a, 0x79, 0xdf, 0xda, 0xdd, 0x2a, 0x74, 0xd);

//The file name for the processed file name.
const std::string PipeBuilderBase::kstrPipeBuilderIni = "\\ProcessedFiles.ini";
const std::string PipeBuilderBase::kstrPipeBuilderPropScope = "PipeBuilderDefault";
const std::string PipeBuilderBase::kstrPipeBuilderPropKey = "DefaultSampler";
const std::string PipeBuilderBase::kstrPipeBuilderDefaultPropValue = "{E414C479-4E9A-423F-AA17-6ABF54967705}";//"{03A11842-7EE9-400e-9CA1-A4942E99E8FE}";
const std::string PipeBuilderBase::kstrPipeBuilderPropDescription = "Default XY Sampler for automated pipe building";


void IPipeBuilder::setConfig(const std::string& key, const std::string value)
{
	//DO NOTHING
}


// Constructor
PipeBuilderBase::PipeBuilderBase() : m_defaultSamplerGuid("{E414C479-4E9A-423F-AA17-6ABF54967705}")
{
}

/*!
Determine if a specific data source can be read by this PipeBuilder. Check if the file extension
is supported and if all required files are present.

\param strPath	The path to the data source. May be a file, directory or url.
\param options	The check options.

\return true if the data source is supported and all required files are present, otherwise false.
*/
bool PipeBuilderBase::isDataSourceSupported(const std::string& strPath, eCheckOptions options) const
{
	// check that path is a file
	boost::filesystem::path path = FileUtils::stringToPath(strPath);
	if (FileUtils::exists(path) && !FileUtils::isDirectory(path))
	{
		// get the file extension
		std::string strExt(FileUtils::getExtensionNoDot(path));
		boost::algorithm::to_lower(strExt);

		// check main file
		if (std::find(m_vecSupportedExtensions.begin(), m_vecSupportedExtensions.end(), strExt) == m_vecSupportedExtensions.end())
		{
			return false;
		}

		// check required files
		if (options == eCheckOptions::knLenient || options == eCheckOptions::knStrict)
		{
			// check required files where all files must be present
			auto it = m_vecRequiredFilesByExtensionAllOf.find(strExt);
			if (it != m_vecRequiredFilesByExtensionAllOf.end())
			{
				auto pvecMissingRequiredFiles = FileUtils::findMissingFiles(strPath, it->second);
				if (!pvecMissingRequiredFiles->empty())
				{
					return false;
				}
			}

			// check required files where one file must be present
			it = m_vecRequiredFilesByExtensionOneOf.find(strExt);
			if (it != m_vecRequiredFilesByExtensionOneOf.end())
			{
				auto pvecMissingRequiredFiles = FileUtils::findMissingFiles(strPath, it->second);
				if (pvecMissingRequiredFiles->size() == it->second.size())
				{
					return false;
				}
			}
		}

		// check optional files
		if (options == eCheckOptions::knStrict)
		{
			auto it = m_vecOptionalFilesByExtension.find(strExt);
			if (it != m_vecOptionalFilesByExtension.end())
			{
				auto pvecMissingOptionalFiles = FileUtils::findMissingFiles(strPath, it->second);
				if (!pvecMissingOptionalFiles->empty())
				{
					return false;
				}
			}
		}

		// all conditions were satisfied
		return true;
	}

	return false;
}

/*!
Build the catalog describing the data source. This includes all data sets within
the data source and any sub-catalogs.

The default implementation simply returns a single catalog with one data set.

\param strPath	The path to the data source. May be a file, directory or url.

\return The catalog or nullptr if the data source is not supported.
*/
PYXPointer<const PYXCatalog> STDMETHODCALLTYPE PipeBuilderBase::buildCatalog(const std::string& strPath) const
{
	if (isDataSourceSupported(strPath, eCheckOptions::knPartial))
	{
		auto strLeaf = FileUtils::pathToString(FileUtils::stringToPath(strPath).leaf());

		auto pCatalog = PYXCatalog::create();
		pCatalog->setUri(strPath);		
		pCatalog->setName(strLeaf);

		auto pDataSet = PYXDataSet::create(strPath, strLeaf);

		// record missing files
		pDataSet->setMissingRequiredFilesAllOf(*findMissingRequiredFilesAllOf(strPath));
		pDataSet->setMissingRequiredFilesOneOf(*findMissingRequiredFilesOneOf(strPath));
		pDataSet->setMissingOptionalFiles(*findMissingOptionalFiles(strPath));

		pCatalog->addDataSet(pDataSet);

		return pCatalog;
	}

	return nullptr;
}

void PipeBuilderBase::setConfig(const std::string& key, const std::string value)
{
	if (key == "default_sampler")
	{
		m_defaultSamplerGuid = value;
	} 
	else
	{
		IPipeBuilder::setConfig(key,value);
	}
}

/*!
Get the default sampler.

\return The default sampler
*/
boost::intrusive_ptr<IProcess> PipeBuilderBase::getDefaultSampler() const
{
	boost::intrusive_ptr<IProcess> spSamplerProc;

	PYXCOMCreateInstance(strToGuid(m_defaultSamplerGuid), 0, IProcess::iid, (void**) &spSamplerProc);

	if (!spSamplerProc)
	{
		PYXTHROW(PYXException, "Unable to get the default sampler");
	}

	return spSamplerProc;
}

bool PipeBuilderBase::isZipFile(const std::string& strPath) const
{
	auto zipPostfix = strPath.rfind(".zip");

	if (zipPostfix == strPath.npos)
	{
		return false;
	}

	auto zipFile = strPath.substr(0,zipPostfix + 4);

	return FileUtils::exists(FileUtils::stringToPath(zipFile));
}

/*! 
Find any missing required files for a data set (all files are required)

\param strPath	The path to a file.

\return	The missing files or an empty vector if not files are missing.
*/
std::auto_ptr< std::vector<std::string> > PipeBuilderBase::findMissingRequiredFilesAllOf(const std::string& strPath) const
{
	// get the file extension
	boost::filesystem::path path = FileUtils::stringToPath(strPath);
	std::string strExt(FileUtils::getExtensionNoDot(path));
	boost::algorithm::to_lower(strExt);

	// find missing required files
	auto it = m_vecRequiredFilesByExtensionAllOf.find(strExt);
	if (it != m_vecRequiredFilesByExtensionAllOf.end())
	{
		return FileUtils::findMissingFiles(strPath, it->second);
	}

	return std::auto_ptr< std::vector<std::string> >(new std::vector<std::string>());
}

/*! 
Find any missing required files for a data set (one file is required)

\param strPath	The path to a file.

\return	The missing files or an empty vector if not files are missing.
*/
std::auto_ptr< std::vector<std::string> > PipeBuilderBase::findMissingRequiredFilesOneOf(const std::string& strPath) const
{
	// get the file extension
	boost::filesystem::path path = FileUtils::stringToPath(strPath);
	std::string strExt(FileUtils::getExtensionNoDot(path));
	boost::algorithm::to_lower(strExt);

	// find missing required files
	auto it = m_vecRequiredFilesByExtensionOneOf.find(strExt);
	if (it != m_vecRequiredFilesByExtensionOneOf.end())
	{
		auto pvecMissing = FileUtils::findMissingFiles(strPath, it->second);
		if (pvecMissing->size() == it->second.size())
		{
			// all files were missing
			return pvecMissing;
		}
	}

	return std::auto_ptr< std::vector<std::string> >(new std::vector<std::string>());
}

/*! 
Find any missing optional files for a data set

\param strPath	The path to a file.

\return	The missing files or an empty vector if not files are missing.
*/
std::auto_ptr< std::vector<std::string> > PipeBuilderBase::findMissingOptionalFiles(const std::string& strPath) const
{
	// get the file extension
	boost::filesystem::path path = FileUtils::stringToPath(strPath);
	std::string strExt(FileUtils::getExtensionNoDot(path));
	boost::algorithm::to_lower(strExt);

	// find missing required files
	auto it = m_vecOptionalFilesByExtension.find(strExt);
	if (it != m_vecOptionalFilesByExtension.end())
	{
		return FileUtils::findMissingFiles(strPath, it->second);
	}

	return std::auto_ptr< std::vector<std::string> >(new std::vector<std::string>());
}

/*!
Add any required and optional supporting files to the pipeline.

\param process	The path process
\param path		The path to the data set 
*/
void PipeBuilderBase::addSupportingFiles(
	const boost::intrusive_ptr<IProcess>& process,
	const boost::filesystem::path& path) const
{
	auto pathProc = process->QueryInterface<IPath>();

	if (FileUtils::isDirectory(path)) // handle directory based data sources
	{
		// add all files in the directory, do not recurse into subdirectories
		boost::filesystem::directory_iterator itEnd; // default constructor is the end
		for (boost::filesystem::directory_iterator it(path); it != itEnd; ++it)
		{
			if (!FileUtils::isDirectory(*it))
			{
				pathProc->addPath(FileUtils::pathToString(it->path()));
			}
		}
	}
	else // handle multi-file based data sources with extra required and optional files
	{
		auto extension = FileUtils::getExtensionNoDot(path);
		boost::algorithm::to_lower(extension);

		auto it = m_vecRequiredFilesByExtensionAllOf.find(extension);
		if (it != m_vecRequiredFilesByExtensionAllOf.end())
		{
			addSupportingFilesByExtension(pathProc, path, it->second);
		}

		it = m_vecRequiredFilesByExtensionOneOf.find(extension);
		if (it != m_vecRequiredFilesByExtensionOneOf.end())
		{
			addSupportingFilesByExtension(pathProc, path, it->second);
		}

		it = m_vecOptionalFilesByExtension.find(extension);
		if (it != m_vecOptionalFilesByExtension.end())
		{
			addSupportingFilesByExtension(pathProc, path, it->second);
		}
	}
}

/*!
Add existing files to the path proc by extension.

\param pathProc			The path process
\param path				The path to the data set
\param vecExtensions	The file extensions to check
*/
void PipeBuilderBase::addSupportingFilesByExtension(
	const boost::intrusive_ptr<IPath>& pathProc,
	const boost::filesystem::path& path,
	const std::vector<std::string>& vecExtensions	) const
{
	for (auto& supportingExtension : vecExtensions)
	{
		auto additionalPath = path;
		additionalPath.replace_extension(supportingExtension);

		if (FileUtils::exists(additionalPath))
		{
			pathProc->addPath(FileUtils::pathToString(additionalPath));
		}
	}
}


