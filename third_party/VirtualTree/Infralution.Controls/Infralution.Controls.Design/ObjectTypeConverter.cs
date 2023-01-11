#region File Header
//
//      FILE:   ObjectTypeConverter.cs.
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
using Infralution.Controls;
namespace Infralution.Controls.Design
{

    /// <summary>
    /// Defines the type converter used to convert Types to/from strings.
    /// </summary>
    public class ObjectTypeConverter : TypeConverter
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
        /// Allows conversion from string
        /// </summary>
        /// <param name="context"></param>
        /// <param name="sourceType"></param>
        /// <returns></returns>
        public override bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType)
        {
            if (sourceType == typeof(string))
            {
                return true;
            }
            return base.CanConvertFrom (context, sourceType);
        }

        /// <summary>
        /// Converts the type to a string
        /// </summary>
        /// <param name="context"></param>
        /// <param name="info"></param>
        /// <param name="value"></param>
        /// <param name="destType"></param>
        /// <returns></returns>
        public override object ConvertTo(ITypeDescriptorContext context,CultureInfo info, object value, Type destType )
        {
            if (destType==typeof(string) && value is Type)
            {
                return (value as Type).FullName;
            }
            return base.ConvertTo(context,info,value,destType);
        }

        /// <summary>
        /// Converts string typename to a type
        /// </summary>
        /// <param name="context"></param>
        /// <param name="culture"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        public override object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
        {
            if (value == null) return null;
            if (value.GetType() == typeof(string))
            {
                string typeName = (value as string).Trim();
                if (typeName.Length == 0) return null;
                return Type.GetType(typeName, true);
            }
            return base.ConvertFrom (context, culture, value);
        }

    }


}
