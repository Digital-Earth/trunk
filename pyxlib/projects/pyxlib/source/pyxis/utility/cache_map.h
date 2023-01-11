#ifndef PYXIS__UTILITY__CACHE_MAP_H
#define PYXIS__UTILITY__CACHE_MAP_H
/******************************************************************************
cache_map.h

begin		: 2011-02-08
copyright	: (C) 2011 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

// pyxlib includes
#include "pyxlib.h"

// standard includes
#include <list>
#include <map>

template<typename Key,typename Value>
class CacheMap
{
protected:
	class ValueType
	{
	public:
		Value m_value;
		typedef typename std::list<Key>::iterator KeyIterator;
		KeyIterator m_usage;

		ValueType(const Value & value,const KeyIterator & usage) : m_value(value),m_usage(usage)
		{
		}

		ValueType() : m_value(),m_usage()
		{
		}

		ValueType(const ValueType & valueType) : m_value(valueType.m_value), m_usage(valueType.m_usage)
		{
		}

		ValueType & operator=(const ValueType & valueType)
		{
			m_value = valueType.m_value;
			m_usage = valueType.m_usage;
			return *this;
		}

		bool operator<(const Value & valueType) const
		{
			return m_vlaue < valueType.m_value;
		}
	};

	typedef std::list<Key> UsageList;
	typedef std::map<Key,ValueType> Map;

	unsigned int m_maxSize;
	mutable UsageList m_usage;
	Map m_map;

public:
	CacheMap(unsigned int maxSize = 100) : m_maxSize(maxSize)
	{
	}

	CacheMap(const CacheMap & other) : m_maxSize(other.m_maxSize)
	{
		//copy other map
		*this = other;
	}

	CacheMap & operator=(const CacheMap & other)
	{
		m_map.clear();
		m_usage.clear();
		m_maxSize = other.m_maxSize;

		//copy the map in reverse usage order.
		for(UsageList::const_reverse_iterator usageIt = other.m_usage.rbegin();usageIt != other.m_usage.rend(); ++usageIt)
		{
			Map::const_iterator it = other.m_map.find(*usageIt);
			add(it->first,it->second.m_value);
		}
		return *this;
	}

	CacheMap & operator=(const std::map<Key,Value> & map)
	{
		m_map.clear();
		m_usage.clear();
		m_maxSize = map.size();

		for(std::map<Key,Value>::const_iterator it = map.begin();it != map.end();++it)
		{
			add(it->first,it->second);
		}
		return *this;
	}

	CacheMap(const std::map<Key,Value> & map)
	{
		*this = map;
	}

	unsigned int size() const
	{
		return m_map.size();
	}

	unsigned int maxSize() const
	{
		return m_maxSize;
	}

	void setMaxSize(unsigned int maxSize)
	{
		m_maxSize = maxSize;

		while (m_maxSize < m_map.size())
		{
			removeLeastUsedItem();
		}
	}

	Key const * const getLeastUsedKey() const
	{
		if (m_usage.size()==0)
		{
			return 0;
		}
		return &(m_usage.back());
	}

	Key const * const getLastUsedKey() const
	{
		if (m_usage.size()==0)
		{
			return 0;
		}
		return &(m_usage.front());
	}

	void eraseLeastUsedItem()
	{
		if (m_usage.size()>0)
		{
			erase(m_usage.back());
		}
	}


	Value & operator[](const Key & key)
	{
		Map::iterator it = m_map.find(key);
		if (it == m_map.end())
		{
			//throw out the last used item
			if (m_map.size() == m_maxSize)
			{
				m_map.erase(m_usage.back());
				m_usage.pop_back();
			}

			ValueType & value = m_map[key] = ValueType(Value(),m_usage.insert(m_usage.begin(),key));
			return value.m_value;
		}
		else
		{
			m_usage.erase(it->second.m_usage);
			it->second.m_usage = m_usage.insert(m_usage.begin(),key);
			return it->second.m_value;
		}
	}

	const Value & operator[](const Key & key) const
	{
		Map::const_iterator it = map.find(key);

		if (it == m_map.end())
		{
 			PYXTHROW(PYXEception,"key not found in a map");
		}
		else
		{
			m_usage.erase(it->m_usage);
			m_usage.push_front(key);
			return it->second.m_value;
		}
	}

	bool exists(const Key & key) const
	{
		return (m_map.find(key) != m_map.end());
	}

	Value & add(const Key & key,const Value & value)
	{
		return (*this)[key] = value;
	}

	void erase(const Key & key)
	{
		Map::iterator it = m_map.find(key);
		if (it != m_map.end())
		{
			m_usage.erase(it->second.m_usage);
			m_map.erase(it);
		}
	}

	template<typename Func>
	void eraseIf(Func & f)
	{
		for(Map::iterator it = m_map.begin();it != m_map.end();)
		{
			if (f(it->first,it->second.m_value))
			{
				const Key & key = it->first;
				++it;
				erase(key);
			}
			else
			{
				++it;
			}
		}
	}

	void clear()
	{
		m_map.clear();
		m_usage.clear();
	}
};

#endif // guard