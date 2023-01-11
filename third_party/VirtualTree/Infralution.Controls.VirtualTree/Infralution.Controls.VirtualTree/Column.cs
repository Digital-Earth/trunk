#region File Header
//
//      FILE:   Column.cs.
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
using System.Diagnostics;
using System.ComponentModel;
using System.ComponentModel.Design;
using System.Xml;
using System.Xml.Serialization;
using Infralution.Common;
using NS=Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree
{
    /// <summary>
    /// Defines information about a column within a <see cref="VirtualTree"/>.  
    /// </summary>
    /// <remarks>
    /// <para>
    /// The <see cref="VirtualTree.Columns">VirtualTree.Columns</see> property defines the set of columns for a 
    /// <see cref="VirtualTree"/>.  Columns define the space in which data values from the <see cref="VirtualTree.DataSource"/>
    /// are displayed. Each tree should contain at least one column to allow data values to be displayed.  
    /// </para>
    /// <para>
    /// Unlike some tree implementations Columns are not data bound directly to attributes of the 
    /// <see cref="VirtualTree.DataSource"/>.  Instead the user creates a <see cref="RowBinding"/> which binds to
    /// a particular type of data item and defines <see cref="CellBinding">CellBindings</see> for it that bind to
    /// particular columns.   This allows different types of items displayed in the tree to have different bindings
    /// for a given column.   For instance a "Name" column may be bound to the "LastName" property for one object and
    /// the "FullName" property of another.   This means that the domain objects to be displayed in the tree do not
    /// have to be modified to force them to fit a common mould.  It also allows for <see cref="ContextSensitive"/> 
    /// columns.   These are columns which are supported by only some row types.  They can be displayed and hidden
    /// depending on the type of row currently selected.
    /// </para>
    /// </remarks>
    /// <seealso href="XtraColumns.html">Using Columns</seealso>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="ColumnHeaderWidget"/>
    /// <seealso cref="RowBinding"/>
    /// <seealso cref="CellBinding"/>
    [DesignTimeVisible(false), ToolboxItem(false)]
    public class Column : Component, INotifyPropertyChanged
    {
        #region Member Variables


        /// <summary>
        /// The tree this column belongs to
        /// </summary>
        private VirtualTree _tree;      

        /// <summary>
        /// The name of this column - when sited matches that of the site
        /// </summary>
        private string _name;

        /// <summary>
        /// The caption to display for this column
        /// </summary>
        private string _caption;

        /// <summary>
        /// The icon to display in the column header
        /// </summary>
        private Icon _icon;

        /// <summary>
        /// The tooltip to display for the column header
        /// </summary>
        private string _toolTip;

        /// <summary>
        /// The current width in pixels of this column
        /// </summary>
        private int _width = _defaultWidth;
        private const int _defaultWidth = 100;

        /// <summary>
        /// The minimum allowed width in pixels of this column
        /// </summary>
        private int _minWidth = _defaultMinWidth;
        private const int _defaultMinWidth = 30;

        /// <summary>
        /// The maximum allowed width in pixels of this column when auto sizing
        /// </summary>
        private int _maxAutoSizeWidth = 0;

        /// <summary>
        /// Determines whether the tree will autosize the column
        /// </summary>
        private ColumnAutoSizePolicy _autoSizePolicy = ColumnAutoSizePolicy.Manual;

        /// <summary>
        /// Determines the width of this column relative to other columns when
        /// <see cref="VirtualTree.AutoFitColumns"/> is true.
        /// </summary>
        private float _autoFitWeight = _defaultAutoFitWeight;
        private const float _defaultAutoFitWeight = 1;

        /// <summary>
        /// The current sort direction for the column
        /// </summary>
        private ListSortDirection _sortDirection = ListSortDirection.Ascending;

        /// <summary>
        /// Can the user sort the column by clicking on the header
        /// </summary>
        private bool _sortable = true;

        /// <summary>
        /// Can the user move this column
        /// </summary>
        private bool _movable = true;

        /// <summary>
        /// Can the user resize this column
        /// </summary>
        private bool _resizable = true;

        /// <summary>
        /// Can this column be shown/hidden by the user
        /// </summary>
        private bool _hidable = true;

        /// <summary>
        /// Is this column shown/hidden automatically depending on the current focus row of the tree
        /// </summary>
        private bool _contextSensitive = false;

        /// <summary>
        /// Should the column be displayed in the current context
        /// </summary>
        private bool _inContext = true;

        /// <summary>
        /// Is this column currently active
        /// </summary>
        private bool _active = true;

        /// <summary>
        /// Is this column a pinned (non-scrollable) header column
        /// </summary>
        private bool _pinned = false;

        /// <summary>
        /// The style for the column header
        /// </summary>
        private Style _headerStyle = new Style();

        /// <summary>
        /// The style when the column header is pressed
        /// </summary>
        private Style _headerPressedStyle = new Style();

        /// <summary>
        /// The style when the column header is hot
        /// </summary>
        private Style _headerHotStyle = new Style();

        /// <summary>
        /// The style when the column header is being dropped
        /// </summary>
        private Style _headerDropStyle = new Style();

        /// <summary>
        /// The style when the column header is printed
        /// </summary>
        private Style _headerPrintStyle = new Style();

        /// <summary>
        /// The cell style for this column
        /// </summary>
        private Style _cellStyle = new Style();

        /// <summary>
        /// The cell style for odd rows in this column
        /// </summary>
        private Style _cellOddStyle = new Style();

        /// <summary>
        /// The cell style for even rows in this column
        /// </summary>
        private Style _cellEvenStyle = new Style();

        /// <summary>
        /// The style to use when printing cells in this column
        /// </summary>
        Style _cellPrintStyle = new Style();

        /// <summary>
        /// The style to use when printing even rows 
        /// </summary>
        Style _cellPrintEvenStyle = new Style();

        /// <summary>
        /// The style to use when printing odd rows 
        /// </summary>
        Style _cellPrintOddStyle = new Style();

        /// <summary>
        /// The editor to use for cells of this column
        /// </summary>
        private CellEditor _cellEditor = null;

        /// <summary>
        /// Can this column be selected when using <see cref="SelectionMode.Cell"/> selection
        /// </summary>
        private bool _selectable = true;

        /// <summary>
        /// The default data field that this column is displaying
        /// </summary>
        private string _dataField;

        /// <summary>
        /// User defined data associated with the column
        /// </summary>
        private object _tag;

        /// <summary>
        /// Should the pin icon be shown if the column is pinned
        /// </summary>
        private bool _showPinIcon = true;

        #endregion

        #region Public Events

        /// <summary>
        /// Raised when the properties of the column are changed.
        /// </summary>
        [Description("Fired when the properties of the column are changed")]
    	public event PropertyChangedEventHandler PropertyChanged;
   
        #endregion

        #region Public Methods
        
        /// <summary>
        /// Initialise a new instance of a column
        /// </summary>
        public Column()
        {
            _headerStyle.PropertyChanged += new PropertyChangedEventHandler(OnStyleChanged);
            _headerPressedStyle.PropertyChanged += new PropertyChangedEventHandler(OnStyleChanged);
            _headerHotStyle.PropertyChanged += new PropertyChangedEventHandler(OnStyleChanged);
            _headerDropStyle.PropertyChanged += new PropertyChangedEventHandler(OnStyleChanged);
            _headerPrintStyle.PropertyChanged += new PropertyChangedEventHandler(OnStyleChanged);
        }

        /// <summary>
        /// Return the Virtual Tree this column is associated with.
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public VirtualTree Tree
        {
            get { return _tree; }
        }

        /// <summary>
        /// The name used to identify this column. 
        /// </summary>
        /// <remarks>
        /// When the column is sited at design time the name matches that of the site
        /// </remarks>
        [Browsable(false)] 
        [DefaultValue(null)]
        public string Name
        {
            get 
            {
                if (this.Site != null)
                    return Site.Name;
                return _name; 
            }
            set { _name = value; }
        }
 
        /// <summary>
        /// The data field that the column is associated with. 
        /// </summary>
        /// <remarks>
        /// This is used by the <see cref="NS.VirtualTree.AutoGenerateBindings()"/> method to
        /// keep track of columns which were originally automatically created.  
        /// </remarks>
        [Browsable(false)] 
        [DefaultValue(null)]
        public string DataField
        {
            get { return _dataField; } 
            set 
            { 
                _dataField = value;
                OnPropertyChanged("DataField");
            }
        }

        /// <summary>
        /// Application specific data associated with the column 
        /// </summary>
        /// <remarks>
        /// Developers can use this property to associated application specific data with a column  
        /// </remarks>
        [Browsable(false)]
        [DefaultValue(null)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public object Tag
        {
            get { return _tag; }
            set
            {
                _tag = value;
                OnPropertyChanged("Tag");
            }
        }

        /// <summary>
        /// The text to display as a caption for this column 
        /// </summary>
        [Category("Appearance")]
        [Description("The text to display as a caption for this column")]
        [Localizable(true)]
        public string Caption
        {
            get { return _caption; }
            set 
            {
                if (_caption != value)
                {
                    _caption = value;
                    OnPropertyChanged("Caption");
                }
            }
        }

        /// <summary>
        /// The tooltip to display for this column header
        /// </summary>
        [Category("Appearance")]
        [DefaultValue(null)]
        [Description("The tooltip to display for this column header")]
        [Localizable(true)]
        public string ToolTip
        {
            get { return _toolTip; }
            set 
            {
                if (_toolTip != value)
                {
                    _toolTip = value;
                    OnPropertyChanged("ToolTip");
                }
            }
        }


        /// <summary>
        /// The icon to display to the left of the column caption 
        /// </summary>
        [Category("Appearance")]
        [Description("The icon to display to the left of the column caption")]
        [DefaultValue(null)]
        [Localizable(true)]
        public Icon Icon
        {
            get { return _icon; }
            set 
            {
                if (_icon != value)
                {
                    _icon = value;
                    OnPropertyChanged("Icon");
                }
            }
        }

        /// <summary>
        /// Should the pin icon be shown when the column is pinned 
        /// </summary>
        [Category("Appearance")]
        [Description("Should the pin icon be shown when the column is pinned")]
        [Localizable(true)]
        [DefaultValue(true)]
        public bool ShowPinIcon
        {
            get { return _showPinIcon; }
            set
            {
                if (_showPinIcon != value)
                {
                    _showPinIcon = value;
                    OnPropertyChanged("ShowPinIcon");
                }
            }
        }

        /// <summary>
        /// The current width of the column in pixels
        /// </summary>
        /// <remarks>
        /// <para>
        /// If <see cref="VirtualTree.AutoFitColumns"/> is set to true or the <see cref="AutoSizePolicy"/> 
        /// for the column is set to <see cref="ColumnAutoSizePolicy.AutoIncrease"/>
        /// or <see cref="ColumnAutoSizePolicy.AutoSize"/> then width of the column may be automatically
        /// set by VirtualTree overriding any programmatically set values. 
        /// </para>
        /// <para>
        /// If <see cref="VirtualTree.AutoFitColumns"/> is set to true then you can use the <see cref="AutoFitWeight"/>
        /// property to set the relative widths of columns.
        /// </para>
        /// </remarks>
        [Category("Layout")]
        [DefaultValue(_defaultWidth)]
        [Description("The width of this column in pixels")]
        [Localizable(true)]
        public int Width
        {
            get { return _width; }
            set 
            {
                if (value < 0)
                    throw new ArgumentOutOfRangeException("Width", value, "Width cannot be negative");
                if (_width != value)
                {
                    _width = value;
                    OnPropertyChanged("Width");
                }
            }
        }

        /// <summary>
        /// Determines the width of this column relative to other columns when
        /// <see cref="VirtualTree.AutoFitColumns"/> is true.
        /// </summary>
        /// <remarks>
        /// <para>
        /// If <see cref="VirtualTree.AutoFitColumns"/> is true then VirtualTree calculates the minimum required 
        /// width for each displayed column and determines the remaining available space.  This additional space is 
        /// allocated to each <see cref="Resizable"/> based on the proportion of the columns AutoFitWeight to the
        /// total AutoFitWeight.  For instance if the total of all resizable columns AutoFitWeights is 100 and this
        /// column has an AutoFitWeight of 20 - then it will be allocated 20% of any additional space available.
        /// </para>
        /// <para>
        /// In AutoFitColumn mode Virtual Tree will attempt to fit all the displayed columns without the need 
        /// for a horizontal scrollbar.  If the total minimum widths for all columns exceeds the available 
        /// control width then a horizontal scrollbar will be shown. 
        /// </para>
        /// </remarks>
        [Category("Layout")]
        [DefaultValue(_defaultAutoFitWeight)]
        [Description("Determines the width of this column relative to other columns when VirtualTree.AutoFitColumns is true")]
        [Localizable(true)]
        public float AutoFitWeight
        {
            get { return _autoFitWeight; }
            set 
            {
                if (value < 0)
                    throw new ArgumentOutOfRangeException("AutoFitWeight", value, "AutoFitWeight cannot be negative");
                if (_autoFitWeight != value)
                {
                    _autoFitWeight = value;
                    OnPropertyChanged("AutoFitWeight");
                }
            }
        }

        /// <summary>
        /// Defines whether the tree will autosize the column to fit the displayed data
        /// </summary>
        /// <remarks>
        /// If the AutoSizePolicy is set to <see cref="ColumnAutoSizePolicy.AutoSize"/> then
        /// the user can not set the width of the column even if <see cref="Resizable"/> is set
        /// to true.
        /// </remarks>
        [Category("Layout")]
        [DefaultValue(ColumnAutoSizePolicy.Manual)]
        [Description("Defines whether the tree will autosize the column to fit the displayed data")]
        public ColumnAutoSizePolicy AutoSizePolicy
        {
            get { return _autoSizePolicy; }
            set 
            {
                if (_autoSizePolicy != value)
                {
                    _autoSizePolicy = value;
                    OnPropertyChanged("AutoSizePolicy");
                }
            }
        }

        /// <summary>
        /// The minimum allowed width that the user can size the column to 
        /// </summary>
        [Category("Layout")]
        [DefaultValue(_defaultMinWidth)]
        [Description("The minimum allowed width that the user can size the column to")]
        [Localizable(true)]
        public int MinWidth
        {
            get { return _minWidth; }
            set 
            {
                if (value < 0)
                    throw new ArgumentOutOfRangeException("MinWidth", value, "MinWidth cannot be negative");
                if (_minWidth != value)
                {
                    _minWidth = value;
                    if (Width < value)
                        Width = value;
                    OnPropertyChanged("MinWidth");
                }
            }
        }

        /// <summary>
        /// The maximum width of column when auto-sizing (zero = no limit) 
        /// </summary>
        [Category("Layout")]
        [DefaultValue(0)]
        [Description("The maximum width of column when auto-sizing (zero = no limit)")]
        [Localizable(true)]
        public int MaxAutoSizeWidth
        {
            get { return _maxAutoSizeWidth; }
            set
            {
                if (value < 0)
                    throw new ArgumentOutOfRangeException("MaxAutoSizeWidth", value, "MaxAutoSizeWidth cannot be negative");
                if (_maxAutoSizeWidth != value)
                {
                    _maxAutoSizeWidth = value;
                    OnPropertyChanged("MaxAutoSizeWidth");
                }
            }
        }

        /// <summary>
        /// Can the column be sorted by the user by clicking on the column header 
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(true)]
        [Description("Can the column be sorted by the user by clicking on the column header")]
        public bool Sortable
        {
            get { return _sortable; }
            set 
            {
                if (_sortable != value)
                {
                    _sortable = value;
                    OnPropertyChanged("Sortable");
                }
            }
        }
        
        /// <summary>
        /// Can the column be moved by the user  
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(true)]
        [Description("Can the column be moved by the user")]
        public bool Movable
        {
            get { return _movable; }
            set 
            {
                if (_movable != value)
                {
                    _movable = value;
                    OnPropertyChanged("Movable");
                }
            }
        }

        /// <summary>
        /// Can the column be resized by the user  
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(true)]
        [Description("Can the column be resized by the user")]
        public bool Resizable
        {
            get { return _resizable; }
            set 
            {
                if (_resizable != value)
                {
                    _resizable = value;
                    OnPropertyChanged("Resizable");
                }
            }
        }

        /// <summary>
        /// Can the column be shown/hidden by the user  
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(true)]
        [Description("Can the column be shown/hidden by the user")]
        public bool Hidable
        {
            get { return _hidable; }
            set
            {
                if (_hidable != value)
                {
                    _hidable = value;
                    OnPropertyChanged("Hidable");
                }
            }
        }

        /// <summary>
        /// Can this column be selected when using <see cref="SelectionMode.Cell"/> selection
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(true)]
        [Description("Can the column be selected by the user when using Cell SelectionMode")]
        public bool Selectable
        {
            get { return _selectable; }
            set
            {
                if (_selectable != value)
                {
                    _selectable = value;
                    OnPropertyChanged("Selectable");
                }
            }
        }

        /// <summary>
        /// The current sort direction for the column 
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(ListSortDirection.Ascending)]
        [Description("The current sort direction for the column")]
        public ListSortDirection SortDirection
        {
            get { return _sortDirection; }
            set 
            {
                if (_sortDirection != value)
                {
                    _sortDirection = value;
                    OnPropertyChanged("SortDirection");
                    if (Tree != null && Tree.SortColumn == this)
                    {
                        Tree.OnSortColumnChanged();
                    }
                }
            }
        }

        /// <summary>
        /// Gets/Sets if this column automatically hidden/shown depending on whether the 
        /// current tree context (typically based on the focus row) supports it.  
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(false)]
        [Description("Is this column automatically hidden/shown depending on whether the current focus row supports it")]
        public bool ContextSensitive
        {
            get { return _contextSensitive; }
            set 
            {
                if (_contextSensitive != value)
                {
                    _contextSensitive = value;
                    OnPropertyChanged("ContextSensitive");
                }
            }
        }

        /// <summary>
        /// Is this column currently active 
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(true)]
        [Description("Is this column currently active")]
        [Localizable(true)]
        public bool Active
        {
            get { return _active; }
            set 
            {
                if (_active != value)
                {
                    _active = value;
                    OnPropertyChanged("Active");
                }
            }
        }

        /// <summary>
        /// Is the column currently visible
        /// </summary>
        [Browsable(false)]
        public bool Visible
        {
            get
            {
                return (InContext || !ContextSensitive) && Active;
            }
        }

        /// <summary>
        /// The default editor to use for editing cells of this column
        /// </summary>
        /// <remarks>
        /// This can be overridden for particular rows by handling the 
        /// <see cref="NS.VirtualTree.GetCellData">GetCellData</see> event or setting
        /// the <see cref="CellBinding.Editor"/> property.
        /// </remarks>
        [Category("Behavior")]
        [Description("The editor to use for editing cells of this column")]
        [DefaultValue(null)]
        public virtual CellEditor CellEditor
        {
            get { return _cellEditor; }
            set { _cellEditor = value; }
        }

        /// <summary>
        /// Should this column be used to display the icon and connections for rows.
        /// </summary>
        /// <remarks>
        /// Setting this to true means that this column will always be used to display the icon and connections for
        /// rows regardless of its position.  This allows you to add columns before the main column (typically for displaying
        /// flags).  If no MainColumn is set then the first visible column is used.  Using a <see cref="Movable"/> or 
        /// <see cref="ContextSensitive"/> column as the main column is likely to cause usability issues. The
        /// <see cref="NS.VirtualTree.MainColumn">VirtualTree.MainColumn</see> property can be used as alternative method of setting the
        /// main column.
        /// </remarks>
        [Category("Behavior")]
        [Description("Should this column always be used to display the icon and connections for rows")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        [DefaultValue(false)]
        public virtual bool MainColumn
        {
            get 
            {
                return (Tree == null) ? false : (Tree.MainColumn == this);
            }
            set
            {
                if (Tree != null)
                {
                    Tree.MainColumn = (value) ? this : null;
                }
            }
        }

        /// <summary>
        /// Should this column be used to display the prefix data and editor (typically a checkbox) for each row.
        /// </summary>
        /// <remarks>
        /// Setting this to true means that this column will be used to display data and/or an editor for each 
        /// <see cref="Row"/> within the <see cref="MainColumn"/> immediately following the row icon.  The 
        /// header for the prefix column is not displayed and it cannot be resized or customized by the user.
        /// Prefix columns are generally used to display a check box (or other state control) for each row.  
        /// The <see cref="NS.VirtualTree.PrefixColumn">VirtualTree.PrefixColumn</see> property can be used as 
        /// alternative method of setting the prefix column.
        /// </remarks>
        [Category("Behavior")]
        [Description("Should this column be used to display the prefix data and editor (typically a checkbox) for each row")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        [DefaultValue(false)]
        public virtual bool PrefixColumn
        {
            get 
            {
                return (Tree == null) ? false : (Tree.PrefixColumn == this);
            }
            set
            {
                if (Tree != null)
                {
                    Tree.PrefixColumn = (value) ? this : null;
                }
            }
        }

        /// <summary>
        /// Is this column a non-scrolling header column
        /// </summary>
        /// <remarks>
        /// <para>
        /// Setting this to true allows you to create columns that act as headers for rows.   This is typically
        /// used in conjunction with the <see cref="MainColumn"/> property.
        /// </para>
        /// <para>
        /// Pinned columns cannot be moved by the end user unless <see cref="VirtualTree.AllowUserPinnedColumns"/>
        /// is set to true and <see cref="Movable"/> is also true
        /// </para>
        /// </remarks>
        [Category("Behavior")]
        [Description("Should this column be used as a non-scrolling header")]
        [DefaultValue(false)]
        [Localizable(true)]
        public virtual bool Pinned
        {
            get 
            {
                if (PrefixColumn && Tree != null)
                {
                    Column mainColumn = Tree.GetMainColumn();
                    if (mainColumn != null)
                    {
                        return mainColumn.Pinned;
                    }
                }
                return _pinned;
            }
            set
            {
                if (_pinned != value)
                {
                    _pinned = value;
                    OnPropertyChanged("Pinned");
                }
            }
        }

        #region Header Styles

        /// <summary>
        /// The drawing attributes to use for this column header
        /// </summary>
        [Category("Appearance"),
        Description("The drawing attributes to use for this column header"),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style HeaderStyle
        {
            get { return _headerStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeHeaderStyle()
        {
            return _headerStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public void ResetHeaderStyle()
        {
            _headerStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use for this column header when pressed
        /// </summary>
        [Category("Appearance"),
        Description("The drawing attributes to use for this column header when pressed"),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style HeaderPressedStyle
        {
            get { return _headerPressedStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeHeaderPressedStyle()
        {
            return _headerPressedStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public void ResetHeaderPressedStyle()
        {
            _headerPressedStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use for this column header when mouse is over the header
        /// </summary>
        [Category("Appearance"),
        Description("The drawing attributes to use for headers when mouse is over the header"),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style HeaderHotStyle
        {
            get { return _headerHotStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeHeaderHotStyle()
        {
            return _headerHotStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public void ResetHeaderHotStyle()
        {
            _headerHotStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use for this column header when dragging and dropping columns
        /// </summary>
        [Category("Appearance"),
        Description("The drawing attributes to use for headers when dragging and dropping columns"),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style HeaderDropStyle
        {
            get { return _headerDropStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeHeaderDropStyle()
        {
            return _headerDropStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public void ResetHeaderDropStyle()
        {
            _headerDropStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use for this column header when printing
        /// </summary>
        [Category("Appearance"),
        Description("The drawing attributes to use when printing this column header"),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style HeaderPrintStyle
        {
            get { return _headerPrintStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeHeaderPrintStyle()
        {
            return _headerPrintStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public void ResetHeaderPrintStyle()
        {
            _headerPrintStyle.Reset();
        }

        #endregion

        #region Cell Styles

        /// <summary>
        /// The style to draw cells with
        /// </summary>
        /// <remarks>
        /// Use this to set style properties that should be applied to cells in both odd and
        /// even rows.   This style derives from <see cref="NS.VirtualTree.RowStyle">VirtualTree.RowStyle</see>.  
        /// Use <see cref="NS.VirtualTree.RowStyle">VirtualTree.RowStyle</see> to set properties which
        /// apply to all columns. 
        /// </remarks>
        /// <seealso cref="CellOddStyle"/>
        /// <seealso cref="CellEvenStyle"/>
        [Category("Appearance"),
        Description("The style to draw cells with"),
        RefreshProperties(RefreshProperties.Repaint), 
        DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public virtual Style CellStyle
        {
            get { return _cellStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeCellStyle()
        {
            return _cellStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        private void ResetCellStyle()
        {
            _cellStyle.Reset();
        }

        /// <summary>
        /// The style to draw cells in odd rows with
        /// </summary>
        /// <remarks>
        /// Use this to override style properties that apply only to cells in odd rows.   
        /// Derives from <see cref="CellStyle"/> and <see cref="NS.VirtualTree.RowOddStyle">VirtualTree.RowOddStyle</see>.   
        /// Use <see cref="NS.VirtualTree.RowOddStyle">VirtualTree.RowOddStyle</see> to set properties which apply to 
        /// all columns.   Use <see cref="CellStyle"/> to set properties which apply 
        /// to both odd and even rows of this column. 
        /// </remarks>
        /// <seealso cref="CellStyle"/>
        /// <seealso cref="CellEvenStyle"/>
        [Category("Appearance"),
        Description("The style to draw cells in odd rows with"),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public virtual Style CellOddStyle
        {
            get { return _cellOddStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeCellOddStyle()
        {
            return _cellOddStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        private void ResetCellOddStyle()
        {
            _cellOddStyle.Reset();
        }

        /// <summary>
        /// The style to draw cells in even rows with
        /// </summary>
        /// <remarks>
        /// Use this to override style properties that apply only to cells in even rows.   
        /// Derives from <see cref="CellStyle"/> and <see cref="NS.VirtualTree.RowEvenStyle">VirtualTree.RowEvenStyle</see>.   
        /// Use <see cref="NS.VirtualTree.RowEvenStyle">VirtualTree.RowEvenStyle</see> to set properties which apply to all
        /// columns.   Use <see cref="CellStyle"/> to set properties which apply 
        /// to both odd and even rows of this column. 
        /// </remarks>
        /// <seealso cref="CellStyle"/>
        /// <seealso cref="CellOddStyle"/>
        [Category("Appearance"),
        Description("The style to draw cells in even rows with"),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public virtual Style CellEvenStyle
        {
            get { return _cellEvenStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeCellEvenStyle()
        {
            return _cellEvenStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        private void ResetCellEvenStyle()
        {
            _cellEvenStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use when printing cells in this column
        /// </summary>
        [Category("Appearance"),
        Description("The drawing attributes to use when printing cells of this column"),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style CellPrintStyle
        {
            get { return _cellPrintStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeCellPrintStyle()
        {
            return _cellPrintStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetCellPrintStyle()
        {
            _cellPrintStyle.Reset();
        }

 
        /// <summary>
        /// The drawing attributes to use when printing cells in odd rows 
        /// </summary>
        [Category("Appearance"),
        Description("The drawing attributes to use when printing cells in odd rows"),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style CellPrintOddStyle
        {
            get { return _cellPrintOddStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeCellPrintOddStyle()
        {
            return _cellPrintOddStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetCellPrintOddStyle()
        {
            _cellPrintOddStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use when printing cells in even rows
        /// </summary>
        [Category("Appearance"),
        Description("The drawing attributes to use when printing cells in even rows"),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style CellPrintEvenStyle
        {
            get { return _cellPrintEvenStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeCellPrintEvenStyle()
        {
            return _cellPrintEvenStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetCellPrintEvenStyle()
        {
            _cellPrintEvenStyle.Reset();
        }


        #endregion

        /// <summary>
        /// Returns the string representation of this column.
        /// </summary>
        /// <returns>The string representation of this column</returns>
        public override string ToString()
        {
            return Name;
        }

        #endregion

        #region Internal Methods

        /// <summary>
        /// Attach the column to a tree.
        /// </summary>
        /// <param name="tree">The tree to attach this column to</param>
        internal void SetTree(VirtualTree tree)
        {
            _tree = tree;
            if (tree != null)
            {
                InitializeStyles();
            }
        }

        /// <summary>
        /// Get/Set whether this column is should be displayed given the current context
        /// </summary>
        internal bool InContext
        {
            get { return _inContext; }
            set 
            {
                if (value != _inContext)
                {
                    _inContext = value; 
                    OnPropertyChanged("InContext");
                }
            }
        }

        /// <summary>
        /// Can this column be resized during auto fitting
        /// </summary>
        internal bool AutoFit
        {
            get
            {
                return _resizable && _autoSizePolicy != ColumnAutoSizePolicy.AutoSize;
            }
        }

        /// <summary>
        /// Can this column be moved (takes into account whether the column is pinned
        /// and the state of <see cref="VirtualTree.AllowUserPinnedColumns"/>
        /// </summary>
        internal bool CanMove
        {
            get
            {
                if (_pinned)
                    return _movable && Tree.AllowUserPinnedColumns;
                else
                    return _movable;
            }
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Initialize the column styles 
        /// </summary>
        protected internal virtual void InitializeStyles()
        {
            _headerStyle.ParentStyle = Tree.HeaderStyle;
            _headerPressedStyle.ParentStyle = Tree.HeaderPressedStyle.Copy(Tree.HeaderStyle, _headerStyle);
            _headerHotStyle.ParentStyle = Tree.HeaderHotStyle.Copy(Tree.HeaderStyle, _headerStyle);
            _headerDropStyle.ParentStyle = Tree.HeaderDropStyle.Copy(Tree.HeaderStyle, _headerStyle);
            _headerPrintStyle.ParentStyle = Tree.HeaderPrintStyle.Copy(Tree.HeaderStyle, _headerStyle);

            _cellStyle.ParentStyle = Tree.RowStyle; 
            _cellOddStyle.ParentStyle = new Style(Tree.RowOddStyle, _cellStyle.Delta);
            _cellEvenStyle.ParentStyle = new Style(Tree.RowEvenStyle, _cellStyle.Delta);
            _cellPrintStyle.ParentStyle = new Style(Tree.RowPrintStyle, _cellStyle.Delta);
            _cellPrintOddStyle.ParentStyle = _cellPrintStyle.Copy(Tree.RowPrintStyle, Tree.RowPrintOddStyle);
            _cellPrintEvenStyle.ParentStyle = _cellPrintStyle.Copy(Tree.RowPrintStyle, Tree.RowPrintEvenStyle);

        }

        /// <summary>
        /// Notify clients of a change to the column
        /// </summary>
        protected virtual void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        /// <summary>
        /// Handle a change to one of the columns styles
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnStyleChanged(object sender, PropertyChangedEventArgs e)
        {
            OnPropertyChanged("Style");
        }
        
        #endregion

        #region XML Serialization

        /// <summary>
        /// Write the end-user customizable attributes (column width etc) as XML
        /// </summary>
        /// <param name="writer">The writer to write the attributes to</param>
        /// <remarks>
        /// This version of the method does not write out the enclosing element tag
        /// </remarks>
        public virtual void WriteXml(XmlWriter writer)
        {
            writer.WriteElementString("Width", Width.ToString());
            writer.WriteElementString("SortDirection", SortDirection.ToString());
            writer.WriteElementString("Active", Active.ToString());
            writer.WriteElementString("Pinned", Pinned.ToString());
            writer.WriteElementString("AutoFitWeight", AutoFitWeight.ToString());
        }

        /// <summary>
        /// Read the end-user customizable attributes (column width etc) from XML
        /// </summary>
        /// <param name="reader">The reader to read the attributes from</param>
        public virtual void ReadXml(XmlReader reader)
        {
            string element;
            try
            {
                element = reader.ReadElementString("Width");
                if (element != null && Resizable && !PrefixColumn)
                    Width = int.Parse(element);

                element = reader.ReadElementString("SortDirection");
                if (element != null && Sortable)
                    SortDirection = (ListSortDirection)Enum.Parse(typeof(ListSortDirection), element);

                element = reader.ReadElementString("Active");
                if (element != null && Hidable)
                    Active = bool.Parse(element);

                if (reader.IsStartElement("Pinned"))
                {
                    element = reader.ReadElementString("Pinned");
                    if (Tree.AllowUserPinnedColumns)
                    {
                        Pinned = bool.Parse(element);
                    }
                }

                if (reader.IsStartElement("AutoFitWeight"))
                {
                    element = reader.ReadElementString("AutoFitWeight");
                    AutoFitWeight = float.Parse(element);
                }
            }
            catch {} // ignore errors while reading

        }


        #endregion
    }
}
