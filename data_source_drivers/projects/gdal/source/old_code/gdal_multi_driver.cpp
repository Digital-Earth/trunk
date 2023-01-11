/******************************************************************************
gdal_multi_driver.cpp

begin		: 2005-04-01
copyright	: (C) 2005 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define MODULE_GDAL_SOURCE
#include "gdal_multi_driver.h"

// local includes
#include "gdal_multi_data_source.h"
#include "gdal_process.h"
#include "pyx_gdal_driver.h"

// pyxis library includes
#include "pyxis/data/exceptions.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/tester.h"

// standard includes
#include <direct.h>
#include <io.h>

// {DFFA3288-7D92-4ca2-ADD0-BD333D42BBBA}
PYXCOM_DEFINE_CLSID(GDALMultiDriver, 
0xdffa3288, 0x7d92, 0x4ca2, 0xad, 0xd0, 0xbd, 0x33, 0x3d, 0x42, 0xbb, 0xba);
PYXCOM_CLASS_INTERFACES(GDALMultiDriver,IDataDriver::iid,IUnknown::iid);

//! The name of the driver
static const std::string kstrDriverName = "GDAL_MULTI";

//! Tester class
TesterUnit<GDALMultiDriver> gTester;

///! Test method
void GDALMultiDriver::test()
{
}

/*!
Get the name of the driver.

\return	The name of the driver.
*/
std::string GDALMultiDriver::getName() const
{
	return kstrDriverName;
}

/*!
Open a GDAL multi data set. 

There are two ways that failure can occur:
1. It is not a valid GDAL multi data set, or
2. It is a valid GDAL multi data set but it had some problem opening it.

If it is not a valid GDAL multi data set, then the open will return an empty shared pointer.
If it is valid but failed to open it, then an exception will be thrown.
If it is valid and was successfully opened, then a shared pointer to the data source will be returned.

\param	strDataSourceName	The name of the data source.

\return	A shared pointer to the data source or an empty shared pointer if the
		data source could not be opened.
*/
PYXPointer<PYXDataSource>
GDALMultiDriver::openForRead(const std::string& strDataSourceName) const
{
	// If the data source name is not a directory, exit.
	if (!FileUtils::isDir(strDataSourceName))
	{
		return PYXPointer<PYXDataSource>();
	}
	
	std::map<std::string, int> mapFileEndings;

	getFileEndings(strDataSourceName, mapFileEndings, 0);

	std::string strFileExt[] = { "src","dt0","dt1","dt2","tif","dem","ntf","img","png","jpg","adf","i12","lf3","on2","on3","tp2","tp3","tl2","" };

	int nPosn = 0;

	while (!strFileExt[nPosn].empty())
	{
		// only search for this ending if it exists in the directory structure.
		if (mapFileEndings[strFileExt[nPosn]] > 0)
		{
			// create the reader.
			PYXPointer<GDALMultiDataSource> spDataSource(GDALMultiDataSource::create());

			// open the reader.
			spDataSource->open(strDataSourceName, strFileExt[nPosn]);
			if (spDataSource->getNumFiles() > 0)
			{
				return spDataSource;
			}
		}

		++nPosn;
	}

	return PYXPointer<PYXDataSource>();
}

void GDALMultiDriver::getFileEndings(	const std::string strDirectory, 
											std::map<std::string, int>& endingMap,
											bool* pbExit) const
{
	// Note: Windows specific code
	try
	{
		std::string strSearchDir(strDirectory.c_str());

		std::string strFileSpec(strSearchDir);
		strFileSpec.append("\\*.*");

		// find all directories in the current directory
		struct _finddata_t dir;
		intptr_t nDir;

		nDir = _findfirst(strFileSpec.c_str(), &dir);
		if (-1L != nDir)
		{
			do
			{
				if (dir.attrib & _A_SUBDIR)
				{
					// ignore current and parent directories
					if ('.' != dir.name[0])
					{
						// open the files in this directory
						std::string strSubDir(strSearchDir);

						strSubDir += '\\';
						strSubDir += dir.name;
						
						getFileEndings(strSubDir, endingMap, pbExit);
					}
				}
				else
				{
					size_t len = strlen(dir.name)-1;
					while ((len > 0) && (dir.name[len] != '.'))
					{
						dir.name[len] = tolower(dir.name[len]);
						len--;
					}

					len++;
					std::string strEnding(&dir.name[len]);
					endingMap[strEnding] += 1;
				}
			} while (0 == _findnext(nDir, &dir) && (pbExit !=0 ? !(*pbExit): true));

			_findclose(nDir);
		}
	}
	catch (PYXException& e)
	{
		PYXRETHROW(e, PYXDataSourceException, "Unable to open directory.");
	}
}


bool GDALMultiDriver::getFileInfo(const std::string& strURI, IDataDriver::FileInfo* pFileInfo, bool* pbExit)
{
	// If the data source name is not a directory, exit.
	if (!FileUtils::isDir(strURI))
	{
		return false;
	}
	
	std::map<std::string, int> mapFileEndings;

	getFileEndings(strURI, mapFileEndings, pbExit);

	std::string strFileExt[] = { "src","dt0","dt1","dt2","tif","dem","ntf","img","png","jpg","adf","i12","lf3","on2","on3","tp2","tp3","tl2","" };

	int nPosn = 0;

	while (!strFileExt[nPosn].empty() && (pbExit !=0 ? !(*pbExit): true))
	{
		// only search for this ending if it exists in the directory structure.
		if (mapFileEndings[strFileExt[nPosn]] > 0)
		{
			// Check the directory.
			bool bResult = openForCheckInfo(strURI, pFileInfo, strFileExt[nPosn], pbExit);
			if (bResult)
			{
				pFileInfo->m_strURI = strURI;
				pFileInfo->m_nFileType = IDataDriver::GDAL_MULTI;
				return true;
			}
		}
		++nPosn;
	}
	return false;
}

bool GDALMultiDriver::openForCheckInfo(	const std::string& strDir,
										  IDataDriver::FileInfo* pFileInfo,
											const std::string& strFileExt,
											bool* pbExit)
{
	// If the data source name is not a directory, exit.
	if (!FileUtils::isDir(strDir))
	{
		return false;
	}

	int nFoundFiles = 0;

	// Note: Windows specific code
	std::string strSearchDir(strDir);
	std::string strFileSpec(strSearchDir);
	strFileSpec.append("\\*.*");

	// find all directories in the current directory
	struct _finddata_t dir;
	intptr_t nDir;

	nDir = _findfirst(strFileSpec.c_str(), &dir);
	if (-1L != nDir)
	{
		do
		{
			if (dir.attrib & _A_SUBDIR)
			{
				// ignore current and parent directories
				if ('.' != dir.name[0])
				{
					// open the files in this directory
					std::string strSubDir(strSearchDir);

					strSubDir += '\\';
					strSubDir += dir.name;
					
					nFoundFiles += openFiles(strSubDir, pFileInfo, strFileExt, pbExit);
				}
			}

		} while (0 == _findnext(nDir, &dir) && (pbExit !=0 ? !(*pbExit): true));

		_findclose(nDir);
	}

	// We do not have a directory of directories.
	// Search specified directory for files.
	if (nFoundFiles == 0)
	{
		nFoundFiles = openFiles(strDir, pFileInfo, strFileExt, pbExit);
	}

	if (nFoundFiles == 0)
	{
		return false;
	}
	
	return true;
}

/*!
Open the files located in the directory specified.  

\param	strDir		Directory containing GDAL files.
\param pFileInfo	Pointer to file info to fill in with info on the specified directory if it is a GDAL multi-file dirtectory.
\param	strFileExt	File extension of GDAL files (handles multiple file types)
\param  pbExit		Pointer to optional boolean set to true when we should exit immediately.

\return	The number of files opened.
*/
int GDALMultiDriver::openFiles(	const std::string& strDir,
								  IDataDriver::FileInfo* pFileInfo, 
								const std::string& strFileExt,
								bool* pbExit)
{
	void* pDataSource = 0;

	// Note: Windows specific code

	std::string strExt = "*.";
	strExt += strFileExt;

	int nOpenedFiles = 0;

	PYXGDALDriver driver;
	//boost::shared_ptr<PYXGDALDriver> spDriver(new PYXGDALDriver());

	try
	{
		std::string strSearchDir(strDir.c_str());

		std::string strFileSpec(strSearchDir);
		strFileSpec.append("\\");
		strFileSpec.append(strExt);

		// find all files in the current directory
		struct _finddata_t file;
		intptr_t nDir;

		nDir = _findfirst(strFileSpec.c_str(), &file);
		if (-1L != nDir)
		{
			do
			{
				// open the files in this directory
				std::string strFile(strSearchDir);

				strFile += '\\';
				strFile += file.name;
				
				// this method can throw
				if (!driver.getFileInfo(strFile, pFileInfo, pbExit))
				{
					return 0;
				}
				
				++nOpenedFiles;

			} while (0 == _findnext(nDir, &file) && (pbExit !=0 ? !(*pbExit): true));

			_findclose(nDir);
		}
	}
	catch (PYXException&)
	{
		nOpenedFiles = 0;
	}

	return nOpenedFiles;
}