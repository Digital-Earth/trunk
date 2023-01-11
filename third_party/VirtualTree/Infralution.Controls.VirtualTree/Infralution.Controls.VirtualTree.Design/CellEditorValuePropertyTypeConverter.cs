#region File Header
//
//      FILE:   CellEditorValuePropertyTypeConverter.cs.
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
using System.Windows.Forms;
using System.ComponentModel;
using System.Drawing;
using System.Reflection;
using System.Windows.Forms.Design;
using System.Drawing.Design;
using System.Globalization;
using System.ComponentModel.Design.Serialization;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a type converter for displaying the ValueProperty of a CellEditor.
    /// </summary>
    /// <remarks>
    /// The standard type converter associated with the PropertyInfo class displays the return type as well
    /// as the name.
    /// </remarks>
    internal class CellEditorValuePropertyTypeConverter : TypeConverter
    {
	
        /// <summary>
        /// Allows conversion to a string
        /// </summary>
        /// <param name="context"></param>
        /// <param name="destType"></param>
        /// <returns>True if the destType is String</returns>
        public override bool CanConvertTo(ITypeDescriptorContext context, Type destType) 
        {
            if (destType == typeof(string)) 
            {
                return true;
            }
            return base.CanConvertTo(context, destType);
        }	
		    

        /// <summary>
        /// Converts the control value to a string
        /// </summary>
        /// <param name="context"></param>
        /// <param name="info"></param>
        /// <param name="value"></param>
        /// <param name="destType"></param>
        /// <returns></returns>
        public override object ConvertTo(ITypeDescriptorContext context,CultureInfo info,object value,Type destType )
        {
            if (destType==typeof(string))
            {
                return (value as PropertyInfo).Name;
            }
            return base.ConvertTo(context,info,value,destType);
        }
		
    }


}
