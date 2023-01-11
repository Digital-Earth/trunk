/******************************************************************************
local_storage.cpp

begin		: Dec 15, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#define PYXLIB_SOURCE
#include "stdafx.h"
#include "pyxis/utility/local_storage.h"
#include "pyxis/utility/local_storage_impl.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/file_utils.h"

// standard includes
#include <cassert>

class PYXTempLocalStorageImpl : public PYXLocalStorage
{
private:
	boost::filesystem::path m_storageFile; 
	PYXPointer<PYXLocalStorage> m_storage;


public:
	PYXTempLocalStorageImpl()
	{
		m_storage = PYXLocalStorageFactory::createSqlite(FileUtils::pathToString(AppServices::makeTempFile(".sqlite")));
	}

	virtual ~PYXTempLocalStorageImpl()
	{
		m_storage.reset();
		boost::filesystem::remove(m_storageFile);
	}

public:
	static PYXPointer<PYXTempLocalStorageImpl> create()
	{
		return PYXNEW(PYXTempLocalStorageImpl);
	}

public:
	virtual std::auto_ptr<PYXConstWireBuffer> get(const std::string & key) 
	{
		return m_storage->get(key);
	}

	virtual void set(const std::string &key, PYXWireBuffer & data) 
	{
		m_storage->set(key,data);
	}

	virtual void setMany(const std::map<std::string,PYXConstBufferSlice> & data)
	{
		m_storage->setMany(data);
	}

	virtual void remove(const std::string & key)
	{
		m_storage->remove(key);
	}

	virtual void removeAll()
	{
		m_storage->removeAll();
	}

	virtual void applyChanges(const std::vector<PYXPointer<PYXLocalStorageChange>> & changes)
	{
		m_storage->applyChanges(changes);
	}
};

PYXPointer<PYXLocalStorage> PYXTempLocalStorage::create()
{
	return PYXTempLocalStorageImpl::create();
};
