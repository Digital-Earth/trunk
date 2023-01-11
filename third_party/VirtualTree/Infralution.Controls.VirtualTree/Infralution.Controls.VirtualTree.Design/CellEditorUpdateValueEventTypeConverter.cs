#region File Header
//
//      FILE:   CellEditorUpdateValueEventTypeConverter.cs.
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
using Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree.Design
{

    /// <summary>
    /// Defines a type converter for displaying the UpdateValueEvent of a CellEditor.
    /// </summary>
    /// <remarks>
    /// The standard type converter associated with the EventInfo class displays the event type as well
    /// as the name.
    /// </remarks>
    internal class CellEditorUpdateValueEventTypeConverter : TypeConverter
    {
	
        /// <summary>
        /// Allows conversion to a string
        /// </summary>
        /// <param name="context"></param>
        /// <param name="destType"></param>
        /// <returns>True if the destType is String</returns>
        public override bool CanConvertTo(ITypeDescriptorContext context, Type destType) 
        {
            if (destType == typeof(string) || destType == typeof(InstanceDescriptor))
            {
                return true;
            }
            return base.CanConvertTo(context, destType);
        }	

        /// <summary>
        /// Converts the event to a string
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
                return (value as EventInfo).Name;
            }
            return base.ConvertTo(context,info,value,destType);
        }
		
    }


}
