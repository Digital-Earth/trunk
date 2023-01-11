/******************************************************************************
identity_cache.cpp

begin		: 2008-05-28
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/identity_cache.h"

// pyxlib includes
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/ssl_utils.h"

// boost includes
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

// required for testing
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/ssl_utils.h"

//! Tester class
Tester<IdentityCache> gTester;

//! Test method
void IdentityCache::test()
{
	// Test parameters for identity cache.
	const boost::filesystem::path pathRoot = AppServices::makeTempDir();

	// Specify a directory.
	const IdentityCache cache(pathRoot);
	TEST_ASSERT(cache.getRootPath() == pathRoot);

	// Create a test identity to cache.
	const std::string strIdentity = "This is a test.";

	// Get path without create, and make sure empty path is returned.
	boost::filesystem::path path = cache.getPath(strIdentity, false);
	TEST_ASSERT(path.empty());

	// Get path with create, and make sure it exists under root.
	path = cache.getPath(strIdentity, true);
	TEST_ASSERT(!path.empty());
	TEST_ASSERT(FileUtils::exists(path));
	TEST_ASSERT(FileUtils::isDirectory(path));
	TEST_ASSERT(path > pathRoot);

	// Verify the directory properties.
	SSLUtils::Checksum checksum(cache.m_strChecksumType);
	checksum.generate(strIdentity);
	const std::string strChecksumDirectoryName = checksum.toHexString().substr(0, m_nMaxChecksumDirectoryNameLength);
	TEST_ASSERT(strChecksumDirectoryName == cache.getChecksumDirectoryName(strIdentity));
	const boost::filesystem::path pathChecksum = pathRoot / strChecksumDirectoryName;
	const boost::filesystem::path pathCollision = pathChecksum / "0";
	TEST_ASSERT(FileUtils::exists(pathCollision));
	TEST_ASSERT(path == pathCollision / getDataDirectoryName());

	// Call get path again, with create, and make sure the existing one gets returned.
	const boost::filesystem::path samePath = cache.getPath(strIdentity, true);
	TEST_ASSERT(samePath == path);

	// Force a collision, and make sure that the subdirectory gets created.
	// We simulate this by modifying the identity file that has been saved.
	const bool bWritten = writeIdentityFile(pathCollision, "This identity file has been messed with.");
	TEST_ASSERT(bWritten && "The identity file was not written.");
	const boost::filesystem::path pathForcedCollision = cache.getPath(strIdentity, true);
	TEST_ASSERT(pathForcedCollision == pathChecksum / "1" / getDataDirectoryName());

	// Modify identity and make sure that a different path is created.
	const std::string strDifferentIdentity = "This is a different test.";
	const boost::filesystem::path differentPath = cache.getPath(strDifferentIdentity, true);
	TEST_ASSERT(!differentPath.empty());
	TEST_ASSERT(FileUtils::exists(differentPath));
	TEST_ASSERT(FileUtils::isDirectory(differentPath));
	TEST_ASSERT(differentPath == pathRoot / cache.getChecksumDirectoryName(strDifferentIdentity) / "0" / getDataDirectoryName());
}

bool IdentityCache::ensureDirectoryExists(
	const boost::filesystem::path& pathDirectory, 
	bool bCreate)
{
	if (!FileUtils::exists(pathDirectory))
	{
		if (!bCreate)
		{
			return false;
		}
		boost::filesystem::create_directory(pathDirectory);
		return true;
	}
	if (!FileUtils::isDirectory(pathDirectory)) 
	{
		assert(false);
		return false;
	}
	return true;
}

const std::string& IdentityCache::getIdentityFileName()
{
	static const std::string strName = "Identity.xml";
	return strName;
}

const std::string& IdentityCache::getDataDirectoryName()
{
	static const std::string strName = "Data";
	return strName;
}

// TODO: Factor the read code out to file_utils or xml_utils.
std::string IdentityCache::readIdentityFile(
	const boost::filesystem::path& pathDirectory)
{
	// need to update the last write time of the file so the cache cleaner does not delete
	FileUtils::touchFile(pathDirectory / getIdentityFileName());

	std::stringstream buffer;
	boost::filesystem::basic_ifstream<char> bifs(pathDirectory / getIdentityFileName());
	buffer << bifs.rdbuf();
	return buffer.str();
}

// TODO: Factor the write code out to file_utils or xml_utils.
bool IdentityCache::writeIdentityFile(
	const boost::filesystem::path& pathDirectory,
	const std::string& strIdentity)
{
	boost::filesystem::path path = pathDirectory / getIdentityFileName();

	// Write the file and close via file stream destructor.
	{
		std::ofstream bofs;
		bofs.open( FileUtils::pathToString(path).c_str(), std::ios::out | std::ios::trunc);
		
		if (!bofs.good())
		{
			return false;
		}
		bofs << strIdentity;
	}

	// If the file exists: success.
	return (FileUtils::exists(path));
}

IdentityCache::IdentityCache(const boost::filesystem::path& pathRoot) :
	m_pathRoot(pathRoot),
	m_strChecksumType("SHA256")
{
}

IdentityCache::~IdentityCache()
{
}

const boost::filesystem::path& IdentityCache::getRootPath() const
{
	return m_pathRoot;
}

std::string IdentityCache::getChecksumDirectoryName(
	const std::string& strIdentity) const
{
	SSLUtils::Checksum checksum(m_strChecksumType);
	checksum.generate(strIdentity);
	return checksum.toHexString().substr(0, m_nMaxChecksumDirectoryNameLength);
}

boost::filesystem::path IdentityCache::getPath(
	const std::string& strIdentity,
	bool bCreate) const
{
	// If the identity string is empty, return an empty path.
	if (strIdentity.empty())
	{
		return boost::filesystem::path();
	}

	// Set the path to the root.
	boost::filesystem::path pathDirectory = m_pathRoot;
	if (!ensureDirectoryExists(pathDirectory, bCreate))
	{
		return boost::filesystem::path();
	}

	// Get the identity checksum.
	const std::string strCheckSum = getChecksumDirectoryName(strIdentity);

	// Set the directory path to the checksum subdirectory.
	pathDirectory /= strCheckSum;
	if (!ensureDirectoryExists(pathDirectory, bCreate))
	{
		return boost::filesystem::path();
	}

	// Find subdirectory.
	{
		// TODO: Address thread safety in the file system; what if the master directory gets erased?
		boost::filesystem::directory_iterator itEnd;
		for (boost::filesystem::directory_iterator itDir(pathDirectory); itDir != itEnd; ++itDir)
		{
			boost::filesystem::path pathSubdirectory = *itDir;

			// TODO: Thread safety in file system; it may no longer exist.
			assert(FileUtils::exists(pathSubdirectory));
			assert(FileUtils::isDirectory(pathSubdirectory));

			// Read the identity file.
			// Note that if it couldn't be written previously, this will return an empty string.
			const std::string strIdentityFile = readIdentityFile(pathSubdirectory);

			// Compare to the identity string.  If match, return true.
			if (strIdentity == strIdentityFile)
			{
				// Data directory.
				pathDirectory = pathSubdirectory / getDataDirectoryName();
				if (!ensureDirectoryExists(pathDirectory, true))
				{
					return boost::filesystem::path();
				}
				return pathDirectory;
			}
		}
	}

	// Not found.  If we're not creating, we're done.
	if (!bCreate)
	{
		return boost::filesystem::path();
	}

	// Append unique leaf to pathDirectory.
	// Name it with a number; if exists, increment and try again.
	// TODO: Filesystem threading issues?
	for (unsigned int nIndex = 0; ; ++nIndex)
	{
		std::ostringstream o;
		if (!(o << nIndex))
		{
			assert(false);
			return boost::filesystem::path();
		}
		std::string strLeaf = o.str();
		if (!FileUtils::exists(pathDirectory / strLeaf))
		{
			pathDirectory /= strLeaf;
			break;
		}
	}

	// Create the directory.
	boost::filesystem::create_directory(pathDirectory);

	// Create identity file in the directory.
	// If this fails, return an empty path.
	if (!writeIdentityFile(pathDirectory, strIdentity))
	{
		return boost::filesystem::path();
	}

	// Create data directory.
	// If this fails, return an empty path.
	pathDirectory /= getDataDirectoryName();
	if (!ensureDirectoryExists(pathDirectory, true))
	{
		return boost::filesystem::path();
	}

	return pathDirectory;
}
