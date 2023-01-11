#region File Header
//
//      FILE:   DataViewCellBindingConverter.cs.
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
using System.Globalization;
using System.ComponentModel.Design.Serialization;
using Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree.Design
{

    /// <summary>
    /// Type Converter for DataViewCellBinding class required for code serialization. 
    /// </summary>
    internal class DataViewCellBindingConverter : ExpandableObjectConverter
    {
	
        /// <summary>
        /// Return true if the conversion can be performed by the type converter
        /// </summary>
        /// <param name="context"></param>
        /// <param name="destType"></param>
        /// <returns></returns>
        public override bool CanConvertTo(ITypeDescriptorContext context, Type destType) 
        {
            if (destType == typeof(InstanceDescriptor)) 
            {
                return true;
            }
            return base.CanConvertTo(context, destType);
        }	
		
        /// <summary>
        /// Convert the given value.
        /// </summary>
        /// <param name="context"></param>
        /// <param name="info"></param>
        /// <param name="value"></param>
        /// <param name="destType"></param>
        /// <returns></returns>
        public override object ConvertTo(ITypeDescriptorContext context, CultureInfo info,
                                         object value, Type destType)
        {
            if (destType == typeof(InstanceDescriptor)) 
            {
                DataViewCellBinding binding = (DataViewCellBinding)value;
                return new InstanceDescriptor(
                    typeof(DataViewCellBinding).GetConstructor(new Type[0]), null,false);

            }
            return base.ConvertTo(context,info,value,destType);
        }

		
    }

}
