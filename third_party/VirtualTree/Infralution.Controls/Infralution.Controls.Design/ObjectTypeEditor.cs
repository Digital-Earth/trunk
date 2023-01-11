#region File Header
//
//      FILE:   ObjectTypeEditor.cs.
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
using System.Reflection;
using System.ComponentModel;
using Infralution.Common;
using Infralution.Controls;
namespace Infralution.Controls.Design
{

    /// <summary>
    /// Defines the UITypeEditor for selecting object type from a drop down list
    /// </summary>
    public class ObjectTypeEditor : Infralution.Controls.ListUITypeEditor  
    {

        /// <summary>
        /// Return the list of items to display
        /// </summary>
        /// <param name="context">The context for the editor</param>
        /// <returns>The items - or null if the context is not supported</returns>
        protected override IList GetItems(ITypeDescriptorContext context)
        {
             return TypeUtilities.GetReferencedTypes(ServiceProvider);
        }

        /// <summary>
        ///  Return the string to display for the given item
        /// </summary>
        /// <param name="item"></param>
        /// <returns></returns>
        protected override string GetDisplayString(object item)
        {
            return (item as Type).FullName;
        }

    }

}


