#region File Header
//
//      FILE:   VirtualDropDownList.cs.
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
using System.Windows.Forms;

namespace Infralution.Controls
{
    /// <summary>
    /// Defines a simple data bound drop down list that provides true virtual loading of items.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The standard .NET ComboBox control performs very badly when bound to large data sources
    /// because it creates a display representation for every item in the bound data
    /// source upfront.  This control provides an alternative implementation that uses 
    /// <see cref="VirtualListBox"/> to provide true virtual loading of data sources that support the IList interface
    /// </para>
    /// <para>
    /// The <see cref="DisplayMember"/> property can be set to determine the field or property of
    /// the displayed items that is displayed in the drop down list.
    /// </para>
    /// </remarks>
    /// <seealso cref="VirtualListBox"/>
    [ToolboxItem(true)]
#if CHECK_LICENSE
    [LicenseProvider(typeof(ControlLicenseProvider))]
#endif
    public class VirtualDropDownList : BorderedControl
    {
        #region Member Variables

        /// <summary>
        /// The currently selected item
        /// </summary>
        private object _selectedItem;

        /// <summary>
        /// Is selection bound to the currency manager.
        /// </summary>
        private bool _useCurrencyManager = true;

        /// <summary>
        /// Should the control height be set automatically
        /// </summary>
        private bool _autoSize = true;

        /// <summary>
        /// The color to use for the background of the drop down button and form
        /// </summary>
        private Color _dropDownBackColor = Color.Empty;

        /// <summary>
        /// The color to use for the foreground of the drop down button and form
        /// </summary>
        private Color _dropDownForeColor = Color.Empty;
       
        /// <summary>
        /// The text box used for text entry
        /// </summary>
        private TextBox _textBox;
   
        /// <summary>
        /// The button used to drop down lists
        /// </summary>
        private DropDownButton _dropDownButton;

        /// <summary> 
        /// Required designer variable.
        /// </summary>
        private Container components = null;
 
        /// <summary>
        ///  The current datasource for the control.
        /// </summary>
        private object _dataSource = null;

        /// <summary>
        /// The property or field to display
        /// </summary>
        private string _displayMember;

        /// <summary>
        /// The type of the item that the displayMemberPropertyDescriptor invokes
        /// </summary>
        private Type _displayMemberType;

        /// <summary>
        /// The cached property descriptor for the display member
        /// </summary>
        private PropertyDescriptor _displayMemberPropertyDescriptor;

        /// <summary>
        /// Should the selected item be constrained to belong to the datasource
        /// </summary>
        private bool _constrainToList = true;

        /// <summary>
        /// The active dropdown form
        /// </summary>
        private DropDownForm _dropDownForm;

        /// <summary>
        /// The active dropdown list
        /// </summary>
        private VirtualListBox _dropDownList;

        /// <summary>
        /// The maximum number of drop down items to display
        /// </summary>
        private int _maxDropDownItems = 12;

        /// <summary>
        /// Time the drop down form was last displayed - used to prevent the clicking on the drop
        /// down button while list is displayed from reactivating list
        /// </summary>
        private DateTime _dropDownTime;

        /// <summary>
        /// The currency manager for the data source
        /// </summary>
        private CurrencyManager _currencyManager;

        /// <summary>
        /// The underlying list of items to display
        /// </summary>
        private IList _items;

        #endregion

        #region Events

        /// <summary>
        /// Raised by the VirtualDropDownList to obtain the display text for a given item.  
        /// Handle this event if you wish to programmatically determine the text for a given item
        /// rather than using standard data binding
        /// </summary>
        [Category("Data"),
        Description("Fired to obtain the text to display for a given item")]
        public event GetItemTextHandler GetItemText;

        /// <summary>
        /// Fired when the <see cref="SelectedItem"/> is changed
        /// </summary>
        [Category("Property Changed"),
        Description("Event fired when the SelectedItem is changed")]
        public event EventHandler SelectedItemChanged;

        #endregion

        #region Public Interface

        #region Initialization

        /// <summary>
        /// Create a new instance of the control
        /// </summary>
        public VirtualDropDownList()
        {
            // This call is required by the Windows.Forms Form Designer.
            InitializeComponent();

            // Set the text box to readonly - then reset the text box colors
            // so that it doesn't appear disabled
            //
            this._textBox.ReadOnly = true;
            this._textBox.BackColor = SystemColors.Window;
            this._textBox.ForeColor = SystemColors.WindowText;
 
            #if CHECK_LICENSE
            ControlLicenseProvider.CheckLicense(ReflectionUtilities.GetInstantiatingAssembly() ,typeof(VirtualDropDownList), this);
            #endif
        }

        #endregion

        #region Hidden Properties

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

        #endregion

        #region Appearance

        /// <summary>
        /// Set/Get the background color of the control
        /// </summary>
        [Description("Set/Get the background color for the control"),
        Category("Appearance"),
        DefaultValue(typeof(Color), "Window")]
        public override Color BackColor
        {
            get { return _textBox.BackColor; }
            set 
            { 
                _textBox.BackColor = value;
                UpdateComponentControlColors();
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
            get { return _textBox.ForeColor; }
            set 
            { 
                _textBox.ForeColor = value;
                UpdateComponentControlColors();
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

        #endregion

        #region Layout

        /// <summary>
        /// The maximum number of items to display in the drop down list
        /// </summary>
        [Description("The maximum number of items to display in the drop down list"),
        Category("Behavior"),
        DefaultValue(12)]
        public virtual int MaxDropDownItems
        {
            get { return _maxDropDownItems; }
            set 
            {
                if (value < 1) throw new ArgumentOutOfRangeException("value", "MaxDropDownItems must be greater than zero");
                if (value > 50) throw new ArgumentOutOfRangeException("value", "MaxDropDownItems must be less than 50");
                _maxDropDownItems = value; 
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
        public virtual new bool AutoSize
        {
            get { return _autoSize; }
            set 
            { 
                _autoSize = value; 
                SetStyle(ControlStyles.FixedHeight, value);
                PerformLayout();
            }
        }

        #endregion

        #region Data Binding

        /// <summary>
        /// Get or set the data source.  
        ///  </summary>
        [Category("Data")]
        [DefaultValue(null)]
        [Description("The object supplying the data to be displayed - this may be a DataTable, DataView or any object which implements IList")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [TypeConverter("System.Windows.Forms.Design.DataSourceConverter, System.Design")]
        [Editor("System.Windows.Forms.Design.DataSourceListEditor, System.Design", typeof(UITypeEditor))]
        public virtual object DataSource
        {
            get 
            { 
                return _dataSource;
            }
            set 
            { 
                if (_dataSource != value)
                {
                    UnbindDataSource();
                    UnbindCurrencyManager();
                    _dataSource = value;
                    
                    // force cached property descriptor to be recalculated
                    _displayMemberType = null;
                    _displayMemberPropertyDescriptor = null;

                    BindDataSource();
                    UnbindCurrencyManager();
                    UpdateData();
                }
            }
        }


        /// <summary>
        /// Updates the displayed data following a changed to the <see cref="DataSource"/>
        /// </summary>
        /// <remarks>
        /// This method is called automatically if the <see cref="DataSource"/> supports
        /// the <see cref="IBindingList"/> notification interface.  If the data source does
        /// not support <see cref="IBindingList"/> (eg <see cref="ArrayList"/>) then you can 
        /// call this method to update the ListBox after making adding or removing items from
        /// the data source.
        /// </remarks>
        public virtual void UpdateData()
        {
            if (ConstrainToList && _items != null)
            {
                if (!_items.Contains(SelectedItem))
                {
                    SelectedItem = (_items.Count > 0) ? _items[0] : null;
                }
                else
                {
                    UpdateText();
                }
            }
        }

        /// <summary>
        /// Defines the property or field of the data source that you wish to be displayed
        /// </summary>
        [Category("Data"),
        DefaultValue(null), 
        Description("Defines the property or field of the data source that you wish to be displayed"),
        RefreshProperties(RefreshProperties.Repaint), 
        TypeConverter("System.Windows.Forms.Design.DataMemberFieldConverter, System.Design"),
        Editor("System.Windows.Forms.Design.DataMemberFieldEditor, System.Design", typeof(UITypeEditor))] 
        public string DisplayMember
        {
            get { return _displayMember; }
            set 
            {
                if (_displayMember != value)
                {
                    _displayMember = value;
                    
                    // force cached property descriptor to be recalculated
                    _displayMemberType = null;
                    _displayMemberPropertyDescriptor = null;
                    PerformLayout();
                }
            }
        }

        /// <summary>
        /// Determines whether the selection is bound to the CurrencyManager.
        /// </summary>
        /// <remarks>
        /// The <see cref="CurrencyManager"/> mechanism is useful where you want to 
        /// automatically link selection between a number of controls on a form.   It can cause
        /// unwanted behaviour however.   For instance changing the order of items in a bound list
        /// will cause selection to change if this property is set to true.
        /// </remarks>
        [Category("Behavior")]
        [Description("Determines whether selection is bound to the CurrencyManager")]
        [DefaultValue(true)]
        public virtual bool UseCurrencyManager
        {
            get { return _useCurrencyManager; }
            set { _useCurrencyManager = value; }
        }

        /// <summary>
        /// Set/Get the selected item.  
        /// </summary>
        [Browsable(false),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual object SelectedItem
        {
            get { return _selectedItem; }
            set 
            {
                bool allowChange = (_selectedItem != value);
                 if (ConstrainToList && _items != null)
                {
                    if (_items.Count == 0)
                    {
                        if (value != null)
                            allowChange = false;
                    }
                    else if (!_items.Contains(value))
                        allowChange = false;
                }
                
                if (allowChange)
                {
                    _selectedItem = value;
                    UpdateText();
                    OnSelectedItemChanged();
                }
            }
        }

        /// <summary>
        /// Should the <see cref="SelectedItem"/> be constrained to belong to the list
        /// </summary>
        [Category("Behavior"),
        DefaultValue(true),
        Description("Should the SelectedItem be constrained to belong to the list")] 
        public bool ConstrainToList
        {
            get { return _constrainToList; }
            set 
            {
                if (_constrainToList != value)
                {
                    _constrainToList = value;
                    UpdateData();
                }
            }
        }


        #endregion

        #endregion

        #region Local Methods

        /// <summary>
        /// The underlying list of items the ListBox is bound to
        /// </summary>
        protected IList Items
        {
            get { return _items; }
        }

        /// <summary>
        /// Bind to the current datasource
        /// </summary>
        protected void BindDataSource()
        {
            if (_dataSource is IList)
            {
                _items = _dataSource as IList;
            }
            else if (_dataSource is IListSource)
            {
                _items = (_dataSource as IListSource).GetList();
            }
            else
            {
                _items = null;
            }

            // attach binding events
            //
            if (_items is IBindingList)
            {
                (_items as IBindingList).ListChanged += new ListChangedEventHandler(OnDataSourceChanged);
            }

        }

        /// <summary>
        /// Unbind from the data source events
        /// </summary>
        protected virtual void UnbindDataSource()
        {
            if (_items is IBindingList)
            {
                (_items as IBindingList).ListChanged -= new ListChangedEventHandler(OnDataSourceChanged);
            }
        }

        /// <summary>
        /// Bind to the current CurrencyManager for the data source
        /// </summary>
        protected virtual void BindCurrencyManager()
        {
            if (UseCurrencyManager)
            {
                if (BindingContext != null && _dataSource != null)
                {
                    _currencyManager = (CurrencyManager)BindingContext[_dataSource];
                }
                else
                {
                    _currencyManager = null;
                }
                if (_currencyManager != null)
                {
                    _currencyManager.CurrentChanged += new EventHandler(OnCurrencyManagerCurrentChanged);
                }
            }
        }

        /// <summary>
        /// Unbind from the current currency manager for the datasource
        /// </summary>
        protected virtual void UnbindCurrencyManager()
        {
            if (_currencyManager != null)
            {
                _currencyManager.CurrentChanged -= new EventHandler(OnCurrencyManagerCurrentChanged);
            }
        }

        /// <summary>
        /// The text box used to display text representations of the selected item
        /// </summary>
        protected TextBox TextBox
        {
            get { return _textBox; }
        }

        /// <summary>
        /// The button used to display drop down lists
        /// </summary>
        protected DropDownButton DropDownButton
        {
            get { return _dropDownButton; }
        }

        /// <summary> 
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose( bool disposing )
        {
            if( disposing )
            {
                if (_dropDownForm != null)
                {
                    _dropDownForm.Dispose();
                }
                UnbindDataSource();
                _dataSource = null;
                UnbindCurrencyManager();
                _currencyManager = null;
                if(components != null)
                {
                    components.Dispose();
                }
            }
            base.Dispose( disposing );
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
        /// Update the text displayed in the text box 
        /// </summary>
        protected virtual void UpdateText()
        {
            TextBox.Text = GetTextForItem(SelectedItem);
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
        /// Handles selection of values using the up down arrows and shift
        /// tab navigation.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnKeyDown(KeyEventArgs e)
        {
            switch (e.KeyCode)
            {
                case Keys.Down:
                    SelectNextItem();
                    e.Handled = true;
                    break;
                case Keys.Up:
                    SelectPrevItem();
                    e.Handled = true;
                    break;
            }
            base.OnKeyDown (e);
        }

        /// <summary>
        /// Step to the next item in the data source following the selected item
        /// </summary>
        protected virtual void SelectNextItem()
        {
            if (_items != null)
            {
                int index = _items.IndexOf(SelectedItem);
                if (index < 0) index = -1;
                index++;
                if (index < _items.Count)
                {
                    SelectedItem = _items[index];
                    TextBox.SelectAll();
                }
            }
        }

        /// <summary>
        /// Step to the previous item in the data source before the selected item
        /// </summary>
        protected virtual void SelectPrevItem()
        {
            if (_items != null)
            {
                int index = _items.IndexOf(SelectedItem);
                if (index > 0)
                {
                    SelectedItem = _items[index-1];
                    TextBox.SelectAll();
                }
            }
        }

        /// <summary>
        /// Handle a click on the drop down button
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnDropDownButtonClick(object sender, EventArgs e)
        {
            ShowDropDown();
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
            _dropDownButton.BackColor = DropDownBackColor;
            _dropDownButton.ForeColor = DropDownForeColor;
        }

        /// <summary>
        /// Update component controls when parent color changes
        /// </summary>
        /// <param name="e"></param>
        protected override void OnParentChanged(EventArgs e)
        {
            base.OnParentChanged (e);
            UnbindCurrencyManager();
            BindCurrencyManager();
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
        /// Handle layout of textbox and button
        /// </summary>
        /// <param name="levent"></param>
        protected override void OnLayout(LayoutEventArgs levent)
        {
            if (this.RightToLeft == RightToLeft.No)
            {
                _dropDownButton.Dock = DockStyle.Right;
            }
            else
            {
                _dropDownButton.Dock = DockStyle.Left;
            }
            if (_autoSize)
            {
                this.Height = PreferredHeight;
            }
            base.OnLayout (levent);

            const int textOffset = 2;
            _textBox.SuspendLayout();
            _textBox.Top = (ClientSize.Height - _textBox.Height) / 2;
            _textBox.Width = ClientSize.Width - _dropDownButton.Width - textOffset;
            if (this.RightToLeft == RightToLeft.No)
            {
                _textBox.Left = textOffset;
            }
            else
            {
                _textBox.Left = _dropDownButton.Width;
            }
            _textBox.ResumeLayout();

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
        /// Raises the SelectedItemChanged event
        /// </summary>
        protected virtual void OnSelectedItemChanged()
        {
            if (_currencyManager != null)
            {
                _currencyManager.Position = _items.IndexOf(SelectedItem);               
            }
            if (SelectedItemChanged != null)
            {
                SelectedItemChanged(this, new EventArgs());
            }
        }

        /// <summary>
        /// Handle a ListChanged event from the current <see cref="DataSource"/>
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnDataSourceChanged(object sender, ListChangedEventArgs e)
        {
            UpdateData();
        }

        /// <summary>
        /// Handle a change to the currency manager item
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected void OnCurrencyManagerCurrentChanged(object sender, EventArgs e)
        {
            SelectedItem = _currencyManager.Current;
        }

        /// <summary>
        /// The active drop down form (if any)
        /// </summary>
        protected DropDownForm DropDownForm
        {
            get { return _dropDownForm; }
        }

        /// <summary>
        /// The active drop down list box (if any)
        /// </summary>
        protected VirtualListBox DropDownList
        {
            get { return _dropDownList; }
        }

        /// <summary>
        /// Show the drop down list
        /// </summary>
        protected virtual void ShowDropDown()
        {
            if (_dropDownForm != null) return;

            // check that the user isn't clicking on the dropdown arrow to get
            // rid of the drop down list
            //
            double interval = DateTime.Now.Subtract(_dropDownTime).TotalMilliseconds;
            if (interval < 200) return;

            VirtualListBox listBox = new VirtualListBox();
            listBox.BeginInit();
            listBox.AllowMultiSelect = false;
            listBox.BorderStyle = BorderStyle.None;
            listBox.MouseUp += new MouseEventHandler(OnListMouseUp);
            listBox.KeyDown += new KeyEventHandler(OnListKeyDown);
            listBox.GetItemText += new GetItemTextHandler(OnListGetItemText);
            
            listBox.DataSource = DataSource;
            listBox.EndInit();

            // determine the number of dropdown items to display and the item to select

            object selectedItem = SelectedItem;
            int dropDownItems = 1; 
            if (_items != null)
            {
                dropDownItems = _items.Count;
                if (dropDownItems < 1) dropDownItems = 1;
                if (dropDownItems > MaxDropDownItems) dropDownItems = MaxDropDownItems;
                if (!_items.Contains(selectedItem))
                {
                    if (_items.Count > 0)
                    {
                        selectedItem = _items[0];
                    }
                }
            }

            DropDownForm form = new DropDownForm();
            form.ManageContainedControlDisposal = true;
            form.BackColor = BackColor;
            form.ForeColor = ForeColor;
            form.ContainedControl = listBox;           
            form.Height = listBox.IntegralHeightForRows(dropDownItems) + 4;
            form.Width = Width;

            // don't set the selected item until we have sized the form because
            // otherwise the list can't ensure that the selected item is visible
            //
            listBox.SelectedItem = selectedItem;

            _dropDownForm = form;
            _dropDownList = listBox;
            form.ShowModal(this);
            _dropDownForm = null;
            _dropDownList = null;
            form.Close();
            _dropDownTime = DateTime.Now;
        }

        /// <summary>
        /// Close the active drop down list (if active)
        /// </summary>
        protected virtual void CloseDropDown()
        {
            if (_dropDownForm != null)
            {
                _dropDownForm.Hide();
            }
        }

        /// <summary>
        /// Handle mouse up events for the list
        /// </summary>
        /// <remarks>
        /// Sets the value of the object being edited and closes the drop down
        /// </remarks>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnListMouseUp(object sender, MouseEventArgs e)
        {
            SelectedItem = DropDownList.SelectedItem;
            CloseDropDown();
        }

        /// <summary>
        /// Handle Enter and Escape keys for the list
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnListKeyDown(object sender, KeyEventArgs e)
        {
            switch(e.KeyCode)
            {
                case Keys.Enter:
                    SelectedItem = DropDownList.SelectedItem;
                    CloseDropDown();
                    break;
                case Keys.Escape:
                    CloseDropDown();
                    break;
            }
        }

        /// <summary>
        /// Return the property descriptor to use for the given item
        /// </summary>
        /// <param name="item">The item to get the property descriptor for</param>
        /// <returns>The property descriptor used to get the DisplayMember field</returns>
        /// <remarks>
        /// Caches the property descriptor to improve performance
        /// </remarks>
        protected virtual PropertyDescriptor GetDisplayMemberPropertyDescriptor(object item)
        {
            ITypedList typedList = _dataSource as ITypedList;
            if (typedList != null)
            {
                if (_displayMemberPropertyDescriptor == null)
                {
                    PropertyDescriptorCollection properties = typedList.GetItemProperties(null);
                    if (properties != null && !string.IsNullOrEmpty(DisplayMember))
                    {
                        _displayMemberPropertyDescriptor = properties[DisplayMember];
                    }
                }
            }
            else
            {
                if (item == null) return null;
                Type itemType = item.GetType();
                if (itemType != _displayMemberType)
                {
                    if (string.IsNullOrEmpty(DisplayMember))
                    {
                        _displayMemberPropertyDescriptor = null;
                    }
                    else
                    {
                        _displayMemberPropertyDescriptor = TypeDescriptor.GetProperties(item)[DisplayMember];
                    }
                    _displayMemberType = itemType;
                }
            }
            return _displayMemberPropertyDescriptor;
        }

        /// <summary>
        /// Raises the GetItemText event to get the text to display for a given item
        /// </summary>
        /// <remarks>
        /// If the GetItemText event is not handled then this method uses the <see cref="DisplayMember"/>
        /// property to get the text display.  If this is not set then it uses the ToString method of the
        /// item being displayed.
        /// </remarks>
        /// <param name="item">The row to get the text for</param>
        internal protected virtual string GetTextForItem(object item) 
        {
            string result = String.Empty;

            if (GetItemText != null)
            {
                GetItemTextEventArgs args = new GetItemTextEventArgs(item);
                GetItemText(this, args);
                result = args.Text;
            }
            else
            {
                PropertyDescriptor pd = GetDisplayMemberPropertyDescriptor(item);
                TypeConverter tc = null;
                if (pd != null)
                {
                    item = pd.GetValue(item);
                    tc = pd.Converter;
                }
                if (tc == null && item != null)
                {
                    tc = TypeDescriptor.GetConverter(item);
                }
                if (tc != null && tc.CanConvertTo(typeof(string)))
                {
                    result = tc.ConvertToString(item);
                }
                else if (item != null)
                {
                    result = item.ToString();
                }
            }
            return result;
        }
 

        /// <summary>
        /// Call GetTextForItem to get the text to display for an item in the list
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnListGetItemText(object sender, GetItemTextEventArgs e)
        {
            e.Text = GetTextForItem(e.Item);
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

        #endregion

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(VirtualDropDownList));
            this._textBox = new System.Windows.Forms.TextBox();
            this._dropDownButton = new Infralution.Controls.DropDownButton();
            this.SuspendLayout();
            // 
            // _textBox
            // 
            this._textBox.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this._textBox.Location = new System.Drawing.Point(0, 0);
            this._textBox.Name = "_textBox";
            this._textBox.Size = new System.Drawing.Size(175, 13);
            this._textBox.TabIndex = 0;
            this._textBox.TabStop = false;
            this._textBox.KeyDown += new System.Windows.Forms.KeyEventHandler(this.OnTextKeyDown);
            // 
            // _dropDownButton
            // 
            this._dropDownButton.Image = ((System.Drawing.Image)(resources.GetObject("_dropDownButton.Image")));
            this._dropDownButton.Location = new System.Drawing.Point(175, 0);
            this._dropDownButton.Name = "_dropDownButton";
            this._dropDownButton.Size = new System.Drawing.Size(16, 20);
            this._dropDownButton.TabIndex = 0;
            this._dropDownButton.TabStop = false;
            this._dropDownButton.Click += new System.EventHandler(this.OnDropDownButtonClick);
            // 
            // VirtualDropDownList
            // 
            this.Controls.Add(this._textBox);
            this.Controls.Add(this._dropDownButton);
            this.Size = new System.Drawing.Size(195, 24);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

    }
}
