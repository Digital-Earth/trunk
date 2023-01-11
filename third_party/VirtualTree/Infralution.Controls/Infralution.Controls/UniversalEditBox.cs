#region File Header
//
//      FILE:   UniversalEditBox.cs.
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
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Design;
using System.Globalization;
using System.Windows.Forms;
using System.Windows.Forms.Design;

namespace Infralution.Controls
{
	/// <summary>
	/// Defines an inplace editor control that allows editing of any object type that supports
	/// a <see cref="UITypeEditor"/>.
	/// </summary>
	/// <remarks>
	/// The <see cref="UITypeEditor"/> mechanism is used by the standard <see cref="PropertyGrid"/> 
	/// to display properties of Controls and Components from within the Visual Studio designer.
	/// This control leverages this mechanism to provide standalone control that can be used within
	/// applications for editing objects of any type.
	/// </remarks>
    [ToolboxItem(true)]
#if CHECK_LICENSE
    [LicenseProvider(typeof(ControlLicenseProvider))]
#endif
	public class UniversalEditBox : BorderedControl, 
        IServiceProvider, IWindowsFormsEditorService, ITypeDescriptorContext
	{

        #region Member Variables

        /// <summary>
        /// The object instance that "owns" the value being edited
        /// </summary>
        private object _valueOwner;

        /// <summary>
        /// The value to be edited
        /// </summary>
        private object _value;

        /// <summary>
        /// The type of the value to be edited
        /// </summary>
        private Type _valueType;

        /// <summary>
        /// Should null values be allowed
        /// </summary>
        private bool _allowNullValues = false;

        /// <summary>
        /// The editor to use
        /// </summary>
        private UITypeEditor _editor;

        /// <summary>
        /// Should the default editor for the given type be used
        /// </summary>
        private bool _useDefaultEditor = true;

        /// <summary>
        /// The type converter to use
        /// </summary>
        private TypeConverter _converter;

        /// <summary>
        /// Should the default type converter for the given type be used
        /// </summary>
        private bool _useDefaultConverter = true;

        /// <summary>
        /// An array of standard values (if supported)
        /// </summary>
        private object[] _standardValues;

        /// <summary>
        /// Should the control show the preview for the current value
        /// </summary>
        private bool _showPreview = true;

        /// <summary>
        /// Should the control show the text for the current value 
        /// </summary>
        private bool _showText = true;

        /// <summary>
        /// Should the control height be set automatically
        /// </summary>
        private bool _autoSize = true;

        /// <summary>
        /// The active drop down form (if any)
        /// </summary>
        private DropDownForm _dropDownForm;

        /// <summary>
        /// The color to use for the background of the drop down button and form
        /// </summary>
        private Color _dropDownBackColor = Color.Empty;

        /// <summary>
        /// The color to use for the foreground of the drop down button and form
        /// </summary>
        private Color _dropDownForeColor = Color.Empty;

        /// <summary>
        /// The caption to use when displaying validation errors
        /// </summary>
        private string _textErrorCaption = DefaultTextErrorCaption;
        private const string DefaultTextErrorCaption = "Invalid Value";

        /// <summary>
        /// The text to use when displaying validation errors
        /// </summary>
        private string _textErrorMessage = DefaultTextErrorMessage;
        private const string DefaultTextErrorMessage = "{0} is not a valid {1} value";
       
        /// <summary>
        /// The text box used for text entry
        /// </summary>
        private TextBox _textBox;
   
        /// <summary>
        /// The button used to display modal editors
        /// </summary>
        private Button _editButton;

        /// <summary>
        /// The button used to drop down lists
        /// </summary>
        private DropDownButton _dropDownButton;

        /// <summary>
        /// Time the value was last edited - used to prevent the clicking on the drop
        /// down button while edit list is displayed from reactivating edit list
        /// </summary>
        private DateTime _editTime;

        /// <summary>
        /// Panel used to display the preview
        /// </summary>
        private Panel _previewPanel;

        /// <summary> 
		/// Required designer variable.
		/// </summary>
		private Container components = null;
 

        #endregion

        #region Events

        /// <summary>
        /// Fired when the Value property is changed
        /// </summary>
        [Category("Property Changed"),
         Description("Event fired when the Value property is changed")]
        public event EventHandler ValueChanged;

        /// <summary>
        /// Fired when validation of user entered text fails
        /// </summary>
        [Category("Behavior"),
        Description("Event fired when validation of user entered text fails")]
        public event ValidateTextErrorHandler ValidateTextError;

        #endregion

        #region Public Interface

        /// <summary>
        /// Create a new instance of the control
        /// </summary>
		public UniversalEditBox()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();
            CultureManager.ApplicationUICultureChanged += new CultureManager.CultureChangedHandler(OnApplicationUICultureChanged);
            BackColor = SystemColors.Window;
            ForeColor = SystemColors.WindowText;
            #if CHECK_LICENSE
            ControlLicenseProvider.CheckLicense(ReflectionUtilities.GetInstantiatingAssembly(), typeof(UniversalEditBox), this);
            #endif
        }

        /// <summary>
        /// Set/Get the value to be edited.  This will set the <see cref="ValueType"/>
        /// property (if not already set) based on the value type. 
        /// </summary>
        [Browsable(false),
         DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual object Value
        {
            get { return _value; }
            set 
            {
                bool unchanged = (_value == null) ? (value == null) : _value.Equals(value);
                if (!unchanged)
                {
                    _value = value;
                    if (value != null && !Convert.IsDBNull(value)) 
                        ValueType = value.GetType();
                     OnValueChanged();
                }

                // make sure the displayed text now reflects the new value
                //
                UpdateText();
                PreviewPanel.Invalidate();
            }
        }

        /// <summary>
        /// Set/Get the object instance that contains (or "owns") the value being
        /// edited. 
        /// </summary>
        /// <remarks>
        /// This value is passed to the <see cref="UITypeEditor"/> and <see cref="TypeConverter"/>
        /// via the <see cref="ITypeDescriptorContext.Instance"/> property to provide them with
        /// contextual information about the object being edited.
        /// </remarks>
        [Browsable(false),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual object ValueOwner
        {
            get { return _valueOwner; }
            set { _valueOwner = value; }
        }

        /// <summary>
        /// Hides the text property in the designer
        /// </summary>
        [EditorBrowsable(EditorBrowsableState.Never),
        Browsable(false),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public override string Text
        {
            get { return base.Text; }
            set { base.Text = value; }
        }

        /// <summary>
        /// Set/Get the background color of the control
        /// </summary>
        [Description("Set/Get the background color for the control"),
        Category("Appearance"),
        DefaultValue(typeof(Color), "Window")]
        public override Color BackColor
        {
            get { return base.BackColor; }
            set 
            { 
                if (value != base.BackColor)
                {
                    base.BackColor = value;
                    UpdateComponentControlColors();
                }
            }
        }

        /// <summary>
        /// Set/Get the foreground color for the control
        /// </summary>
        [Description("Set/Get the foreground color for the control"),
        Category("Appearance"),
        DefaultValue(typeof(Color), "WindowText")]
        public override Color ForeColor
        {
            get { return base.ForeColor; }
            set 
            { 
                if (value != base.ForeColor)
                {
                    base.ForeColor = value;
                    UpdateComponentControlColors();
                }
            }
        }

        /// <summary>
        /// Set/Get the background color of the drop down button
        /// </summary>
        [Description("Set/Get the background color of the drop down button"),
        Category("Appearance"),
        AmbientValue(typeof(Color), "")]
        public virtual Color DropDownBackColor
        {
            get 
            { 
                if (_dropDownBackColor == Color.Empty) 
                {
                    return (Parent == null) ? SystemColors.Control : Parent.BackColor;
                }
                return _dropDownBackColor; 
            }
            set 
            { 
                _dropDownBackColor = value; 
                UpdateComponentControlColors();
            }
        }

        /// <summary>
        /// Should the property be serialized
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeDropDownBackColor()
        {
            return (_dropDownBackColor != Color.Empty);
        }

        /// <summary>
        /// Set/Get the foreground color of the drop down button
        /// </summary>
        [Description("Set/Get the foreground color of the drop down button"),
         Category("Appearance"),
         AmbientValue(typeof(Color), "")]
        public virtual Color DropDownForeColor
        {
            get 
            { 
                if (_dropDownForeColor == Color.Empty) 
                {
                    return (Parent == null) ? SystemColors.ControlText : Parent.ForeColor;
                }
                return _dropDownForeColor; 
            }
            set 
            { 
                _dropDownForeColor = value; 
                UpdateComponentControlColors();
            }
        }

        /// <summary>
        /// Should the property be serialized
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeDropDownForeColor()
        {
            return (_dropDownForeColor != Color.Empty);
        }

        /// <summary>
        /// Set/Get the width of the graphic preview area
        /// </summary>
        [Description("Set/Get the width of the preview area"),
         Category("Layout"),
         DefaultValue(24)]
        public virtual int PreviewWidth
        {
            get { return _previewPanel.Width; }
            set { _previewPanel.Width = value; }
        }

        /// <summary>
        /// Set/Get whether control should display a graphic preview of the current value
        /// </summary>
        [Description("Set/Get whether control should display a graphic preview of the current value"),
        Category("Appearance"),
        DefaultValue(true)]
        public virtual bool ShowPreview
        {
            get { return _showPreview; }
            set 
            { 
                _showPreview = value; 
                PerformLayout();
            }
        }

        /// <summary>
        /// Set/Get whether control should display the text representation of the current value
        /// </summary>
        [Description("Set/Get whether control should display the text representation of the current value"),
        Category("Appearance"),
        DefaultValue(true)]
        public virtual bool ShowText
        {
            get { return _showText; }
            set 
            { 
                _showText = value; 
                PerformLayout();
            }
        }

        /// <summary>
        /// Set/Get whether the control height should be set automatically based on the font height
        /// </summary>
        /// <remarks>
        /// This method overloads the base <see cref="Control.AutoSize"/> property.  The base property
        /// has some severe problems in .NET 2.0 causing the IDE to crash if it is set in the control
        /// constructor.
        /// </remarks>
        [Description("Set/Get whether the control height should be set automatically based on the font height")]
        [Category("Layout")]
        [DefaultValue(true)]
        [Browsable(true)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Visible)]
        public override bool AutoSize
        {
            get { return _autoSize; }
            set 
            { 
                _autoSize = value; 
                SetStyle(ControlStyles.FixedHeight, value);
                PerformLayout();
            }
        }

        /// <summary>
        /// Set/Get the type of the value being edited.  
        /// </summary>
        /// <remarks>
        /// Setting the ValueType sets the <see cref="Editor"/> and <see cref="Converter"/> properties 
        /// to the default for type.  This property allows the client application to set the type of value 
        /// being edited prior to actually setting the value.  This allows the <see cref="Editor"/> and 
        /// <see cref="Converter"/> properties to be set correctly even if the value is null.
        /// </remarks>
        [Description("The type of the value to be edited")]
        [Category("Data")]
        [DefaultValue(null)]
        [Editor("Infralution.Controls.Design.ObjectTypeEditor, " + DesignAssembly.Name, typeof(UITypeEditor))]
        [TypeConverter("Infralution.Controls.Design.ObjectTypeConverter, " + DesignAssembly.Name)]
        public virtual Type ValueType
        {
            get { return _valueType; }
            set
            {
                if (_valueType != value)
                {
                    _valueType = value;
                    if (_useDefaultConverter)
                    {
                        SetDefaultConverter();
                    }
                    if (_useDefaultEditor)
                    {
                        SetDefaultEditor();
                    }
                    PerformLayout();
                }
            }
        }

        /// <summary>
        /// Set/Get whether control should allow null values for value types
        /// </summary>
        /// <remarks>
        /// Reference types typically support null values anyway.  This property allows
        /// the control to return a null value for value types (eg System.Int32)
        /// </remarks>
        [Description("Set/Get whether control should allow null values for value types")]
        [Category("Behavior")]
        [DefaultValue(false)]
        public virtual bool AllowNullValues
        {
            get { return _allowNullValues; }
            set { _allowNullValues = value; }
        }

        /// <summary>
        /// Set/Get the <see cref="UITypeEditor"/> to use to edit the value 
        /// </summary>
        /// <remarks>
        /// This property allows the user to change the editor that is used by the control
        /// from the default <see cref="UITypeEditor"/> associated with the <see cref="ValueType"/>.  
        /// Setting this property sets <see cref="UseDefaultEditor"/> to false.
        /// </remarks>
        [Browsable(false),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public UITypeEditor Editor
        {
            get { return _editor; }
            set
            {
                _useDefaultEditor = false;
                _editor = value;
                PerformLayout();
            }
        }

        /// <summary>
        /// Should the default <see cref="UITypeEditor"/> be used if the the <see cref="ValueType"/>
        /// is changed.
        /// </summary>
        /// <remarks>
        /// Setting this to false allows you to force the use of a given <see cref="Editor"/> regardless
        /// of the actual type of the current value.  
        /// </remarks>
        [Browsable(false),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public bool UseDefaultEditor
        {
            get { return _useDefaultEditor; }
            set { _useDefaultEditor = value; }
        }

        /// <summary>
        /// Set/Get the <see cref="TypeConverter"/> used to convert the value to and from a string
        /// </summary>
        /// <remarks>
        /// This property allows the user to change the converter that is used by the control
        /// from the default <see cref="TypeConverter"/> associated with the <see cref="ValueType"/>.  
        /// Setting this property sets <see cref="UseDefaultConverter"/> to false.
        /// </remarks>
        [Browsable(false),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public TypeConverter Converter
        {
            get { return _converter; }
            set
            {
                if (_converter != value)
                {
                    _useDefaultConverter = false;
                    _converter = value;
                    if (_useDefaultEditor)
                    {
                        SetDefaultEditor();
                    }
                    UpdateStandardValues();
                    PerformLayout();
                }
            }
        }

        /// <summary>
        /// Should the default <see cref="TypeConverter"/> be used if the the <see cref="ValueType"/>
        /// is changed.
        /// </summary>
        /// <remarks>
        /// Setting this to false allows you to force the use of a given <see cref="Converter"/> regardless
        /// of the actual type of the current value.  
        /// </remarks>
        [Browsable(false),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public bool UseDefaultConverter
        {
            get { return _useDefaultConverter; }
            set { _useDefaultConverter = value; }
        }

        /// <summary>
        /// Set/Get the caption to use when displaying text validation errors
        /// </summary>
        [Description("Set/Get the caption to use when displaying text validation errors")]
        [Category("Behavior")]
        [DefaultValue(DefaultTextErrorCaption)]
        [Localizable(true)]
        public virtual string TextErrorCaption
        {
            get { return _textErrorCaption; }
            set { _textErrorCaption = value; }
        }

        /// <summary>
        /// Set/Get the message to use when displaying text validation errors
        /// </summary>
        [Description("Set/Get the message to use when displaying text validation errors")]
        [Category("Behavior")]
        [DefaultValue(DefaultTextErrorMessage)]
        [Localizable(true)]
        public virtual string TextErrorMessage
        {
            get { return _textErrorMessage; }
            set { _textErrorMessage = value; }
        }

        #endregion

        #region Local Methods


        /// <summary>
        /// Handle a change to the application UI culture
        /// </summary>
        /// <param name="newCulture"></param>
        protected virtual void OnApplicationUICultureChanged(CultureInfo newCulture)
        {
            UpdateText();
        }

        /// <summary>
        /// Set the default type converter for the current ValueType
        /// </summary>
        protected virtual void SetDefaultConverter()
        {
            if (_valueType != null)
            {
                Converter = TypeDescriptor.GetConverter(_valueType);
            }
            else
            {
                Converter = null;
            }
        }

        /// <summary>
        /// Set the default editor for the current type and type converter
        /// </summary>
        protected virtual void SetDefaultEditor()
        {
            UITypeEditor editor = null;
            if (_valueType != null)
            {
                editor = (UITypeEditor)TypeDescriptor.GetEditor(_valueType, typeof (UITypeEditor));
            }
            if (editor == null)
            { 
                if (Converter != null && Converter.GetStandardValuesSupported(this))
                {
                    editor = new StandardValueEditor(Converter);
                }
            }
            Editor = editor;
        }

        /// <summary>
        /// The text box used to display and edit text representations of the value
        /// </summary>
        protected TextBox TextBox
        {
            get { return _textBox; }
        }

        /// <summary>
        /// The button used to display the custom editor
        /// </summary>
        protected Button EditButton
        {
            get { return _editButton; }
        }

        /// <summary>
        /// The button used to display drop down lists
        /// </summary>
        protected DropDownButton DropDownButton
        {
            get { return _dropDownButton; }
        }

        /// <summary>
        /// The panel used to display the preview
        /// </summary>
        protected Panel PreviewPanel
        {
            get { return _previewPanel; }
        }

        /// <summary> 
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
                CultureManager.ApplicationUICultureChanged -= new CultureManager.CultureChangedHandler(OnApplicationUICultureChanged); 
                if (components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

        /// <summary>
        /// Handle user click on the drop down button
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnEditButtonClick(object sender, EventArgs e)
        {
            EditValue();
        }

        /// <summary>
        /// Handle single click events from preview panel
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnClick(object sender, EventArgs e)
        {
            TextBox.Focus();
        }

        /// <summary>
        /// Handle double click events from text box and preview panel
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnDoubleClick(object sender, EventArgs e)
        {
            OnDoubleClick(e);
        }

        /// <summary>
        /// Step to the next standard value or display the editor for
        /// types that don't support standard values
        /// </summary>
        /// <param name="e"></param>
        protected override void OnDoubleClick(EventArgs e)
        {
            if (StandardValues != null)
                StepStandardValue(1, true);
            base.OnDoubleClick (e);
        }

        /// <summary>
        /// Use the current editor to change the value
        /// </summary>
        protected virtual void EditValue()
        {
            // validate the text first
            //
            if (_editor != null && ValidateText())
            {
                double interval = DateTime.Now.Subtract(_editTime).TotalMilliseconds;
                if (interval > 200)
                {
                    Value = _editor.EditValue(this, this, Value);
                    _editTime = DateTime.Now;
                }
            }
        }

        /// <summary>
        /// Handle painting the preview
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnPreviewPaint(object sender, PaintEventArgs e)
        {
            if (PreviewSupported)
            {
                const int border = 2;
                Rectangle bounds = new Rectangle(border, border, 
                                                _previewPanel.Width - 2 * border,
                                                _previewPanel.Height - 2 * border);
                PaintValueEventArgs pve = new PaintValueEventArgs(this, _value, e.Graphics, bounds);
                _editor.PaintValue(pve);
            }
        }

        /// <summary>
        /// Return true if the active editor supports preview
        /// </summary>
        protected virtual bool PreviewSupported
        {
            get { return (_editor != null && _editor.GetPaintValueSupported(this)); }
        }

        /// <summary>
        /// Return true if the active editor supports editing
        /// </summary>
        protected virtual bool EditValueSupported
        {
            get { return (_editor != null && _editor.GetEditStyle(this) != UITypeEditorEditStyle.None); }
        }
        
        /// <summary>
        /// Return true if the text displayed can be edited directly by the user
        /// </summary>
        protected virtual bool TextEditable
        {
            get 
            { 
                if (!ShowText || 
                    Converter == null || 
                    !Converter.CanConvertFrom(this, typeof(string)) ||
                    Converter.GetStandardValuesExclusive(this)) 
                {
                    return false;
                } 
                return true;
            }
        }

        /// <summary>
        /// Return the preferred height for the control client area based on the font height
        /// </summary>
        protected virtual int PreferredHeight
        {
            get 
            {
                int border = 0;
                switch (BorderStyle)
                {
                    case BorderStyle.None:
                        border = 0;
                        break;
                    case BorderStyle.FixedSingle:
                    case BorderStyle.Fixed3D:
                        border = 7;
                        break;
                }
                return Font.Height + border; 
            }
        }

        /// <summary>
        /// Return the text representation of the value
        /// </summary>
        /// <param name="value">The value to convert to text</param>
        /// <returns>The text representation of the value</returns>
        protected virtual string GetTextForValue(object value)
        {
            try 
            {
                if (Converter != null && Converter.CanConvertTo(this, typeof(string)))
                    return Converter.ConvertToString(this, value);
            } 
            catch 
            {
            }
            // fallback to the default to string operator
            //
            return (value == null) ? string.Empty : value.ToString();
        }

        /// <summary>
        /// The standard values (if supported) for the current type converter
        /// </summary>
        protected object[] StandardValues 
        {
            get { return _standardValues; }
        }

        /// <summary>
        /// Update the standard values when the type converter is changed
        /// </summary>
        protected virtual void UpdateStandardValues()
        {
            if (Converter != null && Converter.GetStandardValuesSupported(this)) 
            {
                ICollection values = Converter.GetStandardValues();
                _standardValues = new Object[values.Count];
                values.CopyTo(_standardValues, 0);
            }
            else
            {
                _standardValues = null;
            }
        }


        /// <summary>
        /// Select the next standard value
        /// </summary>
        /// <param name="step">The step (direction) to move</param>
        /// <param name="circular">
        /// Determines if stepping past the end or beginning of the list returns to the other
        /// end of the list
        /// </param>
        protected virtual void StepStandardValue(int step, bool circular)
        {
            object[] values = StandardValues;
            if (values == null) return;
            int index = (values as IList).IndexOf(Value);
            if (index < 0)
            {
                index = 0;
            }
            else
            {
                index += step;
                if (index < 0)
                {
                    index = (circular) ? values.Length - 1 : 0;
                }
                else if (index >= values.Length)
                {
                    index = (circular) ? 0 : values.Length - 1;
                }
            }
            Value = values[index];
            TextBox.SelectAll();
        }

        /// <summary>
        /// Update the text displayed in the text box 
        /// </summary>
        protected virtual void UpdateText()
        {
            TextBox.Text = GetTextForValue(Value);
        }

        /// <summary>
        /// Validate the user entered text and set the Value property
        /// </summary>
        /// <returns>True if the text was valid and Value was set</returns>
        protected virtual bool ValidateText()
        {
            if (TextEditable)
            {
                try 
                {
                   if (AllowNullValues && string.IsNullOrEmpty(TextBox.Text))
                    {
                        Value = null;
                    }
                    else
                    {
                        Value = Converter.ConvertFromString(this, CultureInfo.CurrentCulture, TextBox.Text);
                    }
                } 
                catch (Exception e)
                {
                    return HandleTextConversionError(e);
                }
            }
            return true;
        }

        /// <summary>
        /// Handle an exception while converting the text value to the typed value
        /// </summary>
        /// <remarks>
        /// Calls OnValidateTextError and then depending on the result displays the
        /// standard error message.
        /// </remarks>
        /// <param name="e">The exception</param>
        /// <returns>True if the value is reset otherwise false</returns>
        protected virtual bool HandleTextConversionError(Exception e)
        {
            ValidateTextErrorEventArgs args = new ValidateTextErrorEventArgs(TextBox.Text, e);
            OnValidateTextError(args);
            if (!args.Handled)
            {
                string typeName = (_valueType == null) ? null : _valueType.Name;
                string msg = String.Format(TextErrorMessage, TextBox.Text, typeName);
                DialogResult result = MessageBox.Show(this, msg, TextErrorCaption, 
                                                      MessageBoxButtons.OKCancel, 
                                                      MessageBoxIcon.Exclamation);
                if (result == DialogResult.Cancel)
                {
                    args.ResetText = true;
                }
            }
            if (args.ResetText)
            {
                CancelTextEntry();
            }
            return args.ResetText;
        }

        /// <summary>
        /// Handle an error while validating the user entered text.  Raises the
        /// ValidateTextError event.  
        /// </summary>
        /// <param name="args">The event args</param>
        protected virtual void OnValidateTextError(ValidateTextErrorEventArgs args)
        {
            if (ValidateTextError != null)
            {
                 ValidateTextError(this, args);
            }
        }

        /// <summary>
        /// Cancel any changes made to the user entered text
        /// </summary>
        protected virtual void CancelTextEntry()
        {
            UpdateText();
        }

        /// <summary>
        /// Validate the currently displayed text
        /// </summary>
        /// <param name="e"></param>
        protected override void OnValidating(CancelEventArgs e)
        {
            if (TextEditable)
            {
                e.Cancel = !ValidateText();
            }
            base.OnValidating (e);
        }

        /// <summary>
        /// Handle TextBox KeyDown events to navigate standard values using arrow keys
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void OnTextKeyDown(object sender, KeyEventArgs e)
        {
            OnKeyDown(e);
        }

        /// <summary>
        /// Handle KeyDown events to navigate standard values using arrow keys
        /// </summary>
        /// <param name="e"></param>
        protected override void OnKeyDown(KeyEventArgs e)
        {
            e.Handled = true;
            switch (e.KeyCode)
            {
                case Keys.Enter:
                    ValidateText();
                    break;
                case Keys.Escape:
                    CancelTextEntry();
                    break;
                case Keys.Down:
                    if (StandardValues == null)
                        EditValue();
                    else
                        StepStandardValue(1, false);
                    break;
                case Keys.Up:
                    StepStandardValue(-1, false);
                    break;
                case Keys.PageDown:
                    EditValue();
                    break;
                default:
                    e.Handled = false;
                    break;
            }
            base.OnKeyDown (e);
        }

        /// <summary>
        /// Handles Shift-Tab Navigation
        /// </summary>
         /// <param name="keyData"></param>
        /// <returns></returns>
        /// <remarks>
        /// The text box used to display the current selection interferes with
        /// normal handling of Shift-Tab.  This method fixes this.
        /// </remarks>
        protected override bool ProcessDialogKey(Keys keyData)
        {
            switch (keyData)
            {
                case Keys.Tab | Keys.Shift:
                    return Parent.SelectNextControl(this, false, true, true, true);
            }
            return base.ProcessDialogKey (keyData);
        }

        /// <summary>
        /// Ensure initial focus is given to TextBox
        /// </summary>
        /// <param name="e"></param>
        protected override void OnGotFocus(EventArgs e)
        {
            base.OnGotFocus (e);
            TextBox.Focus();
        }

        /// <summary>
        /// Handle text box getting focus directly
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void _textBox_GotFocus(object sender, EventArgs e)
        {
            base.OnGotFocus(e);
        }

        /// <summary>
        /// PerformLayout when the font changes to ensure AutoSize height is updated
        /// </summary>
        /// <param name="e"></param>
        protected override void OnFontChanged(EventArgs e)
        {
            PerformLayout();
            base.OnFontChanged(e);
        }

        /// <summary>
        /// Update the back and fore colors of component controls when a
        /// color property changes
        /// </summary>
        protected virtual void UpdateComponentControlColors()
        {
            _textBox.BackColor = BackColor;
            _textBox.ForeColor = ForeColor;
            _previewPanel.BackColor = BackColor;
            _editButton.BackColor = DropDownBackColor;
            _editButton.ForeColor = DropDownForeColor;
            _dropDownButton.BackColor = DropDownBackColor;
            _dropDownButton.ForeColor = DropDownForeColor;
            if (_dropDownForm != null)
            {
                _dropDownForm.BackColor = BackColor;
                _dropDownForm.ForeColor = ForeColor;
            }
        }

        /// <summary>
        /// Update component controls when parent color changes
        /// </summary>
        /// <param name="e"></param>
        protected override void OnParentChanged(EventArgs e)
        {
            base.OnParentChanged (e);
            UpdateComponentControlColors();
        }

        /// <summary>
        /// Update component controls when parent color changes
        /// </summary>
        /// <param name="e"></param>
        protected override void OnParentBackColorChanged(EventArgs e)
        {
            base.OnParentBackColorChanged (e);
            UpdateComponentControlColors();
        }

        /// <summary>
        /// Update component controls when parent color changes
        /// </summary>
        /// <param name="e"></param>
        protected override void OnParentForeColorChanged(EventArgs e)
        {
            base.OnParentForeColorChanged (e);
            UpdateComponentControlColors();
        }

        /// <summary>
        /// Update the layout when RightToLeft changes
        /// </summary>
        /// <param name="e"></param>
        protected override void OnRightToLeftChanged(EventArgs e)
        {
            base.OnRightToLeftChanged (e);
            PerformLayout();
        }


        /// <summary>
        /// Handle layout of textbox and button
        /// </summary>
        /// <param name="levent"></param>
        protected override void OnLayout(LayoutEventArgs levent)
        {
            if (this.RightToLeft == RightToLeft.No)
            {
                _editButton.Dock = DockStyle.Right;
                _dropDownButton.Dock = DockStyle.Right;
                _previewPanel.Dock = (ShowText) ? DockStyle.Left : DockStyle.Fill;
            }
            else
            {
                _editButton.Dock = DockStyle.Left;
                _dropDownButton.Dock = DockStyle.Left;
                _previewPanel.Dock = (ShowText) ? DockStyle.Right : DockStyle.Fill;
            }
            _textBox.Visible = ShowText;
            _textBox.ReadOnly = !TextEditable;
            bool previewVisible = ShowPreview && PreviewSupported;
            _previewPanel.Visible = previewVisible;

            UITypeEditorEditStyle style = (_editor == null) ?
                UITypeEditorEditStyle.None : _editor.GetEditStyle(this);
            _editButton.Visible = (style == UITypeEditorEditStyle.Modal);
            _dropDownButton.Visible = (style == UITypeEditorEditStyle.DropDown);

            if (_autoSize)
            {
                this.Height = PreferredHeight;
            }
            base.OnLayout (levent);

            const int textOffset = 2;
            _textBox.SuspendLayout();
            _textBox.Top = (ClientSize.Height - _textBox.Height) / 2;
            int width = ClientSize.Width - textOffset;
            if (style != UITypeEditorEditStyle.None)
                width -= _dropDownButton.Width;
            if (previewVisible)
            {
                width -= _previewPanel.Width;
            }
            _textBox.Width = width;
            if (this.RightToLeft == RightToLeft.No)
            {
                if (previewVisible)
                    _textBox.Left = _previewPanel.Width + textOffset;
                else
                    _textBox.Left = textOffset;
            }
            else
            {
                _textBox.Left = (style == UITypeEditorEditStyle.None) ? 0 : _dropDownButton.Width;
            }
            _textBox.ResumeLayout();
        }

        /// <summary>
        /// Raises the ValueChanged event
        /// </summary>
        protected virtual void OnValueChanged()
        {
            if (ValueChanged != null)
            {
                ValueChanged(this, new EventArgs());
            }
        }

        #endregion

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(UniversalEditBox));
            this._textBox = new System.Windows.Forms.TextBox();
            this._editButton = new System.Windows.Forms.Button();
            this._previewPanel = new System.Windows.Forms.Panel();
            this._dropDownButton = new Infralution.Controls.DropDownButton();
            this.SuspendLayout();
            // 
            // _textBox
            // 
            this._textBox.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this._textBox.Location = new System.Drawing.Point(24, 0);
            this._textBox.Name = "_textBox";
            this._textBox.Size = new System.Drawing.Size(132, 13);
            this._textBox.TabIndex = 0;
            this._textBox.TabStop = false;
            this._textBox.DoubleClick += new System.EventHandler(this.OnDoubleClick);
            this._textBox.GotFocus += new System.EventHandler(this._textBox_GotFocus);
            this._textBox.KeyDown += new System.Windows.Forms.KeyEventHandler(this.OnTextKeyDown);
            // 
            // _editButton
            // 
            this._editButton.BackColor = System.Drawing.SystemColors.Control;
            this._editButton.Dock = System.Windows.Forms.DockStyle.Right;
            this._editButton.ForeColor = System.Drawing.SystemColors.ControlText;
            this._editButton.Image = ((System.Drawing.Image)(resources.GetObject("_editButton.Image")));
            this._editButton.ImageAlign = System.Drawing.ContentAlignment.BottomCenter;
            this._editButton.Location = new System.Drawing.Point(158, 0);
            this._editButton.Name = "_editButton";
            this._editButton.Size = new System.Drawing.Size(18, 22);
            this._editButton.TabIndex = 1;
            this._editButton.TabStop = false;
            this._editButton.UseVisualStyleBackColor = false;
            this._editButton.Visible = false;
            this._editButton.Click += new System.EventHandler(this.OnEditButtonClick);
            // 
            // _previewPanel
            // 
            this._previewPanel.BackColor = System.Drawing.SystemColors.Window;
            this._previewPanel.Dock = System.Windows.Forms.DockStyle.Left;
            this._previewPanel.ForeColor = System.Drawing.SystemColors.WindowText;
            this._previewPanel.Location = new System.Drawing.Point(0, 0);
            this._previewPanel.Name = "_previewPanel";
            this._previewPanel.Size = new System.Drawing.Size(24, 22);
            this._previewPanel.TabIndex = 2;
            this._previewPanel.DoubleClick += new System.EventHandler(this.OnDoubleClick);
            this._previewPanel.Click += new System.EventHandler(this.OnClick);
            this._previewPanel.Paint += new System.Windows.Forms.PaintEventHandler(this.OnPreviewPaint);
            // 
            // _dropDownButton
            // 
            this._dropDownButton.Dock = System.Windows.Forms.DockStyle.Right;
            this._dropDownButton.Image = ((System.Drawing.Image)(resources.GetObject("_dropDownButton.Image")));
            this._dropDownButton.Location = new System.Drawing.Point(176, 0);
            this._dropDownButton.Name = "_dropDownButton";
            this._dropDownButton.Size = new System.Drawing.Size(17, 22);
            this._dropDownButton.TabIndex = 0;
            this._dropDownButton.TabStop = false;
            this._dropDownButton.Visible = false;
            this._dropDownButton.Click += new System.EventHandler(this.OnEditButtonClick);
            // 
            // UniversalEditBox
            // 
            this.Controls.Add(this._editButton);
            this.Controls.Add(this._dropDownButton);
            this.Controls.Add(this._previewPanel);
            this.Controls.Add(this._textBox);
            this.Size = new System.Drawing.Size(195, 24);
            this.ResumeLayout(false);
            this.PerformLayout();

        }
		#endregion

        #region IServiceProvider Members

        /// <summary>
        /// Provides only a single service - the IWindowsFormsEditorService 
        /// </summary>
        /// <param name="serviceType"></param>
        /// <returns></returns>
        object IServiceProvider.GetService(Type serviceType)
        {
            if (serviceType == typeof(IWindowsFormsEditorService))
            {
                return this;
            }
            return null;
        }

        #endregion

        #region IWindowsFormsEditorService Members

        /// <summary>
        /// Drop down an editor control for the cell
        /// </summary>
        /// <param name="control">The control to be dropped</param>
        void IWindowsFormsEditorService.DropDownControl(Control control)
        {

            DropDownForm form = new DropDownForm();
            form.ManageContainedControlDisposal = false;
            form.BackColor = BackColor;
            form.ForeColor = ForeColor;

            // force control creation to ensure default width/height are set
            //         
            control.CreateControl();

            // set the width/height of the form - handle DateTimeUI as special case because it is not very
            // well behaved when it comes to resizing
            //
            if (control.GetType().Name == "DateTimeUI")
                form.Width = control.Width + 4;
            else
                form.Width = Math.Max(Width, control.Width + 4);
            form.Height = control.Height + 4;

            form.ContainedControl = control;
            _dropDownForm = form;
            form.ShowModal(this);
            _dropDownForm = null;
            form.Close();
            form.Dispose();
        }

        /// <summary>
        /// Close the current editor drop down (if any)
        /// </summary>
        void IWindowsFormsEditorService.CloseDropDown()
        {
            if (_dropDownForm != null)
            {
                _dropDownForm.Hide();
            }
        }

        /// <summary>
        /// Show an editor dialog for the control
        /// </summary>
        /// <param name="dialog">The dialog to show</param>
        /// <returns>The dialog result</returns>
        DialogResult IWindowsFormsEditorService.ShowDialog(Form dialog)
        {
            return dialog.ShowDialog();
        }

        #endregion

        #region ITypeDescriptorContext Members

        /// <summary>
        /// Return the object that owns the value being edited
        /// </summary>
        object ITypeDescriptorContext.Instance
        {
            get { return _valueOwner; }
        }

        /// <summary>
        /// Returns null always
        /// </summary>
        PropertyDescriptor ITypeDescriptorContext.PropertyDescriptor
        {
            get {  return null;  }
        }

        /// <summary>
        /// Returns the container for the sited control
        /// </summary>
        IContainer ITypeDescriptorContext.Container
        {
            get
            {
                ISite site = this.Site;
                if (site != null)
                {
                    return site.Container;
                }
                return null;
            }
        }

        /// <summary>
        /// Not used at runtime.
        /// </summary>
        void ITypeDescriptorContext.OnComponentChanged()
        {
        }

        /// <summary>
        /// Always returns true.
        /// </summary>
        bool ITypeDescriptorContext.OnComponentChanging()
        {
            return true;
        }
 
        #endregion

    }
}
