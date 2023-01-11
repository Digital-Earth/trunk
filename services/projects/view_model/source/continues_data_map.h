#pragma once
/******************************************************************************
continues_data_map.h

begin		: 2010-06-01
copyright	: (C) 2010 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

#include "view_model_api.h"

#include "pyxis/utility/object.h"

#include <map>
#include <vector>


/*!
ContinuesDataMap is wrapper class that store the information inside a vector to allow passing the data information as a single memory chunk

ContiunesDataMap allow the user to access the items as a std::map<Key,T> with the following API:
1. get() method and operator[] - to fetch items.
2. insert(Key,Value) - to insert new items.
3. erase(Key) - to erase an item.

However, there is a difference with iterators. the iterator given by this class is not for std::map<Key,T> but it for std::vector<T>.
therefore, there is no it->first,it->second for the iterator. only (*it) and it-> as used for vectors.
the API for that is:
1. begin() - return first item inside the data vector.
2. end() - return the end iterator of the data vector.
4. find(Key) - return iterator of where this item is placed inside the vector.

Moreover, there is additional API:
1. getFirstItem() - return a reference for the first item inside the data vector
2. getKey(index) - return the key for item inside the data vector at #index place.
3. getIndex(Key) - return the position inside the data vector of the specific Key

In order to implement this class. the ContinuesDataMap uses 3 internal data structures:
1. m_data - the data vector.
2. m_access - map from Key to size_t (index)
3. m_reverse_access - vector of Keys to provide reverse access. needed for operation like erase.
*/
template<class Key,class T>
class ContinuesDataMap : public PYXObject
{
protected:
	//! the data vector where the items is saved in a continues memory block
	std::vector<T> m_data;

	//! a reverse access that allow to find the key for item at #index inside the data vector
	std::vector<Key> m_reverse_access;

	//! a direct access that translate between the Key and #index inside the data vector
	std::map<Key,size_t> m_access;

public:
	typedef typename std::vector<T>::iterator iterator;
	typedef typename std::vector<T>::const_iterator const_iterator;	

public:
	ContinuesDataMap() {}	

	virtual ~ContinuesDataMap() {}
	
	inline T & getFirstItem()
	{
		return m_data[0];
	}

	inline const T & getFirstItem() const
	{
		return m_data[0];
	}

	inline iterator begin()
	{
		return m_data.begin();
	}

	inline iterator end()
	{
		return m_data.end();
	}	
	
	inline const_iterator begin() const
	{
		return m_data.begin();
	}

	inline const_iterator end() const
	{
		return m_data.end();
	}

	inline size_t size() const
	{
		return m_data.size();
	}
	
	inline T & get(const Key & key)
	{
		return m_data[m_access[key]];
	}

	inline T & operator[](const Key & key)
	{
		return m_data[m_access[key]];
	}

	inline const T & get(const Key & key) const
	{
		return m_data[m_access[key]];
	}

	inline const T & operator[](const Key & key) const
	{
		return m_data[m_access[key]];
	}

	void insert(const Key & key,T & data)
	{
		//add item to the end of our data block
		m_data.push_back(data);
		m_reverse_access.push_back(key);
		m_access[key] = m_data.size()-1;		
	}

	void erase(const Key & key)
	{
		//get index and last key
		size_t earsed_index = m_access[key];
		Key last_key = m_reverse_access.back();
		
		//if we don't erased the last item
		if (earsed_index != m_access.size()-1)
		{
			//copy the last item to the erased item
			m_data[earsed_index] = m_data.back();

			//move the last item to the new freed position
			m_access[last_key] = earsed_index;
			m_reverse_access[earsed_index] = last_key;
		}

		//remove the last item + and the key
		m_access.erase(key);
		m_data.pop_back();
		m_reverse_access.pop_back();
	}

	inline iterator find(Key & key) const
	{
		std::map<Key,size_t>::iterator it = m_access.find(key);
		
		if (it == m_access.end())
		{
			return end();
		}
		else
		{
			return begin()+it->second;
		}
	}

	inline const Key & getKey(size_t index) const
	{
		return m_reverse_access[index];
	}

	inline const size_t getIndex(const Key & key) const
	{
		return m_access[key];
	}
};