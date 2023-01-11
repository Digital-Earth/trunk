/******************************************************************************
DynamicList.cs

begin      : April 30, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace PyxNet.Utilities
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
        private readonly List<T> m_list;

        /// <summary>
        /// An object for serializing access to the list of objects.
        /// </summary>
        private readonly object m_locker = new object();

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
        public event EventHandler<CountChangedEventArgs> CountChanged;

        /// <summary>
        /// Called when [count changed].
        /// </summary>
        protected void OnCountChanged()
        {
            EventHandler<CountChangedEventArgs> handler = CountChanged;

            if (handler != null)
            {
                handler(this, new CountChangedEventArgs(Count));
            }
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

        /// <summary>
        /// Event which is fired when an element is about to be added to the list.
        /// The AllowAddition member of the AddingElementEventArgs is examined after the
        /// event is fired and the potential element is added to the list if allowed.
        /// By default the element will be added.
        /// </summary>
        public event EventHandler<AddingElementEventArgs> AddingElement;

        /// <summary>
        /// Method to safely raise event OnAddingElement.
        /// </summary>
        protected bool OnAddingElement(T potentialElement, int elementCount)
        {
            AddingElementEventArgs args = new AddingElementEventArgs(potentialElement);
            EventHandler<AddingElementEventArgs> handler = AddingElement;
            if (null != handler)
            {
                handler(this, args);
            }
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

        /// <summary>
        /// Event which is fired when element has been added.
        /// </summary>
        public event EventHandler<ElementEventArgs> AddedElement;

        /// <summary>
        /// Method to safely raise event AddedElement.
        /// </summary>
        protected void OnAddedElement(T addedElement, int elementCount)
        {
            EventHandler<ElementEventArgs> handler = AddedElement;
            if (null != handler)
            {
                handler(this, new ElementEventArgs(addedElement, elementCount));
            }
        }

        #endregion AddedElement Event

        #region RemovedElement Event

        /// <summary>
        /// Event which is fired when element has been removed.
        /// </summary>
        public event EventHandler<ElementEventArgs> RemovedElement;

        /// <summary>
        /// Method to safely raise event RemovedElement.
        /// </summary>
        protected void OnRemovedElement(T removedElement, int elementCount)
        {
            EventHandler<ElementEventArgs> handler = RemovedElement;
            if (null != handler)
            {
                handler(this, new ElementEventArgs(removedElement, elementCount));
            }
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
        /// The method is not supported as it has not been made thread safe.
        /// </summary>
        IEnumerator<T> System.Collections.Generic.IEnumerable<T>.GetEnumerator()
        {
            // Copy the list, and return the enumerator of that.
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
        /// The method is not supported as it has not been made thread safe.
        /// </summary>
        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            // Copy the list, and return the enumerator of that.
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

    namespace Test
    {
        using NUnit.Framework;
        using System.Text;

        [TestFixture]
        public class DynamicListTester
        {
            TraceTool Trace = new TraceTool(false);

            class TestClass
            {
                public int data;
                public TestClass(int value)
                {
                    data = value;
                }
            }

            [Test]
            public void TestConstruction()
            {
                DynamicList<int> aList = new DynamicList<int>();
            }

            [Test]
            public void TestAddition()
            {
                DynamicList<int> aList = new DynamicList<int>();
                aList.Add(1);
                aList.Add(3);
                aList.Add(7);
                aList.Add(3);
                NUnit.Framework.Assert.IsTrue(aList.Count == 4);
                NUnit.Framework.Assert.IsTrue(aList[1] == 3);

                aList.RemoveAll(delegate(int element) { return element == 3; });
                NUnit.Framework.Assert.IsTrue(aList.Count == 2);
                NUnit.Framework.Assert.IsTrue(aList[0] == 1);
                NUnit.Framework.Assert.IsTrue(aList[1] == 7);
            }

            [Test]
            public void TestEvents()
            {
                bool shouldAdd = true;
                int numberOfAddedEvents = 0;

                EventHandler<DynamicList<int>.AddingElementEventArgs> handleAdding =
                    delegate(object sender, DynamicList<int>.AddingElementEventArgs args)
                    {
                        args.AllowAddition = shouldAdd;
                    };

                EventHandler<DynamicList<int>.ElementEventArgs> handleAdded =
                    delegate(object sender, DynamicList<int>.ElementEventArgs args)
                    {
                        numberOfAddedEvents++;
                        NUnit.Framework.Assert.IsTrue(
                        	args.ElementCount == numberOfAddedEvents);
                    };

                EventHandler<DynamicList<int>.ElementEventArgs> handleErroneousRemove =
                    delegate(object sender, DynamicList<int>.ElementEventArgs args)
                    {
                        // We're not doing any removing here, so this shouldn't get triggered.
                        NUnit.Framework.Assert.Fail();
                    };

                DynamicList<int> aList = new DynamicList<int>();
                aList.AddingElement += handleAdding;
                aList.AddedElement += handleAdded;
                aList.RemovedElement += handleErroneousRemove;

                aList.Add(1);
                shouldAdd = false;
                aList.Add(3);
                shouldAdd = true;
                aList.Add(7);

                NUnit.Framework.Assert.AreEqual(2, aList.Count);
                NUnit.Framework.Assert.AreEqual(aList.Count, numberOfAddedEvents);
                NUnit.Framework.Assert.AreEqual(7, aList[1]);

                // Remove each element, and ensure that the proper events are called.
                {
                    int numberOfRemovedEventsLeft = aList.Count;

                    EventHandler<DynamicList<int>.ElementEventArgs> handleRemoved =
                        delegate(object sender, DynamicList<int>.ElementEventArgs args)
                        {
                            --numberOfRemovedEventsLeft;
                            NUnit.Framework.Assert.AreEqual(
                            	args.ElementCount,
                            	numberOfRemovedEventsLeft);
                        };

                    aList.RemovedElement -= handleErroneousRemove;
                    aList.RemovedElement += handleRemoved;

                    for (int count = aList.Count; 0 < count; )
                    {
                        aList.Remove(aList[--count]);
                        NUnit.Framework.Assert.AreEqual(count, aList.Count);
                    }
                    NUnit.Framework.Assert.AreEqual(0, aList.Count);
                    NUnit.Framework.Assert.AreEqual(0, numberOfRemovedEventsLeft);
                }
            }

            [Test]
            public void TestThreadSafeIteration()
            {
                DynamicList<TestClass> myDynamicList = new DynamicList<TestClass>();

                System.Threading.ThreadPool.QueueUserWorkItem(delegate(Object unused)
                {
                    for (int count = 1; count < 100; ++count)
                    {
                        int newValue = PyxNet.Test.TestData.GenerateInt(10000);
                        Trace.WriteLine("Adding {0}.", newValue);
                        myDynamicList.Add(new TestClass(newValue));
                    }
                });

                System.Threading.ThreadPool.QueueUserWorkItem(delegate(Object unused)
                {
                    for (int iteration = 0; iteration < 10; ++iteration)
                    {
                        StringBuilder iterationText = new StringBuilder();
                        iterationText.AppendFormat("Iteration #{0} contains about {1} items. ",
                            iteration, myDynamicList.Count);
                        int itemNumber = 0;
                        foreach (TestClass item in myDynamicList)
                        {
                            iterationText.AppendFormat("{0}{1} ",
                                ((itemNumber > 0) ? ", " : ""),
                                item.data);
                            itemNumber++;
                        }
                        Trace.WriteLine(iterationText.ToString());
                        System.Threading.Thread.Sleep(1);
                    }
                });

                System.Threading.ThreadPool.QueueUserWorkItem(delegate(Object unused)
                {
                    for (int iteration = 0; iteration < 10; ++iteration)
                    {
                        StringBuilder iterationText = new StringBuilder();
                        iterationText.AppendFormat("Second Iteration #{0} contains about {1} items. ",
                            iteration, myDynamicList.Count);
                        int itemNumber = 0;
                        foreach (TestClass item in myDynamicList)
                        {
                            iterationText.AppendFormat("{0}{1} ",
                                ((itemNumber > 0) ? ", " : ""),
                                item.data);
                            itemNumber++;
                        }
                        Trace.WriteLine(iterationText.ToString());
                        System.Threading.Thread.Sleep(1);
                    }
                });
            }

            // TODO: write more unit tests that test the thread saftey of the dynamic list.
        }
    }
}
