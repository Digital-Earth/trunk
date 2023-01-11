/******************************************************************************
process_identity_cache.cpp

begin		: 2008-05-28
copyright	: (C) 2008 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/pipe/process_identity_cache.h"

// pyxlib includes
#include "pyxis/utility/tester.h"

//! Tester class
Tester<ProcessIdentityCache> gTester;

//! Test method
void ProcessIdentityCache::test()
{
	// Note that the core functionality is tested in the base class.
}

ProcessIdentityCache::ProcessIdentityCache(
	const boost::filesystem::path& pathRoot) :
	IdentityCache(pathRoot)
{
}

ProcessIdentityCache::~ProcessIdentityCache()
{
}

boost::filesystem::path ProcessIdentityCache::getPath(
	const std::string& strIdentity,
	bool bCreate) const
{
	return IdentityCache::getPath(strIdentity, bCreate);
}

boost::filesystem::path ProcessIdentityCache::getPath(
	boost::intrusive_ptr<IProcess> spProcess,
	bool bCreate) const
{
	if (!spProcess)
	{
		return boost::filesystem::path();
	}

	// Return the path for the identity.
	return getPath(spProcess->getIdentity(), bCreate);
}

boost::filesystem::path ProcessIdentityCache::getPath(
	const ProcRef& procRef,
	bool bCreate) const
{
	return getPath(PipeManager::getProcess(procRef), bCreate);
}
