#region File Header
//
//      FILE:   CellEditor.cs.
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
using System.Diagnostics;
using System.Windows.Forms;
using System.ComponentModel;
using System.Drawing;
using System.Reflection;
using System.Windows.Forms.Design;
using System.Drawing.Design;
using System.Globalization;
using System.ComponentModel.Design;
using System.ComponentModel.Design.Serialization;
using System.Collections;
using Infralution.Common;
using Infralution.Controls.VirtualTree.Properties;
using WF = System.Windows.Forms;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Provides the <see cref="VirtualTree"/> with information about a <see cref="WF.Control"/>
    /// to be used to edit values displayed in tree.
    /// </summary>
    /// <remarks>
    /// This class is an adaptor that allows any <see cref="WF.Control"/> to be used to edit values that
    /// are displayed in a <see cref="VirtualTree"/>.  Typically CellEditors are created and added to the
    /// <see cref="VirtualTree.Editors">VirtualTree.Editors</see> list using the designer.   They
    /// are then associated with individual rows/columns using <see cref="CellBinding">CellBindings</see>
    /// </remarks>
    /// <seealso href="XtraEditors.html">Defining and using Editors</seealso>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="CellEditorList"/>
    /// <seealso cref="CellBinding"/>
    /// <seealso cref="CellData"/>
    [DesignTimeVisible(false), ToolboxItem(false)]
    public class CellEditor : Component
    {
 
        #region Member Variables

        private Control _control = null;
        private CellEditorDisplayMode _displayMode = CellEditorDisplayMode.OnEdit;
        private bool _useCellColors = true;
        private bool _useCellWidth = true;
        private bool _useCellFont = true;
        private bool _useCellHeight = true;
        private ContentAlignment _cellAlignment = ContentAlignment.MiddleLeft;
        private bool _ensureVisibleOnFocus = false;
        private bool _usesUpDownKeys = true;
        private bool _usesLeftRightKeys = true;

        private PropertyInfo _valueProperty = null;
        private string _valuePropertyName;

        private EventInfo _updateValueEvent = null;
        private string _updateValueEventName;

        private Stack _cachedControls = new Stack();

        #endregion

        #region Public Events

        /// <summary>
        /// Raised by the CellEditor to initialize controls created by the editor.
        /// </summary>
        /// <remarks>
        /// Handle this event if you wish to handle control initialization yourself.
        /// The default implementation uses the <see cref="ReflectionUtilities.CopyModifiedProperties"/>
        /// to initialize new controls by copying the properties of the template control.
        /// </remarks>
        [Category("Behavior"),
        Description("Allows you to initialize the control state")]
        public event CellEditorInitializeHandler InitializeControl;

        /// <summary>
        /// Raised by the CellEditor to set the value in the editor control.
        /// </summary>
        /// <remarks>
        /// Handle this event if you wish to handle setting the control value yourself.
        /// The default implementation uses reflection to set the control <see cref="ValueProperty"/>.
        /// </remarks>
        [Category("Behavior"),
        Description("Set the property of the control to the given value")]
        public event CellEditorSetValueHandler SetControlValue;

        /// <summary>
        /// Raised by the CellEditor to get the value from the editor control.
        /// </summary>
        /// <remarks>
        /// Handle this event if you wish to handle getting the control value yourself.
        /// The default implementation uses reflection to get the control <see cref="ValueProperty"/>.
        /// </remarks>
        [Category("Behavior"),
        Description("Get the changed value from the control")]
        public event CellEditorGetValueHandler GetControlValue;

        #endregion

        #region Public Interface

        /// <summary>
        /// Default constructor
        /// </summary>
        public CellEditor()
        {
        }

        /// <summary>
        /// Create a Cell Editor that uses the given control to edit values
        /// </summary>
        /// <param name="control">The control to use to edit values</param>
        public CellEditor(Control control)
        {
            _control = control;
            if (_control != null)
            {
                _control.Visible = false;
            }
            ValueProperty = DefaultValueProperty();
        }

        /// <summary>
        /// Determines when the editor should be displayed.
        /// </summary>
        [Category("Editor"),
        Description("Determines when the editor should be displayed"),
        DefaultValue(CellEditorDisplayMode.OnEdit)]
        public CellEditorDisplayMode DisplayMode
        {
            get { return _displayMode; }
            set { _displayMode = value; }
        }

        /// <summary>
        /// The control that is used to edit values.
        /// </summary>
        [Category("Editor")]
        [Description("The control that is used to edit values")]
        [TypeConverter("Infralution.Controls.VirtualTree.Design.CellEditorControlTypeConverter, " + DesignAssembly.Name)]
        public Control Control
        {
            get { return _control; }
            set 
            { 
                _control = value;
                if (_control != null)
                {
                    _control.Visible = false;
                }
                if (_control is CheckBox)
                {
                    _usesUpDownKeys = false;
                    _usesLeftRightKeys = false;
                }
                UpdatePropertyAndEventInfo();
            }
        }

        /// <summary>
        /// If true the tree is scrolled to ensure that the editor controls is fully visible.
        /// </summary>
        [Category("Editor"),
        Description("Should the tree be scrolled to make the editor control fully visible on focus"),
        DefaultValue(false)]
        public bool EnsureVisibleOnFocus
        {
            get { return _ensureVisibleOnFocus; }
            set { _ensureVisibleOnFocus = value; }
        }

        /// <summary>
        /// Should <see cref="WF.Control.BackColor"/> and  <see cref="WF.Control.ForeColor"/> be set to match 
        /// those of the <see cref="VirtualTree"/> cell color.
        /// </summary>
        [Category("Editor"),
        Description("Should the control colors be set to match the VirtualTree cell colors"),
        DefaultValue(true)]
        public bool UseCellColors
        {
            get { return _useCellColors; }
            set { _useCellColors = value; }
        }

        /// <summary>
        /// Should the <see cref="WF.Control.Font"/> be set to match the <see cref="VirtualTree"/> cell font.
        /// </summary>
        [Category("Editor"),
        Description("Should the control font be set to match the VirtualTree cell font"),
        DefaultValue(true)]
        public bool UseCellFont
        {
            get { return _useCellFont; }
            set { _useCellFont = value; }
        }

        /// <summary>
        /// Should the <see cref="WF.Control.Height"/> be changed to fit the cell. 
        /// </summary>
        /// <remarks>
        /// This can cause problems with controls that autosize their height.
        /// </remarks>
        [Category("Editor"),
        Description("Should the control height be changed to fit the cell."),
        DefaultValue(true)]
        public bool UseCellHeight
        {
            get { return _useCellHeight; }
            set { _useCellHeight = value; }
        }

        /// <summary>
        /// Should the <see cref="WF.Control.Width"/> be changed to fit the cell. 
        /// </summary>
        /// <remarks>
        /// This can cause problems with controls that autosize their width.
        /// </remarks>
        [Category("Editor"),
        Description("Should the control width be changed to fit the cell."),
        DefaultValue(true)]
        public bool UseCellWidth
        {
            get { return _useCellWidth; }
            set { _useCellWidth = value; }
        }

        /// <summary>
        /// Set/Get the alignment of the <see cref="Control"/> with respect to the cell 
        /// </summary>
        [Category("Editor"),
        Description("Set/Get the alignment of the control with respect to the cell"),
        DefaultValue(ContentAlignment.MiddleLeft)]
        public ContentAlignment CellAlignment
        {
            get { return _cellAlignment; }
            set { _cellAlignment = value; }
        }

        /// <summary>
        /// Does the control use the up/down arrow keys. 
        /// </summary>
        /// <remarks>
        /// If this is false then <see cref="VirtualTree"/> will capture the up/down
        /// arrows and use them for navigation
        /// </remarks>
        [Category("Editor")]
        [Description("Does the control use the up/down arrow keys. If false Virtual Tree will use them for navigation")]
        [DefaultValue(true)]
        public bool UsesUpDownKeys
        {
            get { return _usesUpDownKeys; }
            set { _usesUpDownKeys = value; }
        }

        /// <summary>
        /// Does the control use the left/right arrow keys. 
        /// </summary>
        /// <remarks>
        /// If this is false then <see cref="VirtualTree"/> will capture the left/right
        /// arrows and use them for navigation
        /// </remarks>
        [Category("Editor")]
        [Description("Does the control use the left/right arrow keys. If false Virtual Tree will use them for navigation")]
        [DefaultValue(true)]
        public bool UsesLeftRightKeys
        {
            get { return _usesLeftRightKeys; }
            set { _usesLeftRightKeys = value; }
        }

        /// <summary>
        /// The property of the <see cref="Control"/> used to set/get the cell value
        /// </summary>
        [Category("Editor")]
        [Description("The property of the control used to set/get the cell value")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        [Editor("Infralution.Controls.VirtualTree.Design.CellEditorValuePropertyEditor, " + DesignAssembly.Name, typeof(UITypeEditor))]
        [TypeConverter("Infralution.Controls.VirtualTree.Design.CellEditorValuePropertyTypeConverter, " + DesignAssembly.Name)]
        public PropertyInfo ValueProperty
        {
            get 
            {
                if (_valueProperty == null)
                    _valueProperty = DefaultValueProperty();
                return _valueProperty; 
            }
            set 
            { 
                _valueProperty = value; 
                _valuePropertyName = (_valueProperty == null) ? null : _valueProperty.Name;
            }
        }

        /// <summary>
        /// Reset the property to the default
        /// </summary>
        private void ResetValueProperty()
        {
            _valueProperty = null;
        }

        /// <summary>
        /// Should the value property be serialized
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeValueProperty()
        {
            return DefaultValueProperty() != ValueProperty;
        }

        /// <summary>
        /// The name of the property of the <see cref="Control"/> used to set/get the cell value
        /// </summary>
        /// <remarks>
        /// This is an alternative to the <see cref="ValueProperty"/> property. It allows the property to
        /// be selected by name rather than providing an PropertyInfo object.  It is also used for code 
        /// serialization.
        /// </remarks>
        [Browsable(false)]
        public string ValuePropertyName
        {
            get 
            {
                return _valuePropertyName;
            }
            set 
            { 
                _valuePropertyName = value; 
                UpdatePropertyAndEventInfo();
            }
        }

        /// <summary>
        /// Should the property be serialized
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeValuePropertyName()
        {
            return ShouldSerializeValueProperty();
        }

        /// <summary>
        /// The event of the <see cref="Control"/> which, when fired, causes the value
        /// to be set in the <see cref="VirtualTree.DataSource"/>
        /// </summary>
        /// <remarks>
        /// Typically this is set to be the <see cref="WF.Control.Validated"/> event of the editor
        /// control.  This means that the value is set in the data source as focus leaves the
        /// editor controls.  For some controls (eg CheckBoxes) it is may be desirable to set this
        /// to another event (such as the CheckedChanged event) so that the value is set immediately.
        /// </remarks>
        [Category("Editor")]
        [Description("Event which causes the value to be set in the data source")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        [Editor("Infralution.Controls.VirtualTree.Design.CellEditorUpdateValueEventEditor, " + DesignAssembly.Name, typeof(UITypeEditor))]
        [TypeConverter("Infralution.Controls.VirtualTree.Design.CellEditorUpdateValueEventTypeConverter, " + DesignAssembly.Name)]
        public EventInfo UpdateValueEvent
        {
            get 
            {
                if (_updateValueEvent == null)
                    _updateValueEvent = DefaultUpdateValueEvent();
                return _updateValueEvent; 
            }
            set 
            { 
                _updateValueEvent = value;
                _updateValueEventName = (_updateValueEvent == null) ? null : _updateValueEvent.Name;
            }
        }

 
        /// <summary>
        /// Reset the property to the default
        /// </summary>
        private void ResetUpdateValueEvent()
        {
            _updateValueEvent = null;
        }

        /// <summary>
        /// Should the property be serialized
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeUpdateValueEvent()
        {
            return DefaultUpdateValueEvent() != UpdateValueEvent;
        }

        /// <summary>
        /// The name of the event of the <see cref="Control"/> which, when fired, causes the value
        /// to be updated in the <see cref="VirtualTree.DataSource"/>
        /// </summary>
        /// <remarks>
        /// This is an alternative to the <see cref="UpdateValueEvent"/> it allows the event to be selected
        /// by name rather than providing an EventInfo object.  It is also used for code serialization.
        /// </remarks>
        [Browsable(false)]
        public string UpdateValueEventName
        {
            get 
            {
                return _updateValueEventName;
            }
            set 
            { 
                _updateValueEventName = value; 
                UpdatePropertyAndEventInfo();
            }
        }

        /// <summary>
        /// Should the property be serialized
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeUpdateValueEventName()
        {
            return ShouldSerializeUpdateValueEvent();
        }

        /// <summary>
        /// Get a control to use for editor.
        /// </summary>
        /// <remarks>Attempts to get a control from the cache otherwise creates a new control</remarks>
        /// <param name="cellWidget">The CellWidget the editor control is associated with</param>
        /// <returns>A control that can be used for editing</returns>
        public virtual Control GetControl(CellWidget cellWidget)
        {
            Control control = null;
            if (_cachedControls.Count > 0)
            {
                control = (Control) _cachedControls.Pop();
                OnInitializeControl(cellWidget, control, false);
            }
            else
            {
                control = CreateControl();
                OnInitializeControl(cellWidget, control, true);
            }
            return control;
        }

        /// <summary>
        /// Return a control to the cache - enabling it to be reused
        /// </summary>
        /// <param name="control">The control to free to the cache</param>
        public virtual void FreeControl(Control control)
        {
            _cachedControls.Push(control);
        }

        /// <summary>
        /// Setup the control to match the given style
        /// </summary>
        /// <param name="control">The control to set th style of</param>
        /// <param name="style">The style to use</param>
        public virtual void SetControlStyle(Control control, Style style)
        {    
            // note check the values have changed - setting the control properties
            // generates a paint event for control even if the value is unchanged
            //
            if (UseCellColors)
            {
                Color backColor = style.BackColor;
                if (control.BackColor != backColor)
                    control.BackColor = backColor;
                Color foreColor = style.ForeColor;
                if (control.ForeColor != foreColor)
                    control.ForeColor = foreColor;
            }            
            if (UseCellFont)
            {
                Font font = style.Font;
                if (control.Font != style.Font)
                    control.Font = font;
            }
        }
           
        /// <summary>
        /// Position the editor control within the given rectangle
        /// </summary>
        /// <remarks>
        /// This method may not display the control if there is insufficient space for it to be displayed
        /// </remarks>
        /// <param name="control">The control to position</param>
        /// <param name="bounds">The rectangle to position the control within</param>
        /// <param name="showControl">Should the control be shown</param>
        public virtual void LayoutControl(Control control, Rectangle bounds, bool showControl)
        {
            if (showControl)
            {
                // NOTE: we can't set the height/width and location all at once using the control.Bounds
                // property because some AutoSize controls will reset the height or width which requires
                // us to then base the location on the actual height
                //
                if (UseCellHeight && bounds.Height > 0)
                    control.Height = bounds.Height;
                if (UseCellWidth && bounds.Width > 0)
                    control.Width = bounds.Width;

                // if the control won't fit in the avialable width then don't display it
                //
                if (control.Width > bounds.Width)
                {
                    showControl = false;
                }
            }

            if (showControl)
            {
                int left = bounds.X;
                int top = bounds.Y;
                ContentAlignment align = DrawingUtilities.RtlTranslateAlignment(control, CellAlignment);
                switch (align)
                {
                    case ContentAlignment.TopLeft:
                        break;
                    case ContentAlignment.TopCenter:
                        left += (bounds.Width - control.Width) / 2;
                        break;
                    case ContentAlignment.TopRight:
                        left += bounds.Width - control.Width;
                        break;
                    case ContentAlignment.MiddleLeft:
                        top += (bounds.Height - control.Height) / 2;
                        break;
                    case ContentAlignment.MiddleCenter:
                        top += (bounds.Height - control.Height) / 2;
                        left += (bounds.Width - control.Width) / 2;
                        break;
                    case ContentAlignment.MiddleRight:
                        top += (bounds.Height - control.Height) / 2;
                        left += bounds.Width - control.Width;
                        break;
                    case ContentAlignment.BottomLeft:
                        top += bounds.Height - control.Height;
                        break;
                    case ContentAlignment.BottomCenter:
                        top += bounds.Height - control.Height;
                        left += (bounds.Width - control.Width) / 2;
                        break;
                    case ContentAlignment.BottomRight:
                        top += bounds.Height - control.Height;
                        left += bounds.Width - control.Width;
                        break;
                }
                control.Location = new Point(left, top);
                control.Update();
            }
            control.Visible = showControl;
        }

        /// <summary>
        /// Return the current value of the given control
        /// </summary>
        /// <param name="control">The control to get the value of</param>
        /// <remarks>
        /// Uses the <see cref="ValueProperty"/> to get the value from the control using reflection.
        /// </remarks>
        protected virtual object GetValue(Control control)
        {
            PropertyInfo property = ValueProperty;
            if (property != null && control != null)
            {
                return property.GetValue(control, null);
            }
            return null;
        }

        /// <summary>
        /// Return the current value of the given control
        /// </summary>
        /// <param name="cellWidget">The CellWidget getting the value</param>
        /// <param name="control">The control to get the value of</param>
        /// <remarks>
        /// Raises the user defined <see cref="GetControlValue"/> event if present - otherwise
        /// uses the <see cref="ValueProperty"/> to get the value from the control using reflection.
        /// </remarks>
        public virtual object GetValue(CellWidget cellWidget, Control control)
        {
            if (GetControlValue != null)
            {
                CellEditorGetValueEventArgs e = new CellEditorGetValueEventArgs(cellWidget, control);
                GetControlValue(this, e); 
                return e.Value;
            }
            else
            {
                return GetValue(control);
            }
        }

        /// <summary>
        /// Set the value of the control
        /// </summary>
        /// <param name="cellWidget">The CellWidget setting the value</param>
        /// <param name="control">The control to set the value in</param>
        /// <param name="value">The value to set</param>
        /// <returns>True if successful</returns>
        /// <remarks>
        /// Uses the <see cref="ValueProperty"/> to set the value in the control using reflection.
        /// </remarks>
        protected virtual void SetValueInternal(CellWidget cellWidget, Control control, object value)
        {
            PropertyInfo property = ValueProperty;
            if (property == null) throw new InvalidOperationException("CellEditor.ValueProperty must be set before using the CellEditor");
            try
            {
                if (value != null)
                {
                    Type valueType = value.GetType();
                    Type propertyType = property.PropertyType;

                    // if we can't assign the value to the property then attempt to convert it
                    //
                    if (!propertyType.IsAssignableFrom(valueType))
                    {
                        value = Convert.ChangeType(value, propertyType);
                    }
                }
                property.SetValue(control, value, null);
            }
            catch (Exception e)
            {
                VirtualTree tree = cellWidget.Tree;
                tree.DisplayErrorMessage(string.Format(Resources.SetValueErrorMsg, property.Name, control.GetType().FullName, e.GetType().Name));
                if (!tree.SuppressBindingExceptions) throw;
            }

        }

        /// <summary>
        /// Set the value of the control
        /// </summary>
        /// <param name="cellWidget">The CellWidget setting the value</param>
        /// <param name="control">The control to set the value in</param>
        /// <param name="value">The value to set</param>
        /// <returns>True if successful</returns>
        /// <remarks>
        /// Raises the user defined <see cref="SetControlValue"/> event if present - otherwise
        /// uses the <see cref="ValueProperty"/> to set the value in the control using reflection.
        /// </remarks>
        public virtual void SetValue(CellWidget cellWidget, Control control, object value)
        {
            if (control == null) throw new ArgumentNullException("control");
            if (SetControlValue != null)
            {
                SetControlValue(this, new CellEditorSetValueEventArgs(cellWidget, control, value));
            }
            else
            {
                SetValueInternal(cellWidget, control, value);
            }
        }

        /// <summary>
        /// Add an event handler to the nominated UpdateValueEvent of the given control
        /// </summary>
        /// <param name="control">The control to add the handler to</param>
        /// <param name="handler">The handler to add</param>
        public void AddUpdateValueHandler(Control control, System.Delegate handler)
        {
            EventInfo e = UpdateValueEvent;
            if (e != null)
            {
                e.AddEventHandler(control, handler);
            }
        }

        /// <summary>
        /// Remove an event handler from the nominated UpdateValueEvent of the given control
        /// </summary>
        /// <param name="control">The control to add the handler to</param>
        /// <param name="handler">The handler to add</param>
        public void RemoveUpdateValueHandler(Control control, System.Delegate handler)
        {
            EventInfo e = UpdateValueEvent;
            if (e != null)
            {
                e.RemoveEventHandler(control, handler);
            }
        }

        /// <summary>
        /// Returns the string representation of this cell editor.
        /// </summary>
        /// <returns>The string representation of this cell editor</returns>
        public override string ToString()
        {
            if (Site != null)
                return Site.Name;
            else
                return base.ToString();
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Create a new control of the same type as the control template
        /// </summary>
        /// <returns>A new control for editing</returns>
        protected virtual Control CreateControl()
        {
            return (Control)Activator.CreateInstance(_control.GetType());
        }

        /// <summary>
        /// Called to initialize controls before they are provide to clients
        /// </summary>
        /// <remarks>
        /// Override this method if you wish to handle control initialization yourself.
        /// The default implementation uses the <see cref="ReflectionUtilities.CopyModifiedProperties"/>
        /// to initialize new controls by copying the properties of the template control.
        /// </remarks>
        /// <param name="cellWidget">The CellWidget the editor control is associated with</param>
        /// <param name="control">The control to initialize</param>
        /// <param name="newControl">Is this a new control (or a cached control)</param>
        protected virtual void OnInitializeControl(CellWidget cellWidget, Control control, bool newControl)
        {
            if (newControl)
            {
                ReflectionUtilities.CopyModifiedProperties(_control, control);

                // ensure the control is not docked or anchored - as this messes up layout
                // 
                control.Dock = DockStyle.None;
                control.Anchor = AnchorStyles.Top | AnchorStyles.Left;

                // give the control its own unique binding context - this prevents .NET from synching the
                // selection when using DropDownList/Combos which have the same datasource
                //
                control.BindingContext = new BindingContext();
            }
            if (InitializeControl != null)
            {
                CellEditorInitializeEventArgs e = new CellEditorInitializeEventArgs(cellWidget, control, newControl);
                InitializeControl(this, e);
            }
        }

        /// <summary>
        /// Free the cached editor controls
        /// </summary>
        /// <param name="disposing">Is this being called by the GC</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                while (_cachedControls.Count > 0)
                {
                    Control control = (Control) _cachedControls.Pop();
                    control.Dispose();
                }
            }
            base.Dispose (disposing);
        }

 
        /// <summary>
        /// Get the default property to use for the value property for the control
        /// </summary>
        /// <returns>The default property to use</returns>
        protected PropertyInfo DefaultValueProperty()
        {
            PropertyInfo property = null;
            if (_control != null)
            {
                Type type = _control.GetType();
                if (_control is CheckBox)
                {
                    property = ReflectionUtilities.GetProperty(type, "Checked");
                }
                else
                {
                    property = ReflectionUtilities.GetProperty(type, "Value");
                    if (property == null)
                    {
                        property = ReflectionUtilities.GetProperty(type, "Text");
                    }
                }
            }
            return property;
        }

        /// <summary>
        /// Get the default event to use for the UpdateValueEvent 
        /// </summary>
        /// <returns>The default event to use</returns>
        protected EventInfo DefaultUpdateValueEvent()
        {
            EventInfo eventInfo = null;
            if (_control != null)
            {
                Type type = _control.GetType();
                if (_control is CheckBox)
                {
                    eventInfo = type.GetEvent("CheckedChanged");
                }
                else
                {
                    eventInfo = type.GetEvent("Validated");
                }
            }
            return eventInfo;
        }

        private void UpdatePropertyAndEventInfo()
        {
            if (_control == null)
            {
                _updateValueEventName = null;
                _updateValueEvent = null;
                _valuePropertyName = null;
                _valueProperty = null;
            }
            else
            {
                if (_updateValueEventName == null)
                {
                    _updateValueEvent = null;
                }
                else
                {
                    _updateValueEvent = _control.GetType().GetEvent(_updateValueEventName);
                }
                if (_valuePropertyName == null)
                {
                    _valueProperty = null;
                }
                else
                {
                    _valueProperty = ReflectionUtilities.GetProperty(_control.GetType(), _valuePropertyName);
                }
            }
        }

        #endregion

    }




}
