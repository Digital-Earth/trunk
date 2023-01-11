/******************************************************************************
process_local_storage.cpp

begin		: Dec 15, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/pipe/process_local_storage.h"
#include "pyxis/pipe/process_identity_cache.h"
#include "pyxis/utility/local_storage_impl.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/exceptions.h"

// standard includes
#include <cassert>


PYXPointer<PYXLocalStorage> PYXProcessLocalStorage::create(const PYXPointer<IProcess> & process)
{
	return PYXProcessLocalStorage::create(process->getIdentity());
}

PYXPointer<PYXLocalStorage> PYXProcessLocalStorage::create(const std::string & identity)
{
	boost::filesystem::path cacheDir = AppServices::getCacheDir("ProcessCache");
	const ProcessIdentityCache cache(cacheDir);
	const boost::filesystem::path pathCache = cache.getPath(identity, true);
	if (pathCache.empty())
	{
		PYXTHROW(PYXException, "Could not create cache directory.");
	}
	std::string dbPath = FileUtils::pathToString(pathCache);

	return PYXLocalStorageFactory::create(dbPath);
}