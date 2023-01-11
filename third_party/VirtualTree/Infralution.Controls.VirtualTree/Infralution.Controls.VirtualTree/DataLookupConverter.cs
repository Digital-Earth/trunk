#region File Header
//
//      FILE:   DataLookupConverter.cs.
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
using System.ComponentModel;
using System.Data;
using System.Globalization;
namespace Infralution.Controls.VirtualTree
{
	/// <summary>
	/// Defines a TypeConverter for converting an ID to a displayable string using a lookup table.
	/// </summary>
	/// <remarks>
	/// Converts value to a DataRow using the primary key of the table then uses the given 
	/// Column to convert the value to a string.
	/// </remarks>
	public class DataLookupConverter : TypeConverter
	{

        private DataColumn _displayColumn;
        private string     _format; 


        /// <summary>
        /// Initialise a new DataLookupConverter
        /// </summary>
        /// <param name="displayColumn">The column used to generate the display string</param>
        /// <param name="format">The <see cref="String.Format(String, object)"/> string used to display the value</param>
		public DataLookupConverter(DataColumn displayColumn, string format)
		{
            _displayColumn = displayColumn;      
            _format = format;
		}

        /// <summary>
        /// Initialise a new DataLookupConverter
        /// </summary>
        /// <param name="displayColumn">The column used to generate the display string</param>
        public DataLookupConverter(DataColumn displayColumn)
        {
            _displayColumn = displayColumn;      
            _format = "{0}";
        }

        /// <summary>
        /// Set/Get the column the converter should get display data from
        /// </summary>
        public DataColumn DisplayColumn
        {
            get { return _displayColumn; }
            set { _displayColumn = value; }
        }

        /// <summary>
        /// The <see cref="String.Format(String, object)"/> string used to display the value
        /// </summary>
        public string Format
        {
            get { return _format; }
            set { _format = value; }
        }


        /// <summary>
        /// Allows conversion to a string
        /// </summary>
        /// <param name="context"></param>
        /// <param name="destType"></param>
        /// <returns>True</returns>
        public override bool CanConvertTo(ITypeDescriptorContext context, Type destType) 
        {
            return true;
        }	
		    
        /// <summary>
        /// Converts a value to a string using the lookup table
        /// </summary>
        /// <param name="context"></param>
        /// <param name="info"></param>
        /// <param name="value"></param>
        /// <param name="destType"></param>
        /// <returns></returns>
        public override object ConvertTo(ITypeDescriptorContext context, CultureInfo info, object value, Type destType)
        {
            if (destType==typeof(string))
            {
                DataTable table = _displayColumn.Table;
                DataRow row = table.Rows.Find(value);
                return GetDisplayString(row);
            }
            return base.ConvertTo(context,info,value,destType);
        }

        /// <summary>
        /// Return the string to display for the given row of the <see cref="DisplayColumn"/> table
        /// </summary>
        /// <param name="dataRow">The dataRow to display</param>
        /// <returns>The string to use</returns>
        public virtual string GetDisplayString(DataRow dataRow)
        {
            if (dataRow != null)
            {
                object displayValue = dataRow[_displayColumn];
                return String.Format(_format, displayValue);
            }
            return "";
        }

    }
}
