#region File Header
//
//      FILE:   DataRowCellBinding.cs.
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
using System.Windows.Forms.Design;
using System.Drawing.Design;
using Infralution.Common;
using Infralution.Controls.VirtualTree.Properties;
using NS=Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree
{
    /// <summary>
    /// Defines a class for binding information displayed in a cell of a <see cref="VirtualTree"/> 
    /// (defined by a <see cref="Row"/> and <see cref="NS.Column"/>) to the properties of a <see cref="DataRow"/>.
    /// </summary>
    /// <remarks>
    /// This class is typically defined using the <see cref="VirtualTree"/> visual designer.  The <see cref="Field"/>
    /// property specifies the name of the field within the <see cref="DataRow"/> that contains the value
    /// to be displayed in the cell.  The binding supports modification of the underlying data value when the
    /// user edits the cell.
    /// </remarks>
    /// <seealso href="XtraDatasetBinding.html">Data Binding to ADO.NET Datasets and DataViews</seealso>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="NS.CellBinding"/>
    /// <seealso cref="NS.DataRowRowBinding"/>
    /// <seealso cref="NS.Column"/>
    /// <seealso cref="Row"/>
    /// <seealso cref="CellData"/>
    [TypeConverter("Infralution.Controls.VirtualTree.Design.DataRowCellBindingConverter, " + DesignAssembly.Name)]
    public class DataRowCellBinding : CellBinding
    {

        private string _field;
        private string _lookupField;

        private DataLookupConverter _lookupConverter;
        private DataLookupEditor _lookupEditor;

        /// <summary>
        /// Initialise a default object
        /// </summary>
        public DataRowCellBinding()
        {
        }

        /// <summary>
        /// Initialise an Data Row Cell Binding for the given column and field.
        /// </summary>
        public DataRowCellBinding(Column column, string field)
        {
            Column = column;
            Field = field;
        }

        /// <summary>
        /// Get the row binding this cell binding is associated with
        /// </summary>
        [Browsable(false)]
        public new DataRowRowBinding RowBinding
        {
            get { return (DataRowRowBinding)base.RowBinding; }
        }

        /// <summary>
        /// Set/Get the DataRow Field containing the data to be displayed for this cell
        /// </summary>
        [Category("Data")]
        [Description("The DataRow Field containing the data to be displayed for this cell")]
        [Editor("Infralution.Controls.VirtualTree.Design.DataFieldEditor, " + DesignAssembly.Name, typeof(UITypeEditor))]
        public string Field
        {
            get { return _field; }
            set { _field = StringUtilities.BlankToNull(value); }
        }

        /// <summary>
        /// Set/Get the field in a related table used to generate display values for this cell
        /// </summary>
        /// <remarks>
        /// The DataSet must contain a relation on the <see cref="Field"/>.  This is used to find
        /// the entry in the related table and lookup up the value to display.
        /// </remarks>
        [Category("Data")]
        [Description("The field in a related table used to generate display values for this cell")]
        [Editor("Infralution.Controls.VirtualTree.Design.LookupFieldEditor, " + DesignAssembly.Name, typeof(UITypeEditor))]
        public string LookupField
        {
            get { return _lookupField; }
            set { _lookupField = value; }
        }

        /// <summary>
        /// Get the data to be displayed in the given cell.
        /// </summary>
        /// <param name="row">The row that the cell belongs to</param>
        /// <param name="cellData">The data to be displayed in the cell</param>
        public override void GetCellData(Row row, CellData cellData)
        {
            base.GetCellData(row, cellData);
            DataRow dr = (DataRow)row.Item;
            try
            {
                if (_field != null)
                {
                    cellData.Value = dr[_field];
                    cellData.Error = dr.GetColumnError(_field);
                
                    if (_lookupField != null)
                    {
                        InitialiseLookupEditorAndConverter(dr.Table);
                        cellData.TypeConverter = _lookupConverter;
                        cellData.TypeEditor = _lookupEditor;
                        cellData.Format = null;
                    }
                    else
                    {
                        // if the value is null then use the column data type to set the type converter to use
                        //
                        if (cellData.Value == null || Convert.IsDBNull(cellData.Value))
                        {
                            System.Type dataType = dr.Table.Columns[_field].DataType;
                            cellData.TypeConverter = TypeDescriptor.GetConverter(dataType);
                            cellData.TypeEditor = (UITypeEditor)TypeDescriptor.GetEditor(dataType, typeof(UITypeEditor));
                        }
                    }
                }
             }
            catch (Exception e)
            {
                DisplayErrorMessage(string.Format(Resources.InvalidGetFieldMsg,  Field, RowBinding.Table, e.Message));
                if (!Tree.SuppressBindingExceptions) throw;
            }

         }
  
        /// <summary>
        /// Set a cell value in the given row
        /// </summary>
        /// <param name="row">The row to change the value for</param>
        /// <param name="oldValue">The old value</param>
        /// <param name="newValue">The new value</param>
        /// <returns>True if the change was successful</returns>
        public override bool SetValue(Row row, object oldValue, object newValue)
        {
            try
            {
                DataRow dr = (DataRow)row.Item;
                if (_field != null)
                {
                    dr[_field] = newValue;
                }
                return true;
            }
            catch (Exception e)
            {
                DisplayErrorMessage(string.Format(Resources.SetValueErrorMsg,  Field, RowBinding.Table, e.Message));
                if (!SuppressBindingExceptions) throw;
            }
            return false;
        }

        /// <summary>
        /// Intialise the lookup converter and editor
        /// </summary>
        /// <param name="table">The table for this field</param>
        private void InitialiseLookupEditorAndConverter(DataTable table)
        {
            if (_lookupConverter != null || _field == null) return;

            DataColumn column = table.Columns[_field];
            if (column == null) return;

            foreach (DataRelation relation in table.ParentRelations)
            {
                if (relation.ChildColumns[0].ColumnName == Field)
                {
                    table = relation.ParentTable;
                    DataColumn lookupColumn = table.Columns[LookupField];
                    if (column != null)
                    {
                        _lookupConverter = new DataLookupConverter(lookupColumn, Format);
                        _lookupEditor = new DataLookupEditor(_lookupConverter, column.AllowDBNull);
                    }                  
                    else 
                    {
                        DisplayErrorMessage(string.Format(Resources.InvalidLookupFieldMsg,  table.TableName, LookupField));
                    }
                    return;
                }
            }
            DisplayErrorMessage(string.Format(Resources.NoLookupRelationMsg,  LookupField, RowBinding.Table, Field));
        }

    }
}
