//
//      FILE:   BindingCollectionBase.cs.
//
//    AUTHOR:   Grant Frisken
//
// COPYRIGHT:   Copyright 2004 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
using System;
using System.Collections;
using System.ComponentModel;
namespace Infralution.Common
{
	/// <summary>
	/// Provides a base collection class that provides a minimal implementation of 
	/// the IBindingList interface.   This allows applications to easily define
	/// simple lists that support change notification for use as data sources.
	/// </summary>
    public class BindingCollectionBase : CollectionBase, IBindingList
    {

        #region Member Variables

        /// <summary>
        /// Flag allowing change notification to be suppressed
        /// </summary>
        private int _suspendChangeNotificationCount = 0;

        #endregion

        #region Public Interface

        /// <summary>
        /// Raised when the list is changed (items added, removed or changed).
        /// </summary>
        [Description("Raised when the list is changed (items added, removed or changed).")]
        public event ListChangedEventHandler ListChanged;
 
        /// <summary>
        /// Allows <see cref="ListChanged"/> events to be suppressed when performing a series of
        /// updates to the list
        /// </summary>
        /// <seealso cref="SuspendChangeNotification"/>
        public void SuspendChangeNotification()
        {
            _suspendChangeNotificationCount++;
        }

        /// <summary>
        /// Resumes notifying clients of changes using <see cref="ListChanged"/> events. 
        /// </summary>
        /// <param name="notifyNow">If true causes a <see cref="ListChanged"/> (reset) event to be fired</param>
        /// <seealso cref="SuspendChangeNotification"/>
        public void ResumeChangeNotification(bool notifyNow)
        {
            if (_suspendChangeNotificationCount > 0)
            {
                _suspendChangeNotificationCount--;
                if (notifyNow)
                {
                    OnListChanged(new ListChangedEventArgs(ListChangedType.Reset, 0));
                }
            }
        }

        /// <summary>
        /// Resumes notifying clients of changes using <see cref="ListChanged"/> event and fires
        /// a <see cref="ListChanged"/> (reset) event.
        /// </summary>
        /// <seealso cref="SuspendChangeNotification"/>
        public void ResumeChangeNotification()
        {
            ResumeChangeNotification(true);
        }

        #endregion

        #region IBindingList Members

        /// <summary>
        /// Not supported - returns false
        /// </summary>
        bool IBindingList.AllowNew
        {
            get { return false; }
        }

        /// <summary>
        /// Throws NotSupportedException
        /// </summary>
        /// <param name="property"></param>
        /// <param name="direction"></param>
        void IBindingList.ApplySort(PropertyDescriptor property, 
                                    ListSortDirection direction)
        {
            throw new NotSupportedException();
        }

        /// <summary>
        /// Throws NotSupportedException
        /// </summary>
        PropertyDescriptor IBindingList.SortProperty
        {
            get 
            {
                throw new NotSupportedException();
            }
        }

        /// <summary>
        /// Throws NotSupportedException
        /// </summary>
        /// <param name="property"></param>
        /// <param name="key"></param>
        /// <returns></returns>
        int IBindingList.Find(PropertyDescriptor property, object key)
        {
            throw new NotSupportedException();
        }

        /// <summary>
        /// Always returns false
        /// </summary>
        bool IBindingList.SupportsSorting
        {
            get { return false; }
        }

        /// <summary>
        /// Always returns false
        /// </summary>
        bool IBindingList.IsSorted
        {
            get { return false; }
        }

        /// <summary>
        /// Remove is allowed
        /// </summary>
        bool IBindingList.AllowRemove
        {
            get { return true; }
        }

        /// <summary>
        /// Always returns false
        /// </summary>
        bool IBindingList.SupportsSearching
        {
            get { return false; }
        }

        /// <summary>
        /// Throws NotSupportedException
        /// </summary>
        ListSortDirection IBindingList.SortDirection
        {
            get 
            { 
                throw new NotSupportedException();
            }
        }

        /// <summary>
        /// Returns true
        /// </summary>
        bool IBindingList.SupportsChangeNotification
        {
            get { return true; }
        }

        /// <summary>
        /// Throws NotSupportedException
        /// </summary>
        void IBindingList.RemoveSort()
        {
            throw new NotSupportedException();
        }

        /// <summary>
        /// Throws NotSupportedException
        /// </summary>
        /// <returns></returns>
        object IBindingList.AddNew()
        {
            throw new NotSupportedException();
        }

        /// <summary>
        /// Returns true
        /// </summary>
        bool IBindingList.AllowEdit
        {
            get { return true; }
        }

        /// <summary>
        /// Not Supported (noop)
        /// </summary>
        /// <param name="property"></param>
        void IBindingList.AddIndex(PropertyDescriptor property)
        {
        }
        
        /// <summary>
        /// Not Supported (noop)
        /// </summary>
        /// <param name="property"></param>
        void IBindingList.RemoveIndex(PropertyDescriptor property)
        {
        }

        #endregion

        #region Local Methods and Overrides

        /// <summary>
        /// Allows null values by default
        /// </summary>
        /// <remarks>
        /// Typically overriden in derived classes to perform type checking if required
        /// </remarks>
        /// <param name="value"></param>
        protected override void OnValidate(object value)
        {
        }

        /// <summary>
        /// Notify clients of the change via the <see cref="ListChanged"/> event.
        /// </summary>
        protected override void OnClearComplete()
        {
            OnListChanged(new ListChangedEventArgs(ListChangedType.Reset, 0));
        }

        /// <summary>
        /// Notify clients of the change via the <see cref="ListChanged"/> event.
        /// </summary>
        protected override void OnInsertComplete(int index, object value)
        {
            OnListChanged(new ListChangedEventArgs(ListChangedType.ItemAdded, index));
        }

        /// <summary>
        /// Notify clients of the change via the <see cref="ListChanged"/> event.
        /// </summary>
        protected override void OnRemoveComplete(int index, object value)
        {
            OnListChanged(new ListChangedEventArgs(ListChangedType.ItemDeleted, index));
        }

        /// <summary>
        /// Notify clients of the change via the <see cref="ListChanged"/> event.
        /// </summary>
        protected override void OnSetComplete(int index, object oldValue, object newValue)
        {
            OnListChanged(new ListChangedEventArgs(ListChangedType.ItemChanged, index));
        }

        /// <summary>
        /// Notify clients of a change to the list by raising the <see cref="ListChanged"/> event.
        /// </summary>
        /// <param name="e"></param>
        protected virtual void OnListChanged(ListChangedEventArgs e)
        {
            if (_suspendChangeNotificationCount <= 0)
            {
                if (ListChanged != null)
                {
                    ListChanged(this, e);
                }
            }
        }

        #endregion 

     }

}
