#region File Header
//
//      FILE:   OrderedSet.cs.
//
//    AUTHOR:   Grant Frisken
//
// COPYRIGHT:   Copyright 2004 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
#endregion
using System;
using System.Collections;
using System.ComponentModel;
namespace Infralution.Common
{
    /// <summary>
    /// Defines a set collection in which the order that objects are added to 
    /// the set is maintained.  
    /// </summary>
    /// <remarks>
    /// Uses a list to provide ordered iteration and a _hashtable to provide an efficient 
    /// implementation of the <see cref="Contains"/> method to support Set style operations.
    /// Supports IBindingList for notification of changes to the set.
    /// </remarks>
    public class OrderedSet : BindingCollectionBase 
    {

        #region Member Variables

        /// <summary>
        /// Hashtable to provide an efficient implementation of Contains method.
        /// </summary>
        private Hashtable _hashtable = new Hashtable();
        
        /// <summary>
        /// A value used as key for null values
        /// </summary>
        private static object _nullKey = new Object();

        #endregion

        #region Public Interface

        /// <summary>
        /// Get the object at the given index.
        /// </summary>
        public object this[ int index ]  
        {
            get  
            {
                return List[index];
            }
        }

        /// <summary>
        /// Add an object to the set if it is not already in the set.
        /// </summary>
        /// <param name="value">The object to add</param>
        public void Add(object value)  
        {
            object key = GetKey(value);
            if (!_hashtable.Contains(key))
            {
                _hashtable.Add(key, List.Count);
                List.Add(value);
            }
        }

        /// <summary>
        /// Add a collection of objects to the set.
        /// </summary>
        /// <param name="items">The collection of objects to add</param>
        public void AddItems(IEnumerable items)  
        {
            SuspendChangeNotification();
            foreach (object item in items)
            {
                Add(item);
            }
            ResumeChangeNotification();
        }

        /// <summary>
        /// Remove a collection of objects from the set.
        /// </summary>
        /// <param name="items">The collection of objects to remove</param>
        public void RemoveItems(IEnumerable items)  
        {
            SuspendChangeNotification();
            foreach (object item in items)
            {
                Remove(item);
            }
            ResumeChangeNotification();
        }

        /// <summary>
        /// Clears the current set and adds the given collection of objects.
        /// </summary>
        /// <param name="items">The collection of objects defining the new set</param>
        public void SetItems(IEnumerable items)  
        {
            SuspendChangeNotification();
            Clear();
            foreach (object item in items)
            {
                Add(item);
            }
            ResumeChangeNotification();
        }

        /// <summary>
        /// Clears the current set and adds the given object.
        /// </summary>
        /// <param name="item">The object to add to the set</param>
        public void Set(object item)  
        {
            SuspendChangeNotification();
            Clear();
            Add(item);
            ResumeChangeNotification();
        }

        /// <summary>
        /// Return the index of the given object in the set
        /// </summary>
        /// <param name="value">The object to find the index of</param>
        /// <returns>The index of the object in the set or -1 if not found</returns>
        public int IndexOf(object value)  
        {
            object key = GetKey(value);

            // get the index from the hashtable
            //
            object index = _hashtable[key];
            if (index != null)
            {
                // check it hasn't changed since the item was added
                //
                int i = (int)index;
                if (!value.Equals(List[i]))
                {
                    // have to do a linear search for it
                    //
                    i = List.IndexOf(value);

                    // update the hashtable index so next time we find it quicker
                    //
                    _hashtable[key] = i;
                }
                return i;
            }
            return -1;
        }

        /// <summary>
        /// Remove the given object from the set if present
        /// </summary>
        /// <param name="value">The Row to remove</param>
        public void Remove(object value)  
        {
            object key = GetKey(value);
            if (_hashtable.Contains(key))
            {
                _hashtable.Remove(key);
                List.Remove(value);
            }
        }

        /// <summary>
        /// Return true if the set contains the given object
        /// </summary>
        /// <param name="value">The object to look for</param>
        /// <returns>True if the set contains the given object otherwise false</returns>
        public bool Contains(object value)  
        {
            return(_hashtable.Contains(GetKey(value)));
        }

        /// <summary>
        /// Return a new array of object contained in the set
        /// </summary>
        /// <returns>A new object array containing the objects in the set</returns>
        public object[] GetItems()
        {
            object[] items = new object[List.Count];
            List.CopyTo(items, 0);
            return items;
        }

        #endregion

        #region Local Methods


        /// <summary>
        /// Return the object used to represent nulls in the hashtable
        /// </summary>
        protected object NullKey
        {
            get { return _nullKey; }
        }

        /// <summary>
        /// Return the key to use for hashing a given item
        /// </summary>
        /// <remarks>
        /// This method allows the collection to handle null values.   For null values
        /// this returns the NullKey object for all other values it just returns the
        /// value itself
        /// </remarks>
        /// <param name="item">The item to get the key for</param>
        /// <returns>A key to use to hash the item</returns>
        protected virtual object GetKey(object item)
        {
            return (item == null) ? NullKey : item;
        }

        /// <summary>
        /// Clear the associated _hashtable.
        /// </summary>
        protected override void OnClearComplete()
        {
            _hashtable.Clear();
            base.OnClearComplete();
        }

        #endregion
    }
}

