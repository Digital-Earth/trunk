#region File Header
//
//      FILE:   StandardValueEditor.cs.
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
using System.ComponentModel;
namespace Infralution.Controls
{

    /// <summary>
    /// Defines an editor for selecting StandardValues for types that have a TypeConverter
    /// that supports StandardValues but no associated editor.
    /// </summary>
    public class StandardValueEditor : ListUITypeEditor  
    {

        /// <summary>
        /// The type converter to use to display the standard values
        /// </summary>
        private TypeConverter _converter;  

        /// <summary>
        /// Create a new editor
        /// </summary>
        public StandardValueEditor()
        {
        }

        /// <summary>
        /// Create a new editor to use the given type converter
        /// </summary>
        /// <remarks>
        /// This allows allows a type converter other than the default to be specified
        /// </remarks>
        /// <param name="typeConverter">The TypeConverter to display the standard values from</param>
        public StandardValueEditor(TypeConverter typeConverter)
        {
            _converter = typeConverter;
        }

        /// <summary>
        /// Return the items to display
        /// </summary>
        /// <param name="context">The context for the editor</param>
        /// <returns>A list of items to display</returns>
        protected override IList GetItems(ITypeDescriptorContext context)
        {
            // Get the type converter to use (if this wasn't set in the constructor)
            //
            if (_converter == null && context != null && context.PropertyDescriptor != null)
            {
                _converter = context.PropertyDescriptor.Converter;
            }
            if (_converter == null) return null;
            ICollection values = _converter.GetStandardValues(context);
            if (values is IList)
                return values as IList;
            else
                return new ArrayList(values);
        }

        /// <summary>
        /// Return the string to display for the given item
        /// </summary>
        /// <param name="item">The item to get the display string for</param>
        /// <returns>The display string</returns>
        protected override string GetDisplayString(object item)
        {
            return _converter.ConvertToString(Context, item);
        }

    }
}


