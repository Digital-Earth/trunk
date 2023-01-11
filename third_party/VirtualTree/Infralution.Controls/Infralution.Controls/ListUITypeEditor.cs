#region File Header
//
//      FILE:   ListUITypeEditor.cs.
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
using System.Drawing;
using System.Drawing.Design;
using System.Windows.Forms.Design;
using System.Windows.Forms;
using System.ComponentModel;
namespace Infralution.Controls
{

    /// <summary>
    /// Defines a base <see cref="UITypeEditor"/> class that uses a list to
    /// display a set of items to allow user selection.
    /// </summary>
    public abstract class ListUITypeEditor : UITypeEditor  
    {

        /// <summary>
        /// The service provider for the design context
        /// </summary>
        private IServiceProvider _serviceProvider;

        /// <summary>
        /// The editor service currently being used.
        /// </summary>
        private IWindowsFormsEditorService _editorService;

        /// <summary>
        /// The current editing context
        /// </summary>
        private ITypeDescriptorContext _context;

        /// <summary>
        /// The current value being edited
        /// </summary>
        private object _value;

 
        /// <summary>
        /// Returns the style used by this editor (dropdown)
        /// </summary>
        /// <param name="context"></param>
        /// <returns></returns>
        public override UITypeEditorEditStyle GetEditStyle(ITypeDescriptorContext context)
        {
            return UITypeEditorEditStyle.DropDown;
        }

        /// <summary>
        /// Invokes the data lookup editor.
        /// </summary>
        /// <param name="context"></param>
        /// <param name="provider"></param>
        /// <param name="value"></param>
        /// <returns></returns>
        public override object EditValue(ITypeDescriptorContext context, IServiceProvider provider, object value)
        {
            _serviceProvider = provider;
            _context = context;
            _editorService = (IWindowsFormsEditorService)provider.GetService(typeof(IWindowsFormsEditorService));
            if (_editorService == null) return value;
       
            IList items = GetItems(context);
            if (items == null) return value;

            _value = value;

            VirtualListBox listBox = new VirtualListBox();   
            listBox.BeginInit();
            listBox.AllowMultiSelect = false;
            listBox.BorderStyle = BorderStyle.None;
            listBox.Width = 50;
            listBox.MouseUp += new MouseEventHandler(OnListMouseUp);
            listBox.KeyDown += new KeyEventHandler(OnListKeyDown);
            listBox.GetItemText += new GetItemTextHandler(OnGetItemText);
            listBox.DataSource = items;
            listBox.Height = listBox.PreferredHeight;
            listBox.EndInit();
            listBox.SelectedItem = value;
            
            _editorService.DropDownControl(listBox);
            if (context != null && context.PropertyDescriptor != null && _value != value )
            {
                context.PropertyDescriptor.SetValue(context.Instance, _value);
                context.OnComponentChanged();
            }         
            return _value;
        }

        /// <summary>
        /// Return the current editing service
        /// </summary>
        protected ITypeDescriptorContext Context
        {
            get { return _context; }
        }

        /// <summary>
        /// Return the service provider for the current context
        /// </summary>
        protected IServiceProvider ServiceProvider
        {
            get { return _serviceProvider; }
        }


        /// <summary>
        /// Return the current editor context
        /// </summary>
        protected IWindowsFormsEditorService EditorService
        {
            get { return _editorService; }
        }

        /// <summary>
        /// Return the list of items to display
        /// </summary>
        /// <param name="context">The context for the editing operation</param>
        /// <returns>The list of items to display - or null if the context is invalid</returns>
        protected abstract IList GetItems(ITypeDescriptorContext context);

        /// <summary>
        /// Return the display string to use for the given item
        /// </summary>
        /// <remarks>
        /// The default implementation returns item.ToString
        /// </remarks>
        protected virtual string GetDisplayString(object item)
        {
            return item.ToString();
        }

        /// <summary>
        /// Return the value to set for the given item
        /// </summary>
        /// <remarks>
        /// This allows a mapping between the displayed items and the value set by the editor.
        /// The default implementation just returns the item
        /// </remarks>
        /// <param name="item">The item to get the value for</param>
        protected virtual object GetValue(object item)
        {
            return item;
        }

        /// <summary>
        /// Set the current value based on the selected item in the list
        /// </summary>
        /// <param name="list">The list to get the value from</param>
        private void SetSelectedValue(VirtualListBox list)
        {
            _value = GetValue(list.SelectedItem);
        }

        /// <summary>
        /// Handle mouse up events for the list
        /// </summary>
        /// <remarks>
        /// Sets the value of the object being edited and closes the drop down
        /// </remarks>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnListMouseUp(object sender, MouseEventArgs e)
        {
            SetSelectedValue((VirtualListBox)sender);
            _editorService.CloseDropDown();
        }

        /// <summary>
        /// Handle Enter and Escape keys for the list
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnListKeyDown(object sender, KeyEventArgs e)
        {
            switch(e.KeyCode)
            {
                case Keys.Enter:
                    SetSelectedValue((VirtualListBox)sender);
                    _editorService.CloseDropDown();
                    break;
                case Keys.Escape:
                    _editorService.CloseDropDown();
                    break;
            }
        }

        /// <summary>
        /// Call GetDisplayString to get the text to display for an item
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnGetItemText(object sender, GetItemTextEventArgs e)
        {
            e.Text = GetDisplayString(e.Item);
        }
    }

}


