using System;
using System.Collections;
using System.Collections.Generic;

namespace Pyxis.Utilities
{
    /// <summary>
    /// Dictionary with a size limit. The Dictionary will "forget" items using LRU method.
    /// 
    /// Every time an item been accessed it would be moved to the head of the LRU list.
    /// Items will be removed from the dictionary from the end of the LRU list.
    /// 
    /// Item is consider to be accessed by using the following methods:
    /// 1) Add
    /// 2) Dictionary[Tkey] { get; set; }
    /// 3) TryGetValue(key,out value) - if key exists
    /// 
    /// Please note all others methods like: Contains, GetEnumerator, Keys, Values 
    /// doesn't count as "accessing" an dictionary item
    /// </summary>
    /// <typeparam name="TKey">Type to be used as Key</typeparam>
    /// <typeparam name="TValue">Type to be used as Value</typeparam>
    public class LimitedSizeDictionary<TKey, TValue> : IDictionary<TKey, TValue>
    {
        private readonly Dictionary<TKey, TValue> m_dictionary = new Dictionary<TKey, TValue>();
        private readonly List<TKey> m_keysAccess = new List<TKey>();
        private readonly int m_sizeLimit;

        private void TrackKeyUsage(TKey key)
        {
            m_keysAccess.Remove(key);
            m_keysAccess.Add(key);

            while (m_keysAccess.Count > m_sizeLimit)
            {
                m_dictionary.Remove(m_keysAccess[0]);
                m_keysAccess.RemoveAt(0);
            }
        }

        public LimitedSizeDictionary(int sizeLimit)
        {
            m_sizeLimit = Math.Max(1, sizeLimit);
        }

        public int SizeLimit
        {
            get
            {
                return m_sizeLimit;
            }
        }

        public void Add(KeyValuePair<TKey, TValue> item)
        {
            Add(item.Key, item.Value);
        }

        public void Add(TKey key, TValue value)
        {
            m_dictionary.Add(key, value);
            TrackKeyUsage(key);
        }
       
        public void Clear()
        {
            m_dictionary.Clear();
            m_keysAccess.Clear();
        }
            
        public bool Contains(KeyValuePair<TKey, TValue> item)
        {
            return (m_dictionary as ICollection<KeyValuePair<TKey, TValue>>).Contains(item);
        }

        public void CopyTo(KeyValuePair<TKey, TValue>[] array, int arrayIndex)
        {
            (m_dictionary as ICollection<KeyValuePair<TKey,TValue>>).CopyTo(array,arrayIndex);
        }

        public bool Remove(KeyValuePair<TKey, TValue> item)
        {
            return Remove(item.Key);
        }

        public void CopyTo(Array array, int index)
        {
            (m_dictionary as ICollection).CopyTo(array, index);
        }

        public int Count
        {
            get { return m_dictionary.Count;  }
        }

        public object SyncRoot
        {
            get { return ((ICollection) m_dictionary).SyncRoot; }
        }
        public bool IsSynchronized
        {
            get { return ((ICollection)m_dictionary).IsSynchronized; }
        }
        
        public bool IsReadOnly
        {
            get { return false; }
        }

        public bool IsFixedSize
        {
            get { return false; }             
        }

        public bool ContainsKey(TKey key)
        {
            return m_dictionary.ContainsKey(key);
        }

        public bool Remove(TKey key)
        {
            m_keysAccess.Remove(key);
            return m_dictionary.Remove(key);
        }

        public bool TryGetValue(TKey key, out TValue value)
        {
            if (!m_dictionary.TryGetValue(key, out value))
            {
                return false;
            }
            TrackKeyUsage(key);
            return true;
        }

        public TValue this[TKey key]
        {
            get
            {
                var value = m_dictionary[key];
                TrackKeyUsage(key);
                return value;
            }
            set
            {
                m_dictionary[key] = value;
                TrackKeyUsage(key);
            }
        }

        public ICollection<TKey> Keys
        {
            get { return m_dictionary.Keys; }
        }

        public ICollection<TValue> Values 
        {
            get { return m_dictionary.Values; }
        }

        public IEnumerator<KeyValuePair<TKey, TValue>> GetEnumerator()
        {
            return m_dictionary.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return m_dictionary.GetEnumerator();
        }
    }
}