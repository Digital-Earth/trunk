#region File Header
//
//      FILE:   ObjectCellBinding.cs.
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
using System.Reflection;
using System.Globalization;
using System.ComponentModel.Design.Serialization;
using Infralution.Common;
using NS=Infralution.Controls.VirtualTree;
using Infralution.Controls.VirtualTree.Properties;
namespace Infralution.Controls.VirtualTree
{
    /// <summary>
    /// Defines a class for binding information displayed in a cell of a <see cref="VirtualTree"/> 
    /// (defined by a <see cref="Row"/> and <see cref="NS.Column"/>) to the properties of 
    /// an <see cref="Object"/> using <see cref="System.Reflection"/>.
    /// </summary>
    /// <remarks>
    /// This class is typically defined using the <see cref="VirtualTree"/> visual designer.  The <see cref="Field"/>
    /// property specifies the name of a property of the object that provides the value
    /// to be displayed in the cell.  The binding supports modification of the underlying data value when the
    /// user edits the cell.
    /// </remarks>
    /// <seealso href="XtraObjectBinding.html">Data Binding to Object Properties</seealso>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="NS.CellBinding"/>
    /// <seealso cref="NS.ObjectRowBinding"/>
    /// <seealso cref="NS.Column"/>
    /// <seealso cref="Row"/>
    /// <seealso cref="CellData"/>
    [TypeConverter("Infralution.Controls.VirtualTree.Design.ObjectCellBindingConverter, " + DesignAssembly.Name)]
    public class ObjectCellBinding : CellBinding
    {
        private string _field;
        private PropertyDescriptor _fieldPD;
        private Type _fieldType;

        private string _formatField;
        private PropertyDescriptor _formatFieldPD;
        private Type _formatFieldType;

        private string _toolTipField;
        private PropertyDescriptor _toolTipFieldPD;
        private Type _toolTipFieldType;

        /// <summary>
        /// Initialise a default object
        /// </summary>
        public ObjectCellBinding()
        {
        }

        /// <summary>
        /// Initialise an Object Cell Binding for the given column and field.
        /// </summary>
        public ObjectCellBinding(Column column, string field)
         {
            Column = column;
            Field = field;
        }

        /// <summary>
        /// Set/Get the object property to get the value to display in this cell
        /// </summary>
        [Category("Data")]
        [Description("The object property to get the value to display in this cell")]
        [Editor("Infralution.Controls.VirtualTree.Design.ObjectFieldEditor, " + DesignAssembly.Name, typeof(UITypeEditor))]
        [DefaultValue(null)]
        public string Field
        {
            get { return _field; }
            set { _field = StringUtilities.BlankToNull(value); }
        }

        /// <summary>
        /// Set/Get the object property that returns the format string used to display the <see cref="Field"/> value
        /// </summary>
        /// <remarks>
        /// If null then the <see cref="CellBinding.Format"/> property is used.
        /// </remarks>
        [Category("Data")]
        [Description("The object property that returns the format string used to display the value")]
        [Editor("Infralution.Controls.VirtualTree.Design.ObjectFieldEditor, " + DesignAssembly.Name, typeof(UITypeEditor))]
        [DefaultValue(null)]
        public string FormatField
        {
            get { return _formatField; }
            set { _formatField = StringUtilities.BlankToNull(value); }
        }

        /// <summary>
        /// Set/Get the object property that returns the toolTip string to display for this cell (if any)
        /// </summary>
        /// <remarks>
        /// If null then the <see cref="CellBinding.ToolTip"/> property is used.
        /// </remarks>
        [Category("Data")]
        [Description("The object property that returns the format string used to display the value")]
        [Editor("Infralution.Controls.VirtualTree.Design.ObjectFieldEditor, " + DesignAssembly.Name, typeof(UITypeEditor))]
        [DefaultValue(null)]
        public string ToolTipField
        {
            get { return _toolTipField; }
            set { _toolTipField = StringUtilities.BlankToNull(value); }
        }

        /// <summary>
        /// Get the data to be displayed in the given cell.
        /// </summary>
        /// <param name="row">The row that the cell belongs to</param>
        /// <param name="cellData">The data to be displayed in the cell</param>
        public override void GetCellData(Row row, CellData cellData)
        {
            base.GetCellData(row, cellData);

            // if the field is blank then return the item itself
            //
            if (_field == null)
            {
                cellData.Value = row.Item;
            }
            else 
            {
                PropertyDescriptor pd = GetFieldDescriptor(row);
                if (pd != null)
                {
                    // if the descriptor type converter/editor is non null then use it
                    // otherwise don't set and the default converter/editor for the type
                    // will be used
                    //
                    if (pd.Converter != null)
                    {
                        cellData.TypeConverter = pd.Converter;
                    }
                    UITypeEditor typeEditor = pd.GetEditor(typeof(UITypeEditor)) as UITypeEditor;
                    if (typeEditor != null)
                    {
                        cellData.TypeEditor = typeEditor;
                    }
                    cellData.Value = pd.GetValue(row.Item);
                }

                IDataErrorInfo errorInfo = row.Item as IDataErrorInfo;
                if (errorInfo != null)
                {
                    cellData.Error = errorInfo[Field];
                }

            }
            if (_formatField != null)
            {
                PropertyDescriptor pd = GetFormatFieldDescriptor(row);
                if (pd != null)
                {
                    cellData.Format = String.Format(this.Format, pd.GetValue(row.Item));
                }
            }

            if (_toolTipField != null)
            {
                PropertyDescriptor pd = GetToolTipFieldDescriptor(row);
                if (pd != null)
                {
                    cellData.ToolTip = String.Format(this.Format, pd.GetValue(row.Item));
                }
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
                PropertyDescriptor pd = GetFieldDescriptor(row);
                if (pd == null) return false;

                // if we can't assign the value to the property then attempt to convert it
                //
                if (newValue != null && !pd.PropertyType.IsAssignableFrom(newValue.GetType()))
                {
                    newValue = Convert.ChangeType(newValue, pd.PropertyType);
                }
                pd.SetValue(row.Item, newValue);
                return true;
            }
            catch (Exception e)
            {
                ObjectRowBinding rowBinding = RowBinding as ObjectRowBinding;
                DisplayErrorMessage(string.Format(Resources.SetValueErrorMsg,  Field, rowBinding.TypeName, e.Message));
                if (!SuppressBindingExceptions) throw;
            }
            return false;
        }

        /// <summary>
        /// Return the property descriptor for the <see cref="Field"/> of the given row
        /// </summary>
        /// <remarks>
        /// The cell binding caches the property descriptor to improve performance.
        /// </remarks>
        /// <param name="row">The row to get the descriptor for</param>
        /// <returns>The property descriptor</returns>
        protected PropertyDescriptor GetFieldDescriptor(Row row)
        {
            if (_fieldType != row.Item.GetType())
            {
                _fieldPD = ((ObjectRowBinding)RowBinding).GetPropertyDescriptorForRow(row, Field);
                _fieldType = row.Item.GetType();
            }
            return _fieldPD;
        }

        /// <summary>
        /// Return the property descriptor for the <see cref="FormatField"/> of the given row
        /// </summary>
        /// <remarks>
        /// The cell binding caches the property descriptor to improve performance.
        /// </remarks>
        /// <param name="row">The row to get the descriptor for</param>
        /// <returns>The property descriptor</returns>
        protected PropertyDescriptor GetFormatFieldDescriptor(Row row)
        {
            if (_formatFieldType != row.Item.GetType())
            {
                _formatFieldPD = ((ObjectRowBinding)RowBinding).GetPropertyDescriptorForRow(row, FormatField);
                _formatFieldType = row.Item.GetType();
            }
            return _formatFieldPD;
        }

        /// <summary>
        /// Return the property descriptor for the <see cref="ToolTipField"/> of the given row
        /// </summary>
        /// <remarks>
        /// The cell binding caches the property descriptor to improve performance.
        /// </remarks>
        /// <param name="row">The row to get the descriptor for</param>
        /// <returns>The property descriptor</returns>
        protected PropertyDescriptor GetToolTipFieldDescriptor(Row row)
        {
            if (_toolTipFieldType != row.Item.GetType())
            {
                _toolTipFieldPD = ((ObjectRowBinding)RowBinding).GetPropertyDescriptorForRow(row, ToolTipField);
                _toolTipFieldType = row.Item.GetType();
            }
            return _toolTipFieldPD;
        }

    }

}
