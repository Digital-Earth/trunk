/******************************************************************************
ssl_utils.cpp

begin		: 2008-05-28
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/ssl_utils.h"

// pyxlib includes
#include "pyxis/utility/exception.h"
#include "pyxis/utility/tester.h"

// boost includes
#include <boost/filesystem/fstream.hpp>

// used by tests
#include "pyxis/utility/app_services.h"

// openssl includes
#include <openssl/evp.h>

// boost includes
#include <boost/thread/recursive_mutex.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

namespace
{

// Buffer size for file read.  Note that changing this will change
// the checksum of a file if its size exceeds the buffer size.
const std::streamsize knReadBufferSize = 32768;

// Provides for exception-safe code.
class MessageDigestContext
{
	EVP_MD_CTX m_mdctx;

	const EVP_MD& m_md;

	bool m_bInitialized;

	//! A reference to a buffer to hold the resulting hash.
	boost::array<unsigned char, EVP_MAX_MD_SIZE>& m_arrBytes;

	//! A reference to the number of bytes in the hash, held in the buffer.
	unsigned int& m_nLength;

	//! Mutex for thread safety.
	// TODO: Investigate further measures for thread robustness in OpenSSL.  See:
	// - http://www.openssl.org/docs/crypto/threads.html#DESCRIPTION.
	// - http://www.nabble.com/When-to-use-CRYPTO_set_locking_callback()-and-CRYPTO_set_id_callback()--td5849882.html
	mutable boost::recursive_mutex m_mutex;

public:

	MessageDigestContext(
		const EVP_MD& md,
		boost::array<unsigned char, EVP_MAX_MD_SIZE>& arrBytes,
		unsigned int& nLength) :
	m_md(md),
	m_bInitialized(false),
	m_arrBytes(arrBytes),
	m_nLength(nLength)
	{
		EVP_MD_CTX_init(&m_mdctx);
	}

	~MessageDigestContext()
	{
		int nCleanup;

		if (m_bInitialized)
		{
			nCleanup = EVP_DigestFinal_ex(&m_mdctx, m_arrBytes.c_array(), &m_nLength);
			assert(nCleanup);
		}

		nCleanup = EVP_MD_CTX_cleanup(&m_mdctx);
		assert(nCleanup);
	}

	bool update(const void* pBuffer, std::size_t nSize)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		if (!m_bInitialized)
		{
			if (!EVP_DigestInit_ex(&m_mdctx, &m_md, NULL))
			{
				return false;
			}
			m_bInitialized = true;
		}

		return (0 != EVP_DigestUpdate(&m_mdctx, pBuffer, nSize));
	}
};

}

//! Tester class
Tester<SSLUtils::Checksum> gTester;

/*!
Must be called exactly once near program start.
*/
void SSLUtils::initialize()
{
	OpenSSL_add_all_digests();
}

/*!
Must be called exactly once near program end.
*/
void SSLUtils::uninitialize()
{
	EVP_cleanup();
}



struct SSLUtils::Checksum::SSLState
{
	//! A string indicating the type of checksum/hash (e.g. "MD5", "SHA1").
	const std::string m_strType;

	//! An unowned pointer to the message digest.
	const EVP_MD* const m_md;

	//! A buffer to hold the resulting hash.
	boost::array<unsigned char, EVP_MAX_MD_SIZE> m_arrBytes;

	//! The number of bytes in the hash, held in the buffer.
	unsigned int m_nLength;

	//! Mutex for thread safety.
	// TODO: Investigate further measures for thread robustness in OpenSSL.  See:
	// - http://www.openssl.org/docs/crypto/threads.html#DESCRIPTION.
	// - http://www.nabble.com/When-to-use-CRYPTO_set_locking_callback()-and-CRYPTO_set_id_callback()--td5849882.html
	mutable boost::recursive_mutex m_mutex;

public:
	SSLState(const std::string & strType)
		: m_strType(strType),
		m_md(EVP_get_digestbyname(strType.c_str())),
		m_nLength(0)
	{

	}
};

//! Test method
void SSLUtils::Checksum::test()
{
	std::vector<const std::string> vecSources;
	vecSources.push_back("Test Message\n");
	vecSources.push_back("Hello World");

	std::string strMD5;
	{
		Checksum checksumMD5("MD5");
		bool bSuccess = checksumMD5.generate(vecSources);
		TEST_ASSERT(bSuccess);
		strMD5 = checksumMD5.toHexString();
		TEST_ASSERT(!strMD5.empty());
	}

	std::string strSHA256;
	{
		Checksum checksumSHA256("SHA256");
		bool bSuccess = checksumSHA256.generate(vecSources);
		TEST_ASSERT(bSuccess);
		strSHA256 = checksumSHA256.toHexString();
		TEST_ASSERT(!strSHA256.empty());
	}

	TEST_ASSERT(strMD5 != strSHA256);

	// Write the SHA256 one to file, get the checksum of the file, and make sure it's the same.
	const std::string strTest = "This is a test string.";

	// Each read from the buffer gets incorporated into the checksum, resulting in a different
	// checksum than if the entire file would be read into memory and then checksummed.
	// If the test string is larger than the buffer size, the verification step at the 
	// end (between checksum from string and file) will not match.
	assert(strTest.length() < knReadBufferSize);

	// Write to file.
	boost::filesystem::path pathSHA256 = AppServices::makeTempFile();
	{
		boost::filesystem::ofstream testOut(pathSHA256, std::ios::out);
		testOut << strTest;
	}

	std::string strSHA256FromString;
	{
		Checksum checksumSHA256("SHA256");
		bool bSuccess = checksumSHA256.generate(strTest);
		TEST_ASSERT(bSuccess);
		strSHA256FromString = checksumSHA256.toHexString();
		TEST_ASSERT(!strSHA256FromString.empty());
	}

	std::string strSHA256FromFile;
	{
		Checksum checksumSHA256("SHA256");
		bool bSuccess = checksumSHA256.generate(pathSHA256);
		TEST_ASSERT(bSuccess);
		strSHA256FromFile = checksumSHA256.toHexString();
		TEST_ASSERT(!strSHA256FromFile.empty());
	}

	TEST_ASSERT(strSHA256FromFile == strSHA256FromString);
}

SSLUtils::Checksum::Checksum(const std::string& strType) :
m_state(new SSLState(strType))
{
	if (0 == m_state->m_md)
	{
		//free memory before throwing an exception...
		delete m_state;
		m_state = 0;

		PYXTHROW(PYXException, "Invalid checksum type.");
	}
}

SSLUtils::Checksum::~Checksum()
{
	delete m_state;
}

bool SSLUtils::Checksum::generate(const std::string& strSource)
{
	boost::recursive_mutex::scoped_lock lock(m_state->m_mutex);

	MessageDigestContext mdctx(*m_state->m_md, m_state->m_arrBytes, m_state->m_nLength);
	return mdctx.update(strSource.c_str(), strSource.length());
}

//! Generate the checksum from the source memory.
bool SSLUtils::Checksum::generate(const PYXConstBufferSlice & source)
{
	boost::recursive_mutex::scoped_lock lock(m_state->m_mutex);

	MessageDigestContext mdctx(*m_state->m_md, m_state->m_arrBytes, m_state->m_nLength);
	return mdctx.update(source.begin(), source.size());
}



bool SSLUtils::Checksum::generate(const std::vector<const std::string>& vecSources)
{
	boost::recursive_mutex::scoped_lock lock(m_state->m_mutex);

	MessageDigestContext mdctx(*m_state->m_md, m_state->m_arrBytes, m_state->m_nLength);
	const std::vector<const std::string>::const_iterator itEnd = vecSources.end();
	for (std::vector<const std::string>::const_iterator it = vecSources.begin(); it < itEnd; ++it)
	{
		if (!mdctx.update(it->c_str(), it->length()))
		{
			return false;
		}
	}
	return true;
}

bool SSLUtils::Checksum::generate(const boost::filesystem::path& path)
{
	boost::recursive_mutex::scoped_lock lock(m_state->m_mutex);

	boost::filesystem::basic_ifstream<unsigned char> bifs(path, std::ios_base::binary);
	if (bifs.bad())
	{
		return false;
	}

	MessageDigestContext mdctx(*m_state->m_md, m_state->m_arrBytes, m_state->m_nLength);
	unsigned char buf[knReadBufferSize];
	while (!bifs.eof())
	{
		bifs.read(buf, sizeof (buf));
		const std::size_t nSize = (std::size_t)bifs.gcount();
		if (nSize <= 0)
		{
			assert(nSize == 0);
			return (nSize == 0);
		}
		if (!mdctx.update(buf, nSize)) 
		{
			return false;
		}
	}
	return true;
}

unsigned char SSLUtils::Checksum::getByte(unsigned int nIndex) const
{
	boost::recursive_mutex::scoped_lock lock(m_state->m_mutex);

	if (m_state->m_nLength <= nIndex)
	{
		PYXTHROW(PYXException, "Index out of range.");
	}

	return m_state->m_arrBytes[nIndex];
}

std::string SSLUtils::Checksum::toHexString() const
{
	boost::recursive_mutex::scoped_lock lock(m_state->m_mutex);

	std::string strHexString;

	char buf[EVP_MAX_MD_SIZE];
	for (unsigned int nIndex = 0; nIndex < m_state->m_nLength; ++nIndex) 
	{
		const int nChars = sprintf_s(buf, EVP_MAX_MD_SIZE, "%02x", m_state->m_arrBytes[nIndex]);
		strHexString.append(buf, nChars);
	}

	return strHexString;
}

/*!
Get the base 64 representation of the checksum

\return The base 64 representation of the checksum
*/
std::string SSLUtils::Checksum::toBase64String() const
{
	boost::recursive_mutex::scoped_lock lock(m_state->m_mutex);

	// null pad to the nearest multiple of 3 bytes
	unsigned int nPad = (3 - m_state->m_nLength % 3) % 3;
	unsigned int nPaddedLength = m_state->m_nLength + nPad;
	std::vector<unsigned char> buffer(nPaddedLength);

	memcpy(&buffer[0], m_state->m_arrBytes.data(), m_state->m_nLength);
	for (auto i = m_state->m_nLength; i < nPaddedLength; i++)
	{
		buffer[i] = '\0';
	}

	// perform base64 conversion
	using namespace boost::archive::iterators;
    typedef base64_from_binary<transform_width<std::vector<unsigned char>::const_iterator, 6, 8>> It;
    auto str = std::string(It(buffer.begin()), It(buffer.end()));

	// replace padded characters with '='s
	for (auto j = nPad; j > 0; j--)
	{
		str[str.length() - j] = '=';
	}

	return str;
}

const std::string& SSLUtils::Checksum::getType() const
{
	return m_state->m_strType;
}

unsigned int SSLUtils::Checksum::getLength() const
{
	return m_state->m_nLength;
}
