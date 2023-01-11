/******************************************************************************
path.cpp

begin      : 20/09/2007 4:40:41 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/procs/path.h"

// PYXIS includes
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/checksum_calculator.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/xml_transform.h"
#include "pyxis/pipe/process_identity_cache.h"

// boost includes
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/path.hpp>


#define _WIN32_WINNT 0x0600
#include <windows.h>

Notifier FileNotificationManager::m_fileNeededNotifier("File needed notifier.");

Notifier FileNotificationManager::m_pipelineFilesDownloadNeededNotifier("Pipeline Files needed notifier.");

// {1658F006-F142-45b7-B328-499B024C1AF9}
PYXCOM_DEFINE_IID(IPath,
				  0x1658f006, 0xf142, 0x45b7, 0xb3, 0x28, 0x49, 0x9b, 0x2, 0x4c, 0x1a, 0xf9);

// {FF55256E-1F9F-4c43-B194-4648C1FB71EF}
PYXCOM_DEFINE_CLSID(PathInitError,
					0xff55256e, 0x1f9f, 0x4c43, 0xb1, 0x94, 0x46, 0x48, 0xc1, 0xfb, 0x71, 0xef);
PYXCOM_CLASS_INTERFACES(PathInitError, IProcessInitError::iid, PYXCOM_IUnknown::iid);

// {7B50BE20-F6A5-401e-8E76-98C5226DAF0D}
PYXCOM_DEFINE_CLSID(PathProcess,
					0x7b50be20, 0xf6a5, 0x401e, 0x8e, 0x76, 0x98, 0xc5, 0x22, 0x6d, 0xaf, 0xd);
PYXCOM_CLASS_INTERFACES(PathProcess, IPath::iid, IUrl::iid, IProcess::iid, PYXCOM_IUnknown::iid);

IPROCESS_SPEC_BEGIN(PathProcess, "Data File", "A local or network file resource", "Reader",
					IPath::iid, IUrl::iid, PYXCOM_IUnknown::iid)
					IPROCESS_SPEC_END

					// Tester class
					Tester<PathProcess> gTester;

// Test method
void PathProcess::test()
{
	boost::intrusive_ptr<IProcess> spProc(new PathProcess);
	TEST_ASSERT(spProc);
	boost::intrusive_ptr<IPath> spPath = dynamic_cast<IPath*>(spProc.get());
	TEST_ASSERT(spPath);
	TEST_ASSERT(spProc->initProc() == IProcess::knFailedToInit);

	boost::filesystem::path testFilePath = AppServices::makeTempFile(".test");
	TEST_ASSERT(spPath->setPath(FileUtils::pathToString(testFilePath)));
	spProc->initProc();

	// write out a sample process
	boost::filesystem::path tempPath = AppServices::makeTempFile(".ppl");
	TRACE_INFO("Example Path Process file created: " << FileUtils::pathToString(tempPath));
	PipeManager::writeProcessToFile(FileUtils::pathToString(tempPath), spProc);
}

////////////////////////////////////////////////////////////////////////////////
// ProcessImpl
////////////////////////////////////////////////////////////////////////////////

IProcess::eInitStatus PathProcess::initImpl()
{
	// there must be at least one file to init properly
	if (getLength() == 0)
	{
		setInitProcError<PathInitError>("Missing file description.");
		return knFailedToInit;
	}
	m_resolvedPaths.clear();

	// get the cache directory for this process
	boost::filesystem::path cacheDir = AppServices::getCacheDir("ProcessCache");
	const ProcessIdentityCache cache(cacheDir);
	boost::filesystem::path pathCache;	

	// if this process represents a directory of files, as opposed to a collection of files,
	// create the directory inside the cache directory and use the new directory as the cache
	int index = 0;
	if (m_bIsDirectory)
	{
		pathCache = cache.getPath(getIdentity(), true);

		auto dir = FileUtils::stringToPath(m_paths[index]);
		pathCache /= dir.leaf();
		boost::filesystem::create_directory(pathCache);
		if (!FileUtils::exists(pathCache))
		{
			setInitProcError<PathInitError>("Unabled to create cache directory: " + FileUtils::pathToString(pathCache));
			return knFailedToInit;
		}

		m_resolvedPaths.push_back(FileUtils::pathToString(pathCache));

		++index;
	}


	//try to resolve all local files

	std::vector<int> pathFailedtoResolve;
	bool filesInOriginalLocations = true;

	for (; index < getLength(); ++index)
	{
		m_resolvedPaths.push_back("");
		if (!FileUtils::exists(m_paths[index]))
		{
			filesInOriginalLocations = false;
			// send out a notification to see if anyone can get the file for us.
			boost::intrusive_ptr<IPath> spPath;
			const_cast<PathProcess*>(this)->QueryInterface(IPath::iid, (void**) &spPath);

			// correct the index if the first path is a directory as the manifest omits it
			PYXPointer<FileEvent> ev = FileEvent::create(spPath, m_bIsDirectory ? index - 1 : index);
			FileNotificationManager::getFileNeededNotifier().notify(ev);

			if (ev->getFailed())
			{
				pathFailedtoResolve.push_back(index);				
			}
			else
			{
				m_resolvedPaths[index] = ev->getLocalPath();
			}
		}
	}

	// return failure if we couldn't find all files
	if (!pathFailedtoResolve.empty())
	{
		if (pathFailedtoResolve.size() > 1)
		{
			//generate a nice error message
			std::string allFailedPaths = "";
			for(auto & i : pathFailedtoResolve)
			{
				if (!allFailedPaths.empty())
				{
					allFailedPaths += ", ";
				}
				allFailedPaths += m_paths[i];
			}
			setInitProcError<PathInitError>("Unable to locate several paths: " + allFailedPaths);
		}
		else
		{
			setInitProcError<PathInitError>("Unable to locate path: " + m_paths[pathFailedtoResolve[0]]);
		}
		return knFailedToInit;
	}

	// if we have more than 1 file - check if all files are in the same local directory (but different than original path)
	bool allFilesInTheSameDirectory = true;

	if (!filesInOriginalLocations && getLength() > 1)
	{
		index = m_bIsDirectory ? 1 : 0;

		boost::filesystem::path originalPath = FileUtils::stringToPath(m_paths[index]);
		boost::filesystem::path firstPath = FileUtils::stringToPath(m_resolvedPaths[index]);
		index++;

		//same file names - check if we have all other files in the same directory
		if (originalPath.leaf() == firstPath.leaf())
		{
			firstPath.remove_leaf();			
		
			for (; index < getLength(); ++index)
			{
				boost::filesystem::path resolvePath = FileUtils::stringToPath(m_resolvedPaths[index]); 
				boost::filesystem::path secondFile = firstPath / FileUtils::stringToPath(m_paths[index]).leaf();
				
				if (FileUtils::exists(secondFile) && boost::filesystem::file_size(resolvePath) == boost::filesystem::file_size(secondFile) )
				{
					m_resolvedPaths[index] = FileUtils::pathToString(secondFile);
				}
				else
				{
					allFilesInTheSameDirectory = false;
					break;
				}
			}
		}
		else
		{
			firstPath.remove_leaf();

			for (; index < getLength(); ++index)
			{
				boost::filesystem::path secondPath = FileUtils::stringToPath(m_resolvedPaths[index]);
				secondPath.remove_leaf();

				if (firstPath != secondPath)
				{
					allFilesInTheSameDirectory = false;
				}
			}
		}
	}

	if (!allFilesInTheSameDirectory)
	{
		if (pathCache.empty())
		{
			pathCache = cache.getPath(getIdentity(), true);
		}

		for(index = m_bIsDirectory ? 1 : 0; index < getLength(); ++index)
		{
			try
			{
				boost::filesystem::path file = FileUtils::stringToPath(m_resolvedPaths[index]);

				boost::filesystem::path originalPath(m_paths[index]);
				auto localPath = pathCache / originalPath.filename();

				bool needToCopy = true;
				if (boost::filesystem::exists(localPath))
				{
					if (boost::filesystem::last_write_time(file) != boost::filesystem::last_write_time(localPath) ||
						boost::filesystem::file_size(file) != boost::filesystem::file_size(localPath))
					{
						boost::filesystem::remove(localPath);
					}
					else									
					{
						needToCopy  = false;
					}
				}

				if (needToCopy)
				{
					boost::filesystem::copy_file(file, localPath);
				}
			
				m_resolvedPaths[index] = FileUtils::pathToString(localPath);
			}
			catch(...)
			{
				TRACE_ERROR("Failed to copy file to destination directory");
				return knFailedToInit;
			}
		}
	} 
	else
	{
		//ensure first path is same as other files.

		if (m_bIsDirectory)
		{
			auto resolvedDirecotry = FileUtils::pathToString(FileUtils::stringToPath(m_resolvedPaths[1]).remove_leaf());

			if (m_resolvedPaths[0] != resolvedDirecotry)
			{
				m_resolvedPaths[0] = resolvedDirecotry;
			}
		}
	}

	return knInitialized;	
}

////////////////////////////////////////////////////////////////////////////////
// IProcess
////////////////////////////////////////////////////////////////////////////////

std::string STDMETHODCALLTYPE PathProcess::getAttributeSchema() const
{
	return "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<xs:schema targetNamespace=\"http://tempuri.org/XMLSchema.xsd\" "
		"elementFormDefault=\"qualified\" "
		"xmlns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:mstns=\"http://tempuri.org/XMLSchema.xsd\" "
		"xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
		">"
		"<xs:element name=\"FileProcess\">"
		"<xs:complexType>"
		"<xs:sequence>"

		"<xs:element name=\"uri\" type=\"xs:anyURI\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>URI</friendlyName>"
		"<description></description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"<xs:element name=\"is_directory\" type=\"xs:boolean\">"
		"<xs:annotation>"
		"<xs:appinfo>"
		"<friendlyName>Is Directory</friendlyName>"
		"<description></description>"
		"</xs:appinfo>"
		"</xs:annotation>"
		"</xs:element>"

		"</xs:sequence>"
		"</xs:complexType>"
		"</xs:element>"
		"</xs:schema>";
}

std::string STDMETHODCALLTYPE PathProcess::getManifest() const
{
	if (m_strManifest.empty())
	{
		boost::intrusive_ptr<IPath> spPath;
		const_cast<PathProcess*>(this)->QueryInterface(IPath::iid, (void**) &spPath);
		m_strManifest =
			CSharpFunctionProvider::getCSharpFunctionProvider()->getSerializedManifest(spPath);
	}

	return m_strManifest;
}

std::string PathProcess::getIdentity() const
{
	PYXPointer<ProcessSpec> spSpec = getSpec();
	assert(spSpec);

	ProcessIdentity identity(spSpec->getClass());
	identity.setData(getData());

	/*
	The identity will depend on if we have a checksum for all the files that
	this process references.  If we do have all the checksums, then the identity
	will be all checksums concatenated into a large string.  Otherwise, the identity
	will be the attributes of the process.  The way that we can tell if we have all
	the checksums is if we have a non-blank manifest.

	// TODO: (Possibly...)
	Sorting the checksums alphabetically was considered for the identity, so that any
	set of the same files would have the same identity, but since the first file in the list
	has a key role, then it must come first.  It is possible that the rest of the checksums
	should be sorted (all but the first) since order doesn't seem important for those files.
	*/

	std::string strManifest = getManifest();
	std::string strChecksums = "";
	if (!strManifest.empty())
	{
		strChecksums =
			CSharpFunctionProvider::getCSharpFunctionProvider()->getIdentity(strManifest);
	}

	if (strChecksums.empty())
	{
		// Discard the uri, since it is replicated in the manifest.
		std::map<std::string, std::string> attributes = getAttributes();
		attributes.erase( "uri");
		identity.setAttributes(attributes);
	}
	else
	{
		identity.setChecksum(strChecksums);
	}

	return identity();
}

std::map<std::string, std::string> STDMETHODCALLTYPE PathProcess::getAttributes() const
{
	std::map<std::string, std::string> mapAttr;
	mapAttr["filenames"] = CSharpFunctionProvider::getCSharpFunctionProvider()->XMLSerialize(m_paths);
	mapAttr["manifest"] = getManifest();
	mapAttr["uri"] = this->getPath();

	if (m_bIsDirectory)
	{
		// only set attribute when true to avoid changing identities
		mapAttr["is_directory"] = "true";
	}

	return mapAttr;
}

void STDMETHODCALLTYPE PathProcess::setAttributes(const std::map<std::string, std::string>& mapAttr)
{
	// indicate that the process will need to be initialized
	m_initState = knNeedsInit;
	std::string attrValueStr;
	try
	{
		std::map<std::string, std::string>::const_iterator it;

		// We look for old attribute "uri" to be able to load older
		// versions.
		if ((it = mapAttr.find("uri")) != mapAttr.end())
		{
			attrValueStr = it ->second;
			std::string strPath = it->second;
			if (strPath == "")
			{
				return;
			}

			if (strPath.find("file:///") == 0)
			{
				strPath = it->second.substr(8);
			}

			if (strPath.find("file://") == 0)
			{
				strPath = it->second.substr(7);
			}

			// TODO:  Why would we add a slash at the beginning?
			//        Maybe it is a relative path.

			// handle network paths
			if (strPath.find(":") == std::string::npos)
			{
				// if we're not dealing with simply a file name
				if ((strPath.find("/") != std::string::npos) ||
					(strPath.find("\\") != std::string::npos))
				{
					if ((strPath.substr(0, 2) != "\\\\") &&
						(strPath.substr(0, 2) != "//"))
					{
						strPath = "//" + strPath;
					}
				}
			}
			setPath(strPath, 0);
		}

		m_bIsDirectory = false;
		if ((it = mapAttr.find("is_directory")) != mapAttr.end())
		{
			m_bIsDirectory = (boost::iequals(it->second, "true") || boost::iequals(it->second, "1")) ? true : false;
		}

		if ((it = mapAttr.find("filenames")) != mapAttr.end())
		{
			attrValueStr = it->second;
			m_paths.clear();
			CSharpFunctionProvider::getCSharpFunctionProvider()->XMLDeserialize(m_paths, attrValueStr);
		}

		if ((it = mapAttr.find("manifest")) != mapAttr.end())
		{
			attrValueStr = it->second;
			m_strManifest = attrValueStr;
		}
	}
	catch (...)
	{
		TRACE_ERROR("An error occurred while setting the value '" <<
			attrValueStr << ".");
	}
}

////////////////////////////////////////////////////////////////////////////////
// IPath
////////////////////////////////////////////////////////////////////////////////

const std::string PathProcess::getPath(int index) const
{
	if (index < getLength())
	{
		return m_paths[index];
	}
	return "";
}

bool PathProcess::setPath(const std::string& strPath, int index)
{
	if (index > getLength())
	{
		return false;
	}

	if (index == getLength())
	{
		return addPath(strPath);
	}

	boost::filesystem::path path = FileUtils::stringToPath(strPath);
	if (path.is_complete() && FileUtils::exists(path))
	{
		m_paths[index] = FileUtils::pathToString(path);
		return true;
	}

	return false;
}

bool PathProcess::addPath(const std::string& strPath)
{
	boost::filesystem::path path = FileUtils::stringToPath(strPath);
	if (path.is_complete())
	{
		m_paths.push_back(FileUtils::pathToString(path));
		return true;
	}
	return false;
}

const std::string PathProcess::getUrl() const
{
	if (getLength() > 0)
	{
		return "file:///" + m_paths[0];
	}
	return "";
}

bool PathProcess::setUrl(const std::string& url)
{
	// We only support the file:// URL for a path.
	if(url.find("file://") != 0)
	{
		return false;
	}

	std::string path = url;

	if (url.find("file:///") == 0)
	{
		path = url.substr(8);
	}
	else if (url.find("file://") == 0)
	{
		path = url.substr(7);
	}

	setPath(path);

	return true;
}

//! Returns a path to a local copy of the file, if available.
const std::string STDMETHODCALLTYPE PathProcess::getLocallyResolvedPath(int index) const
{
	if (m_initState != knInitialized)
	{
		PYXTHROW(PYXException,"Path proc not initialized");
	}
	std::string strPath = getPath(index);

	// TODO: This is a security gap.  We could share C:\autoexec.bat
	boost::filesystem::path path = FileUtils::stringToPath(strPath);
	if (path.is_complete() && FileUtils::exists(path))
	{
		return FileUtils::pathToString(path);
	}

	if (index < (int) m_resolvedPaths.size() && !m_resolvedPaths[index].empty())
	{
		return m_resolvedPaths[index];
	}

	// the first entry was a directory and couldn't be resolved
	if (m_bIsDirectory && index <= 0)
	{
		// There is no valid directory.  Return the default path, so that the client can work things out.
		return FileUtils::pathToString(path);
	}

	// send out a notification to see if anyone can get the file for us.
	boost::intrusive_ptr<IPath> spPath;
	const_cast<PathProcess*>(this)->QueryInterface(IPath::iid, (void**) &spPath);

	// correct the index if the first path is a directory as the manifest omits it
	PYXPointer<FileEvent> ev = FileEvent::create(spPath, m_bIsDirectory ? index - 1 : index);
	FileNotificationManager::getFileNeededNotifier().notify(ev);

	if (ev->getFailed())
	{
		// There is no valid file.  Return the default path, so that the client can work things out.
		return FileUtils::pathToString(path);
	}

	m_resolvedPaths[index] = ev->getLocalPath();
	return m_resolvedPaths[index];
}