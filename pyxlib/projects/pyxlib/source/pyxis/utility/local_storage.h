#ifndef PYXIS__UTILITY__LOCAL_STORAGE_H
#define PYXIS__UTILITY__LOCAL_STORAGE_H
/******************************************************************************
local_storage.h

begin		: Dec 15, 2011
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/wire_buffer.h"

#include "boost/thread/recursive_mutex.hpp"

#include "set"

///////////////////////////////////////////////////////////////////////////////
// PYXLocalStorageChange
///////////////////////////////////////////////////////////////////////////////
/*!
PYXLocalStorageChange allow to store a change for key/value record to be applied on a local storage.
Please see PYXLocalStorage::applyChanges(std::vector<PYXPointer<PYXLocalStorageChange>> changes) on how to apply a transactional changes.
*/
class PYXLIB_DECL PYXLocalStorageChange : public PYXObject
{
public:
	enum ChangeType
	{
		knSet = 1,
		knRemove = 2,
		knRemoveAll = 4,
	};

private:
	ChangeType m_changeType;
	std::string m_key;
	PYXPointer<PYXConstBufferSlice> m_data;

public:
	//! create a set(key,value) change
	static PYXPointer<PYXLocalStorageChange> createSet(const std::string & key, const PYXPointer<PYXConstBufferSlice> & data)
	{
		return PYXNEW(PYXLocalStorageChange,ChangeType::knSet,key,data);
	}

	//! create a remove(key) change
	static PYXPointer<PYXLocalStorageChange> createRemove(const std::string & key)
	{
		return PYXNEW(PYXLocalStorageChange,ChangeType::knRemove,key,nullptr);
	}

	//! create a removeAll() change
	static PYXPointer<PYXLocalStorageChange> createRemoveAll()
	{
		return PYXNEW(PYXLocalStorageChange,ChangeType::knRemoveAll,"",nullptr);
	}

	PYXLocalStorageChange(ChangeType changeType,const std::string & key, const PYXPointer<PYXConstBufferSlice> & data)
		: m_changeType(changeType), m_key(key), m_data(data)
	{
	}

public:
	ChangeType getChangeType() const { return m_changeType; }
	const std::string & getKey() const { return m_key; }
	const PYXPointer<PYXConstBufferSlice> & getData() const { return m_data; }
};

///////////////////////////////////////////////////////////////////////////////
// PYXLocalStorage 
///////////////////////////////////////////////////////////////////////////////
/*!
Local storage interface to provide to have a persitance storage of key/value.

PYXLocalStorage interface only allow to get/set/remove by key. it doesn't provide enumaration functionality.
*/
class PYXLIB_DECL PYXLocalStorage : public PYXObject
{
protected:
	PYXLocalStorage() {}

private:
	PYXLocalStorage(const PYXLocalStorage & other) {}
	PYXLocalStorage& operator=(const PYXLocalStorage & other) { return *this; }

public:
	virtual std::auto_ptr<PYXConstWireBuffer> get(const std::string & key) = 0;
	virtual void set(const std::string &key, PYXWireBuffer & data) = 0;
	virtual void setMany(const std::map<std::string,PYXConstBufferSlice> & data) = 0;
	virtual void remove(const std::string & key) = 0;

	virtual void applyChanges(const std::vector<PYXPointer<PYXLocalStorageChange>> & changes) = 0;

	virtual void removeAll() = 0;
};

///////////////////////////////////////////////////////////////////////////////
// PYXBufferedLocalStorage 
///////////////////////////////////////////////////////////////////////////////
/*!
PYXBufferedLocalStorage provide a buffer layer before a storage layer to store several modifictions before they are sent to underlying storage.
*/
class PYXLIB_DECL PYXBufferedLocalStorage : public PYXLocalStorage
{
public:
	static PYXPointer<PYXBufferedLocalStorage> create(const PYXPointer<PYXLocalStorage> & storage)
	{
		PYXBufferedLocalStorage * buffered = dynamic_cast<PYXBufferedLocalStorage*>(storage.get());

		if (buffered != NULL)
		{
			return buffered;
		}

		return PYXNEW(PYXBufferedLocalStorage,storage);
	}

	PYXBufferedLocalStorage(const PYXPointer<PYXLocalStorage> & storage) 
		: m_storage(storage), m_totalBufferSize(0), m_maxBufferSize(s_maxBufferSize)
	{
		PYXBufferedLocalStorage * buffered = dynamic_cast<PYXBufferedLocalStorage*>(m_storage.get());

		if (buffered != NULL)
		{
			m_storage = buffered->m_storage;
		}
	}

	~PYXBufferedLocalStorage()
	{
		commit();
	}

	int getBufferSize() const 
	{
		return m_maxBufferSize;
	}

	void setBufferSize(int maxBufferSizeInBytes)
	{
		if (maxBufferSizeInBytes <= 0)
		{
			PYXTHROW(PYXException,"max buffer size must be larger than 0");
		}

		m_maxBufferSize = maxBufferSizeInBytes;

		if (m_totalBufferSize >= m_maxBufferSize) 
		{
			commit();
		}
	}

private:
	PYXBufferedLocalStorage(const PYXLocalStorage & other) {}
	PYXBufferedLocalStorage & operator=(const PYXLocalStorage & other) { return *this; }

public:
	virtual std::auto_ptr<PYXConstWireBuffer> get(const std::string & key)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		auto it = m_sets.find(key);

		if (it != m_sets.end())
		{
			return std::auto_ptr<PYXConstWireBuffer>(new PYXConstWireBuffer(it->second));
		}

		auto removeIt = m_removes.find(key);
		if (removeIt != m_removes.end())
		{
			return std::auto_ptr<PYXConstWireBuffer>();
		}
		
		return m_storage->get(key);
	}

	virtual void set(const std::string &key, PYXWireBuffer & data)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		m_sets[key] = data.getBuffer();
		m_totalBufferSize += m_sets[key]->size();

		m_removes.erase(key);
		
		if (m_totalBufferSize > m_maxBufferSize)
		{
			commit();
		}
	}

	virtual void setMany(const std::map<std::string,PYXConstBufferSlice> & data) 
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		for(std::map<std::string,PYXConstBufferSlice>::const_iterator it = data.begin();it != data.end(); ++ it)
		{
			m_sets[it->first] = PYXConstBufferSlice::create(it->second);
			m_totalBufferSize += it->second.size();
			m_removes.erase(it->first);
		}

		if (m_totalBufferSize > m_maxBufferSize)
		{
			commit();
		}
	}

	virtual void remove(const std::string & key)
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		m_removes.insert(key);

		auto it = m_sets.find(key);

		if (it != m_sets.end())
		{
			m_totalBufferSize -= it->second->size();
			m_sets.erase(it);
		}
	}

	virtual void removeAll()
	{
		{
			boost::recursive_mutex::scoped_lock lock(m_mutex);
			m_sets.clear();
			m_removes.clear();
			m_totalBufferSize = 0;
		}

		m_storage->removeAll();
	}

	virtual void applyChanges(const std::vector<PYXPointer<PYXLocalStorageChange>> & changes)
	{
		commit();
		m_storage->applyChanges(changes);
	}

public:
	void commit()
	{
		boost::recursive_mutex::scoped_lock lock(m_mutex);

		std::vector<PYXPointer<PYXLocalStorageChange>> changes;

		if (m_sets.size()>0)
		{
			for(auto & set : m_sets)
			{
				changes.push_back(PYXLocalStorageChange::createSet(set.first,set.second));
			}
			m_sets.clear();
			m_totalBufferSize = 0;
		}

		if (m_removes.size()>0)
		{
			for(auto & remove : m_removes)
			{
				changes.push_back(PYXLocalStorageChange::createRemove(remove));
			}

			m_removes.clear();
		}

		if (changes.size() > 0)
		{
			m_storage->applyChanges(changes);
		}
	}

private:
	boost::recursive_mutex m_mutex;
	std::map<std::string,PYXPointer<PYXConstBufferSlice>> m_sets;
	std::set<std::string> m_removes;
	static const int s_maxBufferSize = 10*1024*1024;
	int m_maxBufferSize;
	int m_totalBufferSize;
	PYXPointer<PYXLocalStorage> m_storage;
};

///////////////////////////////////////////////////////////////////////////////
// PYXTempLocalStorage
///////////////////////////////////////////////////////////////////////////////

class PYXLIB_DECL PYXTempLocalStorage
{
public:
	static PYXPointer<PYXLocalStorage> create();
};

#endif // guard
