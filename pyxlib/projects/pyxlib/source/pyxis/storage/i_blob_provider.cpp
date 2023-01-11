/******************************************************************************
i_blob_provider.cpp

begin		: 2016-03-01
copyright	: (C) 2016 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/storage/i_blob_provider.h"

// local includes

// standard includes

/*!
Retrieve multiple blobs at once.

\param keys	The blob keys.

\return A map of blobs indexed by their keys.
*/
std::shared_ptr<AbstractBlobProvider::BlobMap> AbstractBlobProvider::getBlobs(const KeyVector& keys)
{
	auto blobMap = std::make_shared<BlobMap>();

    for (auto& key : keys)
    {
		std::ostringstream oss;
		if (getBlob(key, oss))
		{
			(*blobMap)[key] = std::make_shared<std::string>(oss.str());
		}
    }

	return blobMap;
}

/*!
Check for the existence of multiple blobs.

\param keys			The keys to check for existence.

\return Keys for the missing blobs.
*/
std::shared_ptr<AbstractBlobProvider::KeyVector> AbstractBlobProvider::missingBlobs(const KeyVector& keys)
{
	auto missingKeys = std::make_shared<KeyVector>();

	std::for_each(keys.begin(), keys.end(), [&] (const std::string& key)
	{
		if (!blobExists(key))
		{
			missingKeys->push_back(key);
		}
	});

	return missingKeys;
}

