 #region File Header
//
//      FILE:   CellBindingList.cs.
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
using NS = Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a list of <see cref="CellBinding">CellBindings</see> associated with a 
    /// <see cref="NS.RowBinding"/> of a <see cref="VirtualTree"/>.
    /// </summary>
    /// <remarks>Items added to the list must be derived from <see cref="CellBinding"/></remarks>
    /// <seealso href="XtraDatasetBinding.html">Data Binding to ADO.NET Datasets and DataViews</seealso>
    /// <seealso href="XtraObjectBinding.html">Data Binding to Object Properties</seealso>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="NS.RowBinding"/>
    /// <seealso cref="NS.CellBinding"/>
    public class CellBindingList : BindingCollectionBase  
    {

        RowBinding _rowBinding;

        /// <summary>
        /// Initialise a new instance of a cell binding list
        /// </summary>
        /// <param name="rowBinding">The row bindings the cell bindings are associated with</param>
        internal CellBindingList(RowBinding rowBinding)
        {
            _rowBinding = rowBinding;
        }

        /// <summary>
        /// Return the row binding that this list is associated with.
        /// </summary>
        public RowBinding RowBinding 
        {
            get { return _rowBinding; }
        }

        /// <summary>
        /// Set/Get the CellBinding at the given index.
        /// </summary>
        public CellBinding this[ int index ]  
        {
            get  
            {
                return((CellBinding) List[index]);
            }
            set  
            {
                List[index] = value;
            }
        }

        /// <summary>
        /// Return the CellBinding for the given column.
        /// </summary>
        public CellBinding this[Column column]  
        {
            get  
            {
                foreach (CellBinding binding in List)
                {
                    if (binding.Column == column)
                        return binding;
                }
                return null;
            }
        }

        /// <summary>
        /// Add a CellBinding to the list.
        /// </summary>
        /// <param name="value">The CellBinding to add</param>
        /// <returns>The index at which the CellBinding was added</returns>
        public int Add(CellBinding value)  
        {
            return List.Add(value);
        }


        /// <summary>
        /// Return the index of the given CellBinding in the list
        /// </summary>
        /// <param name="value">The CellBinding to find the index of</param>
        /// <returns>The index of the CellBinding in the list or -1 if not found</returns>
        public int IndexOf(CellBinding value)  
        {
            return(List.IndexOf(value));
        }

        /// <summary>
        /// Remove the given CellBinding from the list if present
        /// </summary>
        /// <param name="value">The CellBinding to remove</param>
        public void Remove(CellBinding value)  
        {
            List.Remove(value);
        }

        /// <summary>
        /// Return true if the list contains the given CellBinding
        /// </summary>
        /// <param name="value">The CellBinding to look for</param>
        /// <returns>True if the list contains the given CellBinding otherwise false</returns>
        public bool Contains(CellBinding value)  
        {
            return(List.Contains(value));
        }

        /// <summary>
        /// Validates that the object to be added is a <see cref="CellBinding"/>
        /// </summary>
        /// <param name="value">The object to be added to the list</param>
        protected override void OnValidate(Object value)  
        {
            if (!(value is CellBinding))
                throw new ArgumentException("value must be of type CellBinding", "value");
        }

        /// <summary>
        /// Sets the <see cref="CellBinding.RowBinding"/> property of CellBindings
        /// when they are added to the list
        /// </summary>
        /// <param name="index"></param>
        /// <param name="value"></param>
        protected override void OnInsertComplete(int index, Object value)  
        {
            // ensure the column is attached to the tree
            //
            (value as CellBinding).SetRowBinding(_rowBinding);
            base.OnInsertComplete(index, value);
        }

    }

}


