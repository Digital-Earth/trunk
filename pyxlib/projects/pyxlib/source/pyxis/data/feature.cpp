/******************************************************************************
feature.cpp

begin		: 2006-10-19
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h" 
#include "pyxis/data/feature.h"

// pyxlib includes
#include "pyxis/utility/tester.h"

// {1D17EABA-649E-4942-B0D0-F772D3B56423}
PYXCOM_DEFINE_IID(IFeature, 
0x1d17eaba, 0x649e, 0x4942, 0xb0, 0xd0, 0xf7, 0x72, 0xd3, 0xb5, 0x64, 0x23);

namespace
{

//! Tester class
Tester<FIDStr> gTester;

}

void FIDStr::test()
{
	std::string str;
	std::vector<std::string> vecFID;
	std::vector<std::string> vecFID2;

	vecFID.push_back("foo");
	vecFID.push_back("bar");
	encode(&str, vecFID);
	decode(&str, &vecFID2);
	TEST_ASSERT(vecFID == vecFID2 && count(&str) == vecFID.size());
	TEST_ASSERT(contains(&str, "bar") == true);

	add(&str, "foo");
	decode(&str, &vecFID2);
	TEST_ASSERT(vecFID == vecFID2 && count(&str) == vecFID.size());

	vecFID.push_back(" s p a c e ");
	add(&str, " s p a c e ");
	decode(&str, &vecFID2);
	TEST_ASSERT(vecFID == vecFID2 && count(&str) == vecFID.size());

	vecFID.erase(vecFID.begin() + 1);
	remove(&str, "bar");
	decode(&str, &vecFID2);
	TEST_ASSERT(vecFID == vecFID2 && count(&str) == vecFID.size());
	TEST_ASSERT(contains(&str, "bar") == false);
}

/*!
\param pStr		The string. (must not be null)
\param vecFID	The feature IDs.
*/
void FIDStr::encode(std::string* pStr, const std::vector<std::string>& vecFID)
{
	assert(pStr);
	pStr->clear();

	// Currently only support format 0
	int nFormat = 0;
	pStr->append(1, static_cast<char>(nFormat));

	int nCount = static_cast<int>(vecFID.size());
	for (int n = 0; n != nCount; ++n)
	{
		pStr->append(1, static_cast<char>(vecFID[n].size()));
		pStr->append(vecFID[n]);
	}
}

/*!
\param pStr		The string. (must not be null)
\param pVecFID	The feature IDs. (must not be null)
*/
void FIDStr::decode(const std::string* pStr, std::vector<std::string>* pVecFID)
{
	assert(pStr);
	assert(pVecFID);
	pVecFID->clear();

	if (pStr->empty())
	{
		return;
	}

	// Currently only support format 0
	int nFormat = (*pStr)[0];

	switch (nFormat)
	{
		case 0:
		{
			int nPos = 1;
			int nStrSize = static_cast<int>(pStr->size());
			while (nPos < nStrSize)
			{
				int nSize = (*pStr)[nPos++];
				pVecFID->resize(pVecFID->size() + 1);
				pVecFID->back().append(pStr->begin() + nPos, pStr->begin() + nPos + nSize);
				nPos += nSize;
			}
			break;
		}

		default:
			assert(false && "unsupported encoding format");
			break;
	}
}

/*!
\param pStr		The string. (must not be null)
\param strFID	The feature ID.
\return Whether the feature ID was added (i.e. wasn't already present).
*/
bool FIDStr::add(std::string* pStr, const std::string& strFID)
{
	assert(pStr);

	if (pStr->empty())
	{
		pStr->append(1, static_cast<char>(0));
		pStr->append(1, static_cast<char>(strFID.size()));
		pStr->append(strFID);
		return true;
	}

	switch ((*pStr)[0])
	{
		case 0:
		{
			int nFIDSize = static_cast<int>(strFID.size());
			std::string::const_iterator itFIDBegin = strFID.begin();
			std::string::const_iterator itFIDEnd = strFID.end();
			std::string::iterator it = pStr->begin() + 1;
			std::string::const_iterator itEnd = pStr->end();
			while (it != itEnd)
			{
				int nSize = *it++;
				if (nSize == nFIDSize && std::equal(itFIDBegin, itFIDEnd, it))
				{
					return false;
				}
				it += nSize;
			}
			pStr->append(1, static_cast<char>(nFIDSize));
			pStr->append(strFID);
			return true;
		}

		default:
		{
			assert(false && "this works but please write some code");
			std::vector<std::string> vecFID;
			decode(pStr, &vecFID);
			std::vector<std::string>::iterator it =
				std::find(vecFID.begin(), vecFID.end(), strFID);
			if (it == vecFID.end())
			{
				vecFID.push_back(strFID);
				encode(pStr, vecFID);
				return true;
			}
			return false;
		}
	}
}

/*!
This function does not check if the feature ID is already present in the
encoded string. Use only if you are certain this is the case!

\param pStr		The string. (must not be null)
\param strFID	The feature ID.
*/
void FIDStr::uncheckedAdd(std::string* pStr, const std::string& strFID)
{
	assert(pStr);

	if (pStr->empty())
	{
		pStr->append(1, static_cast<char>(0));
		pStr->append(1, static_cast<char>(strFID.size()));
		pStr->append(strFID);
		return;
	}

	switch ((*pStr)[0])
	{
		case 0:
		{
			pStr->append(1, static_cast<char>(strFID.size()));
			pStr->append(strFID);
			return;
		}

		default:
		{
			assert(false && "this works but please write some code");
			std::vector<std::string> vecFID;
			decode(pStr, &vecFID);
			vecFID.push_back(strFID);
			encode(pStr, vecFID);
			return;
		}
	}
}

/*!
\param pStr		The string. (must not be null)
\param strFID	The feature ID.
\return Whether the feature ID was removed (i.e. wasn't already absent).
*/
bool FIDStr::remove(std::string* pStr, const std::string& strFID)
{
	assert(pStr);

	switch ((*pStr)[0])
	{
		case 0:
		{
			int nFIDSize = static_cast<int>(strFID.size());
			std::string::const_iterator itFIDBegin = strFID.begin();
			std::string::const_iterator itFIDEnd = strFID.end();
			std::string::iterator it = pStr->begin() + 1;
			std::string::const_iterator itEnd = pStr->end();
			while (it != itEnd)
			{
				int nSize = *it++;
				if (nSize == nFIDSize && std::equal(itFIDBegin, itFIDEnd, it))
				{
					pStr->erase(it - 1, it + nSize);
					return true;
				}
				it += nSize;
			}
			return false;
		}

		default:
		{
			assert(false && "this works but please write some code");
			std::vector<std::string> vecFID;
			decode(pStr, &vecFID);
			std::vector<std::string>::iterator it =
				std::find(vecFID.begin(), vecFID.end(), strFID);
			if (it != vecFID.end())
			{
				vecFID.erase(it);
				encode(pStr, vecFID);
				return true;
			}
			return false;
		}
	}
}

/*!
\param pStr		The string. (must not be null)
\param strFID	The feature ID.
\return Whether the feature ID is present.
*/
bool FIDStr::contains(const std::string* pStr, const std::string& strFID)
{
	assert(pStr);

	if (pStr->empty())
	{
		return false;
	}

	switch ((*pStr)[0])
	{
		case 0:
		{
			int nFIDSize = static_cast<int>(strFID.size());
			std::string::const_iterator itFIDBegin = strFID.begin();
			std::string::const_iterator itFIDEnd = strFID.end();
			std::string::const_iterator it = pStr->begin() + 1;
			std::string::const_iterator itEnd = pStr->end();
			while (it != itEnd)
			{
				int nSize = *it++;
				if (nSize == nFIDSize && std::equal(itFIDBegin, itFIDEnd, it))
				{
					return true;
				}
				it += nSize;
			}
			return false;
		}

		default:
		{
			assert(false && "this works but please write some code");
			std::vector<std::string> vecFID;
			decode(pStr, &vecFID);
			return std::find(vecFID.begin(), vecFID.end(), strFID) != vecFID.end();
		}
	}
}

/*!
\param pStr		The string. (must not be null)
\return The number of feature IDs.
*/
int FIDStr::count(const std::string* pStr)
{
	assert(pStr);

	if (pStr->empty())
	{
		return 0;
	}

	switch ((*pStr)[0])
	{
		case 0:
		{
			int nCount = 0;
			std::string::const_iterator it = pStr->begin() + 1;
			std::string::const_iterator itEnd = pStr->end();
			while (it != itEnd)
			{
				int nSize = *it++;
				++nCount;
				it += nSize;
			}
			return nCount;
		}

		default:
		{
			assert(false && "this works but please write some code");
			std::vector<std::string> vecFID;
			decode(pStr, &vecFID);
			return static_cast<int>(vecFID.size());
		}
	}
}
