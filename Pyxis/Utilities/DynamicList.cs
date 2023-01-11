/******************************************************************************
DynamicList.cs

begin      : April 30, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace Pyxis.Utilities
{
    /// <summary>
    /// A generic list that notifies through the OnNewElement event of new entires into the list.
    /// It also gives a chance to deny new entries to the list through the AddingElement event.
    /// For synchronization the Locker property is provided, and this object is locked during any
    /// operation performed on the list from inside this class.  This is mostly a thin wrapper
    /// around the IList&lt;T&gt; interface other than the extentions mentioned above.
    /// All implemented properties and methods are thread safe.
    /// </summary>
    /// <typeparam name="T">The type of object to store in the list.</typeparam>
    public class DynamicList<T> : IList<T>, ICollection<T>, IEnumerable<T>
    {
        /// <summary>
        /// Storage for the elements in the list.
        /// </summary>
        protected readonly List<T> m_list;

        /// <summary>
        /// An object for serializing access to the list of objects.
        /// </summary>
        protected readonly object m_locker = new object();

        #region Construction

        /// <summary>
        /// Construct an empty list.
        /// </summary>
        public DynamicList()
        {
            m_list = new List<T>();
        }

        /// <summary>
        /// Construct a list whose elements are copied from an existing list.
        /// </summary>
        /// <param name="copyFrom">The list whose elements are to be copied into this list.</param>
        public DynamicList(IEnumerable<T> copyFrom)
        {
            m_list = new List<T>(copyFrom);
        }

        #endregion

        #region Events

        #region CountChanged Event
        /// <summary>
        /// Class for passing CountChangedEventArgs used in the 
        /// DynamicList<T>.CountChanged event.
        /// </summary>
        public class CountChangedEventArgs : EventArgs
        {
            private readonly int m_newCount;

            /// <summary>
            /// The new count for this list.
            /// </summary>
            public int NewCount
            {
                get { return m_newCount; }
            }

            /// <summary>
            /// Constructor
            /// </summary>
            /// <param name="count">The count.</param>
            public CountChangedEventArgs(int count)
            {
                m_newCount = count;
            }
        }

        /// <summary>
        /// Occurs when [count changed].
        /// </summary>
        public event EventHandler<CountChangedEventArgs> CountChanged
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
        private Pyxis.Utilities.EventHelper<CountChangedEventArgs> m_CountChanged = new Pyxis.Utilities.EventHelper<CountChangedEventArgs>();

        /// <summary>
        /// Called when [count changed].
        /// </summary>
        protected void OnCountChanged()
        {
            m_CountChanged.Invoke(this, new CountChangedEventArgs(Count));
        }
        #endregion

        #region AddingElement Event

        /// <summary>
        /// Class which will be passed as the second argument to a AddingElementHandler which 
        /// wraps a list entry object and a boolean that can be set to stop addition to the list.
        /// </summary>
        public class AddingElementEventArgs : EventArgs
        {
            private T m_potentialElement;

            public T PotentialElement
            {
                get { return m_potentialElement; }
                set { m_potentialElement = value; }
            }

            private bool m_allowAddition;

            public bool AllowAddition
            {
                get { return m_allowAddition; }
                set { m_allowAddition = value; }
            }

            internal AddingElementEventArgs(T potentialElement)
            {
                m_potentialElement = potentialElement;
                m_allowAddition = true;
            }
        }

        private EventHelper<AddingElementEventArgs> m_AddingElement = new EventHelper<AddingElementEventArgs>();

        /// <summary>
        /// Event which is fired when an element is about to be added to the list.
        /// The AllowAddition member of the AddingElementEventArgs is examined after the
        /// event is fired and the potential element is added to the list if allowed.
        /// By default the element will be added.
        /// </summary>
        public event EventHandler<AddingElementEventArgs> AddingElement
        {
            add
            {
                m_AddingElement.Add(value);
            }
            remove
            {
                m_AddingElement.Remove(value);
            }
        }

        /// <summary>
        /// Method to safely raise event OnAddingElement.
        /// </summary>
        protected bool OnAddingElement(T potentialElement, int elementCount)
        {
            AddingElementEventArgs args = new AddingElementEventArgs(potentialElement);
            m_AddingElement.Invoke(this, args);
            return args.AllowAddition;
        }

        #endregion AddingElement Event

        #region AddedElement Event

        /// <summary>
        /// Class which will be passed as the second argument to an element event handler which 
        /// wraps a list entry object.
        /// </summary>
        public class ElementEventArgs : EventArgs
        {
            private T m_element;

            public T Element
            {
                get { return m_element; }
                set { m_element = value; }
            }

            private int m_elementCount;

            /// <summary>
            /// The number of elements in the list immediately following the operation.
            /// Providing this makes thread safe clients simpler.
            /// </summary>
            public int ElementCount
            {
                get { return m_elementCount; }
                set { m_elementCount = value; }
            }

            internal ElementEventArgs(T element, int elementCount)
            {
                m_element = element;
                m_elementCount = elementCount;
            }
        }

        private EventHelper<ElementEventArgs> m_AddedElement = new EventHelper<ElementEventArgs>();

        /// <summary>
        /// Event which is fired when element has been added.
        /// </summary>
        public event EventHandler<ElementEventArgs> AddedElement
        {
            add
            {
                m_AddedElement.Add(value);
            }
            remove
            {
                m_AddedElement.Remove(value);
            }
        }

        /// <summary>
        /// Method to safely raise event AddedElement.
        /// </summary>
        protected void OnAddedElement(T addedElement, int elementCount)
        {
            m_AddedElement.Invoke( this, new ElementEventArgs(addedElement, elementCount));
        }

        #endregion AddedElement Event

        #region RemovedElement Event

        private EventHelper<ElementEventArgs> m_RemovedElement = new EventHelper<ElementEventArgs>();

        /// <summary>
        /// Event which is fired when element has been removed.
        /// </summary>
        public event EventHandler<ElementEventArgs> RemovedElement
        {
            add
            {
                m_RemovedElement.Add(value);
            }
            remove
            {
                m_RemovedElement.Remove(value);
            }
        }

        /// <summary>
        /// Method to safely raise event RemovedElement.
        /// </summary>
        protected void OnRemovedElement(T removedElement, int elementCount)
        {
            m_RemovedElement.Invoke(this, new ElementEventArgs(removedElement, elementCount));
        }

        #endregion RemovedElement Event

        #endregion

        #region IList<T> Members

        /// <summary>
        /// Thread safe wrapper around the list method of the same name.
        /// </summary>
        public int IndexOf(T item)
        {
            lock (m_locker)
            {
                return m_list.IndexOf(item);
            }
        }

        /// <summary>
        /// Thread safe wrapper around the list method of the same name.
        /// </summary>
        public void Insert(int index, T item)
        {
            //TODO:  in review -- decide if we should fire the AddingElement event and prevent the insertion.
            lock (m_locker)
            {
                m_list.Insert(index, item);
            }
            //TODO:  in review -- decide if we should fire the AddedElement event.
            OnCountChanged();
        }

        /// <summary>
        /// Thread safe wrapper around the list method of the same name.
        /// </summary>
        public void RemoveAt(int index)
        {
            lock (m_locker)
            {
                m_list.RemoveAt(index);
            }
            //TODO:  in review -- decide if we should fire the RemovedElement event.
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
                    return m_list[index];
                }
            }
            set
            {
                lock (m_locker)
                {
                    m_list[index] = value;
                }
            }
        }

        #endregion

        #region ICollection<T> Members

        /// <summary>
        /// Thread safe wrapper around the list method of the same name.
        /// </summary>
        public int Count
        {
            get
            {
                lock (m_locker)
                {
                    return m_list.Count;
                }
            }
        }

        /// <summary>
        /// Method to add to the end of the list.  This will first raise the OnAddingElement event
        /// to allow the addition to be verified.  By default the addition is allowed.
        /// If the addition is allowed, then the new item will be added to the list, and
        /// the OnNewElement event will be raise.
        /// </summary>
        /// <param name="item">The item to potentially add to the list.</param>
        new public void Add(T item)
        {
            Add(item, true);
        }

        /// <summary>
        /// Method to add to the end of the list.  This will first raise the OnAddingElement event
        /// to allow the addition to be verified.  By default the addition is allowed.
        /// If the addition is allowed, then the new item will be added to the list, and
        /// the OnNewElement event will be raise.
        /// </summary>
        /// <param name="item">The item to potentially add to the list.</param>
        /// <param name="allowDuplicate">If false, does not add if duplicate.</param>
        /// <returns>True if it was added.</returns>
        public bool Add(T item, bool allowDuplicate)
        {
            int count;
            lock (m_locker)
            {
                if (!allowDuplicate && m_list.Contains(item)) return false;
                if (!OnAddingElement(item, m_list.Count)) return false;
                m_list.Add(item);
                count = m_list.Count;
            }
            OnAddedElement(item, count);
            OnCountChanged();
            return true;
        }

        /// <summary>
        /// Thread safe wrapper around the list method of the same name.
        /// </summary>
        public void Clear()
        {
            lock (m_locker)
            {
                m_list.Clear();
            }
            OnCountChanged();
        }

        /// <summary>
        /// Thread safe wrapper around the list method of the same name.
        /// </summary>
        public bool Contains(T item)
        {
            lock (m_locker)
            {
                return m_list.Contains(item);
            }
        }

        /// <summary>
        /// Thread safe wrapper around the list method of the same name.
        /// </summary>
        public void CopyTo(T[] array, int arrayIndex)
        {
            lock (m_locker)
            {
                m_list.CopyTo(array, arrayIndex);
            }
        }

        /// <summary>
        /// Thread safe wrapper around the list method of the same name.
        /// </summary>
        public void CopyTo(T[] array)
        {
            lock (m_locker)
            {
                m_list.CopyTo(array);
            }
        }

        /// <summary>
        /// Copies the list to an array in a thread-safe manner.
        /// </summary>
        /// <returns>An array copy of the list.</returns>
        public T[] ToArray()
        {
            T[] array;
            lock (m_locker)
            {
                array = new T[m_list.Count];
                m_list.CopyTo(array);
            }
            return array;
        }

        /// <summary>
        /// Thread safe wrapper around the list method of the same name.
        /// </summary>
        public bool IsReadOnly
        {
            get
            {
                return false;
                // TODO:  why can't we use this code...
                // return m_list.IsReadOnly;
            }
        }

        /// <summary>
        /// Thread safe wrapper around the list method of the same name.
        /// </summary>
        public bool Remove(T item)
        {
            bool bRemoved;
            int count;
            lock (m_locker)
            {
                bRemoved = m_list.Remove(item);
                count = m_list.Count;
            }
            if (bRemoved)
            {
                OnRemovedElement(item, count);
            }
            if (bRemoved)
            {
                OnCountChanged();
            }
            return bRemoved;
        }

        #endregion

        #region IEnumerable<T> Members

        /// <summary>
        /// Copies the list, and returns the enumerator of that.
        /// </summary>
        IEnumerator<T> System.Collections.Generic.IEnumerable<T>.GetEnumerator()
        {
            List<T> list;
            lock (m_locker)
            {
                list = new List<T>(m_list);
            }
            return list.GetEnumerator();
        }

        #endregion

        #region IEnumerable Members

        /// <summary>
        /// Copies the list, and returns the enumerator of that.
        /// </summary>
        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            List<T> list;
            lock (m_locker)
            {
                list = new List<T>(m_list);
            }
            return list.GetEnumerator();
        }

        #endregion

        #region List<T> Members

        /// <summary>
        /// Finds the first matching element.
        /// </summary>
        /// <param name="match">The predicate that tests each element.</param>
        /// <returns>The found element,</returns>
        public T Find(Predicate<T> match)
        {
            // Copy the list, and Find on the copy.
            List<T> list;
            lock (m_locker)
            {
                list = new List<T>(m_list);
            }
            return list.Find(match);
        }

        /// <summary>
        /// Performs an action on each element.
        /// </summary>
        /// <param name="action">The action to perform on each element.</param>
        public void ForEach(Action<T> action)
        {
            // Copy the list, and ForEach on the copy.
            List<T> list;
            lock (m_locker)
            {
                list = new List<T>(m_list);
            }
            list.ForEach(action);
        }

        /// <summary>
        /// Returns a read-only version of the list.
        /// Note that this does a copy for thread safety.
        /// </summary>
        /// <returns>A read-only wrapper on a copy of the list.</returns>
        public ReadOnlyCollection<T> AsReadOnly()
        {
            List<T> list;
            lock (m_locker)
            {
                list = new List<T>(m_list);
            }
            return new ReadOnlyCollection<T>(list);
        }

        /// <summary>
        /// Makes a thread-safe copy of the list, iterates through,
        /// and does a thread-safe remove of each element that matches.
        /// Not the most efficient, but safe from thread locks.
        /// </summary>
        /// <param name="match">The predicate that returns true if the element is a match.</param>
        /// <returns>The number of elements removed.</returns>
        public int RemoveAll(Predicate<T> match)
        {
            ThreadSafeInt count = new ThreadSafeInt();
            ForEach(
                delegate(T element)
                {
                    if (match(element) && Remove(element))
                    {
                        ++count;
                    }
                });
            if (count > 0)
            {
                OnCountChanged();
            }
            return count;
        }

        #endregion        
    }    
}
