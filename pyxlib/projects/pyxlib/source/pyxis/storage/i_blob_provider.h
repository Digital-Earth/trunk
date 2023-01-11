#ifndef I_BLOB_PROVIDER_H
#define I_BLOB_PROVIDER_H
/******************************************************************************
i_blob_provider.h

begin		: 2016-03-01
copyright	: (C) 2016 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// standard includes
#include <string>

/*!
Provides methods for adding and removing blobs of data from storage.
Blobs are passed as streams and each is referenced by a unique key.
*/
class PYXLIB_DECL IBlobProvider
{
public:

	//! Vector of keys
	typedef std::vector<std::string> KeyVector;

	//! Map of blobs indexed by keys
	typedef std::map<std::string, std::shared_ptr<std::string> > BlobMap;

    /*!
    Get a blob with the given key from storage.

	\param key		The blob key
	\param ostream	The stream to write the blob into

	\return true if the blob is found, otherwise false
	*/
    virtual bool getBlob(const std::string& key, std::ostream& ostream) = 0;

    /*!
    Retrieve multiple blobs at once.

    \param keys		The blob keys.

	\return A map of blobs indexed by their keys.
	*/
    virtual std::shared_ptr<BlobMap> getBlobs(const KeyVector& keys) = 0;

	/*!
    Add a blob with the given key to storage.

    \param key		The blob key
    \param istream	The contents of the blob

    \return true if the blob was added or false if the key already exists
	*/
    virtual bool addBlob(const std::string& key, std::istream& istream) = 0;

    /*!
    Remove the blob with the given key from storage.

    \param key		The blob key

	\return true if the blob was removed, otherwise false
	*/
    virtual bool removeBlob(const std::string& key) = 0;

    /*!
    Check if a blob exists.

	\param key	The blob key

    \return true if a blob exists for the given key, otherwise false
	*/
    virtual bool blobExists(const std::string& key) = 0;

    /*!
    Check for the existence of multiple blobs.

    \param keys	The keys to check for existence.

	\return Keys for the missing blobs.
	*/
    virtual std::shared_ptr<KeyVector> missingBlobs(const KeyVector& keys) = 0;
};

/*!
Provides implementations for some of the IBlobProvider methods.
*/
class PYXLIB_DECL AbstractBlobProvider : public IBlobProvider
{
public:

    //! Retrieve multiple blobs at once.
    std::shared_ptr<BlobMap> getBlobs(const std::vector<std::string>& keys);

	//! Check for the existence of multiple blobs.
    std::shared_ptr<KeyVector> missingBlobs(const KeyVector& keys);
};
#endif // guard