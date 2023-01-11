#region File Header
//
//      FILE:   ObjectTypeNameEditor.cs.
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
    /// Defines the UITypeEditor for selecting object type names from a drop down list
    /// </summary>
    public class ObjectTypeNameEditor : Infralution.Controls.ListUITypeEditor  
    {

        /// <summary>
        /// Return the list of items to display
        /// </summary>
        /// <param name="context">The context for the editor</param>
        /// <returns>The items - or null if the context is not supported</returns>
        protected override IList GetItems(ITypeDescriptorContext context)
        {
            return TypeUtilities.GetCodeModelTypeNames(ServiceProvider);
        }
    }

}


