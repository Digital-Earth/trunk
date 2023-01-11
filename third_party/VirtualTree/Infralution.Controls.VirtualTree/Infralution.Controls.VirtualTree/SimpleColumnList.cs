#region File Header
//
//      FILE:   SimpleColumnList.cs.
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
using System.Xml;
using System.Xml.Serialization;
using Infralution.Common;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a simple, strongly typed, non-binding list of <see cref="Column">Columns</see>.
    /// </summary>
    /// <remarks>
    /// This list is used where simple collections of columns are required.  Unlike <see cref="ColumnList"/>
    /// this list does not support IBindingList nor does it "own" the contained columns. 
    /// </remarks>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="Column"/>
    /// <seealso cref="ColumnList"/>
    public class SimpleColumnList : CollectionBase  
    {

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of a simple column list
        /// </summary>
        public SimpleColumnList()
        {
        }

        /// <summary>
        /// Return the Column at the given index.
        /// </summary>
        public Column this[ int index ]  
        {
            get  
            {
                return((Column) List[index]);
            }
            set  
            {
                List[index] = value;
            }
        }

        /// <summary>
        /// Return the Column with the given name.
        /// </summary>
        public Column this[ string name ]  
        {
            get  
            {
                foreach (Column column in List)
                {
                    if (column.Name == name)
                        return column;
                }
                return null;
            }
        }

        /// <summary>
        /// Add an existing Column to the list.
        /// </summary>
        /// <param name="value">The Column to add</param>
        /// <returns>The index at which the Column was added</returns>
        public int Add(Column value)  
        {
            return(List.Add(value));
        }

        /// <summary>
        /// Insert a column into the list at the given index.
        /// </summary>
        /// <param name="index">The index at which to insert the Column</param>
        /// <param name="column">The column to insert</param>
        public void Insert(int index, Column column)  
        {
            List.Insert(index, column);
        }

        /// <summary>
        /// Return the index of the given Column in the list
        /// </summary>
        /// <param name="value">The Column to find the index of</param>
        /// <returns>The index of the Column in the list or -1 if not found</returns>
        public int IndexOf(Column value)  
        {
            return(List.IndexOf(value));
        }

        /// <summary>
        /// Remove the given Column from the list if present
        /// </summary>
        /// <param name="value">The Column to remove</param>
        public void Remove(Column value)  
        {
            List.Remove(value);
        }

        /// <summary>
        /// Return true if the list contains the given Column
        /// </summary>
        /// <param name="value">The Column to look for</param>
        /// <returns>True if the list contains the given Column otherwise false</returns>
        public bool Contains(Column value)  
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
            if (!(value is Column))
                throw new ArgumentException("value must be of type Column", "value");
        }


        #endregion

    }

}


