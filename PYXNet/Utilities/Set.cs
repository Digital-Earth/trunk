/******************************************************************************
Set.cs

begin      : September 29, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

namespace PyxNet.Utilities
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Collections.ObjectModel;

    // TODO: Add more unit tests before using this class.
    /// <summary>
    /// A thread-safe set.
    /// The items stored within should have GetHashCode() implemented.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public class Set<T> : ICollection<T>, IEnumerable<T>, IEnumerable
    {
        /// <summary>
        /// Storage for the elements in the set.
        /// </summary>
        private readonly Dictionary<T, T> m_set = new Dictionary<T, T>();

        /// <summary>
        /// The keys in the underlying dictionary.
        /// </summary>
        private Dictionary<T, T>.KeyCollection Keys
        {
            get
            {
                return m_set.Keys;
            }
        }

        /// <summary>
        /// A lock for the object.
        /// </summary>
        private readonly Object m_locker = new Object();

        /// <summary>
        /// Constructs an empty set.
        /// </summary>
        public Set()
        {
        }

        /// <summary>
        /// Constructs a set containing the specified elements.
        /// </summary>
        /// <param name="elements"></param>
        public Set(IEnumerable<T> elements)
        {
            foreach (T element in elements)
            {
                m_set.Add(element, element);
            }
        }

        #region ICollection<T> Members

        /// <summary>
        /// Returns the number of elements in the set.
        /// </summary>
        public int Count
        {
            get
            {
                lock (m_locker)
                {
                    return m_set.Count;
                }
            }
        }

        /// <summary>
        /// Adds an element to the set if it is not already there.
        /// </summary>
        /// <param name="item">The item to add.</param>
        new public void Add(T item)
        {
            bool added;
            Add(item, out added);
        }

        /// <summary>
        /// Clears the set.
        /// </summary>
        public void Clear()
        {
            lock (m_locker)
            {
                m_set.Clear();
            }
        }

        /// <summary>
        /// Returns whether or not the set contains the item.
        /// </summary>
        /// <param name="item">The item to check for.</param>
        /// <returns>True if the item is in the set; false if not.</returns>
        public bool Contains(T item)
        {
            lock (m_locker)
            {
                T value;
                return m_set.TryGetValue(item, out value);
            }
        }

        /// <summary>
        /// Copy the contents of the set into the array.
        /// </summary>
        /// <param name="array">The array to copy to.</param>
        /// <param name="arrayIndex">The array index to start copying to.</param>
        public void CopyTo(T[] array, int arrayIndex)
        {
            lock (m_locker)
            {
                Keys.CopyTo(array, arrayIndex);
            }
        }

        /// <summary>
        /// Returns whether the collection is read-only.
        /// </summary>
        public bool IsReadOnly
        {
            get
            {
                return false;
            }
        }

        /// <summary>
        /// Remove the item.
        /// </summary>
        /// <param name="item">The item to remove.</param>
        /// <returns>True if removed; false if not (it wasn't there).</returns>
        public bool Remove(T item)
        {
            lock (m_locker)
            {
                return m_set.Remove(item);
            }
        }

        #endregion

        #region IEnumerable<T> Members

        /// <summary>
        /// Returns an enumerator for the set.
        /// </summary>
        /// <returns>The enumerator.</returns>
        IEnumerator<T> System.Collections.Generic.IEnumerable<T>.GetEnumerator()
        {
            // Copy the keys, and return the enumerator of that.
            IEnumerable<T> keys;
            lock (m_locker)
            {
                keys = new List<T>(Keys);
            }
            return keys.GetEnumerator();
        }

        #endregion

        #region IEnumerable Members

        /// <summary>
        /// Returns an enumerator for the set.
        /// </summary>
        /// <returns>The enumerator.</returns>
        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            // Copy the list, and return the enumerator of that.
            IEnumerable<T> keys;
            lock (m_locker)
            {
                keys = new List<T>(Keys);
            }
            return keys.GetEnumerator();
        }

        #endregion

        #region IList<T> Members

        /// <summary>
        /// Performs an action on each element.
        /// </summary>
        /// <param name="action">The action to perform on each element.</param>
        public void ForEach(Action<T> action)
        {
            foreach (T element in this)
            {
                action(element);
            }
        }

        #endregion

        #region Equality

        /// <summary>
        /// Checks for equivalence with an object.
        /// </summary>
        /// <param name="setObject">The object to compare with.</param>
        /// <returns>True if equivalent; false if not.</returns>
        public override bool Equals(object setObject)
        {
            return Equals(setObject as Set<T>);
        }

        /// <summary>
        /// Checks for equivalence with an set.
        /// </summary>
        /// <param name="set">The set to compare with.</param>
        /// <returns>True if equivalent; false if not.</returns>
        public bool Equals(Set<T> set)
        {
            if (null == set)
            {
                return false;
            }
            lock (m_locker)
            {
                if (m_set.Count != set.m_set.Count)
                {
                    return false;
                }
                foreach (T key in set.Keys)
                {
                    T value;
                    if (!m_set.TryGetValue(key, out value))
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        /// <summary>
        /// Returns a hash code for the set.
        /// </summary>
        /// <returns>The hash code.</returns>
        public override int GetHashCode()
        {
            // TODO: Implement this.
            throw new NotImplementedException();
        }

        #endregion

        /// <summary>
        /// Adds an element to the set if it is not already there.
        /// </summary>
        /// <param name="item">The item to add.</param>
        /// <param name="added">True if added; false if not.</param>
        public void Add(T item, out bool added)
        {
            lock (m_locker)
            {
                if (m_set.ContainsKey(item))
                {
                    added = false;
                    return;
                }
                m_set.Add(item, item);
            }
            added = true;
        }

        /// <summary>
        /// Tries to add an item to the set.
        /// </summary>
        /// <param name="item">The item to add to the set.</param>
        /// <returns>True if it was added.</returns>
        public bool TryAdd(T item)
        {
            bool added;
            Add(item, out added);
            return added;
        }
    }

    namespace Test
    {
        using NUnit.Framework;
        using System.Text;

        [TestFixture]
        public class SetTester
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
                Set<int> set = new Set<int>();
            }

            [Test]
            public void TestAddition()
            {
                Set<int> set = new Set<int>();
                set.Add(1);
                set.Add(3);
                set.Add(7);
                NUnit.Framework.Assert.IsTrue(set.Count == 3);
                NUnit.Framework.Assert.IsTrue(set.Contains(1));
                NUnit.Framework.Assert.IsTrue(set.Contains(3));
                NUnit.Framework.Assert.IsTrue(set.Contains(7));

                // Test [non-]addition of duplicate.
                set.Add(3);
                NUnit.Framework.Assert.IsTrue(set.Count == 3);
            }

            // TODO: Test removal

            // TODO: Test comparison of sets

            [Test]
            public void TestThreadSafeIteration()
            {
                Set<TestClass> set = new Set<TestClass>();

                System.Threading.ThreadPool.QueueUserWorkItem(delegate(Object unused)
                {
                    for (int count = 1; count < 100; ++count)
                    {
                        Trace.WriteLine("Adding {0}.", count);
                        set.Add(new TestClass(count));
                    }
                });

                System.Threading.ThreadPool.QueueUserWorkItem(delegate(Object unused)
                {
                    for (int iteration = 0; iteration < 10; ++iteration)
                    {
                        StringBuilder iterationText = new StringBuilder();
                        iterationText.AppendFormat("Iteration #{0} contains about {1} items. ",
                            iteration, set.Count);
                        int itemNumber = 0;
                        foreach (TestClass item in set)
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
                            iteration, set.Count);
                        int itemNumber = 0;
                        foreach (TestClass item in set)
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

            // TODO: write more unit tests that test the thread safety of the set.
        }
    }
}
