/******************************************************************************
WeakReferenceList.cs

begin      : March 12, 207
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;

namespace Pyxis.Utilities
{
    /// <summary>
    /// A list of weak references to instances of type T.
    /// </summary>
    /// <remarks>
    /// TODO: Implement list-related interfaces.
    /// TODO: Can this be implemented as a specialization of DynamicList?
    /// </remarks>
    public class WeakReferenceList<T> : ICollection<T>, IEnumerable<T> 
        where T : class
    {
        /// <summary>
        /// Class for passing CountChangedEventArgs used in the 
        /// WeakReferenceList<T>.CountChanged event.
        /// </summary>
        public class CountChangedEventArgs<U> : EventArgs where U : class
        {
            private readonly WeakReferenceList<U> m_list;

            /// <summary>
            /// The list for which the count changed.
            /// </summary>
            public WeakReferenceList<U> List
            {
                get { return m_list; }
            }

            /// <summary>
            /// Constructor
            /// </summary>
            /// <param name="list">The list that changed.</param>
            public CountChangedEventArgs(WeakReferenceList<U> list)
            {
                m_list = list;
            }
        }

        /// <summary>
        /// The largest number of elements this list has contained.
        /// </summary>
        private int m_maxCount = 0;

        /// <summary>
        /// The largest number of elements this list has contained.
        /// </summary>
        public int MaxCount
        {
            get { return m_maxCount; }
        }
        
        private EventHelper<CountChangedEventArgs<T>> m_CountChanged = new EventHelper<CountChangedEventArgs<T>>();

        /// <summary>
        /// Occurs when [count changed].
        /// </summary>
        public event EventHandler<CountChangedEventArgs<T>> CountChanged
        {
            add
            {
                m_CountChanged.Add(value);
            }
            remove
            {
                m_CountChanged.Remove(value);
            }
        }

        /// <summary>
        /// Called when [count changed].
        /// </summary>
        protected void OnCountChanged()
        {
            m_CountChanged.Invoke(this, new CountChangedEventArgs<T>(this));            
        }

        /// <summary>
        /// The list of weak references.
        /// </summary>
        private readonly List<TypedWeakReference<T>> m_references =
            new List<TypedWeakReference<T>>();

        /// <summary>
        /// Lock.
        /// </summary>
        private readonly object m_locker = new object();

        /// <summary>
        /// Finds the first match in the list.
        /// Note that this makes a copy of the list to ensure against reentrant deadlocks.
        /// </summary>
        /// <param name="isMatch">The predicate that is used to test each element for a match.</param>
        /// <returns>The first matching element in the list, or null if none.</returns>
        public T Find(Predicate<T> isMatch)
        {
            // Copy the list to avoid reentrant deadlock through the predicate.
            List<TypedWeakReference<T>> references;
            lock (m_locker)
            {
                references = new List<TypedWeakReference<T>>(m_references);
            }

        	// Find the element in the copy.
            T found = null;
            references.Find(delegate(TypedWeakReference<T> reference)
            {
                T element = (reference == null) ? null : reference.Target;
                if (null != element && isMatch(element))
                {
                    // We don't want to let go of our reference.
                    found = element;
                    return true;
                }

                return false;
            });
            return found;
        }

        /// <summary>
        /// Adds an element to the list.
        /// </summary>
        /// <param name="element">The element to add.</param>
        public void Add(T element)
        {
            lock (m_locker)
            {
                m_references.Add(new TypedWeakReference<T>(element));
                if (m_references.Count > m_maxCount)
                {
                    m_maxCount = m_references.Count;
                }
            }
            OnCountChanged();
        }

        /// <summary>
        /// Thread safe wrapper around the list method of the same name.
        /// </summary>
        public T this[int index]
        {
            get
            {
                lock (m_locker)
                {
                    return m_references[index];
                }
            }
            set
            {
                lock (m_locker)
                {
                    m_references[index] = new TypedWeakReference<T>(value);
                }
            }
        }

        /// <summary>
        /// Returns true if the list contains a specific element.
        /// </summary>
        /// <param name="element">The element to check for.</param>
        /// <returns>True if the list contains the element.</returns>
        public bool Contains(T element)
        {
            return null != Find(delegate(T candidate)
            {
                return candidate == element; 
            });
        }

        /// <summary>
        /// Removes all references to element.
        /// </summary>
        /// <param name="element">The element to remove all references from in the list.</param>
        /// <returns>True if any were removed.</returns>
        public bool Remove(T element)
        {
            lock (m_locker)
            {
                return 0 < m_references.RemoveAll(
                    delegate(TypedWeakReference<T> reference)
                    {
                        bool listChanged = (reference != null) && (reference.Target == element);
                        if (listChanged)
                        {
                            OnCountChanged();
                        }
                        return listChanged;
                    });
            }
        }

        /// <summary>
        /// Applies an action to each element in the list.
        /// Note that this makes a copy of the list to guarantee against reentrant deadlocks.
        /// </summary>
        /// <param name="action">The action to perform on each element in the list.</param>
        public void ForEach(Action<T> action)
        {
            // Copy the list to avoid reentrant deadlock through the action.
            List<TypedWeakReference<T>> references;
            lock (m_locker)
            {
                references = new List<TypedWeakReference<T>>(m_references);
            }

			// Apply the "ForEach" action to each element in the copy.
            references.ForEach(delegate(TypedWeakReference<T> reference)
            {
                T element = (reference == null) ? null : reference.Target;
                if (null != element)
                {
                    action(element);
                }
            });
        }

        /// <summary>
        /// Clears the list.
        /// </summary>
        public void Clear()
        {
            int precount;
            lock (m_locker)
            {
                precount = m_references.Count;
                m_references.Clear();
            }
            if (precount > 0)
            {
                // Only fire if we got rid of some.
                OnCountChanged();
            }
        }

        /// <summary>
        /// The number of elements in the list, including nulls.
        /// </summary>
        public int Count
        {
            get
            {
                lock (m_locker)
                {
                    return m_references.Count;
                }
            }
        }

        /// <summary>
        /// Copy to a List<T>
        /// </summary>
        /// <param name="list">The list to copy into</param>
        public void CopyTo(List<T> list)
        {
            // TODO: unit test.
            // Copy the list to avoid reentrant deadlock through the action.
            List<TypedWeakReference<T>> references;
            lock (m_locker)
            {
                references = new List<TypedWeakReference<T>>(m_references);
            }

            // Apply the "ForEach" action to each element in the copy.
            references.ForEach(delegate(TypedWeakReference<T> reference)
            {
                T element = (reference == null) ? null : reference.Target;
                if (null != element)
                {
                    list.Add(element);
                }
            });
        }

        #region ICollection<T> Members

        public void CopyTo(T[] array, int arrayIndex)
        {
            // TODO: unit test.
            // Copy the list to avoid reentrant deadlock through the action.
            List<TypedWeakReference<T>> references;
            lock (m_locker)
            {
                references = new List<TypedWeakReference<T>>(m_references);
            }

            // Apply the "ForEach" action to each element in the copy.
            references.ForEach(delegate(TypedWeakReference<T> reference)
            {
                T element = (reference == null) ? null : reference.Target;
                if (null != element)
                {
                    array[arrayIndex] = element;
                    ++arrayIndex;
                }
            });
        }

        public bool IsReadOnly
        {
            get { return false; }
        }

        #endregion

        #region IEnumerable<T> Members

        /// <summary>
        /// Caution:  while a returned enumerator exists, the references will not be weak.
        /// </summary>
        /// <returns>A typed enumerator to the list.</returns>
        public IEnumerator<T> GetEnumerator()
        {
            // TODO: unit test.
            List<T> list = new List<T>(m_references.Count);
            CopyTo(list);
            return list.GetEnumerator();
        }

        #endregion

        //#region IEnumerable Members

        /// <summary>
        /// Caution:  while a returned enumerator exists, the references will not be weak.
        /// </summary>
        /// <returns>An enumerator to the list.</returns>
        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            // TODO: unit test.
            List<T> list = new List<T>(m_references.Count);
            CopyTo(list);
            return list.GetEnumerator();
        }

        //#endregion
    }
}
