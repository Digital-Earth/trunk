#ifndef PYXIS__PIPE__PROCESS_IDENTITY_CACHE_H
#define PYXIS__PIPE__PROCESS_IDENTITY_CACHE_H
/******************************************************************************
process_identity_cache.h

begin		: 2008-05-29
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/pipe/pipe_manager.h"
#include "pyxis/utility/identity_cache.h"

// stl includes
#include <string>

/*!
A directory cache for process identities.
Each directory in the root has the following structure:

	<first n characters of hex identity checksum>/
		<collision #>/
			Identity.xml
			Data/
				<whatever data the client wants to put here>

Directories are created lazily.
*/
//! A directory cache for process identities.
class PYXLIB_DECL ProcessIdentityCache : public IdentityCache
{

public:

	//! Test method
	static void test();

public:

	//! Construct an identity cache at the given root directory.
	ProcessIdentityCache(
		const boost::filesystem::path& pathRoot);

	virtual ~ProcessIdentityCache();

public:

	//! Return the path of the data directory for the process identity, or the empty path if none.
	virtual boost::filesystem::path getPath(
		const std::string& strIdentity,
		bool bCreate) const;

	//! Return the path of the data directory for the process, or the empty path if none.
	virtual boost::filesystem::path getPath(
		boost::intrusive_ptr<IProcess> spProcess,
		bool bCreate) const;

	//! Return the path of the data directory for the process, or the empty path if none.
	virtual boost::filesystem::path getPath(
		const ProcRef& procRef,
		bool bCreate) const;
};

#endif
