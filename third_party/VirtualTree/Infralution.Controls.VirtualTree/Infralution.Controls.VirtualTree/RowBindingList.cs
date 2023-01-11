 #region File Header
//
//      FILE:   RowBindingList.cs.
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
using System.Drawing.Design;
using System.ComponentModel;
using Infralution.Common;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a list of <see cref="RowBinding">RowBindings</see> associated with a <see cref="VirtualTree"/>.
    /// </summary>
    /// <remarks>Items in the list must be derived from <see cref="RowBinding"/></remarks>
    /// <seealso href="XtraDatasetBinding.html">Data Binding to ADO.NET Datasets and DataViews</seealso>
    /// <seealso href="XtraObjectBinding.html">Data Binding to Object Properties</seealso>
    public class RowBindingList : BindingCollectionBase  
    {

        #region Member Variables
        
        /// <summary>
        /// The tree that this column list is associated with
        /// </summary>
        private VirtualTree _tree;

        #endregion

        #region Public Interface

        /// <summary>
        /// Return the Virtual Tree this collection is associated with.
        /// </summary>
        public VirtualTree Tree
        {
            get { return _tree; }
        }

        /// <summary>
        /// Return the RowBinding at the given index.
        /// </summary>
        public RowBinding this[ int index ]  
        {
            get  
            {
                return((RowBinding) List[index]);
            }
            set  
            {
                List[index] = value;
            }
        }

        /// <summary>
        /// Return the RowBinding with the given name.
        /// </summary>
        public RowBinding this[ string name ]  
        {
            get  
            {
                foreach (RowBinding binding in List)
                {
                    if (binding.Name == name)
                        return binding;
                }
                return null;
            }
        }

        /// <summary>
        /// Add a RowBinding to the list.
        /// </summary>
        /// <param name="value">The RowBinding to add</param>
        /// <returns>The index at which the RowBinding was added</returns>
        public int Add(RowBinding value)  
        {
            return(List.Add(value));
        }

        /// <summary>
        /// Return the index of the given RowBinding in the list
        /// </summary>
        /// <param name="value">The RowBinding to find the index of</param>
        /// <returns>The index of the RowBinding in the list or -1 if not found</returns>
        public int IndexOf(RowBinding value)  
        {
            return(List.IndexOf(value));
        }

        /// <summary>
        /// Set the index of the given binding within the list
        /// </summary>
        /// <param name="binding">The binding to set the index for</param>
        /// <param name="newIndex">The new index for the binding</param>
        public void SetIndexOf(RowBinding binding, int newIndex)
        {
            int index = IndexOf(binding);
            if (index < 0) throw new ArgumentException("The row binding does not belong to this list", "binding");
            if (newIndex < 0 || newIndex > Count - 1) throw new ArgumentOutOfRangeException("newIndex");
            if (index != newIndex)
            {
                SuspendChangeNotification();
                RemoveAt(index);
                List.Insert(newIndex, binding);
                ResumeChangeNotification(false);
                OnListChanged(new ListChangedEventArgs(ListChangedType.ItemMoved, newIndex, index));
            }
        }

        /// <summary>
        /// Insert the given RowBinding into the list at the given index.
        /// </summary>
        /// <param name="index">The index at which to insert the RowBinding</param>
        /// <param name="value">The RowBinding to insert</param>
        public void Insert(int index, RowBinding value)  
        {
            List.Insert(index, value);
        }

        /// <summary>
        /// Remove the given RowBinding from the list if present
        /// </summary>
        /// <param name="value">The RowBinding to remove</param>
        public void Remove(RowBinding value)  
        {
            List.Remove(value);
        }

        /// <summary>
        /// Return true if the list contains the given RowBinding
        /// </summary>
        /// <param name="value">The RowBinding to look for</param>
        /// <returns>True if the list contains the given RowBinding otherwise false</returns>
        public bool Contains(RowBinding value)  
        {
            return(List.Contains(value));
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Initialise a new instance of a row binding list
        /// </summary>
        /// <param name="tree">The tree that this list is associated with</param>
        internal RowBindingList(VirtualTree tree)
        {
            _tree = tree;
        }

        /// <summary>
        /// Validates that the given object can be added to the list
        /// </summary>
        /// <param name="value">The object to be added to the list</param>
        protected override void OnValidate(Object value)  
        {
            if (!(value is RowBinding))
                throw new ArgumentException("value must be of type RowBinding", "value");
        }

        /// <summary>
        /// Sets the <see cref="RowBinding.Tree"/> property of the added <see cref="RowBinding"/>. 
        /// </summary>
        /// <param name="index"></param>
        /// <param name="value"></param>
        protected override void OnInsertComplete(int index, Object value)  
        {
            (value as RowBinding).SetTree(_tree);
            base.OnInsertComplete(index, value);
        }


        #endregion
    }

}


