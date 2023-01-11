#region File Header
//
//      FILE:   DataLookupEditor.cs.
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
using System.Windows.Forms.Design;
using System.Windows.Forms;
using System.Data;
using System.ComponentModel;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a <see cref="UITypeEditor"/> for displaying a list of items
    /// from a <see cref="DataTable"/>.
    /// </summary>
    public class DataLookupEditor : Infralution.Controls.ListUITypeEditor  
    {
        /// <summary>
        /// The data column to display value from
        /// </summary>
        private DataColumn _primaryColumn;
        private bool _allowNullValues;
        private DataLookupConverter _converter;

        /// <summary>
        /// Initialise a new DataLookupEditor
        /// </summary>
        /// <param name="converter">The converter used to convert values to strings</param>
        /// <param name="allowNullValues">Should null values be allowed</param>
        public DataLookupEditor(DataLookupConverter converter, bool allowNullValues)
        {
            if (converter == null) throw new ArgumentNullException("converter");
            _converter = converter;
            _primaryColumn = _converter.DisplayColumn.Table.PrimaryKey[0];
            _allowNullValues = allowNullValues;
        }

        /// <summary>
        /// Initialise a new DataLookupEditor
        /// </summary>
        /// <param name="displayColumn">The column used to generate the display strings</param>
        /// <param name="allowNullValues">Should null values be allowed</param>
        public DataLookupEditor(DataColumn displayColumn, bool allowNullValues)
            : this(new DataLookupConverter(displayColumn), allowNullValues)
        {
        }

        /// <summary>
        /// Return the converter used by the editor convert values to strings
        /// </summary>
        public DataLookupConverter Converter
        {
            get { return _converter; }
        }

        /// <summary>
        /// Return the list of items to display
        /// </summary>
        /// <param name="context">The context for the editor</param>
        /// <returns>The items - or null if the context is not supported</returns>
        protected override IList GetItems(ITypeDescriptorContext context)
        {
            DataTable table = _primaryColumn.Table;
            ArrayList items = new ArrayList();
            if (_allowNullValues)
            {
                items.Add(DBNull.Value);
            }
            foreach (DataRowView drv in table.DefaultView)
            {
                items.Add(drv.Row);
            }
            return items;
        }
   
        /// <summary>
        /// Return the string to display for the given item
        /// </summary>
        /// <param name="item">The item to get the display string for</param>
        /// <returns>The display string</returns>
        protected override string GetDisplayString(object item)
        {
            return _converter.GetDisplayString(item as DataRow);
        }

        /// <summary>
        /// Return the value to set for the given item
        /// </summary>
        /// <param name="item">The item to get the value for</param>
        protected override object GetValue(object item)
        {
            DataRow dr = item as DataRow;
            if (dr == null) return DBNull.Value;
            return dr[_primaryColumn.ColumnName];
        }

    }

}


