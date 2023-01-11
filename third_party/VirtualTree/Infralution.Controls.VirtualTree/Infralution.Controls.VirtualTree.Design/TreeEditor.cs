#region File Header
//
//      FILE:   TreeEditor.cs.
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
using System.ComponentModel.Design;
using Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree.Design
{

    /// <summary>
    /// Defines a UITypeEditor which invokes the TreeEditorForm to allow the user to edit custom 
    /// properties of a <see cref="VirtualTree"/>.
    /// </summary>
    internal class TreeEditor : UITypeEditor  
    {
 
        /// <summary>
        /// Returns the style used by this editor (Modal)
        /// </summary>
        /// <param name="context"></param>
        /// <returns></returns>
        public override UITypeEditorEditStyle GetEditStyle(System.ComponentModel.ITypeDescriptorContext context)
        {
            return UITypeEditorEditStyle.Modal;
        }

        /// <summary>
        /// Invokes the Virtual Tree editor.
        /// </summary>
        /// <param name="context"></param>
        /// <param name="provider"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        public override object EditValue(System.ComponentModel.ITypeDescriptorContext context, IServiceProvider provider, object value)
        {
            if (!(context.Instance is VirtualTree)) return value; 
            IWindowsFormsEditorService editorService;
            editorService = (IWindowsFormsEditorService)provider.GetService(typeof(IWindowsFormsEditorService));
            TreeEditorForm form = new TreeEditorForm(context.Instance as VirtualTree, provider);   
  
            if (value is RowBindingList)
                form.ActiveTab = TreeEditorForm.Tabs.Bindings;
            else if (value is ColumnList)
                form.ActiveTab = TreeEditorForm.Tabs.Columns;
            else if (value is CellEditorList)
                form.ActiveTab = TreeEditorForm.Tabs.Editors;

            editorService.ShowDialog(form);
            return value;
        }


    }

}


