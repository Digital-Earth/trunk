/******************************************************************************
ThreadSafeDictionary.cs

begin      : April 8, 2010
copyright  : (c) 2010 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities
{
    /// <summary>
    /// This class is thread-safe, so that all operations are atomic (they
    /// happen within a lock, and therefore won`t interfere with other calls.)
    /// Note that this class does not currently support "transactional" level
    /// locking.
    /// </summary>
    /// <typeparam name="TKey">The type of the key.</typeparam>
    /// <typeparam name="TValue">The type of the value.</typeparam>
    public class ThreadSafeDictionary<TKey, TValue>: IDictionary<TKey, TValue>
    {
        private Dictionary<TKey, TValue> m_implementation = new Dictionary<TKey, TValue>();
        private object m_lock = new object();

        #region IDictionary<TKey,TValue> Members

        public void Add(TKey key, TValue value)
        {
            lock (m_lock)
            {
                m_implementation.Add(key, value);
            }
        }

        public bool ContainsKey(TKey key)
        {
            lock (m_lock)
            {
                return m_implementation.ContainsKey( key);
            }
        }

        public ICollection<TKey> Keys
        {
            get
            {
                TKey[] array;
                lock (m_lock)
                {
                    array = new TKey[m_implementation.Keys.Count];
                    m_implementation.Keys.CopyTo(array, 0);
                }
                return array;
            }
        }

        public bool Remove(TKey key)
        {
            lock (m_lock)
            {
                return m_implementation.Remove(key);
            }
        }

        public bool TryGetValue(TKey key, out TValue value)
        {
            lock (m_lock)
            {
                return m_implementation.TryGetValue(key, out value);
            }
        }

        public ICollection<TValue> Values
        {
            get
            {
                TValue[] array;
                lock (m_lock)
                {
                    array = new TValue[m_implementation.Keys.Count];
                    m_implementation.Values.CopyTo(array, 0);
                }
                return array;
            }
        }

        public TValue this[TKey key]
        {
            get
            {
                lock (m_lock)
                {
                    return m_implementation[key];
                }
            }
            set
            {
                lock (m_lock)
                {
                    m_implementation[key] = value;
                }
            }
        }

        #endregion

        #region ICollection<KeyValuePair<TKey,TValue>> Members

        public void Add(KeyValuePair<TKey, TValue> item)
        {
            lock (m_lock)
            {
                var alternateImplementation = m_implementation as ICollection<KeyValuePair<TKey,TValue>>;
                alternateImplementation.Add(item);
            }
        }

        public void Clear()
        {
            lock (m_lock)
            {
                m_implementation.Clear();
            }
        }

        public bool Contains(KeyValuePair<TKey, TValue> item)
        {
            lock (m_lock)
            {
                return (m_implementation as ICollection<KeyValuePair<TKey, TValue>>).Contains( item);
            }
        }

        public void CopyTo(KeyValuePair<TKey, TValue>[] array, int arrayIndex)
        {
            lock (m_lock)
            {
                (m_implementation as ICollection<KeyValuePair<TKey, TValue>>).CopyTo( array, arrayIndex);
            }
        }

        public int Count
        {
            get 
            {
                lock (m_lock)
                {
                    return (m_implementation as ICollection<KeyValuePair<TKey, TValue>>).Count;
                }
            }
        }

        public bool IsReadOnly
        {
            get
            {
                lock (m_lock)
                {
                    return (m_implementation as ICollection<KeyValuePair<TKey, TValue>>).IsReadOnly;
                }
            }
        }

        public bool Remove(KeyValuePair<TKey, TValue> item)
        {
            lock (m_lock)
            {
                return (m_implementation as ICollection<KeyValuePair<TKey, TValue>>).Remove(item);
            }
        }

        #endregion

        #region IEnumerable<KeyValuePair<TKey,TValue>> Members

        public IEnumerator<KeyValuePair<TKey, TValue>> GetEnumerator()
        {
            List<KeyValuePair<TKey, TValue>> result = new List<KeyValuePair<TKey,TValue>>();
            lock (m_lock)
            {
                foreach (var item in (m_implementation as ICollection<KeyValuePair<TKey, TValue>>))
                {
                    result.Add(item);
                }
            }
            return result.GetEnumerator();
        }

        #endregion

        #region IEnumerable Members

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            List<KeyValuePair<TKey, TValue>> result = new List<KeyValuePair<TKey, TValue>>();
            lock (m_lock)
            {
                foreach (var item in (m_implementation as ICollection<KeyValuePair<TKey, TValue>>))
                {
                    result.Add(item);
                }
            }
            return result.GetEnumerator();
        }

        #endregion
    }
}
