#region File Header
//
//      FILE:   VirtualListBox.cs.
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
using Infralution.Common;

namespace Infralution.Controls
{

    /// <summary>
    /// Defines a simple data bound ListBox control that provides true virtual loading of items.
    /// </summary>
    /// <remarks>
    /// <para>
    /// The standard .NET ListBox control performs very badly when bound to large data sources
    /// because it creates a display representation for every item in the bound data
    /// source upfront.  
    /// </para>
    /// <para>
    /// The <see cref="DisplayMember"/> property can be set to determine the field or property of
    /// the displayed items that is displayed in the list.
    /// </para>
    /// </remarks>
    [ToolboxItem(true)]
#if CHECK_LICENSE
    [LicenseProvider(typeof(ControlLicenseProvider))]
#endif
    public class VirtualListBox : WidgetControl, ISupportInitialize
    {

        #region Local Types

        /// <summary>
        /// Defines how the <see cref="MoveFocusItem"/> method handles selection
        /// </summary>
        protected enum KeyboardSelectionMode
        {
            /// <summary>
            /// Selection is set to be the new focus row
            /// </summary>
            MoveSelection,

            /// <summary>
            /// The selection anchor is moved to the new focus row
            /// </summary>
            MoveAnchor,

            /// <summary>
            /// Selection is set to the range between the anchor and the new focus row
            /// </summary>
            SelectRange,

            /// <summary>
            /// Selection is extended to include the range between the anchor and the new focus row
            /// </summary>
            ExtentSelectRange
        }

        /// <summary>
        /// Defines the widget used to display a single item in a <see cref="VirtualListBox"/>
        /// </summary>
        public class RowWidget : Widget
        {
            #region Member Variables

            /// <summary>
            /// The row the widget is associated with
            /// </summary>
            private int _row;
            private string _text;
        
            #endregion

            #region Public Interface

            /// <summary>
            /// Creates a new widget.
            /// </summary>
            /// <param name="listBox">The control the widget belongs</param>
            /// <param name="row">The row the widget is to display</param>
            public RowWidget(VirtualListBox listBox, int row)
                : base(listBox)
            {
                _row = row;
                _text = listBox.GetTextForItem(listBox.Item(row));
            }

            /// <summary>
            /// Return the ListBox the widget is associated with
            /// </summary>
            public VirtualListBox ListBox
            {
                get { return (VirtualListBox)WidgetControl; }
            }

            /// <summary>
            /// The row the widget is associated with
            /// </summary>
            public int Row
            {
                get { return _row; }
            }
            
            /// <summary>
            /// Returns the width in pixels required to display the item without clipping
            /// </summary>
            /// <param name="graphics">The graphics context</param>
            /// <returns>The width in pixels</returns>
            public virtual int GetItemWidth(Graphics graphics)
            {
                SizeF size = graphics.MeasureString(Text, ListBox.Font);
                return (int)size.Width;
            }

            #endregion

            #region Local Methods/Overrides

            /// <summary>
            /// Returns the text the widget is to display
            /// </summary>
            protected string Text
            {
                get { return _text; }
            }

            /// <summary>
            /// Draw the focus indicator for this item
            /// </summary>
            /// <param name="graphics">The context to paint to</param>
            public virtual void DrawFocusIndicator(Graphics graphics)
            {        
                ControlPaint.DrawFocusRectangle(graphics, Bounds, ListBox.SelectedForeColor, ListBox.SelectedBackColor);
            }

            /// <summary>
            /// Handle painting for this widget
            /// </summary>
            /// <param name="graphics"></param>
            public override void OnPaint(Graphics graphics)
            {
                Color textColor = ListBox.ForeColor;
                if (ListBox.Highlighted(Row))
                {
                    using (Brush brush = new SolidBrush(ListBox.SelectedBackColor))
                    {
                        graphics.FillRectangle(brush, Bounds);
                    }
                    textColor = ListBox.SelectedForeColor;
                }

                TextFormatFlags flags = TextFormatFlags.Default;
                if (this.RightToLeft == RightToLeft.Yes)
                {
                    flags |= TextFormatFlags.RightToLeft;
                    flags |= TextFormatFlags.Right;
                }
                TextRenderer.DrawText(graphics, _text, ListBox.Font, Bounds, textColor, flags);
            }

            /// <summary>
            /// Handle mouse selection for this row
            /// </summary>
            /// <param name="e">The original mouse event</param>
            protected virtual void OnMouseSelection(MouseEventArgs e)
            {
                switch (ModifierKeys)
                {
                    case Keys.Control:
                        ListBox.ExtendSelectRow(Row);
                        break;
                    case Keys.Shift:
                        ListBox.SelectRowRange(Row);
                        break;
                    case Keys.Control | Keys.Shift:
                        ListBox.ExtendSelectRowRange(Row);
                        break;
                    default:
                        ListBox.SelectRow(Row);
                        break;
                }
                ListBox.FocusRow = Row;
            }

            /// <summary>
            /// Handle a left mouse down event.
            /// </summary>
            /// <param name="e">The mouse event details.</param>
            protected virtual void OnLeftMouseDown(MouseEventArgs e)
            {
                OnMouseSelection(e);
            }

            /// <summary>
            /// Handle a right mouse down event
            /// </summary>
            /// <param name="e">The mouse event details.</param>
            protected virtual void OnRightMouseDown(MouseEventArgs e)
            {
                if (!ListBox.IsRowSelected(Row))
                {
                    ListBox.SelectRow(Row);
                    ListBox.FocusRow = Row;
                }
            }

            /// <summary>
            /// Handle MouseDown events for this Widget
            /// </summary>
            /// <param name="e"></param>
            public override void OnMouseDown(MouseEventArgs e)
            {
                switch (e.Button)
                {
                    case MouseButtons.Left:
                        OnLeftMouseDown(e);
                        break;
                    case MouseButtons.Right:
                        OnRightMouseDown(e);
                        break;
                }
            }

            /// <summary>
            /// Handle MouseUp events for this Widget.
            /// </summary>
            /// <param name="e"></param>
            public override void OnMouseUp(MouseEventArgs e)
            {
                if (ListBox.EnableDragSelect)
                {
                    // ensure the drag timer is stopped
                    //
                    ListBox.StopDragSelectTimer();
                    if (e.Button == MouseButtons.Left)
                    {
                        ListBox.CompleteDragSelection();
                    }
                }
            }

            /// <summary>
            /// Handle mouse movement when drag selecting rows
            /// </summary>
            /// <param name="e"></param>
            protected virtual void OnMouseDragSelect(MouseEventArgs e)
            {
                // get the row widget which the mouse is now over
                //
                Widget widget = ListBox.GetWidget(e.X, e.Y);
                if (widget is RowWidget)
                {
                    RowWidget rowWidget = widget as RowWidget;
                    ListBox.DragSelectRow(rowWidget.Row);
                }
                ListBox.StartDragSelectTimer();
            }
 
            /// <summary>
            /// Handle MouseMove events for this Widget
            /// </summary>
            /// <param name="e"></param>
            public override void OnMouseMove(MouseEventArgs e)
            {
                if (MouseButtons == MouseButtons.Left)
                {
                    if (ListBox.EnableDragSelect)
                    {
                        OnMouseDragSelect(e);
                    }
                }
            }

            #endregion

        }

        #endregion

        #region Member Variables

        #region Data Binding

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
        /// Is the control currently being initialized
        /// </summary>
        private bool _initializing = false;

        /// <summary>
        /// The currency manager for the data source
        /// </summary>
        private CurrencyManager _currencyManager;

        /// <summary>
        /// The underlying list of items to display
        /// </summary>
        private IList _items;

        #endregion

        #region Appearance

        /// <summary>
        /// The background color for selected items
        /// </summary>
        private Color _selectedBackColor = SystemColors.Highlight;

        /// <summary>
        /// The foreground color for selected items
        /// </summary>
        private Color _selectedForeColor = SystemColors.HighlightText;
        
        #endregion

        #region Selection/Focus

        /// <summary>
        /// Is selection bound to the currency manager.
        /// </summary>
        private bool _useCurrencyManager = true;

        /// <summary>
        /// Is multiple selection allowed.
        /// </summary>
        private bool _allowMultiSelect = true;

        /// <summary>
        /// Can the user select items by dragging the mouse over them
        /// </summary>
        private bool _enableDragSelect = true;

        /// <summary>
        /// The list of items selected by the user.
        /// </summary>
        private OrderedSet _selectedItems = new OrderedSet();

        /// <summary>
        /// The index of the row to display at the top of the list
        /// </summary>
        private int _topRow = 0;
   
        /// <summary>
        /// The index of the row that currently has focus
        /// </summary>
        private int _focusRow = 0;

        /// <summary>
        /// The item that currently has focus.
        /// </summary>
        private object _focusItem;

        /// <summary>
        /// The row at which an extended selection was started (if any)
        /// </summary>
        private int _anchorRow = -1;

        /// <summary>
        /// The current drag selection - the items being added by dragging the mouse over rows
        /// </summary>
        private IList _dragSelection = null;
        private int _lastDragRow = -1;    

        /// <summary>
        /// The number of rows to scroll the list box for each mouse wheel detent
        /// </summary>
        private int _wheelDelta = DefaultWheelDelta;
        private const int DefaultWheelDelta = 3;

        #endregion
  
        #region Layout

        /// <summary>
        /// The number of fully visible rows which can be displayed in the available client height
        /// </summary>
        private int _numVisibleItems = 0;

        /// <summary>
        /// Should horizontal scrolling be enabled
        /// </summary>
        private bool _allowHorizontalScrolling = false;

        /// <summary>
        /// The offset in pixels for horizontal scrolling
        /// </summary>
        private int _horzScrollOffset = 0;

        /// <summary>
        /// The height in pixels of each row 
        /// </summary>
        private int _rowHeight = 0;
    
        /// <summary>
        /// Should the horizontal scroll bar be shown
        /// </summary>
        private bool _showHorzScroll = false;

        /// <summary>
        /// Should the vertical scroll bar be shown
        /// </summary>
        private bool _showVertScroll = false;


        #endregion

        #region Components/Controls

         /// <summary>
        /// The vertical scrollbar.
        /// </summary>
        private VScrollBar _vertScroll;

        /// <summary>
        /// The horizontal scrollbar.
        /// </summary>
        private HScrollBar _horzScroll;

        /// <summary>
        /// Timer used to perform autoscrolling when dragging outside the list box.
        /// </summary>
        private Timer _dragSelectTimer;

        /// <summary>
        /// Required by Forms Designers
        /// </summary>
        private IContainer components = null;

        #endregion

        #endregion

        #region Component Designer generated code
     
        /// <summary>
        /// Releases all resources used by the VirtualTree.
        /// </summary>
        protected override void Dispose( bool disposing )
        {
            if( disposing )
            {
                UnbindDataSource();
                UnbindCurrencyManager();
                _currencyManager = null;
                _items = null;
                _dataSource = null;
                if( components != null )
                    components.Dispose();
            }
            base.Dispose( disposing );
        }

        /// <summary>
        /// Intialize the properties of the control and its components
        /// </summary>
        private void InitializeComponent()
        {
            this._vertScroll = new System.Windows.Forms.VScrollBar();
            this._horzScroll = new System.Windows.Forms.HScrollBar();
            this.SuspendLayout();
            // 
            // _vertScroll
            // 
            this._vertScroll.Location = new System.Drawing.Point(17, 17);
            this._vertScroll.Name = "_vertScroll";
            this._vertScroll.TabIndex = 0;
            this._vertScroll.Visible = false;
            this._vertScroll.Scroll += new System.Windows.Forms.ScrollEventHandler(this.OnVerticalScroll);
            // 
            // _horzScroll
            // 
            this._horzScroll.Location = new System.Drawing.Point(121, 17);
            this._horzScroll.Name = "_horzScroll";
            this._horzScroll.SmallChange = 5;
            this._horzScroll.TabIndex = 0;
            this._horzScroll.Visible = false;
            this._horzScroll.Scroll += new System.Windows.Forms.ScrollEventHandler(this.OnHorizontalScroll);
            // 
            // VirtualTree
            // 
            this.Controls.Add(this._vertScroll);
            this.Controls.Add(this._horzScroll);
            this.ResumeLayout(false);

        }
		
        #endregion
        
        #region Public Events

        /// <summary>
        /// Raised by the VirtualListBox to obtain the display text for a given item.  
        /// Handle this event if you wish to programmatically determine the text for a given item
        /// rather than using standard data binding
        /// </summary>
        [Category("Data"),
        Description("Fired to obtain the text to display for a given item")]
        public event GetItemTextHandler GetItemText;

        /// <summary>
        /// Raised when the selected items have been changed.
        /// </summary>
        [Category("Behavior"),
        Description("Fired following a change to the selected items")]
        public event EventHandler SelectedItemsChanged;

        #endregion

        #region Public Methods

        #region Initialization

        /// <summary>
        /// Initializes a new instance of the VirtualListBox class.  
        /// </summary>
        public VirtualListBox()
        {
            InitializeComponent();
            BackColor = SystemColors.Window;
            ForeColor = SystemColors.WindowText;
            _selectedItems.ListChanged += new ListChangedEventHandler(OnSelectedItemsChanged);
            
            #if CHECK_LICENSE
            ControlLicenseProvider.CheckLicense(ReflectionUtilities.GetInstantiatingAssembly(), typeof(VirtualListBox), this);
            #endif
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
                    BindCurrencyManager();
                    if (!_initializing)
                    {
                        UpdateData();
                    }
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
            SuspendLayout();

            // update the focus row using the focus item
            //
            if (_focusItem != null)
            {
                FocusRow = Row(_focusItem);
            }
            
            // remove items from the selected items list that do not belong to the
            // data source
            //
            ArrayList removeItems = new ArrayList();
            if (_items != null)
            {
                foreach (object item in SelectedItems)
                {
                    if (!_items.Contains(item))
                    {
                        removeItems.Add(item);
                    }
                }
            }
            SelectedItems.Remove(removeItems);
            ResumeLayout();
            PerformLayout();
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

        #endregion

        #region Selection/Focus

        /// <summary>
        /// Get/Sets whether the user can select multiple rows.
        /// </summary>
        /// <remarks>
        /// This does not affect the ability of the application to select multiple rows via code.
        /// </remarks>
        [Category("Behavior"), 
        Description("Determines whether the user can select multiple rows"),
        DefaultValue(true)] 
        public virtual bool AllowMultiSelect
        {
            get { return _allowMultiSelect; }
            set { _allowMultiSelect = value; }
        }

        /// <summary>
        /// Get/Sets whether the user can drag the mouse over rows to select them.
        /// </summary>
        [Category("Behavior"), 
        Description("Can the user can drag the mouse over rows to select them.  Disables drag and drop of Rows"),
        DefaultValue(true)] 
        public virtual bool EnableDragSelect
        {
            get { return _enableDragSelect; }
            set { _enableDragSelect = value; }
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
        /// Return the set of currently selected items
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual OrderedSet SelectedItems
        {
            get 
            { 
                return _selectedItems; 
            }
        }

        /// <summary>
        /// Set/Get the selected item.  This provides a simple interface for handling selection 
        /// when using AllowMultiSelect set to false.
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual object SelectedItem
        {
            get 
            { 
                int count = _selectedItems.Count;
                return (count > 0) ? _selectedItems[count-1] : null;
            }
            set 
            { 
                _selectedItems.Clear();
                int row = Row(value);
                if (row >= 0)
                {
                    _selectedItems.Add(value);
                    FocusRow = row;
                }
                Invalidate();
            }
        }

        /// <summary>
        /// Returns true if the given item is selected
        /// </summary>
        /// <param name="item">The item to check for selection</param>
        /// <returns>True if the item is selected otherwise false</returns>
        public bool IsItemSelected(object item)
        {
            return (item == null) ? false : _selectedItems.Contains(item);
        }

        /// <summary>
        /// Returns true if the given row is selected
        /// </summary>
        /// <param name="row">The row to check for selection</param>
        /// <returns>True if the row is selected otherwise false</returns>
        public bool IsRowSelected(int row)
        {
            return IsItemSelected(Item(row));
        }

        /// <summary>
        /// Set/Get the index of the row that has focus
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public int FocusRow
        {
            get 
            { 
                return _focusRow; 
            }
            set 
            { 
                if (value != _focusRow)
                {
                    if (value > LastRow) value = LastRow;
                    if (value < 0) value = 0;
                    _focusRow = value;
                    _focusItem = Item(value);
                    EnsureRowVisible(_focusRow);
                    Invalidate();
                }
            }
        }

        /// <summary>
        /// Set/Get the row that currently has focus
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual object FocusItem
        {
            get 
            { 
                if (_focusItem == null)
                {
                    _focusItem = Item(FocusRow);
                }
                return _focusItem; 
            }
            set 
            {
                FocusRow = Row(value);
            }
        }

        /// <summary>
        /// The number of rows to scroll for each mouse wheel detent
        /// </summary>
        [Category("Behavior"), 
         Description("The number of rows to scroll for each mouse wheel detent"),
         DefaultValue(DefaultWheelDelta)]   
        public virtual int WheelDelta
        {
            get { return _wheelDelta; }
            set { _wheelDelta = value; }
        }

        #endregion

        #region Rows
        
        /// <summary>
        /// Return the row in the list box corresponding to the given item
        /// </summary>
        /// <param name="item">The item to get the row of</param>
        /// <returns>The index of the row or -1 if the item is not in the datasource</returns>
        public virtual int Row(object item)
        {
            return (_items == null) ? -1 : _items.IndexOf(item);
        }

        /// <summary>
        /// Return the item at the given row of the list box
        /// </summary>
        /// <param name="row">The row to get the item at</param>
        /// <returns>The item or null if the row is out of bounds</returns>
        public virtual object Item(int row)
        {
            if (_items == null) return null;
            if (row < 0 || row >= _items.Count) return null;
            return _items[row];
        }

        /// <summary>
        /// Set/Get the index of the row displayed at the top of the list
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual int TopRow
        {
            get { return _topRow; }
            set 
            { 
                if (value != _topRow)
                {
                    if (value > LastRow) value = LastRow;
                    if (value < 0) value = 0;
                    _topRow = value;
                    PerformLayout();
                }
            }
        }

        /// <summary>
        /// Set/Get the item displayed at the top of the list
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual object TopItem
        {
            get { return Item(TopRow); }
            set { TopRow = Row(value); }
        }

        /// <summary>
        /// Get/Set the row of the bottom fully visible item
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual int BottomRow
        {
            get 
            { 
                int row = TopRow + NumVisibleItems - 1;
                if (row > LastRow) row = LastRow;
                return row; 
            }
            set
            {
                TopRow = value - NumVisibleItems + 1;
            }
        }

        /// <summary>
        /// Set/Get the item displayed at the bottom of the list
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual object BottomItem
        {
            get { return Item(BottomRow); }
            set 
            {
                BottomRow = Row(BottomItem);
            }
        }

        /// <summary>
        /// Return the row of the last item in the list box.
        /// </summary>
        /// <returns>The row of the last item in the list box</returns>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public int LastRow
        {
            get { return (_items == null) ? -1 : _items.Count - 1; }
        }

        /// <summary>
        /// Ensures that the given row is visible by moving the top/bottom row
        /// </summary>
        /// <param name="row">The row to ensure is visible</param>
        public void EnsureRowVisible(int row)
        {
            // can't do this until the control has actually been made visible for the first time
            //
            if (Height > 0)
            {
                if (row > BottomRow)
                {
                    BottomRow = row;
                }
                else if (row < TopRow)
                {
                    TopRow = row;
                }
            }
        }

        /// <summary>
        /// Ensures that the given item is visible
        /// </summary>
        /// <param name="item">The item to ensure is visible</param>
        public void EnsureItemVisible(object item)
        {
            int row = Row(item);
            if (row != -1)
            {
                EnsureRowVisible(row);
            }
        }


        #endregion

        #region Layout

        /// <summary>
        /// Get/Set the height of each row
        /// </summary>
        [Category("Layout"),
        Description("The height of each row in the list box")] 
        public virtual int RowHeight
        {
            get 
            {
                if (_rowHeight == 0)
                {
                    return DefaultRowHeight;
                }
                return _rowHeight; 
            }
            set
            {
                if (value < 0) throw new ArgumentOutOfRangeException();
                _rowHeight = value;
                PerformLayout();
            }
        }

        /// <summary>
        /// Should the property be serialized
        /// </summary>
        /// <returns></returns>
        private bool ShouldSerializeRowHeight()
        {
            return (_rowHeight != 0);
        }

        /// <summary>
        /// Reset the property to its default value
        /// </summary>
        private void ResetRowHeight()
        {
            RowHeight = 0;
        }

        /// <summary>
        /// Return the default row height to use for the current font
        /// </summary>
        protected virtual int DefaultRowHeight
        {
            get 
            {
                return Font.Height + 2;
            }
        }

        /// <summary>
        /// Return the height the control must be to display the given number of rows
        /// fully
        /// </summary>
        /// <param name="numRows">The number of rows to display</param>
        /// <returns>The height the control needs to be to display the given number of rows</returns>
        public virtual int IntegralHeightForRows(int numRows)
        {
            return RowHeight * numRows + (this.Height - this.ClientSize.Height);
        }

        /// <summary>
        /// Return the preferred height for the listbox based on the number of
        /// items in the data source
        /// </summary>
        [Browsable(false),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual int PreferredHeight
        {
            get
            {
                int numRows = (_items == null) ? 0 : _items.Count;
                if (numRows <= 0) numRows = 1;
                if (numRows > 12) numRows = 12;
                return IntegralHeightForRows(numRows);
            }
        }
 
        /// <summary>
        /// Should horizontal scrolling be enabled
        /// </summary>
        [Category("Layout"),
         DefaultValue(false),
        Description("Should horizontal scrolling be enabled")] 
        public virtual bool AllowHorizontalScrolling
        {
            get { return _allowHorizontalScrolling; }
            set 
            {
                _allowHorizontalScrolling = value;
                PerformLayout();
            }
        }

        /// <summary>
        /// Return the number of rows that can be displayed in the available space
        /// </summary>
        [Browsable(false)]
        public int NumVisibleItems
        {
            get { return _numVisibleItems; }
        }

        /// <summary>
        /// Set/Get the current horizontal scrolling offset in pixels.
        /// </summary>
        [Browsable(false),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        protected virtual int HorzScrollOffset
        {
            get { return _horzScrollOffset; }
            set 
            {
                int maxValue = HorzScrollBar.Maximum - HorzScrollBar.LargeChange;
                if (value < 0) value = 0;
                if (value > maxValue) value = maxValue;

                _horzScrollOffset = value;
                HorzScrollBar.Value = _horzScrollOffset;
                LayoutWidgets();
                Invalidate();
            }
        }

        /// <summary>
        /// Return the actual display area of the control minus scrollbars and headers
        /// </summary>
        [Browsable(false)]
        public override Rectangle DisplayRectangle
        {
            get
            {
                return new Rectangle(DisplayLeft, 0, DisplayWidth, DisplayHeight);
            }
        }

        /// <summary>
        /// Returns the location of the left of the display area
        /// </summary>
        [Browsable(false)]
        protected virtual int DisplayLeft
        {
            get 
            { 
                if (this.RightToLeft == RightToLeft.Yes)
                    return  (this.ShowVertScroll) ? VertScrollBar.Width : 0;
                else
                    return 0;
            }
        }

        /// <summary>
        /// Return the height of the row display area
        /// </summary>
        [Browsable(false)]
        protected virtual int DisplayHeight
        {
            get
            {
                int height = ClientSize.Height;
                if (ShowHorzScroll)
                {
                    height -= HorzScrollBar.Height;
                }
                return Math.Max(0, height);
            }
        }

        /// <summary>
        /// Gets the width of the row display area 
        /// </summary>
        [Browsable(false)]
        protected virtual int DisplayWidth
        {
            get
            {
                int width = ClientSize.Width;
                if (ShowVertScroll)
                    width -= VertScrollBar.Width;
                return Math.Max(0, width);
            }
        }

        #endregion

        #region Appearance

        /// <summary>
        /// The background color for selected items.  
        /// </summary>
        [Category("Appearance"),
        DefaultValue(typeof(Color), "Highlight"),
        Description("The background color for selected items")]
        public virtual Color SelectedBackColor
        {
            get { return _selectedBackColor; }
            set 
            { 
                _selectedBackColor = value; 
                Invalidate();
            }
        }

        /// <summary>
        /// The foreground color for selected items.  
        /// </summary>
        [Category("Appearance"),
        DefaultValue(typeof(Color), "HighlightText"),
        Description("The foreground color for selected items")]
        public virtual Color SelectedForeColor
        {
            get { return _selectedForeColor; }
            set 
            { 
                _selectedForeColor = value;
                Invalidate();
            }
        }

        #endregion

        #endregion

        #region Overridden/Hidden Properties

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
        /// Sets the default BackColor to Window.  
        /// </summary>
        [Category("Appearance"),
        DefaultValue(typeof(Color), "Window"),
        Description("The background color for the control")]
        public override Color BackColor
        {
            get { return base.BackColor; }
            set { base.BackColor = value; }
        }

        /// <summary>
        /// Sets the default ForeColor to WindowText.  
        /// </summary>
        [Category("Appearance"),
        DefaultValue(typeof(Color), "WindowText"),
        Description("The foreground color for the control")]
        public override Color ForeColor
        {
            get { return base.ForeColor; }
            set { base.ForeColor = value; }
        }

        #endregion

        #region Local Methods

        #region Data Binding 
        
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
        /// Handle binding to the currency manager for the new parent
        /// </summary>
        /// <param name="e"></param>
        protected override void OnParentChanged(EventArgs e)
        {
            UnbindCurrencyManager();
            BindCurrencyManager();
            base.OnParentChanged (e);
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

        #endregion

        #region Widgets/Painting

         /// <summary>
        /// Update the size/position of widgets when control size or horizontal scroll changes
        /// </summary>
        protected override void LayoutWidgets()
        {
            int rowOffset = -HorzScrollOffset;
            int rowWidth = ClientSize.Width - rowOffset;
 
            // layout the row widgets
            //
            Rectangle bounds = new Rectangle(rowOffset, 0, rowWidth, RowHeight);
            bounds = DrawingUtilities.RtlTranslateRect(this, bounds);
            foreach(Widget widget in Widgets)
            {
                widget.Bounds = bounds;
                bounds.Y += bounds.Height;
            }
        }

        /// <summary>
        /// Paint the focus indicator for the list box
        /// </summary>
        /// <param name="graphics"></param>
        protected virtual void PaintFocusIndicator(Graphics graphics)
        {
            if (FocusRow >= 0 && Focused && ShowFocusCues)
            {
                RowWidget widget = GetRowWidget(FocusRow);
                if (widget != null)
                {
                    widget.DrawFocusIndicator(graphics);
                }
            }
        }

        /// <summary>
        /// Override to set the clipping bounds to restrict painting to the client area
        /// </summary>
        /// <param name="e"></param>
        protected override void OnPaint(PaintEventArgs e)
        {
            Rectangle clip = DisplayRectangle;
            using (Brush brush = new SolidBrush(BackColor))
            {
                e.Graphics.FillRectangle(brush, e.ClipRectangle);
            }
            DrawingUtilities.SetClip(e.Graphics, clip);
        
            // draw the widgets
            //
            base.OnPaint (e);
            PaintFocusIndicator(e.Graphics);

            // must reset the clipping region or we get some very funny effects with double
            // buffering interactions with other controls on the same form
            //
            DrawingUtilities.ResetClip(e.Graphics);
        }

        /// <summary>
        /// Overridden to prevent base Control class painting the <see cref="Control.BackgroundImage"/>.
        /// </summary>
        /// <remarks>
        /// All painting is done in the OnPaint method.
        /// </remarks>
        /// <param name="pevent"></param>
        protected override void OnPaintBackground(PaintEventArgs pevent)
        {
        }

        /// <summary>
        /// Invalidates the control to update the focus indicator
        /// </summary>
        /// <param name="e"></param>
        protected override void OnGotFocus(EventArgs e)
        {
            Invalidate();
            base.OnGotFocus (e);
        }

        /// <summary>
        /// Invalidates the control to update the focus indicator
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLostFocus(EventArgs e)
        {
            Invalidate();
            base.OnLostFocus (e);
        }

        /// <summary>
        /// Invalidates the control when the UICues change
        /// </summary>
        /// <param name="e"></param>
        protected override void OnChangeUICues(UICuesEventArgs e)
        {
            Invalidate();
            base.OnChangeUICues (e);
        }

        #endregion

        #region Control Layout and Intialization

 
        /// <summary>
        /// Get the vertical scrollbar used by the control
        /// </summary>
        protected VScrollBar VertScrollBar
        {
            get { return _vertScroll; }
        }

        /// <summary>
        /// Get the horizontal scrollbar used by the control
        /// </summary>
        protected HScrollBar HorzScrollBar
        {
            get { return _horzScroll; }
        }

        /// <summary>
        /// Should the horizontal scroll bar be shown.
        /// </summary>
        protected virtual bool ShowHorzScroll
        {
            get { return _showHorzScroll; }
            set 
            {
                _showHorzScroll = value;
                HorzScrollBar.Visible = value;
            }
        }

        /// <summary>
        /// Should the vertical scroll bar be shown.
        /// </summary>
        protected virtual bool ShowVertScroll
        {
            get { return _showVertScroll; }
            set 
            {
                _showVertScroll = value;
                VertScrollBar.Visible = value;
            }
        }

        /// <summary>
        /// Create a widget to represent the given row
        /// </summary>
        /// <remarks>
        /// This method can be override to create widgets derived from <see cref="RowWidget"/>
        /// to allow custom painting of listbox items
        /// </remarks>
        /// <param name="row">The row the widget is associated with</param>
        /// <returns>A new widget</returns>
        protected virtual RowWidget CreateRowWidget(int row)
        {
            return new RowWidget(this, row);
        }

        /// <summary>
        /// Return the widget associated with the given row
        /// </summary>
        /// <param name="row">The row to get the widget for</param>
        /// <returns>The RowWidget used to display the row</returns>
        protected virtual RowWidget GetRowWidget(int row)
        {
            foreach (RowWidget widget in Widgets)
            {
                if (widget.Row == FocusRow)
                {
                    return widget;
                }
            }
            return null;
        }

        /// <summary>
        /// Calculates the number of visible items based on the current <see cref="DisplayHeight"/>
        /// and <see cref="RowHeight"/>
        /// </summary>
        protected virtual void UpdateNumVisibleItems()
        {
            _numVisibleItems = DisplayHeight / RowHeight;
        }

        /// <summary>
        /// Update the displayed widgets following a change
        /// </summary>
        protected virtual void UpdateWidgets()
        {
            Widgets.Clear();
            int bottomRow = TopRow + NumVisibleItems;
            if (bottomRow > LastRow) bottomRow = LastRow;
            for (int row = TopRow; row <= bottomRow; row++)
            {
                RowWidget widget = CreateRowWidget(row);
                Widgets.Add(widget);
            }
        }

        /// <summary>
        /// Positions the scrollbars and updates the visible widgets.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLayout(LayoutEventArgs e)
        {
            UpdateNumVisibleItems();
            UpdateWidgets();
            UpdateScrollBars();
            LayoutWidgets();
            
            int clientHeight = ClientSize.Height;
            int clientWidth = ClientSize.Width;
            Rectangle bounds;

            // position the scrollbars
            //
            int scrollHeight = (ShowHorzScroll) ?  clientHeight - HorzScrollBar.Height : clientHeight;
            bounds = new Rectangle(clientWidth - VertScrollBar.Width, 0, VertScrollBar.Width, scrollHeight);
            bounds = DrawingUtilities.RtlTranslateRect(this, bounds);
            VertScrollBar.SetBounds(bounds.X, bounds.Y, bounds.Width, bounds.Height);

            int scrollWidth = (ShowVertScroll) ? clientWidth - VertScrollBar.Width : clientWidth;
            bounds = new Rectangle(0, clientHeight - HorzScrollBar.Height,scrollWidth, HorzScrollBar.Height);
            bounds = DrawingUtilities.RtlTranslateRect(this, bounds);
            HorzScrollBar.SetBounds(bounds.X, bounds.Y, bounds.Width, bounds.Height);     
            Invalidate();
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
        /// Sets up double buffered painting for the control.
        /// </summary>
        protected override void OnCreateControl()
        {
            base.OnCreateControl ();

            // setup the control to be double buffered
            //
            SetStyle(ControlStyles.DoubleBuffer | ControlStyles.UserPaint | 
                     ControlStyles.AllPaintingInWmPaint, true);
        }

        /// <summary>
        /// Handles the WM_GETDLGCODE message to get windows to send ArrowKey messages to the control
        /// </summary>
        /// <param name="m"></param>
        protected override void WndProc(ref Message m)
        {
            const int WM_GETDLGCODE = 0x0087;
            const int DLGC_WANTARROWS = 0x001;
            const int DLGC_WANTCHARS = 0x080;

            base.WndProc (ref m);
            if (m.Msg == WM_GETDLGCODE)
            {
                m.Result = new IntPtr(DLGC_WANTARROWS | DLGC_WANTCHARS);
            }
        }

        /// <summary>
        /// Update the row height automatically when the font is changed
        /// </summary>
        /// <param name="e"></param>
        protected override void OnFontChanged(EventArgs e)
        {
            base.OnFontChanged (e);
            PerformLayout();
        }

        #endregion

        #region Keyboard Event Handling


        /// <summary>
        /// Handles keyboard navigation within the list box
        /// </summary>
        /// <param name="msg"></param>
        /// <param name="keys"></param>
        /// <returns></returns>
        protected override bool ProcessCmdKey(ref Message msg, Keys keys)
        {
            const int WM_KEYDOWN = 0x100;
            
            bool processed = false;
            if (msg.Msg == WM_KEYDOWN)
            {
                processed = true;

                switch (keys)
                {
                    case Keys.Up:
                        MoveFocusItem(-1, KeyboardSelectionMode.MoveSelection, true);
                        break;
                    case Keys.Up | Keys.Shift:
                        MoveFocusItem(-1, KeyboardSelectionMode.SelectRange, false);
                        break;
                    case Keys.Up | Keys.Control:
                        MoveFocusItem(-1, KeyboardSelectionMode.MoveAnchor, false);
                        break;
                    case Keys.Up | Keys.Control | Keys.Shift:
                        MoveFocusItem(-1, KeyboardSelectionMode.ExtentSelectRange, false);
                        break;
                    case Keys.Down:
                        MoveFocusItem(1, KeyboardSelectionMode.MoveSelection, true);
                        break;
                    case Keys.Down | Keys.Shift:
                        MoveFocusItem(1, KeyboardSelectionMode.SelectRange, false);
                        break;
                    case Keys.Down | Keys.Control:
                        MoveFocusItem(1, KeyboardSelectionMode.MoveAnchor, false);
                        break;
                    case Keys.Down | Keys.Control | Keys.Shift:
                        MoveFocusItem(1, KeyboardSelectionMode.ExtentSelectRange, false);
                        break;
                    case Keys.PageUp:
                        MoveFocusItem(-NumVisibleItems, KeyboardSelectionMode.MoveSelection, false);
                        break;
                    case Keys.PageUp | Keys.Shift:
                        MoveFocusItem(-NumVisibleItems, KeyboardSelectionMode.SelectRange, false);
                        break;
                    case Keys.PageUp | Keys.Control:
                        MoveFocusItem(-NumVisibleItems, KeyboardSelectionMode.MoveAnchor, false);
                        break;
                    case Keys.PageUp | Keys.Control | Keys.Shift:
                        MoveFocusItem(-NumVisibleItems, KeyboardSelectionMode.ExtentSelectRange, false);
                        break;
                    case Keys.PageDown:
                        MoveFocusItem(NumVisibleItems, KeyboardSelectionMode.MoveSelection, false);
                        break;
                    case Keys.PageDown | Keys.Shift:
                        MoveFocusItem(NumVisibleItems, KeyboardSelectionMode.SelectRange, false);
                        break;
                    case Keys.PageDown | Keys.Control:
                        MoveFocusItem(NumVisibleItems, KeyboardSelectionMode.MoveAnchor, false);
                        break;
                    case Keys.PageDown | Keys.Control | Keys.Shift:
                        MoveFocusItem(NumVisibleItems, KeyboardSelectionMode.ExtentSelectRange, false);
                        break;
                    default:
                        processed = false;
                        break;
                }
                Update();  // force any paint events to be processed
            }
 
            if (!processed)
                processed = base.ProcessCmdKey(ref msg, keys);
            return processed;
        }
          
        #endregion

        #region Selection and Focus Management

        /// <summary>
        /// Get/Set the currently selected anchor row (used for extended selection)
        /// </summary>
        internal virtual protected int AnchorRow
        {
            get { return _anchorRow; }
            set { _anchorRow = value; }
        }

        /// <summary>
        /// Get/Set the current select anchor item (used for extended selection)
        /// </summary>
        internal virtual protected object AnchorItem
        {
            get { return Item(_anchorRow); }
            set { AnchorRow = Row(value); }
        }

        /// <summary>
        /// Sets the current databinding position and raises the SelectedItemsChanged Event.  
        /// </summary>
        protected virtual void OnSelectedItemsChanged(object sender, ListChangedEventArgs e)
        {
            Invalidate();

            if (_currencyManager != null && SelectedItems.Count == 1)
            {
                _currencyManager.Position = this.Row(SelectedItem);               
            }
             
            if (SelectedItemsChanged != null)
            {
                SelectedItemsChanged(this, e);
            }

        }

        /// <summary>
        /// Move the current focus row the number of rows indicated by rowOffset. 
        /// </summary>
        /// <param name="rowOffset">The number/direction in which to move focus</param>
        /// <param name="mode">Determines the effect of this operation on selection</param>
        /// <param name="moveEdit">Determines whether the edit widget should move with focus</param>
        /// <returns>True if the focus was moved</returns>
        protected virtual bool MoveFocusItem(int rowOffset, 
                                            KeyboardSelectionMode mode,
                                            bool moveEdit)
        {
            // if there is no focus item then default to the first row
            //
            int focusRow = FocusRow;
            if (focusRow == -1)
               focusRow = TopRow;
            focusRow += rowOffset;
            if (focusRow < 0) focusRow = 0;
            if (focusRow > LastRow) focusRow = LastRow;

            if (focusRow != -1) 
            {
                if (focusRow < TopRow)
                {
                    // if the focus row is above the first visible row then shift the 
                    // first visible row to ensure the focus row is displayed
                    //
                    TopRow = focusRow;
                }
                else if (focusRow > BottomRow)
                {
                    // if the focus row is below the last visible row then shift the 
                    // last visible row to ensure the focus row is displayed
                    //
                    BottomRow = focusRow;
                }
                switch (mode)
                {
                    case KeyboardSelectionMode.MoveSelection:
                        SelectRow(focusRow);
                        break;
                    case KeyboardSelectionMode.MoveAnchor:
                        AnchorRow = focusRow;
                        break;
                    case KeyboardSelectionMode.SelectRange:
                        SelectRowRange(focusRow);
                        break;
                    case KeyboardSelectionMode.ExtentSelectRange:
                        KeyboardExtendSelectRowRange(focusRow);
                        break;
                }       
                FocusRow = focusRow;
            }
            return (focusRow != -1);
        }

        /// <summary>
        /// Return a list of the items between the given rows
        /// </summary>
        /// <param name="fromRow"></param>
        /// <param name="toRow"></param>
        /// <returns>A list of the items</returns>
        protected IList GetItems(int fromRow, int toRow)
        {
            ArrayList items = new ArrayList(Math.Abs(toRow - fromRow));
            if (_items != null)
            {
                // handle possible reversal of from and to rows
                //
                if (toRow >= fromRow)
                {
                    for (int i = fromRow; i <= toRow; i++)
                    {
                        items.Add(_items[i]);
                    }
                }
                else
                {
                    for (int i = fromRow; i >= toRow; i--)
                    {
                        items.Add(_items[i]);
                    }
                }
            }
            return items;
        }

        /// <summary>
        /// Should the given row be highlighted - this checks both the SelectedItems list 
        /// and the DragSelection list to determine the current selection state of the item.
        /// </summary>
        /// <param name="row">The row to determine the selection state of</param>
        /// <returns>Returns true if the row should be highlighted</returns>
        protected internal bool Highlighted(int row)
        {
            object item = Item(row);
            bool selected = SelectedItems.Contains(item);
            if (_dragSelection != null)
            {
                bool inDragSelection = _dragSelection.Contains(item);
                if (SelectedItems.Contains(Item(AnchorRow)))
                    selected = selected || inDragSelection;
                else
                    selected = selected && !inDragSelection;
            }
            return selected;
        }

        /// <summary>
        /// Clears the current selection (if any) and selects the given row
        /// </summary>
        /// <param name="row">The row to select</param>
        internal protected virtual void SelectRow(int row)
        {
            AnchorRow = row;
            SelectedItems.Set(Item(row));
        }

        /// <summary>
        /// Extends the current selection by adding the given row to the selection set.
        /// If the row is already selected then it removed from the selection set.
        /// </summary>
        /// <param name="row">The row to add to the selection</param>
        internal protected virtual void ExtendSelectRow(int row)
        {
            if (AllowMultiSelect)
            {
                object item = Item(row);
                if (IsItemSelected(item))
                {
                    AnchorRow = row;
                    SelectedItems.Remove(item);
                }
                else
                {
                    AnchorRow = row;
                    SelectedItems.Add(item);
                }
            }
            else
            {
                SelectRow(row);
            }
        }

        /// <summary>
        /// Clears the current selection and selects the range of rows
        /// bounded by the current selection anchor and the given row.
        /// </summary>
        /// <param name="toRow">The row to select to.</param>
        internal protected virtual void SelectRowRange(int toRow)
        {
            if (AnchorRow == -1 || !AllowMultiSelect)
            {
                SelectRow(toRow);
            }
            else
            {
                SelectedItems.SetItems(GetItems(AnchorRow, toRow));
            }
        }

        /// <summary>
        /// Adds or removes the range of rows bounded by the current selection anchor 
        /// and the given row to the current selection set.   
        /// </summary>
        /// <remarks>
        /// If the current selection anchor is selected then the rows are added 
        /// otherwise they are removed
        /// </remarks>
        /// <param name="toRow">The row to select to.</param>
        internal protected virtual void ExtendSelectRowRange(int toRow)
        {
            if (AnchorRow == -1 || !AllowMultiSelect)
            {
                SelectRow(toRow);
            }
            else
            {
                if (IsRowSelected(AnchorRow))
                {
                    SelectedItems.AddItems(GetItems(AnchorRow, toRow));
                }
                else
                {
                    SelectedItems.RemoveItems(GetItems(AnchorRow, toRow));
                }
            }
        }

        /// <summary>
        /// Adds the range of rows bounded by the current selection anchor 
        /// and the given row to the current selection set.   
        /// </summary>
        /// <remarks>
        /// This method is designed to support extended range selection using
        /// the keyboard - unlike mouse selection this can't handle deselection
        /// </remarks>
        /// <param name="toRow">The row to select to.</param>
        protected virtual void KeyboardExtendSelectRowRange(int toRow)
        {
            if (AnchorRow == -1 || !AllowMultiSelect)
            {
                SelectRow(toRow);
            }
            else
            {
                SelectedItems.AddItems(GetItems(AnchorRow, toRow));
            }
        }

        /// <summary>
        /// Handle the user dragging selection over the given row.
        /// </summary>
        /// <param name="row">The row that selection is being dragged over</param>
        internal protected virtual void DragSelectRow(int row)
        {
            if (AllowMultiSelect && AnchorRow != -1)
            {
                // if there isn't already a drag selection in progress
                //
                if (_dragSelection == null)
                {
                    // if this row isn't the selection anchor then start a new drag selection
                    //
                    if (row != AnchorRow)
                    {
                        _dragSelection = GetItems(AnchorRow, row);
                        _lastDragRow = row;
                        Invalidate();
                    }
                }
                else
                {
                    // if row which the mouse is over has changed since the last time the
                    // drag Selection list was updated then update the drag selection list.
                    //
                    if (row != _lastDragRow)
                    {
                        _dragSelection = GetItems(AnchorRow, row);
                        _lastDragRow = row;
                        Invalidate();
                    }
                }
            }
            else
            {
                if (row != AnchorRow)
                {
                    FocusRow = row;
                    SelectRow(row);
                }
            }
        }
 

        /// <summary>
        /// Completes a drag selection by adding/removing the drag selected rows to the SelectedItems list
        /// </summary>
        internal protected virtual void CompleteDragSelection()
        {
            if (_dragSelection != null && _dragSelection.Count > 0)
            {
                if (IsRowSelected(AnchorRow))
                {
                    SelectedItems.AddItems(_dragSelection);
                }
                else
                {
                    SelectedItems.RemoveItems(_dragSelection);
                }
                _dragSelection = null;
                Invalidate();
            }
        }
  
        /// <summary>
        /// Clears the selection when the mouse is clicked on empty space.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseDown(MouseEventArgs e)
        {
            if (Focus())
            {
                base.OnMouseDown (e);
                if (e.Button == MouseButtons.Left && this.MouseDownWidget == null)
                {
                    AnchorRow = -1;
                    FocusItem = -1;
                    SelectedItems.Clear();
                    Invalidate();
                }
            }
        }

        #endregion

        #region Scrolling

        /// <summary>
        /// Calculates the number of visible rows and sets the vertical scrolling parameters.
        /// </summary>
        /// <returns>Returns true if the visibility of the vertical scrollbar is changed.</returns>
        private bool UpdateVertScrollBar()
        {
            bool showScroll;
            int scrollPosition = TopRow;
            
            VertScrollBar.Minimum = 0;
            VertScrollBar.Maximum = LastRow;
            
            if (scrollPosition > VertScrollBar.Maximum) scrollPosition = VertScrollBar.Maximum;
            if (scrollPosition < VertScrollBar.Minimum) scrollPosition = VertScrollBar.Minimum;
            VertScrollBar.Value = scrollPosition;

            // update the actual number of visible rows
            //
            UpdateNumVisibleItems();
            if (_numVisibleItems <= VertScrollBar.Maximum)
            {
                VertScrollBar.LargeChange = _numVisibleItems;
                showScroll = true;
            }
            else
            {
                if (VertScrollBar.Maximum > 0)
                {
                    VertScrollBar.LargeChange = VertScrollBar.Maximum;
                }
                showScroll = (TopRow > 0);
            }

            // determine whether the visibility of the scroll bar has changed
            //
            bool visibilityChanged = (showScroll != ShowVertScroll);
            ShowVertScroll = showScroll;
            return visibilityChanged;
        }

        /// <summary>
        /// Return the maximum width of the currently displayed items
        /// </summary>
        /// <returns></returns>
        protected virtual int GetMaxItemWidth()
        {
            int result = 0;
            using (Graphics graphics = CreateGraphics())
            {
                foreach (RowWidget widget in Widgets)
                {
                    int width = widget.GetItemWidth(graphics);
                    if (width > result)
                    {
                        result = width;
                    }
                }
            }      
            return result;
        }

        /// <summary>
        /// Calculates the horizontal scrolling parameters.
        /// </summary>
        /// <returns>Returns true if the visibility of the horizontal scrollbar is changed.</returns>
        private bool UpdateHorzScrollBar()
        {
            int displayWidth = DisplayWidth;
            int totalWidth = GetMaxItemWidth();
            bool showScroll = (totalWidth > displayWidth);
            if (showScroll)
            {
                HorzScrollBar.LargeChange = displayWidth;
                HorzScrollBar.Maximum = totalWidth;
            }
            else
            {
                HorzScrollBar.Value = 0;
                _horzScrollOffset = 0;
            }
 
            // determine whether the visibility of the scroll bar has changed
            //
            bool visibilityChanged = (showScroll != ShowHorzScroll);
            ShowHorzScroll = showScroll;
            return visibilityChanged;
        }

        /// <summary>
        /// Update the horizontal and vertical scroll bar positions
        /// </summary>
        protected virtual void UpdateScrollBars()
        {
            if (_items == null) 
            {
                ShowVertScroll = false;
                ShowHorzScroll = false;
            }
            else
            {
                // handle the interaction between the horizontal and vertical scrollbar visibilty
                // by recalculating the parameters if the visibility is changed
                //
                UpdateVertScrollBar();
                if (AllowHorizontalScrolling)
                {
                    if (UpdateHorzScrollBar())
                    {
                        // if the visibility of the horz scroll changed then we need to recalculate
                        // the vertical scrolling parameters
                        //
                        if (UpdateVertScrollBar())
                        {
                            // if the visibility of the vert scroll changed then we need to 
                            // recalculate the horizontal parameters
                            //
                            UpdateHorzScrollBar();
                        }
                    }
                }
                else
                {
                    ShowHorzScroll = false;
                }
            }
        }

        /// <summary>
        /// Handle a vertical scrolling event.  Updates the row of the top row displayed by
        /// the list box and calls PerformLayout to display the new range of rows.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnVerticalScroll(object sender, ScrollEventArgs e)
        {
            switch (e.Type)
            {
                case ScrollEventType.First:
                case ScrollEventType.Last:
                case ScrollEventType.SmallDecrement:
                case ScrollEventType.SmallIncrement:
                case ScrollEventType.LargeDecrement:
                case ScrollEventType.LargeIncrement:
                case ScrollEventType.ThumbTrack:
                    
                    // update the top row 
                    //
                    TopRow = e.NewValue;
                    Update();  // force paint event to be processed for smooth scrolling
                    break;
                case ScrollEventType.EndScroll:
                    if (TopRow != e.NewValue)
                    {
                        TopRow = e.NewValue;
                    }
                    break;
            }
        }

        /// <summary>
        /// Handles a horizontal scroll event.   Forces a repaint.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnHorizontalScroll(object sender, ScrollEventArgs e)
        {
            _horzScrollOffset = e.NewValue;
            LayoutWidgets();
            Refresh();
        }

        /// <summary>
        /// Handle mouse wheel scrolling
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseWheel(MouseEventArgs e)
        {
            base.OnMouseWheel (e);
            if (e.Delta > 0)
            {
                TopRow -= WheelDelta;
            }
            else
            {
                int row = BottomRow + WheelDelta;
                if (row > LastRow) row = LastRow;
                BottomRow = row;
            }
        }

        #endregion

        #region Drag Timer

        /// <summary>
        /// Starts the drag selection timer if the current mouse position is outside the row area
        /// </summary>
        internal protected virtual void StartDragSelectTimer()
        {
            Point point = PointToClient(MousePosition);
            int offset = 0;
            if (point.Y > HorzScrollBar.Top)
            {
                offset = point.Y - HorzScrollBar.Top;
            }
            else if (point.Y < 0)
            {
                offset = -point.Y;
            }
            if (offset > 0)
            {
                if (_dragSelectTimer == null)
                {
                    _dragSelectTimer = new Timer();
                    _dragSelectTimer.Tick += new EventHandler(OnDragSelectTimerTick);
                }
                _dragSelectTimer.Interval = 500 / offset;
                _dragSelectTimer.Start();
            }
            else
            {
                StopDragSelectTimer();
            }
        }

        /// <summary>
        /// Stops the drag selection timer
        /// </summary>
        internal protected virtual void StopDragSelectTimer()
        {
            if (_dragSelectTimer != null)
            {
                _dragSelectTimer.Dispose();
                _dragSelectTimer = null;
            }
        }

        /// <summary>
        /// Handles a tick event from the Drag Timer.  
        /// </summary>
        /// <remarks>
        /// The drag timer is started when the user attempts to drag selection past the first 
        /// or last displayed rows.   The event scrolls the list box in the direction that the
        /// user is dragging to enable drag selection select elements outside the currently
        /// displayed rows.
        /// </remarks>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnDragSelectTimerTick(object sender, EventArgs e)
        {
            Point point = PointToClient(MousePosition);
            int row = -1;
            if (point.Y > HorzScrollBar.Top)
            {
                BottomRow++;
                row = BottomRow;
            }
            else if (point.Y < 0)
            {
                TopRow--;
                row = TopRow;
            }

            if (row == -1)
            {
                StopDragSelectTimer();
            }
            else
            {
                DragSelectRow(row);
            }
        }
    
        #endregion

        #endregion

        #region ISupportInitialize Members

        /// <summary>
        /// Is the control currently being initialized (ie BeginInit has been called
        /// but EndInit has not)
        /// </summary>
        protected bool Initializing
        {
            get { return _initializing; }
        }

        /// <summary>
        /// Start initialization of the virtual list box.
        /// </summary>
        public void BeginInit()
        {
            SuspendLayout();
            _initializing = true;
        }

        /// <summary>
        /// End initialization of the virtual list box
        /// </summary>
        public void EndInit()
        {
            _initializing = false;
            UpdateData();
            ResumeLayout();
        }

        #endregion

 
    }
}
