#region File Header
//
//      FILE:   BindingList.cs.
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
    /// Defines a simple array list which supports <see cref="IBindingList"/> notification 
    /// </summary>
    public class BindingList : BindingCollectionBase 
    {
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
        /// Add an object to the list.
        /// </summary>
        /// <param name="value">The object to add</param>
        public virtual void Add(object value)  
        {
            List.Add(value);
        }

        /// <summary>
        /// Add a collection of objects to the list.
        /// </summary>
        /// <param name="items">The collection of objects to add</param>
        public virtual void AddItems(IEnumerable items)  
        {
            SuspendChangeNotification();
            foreach (object item in items)
            {
                Add(item);
            }
            ResumeChangeNotification();
        }

        /// <summary>
        /// Remove a collection of objects from the list.
        /// </summary>
        /// <param name="items">The collection of objects to remove</param>
        public virtual void RemoveItems(IEnumerable items)  
        {
            SuspendChangeNotification();
            foreach (object item in items)
            {
                Remove(item);
            }
            ResumeChangeNotification();
        }

        /// <summary>
        /// Clears the current list and adds the given collection of objects.
        /// </summary>
        /// <param name="items">The collection of objects to add</param>
        public virtual void SetItems(IEnumerable items)  
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
        /// Clears the current list and adds the given object.
        /// </summary>
        /// <param name="item">The object to add</param>
        public virtual void Set(object item)  
        {
            SuspendChangeNotification();
            Clear();
            if (item != null)
                Add(item);
            ResumeChangeNotification();
        }

        /// <summary>
        /// Return the index of the given object in the list
        /// </summary>
        /// <param name="value">The object to find the index of</param>
        /// <returns>The index of the object in the list or -1 if not found</returns>
        public virtual int IndexOf(object value)  
        {
            if (value == null) throw new ArgumentNullException("value");
            return(List.IndexOf(value));
        }

        /// <summary>
        /// Remove the given object from the list if present
        /// </summary>
        /// <param name="value">The Row to remove</param>
        public virtual void Remove(object value)  
        {
            List.Remove(value);
        }

        /// <summary>
        /// Return true if the lsit contains the given object
        /// </summary>
        /// <param name="value">The object to look for</param>
        /// <returns>True if the set contains the given object otherwise false</returns>
        public virtual bool Contains(object value)  
        {
            return List.Contains(value);
        }

        /// <summary>
        /// Return a new array of objects contained in the list
        /// </summary>
        /// <returns>A new object array containing the objects in the list</returns>
        public virtual object[] GetItems()
        {
            object[] items = new object[List.Count];
            List.CopyTo(items, 0);
            return items;
        }

        #endregion

     }
}

