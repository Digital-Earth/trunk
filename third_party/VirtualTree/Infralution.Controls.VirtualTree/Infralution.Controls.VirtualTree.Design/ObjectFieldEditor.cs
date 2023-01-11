#region File Header
//
//      FILE:   ObjectFieldEditor.cs.
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
using System.Data;
using System.ComponentModel;
using System.Reflection;
using Infralution.Controls.VirtualTree;
using Infralution.Controls.Design;
namespace Infralution.Controls.VirtualTree.Design
{

    /// <summary>
    /// Defines the <see cref="UITypeEditor"/> for Field names for 
    /// <see cref="ObjectCellBinding"/> and <see cref="ObjectRowBinding"/>
    /// classes.
    /// </summary>
    internal class ObjectFieldEditor : Infralution.Controls.ListUITypeEditor  
    {

        public override UITypeEditorEditStyle GetEditStyle(ITypeDescriptorContext context)
        {
            // In VS2005 GetEditStyle the context passed to GetEditStyle is always null
            // Use the cached value if possible. 
            //
            if (context == null)
                context = this.Context;
            if (context == null) return UITypeEditorEditStyle.None;

            // check that the object is a ObjectCellBinding
            //
            ObjectRowBinding rowBinding = null;
            if (context.Instance is ObjectCellBinding)
            {
                rowBinding = (context.Instance as ObjectCellBinding).RowBinding as ObjectRowBinding;
            }
            else if (context.Instance is ObjectRowBinding)
            {
                rowBinding = context.Instance as ObjectRowBinding;
            }
            else
            {
                return UITypeEditorEditStyle.None;
            }

            // if there is no type name set or if they are using ITypedList binding
            //
            if (rowBinding.TypeName == null || rowBinding.TypedListName != null)
            {
                return UITypeEditorEditStyle.None;
            }
            return base.GetEditStyle (context);
        }

        /// <summary>
        /// Return the list of items to display
        /// </summary>
        /// <param name="context">The context for the editor</param>
        /// <returns>The items - or null if the context is not supported</returns>
        protected override IList GetItems(ITypeDescriptorContext context)
        {

            // check that the object is a ObjectCellBinding
            //
            ObjectRowBinding rowBinding = null;
            if (context.Instance is ObjectCellBinding)
            {
                rowBinding = (context.Instance as ObjectCellBinding).RowBinding as ObjectRowBinding;
            }
            else if (context.Instance is ObjectRowBinding)
            {
                rowBinding = context.Instance as ObjectRowBinding;
            }
            else
            {
                return null;
            }

            if (rowBinding.TypeName == null) return null;

            // The overloaded version of GetCodeModelPropertyNames does not properly work when the property type is derived
            // from an interface such as IList
            //
            //if (context.PropertyDescriptor.Name == "ChildProperty")
            //    return TypeUtilities.GetCodeModelPropertyNames(ServiceProvider, rowBinding.TypeName, "System.Collections.IList");
            //else
            //    return TypeUtilities.GetCodeModelPropertyNames(ServiceProvider, rowBinding.TypeName);

            return TypeUtilities.GetCodeModelPropertyNames(ServiceProvider, rowBinding.TypeName);
        }


    }

}


