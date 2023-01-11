#ifndef PYXIS_BLOB_PROVIDER_H
#define PYXIS_BLOB_PROVIDER_H
/******************************************************************************
pyxis_blob_provider.h

begin		: 2016-03-01
copyright	: (C) 2016 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// Pyxis includes
#include "i_blob_provider.h"

// Casablanca includes
#include <cpprest/http_client.h>

// Boost includes
#include <boost/thread/recursive_mutex.hpp>

// Standard includes
#include <queue>
#include <unordered_set>

/*!
Provides methods for adding and removing blobs of data from Pyxis storage.
Blobs are passed as streams and each is referenced by a unique key.
*/
class PYXLIB_DECL PyxisBlobProvider : public AbstractBlobProvider
{
private:

    /*!
    KnownMissingKeysCache is a helper class to reduce requests over the network.

	The class keeps a small set of keys that are known not to exist on
    the remote server
	*/
    class KnownMissingKeysCache
    {
	public:

        /*!
        Default cache size to use.

		Under the assumption each part is 1MB -> we have a cache for 1GB of data.
		*/
        static const std::size_t knDefaultCacheSize = 1000;

	private:

		//! Maximum number of keys in cache
        std::size_t m_maxSize;

		//! The mutex for thread safety.
		mutable boost::recursive_mutex m_knownMissingKeysMutex;

		//! Lookup table for keys
        std::unordered_set<std::string> m_knownMissingKeys;

		//! Order of keys reported to the cache (least recently reported)
        std::queue<std::string> m_knownMissingKeysOrder;

	public:

		/*!
		Constructor

		\param maxSize	The maximum number of keys in the cache
		*/
		KnownMissingKeysCache(std::size_t maxSize) : m_maxSize(maxSize) {}

		//! Add a list of missing keys into the cache.
        void addMissingKeys(const std::vector<std::string>& keys);

        //! Is the given key missing?
        bool isMissing(const std::string& key) const;

        //! Report that a key exists.
        void reportKeyExists(const std::string& key);
    };

private:
	//! The HTTP client
	web::http::client::http_client m_client;

	//! The blob keys that are known to be missing
    KnownMissingKeysCache m_knownMissingKeysCache;

public:

	//! Test method
	static void test();

	/*!
	Default constructor
	*/
    PyxisBlobProvider() :
		m_client(U("http://storage-pyxis.azurewebsites.net/api/v1/storage/blobs/")),
		m_knownMissingKeysCache(KnownMissingKeysCache::knDefaultCacheSize) {}

	/*!
	Constructor

	\param serverURL	The server URL
	*/
	PyxisBlobProvider(const std::string& serverURL) :
		m_client(utility::conversions::to_string_t(serverURL + "/api/v1/storage/blobs/")),
		m_knownMissingKeysCache(KnownMissingKeysCache::knDefaultCacheSize) {}

    //! Get a blob with the given key from storage.
    bool getBlob(const std::string& key, std::ostream& ostream);

    //! Retrieve multiple blobs at once.
    std::shared_ptr<BlobMap> getBlobs(const KeyVector& keys);

    //! Add a blob with the given key to storage.
    bool addBlob(const std::string& key, std::istream& istream);

    //! Remove the blob with the given key from storage.
    bool removeBlob(const std::string& key);

    //! Check if a blob exists.
    bool blobExists(const std::string& key);

    //! Check for the existence of multiple blobs.
    std::shared_ptr<KeyVector> missingBlobs(const KeyVector& keys);
};
#endif // guard