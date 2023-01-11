#include "StdAfx.h"
#include "utilities.h"

#include <direct.h>
#include <io.h>

utilities::utilities(void)
{
}

utilities::~utilities(void)
{
}

std::string utilities::toString(double fValue)
{
	char szBuffer[40];

	_gcvt_s(szBuffer, 39, fValue, 14);

	return std::string(szBuffer);
}

std::string utilities::toString(int nValue)
{
	char szBuffer[40];

	_itoa_s(nValue, szBuffer, 39, 10);

	return std::string(szBuffer);
}

std::string utilities::toString(size_t nValue)
{
	char szBuffer[70];

	_ui64toa_s(nValue, szBuffer, 69, 10);

	return std::string(szBuffer);
}

void utilities::replaceAll(std::string& strSource, char szToFind, char szToReplace)
{
	for (size_t x = 0; x < strSource.length(); ++x)
	{
		if (strSource[x] == szToFind)
		{
			strSource[x] = szToReplace;
		}
	}
}

void utilities::createDirectory(std::string &strDirectory)
{
	size_t nPosn = 0;

	char szTemp;

	size_t nLen = strDirectory.size();

	while (nPosn < nLen)
	{
		while (nPosn < nLen && strDirectory[nPosn] != '/' && strDirectory[nPosn] != '\\')
		{
			++nPosn;
		}

		if (nPosn < nLen)
		{
			if (strDirectory[nPosn-1] != ':')
			{
				szTemp = strDirectory[nPosn];
				strDirectory[nPosn] = 0;
				_mkdir(strDirectory.c_str());
				strDirectory[nPosn] = szTemp;
				++nPosn;
			}
			else
			{
				++nPosn;
			}
		}
		else
		{
			_mkdir(strDirectory.c_str());
		}
	}
}

bool utilities::isFile(std::string& strPath)
{
	_finddata_t fileInfo;

	intptr_t hFile = _findfirst(strPath.c_str(), &fileInfo);

	if (hFile == -1)
	{
		return false;
	}

	_findclose(hFile);

	// If not a directory, then we are a file.
	if ((fileInfo.attrib & _A_SUBDIR) == 0)
	{
		return true;
	}

	return false;
}