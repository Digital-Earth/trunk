 #region File Header
//
//      FILE:   CellEditorList.cs.
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
using System.Windows.Forms;
using Infralution.Common;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a list of <see cref="CellEditor">CellEditors</see> for a <see cref="VirtualTree"/>
    /// </summary>
    /// <remarks>Items in the list must be derived from <see cref="CellEditor"/></remarks>
    /// <seealso href="XtraEditors.html">Defining and using Editors</seealso>
    /// <seealso cref="VirtualTree.Editors">VirtualTree.Editors</seealso>
    /// <seealso cref="CellEditor"/>
    public class CellEditorList : BindingCollectionBase  
    {

        #region Public Interface

        /// <summary>
        /// Return the CellEditor at the given index.
        /// </summary>
        public CellEditor this[ int index ]  
        {
            get  
            {
                return((CellEditor) List[index]);
            }
            set  
            {
                List[index] = value;
            }
        }

        /// <summary>
        /// Add a CellEditor to the list.
        /// </summary>
        /// <param name="value">The CellEditor to add</param>
        /// <returns>The index at which the CellEditor was added</returns>
        public int Add(CellEditor value)  
        {
            return(List.Add(value));
        }

        /// <summary>
        /// Return the index of the given CellEditor in the list
        /// </summary>
        /// <param name="value">The CellEditor to find the index of</param>
        /// <returns>The index of the CellEditor in the list or -1 if not found</returns>
        public int IndexOf(CellEditor value)  
        {
            return(List.IndexOf(value));
        }

        /// <summary>
        /// Insert the given CellEditor into the list at the given index.
        /// </summary>
        /// <param name="index">The index at which to insert the CellEditor</param>
        /// <param name="value">The CellEditor to insert</param>
        public void Insert(int index, CellEditor value)  
        {
            List.Insert(index, value);
        }

        /// <summary>
        /// Remove the given CellEditor from the list if present
        /// </summary>
        /// <param name="value">The CellEditor to remove</param>
        public void Remove(CellEditor value)  
        {
            List.Remove(value);
        }

        /// <summary>
        /// Return true if the list contains the given CellEditor
        /// </summary>
        /// <param name="value">The CellEditor to look for</param>
        /// <returns>True if the list contains the given CellEditor otherwise false</returns>
        public bool Contains(CellEditor value)  
        {
            return(List.Contains(value));
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Validates that the given object can be added to the list
        /// </summary>
        /// <param name="value">The object to be added to the list</param>
        protected override void OnValidate(Object value)  
        {
            if (!(value is CellEditor))
                throw new ArgumentException("value must be of type CellEditor", "value");
        }


        #endregion
    }

}


