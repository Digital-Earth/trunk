#region File Header
//
//      FILE:   DataViewList.cs.
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
using System.Data;
using System.Collections;
using System.ComponentModel;
using System.Diagnostics;
namespace Infralution.Controls.VirtualTree
{
    /// <summary>
    /// Defines a wrapper class for <see cref="DataView"/> objects that supports the 
    /// <see cref="IBindingList"/> that returns <see cref="DataRow"/> objects rather than
    /// <see cref="DataRowView"/> objects.
    /// </summary>
    /// <remarks>
    /// The <see cref="DataView"/> class supports the <see cref="IBindingList"/> interface - 
    /// however it does not provide any mechanism of finding the index of a <see cref="DataRowView"/> 
    /// within the <see cref="DataView"/> based on the underlying <see cref="DataRow"/> object.   
    /// This class provides that ability.  
    /// </remarks>
    public class DataViewList : IBindingList
    {
        #region Local Types and Variables

        /// <summary>
        /// Enumerator for DataViewList
        /// </summary>
        protected class Enumerator : IEnumerator
        {
            #region IEnumerator Members

            private DataViewList _list;
            private int _index = -1;

            /// <summary>
            /// Create a new instance of the enumerator
            /// </summary>
            /// <param name="list"></param>
            public Enumerator(DataViewList list)
            {
                _list = list;
            }

            /// <summary>
            /// Reset the enumerator
            /// </summary>
            public void Reset()
            {
                _index = -1;
            }

            /// <summary>
            /// Return the item at the current position
            /// </summary>
            public object Current
            {
                get
                {
                    return _list[_index];
                }
            }

            /// <summary>
            /// Move the enumerator to the next item
            /// </summary>
            /// <returns>True if the move was successful</returns>
            public bool MoveNext()
            {
                _index++;
                return (_index < _list.Count);
            }

            #endregion

        }

        /// <summary>
        /// The wrapped DataView
        /// </summary>
        private DataView _view;

        #endregion

        #region Public Interface

        /// <summary>
        /// Create a new wrapper for the given DataView
        /// </summary>
        /// <param name="view">The data view to wrap</param>
        public DataViewList(DataView view)
        {
            _view = view;
        }

        /// <summary>
        /// Return the wrapped data view
        /// </summary>
        public DataView DataView
        {
            get { return _view; }
        }

        #endregion

        #region IBindingList Members

        /// <summary>
        /// Exposes the underlying <see cref="IBindingList.ListChanged">DataView.ListChanged</see> event
        /// </summary>
        public event System.ComponentModel.ListChangedEventHandler ListChanged
        {
            add { _view.ListChanged += value; }
            remove { _view.ListChanged -= value; }
        }

        /// <summary>
        /// Returns true.
        /// </summary>
        bool IList.IsReadOnly
        {
            get { return true; }
        }

        /// <summary>
        /// Return the underlying DataRow at the given index of the DataView.
        /// </summary>
        public object this[int index]
        {
            get 
            {
                return _view[index].Row; 
            }
            set 
            {
                throw new NotSupportedException();
            } 
        }

        /// <summary>
        /// Not supported.
        /// </summary>
        /// <param name="index"></param>
        void IList.RemoveAt(int index)
        {
            throw new NotSupportedException();
        }

        /// <summary>
        /// Callthrough to underlying DataView.
        /// </summary>
        /// <param name="index"></param>
        /// <param name="value"></param>
        void IList.Insert(int index, object value)
        {
            throw new NotSupportedException();
        }

        /// <summary>
        /// Not supported.
        /// </summary>
        /// <param name="value"></param>
        void IList.Remove(object value)
        {
            throw new NotSupportedException();
        }

        /// <summary>
        /// Not supported.
        /// </summary>
        void IList.Clear()
        {
            throw new NotSupportedException();
        }

        /// <summary>
        /// Returns true if the <see cref="DataView"/> contains the given <see cref="DataRow"/>
        /// </summary>
        /// <param name="value">The <see cref="DataRow"/> to find the index of</param>
        /// <returns>True if the <see cref="DataView"/> contains the given <see cref="DataRow"/></returns>
        public bool Contains(object value)
        {
            return (IndexOf(value) >= 0);
        }

        /// <summary>
        /// Finds the index in the <see cref="DataView"/> of the <see cref="DataRowView"/> that references the 
        /// given <see cref="DataRow"/> .  
        /// </summary>
        /// <param name="value">The <see cref="DataRow"/> to find the index of</param>
        /// <returns>The index of the <see cref="DataRowView"/> that references the given <see cref="DataRow"/> 
        /// or -1 if the underlying <see cref="DataRow"/> could not be found</returns>
        public int IndexOf(object value)
        {
            DataRow row = value as DataRow;
            if (row == null) return -1;

            int count = _view.Count;
            for (int i = 0; i < count; i++)
            {
                DataRowView rowView = _view[i];                
                if (row.Equals(rowView.Row))
                {
                    return i;
                }
            }
            return -1;
        }

        /// <summary>
        /// Not supported.
        /// </summary>
        /// <param name="value"></param>
        /// <returns></returns>
        int IList.Add(object value)
        {
            throw new NotSupportedException();
        }

        /// <summary>
        /// Returns false.
        /// </summary>
        bool IList.IsFixedSize
        {
            get  { return false; }
        }

        #endregion

        #region Binding List Interface

        /// <summary>
        /// Not supported.
        /// </summary>
        /// <param name="property"></param>
        void IBindingList.AddIndex(PropertyDescriptor property)
        {
            throw new NotSupportedException();
        }

        /// <summary>
        /// Returns false.
        /// </summary>
        bool IBindingList.AllowNew
        {
            get { return false; }
        }

        /// <summary>
        /// Not supported.
        /// </summary>
        /// <param name="property"></param>
        /// <param name="direction"></param>
        void IBindingList.ApplySort(PropertyDescriptor property, System.ComponentModel.ListSortDirection direction)
        {
            throw new NotSupportedException();
        }

        /// <summary>
        /// Returns null.
        /// </summary>
        PropertyDescriptor IBindingList.SortProperty
        {
            get { return null; }
        }

        /// <summary>
        /// Not supported.
        /// </summary>
        /// <param name="property"></param>
        /// <param name="key"></param>
        /// <returns></returns>
        int IBindingList.Find(PropertyDescriptor property, object key)
        {
            throw new NotSupportedException();
        }

        /// <summary>
        /// Returns false.
        /// </summary>
        bool IBindingList.SupportsSorting
        {
            get { return false; }
        }

        /// <summary>
        /// Returns false.
        /// </summary>
        bool IBindingList.IsSorted
        {
            get { return false; }
        }

        /// <summary>
        /// Returns false.
        /// </summary>
        bool IBindingList.AllowRemove
        {
            get { return false; }
        }

        /// <summary>
        /// Returns false.
        /// </summary>
        bool IBindingList.SupportsSearching
        {
            get { return false; }
        }

        /// <summary>
        /// Not supported.
        /// </summary>
        ListSortDirection IBindingList.SortDirection
        {
            get 
            {
                throw new NotSupportedException();
            }
        }

        /// <summary>
        /// Returns true.
        /// </summary>
        bool IBindingList.SupportsChangeNotification
        {
            get { return true; }
        }

        /// <summary>
        /// Not supported.
        /// </summary>
        void IBindingList.RemoveSort()
        {
            throw new NotSupportedException();
        }

        /// <summary>
        /// Not supported.
        /// </summary>
        object IBindingList.AddNew()
        {
            throw new NotSupportedException();
        }

        /// <summary>
        /// Returns false.
        /// </summary>
        bool IBindingList.AllowEdit
        {
            get { return false; }
        }

        /// <summary>
        /// Not supported.
        /// </summary>
        /// <param name="property"></param>
        void IBindingList.RemoveIndex(PropertyDescriptor property)
        {
            throw new NotSupportedException();
        }

        #endregion

        #region ICollection Members

        /// <summary>
        /// Returns false.
        /// </summary>
        bool ICollection.IsSynchronized
        {
            get  { return false; }
        }

        /// <summary>
        /// Return the number of rows in the underyling <see cref="DataView"/>.
        /// </summary>
        public int Count
        {
            get  { return _view.Count; }
        }

        /// <summary>
        /// Not supported.
        /// </summary>
        /// <param name="array"/>
        /// <param name="index"/>
        void ICollection.CopyTo(Array array, int index)
        {
            throw new NotSupportedException();
        }

        /// <summary>
        /// Returns null.
        /// </summary>
        object ICollection.SyncRoot
        {
            get  { return null; }
        }

        #endregion

        #region IEnumerable Members

        /// <summary>
        /// Return an enumerator for the list.
        /// </summary>
        IEnumerator IEnumerable.GetEnumerator()
        {
            return new Enumerator(this);
        }

        #endregion

    }


}
