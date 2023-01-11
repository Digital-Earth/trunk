/******************************************************************************
pyxis_blob_provider.cpp

begin		: 2016-03-01
copyright	: (C) 2016 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"

// windows includes
#pragma warning(push)
#pragma warning(disable: 4005) // warning C4005: 'S_OK' : macro redefinition
#include <windows.h>
#pragma warning(pop)

// Pyxis includes
#include "pyxis/storage/pyxis_blob_provider.h"
#include "pyxis/storage/storage_exceptions.h"
#include "pyxis/utility/exceptions.h"
#include "pyxis/utility/ssl_utils.h"
#include "pyxis/utility/tester.h"

// Casablanca includes
#include <cpprest/interopstream.h>

// standard includes
#include <codecvt>

//! Tester class
Tester<PyxisBlobProvider> gTester;

//! Convert a GUID to a string
std::string guidToString(const GUID& guid)
{
	char szGuid[40]={0};
	_snprintf_s(szGuid, sizeof(szGuid), _TRUNCATE, "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

	return szGuid;
}

//! Generate a key in JSON format from a string
std::string generateKey(const std::string& str)
{
	SSLUtils::Checksum checksumSHA256("SHA256");
	assert(checksumSHA256.generate(str));

	std::ostringstream oss;
	oss << "{\"Size\":" << str.length() << ",\"Hash\":\"" << checksumSHA256.toBase64String() << "\"}";

	return oss.str();
}

//! Test method
void PyxisBlobProvider::test()
{
	return; // This test consumes a small amount of disk space on Azure so it is disabled

	// create a blob provider for testing
    const std::string serverAddress = "http://storage-pyxis.azurewebsites.net";
    PyxisBlobProvider client(serverAddress);

	{
		// missing keys test
		GUID guid;
		CoCreateGuid(&guid);

		const std::string data = "this is a test : " + guidToString(guid);
		auto key = generateKey(data);

		TEST_ASSERT(!client.blobExists(key));
		std::istringstream ss(data);
		client.addBlob(key, ss);
		TEST_ASSERT(client.blobExists(key));

		// blob comparison test
		std::ostringstream downloaded;
		client.getBlob(key, downloaded);
		TEST_ASSERT(data.compare(downloaded.str()) == 0);
	}

	{
		// failed download test
		GUID guid;
		CoCreateGuid(&guid);

		const std::string data = "this is a test : " + guidToString(guid);
		auto key = generateKey(data);

		std::ostringstream oss;
		TEST_ASSERT(!client.getBlob(key, oss));
    }

	{
		// repeated upload test
		GUID guid;
		CoCreateGuid(&guid);

		auto key = generateKey(guidToString(guid));
		std::istringstream iss(key);
		TEST_ASSERT(client.addBlob(key, iss));
		TEST_ASSERT(!client.addBlob(key, iss));
	}

	{
		// missing blobs and get blobs test

		// repeated upload test
		GUID guid1;
		CoCreateGuid(&guid1);
		auto key1 = generateKey(guidToString(guid1));
		std::istringstream iss1(key1);
		TEST_ASSERT(client.addBlob(key1, iss1));

		GUID guid2;
		CoCreateGuid(&guid2);
		auto key2 = generateKey(guidToString(guid2));
		std::istringstream iss2(key2);
		TEST_ASSERT(client.addBlob(key2, iss2));

		GUID guid3;
		CoCreateGuid(&guid3);
		auto key3 = generateKey(guidToString(guid3));

		std::vector<std::string> keys;
		keys.push_back(key1);
		keys.push_back(key2);
		keys.push_back(key3);
		auto blobMap = client.getBlobs(keys);
		TEST_ASSERT(blobMap->size() == 2);

		auto it = blobMap->find(key1);
		TEST_ASSERT(it != blobMap->end());
		TEST_ASSERT(it->second->compare(key1) == 0);

		it = blobMap->find(key2);
		TEST_ASSERT(it != blobMap->end());
		TEST_ASSERT(it->second->compare(key2) == 0);

		auto missingKeys = client.missingBlobs(keys);
		TEST_ASSERT(missingKeys->size() == 1);
		TEST_ASSERT((*missingKeys)[0].compare(key3) == 0);
	}
}

/*!
Add a list of missing keys into the cache.

\param keys	The keys to add.
*/
void PyxisBlobProvider::KnownMissingKeysCache::addMissingKeys(const std::vector<std::string>& keys)
{
	boost::recursive_mutex::scoped_lock lock(m_knownMissingKeysMutex);

	for (auto& key : keys)
	{
		if (m_knownMissingKeys.find(key) == m_knownMissingKeys.end())
		{
			m_knownMissingKeys.insert(key);
			m_knownMissingKeysOrder.push(key);
		}
	}

    while (m_knownMissingKeysOrder.size() > m_maxSize)
    {
		m_knownMissingKeys.erase(m_knownMissingKeysOrder.front());
        m_knownMissingKeysOrder.pop();
    }
}

/*!
Is the given key missing?

\param key	The key to check.
			
\return true if the key is missing or false if the status of the key is unknown.
*/
bool PyxisBlobProvider::KnownMissingKeysCache::isMissing(const std::string& key) const
{
	boost::recursive_mutex::scoped_lock lock(m_knownMissingKeysMutex);
    {
        return m_knownMissingKeys.find(key) != m_knownMissingKeys.end();
    }
}

/*!
Report that a key exists.

\param key	The key that is known to exist.
*/
void PyxisBlobProvider::KnownMissingKeysCache::reportKeyExists(const std::string& key)
{
	boost::recursive_mutex::scoped_lock lock(m_knownMissingKeysMutex);
    {
        m_knownMissingKeys.erase(key);                    
    }
}

/*!
Get a blob with the given key from storage.

\param key		The blob key
\param stream	The stream to write the blob into

\return true if the blob is found, otherwise false.
\throws PYXStorageException if the operation failed.
*/
bool PyxisBlobProvider::getBlob(const std::string& key, std::ostream& ostream)
{
	// build the request - key is encoded twice to match C# implementation
	web::uri_builder builder(U("blob"));
	auto encodedKey = web::uri::encode_data_string(utility::conversions::to_string_t(key));
	encodedKey = web::uri::encode_data_string(encodedKey);
	builder.append_path(encodedKey);

	// send the request to the server
	auto task = m_client.request(web::http::methods::GET, builder.to_string())

	// handle the response headers arriving
	.then([&](pplx::task<web::http::http_response> previousTask) -> pplx::task<size_t>
	{
		auto response = previousTask.get();
		if (response.status_code() == web::http::status_codes::OK)
		{
			// write response body into the stream
			concurrency::streams::stdio_ostream<char> astream(ostream);
			return response.body().read_to_end(astream.streambuf());
		}
		else if (response.status_code() == web::http::status_codes::NotFound)
		{
			return pplx::task_from_result<size_t>(0);
		}

		PYXTHROW(PYXException, "Unable to get blob. HTTP code: " << response.status_code());
	})

	// wait for the I/O to complete
	.then([&](pplx::task<size_t> previousTask)
	{
		// this will block until all IO is done
		return previousTask.get() > 0;
	});

	// wait for outstanding I/O to complete and handle exceptions
	try
	{
		task.wait();
		return task.get();
	}
	catch (PYXException& e)
	{
		PYXRETHROW(e, PYXStorageException, "Unable to get blob: " << key);
	}
	catch (const std::exception& e)
	{
		PYXTHROW(PYXStorageException, "Unable to get blob: " << key << " " << e.what());
	}

	return false;
}

/*!
Retrieve multiple blobs at once.

\param keys	The blob keys.

\return blobMap	A map of blobs indexed by their keys.
\throws PYXStorageException if the operation fails.
*/
std::shared_ptr<PyxisBlobProvider::BlobMap> PyxisBlobProvider::getBlobs(const PyxisBlobProvider::KeyVector& keys)
{
	// build the request
	web::uri_builder builder(U("multiblobs"));

	web::http::http_request request(web::http::methods::POST);
	request.set_request_uri(builder.to_uri());

	// build the body - a plain text string containing JSON
	int i = 0;
	auto jsonKeys = web::json::value::array();
	for (auto& key : keys)
	{
		// keys are in JSON format, but treat as strings for HTTP request
		jsonKeys[i++] = web::json::value::string(utility::conversions::to_string_t(key));
	}
	request.set_body(jsonKeys.serialize());

	// send the request to the server
	auto task = m_client.request(request)

	// handle the response headers arriving
	.then([](pplx::task<web::http::http_response> previousTask) -> pplx::task<utility::string_t>
	{
		auto response = previousTask.get();
		if (response.status_code() == web::http::status_codes::OK)
		{
			// parse JSON response
			return response.extract_string();
		}

		PYXTHROW(PYXHttpException, "Unable to get blobs. HTTP code: " << response.status_code());
	})

	// process the string
	.then([&](pplx::task<utility::string_t> previousTask)
	{
		auto str = previousTask.get();
		try
		{
			auto v = web::json::value::parse(str);
			if (v.is_object())
			{
				const auto& obj = v.as_object();

				// store information about missing keys
				auto blobMap = std::make_shared<BlobMap>();
				for (auto it : obj)
				{
					// use emplace to avoid copying large strings
					blobMap->emplace(
						utility::conversions::to_utf8string(it.first),
						std::make_shared<std::string>(utility::conversions::to_utf8string(it.second.as_string())));
				}

				return blobMap;
			}
		}
		catch (const std::exception& e)
		{
			TRACE_INFO("Failed to parse JSON: " << utility::conversions::to_utf8string(str));
			TRACE_INFO(e.what());
		}

		PYXTHROW(PYXHttpException, "Unable to get blobs. JSON: " << utility::conversions::to_utf8string(str));
	});

	// wait for outstanding I/O to complete and handle exceptions
	try
	{
		task.wait();
		return task.get();
	}
	catch (PYXException& e)
	{
		PYXRETHROW(e, PYXStorageException, "Unable to get blobs.");
	}
	catch (const std::exception& e)
	{
        PYXTHROW(PYXStorageException, "Unable get blobs: " << e.what());
	}
}

/*!
Add a blob with the given key to storage.

\param key		The blob key
\param stream	The contents of the blob

\return true if the blob was added or false if the key already exists.
\throws PYXStorageException if the operation failed.
*/
bool PyxisBlobProvider::addBlob(const std::string& key, std::istream& istream)
{
	if (blobExists(key))
	{
		return false;
	}

	// build the request - key is encoded twice to match C# implementation
	web::uri_builder builder(U("blob"));
	auto encodedKey = web::uri::encode_data_string(utility::conversions::to_string_t(key));
	encodedKey = web::uri::encode_data_string(encodedKey);
	builder.append_path(encodedKey);

	web::http::http_request request(web::http::methods::POST);
	request.set_request_uri(builder.to_uri());

	concurrency::streams::stdio_istream<uint8_t> astream(istream);
	request.set_body(astream);

	// send the request to the server
	auto task = m_client.request(request)

	// handle the response headers arriving
	.then([&](pplx::task<web::http::http_response> previousTask)
	{
		auto response = previousTask.get();
		if (response.status_code() == web::http::status_codes::Created)
		{
			// we know this blob exists now
            m_knownMissingKeysCache.reportKeyExists(key);
			return true;
		}

		PYXTHROW(PYXHttpException, "Unable to upload blob. HTTP code: " << response.status_code());
	});

	// wait for outstanding I/O to complete and handle exceptions
	try
	{
		task.wait();
		return task.get();
	}
	catch (PYXException& e)
	{
		PYXRETHROW(e, PYXStorageException, "Unable to upload blob.");
	}
	catch (const std::exception& e)
	{
		PYXTHROW(PYXStorageException, "Unable to upload blob: " << e.what());
	}
}

/*!
Remove the blob with the given key from storage.

\param key		The blob key

\return true if the blob was removed, otherwise false
*/
bool PyxisBlobProvider::removeBlob(const std::string& key)
{
	PYXTHROW_NOT_IMPLEMENTED();
}

/*!
Check if a blob exists.

\param key	The blob key

\return true if a blob exists for the given key, otherwise false
*/
bool PyxisBlobProvider::blobExists(const std::string& key)
{
    // check if we already know the given key is missing
    if (m_knownMissingKeysCache.isMissing(key))
    {
        return false;
    }

	std::vector<std::string> keys;
	keys.push_back(key);

	auto missingKeys = missingBlobs(keys);

	return missingKeys->empty();
}

/*!
Check for the existence of multiple blobs.

\param keys	The keys to check for existence.

\return	Keys for the missing blobs.
\throws	PYXStorageException if the operation failed.
*/
std::shared_ptr<PyxisBlobProvider::KeyVector> PyxisBlobProvider::missingBlobs(const KeyVector& keys)
{
    // if no keys are requested, none are missing
	if (keys.empty())
	{
		return std::make_shared<KeyVector>();
	}

	// build the request
	web::uri_builder builder(U("missingblobs"));

	web::http::http_request request(web::http::methods::POST);
	request.set_request_uri(builder.to_uri());

	// build the body - a plain text string containing JSON
	int i = 0;
	auto jsonKeys = web::json::value::array();
	std::for_each(keys.begin(), keys.end(), [&] (const std::string& key)
	{
		jsonKeys[i++] = web::json::value::string(utility::conversions::to_string_t(key));
	});
	request.set_body(jsonKeys.serialize());

	// send the request to the server
	auto task = m_client.request(request)

	// handle the response headers arriving
	.then([](pplx::task<web::http::http_response> previousTask) -> pplx::task<utility::string_t>
	{
		auto response = previousTask.get();
		if (response.status_code() == web::http::status_codes::OK)
		{
			return response.extract_string();
		}

		PYXTHROW(PYXHttpException, "Unable to get missing blobs. HTTP code: " << response.status_code());
	})

	// process the string
	.then([&](pplx::task<utility::string_t> previousTask)
	{
		auto str = previousTask.get();

		try
		{
			web::json::value v = web::json::value::parse(str);

			if (v.is_array())
			{
				const web::json::array& a = v.as_array();

				// store information about missing keys
				auto missingKeys = std::make_shared<KeyVector>();
				missingKeys->reserve(a.size());
				for (auto it : a)
				{
					missingKeys->emplace_back(utility::conversions::to_utf8string(it.as_string()));
				}
				m_knownMissingKeysCache.addMissingKeys(*missingKeys);

				return missingKeys;
			}
		}
		catch (const std::exception& e)
		{
			TRACE_INFO("Failed to parse JSON: " << utility::conversions::to_utf8string(str));
			TRACE_INFO(e.what());
		}

		PYXTHROW(PYXHttpException, "Unable to get missing blobs. JSON: " << utility::conversions::to_utf8string(str));
	});

	// wait for outstanding I/O to complete and handle exceptions
	try
	{
		task.wait();
		return task.get();
	}
	catch (PYXException& e)
	{
		PYXRETHROW(e, PYXStorageException, "Unable to check if blobs exist.");
	}
	catch (const std::exception& e)
	{
        PYXTHROW(PYXStorageException, "Unable to check if blobs exist: " << e.what());
 	}
}
