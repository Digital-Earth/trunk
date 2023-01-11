#ifndef PYXIS__UTILITY__IDENTITY_CACHE_H
#define PYXIS__UTILITY__IDENTITY_CACHE_H
/******************************************************************************
identity_cache.h

begin		: 2008-05-29
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

// stl includes
#include <string>

// boost includes
#include <boost/filesystem/path.hpp>

/*!
A directory cache for identities.
Each directory in the root has the following structure:

	<first n characters of hex identity checksum>/
		<collision #>/
			Identity.xml
			Data/
				<whatever data the client wants to put here>

Directories are created lazily.
*/
//! A directory cache for identities.
class PYXLIB_DECL IdentityCache
{
	//! The maximum length of the checksum directory name.
	static const int m_nMaxChecksumDirectoryNameLength = 32;

	//! The root directory for the cache.
	const boost::filesystem::path m_pathRoot;

	//! The type of checksum used for the checksum directory name.
	const std::string m_strChecksumType;

public:

	//! Test method
	static void test();

private:

	//! Return the name (not full path) of the identity file.
	static const std::string& getIdentityFileName();

	//! Return the name (not full path) of the data directory.
	static const std::string& getDataDirectoryName();

	//! Return true if the directory exists, creating if bCreate is true.
	static bool IdentityCache::ensureDirectoryExists(
		const boost::filesystem::path& pathDirectory, 
		bool bCreate);

	//! Read the identity file in the given directory into a string and return it.
	static std::string readIdentityFile(
		const boost::filesystem::path& pathDirectory);

	//! Write the identity to the identity file in the given directory.
	static bool writeIdentityFile(
		const boost::filesystem::path& pathDirectory,
		const std::string& strIdentity);

public:

	//! Construct an identity cache at the given root directory.
	IdentityCache(const boost::filesystem::path& pathRoot);

	virtual ~IdentityCache();

private:

	//! Return the name (not full path) of the checksum directory.
	std::string getChecksumDirectoryName(
		const std::string& strIdentity) const;

public:

	//! Return the path of the root directory for the cache.
	const boost::filesystem::path& getRootPath() const;

	//! Return the checksum type.
	const std::string& getChecksumType() const;

	//! Return the path of the data directory for the identity, or the empty path if none.
	virtual boost::filesystem::path getPath(
		const std::string& strIdentity,
		bool bCreate) const;
};

#endif
