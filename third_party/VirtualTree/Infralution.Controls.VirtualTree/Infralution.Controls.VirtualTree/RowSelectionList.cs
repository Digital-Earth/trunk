#region File Header
//
//      FILE:   RowSelectionList.cs.
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
using Infralution.Common;
namespace Infralution.Controls.VirtualTree
{
	/// <summary>
	/// Defines the list of currently selected <see cref="Row">Rows</see> for a <see cref="VirtualTree"/>.  
	/// </summary>
	/// <remarks>
    /// The list order is determined by the order in which the user selects <see cref="Row">Rows</see> 
    /// (ie the first row selected will be the first in the list).  Uses an internal hashtable to provide an
    /// efficient implementation of the <see cref="Contains"/> method and an internal list to maintain the 
    /// selection order.
	/// </remarks>
    public class RowSelectionList : BindingCollectionBase 
    {

        #region Member Variables

        // selected rows indexed by row to provide an efficient implementation of Contains even
        // with large selection sets.
        //
        private Hashtable hashtable = new Hashtable();
        
        #endregion

        #region Public Interface

        /// <summary>
        /// Get the Row at the given index.
        /// </summary>
        public Row this[ int index ]  
        {
            get  
            {
                return((Row) List[index]);
            }
        }

        /// <summary>
        /// Add a Row to the list if it is not already in the list.
        /// </summary>
        /// <param name="value">The Row to add</param>
        public void Add(Row value)  
        {
            if (value == null) throw new ArgumentNullException("value");
            if (!hashtable.Contains(value))
            {
                hashtable.Add(value, null);
                List.Add(value);
            }
        }

        /// <summary>
        /// Add a collection of rows to the list.
        /// </summary>
        /// <param name="rows">The collection of rows to add</param>
        public void Add(IEnumerable rows)  
        {
            SuspendChangeNotification();
            foreach (Row row in rows)
            {
                Add(row);
            }
            ResumeChangeNotification();
        }

        /// <summary>
        /// Remove a collection of rows from the list.
        /// </summary>
        /// <param name="rows">The collection of rows to remove</param>
        public void Remove(IEnumerable rows)  
        {
            SuspendChangeNotification();
            foreach (Row row in rows)
            {
                Remove(row);
            }
            ResumeChangeNotification();
        }

        /// <summary>
        /// Clears the current selection and adds the given collection of rows.
        /// </summary>
        /// <param name="rows">The collection of rows to to set as selected</param>
        public void Set(IEnumerable rows)  
        {
            SuspendChangeNotification();
            Clear();
            foreach (Row row in rows)
            {
                Add(row);
            }
            ResumeChangeNotification();
        }

        /// <summary>
        /// Clears the current selection and adds the given row.
        /// </summary>
        /// <param name="row">The row to select</param>
        public void Set(Row row)  
        {
            // only perform notification if the selection
            // will change
            //
            if (this.Count != 1 || row != List[0])
            {
                SuspendChangeNotification();
                Clear();
                if (row != null)
                    Add(row);
                ResumeChangeNotification();
            }
        }

        /// <summary>
        /// Return the index of the given Row in the list
        /// </summary>
        /// <param name="value">The Row to find the index of</param>
        /// <returns>The index of the Row in the list or -1 if not found</returns>
        public int IndexOf(Row value)  
        {
            if (value == null) throw new ArgumentNullException("value");
            return(List.IndexOf(value));
        }

        /// <summary>
        /// Remove the given Row from the list if present
        /// </summary>
        /// <param name="value">The Row to remove</param>
        public void Remove(Row value)  
        {
            if (value == null) throw new ArgumentNullException("value");
            if (hashtable.Contains(value))
            {
                hashtable.Remove(value);
                List.Remove(value);
            }
        }

        /// <summary>
        /// Return true if the list contains the given Row
        /// </summary>
        /// <param name="value">The Row to look for</param>
        /// <returns>True if the list contains the given Row otherwise false</returns>
        public bool Contains(Row value)  
        {
            if (value == null) throw new ArgumentNullException("value");
            return(hashtable.Contains(value));
        }

        /// <summary>
        /// Return a new array of rows contained in the selection list
        /// </summary>
        /// <returns>A new Row array containing the rows in the selection list</returns>
        public Row[] GetRows()
        {
            Row[] rows = new Row[List.Count];
            List.CopyTo(rows, 0);
            return rows;
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Clear the associated hashtable.
        /// </summary>
        protected override void OnClearComplete()
        {
            hashtable.Clear();
            base.OnClearComplete();
        }

        /// <summary>
        /// Validates that the given object can be added to the list
        /// </summary>
        /// <param name="value">The object to be added to the list</param>
        protected override void OnValidate(Object value)  
        {
            if (!(value is Row))
                throw new ArgumentException("value must be of type Row", "value");
        }

        #endregion
    }
}

