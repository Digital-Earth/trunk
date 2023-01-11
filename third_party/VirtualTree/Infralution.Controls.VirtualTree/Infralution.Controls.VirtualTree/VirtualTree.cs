#region File Header
//
//      FILE:   VirtualTree.cs.
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
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.Drawing.Drawing2D;
using System.Diagnostics;
using System.Windows.Forms;
using System.Data;
using System.ComponentModel;
using System.Drawing.Design;
using System.Xml;
using System.Xml.Serialization;
using Infralution.Controls;
using Infralution.Common;
using System.Reflection;
using System.Text;
using System.Windows.Forms.VisualStyles;
using System.Globalization;
using Infralution.Controls.VirtualTree.Properties;
using NS = Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree
{

	/// <summary>
	/// A tree/listview control with support for advanced databinding, multiple <see cref="Column">Columns</see>, and 
	/// display of very large <see cref="DataSource">DataSources</see>.  See <see href="XtraGettingStarted.html">Getting Started</see>
	/// for information on how to get up and running quickly with VirtualTree.
	/// </summary>
	/// <remarks>
	/// <para>
	/// The Infralution <see cref="VirtualTree"/> is a tree/listview control with the following capabilities:
	/// <list type="bullet">
	/// <item>Advanced data binding mechanism</item>
    /// <item>True virtual loading of data on demand</item>
    /// <item>Support for multiple sortable <see cref="Column">Columns</see> per <see cref="Row"/></item>
    /// <item>Support for <see cref="Column"/> customization (moving, resizing, hiding/showing)</item>
    /// <item>Support for persisting user customizations to XML</item>
    /// <item>Inplace editing of cell values using any .NET control</item>
    /// <item>Extensive drag and drop support</item>
    /// <item>Support for multiple selection</item>
    /// </list>
	/// </para>
    /// <para>
    /// Unlike most other tree implementations <see cref="VirtualTree"/> is designed from the 
    /// ground up to be data driven.   Most other tree implementations have a "unbound" mode where the
    /// tree representation (consisting of tree nodes) is built manually by the user.  If they support
    /// databinding at all, it is built on top of this "unbound" mode and simply autogenerates the
    /// in-memory tree representation based on the Data Source.   This means that for Data Sources with very
    /// large numbers of nodes at a given level, the entire tree/node representation must be built before the
    /// tree can be displayed.  For a product database with 30,000 products this would require at least 30,000 tree 
    /// nodes being created up front to display a tree representation with products at the root level.   
    /// Not only is this approach memory and resource intensive, it is also extremely slow.
    /// </para>
    /// <para>
    /// The <see cref="VirtualTree"/> approach is radically different.   It does not have an "unbound" mode 
    /// at all.  This allows the design to leverage the power of data binding to only extract the information
    /// from the <see cref="DataSource"/> that is required to display the current state of the tree.  
    /// This means that, regardless of the size of the <see cref="DataSource"/>, the <see cref="VirtualTree"/> 
    /// only needs to build a display representation of the first 30 or so items in <see cref="DataSource"/> 
    /// (ie the typical number of rows displayed on a screen).  As the user browses the tree the information
    /// to be displayed is extracted on-demand from the <see cref="DataSource"/>.  This vastly improves 
    /// initial load time and greatly reduces the memory footprint. 
    /// </para>
    /// <para>
    /// Another key difference between <see cref="VirtualTree"/> and other tree implementations is the
    /// <see cref="VirtualTree"/> approach to data binding.   Other implementations typically require 
    /// all nodes within the tree to be data bound using the same binding (which maps the tree concepts,
    /// such as such as "Parent", "CellValue" etc, to properties of the data object).   This means that you can 
    /// only use data binding where the data is in a homogeneous format.  This is rarely the case in the real 
    /// world - which means you either have to force an unnatural flat model on your data (which is impractical
    /// for large projects) or abandon databinding and its ease of use altogether. 
    /// </para>
    /// <para>
    /// <see cref="VirtualTree"/> overcomes these problems by allowing the application designer to specify
    /// different <see cref="RowBinding">RowBindings</see> that apply to different types of items within
    /// the tree.   This allows use of <see cref="VirtualTree"/> data binding to display the complex
    /// relationships and variety of data found in the real world.   A tree representation of the standard 
    /// Microsoft sample Northwind database can, for instance, be displayed very easily using databinding.
    /// This model also allows the use of <see cref="Column.ContextSensitive"/> <see cref="Column">Columns</see>.
    /// These are <see cref="Column">Columns</see> which are displayed/hidden dynamically depending on the
    /// currently selected <see cref="Row"/>.    These can provide a great deal of power when visualizing
    /// heterogenous data. 
    /// </para>
    /// <para>
    /// <see cref="VirtualTree"/> supports binding to <see cref="DataSet">DataSets</see>, 
    /// <see cref="DataTable">DataTables</see>, <see cref="DataView">DataViews</see> or any <see cref="DataSource"/>
    /// that supports the <see cref="IList"/> interface.  If the <see cref="DataSource"/> also supports the
    /// <see cref="IBindingList"/> interface then <see cref="VirtualTree"/> will automatically update the display
    /// when the <see cref="DataSource"/> changes.
    /// </para>
    /// <para>
    /// <see cref="VirtualTree"/> also provides a lower level mechanism for programmatically binding the tree
    /// representation to the underlying data.  To use this mechanism the application designer handles the data
    /// events (or overrides the correponding methods) which retrieve the data to be displayed.   These events
    /// include:
    /// <list type="bullet">
    /// <item><see cref="GetRowData"/> - get the data for a row</item>
    /// <item><see cref="GetCellData"/> - get the data for a column of a row</item>
    /// <item><see cref="GetChildren"/> - get the child items for a given row</item>
    /// </list>
    /// This mechanism is typically used when more programmatic control is required over the appearance or behavior
    /// of the <see cref="VirtualTree"/> then can be achieved using Data Binding alone.  It can also be used to handle
    /// binding to data that does not support the <see cref="IList"/> interface.   This lower level mechanism can be
    /// mixed with data binding to give the best of both worlds.   Data binding can be used for the bulk of the work and
    /// appropriate data events handled to provide more programmatic control where required.
    /// </para>
    /// </remarks>
    /// <seealso href="XtraOverview.html">Overview</seealso>
    /// <seealso href="XtraGettingStarted.html">Getting Started</seealso>
    /// <seealso cref="RowBinding"/>
    /// <seealso cref="Row"/>
    /// <seealso cref="Column"/>
    /// <seealso cref="CellEditor"/>
    [Designer("Infralution.Controls.VirtualTree.Design.TreeDesigner, " + DesignAssembly.Name)]
    [ToolboxItem(true)]
#if CHECK_LICENSE
    [LicenseProvider(typeof(VirtualTreeLicenseProvider))]
#endif
    public class VirtualTree : WidgetControl, ISupportInitialize
    {

        #region Local Types

        /// <summary>
        /// Defines how the <see cref="MoveFocusRow"/> method handles selection
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

        #endregion

        #region Member Variables

        #region Data Binding

        /// <summary>
        ///  The current datasource for the tree.
        /// </summary>
        private object _dataSource = null;

        /// <summary>
        /// Is the control currently being initialized
        /// </summary>
        private bool _initializing = false;

        /// <summary>
        /// Are data updates currently suspended
        /// </summary>
        private int _suspendDataUpdateCount = 0;

        /// <summary>
        /// Should <see cref="UpdateRowData()"/> be called when <see cref="ResumeDataUpdate()"/> is next called
        /// </summary>
        private bool _updateRowDataRequired = false;
        
        /// <summary>
        /// Should <see cref="UpdateRows()"/> be called when <see cref="ResumeDataUpdate()"/> is next called
        /// </summary>
        private bool _updateRowsRequired = false;

        /// <summary>
        /// If true the bindings will catch and suppress exceptions while setting and getting cell values
        /// </summary>
        private bool _suppressBindingExceptions = true;
 
        /// <summary>
        /// The list of row bindings
        /// </summary>
        private RowBindingList _rowBindings;

        /// <summary>
        /// The last rowbinding returned by GetBindingForRow
        /// </summary>
        private RowBinding _cachedBinding;

        /// <summary>
        /// The last row that GetBindingForRow was called for
        /// </summary>
        private Row _cachedBindingRow;

        #endregion
     
        #region Rows

        /// <summary>
        /// Currently visible Rows indexed by the current absolute row index.
        /// </summary>
        /// <seealso cref="UpdateVisibleRows"/>
        /// <seealso cref="GetVisibleRow"/>
        private Hashtable _visibleRows = new Hashtable();

        /// <summary>
        /// Cached RowData indexed by the Row object.
        /// </summary>
        /// <seealso cref="GetDataForRow"/>
        private Hashtable _cachedRowData = new Hashtable();

        /// <summary>
        /// The individually adjusted row height (if any) indexed by row
        /// </summary>
        private Hashtable _userRowHeight = new Hashtable();

        /// <summary>
        /// Can the user adjust the heights of individual rows.
        /// </summary>
        private bool _allowIndividualRowResize = true;

        /// <summary>
        /// The root row for the tree
        /// </summary>
        private Row _rootRow = null;

        /// <summary>
        /// The row which is currently displayed at the top of the tree.
        /// </summary>
        private Row _topRow = null;      

        /// <summary>
        /// Should the root row be displayed
        /// </summary>
        private bool _showRootRow = true;

        /// <summary>
        /// Should we display row headers
        /// </summary>
        private bool _showRowHeaders = false;

        /// <summary>
        /// If true rows once created are cached
        /// </summary>
        private bool _enableRowCaching = true;

        #endregion

        #region Columns

        /// <summary>
        /// The list of columns for this tree
        /// </summary>
        private ColumnList _columns;

        /// <summary>
        /// The column to use to display the icon and connections for rows
        /// </summary>
        private Column _mainColumn;

        /// <summary>
        /// The column being used as a prefix column (if any)
        /// </summary>
        private Column _prefixColumn;

        /// <summary>
        /// Should we display column headers
        /// </summary>
        private bool _showColumnHeaders = true;

        /// <summary>
        /// Should columns widths be automatically sized to fit the available width of the control
        /// </summary>
        private bool _autoFitColumns = false;

        /// <summary>
        /// The column currently being sorted on
        /// </summary>
        private Column _sortColumn = null;

        /// <summary>
        /// The column customization form (if created)
        /// </summary>
        private ColumnCustomizeForm _columnCustomizeForm;

        /// <summary>
        /// The minimum width required for the scrollable columns
        /// </summary>
        private int _minScrollableWidth = 0;

        /// <summary>
        /// Can pinned columns be moved by the end user.
        /// </summary>
        private bool _allowUserPinnedColumns = true;

 
        #endregion

        #region Editing

        /// <summary>
        /// List of editor controls
        /// </summary>
        private CellEditorList _editors = new CellEditorList();

        /// <summary>
        /// Determines whether the use must select a cell/row before being able to edit the cell 
        /// </summary>
        private bool _selectBeforeEdit = false;

        /// <summary>
        /// Determines whether double click a cell will initiate an edit
        /// </summary>
        private bool _editOnDoubleClick = false;

        /// <summary>
        /// Determines if a cell edit will automatically be started when traversing to this control
        /// using the tab key
        /// </summary>
        private bool _editOnKeyboardFocus = false;

        /// <summary>
        /// Determines if a cell edit will automatically be started on the selected row/cell when the user
        /// starts typing
        /// </summary>
        private bool _editOnKeyPress = false;

        #endregion

        #region Selection/Focus
 
        /// <summary>
        /// Is multiple selection allowed.
        /// </summary>
        private bool _allowMultiSelect = true;

        /// <summary>
        /// How is selection displayed.
        /// </summary>
        private SelectionMode _selectionMode = SelectionMode.FullRow;

        /// <summary>
        /// Can the user select rows by dragging the mouse over them
        /// </summary>
        private bool _enableDragSelect = false;

        /// <summary>
        /// Can the user select rows by dragging the mouse over the row header
        /// </summary>
        private bool _enableRowHeaderDragSelect = false;

        /// <summary>
        /// The list of rows which have been selected by the user.
        /// </summary>
        private RowSelectionList _selectedRows;

        /// <summary>
        /// The current selected column (when <see cref="SelectionMode"/> is Cell
        /// </summary>
        private Column _selectedColumn = null;

        /// <summary>
        /// The row that currently has focus.
        /// </summary>
        private Row _focusRow = null;

        /// <summary>
        /// The row at which an extended selection was started (if any)
        /// </summary>
        private Row _anchorRow = null;

        /// <summary>
        /// The row that (by default) determines which Columns are in context
        /// </summary>
        private Row _contextRow = null;

        /// <summary>
        /// The current drag selection - the rows being added by dragging the mouse over rows
        /// </summary>
        private IList _dragSelection = null;

        /// <summary>
        /// The number of rows to scroll the tree for each mouse wheel detent
        /// </summary>
        private int _wheelDelta = DefaultWheelDelta;
        private const int DefaultWheelDelta = 3;

        /// <summary>
        /// Is the keyboard being used to navigate.
        /// </summary>
        private bool _keyboardNavigationInUse = false;

        /// <summary>
        /// Should the tree automatically scroll to display the children of a row when it is
        /// expanded by the user
        /// </summary>
        private bool _autoScrollOnExpand = true;

        /// <summary>
        /// Should the tree expand/collapse rows when the user double clicks on them
        /// </summary>
        private bool _expandOnDoubleClick = true;

        #endregion

        #region Drag and Drop

        /// <summary>
        /// The interval at which the tree is scrolled (in milliseconds) when drag and dropping.  
        /// </summary>
        private int _dropScrollTimerInterval = DefaultDropScrollTimerInterval;
        private const int DefaultDropScrollTimerInterval = 100;

        /// <summary>
        /// The interval (in milliseconds) before a row is expanded while drag and dropping
        /// </summary>
        private int _dropExpandTimerInterval = DefaultDropExpandTimerInterval;
        private const int DefaultDropExpandTimerInterval = 1500;

        #endregion

        #region Widget Management

        /// <summary>
        /// PanelWidget used to display row headers and pinned columns
        /// </summary>
        private PanelWidget _pinnedPanel;

        /// <summary>
        /// PanelWidget used to display scrollable columns
        /// </summary>
        private PanelWidget _scrollablePanel;

        /// <summary>
        /// The cell widget which we are currently editing (if any)
        /// </summary>
        private CellWidget _editWidget = null;

        #endregion

        #region Layout


        /// <summary>
        /// The number of fully visible rows which can be displayed in the available client height
        /// </summary>
        private int _numVisibleRows = 0;

        /// <summary>
        /// The offset in pixels for horizontal scrolling
        /// </summary>
        private int _horzScrollOffset = 0;

        /// <summary>
        /// The height in pixels of each row of the tree
        /// </summary>
        private int _rowHeight = DefaultRowHeight;
        private const int DefaultRowHeight = 18;

        /// <summary>
        /// The minimum height in pixels for each row of the tree
        /// </summary>
        private int _minRowHeight = DefaultMinRowHeight;
        private const int DefaultMinRowHeight = 14;

        /// <summary>
        /// The maximum height in pixels for each row of the tree
        /// </summary>
        private int _maxRowHeight = DefaultMaxRowHeight;
        private const int DefaultMaxRowHeight = 120;

        /// <summary>
        /// Can the user resize rows by dragging the row header dividers
        /// </summary>
        private bool _allowRowResize = true;

        /// <summary>
        /// The height in pixels of the column headers
        /// </summary>
        private int _headerHeight = DefaultHeaderHeight;
        private const int DefaultHeaderHeight = 18;

        /// <summary>
        /// The width in pixels of the row headers
        /// </summary>
        private int _rowHeaderWidth = DefaultRowHeaderWidth;
        private const int DefaultRowHeaderWidth = 30;

        /// <summary>
        /// The amount to indent for each level of the tree
        /// </summary>
        private int _indentWidth = DefaultIndentWidth;
        private const int DefaultIndentWidth = 20;

        /// <summary>
        /// The offset of the icon/text from the default location 
        /// </summary>
        private int _indentOffset = 0;
    
        /// <summary>
        /// Should the horizontal scroll bar be shown
        /// </summary>
        private bool _showHorzScroll = false;

        /// <summary>
        /// Should the vertical scroll bar be shown
        /// </summary>
        private bool _showVertScroll = false;

        /// <summary>
        /// Should the tree scroll as the vertical scroll thumb is moved
        /// </summary>
        private bool _trackVertScroll = true;

        /// <summary>
        /// Is layout of editor controls currently suspended
        /// </summary>
        private bool _editorLayoutSuspended = true;

        #endregion

        #region Icons

        /// <summary>
        /// The default indicator icon displayed to the user to allow them to collapse a row
        /// </summary>
        private static Icon _defaultCollapseIcon = Resources.CollapseIcon;

        /// <summary>
        /// The indicator icon displayed to the user to allow them to collapse a row
        /// </summary>
        private Icon _collapseIcon = null;

        /// <summary>
        /// The default indicator icon displayed to the user to allow them to expand a row
        /// </summary>
        private static Icon _defaultExpandIcon = Resources.ExpandIcon;

        /// <summary>
        /// The indicator icon displayed to the user to allow them to expand a row
        /// </summary>
        private Icon _expandIcon = null;

        /// <summary>
        /// The default Icon displayed for columns sorted in ascending order
        /// </summary>
        private static Icon _defaultSortAscendingIcon = Resources.SortAscendingIcon;

        /// <summary>
        /// Icon displayed for columns sorted in ascending order
        /// </summary>
        private Icon _sortAscendingIcon = _defaultSortAscendingIcon;

        /// <summary>
        /// The default Icon displayed for columns sorted in descending order
        /// </summary>
        private static Icon _defaultSortDescendingIcon = Resources.SortDescendingIcon;

        /// <summary>
        /// Icon displayed for columns sorted in descending order
        /// </summary>
        private Icon _sortDescendingIcon = _defaultSortDescendingIcon;

        /// <summary>
        /// The default focus icon displayed in row headers
        /// </summary>
        private static Icon _defaultFocusIcon = Resources.FocusIcon;

        /// <summary>
        /// The focus icon displayed in the row header of the row with focus
        /// </summary>
        private Icon _focusIcon = _defaultFocusIcon;

        /// <summary>
        /// The default error icon 
        /// </summary>
        private static Icon _defaultErrorIcon = Resources.ErrorIcon;

        /// <summary>
        /// The error icon displayed in the row header and cells
        /// </summary>
        private Icon _errorIcon = _defaultErrorIcon;

        /// <summary>
        /// Icon displayed to indicate that a column is pinned
        /// </summary>
        private Icon _pinIcon = _defaultPinIcon;

        /// <summary>
        /// The default Icon displayed to indicate that a column is pinned
        /// </summary>
        private static Icon _defaultPinIcon = Resources.PinIcon;

        #endregion

        #region Appearance

        private StyleTemplate _styleTemplate = StyleTemplate.ClassicXP;

        private Style _defaultStyle;
        private Style _defaultHeaderStyle;
        private Style _headerStyle;
        private Style _defaultHeaderPressedStyle;
        private Style _headerPressedStyle;
        private Style _defaultHeaderHotStyle;
        private Style _headerHotStyle;
        private Style _defaultHeaderDropStyle;
        private Style _headerDropStyle;
        private Style _defaultHeaderPrintStyle;
        private Style _headerPrintStyle;

        private bool _useDefaultThemedHeaders = true;
        private bool _useThemedHeaders = true;

        private Style _defaultRowStyle;
        private Style _rowStyle;
        private Style _rowOddStyle;
        private Style _rowEvenStyle;
        private Style _defaultRowSelectedStyle;
        private Style _rowSelectedStyle;
        private Style _defaultRowSelectedUnfocusedStyle;
        private Style _rowSelectedUnfocusedStyle;

        private Style _defaultRowPrintStyle;
        private Style _rowPrintStyle;
        private Style _defaultRowPrintEvenStyle;
        private Style _rowPrintEvenStyle;
        private Style _defaultRowPrintOddStyle;
        private Style _rowPrintOddStyle;

        private LineStyle _defaultLineStyle = LineStyle.Solid;
        private LineStyle _lineStyle = LineStyle.Default;

        private Color _defaultLineColor = SystemColors.WindowText;
        private Color _lineColor = Color.Empty;

        private ImageDrawMode _backgroundImageMode = ImageDrawMode.Tile;
        private bool _printBackgroundImage = true;            

        #endregion

        #region Context Menus

        /// <summary>
        /// The header context menu for the tree
        /// </summary>
        private ContextMenuStrip _headerContextMenu;

        // Is the Header Context Menu the default
        //
        private bool _defaultHeaderContextMenu = true;

        // Header Context Menu Items
        //
        private ToolStripMenuItem _sortAscendingMenuItem;
        private ToolStripMenuItem _sortDescendingMenuItem;
        private ToolStripMenuItem _bestFitMenuItem;
        private ToolStripMenuItem _bestFitAllMenuItem;
        private ToolStripMenuItem _autoFitMenuItem;
        private ToolStripMenuItem _pinnedMenuItem;
        private ToolStripMenuItem _customizeMenuItem;
        private ToolStripMenuItem _showColumnsMenuItem;
  
        /// <summary>
        /// The column the context menu was displayed for
        /// </summary>
        private Column _contextMenuColumn = null;

        /// <summary>
        /// The row the context menu was displayed for
        /// </summary>
        private Row _contextMenuRow = null;

        #endregion

        #region Components/Controls

        /// <summary>
        /// The control used to parent editor controls in the scrollable area
        /// </summary>
        private ClipControl _clipControl;

        /// <summary>
        /// Panel that contains the vertical scrollbar
        /// </summary>
        private System.Windows.Forms.Panel _vertScrollPanel;

        /// <summary>
        /// The vertical scrollbar.
        /// </summary>
        private System.Windows.Forms.VScrollBar _vertScroll;

        /// <summary>
        /// The horizontal scrollbar.
        /// </summary>
        private System.Windows.Forms.HScrollBar _horzScroll;

        /// <summary>
        /// Timer used to perform autoscrolling when dragging outside the tree.
        /// </summary>
        private Timer _dragSelectTimer;

        /// <summary>
        /// The previous tooltip associated with the control
        /// </summary>
        private string _savedToolTip;

        /// <summary>
        /// Component used to display tooltips
        /// </summary>
        private ToolTip _toolTipComponent;

        /// <summary>
        /// Required by Forms Designers
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        #endregion

        #endregion

        #region Component Designer generated code

      
        /// <summary>
        /// Releases all resources used by the VirtualTree.
        /// </summary>
        protected override void Dispose( bool disposing )
        {
            base.Dispose(disposing);
            if (disposing)
            {
                CultureManager.ApplicationUICultureChanged -= new CultureManager.CultureChangedHandler(OnApplicationUICultureChanged);
                
                if( components != null )
                    components.Dispose();

                if (_pinnedPanel != null)
                {
                    _pinnedPanel.Dispose();
                    _pinnedPanel = null;
                }
                if (_scrollablePanel != null)
                {
                    _scrollablePanel.Dispose();
                    _scrollablePanel = null;
                }
                if (_rootRow != null)
                {
                    _rootRow.Dispose();
                    _rootRow = null;
                }
                _dataSource = null;
            }
        }

        /// <summary>
        /// Intialize the properties of the control and its components
        /// </summary>
        private void InitializeComponent()
        {
            this._clipControl = new Infralution.Controls.VirtualTree.ClipControl();
            this._vertScrollPanel = new System.Windows.Forms.Panel();
            this._vertScroll = new System.Windows.Forms.VScrollBar();
            this._horzScroll = new System.Windows.Forms.HScrollBar();
            this._vertScrollPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // _clipControl
            // 
            this._clipControl.Location = new System.Drawing.Point(0, 0);
            this._clipControl.Name = "_clipControl";
            this._clipControl.Size = new System.Drawing.Size(0, 0);
            this._clipControl.TabIndex = 1;
            this._clipControl.TabStop = false;
            // 
            // _vertScrollPanel
            // 
            this._vertScrollPanel.Controls.Add(this._vertScroll);
            this._vertScrollPanel.Location = new System.Drawing.Point(0, 0);
            this._vertScrollPanel.Name = "_vertScrollPanel";
            this._vertScrollPanel.Size = new System.Drawing.Size(17, 100);
            this._vertScrollPanel.TabIndex = 0;
            this._vertScrollPanel.Visible = false;
            // 
            // _vertScroll
            // 
            this._vertScroll.Location = new System.Drawing.Point(0, 0);
            this._vertScroll.Name = "_vertScroll";
            this._vertScroll.Size = new System.Drawing.Size(17, 80);
            this._vertScroll.TabIndex = 0;
            this._vertScroll.Scroll += new System.Windows.Forms.ScrollEventHandler(this.OnVerticalScroll);
            // 
            // _horzScroll
            // 
            this._horzScroll.Location = new System.Drawing.Point(0, 0);
            this._horzScroll.Name = "_horzScroll";
            this._horzScroll.Size = new System.Drawing.Size(80, 17);
            this._horzScroll.SmallChange = 5;
            this._horzScroll.TabIndex = 1;
            this._horzScroll.Visible = false;
            this._horzScroll.Scroll += new System.Windows.Forms.ScrollEventHandler(this.OnHorizontalScroll);
            // 
            // VirtualTree
            // 
            this.AllowDrop = true;
            this.Controls.Add(this._vertScrollPanel);
            this.Controls.Add(this._horzScroll);
            this.Controls.Add(this._clipControl);
            this._vertScrollPanel.ResumeLayout(false);
            this.ResumeLayout(false);

        }
		
        #endregion
        
        #region Public Events

        /// <summary>
        /// Raised by the VirtualTree to obtain the <see cref="RowBinding"/> used to display an item.  
        /// </summary>
        /// <remarks>
        /// Handle this event if you wish to determine the RowBinding to use based on runtime conditions. 
        /// Alternatively you may override the <see cref="GetRowBinding(object, Row)"/> method.
        /// </remarks>
        /// <seealso href="XtraProgrammaticBinding.html">Programmatic Data Binding</seealso>
        [Category("Data"),
        Description("Fired by the tree to obtain the row binding used to display a given item.")]
        public event GetBindingHandler GetBinding;

        /// <summary>
        /// Raised by the VirtualTree to obtain the data required to display a row.  
        /// </summary>
        /// <remarks>
        /// Handle this event if you wish to programmatically determine the display properties 
        /// of rows in the tree instead of using databinding to do this.  
        /// Alternatively you may override the <see cref="OnGetRowData"/> method.
        /// </remarks>
        /// <seealso href="XtraProgrammaticBinding.html">Programmatic Data Binding</seealso>
        [Category("Data"),
        Description("Fired by the tree to obtain the data to display a given row.")]
        public event GetRowDataHandler GetRowData;

        /// <summary>
        /// Raised by the VirtualTree to obtain the data to display a cell (identified by
        /// a row and column). 
        /// </summary>
        /// <remarks>
        /// Handle this event if you wish to programmatically determine 
        /// the display properties of rows in the tree instead of using databinding to do this.  
        /// Alternatively you may override the <see cref="OnGetCellData"/> method.
        /// </remarks>
        /// <seealso href="XtraProgrammaticBinding.html">Programmatic Data Binding</seealso>
        [Category("Data"),
        Description("Fired by the tree to obtain the data to display in a given row/column.")]
        public event GetCellDataHandler GetCellData;
        
        /// <summary>
        /// Raised by the VirtualTree to set the value of a edited cell in the underlying datasource.  
        /// </summary>
        /// <remarks>
        /// Handle this event if you wish to programmatically handle changing edited values instead 
        /// of using databinding to do this.  
        /// Alternatively you may override the <see cref="SetValueForCell"/> method.
        /// </remarks>
        /// <seealso href="XtraProgrammaticBinding.html">Programmatic Data Binding</seealso>
        [Category("Data"),
        Description("Fired by the tree to set the value of a edited cell in the underlying datasource")]
        public event SetCellValueHandler SetCellValue;

        /// <summary>
        /// Raised by the VirtualTree before the ToolTip is displayed for a cell (identified by
        /// a row and column) to allow the CellData to be updated
        /// </summary>
        /// <remarks>
        /// Handle this event if you wish to programmatically set the ToolTip on demand (as the
        /// user hovers over the cell) rather than setting it when the cell is first
        /// displayed. This can be useful if generating the ToolTip text is computationally expensive.  
        /// Alternatively you may override the <see cref="OnGetToolTipCellData"/> method.
        /// </remarks>
        /// <seealso href="XtraProgrammaticBinding.html">Programmatic Data Binding</seealso>
        [Category("Data")]
        [Description("Fired by the tree before the ToolTip is displayed for a cell")]
        public event GetCellDataHandler GetToolTipCellData;

        /// <summary>
        /// Raised when displaying rows for the first time to determine whether children
        /// of the row should be loaded and/or expanded.  
        /// </summary>
        /// <remarks>
        /// Handle this event if you wish to  programmatically determine the child policy for a row 
        /// instead of using databinding to do this.  Alternatively you may override the 
        /// <see cref="GetChildPolicyForRow"/> method.
        /// </remarks>
        /// <seealso href="XtraProgrammaticBinding.html">Programmatic Data Binding</seealso>
        [Category("Data"),
        Description("Fired when the tree needs to determine how to display children of a row")]
        public event GetChildPolicyHandler GetChildPolicy;

        /// <summary>
        /// Raised when the VirtualTree needs to locate the children of a row.  
        /// </summary>
        /// <remarks>
        /// Handle this event if you wish to programmatically setup the relationships 
        /// between items in the tree instead of using databinding to do this. Alternatively 
        /// you may override the <see cref="GetChildrenForRow"/> method.
        /// </remarks>
        /// <seealso href="XtraProgrammaticBinding.html">Programmatic Data Binding</seealso>
        [Category("Data"),
        Description("Fired when the tree needs to find the child items for a row")]
        public event GetChildrenHandler GetChildren;

        /// <summary>
        /// Raised when the VirtualTree needs to locate the parent of a given item.   
        /// </summary>
        /// <remarks>
        /// <para>
        /// Handle this event if you wish to programmatically setup the relationships 
        /// between items in the tree instead of using databinding to do this. Alternatively 
        /// you may override the <see cref="GetParentForItem"/> method.
        /// </para>
        /// <para>
        /// This event is only used if the client application uses the <see cref="FindRow(object)"/> method to locate
        /// the Row in the tree corresponding to a given item or uses the SelectedItems property
        /// to set the selected rows.   These methods provide a simpler mechanism for selecting and
        /// identifying rows in the tree but require that every item in the must have a single parent
        /// ie an item can only appear once in the hierarchy.
        /// </para>
        /// </remarks>
        /// <seealso href="XtraProgrammaticBinding.html">Programmatic Data Binding</seealso>
        [Category("Data"),
        Description("Fired when the tree needs to find the parent item for a given item in the tree")]
        public event GetParentHandler GetParent;

        /// <summary>
        /// Raised by the VirtualTree to determine whether a given column should be displayed
        /// in the current context.  
        /// </summary>
        /// <remarks>
        /// Handle this event if you wish to programmatically determine 
        /// which columns are displayed for a given context instead of using databinding to do this.  
        /// Alternatively you may override the <see cref="ColumnInContext"/> method.
        /// </remarks>
        [Category("Behavior"),
        Description("Fired by the tree to determine whether a column should be displayed in the current context.")]
        public event GetColumnInContextHandler GetColumnInContext;

        /// <summary>
        /// Raised to determine the context menu to display for the given row.
        /// </summary>
        [Category("Behavior"),
        Description("Fired when the tree needs to display a context menu for a row")]
        public event GetContextMenuStripHandler GetContextMenuStrip;

        /// <summary>
        /// Raised by the VirtualTree to determine whether the given row is allowed
        /// to be dragged.  
        /// </summary>
        /// <remarks>
        /// Handle this event if you wish to programmatically set whether
        /// drag operations are allowed  instead of using databinding to do this. Alternatively 
        /// you may override the <see cref="AllowRowDrag"/> method.
        /// </remarks>
        [Category("Drag Drop"),
        Description("Fired to determine the given row is allowed to be dragged")]
        public event GetAllowRowDragHandler GetAllowRowDrag;

        /// <summary>
        /// Raised by the VirtualTree to determine the allowed drop locations for a given
        /// row.   
        /// </summary>
        /// <remarks>
        /// If RowDropLocation.None is returned then the row will not support
        /// dropping.  Handle this event if you wish to programmatically set the allowed
        /// drop locations instead of using databinding to do this. Alternatively 
        /// you may override the <see cref="AllowedRowDropLocations"/> method.
        /// </remarks>
        [Category("Drag Drop"),
        Description("Fired to determine the allowed drop locations for a given row")]
        public event GetAllowedRowDropLocationsHandler GetAllowedRowDropLocations;

        /// <summary>
        /// Raised by the VirtualTree to determine the drop effect when dropping an
        /// item on a given row. 
        /// </summary>
        /// <remarks>
        /// Handle this event if you wish to programmatically set the drop effect
        /// instead of using databinding to do this. Alternatively you may override the 
        /// <see cref="RowDropEffect"/> method.
        /// </remarks>
        [Category("Drag Drop"),
        Description("Fired to determine the allowed drop locations for a given row")]
        public event GetRowDropEffectHandler GetRowDropEffect;

        /// <summary>
        /// Raised by the VirtualTree when an item is dragged and dropped onto a row.
        /// </summary>
        /// <remarks>
        /// Handle this event if you wish to programmatically handle dropping data
        /// instead of using databinding to do this. Alternatively you may override the 
        /// <see cref="OnRowDrop"/> method.
        /// </remarks>
        [Category("Drag Drop"),
        Description("Fired when the user drags and drops an item onto a row")]
        public event RowDropHandler RowDrop;

        /// <summary>
        /// Raised when the sort column is changed or the sort direction of the 
        /// sort column is changed
        /// </summary>
        [Category("Behavior"),
        Description("Fired when the sort column (or sort order) is changed")] 
        public event EventHandler SortColumnChanged;

        /// <summary>
        /// Raised prior to the selection changing.   Allows the client to cancel the selection
        /// change or warn the user if the selection is very large.
        /// </summary>
        [Category("Behavior"),
        Description("Fired prior to the tree changing the selected rows/items")]
        public event SelectionChangingHandler SelectionChanging;

        /// <summary>
        /// Raised after the selection has been changed.
        /// </summary>
        [Category("Behavior"),
        Description("Fired following a change to the selected rows/items")]
        public event EventHandler SelectionChanged;

        /// <summary>
        /// Raised after the FocusRow changes.
        /// </summary>
        [Category("Behavior"),
        Description("Fired following a change to the current focus row")]
        public event EventHandler FocusRowChanged;

        /// <summary>
        /// Fired when the user presses the mouse over a <see cref="CellWidget"/>.
        /// </summary>
        /// <remarks>
        /// Provides a convenient means to allow you to add additional behaviour to the 
        /// MouseDown event.  If you wish to override the default CellWidget MouseDown 
        /// behaviour then you need derive a new CellWidget class and override the 
        /// <see cref="CellWidget.OnMouseDown"/> method.
        /// The sender is the <see cref="CellWidget"/> that was clicked on
        /// </remarks>
        [Category("Mouse"),
        Description("Fired when the user presses the mouse over a cell")]
        public event MouseEventHandler CellMouseDown;

        /// <summary>
        /// Fired when the user releases the mouse over a <see cref="CellWidget"/>.
        /// </summary>
        /// <remarks>
        /// Provides a convenient means to allow you to add additional behaviour to the 
        /// MouseUp event.  If you wish to override the default CellWidget MouseUp 
        /// behaviour then you need derive a new CellWidget class and override the 
        /// <see cref="CellWidget.OnMouseUp"/> method.
        /// The sender is the <see cref="CellWidget"/> that was clicked on
        /// </remarks>
        [Category("Mouse"),
        Description("Fired when the user releases the mouse over a cell")]
        public event MouseEventHandler CellMouseUp;

        /// <summary>
        /// Fired when the user clicks on a <see cref="CellWidget"/>.
        /// </summary>
        /// <remarks>
        /// Provides a convenient means to allow you to add additional behaviour to the 
        /// Click event.  If you wish to override the default CellWidget Click 
        /// behaviour then you need derive a new CellWidget class and override the 
        /// <see cref="CellWidget.OnClick"/> method.
        /// The sender is the <see cref="CellWidget"/> that was clicked on
        /// </remarks>
        [Category("Action"),
        Description("Fired when the user releases the mouse over a cell")]
        public event EventHandler CellClick;

        /// <summary>
        /// Fired when the user double clicks on a <see cref="CellWidget"/>.
        /// </summary>
        /// <remarks>
        /// Provides a convenient means to allow you to add additional behaviour to the 
        /// DoubleClick event.  If you wish to override the default CellWidget DoubleClick 
        /// behaviour then you need derive a new CellWidget class and override the 
        /// <see cref="CellWidget.OnDoubleClick"/> method.
        /// The sender is the <see cref="CellWidget"/> that was double clicked on
        /// </remarks>
        [Category("Action"),
        Description("Fired when the user releases the mouse over a cell")]
        public event EventHandler CellDoubleClick;

        /// <summary>
        /// Fired when a row of the tree is expanded
        /// </summary>
        [Category("Behaviour"),
        Description("Fired when a row of the tree is expanded")]
        public event RowEventHandler RowExpand;

        /// <summary>
        /// Fired when a row of the tree is collapsed
        /// </summary>
        [Category("Behaviour"),
        Description("Fired when a row of the tree is collapsed")]
        public event RowEventHandler RowCollapse;

        #endregion

        #region Widget Creation Delegates

        /// <summary>
        /// Defines the method used to create a <see cref="PanelWidget"/>
        /// </summary>
        /// <remarks>
        /// Allows you to set the method used to create widgets of this type without having
        /// to derive a new VirtualTree class and override the corresponding CreateWidget method
        /// </remarks>
        public PanelWidgetCreator PanelWidgetCreator;

        /// <summary>
        /// Defines the method used to create a <see cref="RowWidget"/>
        /// </summary>
        /// <remarks>
        /// Allows you to set the method used to create widgets of this type without having
        /// to derive a new VirtualTree class and override the corresponding CreateWidget method
        /// </remarks>
        public RowWidgetCreator RowWidgetCreator;

        /// <summary>
        /// Defines the method used to create a <see cref="CellWidget"/>
        /// </summary>
        /// <remarks>
        /// Allows you to set the method used to create widgets of this type without having
        /// to derive a new VirtualTree class and override the corresponding CreateWidget method
        /// </remarks>
        public CellWidgetCreator CellWidgetCreator;

        /// <summary>
        /// Defines the method used to create a <see cref="HeaderWidget"/>
        /// </summary>
        /// <remarks>
        /// Allows you to set the method used to create widgets of this type without having
        /// to derive a new VirtualTree class and override the corresponding CreateWidget method
        /// </remarks>
        public HeaderWidgetCreator HeaderWidgetCreator;

        /// <summary>
        /// Defines the method used to create a <see cref="ColumnHeaderWidget"/>
        /// </summary>
        /// <remarks>
        /// Allows you to set the method used to create widgets of this type without having
        /// to derive a new VirtualTree class and override the corresponding CreateWidget method
        /// </remarks>
        public ColumnHeaderWidgetCreator ColumnHeaderWidgetCreator;

        /// <summary>
        /// Defines the method used to create a <see cref="ColumnDividerWidget"/>
        /// </summary>
        /// <remarks>
        /// Allows you to set the method used to create widgets of this type without having
        /// to derive a new VirtualTree class and override the corresponding CreateWidget method
        /// </remarks>
        public ColumnDividerWidgetCreator ColumnDividerWidgetCreator;

        /// <summary>
        /// Defines the method used to create a <see cref="RowHeaderWidget"/>
        /// </summary>
        /// <remarks>
        /// Allows you to set the method used to create widgets of this type without having
        /// to derive a new VirtualTree class and override the corresponding CreateWidget method
        /// </remarks>
        public RowHeaderWidgetCreator RowHeaderWidgetCreator;

        /// <summary>
        /// Defines the method used to create a <see cref="RowDividerWidget"/>
        /// </summary>
        /// <remarks>
        /// Allows you to set the method used to create widgets of this type without having
        /// to derive a new VirtualTree class and override the corresponding CreateWidget method
        /// </remarks>
        public RowDividerWidgetCreator RowDividerWidgetCreator;

        /// <summary>
        /// Defines the method used to create an <see cref="ExpansionWidget"/>
        /// </summary>
        /// <remarks>
        /// Allows you to set the method used to create widgets of this type without having
        /// to derive a new VirtualTree class and override the corresponding CreateWidget method
        /// </remarks>
        public ExpansionWidgetCreator ExpansionWidgetCreator;

        #endregion

        #region Public Methods

        #region Initialization

        /// <summary>
        /// Handle registration of default row binding types
        /// </summary>
        static VirtualTree()
        {
            RowBinding.RegisterRowBindingType(typeof(ObjectRowBinding), "Object Binding");
            RowBinding.RegisterRowBindingType(typeof(DataRowRowBinding), "Data Row Binding");
            RowBinding.RegisterRowBindingType(typeof(DataViewRowBinding), "Data View Binding");
            RowBinding.RegisterRowBindingType(typeof(DataSetRowBinding), "Data Set Binding");
        }

        /// <summary>
        /// Initializes a new instance of the VirtualTree class.  
        /// </summary>
        public VirtualTree()
        {
            InitializeComponent();

            CultureManager.ApplicationUICultureChanged += new CultureManager.CultureChangedHandler(OnApplicationUICultureChanged);
            // create the panel widgets used to display data
            //
            _pinnedPanel = CreatePanelWidget();
            _scrollablePanel = CreatePanelWidget();
            Widgets.Add(_scrollablePanel);
            Widgets.Add(_pinnedPanel);

            // initialize collection properties
            //
            _selectedRows = new RowSelectionList();
            _selectedRows.ListChanged += new ListChangedEventHandler(OnSelectedRowsChanged);
            _columns = new ColumnList(this);
            _columns.ListChanged += new ListChangedEventHandler(OnColumnsChanged); 
            _rowBindings = new RowBindingList(this);

            #if CHECK_LICENSE

            VirtualTreeLicenseProvider.CheckLicense(Assembly.GetCallingAssembly(), this);

            #endif

            // create and initialize the styles
            //
            CreateStyles();
            InitializeStyles();

        }

         #endregion

        #region Data Binding

        /// <summary>
        /// Get or set the data source.  The data source is the item associated with the root row of the tree.
        /// </summary>
        [Category("Data")]
        [DefaultValue(null)]
        [Description("The object supplying the data to be displayed in the tree - this may be a DataSet, DataTable, DataView or any object which implements IList")]
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
                _dataSource = value;
                if (!_initializing)
                {
                    BindDataSource();
                }
            }
        }

        /// <summary>
        /// Force the tree to rebuild all <see cref="Row">Rows</see> for the current
        /// <see cref="DataSource"/>
        /// </summary>
        /// <remarks>
        /// This is used when the <see cref="DataSource"/> does not support the 
        /// <see cref="IBindingList"/> interface to manually force the tree to update
        /// its representation following a changed to underlying data.  This is the same as 
        /// calling the alternative UpdateRows method with reloadChildren set to true.
        /// </remarks>
        public virtual void UpdateRows()
        {
            UpdateRows(true);
        }

        /// <summary>
        /// Force the tree to update the <see cref="Row">Rows</see> for the current <see cref="DataSource"/>
        /// </summary>
        /// <param name="reloadChildren">
        /// If true the child items are reloaded (by calling <see cref="GetChildrenForRow"/>) for each row. 
        /// This is useful if you need to generate a completely new list of children for a row rather than 
        /// modifying the current list of children.
        /// </param>
        /// <remarks>
        /// This is used when the <see cref="DataSource"/> does not support the 
        /// <see cref="IBindingList"/> interface to manually force the tree to update
        /// its representation following a changed to underlying data.
        /// </remarks>
        public virtual void UpdateRows(bool reloadChildren)
        {
            if (DataUpdateSuspended)
            {
                this._updateRowsRequired = true;
            }
            else
            {
                if (_rootRow != null)
                {
                    _rootRow.UpdateChildren(reloadChildren, true);
                }
            }
        }

        /// <summary>
        /// Force the tree to refresh the displayed data for all rows following a change to the data source.
        /// </summary>
        /// <remarks>
        /// This method is an alternative to <see cref="UpdateRows()"/> which requires less work and can be 
        /// used if data source hierarchy is unchanged and only the displayed data needs updating. 
        /// </remarks>
        public virtual void UpdateRowData()
        {
            if (DataUpdateSuspended)
            {
                this._updateRowDataRequired = true;
            }
            else
            {
                if (_rootRow != null)
                {
                    ClearCachedRowBindings();
                    ClearCachedRowData();
                    PinnedPanel.UpdateRowData();
                    ScrollablePanel.UpdateRowData();
                    PerformLayout();
                }
            }
        }


        /// <summary>
        /// Force the tree to refresh the displayed data for the given row following a change to the data source.
        /// </summary>
        /// <remarks>
        /// This method is an alternative to <see cref="UpdateRows()"/> which requires less work and can be 
        /// used if data source hierarchy is unchanged and only the displayed data needs updating. 
        /// </remarks>
        /// <param name="row">The row to update the data for</param>
        public virtual void UpdateRowData(Row row)
        {
            // check if we actually need to do an update
            //
            if (RowWidgetCreated(row))
            {
                if (DataUpdateSuspended)
                {
                    this._updateRowDataRequired = true;
                }
                else
                {
                    if (_rootRow != null)
                    {
                        ClearCachedRowBindings();
                        ClearCachedRowData();
                        RowWidget rowWidget = PinnedPanel.GetRowWidget(row);
                        if (rowWidget != null)
                            rowWidget.UpdateData();
                        rowWidget = ScrollablePanel.GetRowWidget(row);
                        if (rowWidget != null)
                            rowWidget.UpdateData();
                        PerformLayout();
                    }
                }
            }
        }

        /// <summary>
        /// Allows you to temporarily suspend handling of <see cref="IBindingList.ListChanged"/> events.
        /// </summary>
        /// <remarks> 
        /// This is useful for improving performance when the data source does not provide 
        /// any mechanism to turn notifications off and you need to make a number of changes.
        /// This uses a counting mechanism and so can be called recursively.
        /// </remarks>
        /// <seealso cref="ResumeDataUpdate()"/>
        /// <seealso cref="DataUpdateSuspended"/>
        public virtual void SuspendDataUpdate()
        {
            _suspendDataUpdateCount++;
        }

        /// <summary>
        /// Resume normal handling of <see cref="IBindingList.ListChanged"/> events.
        /// </summary>
        /// <param name="reloadChildren">If true reloads the child items for each row if required</param>
        /// <seealso cref="SuspendDataUpdate"/>
        /// <seealso cref="DataUpdateSuspended"/>
        public virtual void ResumeDataUpdate(bool reloadChildren)
        {
            if (_suspendDataUpdateCount > 0)
            {
                _suspendDataUpdateCount--;
                if (_suspendDataUpdateCount == 0)
                {
                    if (_updateRowsRequired)
                    {
                        _updateRowsRequired = false;
                        _updateRowDataRequired = false;
                        UpdateRows(reloadChildren);
                    }
                    else if (_updateRowDataRequired)
                    {
                        _updateRowDataRequired = false;
                        UpdateRowData();
                    }
                }
            }
        }

        /// <summary>
        /// Resume normal handling of <see cref="IBindingList.ListChanged"/> events.
        /// </summary>
        /// <remarks>
        /// Calls <see cref="ResumeDataUpdate(bool)"/> with reloadChildren set to false.
        /// </remarks>
        /// <seealso cref="SuspendDataUpdate"/>
        /// <seealso cref="DataUpdateSuspended"/>
        public virtual void ResumeDataUpdate()
        {
            ResumeDataUpdate(false);
        }

        /// <summary>
        /// Is handling of <see cref="IBindingList.ListChanged"/> events currently suspended
        /// </summary>
        /// <seealso cref="SuspendDataUpdate"/>
        /// <seealso cref="ResumeDataUpdate()"/>
        [Browsable(false)]
        public bool DataUpdateSuspended
        {
            get { return (_suspendDataUpdateCount > 0); }
        }

        /// <summary>
        /// The list of row bindings for the tree which define how the DataSource is mapped to tree nodes.
        /// </summary>
        /// <remarks>
        /// Row bindings are used to establish the display properties and relationships
        /// between items in the tree.    The tree can contain heterogenous items (ie items
        /// of different types).  When displaying a row the tree searches the list of
        /// RowBindings to find an RowBinding that will "bind" to the given item.  The
        /// binding defines the relationships (parent/children) and display properties for
        /// the item.
        /// </remarks>
        /// <seealso href="XtraDatasetBinding.html">Data Binding to ADO.NET Datasets and DataViews</seealso>
        /// <seealso href="XtraObjectBinding.html">Data Binding to Object Properties</seealso>
        [Category("Data")]
        [Editor("Infralution.Controls.VirtualTree.Design.TreeEditor, " + DesignAssembly.Name, typeof(UITypeEditor))]
        [Description("The list of row bindings for the tree which define how the DataSource is mapped to tree nodes")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public RowBindingList RowBindings
        {
            get { return _rowBindings; }
        }

        /// <summary>
        /// Called by framework to determine whether row bindings should be code serialized
        /// </summary>
        /// <returns>True if the row bindings aren't empty</returns>
        private bool ShouldSerializeRowBindings()
        {
            return _rowBindings.Count > 0;
        }

        /// <summary>
        /// Get/Sets whether the Virtual Tree bindings will catch and suppress exceptions that occur while getting or setting cell values
        /// </summary>
        /// <remarks>
        /// Information about the binding exception is always written to the trace output via the <see cref="DisplayErrorMessage"/> method.
        /// </remarks>
        [Category("Behavior")]
        [Description("Get/Sets whether the Virtual Tree bindings will catch and suppress exceptions that occur while getting or setting cell values")]
        [DefaultValue(true)] 
        public virtual bool SuppressBindingExceptions
        {
            get { return _suppressBindingExceptions; }
            set { _suppressBindingExceptions = value; }
        }

        /// <summary>
        /// Returns the first RowBinding from the <see cref="RowBindings"/> collection that binds to 
        /// the given item.
        /// </summary>
        /// <remarks>
        /// This method is a convenience function that simply calls GetRowBinding(object item, Row row)
        /// with row set to null
        /// </remarks>
        /// <param name="item">The item to get the binding for</param>
        /// <returns>The first RowBinding that binds to the given item/row</returns>
        /// <seealso cref="GetBindingForItem"/>
        public RowBinding GetRowBinding(object item)
        {
            return GetRowBinding(item, null);
        }
        
        /// <summary>
        /// Returns the first RowBinding from the <see cref="RowBindings"/> collection that binds to 
        /// the given row.
        /// </summary>
        /// <remarks>
        /// This method is a convenience function that simply calls GetRowBinding(object item, Row row)
        /// </remarks>
        /// <param name="row">The row to get the binding for</param>
        /// <returns>The first RowBinding that binds to the given row</returns>
        /// <seealso cref="GetBindingForItem"/>
        public RowBinding GetRowBinding(Row row)
        {
            return GetRowBinding(row.Item, row);
        }

        /// <summary>
        /// Returns the <see cref="RowBinding"/> to use to display the given item.  Raises the 
        /// <see cref="GetBinding"/> event to allow clients to programmatically set this.
        /// </summary>
        /// <remarks>
        /// If the <see cref="GetBinding"/> event is not handled by a client the method calls 
        /// <see cref="GetRowBinding(object, Row)"/> to return the first RowBinding that binds to the given item. 
        /// </remarks>
        /// <param name="item">The item to find a binding for</param>
        /// <returns>The RowBinding to use to display the item</returns>
        /// <seealso cref="GetRowBinding(object, Row)"/>
        public virtual RowBinding GetBindingForItem(object item)
        {
            if (GetBinding == null)
            {
                return GetRowBinding(item, null);
            }
            else
            {
                GetBindingEventArgs e = new GetBindingEventArgs(item);
                GetBinding(this, e);
                return e.RowBinding;
            }
        }

        /// <summary>
        /// Returns the <see cref="RowBinding"/> to use to display the given row.  Raises the 
        /// <see cref="GetBinding"/> event to allow clients to programmatically set this.
        /// </summary>
        /// <remarks>
        /// If the <see cref="GetBinding"/> event is not handled by a client the method calls 
        /// <see cref="GetRowBinding(object, Row)"/> to return the first RowBinding that binds to the given row. 
        /// </remarks>
        /// <param name="row">The row to find a binding for</param>
        /// <returns>The RowBinding to use to display the rwo</returns>
        /// <seealso cref="GetRowBinding(object, Row)"/>
        public virtual RowBinding GetBindingForRow(Row row)
        {
            if (row == _cachedBindingRow) return _cachedBinding;            
            if (row == null) return null;
            if (GetBinding == null)
            {
                _cachedBinding = GetRowBinding(row.Item, row);
            }
            else
            {
                GetBindingEventArgs e = new GetBindingEventArgs(row);
                GetBinding(this, e);
                _cachedBinding = e.RowBinding;
            }
            _cachedBindingRow = row;
            return _cachedBinding;
        }

        /// <summary>
        /// Automatically generate columns and row bindings for the current data source
        /// </summary>
        /// <remarks>
        /// This is used by the designer to provide automatic generation of columns and
        /// row bindings for dataset and datatables.  You must call <see cref="UpdateRows()"/>
        /// to force VirtualTree to update using the newly generated bindings.
        /// </remarks>
        public virtual void AutoGenerateBindings()
        {
            object rootItem = RootItem;
            Columns.SuspendChangeNotification();
            RowBindings.SuspendChangeNotification();
            if (rootItem is DataTable)
            {
                AutoGenerateDataTableBindings(rootItem as DataTable);
            }
            else if (rootItem is DataView)
            {
                AutoGenerateDataTableBindings((rootItem as DataView).Table);
            }
            else if (rootItem is DataSet)
            {
                AutoGenerateDataSetBindings(rootItem as DataSet);
            }
            else if (rootItem is ITypedList)
            {
                AutoGenerateTypedListBindings(rootItem as ITypedList);
            }
            else if (rootItem != null)
            {
                AutoGenerateObjectBindings(rootItem.GetType());
            }
            Columns.ResumeChangeNotification();
            RowBindings.ResumeChangeNotification();
        }

        /// <summary>
        /// Automatically generate columns and row bindings for the given object type
        /// </summary>
        /// <remarks>
        /// This is used by the designer to provide automatic generation of columns and
        /// row bindings when their is no data source set.
        /// </remarks>
        public virtual void AutoGenerateBindings(Type type)
        {
            Columns.SuspendChangeNotification();
            RowBindings.SuspendChangeNotification();
            AutoGenerateObjectBindings(type);
            Columns.ResumeChangeNotification();
            RowBindings.ResumeChangeNotification();
        }

        #endregion

        #region Selection/Focus

        /// <summary>
        /// Get/Sets whether the Virtual Tree will allow the user to select multiple rows.
        /// </summary>
        /// <remarks>
        /// This does not affect the ability of the application to select multiple rows via code.
        /// </remarks>
        /// <seealso cref="SelectionMode"/>
        [Category("Behavior"), 
        Description("Determines whether the tree will allow the user to select multiple rows"),
        DefaultValue(true)] 
        public virtual bool AllowMultiSelect
        {
            get { return _allowMultiSelect; }
            set { _allowMultiSelect = value; }
        }

        /// <summary>
        /// Defines how selections are displayed 
        /// </summary>
        /// <seealso cref="AllowMultiSelect"/>
        [Category("Behavior"), 
        Description("Defines how selections are displayed"),
        DefaultValue(SelectionMode.FullRow)] 
        public virtual SelectionMode SelectionMode
        {
            get { return _selectionMode; }
            set 
            { 
                if (_selectionMode != value)
                {
                    _selectionMode = value;
                    Invalidate();
                }
            }
        }

        /// <summary>
        /// Get/Sets whether the user can drag the mouse over rows to select them.
        /// </summary>
        /// <remarks>
        /// Enabling drag selection prevents you being able to drag and drop rows.
        /// </remarks>
        /// <seealso cref="EnableRowHeaderDragSelect"/>
        [Category("Behavior"), 
        Description("Can the user can drag the mouse over rows to select them.  Disables drag and drop of rows"),
        DefaultValue(false)] 
        public virtual bool EnableDragSelect
        {
            get { return _enableDragSelect; }
            set { _enableDragSelect = value; }
        }

        /// <summary>
        /// Get/Sets whether the user can drag the mouse over row headers to select rows.
        /// </summary>
        /// <remarks>
        /// This property can be used in conjunction with <see cref="EnableDragSelect"/> to
        /// enable the user to drag select over row header region - but still drag and drop rows 
        /// by dragging over the main row region.  The reverse configuration can also be used.
        /// </remarks>
        /// <seealso cref="EnableDragSelect"/>
        [Category("Behavior"), 
        Description("Can the user can drag the mouse over row headers to select rows. Disables drag and drop of rows using row headers."),
        DefaultValue(false)] 
        public virtual bool EnableRowHeaderDragSelect
        {
            get { return _enableRowHeaderDragSelect; }
            set { _enableRowHeaderDragSelect = value; }
        }

        /// <summary>
        /// Get/Sets whether the Tree should display the root row.
        /// </summary>
        [Category("Appearance"), 
        Description("Determines whether the Tree should display the root row"),
        DefaultValue(true)] 
        public virtual bool ShowRootRow
        {
            get { return _showRootRow; }
            set 
            { 
                if (_showRootRow != value)
                {
                    _showRootRow = value; 
                    TopRowIndex = FirstRowIndex;
                    PerformLayout();
                }
            }
        }

        /// <summary>
        /// Return the list of selected rows.
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual RowSelectionList SelectedRows
        {
            get { return _selectedRows; }
        }
        
        /// <summary>
        /// Set/Get the selected row.  This provides a simple interface for handling row selection 
        /// when using AllowMultiSelect set to false.
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual Row SelectedRow
        {
            get 
            { 
                if (_selectedRows.Count > 0)
                    return _selectedRows[_selectedRows.Count-1];
                else
                    return null;
            }
            set 
            {
                SetSelectedRow(value, true);
            }
        }

        /// <summary>
        /// Select all rows in the tree
        /// </summary>
        /// <remarks>
        /// Depending on the data source size this method may take considerable time to execute because
        /// it requires Virtual Tree to retrieve all data for all rows in the tree.  It should be used
        /// with caution.
        /// </remarks>
        public virtual void SelectAllRows()
        {
            int firstRowIndex = FirstRowIndex;
            int lastRowIndex = LastRowIndex;
            if (lastRowIndex >= firstRowIndex)
            {
                List<Row> allRows = GetRows(firstRowIndex, lastRowIndex);
                SelectedRows.Set(allRows);
            }
        }

        /// <summary>
        /// Get/Set the list of selected items. This provides a simple interface for selecting
        /// rows within the tree in the case where every item is unique within the tree hierarchy
        /// </summary>
        /// <remarks>
        /// This method requires that the parent-child relationships have been setup correctly in the 
        /// RowBindings or the client has overridden the GetParentForItem method or handled the GetParent event.
        /// Displays a message in the trace console if a row cannot be found for an item, 
        /// to assist with debugging parent child relationships.
        /// </remarks>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual IList SelectedItems
        {
            get 
            { 
                ArrayList selectedItems = new ArrayList();
                foreach (Row row in _selectedRows)
                {
                    selectedItems.Add(row.Item);
                }
                return selectedItems; 
            }
            set
            {
                ArrayList rows = new ArrayList();
                foreach (object item in value)
                {
                    Row row = FindRow(item);
                    if (row != null)
                    {
                        rows.Add(row);
                    }
                }
                _selectedRows.Set(rows);
            }
        }

        /// <summary>
        /// Set/Get the selected row.  This provides a simple interface for handling row selection 
        /// when using AllowMultiSelect set to false and every item is unique within the tree hierarchy.
        /// </summary>
        /// <remarks>
        /// This method requires that the parent-child relationships have been setup correctly in the 
        /// RowBindings or the client has overridden the GetParentForItem method or handled the GetParent event.
        /// Displays a message in the trace console if a row cannot be found for an item, to assist with debugging parent child relationships.
        /// </remarks>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual object SelectedItem
        {
            get 
            { 
                Row row = SelectedRow;
                return (row == null) ? null : row.Item;
            }
            set 
            { 
                Row row = (value == null) ? null : FindRow(value);
                SelectedRow = row;
            }
        }

        /// <summary>
        /// Ensure that the row corresponding to the given item is visible.  
        /// </summary>
        /// <remarks>
        /// This method requires that the parent-child relationships have been setup correctly in the 
        /// RowBindings or the client has overridden the GetParentForItem method or handled the GetParent event.
        /// Displays a message in the trace console if a row cannot be found for an item, to assist with debugging parent child relationships.
        /// </remarks>
        public virtual void EnsureItemVisible(object item)
        {
            Row row = FindRow(item);
            if (row != null)
            {
                row.EnsureVisible();
            }
        }

        /// <summary>
        /// The currently selected column (when <see cref="SelectionMode"/> is Cell
        /// </summary>
        /// <remarks>
        /// If the currently selected column is null then then entire row is selected
        /// </remarks>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual Column SelectedColumn
        {
            get
            {
                return _selectedColumn;
            }
            set
            {
                if (value != null)
                {
                    if (SelectionMode != SelectionMode.Cell)
                        throw new InvalidOperationException("The Selection mode must be Cell to use this property");
                    if (!value.Selectable)
                        throw new ArgumentException("The given column is not selectable");
                }
                if (_selectedColumn != value)
                {
                    _selectedColumn = value;
                    Invalidate();
                    OnSelectionChanged();
                }
            }
        }

        /// <summary>
        /// Set the focus to the given row - optionally ensuring it is visible
        /// </summary>
        /// <param name="row">The row to shift focus to</param>
        /// <param name="ensureVisible">Should the tree be scrolled if required to ensure the row is visible</param>
        public virtual void SetFocusRow(Row row, bool ensureVisible)
        {
            SuspendLayout();
            try
            {
                if (row != _focusRow)
                {
                    CompleteEdit();
                    _focusRow = row;
                    if (ensureVisible && _focusRow != null)
                    {
                        _focusRow.EnsureVisible();
                    }
                    OnFocusRowChanged();
                }
                ContextRow = _focusRow;
            }
            finally
            {
                ResumeLayout(false);
            }

            // always do a perform layout so that we get a single layout performed
            // when selection changes - even if the focus row hasn't changed
            //
            PerformLayout();
        }

        /// <summary>
        /// Set the selected row - optionally ensuring it is visible
        /// </summary>
        /// <param name="row">The row to select</param>
        /// <param name="ensureVisible">Should the tree be scrolled if required to ensure the row is visible</param>
        public virtual void SetSelectedRow(Row row, bool ensureVisible)
        {
            SuspendLayout();
            try
            {
                _selectedRows.Set(row);
                SetFocusRow(row, ensureVisible);
                AnchorRow = row;
            }
            finally
            {
                ResumeLayout();
            }
        }

        /// <summary>
        /// Set/Get the row that currently has focus
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual Row FocusRow
        {
            get 
            { 
                return _focusRow; 
            }
            set 
            {
                SetFocusRow(value, true);
            }
        }

        /// <summary>
        /// Set/Get the item that currently has focus
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual object FocusItem
        {
            get
            {
                return (_focusRow != null) ? _focusRow.Item : null;
            }
            set
            {
                Row row = FindRow(value);
                if (row != null)
                {
                    SetFocusRow(row, true);
                }
            }
        }

        /// <summary>
        /// Set/Get the row that (by default) determines which columns are in context
        /// </summary>
        /// <remarks>
        /// Columns that have <see cref="Column.ContextSensitive"/> set to true are (by default)
        /// only displayed if the current <see cref="ContextRow"/> displays data in the column 
        /// (ie there is a <see cref="CellBinding"/> for the <see cref="ContextRow"/> mapping data 
        /// to the <see cref="Column"/>).   The <see cref="ContextRow"/> is generally set to the 
        /// current <see cref="FocusRow"/>.
        /// </remarks>
        /// <seealso cref="ColumnInContext"/>
        /// <seealso cref="GetColumnInContext"/>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual Row ContextRow
        {
            get
            {
                return _contextRow;
            }
            set
            {
                if (_contextRow != value)
                {
                    _contextRow = value;
                    PerformLayout();
                }
            }
        }

 
        /// <summary>
        /// The row currently being edited (if any)
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual Row EditRow
        {
            get 
            { 
                return (_editWidget == null) ? null : _editWidget.Row; 
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
            get
            {
                return _wheelDelta;
            }
            set
            {
                _wheelDelta = value;
            }
        }

        /// <summary>
        /// Should the tree automatically scroll to display the children of a row when it is
        /// expanded by the user
        /// </summary>
        [Category("Behavior"), 
        Description("Should the tree automatically scroll to display the children of a row when it is expanded by the user"),
        DefaultValue(true)] 
        public virtual bool AutoScrollOnExpand
        {
            get { return _autoScrollOnExpand; }
            set { _autoScrollOnExpand = value; }
        }


        /// <summary>
        /// Should the tree expand/collapse rows when the user double clicks on them
        /// </summary>
        [Category("Behavior"),
        Description("Should the tree expand/collapse rows when the user double clicks on them"),
        DefaultValue(true)]
        public virtual bool ExpandOnDoubleClick
        {
            get { return _expandOnDoubleClick; }
            set { _expandOnDoubleClick = value; }
        }


        /// <summary>
        /// Should the focus indicator be shown 
        /// </summary>
        [Browsable(false)]
        public virtual bool ShowFocusIndicator
        {
            get { return Focused && (ShowFocusCues || KeyboardNavigationInUse); } 
        }


        #endregion

        #region Drag and Drop

        /// <summary>
        /// The interval at which the tree is scrolled (in milliseconds) when drag and dropping.  The default is 100.
        /// </summary>
        [Category("Behavior")]
        [Description("The interval at which the tree is scrolled (in milliseconds) when drag and dropping")]
        [DefaultValue(DefaultDropScrollTimerInterval)]
        public virtual int DropScrollTimerInterval
        {
            get { return _dropScrollTimerInterval; }
            set 
            {
                if (value < 0) throw new ArgumentOutOfRangeException("value", "value must be positive");
                _dropScrollTimerInterval = value; 
            }
        }

        /// <summary>
        /// The interval (in milliseconds) before a row is expanded while drag and dropping
        /// </summary>
        [Category("Behavior")]
        [Description("The interval (in milliseconds) before a row is expanded while drag and dropping")]
        [DefaultValue(DefaultDropExpandTimerInterval)]
        public virtual int DropExpandTimerInterval
        {
            get { return _dropExpandTimerInterval; }
            set
            {
                if (value < 0) throw new ArgumentOutOfRangeException("value", "value must be positive");
                _dropExpandTimerInterval = value;
            }
        }

        #endregion

        #region Rows

        /// <summary>
        /// Get the root row for the tree.
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public Row RootRow
        {
            get { return _rootRow; }
        }
        
        /// <summary>
        /// Get the root item for the tree.
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public object RootItem
        {
            get 
            {
                if (_rootRow == null) return null;
                return _rootRow.Item; 
            }
        }

        /// <summary>
        /// Set/Get the topmost visible row.
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual Row TopRow
        {
            get 
            {
                if (_topRow == null)
                {
                    _topRow = GetRow(FirstRowIndex);
                }
                return _topRow; 
            }
            set 
            {
                if (value != _topRow)
                {
                    _topRow = value;
                    PerformLayout();
                }
            }
        }

        /// <summary>
        /// Return the absolute row index of the first row in the tree.
        /// </summary>
        /// <returns>This will be 0 or 1 depending on the value of ShowRootRow</returns>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public int FirstRowIndex
        {
            get { return _showRootRow ? 0 : 1; }
        }

        /// <summary>
        /// Return the absolute row index of the last row in the tree.
        /// </summary>
        /// <returns>The absolute row index of the last item in the tree</returns>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public int LastRowIndex
        {
            get { return (_rootRow == null) ? -1 : _rootRow.LastDescendantRowIndex; }
        }

        /// <summary>
        /// Set/Get the absolute row index of the top row displayed
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public int TopRowIndex
        {
            get 
            { 
                if (_topRow == null)
                    return FirstRowIndex;
                else
                    return _topRow.RowIndex;
            }
            set 
            {
                // check that the value is changing
                //
                if (_topRow != null && _topRow.RowIndex == value) return;
                if (value > LastRowIndex) value = LastRowIndex;
                if (value < FirstRowIndex) value = FirstRowIndex;
                TopRow = GetRow(value);
            }
        }

        /// <summary>
        /// Get/Set the absolute row index of the bottom fully visible row displayed
        /// </summary>
        /// <remarks>
        /// Setting this property when Layout is suspended may give unpredictable results if using
        /// variable height rows.  
        /// </remarks>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public int BottomRowIndex
        {
            get 
            { 
                int index = TopRowIndex + _numVisibleRows - 1;
                if (index > LastRowIndex) index = LastRowIndex;
                return index; 
            }
            set
            {
                if (_numVisibleRows > 0)
                {
                    if (value > LastRowIndex) value = LastRowIndex;

                    // for variable height rows the initial value of _numVisible rows may not be correct
                    //
                    int retry = 0;
                    while (value > BottomRowIndex && retry < 5)
                    {
                        SuspendLayout();
                        TopRowIndex = value - _numVisibleRows + 1;

                        // call OnLayout (regardless of SuspendLayout) to ensure _numVisibleRows
                        // is recalculated
                        //
                        OnLayout(new LayoutEventArgs(this, "BottomRowIndex"));
                        ResumeLayout(false);
                        retry++;
                    }
                }
            }
        }

        /// <summary>
        /// Get the bottom visible row.
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual Row BottomRow
        {
            get 
            {
                return GetVisibleRow(BottomRowIndex);; 
            }
        }

        /// <summary>
        /// Find the row corresponding to the given tree path
        /// </summary>
        /// <param name="path">
        /// The path (child,grandchild,greatgrandchild etc) to the row, relative to (but not including) 
        /// the root item
        /// </param>
        /// <returns>The row corresponding to the given path (or null if the row could not be found)</returns>
        public virtual Row FindRow(IList path)
        {
            if (_rootRow == null) return null;
            return _rootRow.FindDescendantRow(path);
        }

        /// <summary>
        /// Find the row corresponding to the given tree positional path.
        /// </summary>
        /// <param name="path">
        /// An array of child indices which specifies the path (child,grandchild,greatgrandchild etc) to 
        /// the row, relative to (but not including) the root item.
        /// </param>
        /// <returns>The row corresponding to the given positional path (or null if the row could not be found)</returns>
        public virtual Row FindRowByIndex(int[] path)
        {
            if (_rootRow == null) return null;
            return _rootRow.FindDescendantRowByIndex(path);
        }

        /// <summary>
        /// Find the row corresponding to the given item.  This provides a simple interface for  
        /// locating rows within the tree in the case where every item is unique within the tree hierarchy
        /// </summary>
        /// <remarks>
        /// This method requires that the parent-child relationships have been setup correctly in the 
        /// RowBindings or the client has overridden the GetParentForItem method or handled the GetParent event.
        /// Display a message in the trace console if a row cannot be found for an item, to assist with debugging parent child relationships.
        /// </remarks>
        /// <param name="item">The item to find the row for</param>
        /// <returns>The row corresponding to the given item (or null if not found)</returns>
        public virtual Row FindRow(object item)
        {
            if (_rootRow == null) return null;

            const int maxPathLength = 10000;

            ArrayList path = new ArrayList();
            object rootItem = _rootRow.Item;
            object child = item;
            while (child != null && !rootItem.Equals(child))
            {
                path.Insert(0, child);
                child = GetParentForItem(child);
                if (path.Count > maxPathLength)
                {
                    HandleParentingError("Recursive Parent Definition", path);
                    return null;
                }
            }
            Row row = FindRow(path);
            if (row == null)
            {
                HandleParentingError("Unable to find path", path);
            }
            return row;
        }

        /// <summary>
        /// Return a list of the rows between the given start and end rows.  
        /// The endRow may be before the startRow in which case the list is in reverse order.
        /// </summary>
        /// <param name="startRow">The row to start from</param>
        /// <param name="endRow">The row to end at</param>
        /// <returns>The list of rows between the given start and end rows (inclusive)</returns>
        public virtual List<Row> GetRows(Row startRow, Row endRow)
        {
            List<Row> rowList = new List<Row>();
            Hashtable rowTable = new Hashtable();

            // swap start and end rows so that startRow.RowIndex <= endRow.RowIndex
            //
            bool reverseOrder = false;
            if (startRow.RowIndex > endRow.RowIndex)
            {
                Row swap = startRow;
                startRow = endRow;
                endRow = swap;
                reverseOrder = true;
            }

            // the extra outside loop handles the case where calling GetRows causes
            // AutoExpand rows to be added changing the index of the EndRow.
            //
            int index = startRow.RowIndex;
            while (index <= endRow.RowIndex)
            {
                int endIndex = endRow.RowIndex;
                GetRows(index, endIndex, rowTable);
                while (index <= endIndex)
                {
                    Row row = (Row)rowTable[index];
                    if (row != null) rowList.Add(row);
                    index++;
                }
            }

            if (reverseOrder)
            {
                rowList.Reverse();
            }
            return rowList;
        }

        /// <summary>
        /// Return a list of the rows between the given start and end indexes.  
        /// </summary>
        /// <param name="startIndex">The row index to start from</param>
        /// <param name="endIndex">The row index to end at</param>
        /// <returns>The list of rows between the given start and end row indices (inclusive)</returns>
        public virtual List<Row> GetRows(int startIndex, int endIndex)
        {
            if (startIndex < 0) 
                throw new ArgumentException("startIndex must be greater than zero", "startIndex");
            if (endIndex < startIndex) 
                throw new ArgumentException("endIndex must be greater than startIndex", "endIndex");
            List<Row> rowList = new List<Row>();
            Hashtable rowTable = new Hashtable();
            GetRows(startIndex, endIndex, rowTable);
            for (int index = startIndex; index <= endIndex; index++)
            {
                Row row = (Row)rowTable[index];
                if (row == null) break;
                rowList.Add(row);
            }
            return rowList;
        }

        /// <summary>
        /// Return the <see cref="Row"/> at the given absolute row index
        /// </summary>
        /// <param name="index">The absolute row index of the row to be returned</param>
        /// <returns>The row object at the given index</returns>
        public virtual Row GetRow(int index)
        {
            Hashtable rows = new Hashtable();
            GetRows(index, index, rows);
            return (Row)rows[index];
        }

        /// <summary>
        /// Should row headers be displayed.
        /// </summary>
        [Category("Appearance"), 
        Description("Should row headers be displayed"),
        DefaultValue(false)]
        public virtual bool ShowRowHeaders
        {
            get { return _showRowHeaders; }
            set 
            { 
                _showRowHeaders = value;
                PerformLayout();
            }
        }

        /// <summary>
        /// Should rows be cached once they are created.
        /// </summary>
        [Category("Behavior"), 
        Description("Should rows be cached once they are created"),
        DefaultValue(true)]
        public virtual bool EnableRowCaching
        {
            get { return _enableRowCaching; }
            set 
            {   
                _enableRowCaching = value;
            }
        }

        #endregion

        #region Columns

        /// <summary>
        /// The list of columns to display for the tree.
        /// </summary>
        /// <seealso href="XtraColumns.html">Using Columns</seealso>
        [Category("Behavior")]
        [Editor("Infralution.Controls.VirtualTree.Design.TreeEditor, " + DesignAssembly.Name, typeof(UITypeEditor))]
        [Description("The list of columns to display for the tree")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ColumnList Columns
        {
            get { return _columns; }
        }

        /// <summary>
        /// Called by framework to determine whether columns should be code serialized
        /// </summary>
        /// <returns>True if the columns aren't empty</returns>
        private bool ShouldSerializeColumns()
        {
            return _columns.Count > 0;
        }

        /// <summary>
        /// Should column headers be displayed.
        /// </summary>
        [Category("Appearance"), 
        Description("Should column headers be displayed"),
        DefaultValue(true)]
        public virtual bool ShowColumnHeaders
        {
            get { return _showColumnHeaders; }
            set 
            { 
                if (value != _showColumnHeaders)
                {
                    _showColumnHeaders = value;
                    PerformLayout();
                }
            }
        }

        /// <summary>
        /// Should column widths be automatically adjusted to fill the available space 
        /// </summary>
        /// <remarks>
        /// <para>
        /// In AutoFitColumn mode Virtual Tree will attempt to fit all the displayed columns without the need 
        /// for a horizontal scrollbar.  If the total minimum widths for all columns exceeds the available 
        /// control width then a horizontal scrollbar will be shown. 
        /// </para>
        /// <para>
        /// Virtual Tree uses the <see cref="Column.AutoFitWeight"/> property to determine the relative 
        /// proportions of the columns.
        /// </para>
        /// </remarks>
        [Category("Behavior"), 
        Description("Should column widths be automatically adjusted to fill the available space"),
        DefaultValue(false)]
        public virtual bool AutoFitColumns
        {
            get { return _autoFitColumns; }
            set 
            { 
                if (value != _autoFitColumns)
                {
                    _autoFitColumns = value;
                    PerformLayout();
                }
            }
        }

        /// <summary>
        /// Set the column used to display the icon and connections for rows.
        /// </summary>
        /// <remarks>
        /// If set to null then the first visible column is used.  Using a <see cref="Column.Movable">Movable</see> or 
        /// <see cref="Column.ContextSensitive">ContextSensitive</see> column as the main column is likely to 
        /// cause usability issues.  The <see cref="Column.MainColumn"/> property can also be used to set this.
        /// </remarks>
        [Category("Behavior"), 
        Description("Set the column used to display the icon and connections for rows"),
        DefaultValue(null)]
        public virtual Column MainColumn
        {
            get { return _mainColumn; }
            set 
            { 
                if (value != _mainColumn)
                {
                    _mainColumn = value;
                    PerformLayout();
                }
            }
        }

        /// <summary>
        /// Set the column that displays prefix data and editor (typically a checkbox) for each row .
        /// </summary>
        /// <remarks>
        /// The prefix column defines the <see cref="Column"/> used to display data and/or an editor for each 
        /// <see cref="Row"/> within the <see cref="MainColumn"/> immediately following the row icon.  The 
        /// header for the prefix column is not displayed and it cannot be resized or customized by the user.
        /// Prefix columns are generally used to display a check box (or other state control) for each row.  
        /// The <see cref="Column.PrefixColumn"/> property can also be used to set this.
        /// </remarks>
        [Category("Behavior"), 
        Description("Set the column that defines prefix data for the row (typically a checkbox)"),
        DefaultValue(null)]
        public virtual Column PrefixColumn
        {
            get { return _prefixColumn; }
            set 
            { 
                if (value != _prefixColumn)
                {
                    _prefixColumn = value;
                    PerformLayout();
                }
            }
        }

        /// <summary>
        /// The column that is currently being used to sort data.
        /// </summary>
        [Category("Behavior"), 
        Description("The column that is currently being used to sort data)"),
        DefaultValue(null)]
        public virtual Column SortColumn
        {
            get 
            { 
                return _sortColumn; 
            }
            set 
            { 
                if (_sortColumn != value)
                {
                    _sortColumn = value;
                    OnSortColumnChanged();
                    Invalidate();
                }
            }
        }

        /// <summary>
        /// Can the pin and unpin columns and move pinned columns.
        /// </summary>
        /// <remarks>
        /// Enabling the end user to move pinned columns provides more flexibility but also
        /// may cause some confusion for inexperienced users.   If this enabled then the user
        /// can pin scrollable columns by dropping them onto the pinned column header region
        /// or using the Pin menu.
        /// </remarks>
        [Category("Behavior"), 
        Description("Can the user pin and unpin columns and move pinned columns"),
        DefaultValue(true)]
        public virtual bool AllowUserPinnedColumns
        {
            get { return _allowUserPinnedColumns; }
            set { _allowUserPinnedColumns = value; }
        }

        /// <summary>
        /// Display the column customize form for the tree.
        /// </summary>
        public virtual void ShowColumnCustomizeForm()
        {
            if (_columnCustomizeForm == null)
            {
                _columnCustomizeForm = new ColumnCustomizeForm();
                _columnCustomizeForm.Tree = this;
                _columnCustomizeForm.StartPosition = FormStartPosition.Manual;
                _columnCustomizeForm.Location = PointToScreen(new Point(Width - _columnCustomizeForm.Width, 
                                                                        Height - _columnCustomizeForm.Height));
                _columnCustomizeForm.Owner = this.FindForm(); 
            }
            _columnCustomizeForm.Show();
        }

        /// <summary>
        /// Hide the column customize form for the tree.
        /// </summary>
        public virtual void HideColumnCustomizeForm()
        {
            if (_columnCustomizeForm != null)
            {
                _columnCustomizeForm.Hide();
            }
        }

        /// <summary>
        /// Set the width of the column to the optimal value
        /// </summary>
        /// <param name="column"></param>
        public virtual void SetBestFitWidth(Column column)
        {
            if (column == null) throw new ArgumentNullException("column");
            int width = GetOptimalColumnWidth(column);
            if (AutoFitColumns && !column.Pinned)
            {
                this.AdjustColumnAutoFitWeights(column, width);
            }
            else
            {
                column.Width = width;
            }
        }

        /// <summary>
        /// Set the width of all columns to fit the currently displayed data
        /// </summary>
        public virtual void SetBestFitWidthAllColumns()
        {
            const float minWeight = 0.01F;
            SuspendLayout();
            foreach (Column column in Columns)
            {
                if (column.Visible && column.Resizable)
                {
                    int width = this.GetOptimalColumnWidth(column);
                    if (AutoFitColumns && !column.Pinned)
                    {
                        column.AutoFitWeight = Math.Max(width - column.MinWidth, minWeight);
                    }
                    else
                    {
                        column.Width = width;
                    }
                }
            }
            ResumeLayout();
        }

        /// <summary>
        /// Set the width of the column to fit the data in the given rows
        /// </summary>
        /// <param name="column">The column to set the width for</param>
        /// <param name="startRowIndex">The row to start from</param>
        /// <param name="endRowIndex">The row to end at</param>
        public virtual void SetBestFitWidth(Column column, int startRowIndex, int endRowIndex)
        {
            // get the rows to fit
            //
            Hashtable rows = new Hashtable();
            GetRows(startRowIndex, endRowIndex, rows);
            PanelWidget dummyPanel = this.CreatePanelWidget();

            int columnWidth = column.MinWidth;
            SimpleColumnList columns = Columns.GetVisibleColumns();
            using (Graphics graphics = CreateGraphics())
            {
                // for each row in the given range
                //
                for (int i = startRowIndex; i <= endRowIndex; i++)
                {
                    Row row = (Row)rows[i];
                    if (row == null) break;
                    RowWidget rowWidget = CreateRowWidget(dummyPanel, row);
                    rowWidget.ShowRowHeader = this.ShowRowHeaders;
                    rowWidget.Columns = columns;
                    rowWidget.MainColumn = this.GetMainColumn();

                    int width = rowWidget.GetOptimalColumnWidth(column, graphics);
                    if (width > columnWidth)
                    {
                        columnWidth = width;
                    }
                    rowWidget.Dispose();

                    // clear the cached row data - so that it (and especially the icons) 
                    // can be garbage collected
                    //
                    ClearCachedRowData();
                }
            }
            if (AutoFitColumns && !column.Pinned)
            {
                this.AdjustColumnAutoFitWeights(column, columnWidth);
            }
            else
            {
                column.Width = columnWidth;
            }
        }

        /// <summary>
        /// Set the width of all columns to fit the data in the given rows
        /// </summary>
        /// <param name="startRowIndex">The row to start from</param>
        /// <param name="endRowIndex">The row to end at</param>
        public virtual void SetBestFitWidthAllColumns(int startRowIndex, int endRowIndex)
        {
            // get the rows to fit
            //
            Hashtable rows = new Hashtable();
            GetRows(startRowIndex, endRowIndex, rows);
            PanelWidget dummyPanel = this.CreatePanelWidget();

            Dictionary<Column, int> columnWidth = new Dictionary<Column, int>();
            foreach (Column column in Columns)
            {
                columnWidth[column] = column.MinWidth;
            }

            SimpleColumnList columns = Columns.GetVisibleColumns();
            using (Graphics graphics = CreateGraphics())
            {
                // for each row in the given range
                //
                for (int i = startRowIndex; i <= endRowIndex; i++)
                {
                    Row row = (Row)rows[i];
                    if (row == null) break;
                    RowWidget rowWidget = CreateRowWidget(dummyPanel, row);
                    rowWidget.ShowRowHeader = this.ShowRowHeaders;
                    rowWidget.Columns = columns;
                    rowWidget.MainColumn = this.GetMainColumn();

                    // update the optimal width for the column
                    //
                    foreach (Column column in Columns)
                    {
                        int width = rowWidget.GetOptimalColumnWidth(column, graphics);
                        if (width > columnWidth[column])
                            columnWidth[column] = width;
                    }
                    rowWidget.Dispose();

                    // clear the cached row data - so that it (and especially the icons) 
                    // can be garbage collected
                    //
                    ClearCachedRowData();
                }
            }

            // now set the column widths
            //
            SuspendLayout();
            foreach (Column column in Columns)
            {
                if (column.Resizable)
                {
                    column.Width = columnWidth[column];
                }
            }
            ResumeLayout();
        }

        /// <summary>
        /// Scrolls the virtual tree horizontally if necessary to make the given
        /// column full visible
        /// </summary>
        /// <remarks>This does not make the column visible if it is not 
        /// <see cref="Column.Active">Active</see> or if it is 
        /// <see cref="Column.ContextSensitive">ContextSensitive</see> and is out
        /// of context.
        /// </remarks>
        /// <param name="column">The column to make visible</param>
        public virtual void EnsureColumnVisible(Column column)
        {
            if (column == null) throw new ArgumentNullException("column");
            
            // only have to worry about scrollable columns
            //
            if (!column.Pinned)
            {
 
                int viewLeft = HorzScrollOffset;
                int columnLeft = GetScrollableColumnOffset(column);

                // if the left of the column is out of view scroll to display it
                //
                if (viewLeft > columnLeft)
                {
                    HorzScrollOffset = columnLeft;
                }
                else
                {
                    // if the right of the column is out view scroll to display
                    // it if possible (without scrolling the left out of view)
                    //
                    int viewWidth = ClientSize.Width - _pinnedPanel.Bounds.Width;
                    if (this.ShowVertScroll)
                    {
                        viewWidth -= VertScrollBar.Width;
                    }
                    int viewRight = viewLeft + viewWidth;

                    int columnWidth = column.Width;
                    if (columnWidth > viewWidth)
                    {
                        // limit the column width so we don't scroll the left of the column
                        // out of view
                        //
                        columnWidth = viewWidth;
                    }
                    int columnRight = columnLeft + columnWidth;

                    if (viewRight < columnRight)
                    {
                        HorzScrollOffset = viewLeft + (columnRight - viewRight);
                    }
                }
            }
        }

        #endregion

        #region Printing
        
        /// <summary>
        /// Print the given rows of the tree to a rectangle within the given graphics context.
        /// </summary>
        /// <remarks>
        /// This is a low level print operation that provides maximum control over the print process.
        /// Generally you should use the <see cref="VirtualTreePrintDocument"/> to handle printing as it
        /// handles splitting columns over multiple pages if required.
        /// </remarks>
        /// <seealso href="XtraPrinting.html">Printing</seealso>
        /// <param name="graphics">The context to print to</param>
        /// <param name="rect">The rectangle bounding the area to print to</param>
        /// <param name="startRowIndex">The row to start from</param>
        /// <param name="endRowIndex">The row to end at</param>
        /// <param name="columns">The columns to print - if null then prints the currently visible columns</param>
        /// <param name="includeColumnHeaders">Should the column headers be printed</param>
        /// <param name="includeRowHeaders">Should the row headers be printed</param>
        /// <returns>The index of the last full row printed</returns>
        public virtual int Print(Graphics graphics, 
                                 Rectangle rect, 
                                 int startRowIndex, int endRowIndex, 
                                 SimpleColumnList columns, 
                                 bool includeColumnHeaders,
                                 bool includeRowHeaders)
        {
            SimpleColumnList visibleColumns = Columns.GetVisibleColumns();
            if (columns == null)
            {
                columns = visibleColumns;
            }
            if (columns.Count == 0) return -1;    // nothing to do

            // save the graphics context state
            //
            GraphicsState savedState = graphics.Save();
            DrawingUtilities.SetClip(graphics, rect);
            
            if (PrintBackgroundImage)
            {
                PaintBackgroundImage(graphics, rect);
            }
       
            // create a panel widget for printing
            //
            PanelWidget printPanel = this.CreatePanelWidget();
            printPanel.Bounds = rect;
            printPanel.Printing = true;

            // print the column headers
            //
            Rectangle bounds = new Rectangle(rect.X, rect.Y, rect.Width, this.HeaderHeight);
            if (includeColumnHeaders)
            {
                HeaderWidget headerWidget = CreateHeaderWidget(printPanel);
                headerWidget.ShowRowHeader = includeRowHeaders;
                headerWidget.Columns = columns;
                headerWidget.Bounds = bounds;
                headerWidget.OnPrint(graphics);
                bounds.Y += HeaderHeight;
            }

            // get the rows to print
            //
            Hashtable rows = new Hashtable();
            GetRows(startRowIndex, endRowIndex, rows);    
        
            // print the rows
            //
            bounds.Height = RowHeight;
            Column mainColumn = GetMainColumn();
            int lastRowIndex = -1;
            for (int i=startRowIndex; i <= endRowIndex; i++)
            {
                Row row = (Row)rows[i];
                if (row == null) break;
                RowWidget rowWidget = CreateRowWidget(printPanel, row);
                rowWidget.ShowRowHeader = includeRowHeaders;
                rowWidget.Columns = columns;
                rowWidget.MainColumn = mainColumn;

                // set the height of the row 
                //
                bounds.Height = GetUserRowHeight(row);
                if (bounds.Height == 0) 
                {
                    // hasn't been adjusted by the user so use the height from rowData.Height
                    //
                    RowData rowData = rowWidget.RowData;
                    if (rowData.AutoFitHeight)
                    {
                        // we are only printing certain columns - but AutoFitHeight needs to be done across
                        // all visible columns - so temporarily change this to calculate the optimal row height
                        //
                        rowWidget.Columns = visibleColumns;
                        rowWidget.OnLayout();
                        rowData.Height = rowWidget.GetOptimalRowHeight(graphics);
                        rowWidget.Columns = columns;
                    }
                    bounds.Height = rowData.Height;
                }
                if (bounds.Bottom > rect.Bottom) break;
               
                lastRowIndex = i;
                rowWidget.Bounds = bounds;
                rowWidget.OnPrint(graphics);
                rowWidget.Dispose();
                bounds.Y += bounds.Height;
            }

            // restore the graphics context state
            //
            graphics.Restore(savedState);

            // clear the cached row data - so that it (and especially the icons) 
            // can be garbage collected
            //
            ClearCachedRowData();
            return lastRowIndex;
        }

        #endregion

        #region Context Menus

        /// <summary>
        /// Return the context menu used for the column header area.
        /// </summary>
        /// <remarks>
        /// This allows the end user to customize the menu or even change it completely.
        /// </remarks>
        [Category("Behavior"),
        Description("The context menu displayed when the user clicks on the column headers"),
        DefaultValue(null)]
        public virtual ContextMenuStrip HeaderContextMenu
        {
            get 
            {
                if (_headerContextMenu == null)
                {
                    _headerContextMenu = CreateHeaderContextMenu(false);
                    HookHeaderContextMenuItems();
                }
                return _headerContextMenu; 
            }
            set 
            { 
                if (value != _headerContextMenu)
                {
                    _defaultHeaderContextMenu = (value != null);
                    _headerContextMenu = value;
                    if (!_initializing)
                    {
                        HookHeaderContextMenuItems();
                    }
                }
            }
        }

        /// <summary>
        /// The column that the context menu was last displayed for
        /// </summary>
        [Browsable(false)]
        public Column ContextMenuColumn
        {
            get { return _contextMenuColumn; }
        }

        /// <summary>
        /// The row that the context menu was last displayed for
        /// </summary>
        [Browsable(false)]
        public Row ContextMenuRow
        {
            get { return _contextMenuRow; }
        }

        /// <summary>
        /// Display the header context menu at the current mouse location
        /// </summary>
        /// <param name="column">The column to show the menu for</param>
        public virtual void ShowHeaderContextMenu(Column column)
        {
            if (HeaderContextMenu != null)
            {
                _contextMenuColumn = column;
                if (SortAscendingMenuItem != null)
                {
                    SortAscendingMenuItem.Enabled = (column != null && column.Sortable);
                    SortAscendingMenuItem.Checked = (column != null) && (column == SortColumn) && 
                        (column.SortDirection == ListSortDirection.Ascending);
                }
                if (SortAscendingMenuItem != null)
                {
                    SortDescendingMenuItem.Enabled = (column != null && column.Sortable);
                    SortDescendingMenuItem.Checked = (column != null) && (column == SortColumn) && 
                        (column.SortDirection == ListSortDirection.Descending);
                }
                if (BestFitMenuItem != null)
                {
                    BestFitMenuItem.Enabled = (column != null && column.Resizable);
                }
                if (AutoFitMenuItem != null)
                {
                    AutoFitMenuItem.Checked = AutoFitColumns;
                }
                if (PinnedMenuItem != null)
                {
                    PinnedMenuItem.Enabled = (column != null && column.Movable);
                    PinnedMenuItem.Checked = (column != null && column.Pinned);
                }
                HeaderContextMenu.Show(this, PointToClient(MousePosition));
            }
        }

        /// <summary>
        /// Display the context menu for the given row at the current mouse location
        /// </summary>
        /// <param name="row">The row to show the menu for</param>
        public virtual void ShowRowContextMenu(Row row)
        {
            ShowRowContextMenu(row, PointToClient(MousePosition));
        }

        /// <summary>
        /// Display the context menu strip for the given row at the given location
        /// </summary>
        /// <param name="row">The row to show the menu for</param>
        /// <param name="location">The location to display the menu</param>
        public virtual void ShowRowContextMenu(Row row, Point location)
        {
            ContextMenuStrip menu = GetContextMenuStripForRow(row);
            if (menu != null)
            {
                _contextMenuRow = row;
                menu.Show(this, location);
            }
        }

        #endregion

        #region ToolTips

        /// <summary>
        /// Return the ToolTip component used to display tool tips.
        /// </summary>
        /// <remarks>
        /// If you wish to customize the way in which Virtual Tree displays tooltips
        /// then you can add a standard ToolTip component to your form and set this
        /// property so that VirtualTree will use your tooltip component (and its 
        /// settings) to display tooltips.  If this is not set Virtual Tree will create
        /// a ToolTip component automatically with default settings.
        /// </remarks>
        [Category("Behavior"),
        Description("The tooltip component used to display tooltips"),
        DefaultValue(null)]
        public virtual ToolTip ToolTipComponent
        {
            get 
            {
                return _toolTipComponent; 
            }
            set 
            { 
                _toolTipComponent = value;
            }
        }

        /// <summary>
        /// Show the given tooltip
        /// </summary>
        /// <param name="text">The text of the tooltip to display</param>
        public virtual void ShowToolTip(string text)
        {
            if (_toolTipComponent == null)
            {
                _toolTipComponent = new ToolTip();
            }
            _savedToolTip = _toolTipComponent.GetToolTip(this);
            _toolTipComponent.SetToolTip(this, TrimTooltip(text));
            _toolTipComponent.Active = true;
        }

        /// <summary>
        /// Hide the current tooltip if any
        /// </summary>
        public virtual void HideToolTip()
        {
            if (_toolTipComponent != null)
            {
                _toolTipComponent.SetToolTip(this, _savedToolTip);
            }
        }

        /// <summary>
        /// Trim the given text for display in a tooltip
        /// </summary>
        protected virtual string TrimTooltip(string text)
        {
            float fontHeight = SystemFonts.StatusFont.Height;
            Rectangle bounds = Screen.PrimaryScreen.Bounds;
            int maxNumLines = (int)(0.5 * bounds.Height / fontHeight);
            int maxLineLength = (int)(2.0 * bounds.Width / fontHeight);
            
            string[] crlfs = { "\r\n", "\n", "\r" };
            string[] lines = text.Split(crlfs, StringSplitOptions.None);
            StringBuilder builder = new StringBuilder();
            int lineCount = Math.Min(maxNumLines, lines.Length);
            for (int li = 0; li < lineCount; li++)
            {
                if (li > 0)
                {
                    builder.AppendLine();
                }
                string line = lines[li];
                if (line.Length > maxLineLength)
                    builder.Append(line.Substring(0, maxLineLength) + "...");
                else
                    builder.Append(line);
            }
            if (lines.Length > maxNumLines)
            {
                builder.AppendLine();
                builder.Append("...");
            }
            return builder.ToString();
        }

        #endregion

        #region Editing

        /// <summary>
        /// Get the cell widget which is currently being edited
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual CellWidget EditWidget
        {
            get { return _editWidget; }
        }

        /// <summary>
        /// The available editors used for editing cell values.
        /// </summary>
        /// <seealso href="XtraEditors.html">Defining and using Editors</seealso>
        [Category("Behavior")]
        [Editor("Infralution.Controls.VirtualTree.Design.TreeEditor, " + DesignAssembly.Name, typeof(UITypeEditor))]
        [Description("The available editors used for editing cell values")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public CellEditorList Editors
        {
            get { return _editors; }
        }

        /// <summary>
        /// Called by framework to determine whether editors should be code serialized
        /// </summary>
        /// <returns>True if the editors aren't empty</returns>
        private bool ShouldSerializeEditors()
        {
            return _editors.Count > 0;
        }

        /// <summary>
        /// Get/Set whether the user must first select a row before editing cells in the row
        /// </summary>
        /// <remarks>
        /// Turning this option on gives Windows Explorer like behavior.  This can also be used in
        /// conjunction with <see cref="EditOnKeyPress"/> to provide Excel like behaviour where you
        /// can drag select cells and rows but still initiate editing with a key press.
        /// </remarks>
        [Category("Behavior"),
        Description("Determines whether the user must first select a row before editing cells in the row"),
        DefaultValue(false)]   
        public virtual bool SelectBeforeEdit
        {
            get { return _selectBeforeEdit; }
            set { _selectBeforeEdit = value; }
        }

        /// <summary>
        /// Determines whether double clicking a cell will initiate an edit
        /// </summary>
        /// <remarks>
        /// This option overrides <see cref="ExpandOnDoubleClick"/> for editable cells if set.
        /// </remarks>
        [Category("Behavior"),
         Description("Determines whether double clicking a cell will initiate an edit"),
         DefaultValue(false)]
        public virtual bool EditOnDoubleClick
        {
            get { return _editOnDoubleClick; }
            set { _editOnDoubleClick = value; }
        }

        /// <summary>
        /// Determines if a cell edit will automatically be started when the control gets focus via keyboard traversal.
        /// </summary>
        [Category("Behavior")]
        [Description("Determines if a cell edit will automatically be started when the control gets focus via keyboard traversal")]
        [DefaultValue(false)]   
        public virtual bool EditOnKeyboardFocus
        {
            get { return _editOnKeyboardFocus; }
            set { _editOnKeyboardFocus = value; }
        }

        /// <summary>
        /// Get/Set whether a cell edit will be started for the current row/cell when the user starts typing
        /// </summary>
        /// <remarks>
        /// Generally this is used in conjunction with <see cref="SelectBeforeEdit"/> to allow easy selection
        /// of rows and cells while still allowing editing to be initiated with a key stroke.
        /// </remarks>
        [Category("Behavior")]
        [Description("Determines whether a cell edit will be started for the current row/cell when the user starts typing")]
        [DefaultValue(false)]
        public virtual bool EditOnKeyPress
        {
            get { return _editOnKeyPress; }
            set { _editOnKeyPress = value; }
        }

        /// <summary>
        /// Edit the first cell in the current focus row
        /// </summary>
        /// <returns>True if an edit could be started</returns>
        public virtual bool EditFirstCellInFocusRow()
        {
            CellWidget cellWidget = GetFirstEditableCellWidgetInRow(FocusRow);
            if (cellWidget != null)
            {
                EnsureColumnVisible(cellWidget.Column);
            }
            StartEdit(cellWidget); 
            return (cellWidget != null);
        }

        /// <summary>
        /// Edit the current cell in the current focus row
        /// </summary>
        /// <returns>True if an edit could be started</returns>
        public virtual bool EditCurrentCellInFocusRow()
        {
            if (SelectedColumn != null)
            {
                CellWidget cellWidget = GetCellWidget(FocusRow, SelectedColumn);
                if (cellWidget.Editable)
                {
                    StartEdit(cellWidget);
                    return true;
                }
            }
            return false;
        }

        /// <summary>
        /// Edit the last cell in the current focus row
        /// </summary>
        /// <returns>True if an edit could be started</returns>
        public virtual bool EditLastCellInFocusRow()
        {
            CellWidget cellWidget = GetLastEditableCellWidgetInRow(FocusRow);
            if (cellWidget != null)
            {
                EnsureColumnVisible(cellWidget.Column);
            }
            StartEdit(cellWidget); 
            return (cellWidget != null);
        }

        /// <summary>
        /// Edit the next cell in the tab order
        /// </summary>
        /// <returns>True if an edit could be started</returns>
        public virtual bool EditNextCell()
        {
            // get the current edit widget
            //
            CellWidget currentCell = EditWidget;

            // Finish editing the current cell.  
            //
            if (!CompleteEdit()) return false;
            return EditNext(currentCell);
        }

        /// <summary>
        /// Edit the prior cell in the tab order
        /// </summary>
        /// <returns>True if an edit could be started</returns>
        public virtual bool EditPriorCell()
        {
            // get the current edit widget
            //
            CellWidget currentCell = EditWidget;

            // Finish editing the current cell.  
            //
            if (!CompleteEdit()) return false;
            return EditPrior(currentCell);
        }

        /// <summary>
        /// Edit the next cell the same column
        /// </summary>
        /// <returns>True if an edit could be started</returns>
        public virtual bool EditNextCellInColumn()
        {
            // get the current edit widget
            //
            CellWidget currentCell = EditWidget;

            // Finish editing the current cell.  
            //
            if (CompleteEdit())
            {
                if (MoveFocusRow(1, KeyboardSelectionMode.MoveSelection))
                {
                    return EditCurrentCellInFocusRow();
                }
            }
            return false;
        }

        /// <summary>
        /// Edit the previous cell the same column
        /// </summary>
        /// <returns>True if an edit could be started</returns>
        public virtual bool EditPriorCellInColumn()
        {
            // get the current edit widget
            //
            CellWidget currentCell = EditWidget;

            // Finish editing the current cell.  
            //
            if (CompleteEdit())
            {
                if (MoveFocusRow(-1, KeyboardSelectionMode.MoveSelection))
                {
                    return EditCurrentCellInFocusRow();
                }
            }
            return false;
        }


        /// <summary>
        /// Abandon editing the current cell
        /// </summary>
        public virtual void AbandonEdit()
        {
            if (_editWidget != null)
            {
                _editWidget.AbandonEdit();
                _editWidget = null;
            }
        }

        /// <summary>
        /// Complete editing the current cell
        /// </summary>
        /// <returns>True if editing could be succesfully completed</returns>
        public virtual bool CompleteEdit()
        {
            bool completed = true;
            if (_editWidget != null)
            {
                // change focus to generate any validation events for the edit
                // control
                //
                if (_editWidget.ContainsFocus())
                {
                   completed = this.Focus();
                }

                // validation may change the current edit widget
                //
                if (completed && _editWidget != null)
                {
                    _editWidget.CompleteEdit();
                    _editWidget = null;
                }

                // ensure we repaint the whole control so that the focus indicator gets
                // repainted if required
                //
                Invalidate();
            }
            return completed;
        }


        #endregion

        #region Layout

        /// <summary>
        /// Get/Set the height of each row
        /// </summary>
        [Category("Layout")]
        [DefaultValue(DefaultRowHeight)]
        [Description("The height of each row in the tree")] 
        [Localizable(true)]
        public virtual int RowHeight
        {
            get { return _rowHeight; }
            set
            {
                if (value < MinRowHeight)
                {
                    value = MinRowHeight;
                }
                _rowHeight = value;
                UpdateRowData();
            }
        }

        /// <summary>
        /// Get/Set the minimum height that the user can resize a row to
        /// </summary>
        /// <seealso cref="AllowRowResize"/>
        [Category("Layout")]
        [DefaultValue(DefaultMinRowHeight)]
        [Description("The minimum height that the user can resize a row to")] 
        [Localizable(true)]
        public virtual int MinRowHeight
        {
            get { return _minRowHeight; }
            set
            {
                if (_minRowHeight < 5)
                    throw new ArgumentOutOfRangeException("MinRowHeight", "Value must be greater than 5");
                _minRowHeight = value;
                if (RowHeight < value)
                {
                    RowHeight = value;
                }
            }
        }

        /// <summary>
        /// Get/Set the maximum height that the user can resize a row to
        /// </summary>
        [Category("Layout")]
        [DefaultValue(DefaultMaxRowHeight)]
        [Description("The minimum height that the user can resize a row to")] 
        [Localizable(true)]
        public virtual int MaxRowHeight
        {
            get { return _maxRowHeight; }
            set
            {
                _maxRowHeight = value;
            }
        }

        /// <summary>
        /// Can the user change the <see cref="RowHeight"/> by dragging the RowHeader dividers
        /// </summary>
        /// <remarks>
        /// For the user to be able to resize the <see cref="RowHeight"/> you must also set
        /// <see cref="ShowRowHeaders"/> to be true.
        /// </remarks>
        /// <seealso cref="ShowRowHeaders"/> 
        /// <seealso cref="MinRowHeight"/>
        /// <seealso cref="MaxRowHeight"/>
        [Category("Behavior"),
        DefaultValue(true),
        Description("Can the user resize the row height by dragging the RowHeader dividers")] 
        public virtual bool AllowRowResize
        {
            get { return _allowRowResize; }
            set
            {
                _allowRowResize = value;
                PerformLayout();
            }
        }

        /// <summary>
        /// Can the user change the height of individual rows by dragging the RowHeader dividers
        /// </summary>
        /// <remarks>
        /// If this is set to false then dragging the Row Header dividers changes the default height
        /// for all rows.  If set to true the user can still change the default height by holding
        /// down the control key.  Note that changes a user makes to the height of an individual row
        /// override the 
        /// </remarks>
        /// <seealso cref="AllowRowResize"/> 
        /// <seealso cref="ShowRowHeaders"/> 
        /// <seealso cref="MinRowHeight"/>
        /// <seealso cref="MaxRowHeight"/>
        [Category("Behavior"),
        DefaultValue(true),
        Description("Can the user resize individual rows by dragging the RowHeader dividers")] 
        public virtual bool AllowIndividualRowResize
        {
            get { return _allowIndividualRowResize; }
            set { _allowIndividualRowResize = value; }
        }

        /// <summary>
        /// Returns the user adjusted height for the given row (if any)
        /// </summary>
        /// <param name="row">The row to get the height for</param>
        /// <returns>The user adjusted height or zero if the row has not been adjusted</returns>
        /// <remarks>
        /// If <see cref="AllowIndividualRowResize"/> and <see cref="AllowRowResize"/> are set to
        /// true then the user can resize an individual row by dragging the Row Header divider.
        /// This method returns the user specified height for a given row if any.
        /// </remarks>
        /// <seealso cref="AllowIndividualRowResize"/>
        /// <seealso cref="SetUserRowHeight"/>
        /// <seealso cref="ClearUserRowHeights"/>
        public virtual int GetUserRowHeight(Row row)
        {
            object height = _userRowHeight[row];
            return (height == null) ? 0 : (int)height;
        }

        /// <summary>
        /// Sets the user adjusted height for the given row 
        /// </summary>
        /// <param name="row">The row to set the height for</param>
        /// <param name="height">The height of the row to set</param>
        /// <remarks>
        /// If <see cref="AllowIndividualRowResize"/> and <see cref="AllowRowResize"/> are set to
        /// true then the user can resize an individual row by dragging the Row Header divider.
        /// This method sets the user specified height for a given row.
        /// </remarks>
        /// <seealso cref="AllowIndividualRowResize"/>
        /// <seealso cref="GetUserRowHeight"/>
        /// <seealso cref="ClearUserRowHeights"/>
        public virtual void SetUserRowHeight(Row row, int height)
        {
            _userRowHeight[row] = height;
            PerformLayout();
        }
 
        /// <summary>
        /// Clears any individual row height adjustments made by the user
        /// </summary>
        /// <remarks>
        /// If <see cref="AllowIndividualRowResize"/> and <see cref="AllowRowResize"/> are set to
        /// true then the user can resize an individual row by dragging the Row Header divider.
        /// This method allows you to clear any user ajdustments made to individual row heights and
        /// revert to the standard row heights.
        /// </remarks>
        /// <seealso cref="AllowIndividualRowResize"/>
        /// <seealso cref="GetUserRowHeight"/>
        /// <seealso cref="SetUserRowHeight"/>
        public virtual void ClearUserRowHeights()
        {
            _userRowHeight.Clear();
            PerformLayout();
        }

        /// <summary>
        /// Clears the individual row height adjustments made by the user for the given row
        /// </summary>
        /// <remarks>
        /// If <see cref="AllowIndividualRowResize"/> and <see cref="AllowRowResize"/> are set to
        /// true then the user can resize an individual row by dragging the Row Header divider.
        /// This method allows you to clear any user ajdustments made to the given row and 
        /// revert to the standard row height.
        /// </remarks>
        /// <seealso cref="AllowIndividualRowResize"/>
        /// <seealso cref="GetUserRowHeight"/>
        /// <seealso cref="SetUserRowHeight"/>
        public virtual void ClearUserRowHeight(Row row)
        {
            _userRowHeight.Remove(row);
            PerformLayout();
        }

        /// <summary>
        /// Get/Set the height of the column headers
        /// </summary>
        [Category("Layout")]
        [DefaultValue(DefaultHeaderHeight)]
        [Description("The height of the column header")] 
        [Localizable(true)]
        public virtual int HeaderHeight
        {
            get { return _headerHeight; }
            set
            {
                _headerHeight = value;
                PerformLayout();
            }
        }

        /// <summary>
        /// Get/Set the width of the row headers
        /// </summary>
        [Category("Layout")]
        [DefaultValue(DefaultRowHeaderWidth)]
        [Description("The width of the row headers")] 
        [Localizable(true)]
        public virtual int RowHeaderWidth
        {
            get { return _rowHeaderWidth; }
            set
            {
                _rowHeaderWidth = value;
                PerformLayout();
            }
        }

        /// <summary>
        /// Return the number of rows that can be displayed in the available space
        /// </summary>
        [Browsable(false)]
        public int NumVisibleRows
        {
            get { return _numVisibleRows; }
        }

        /// <summary>
        /// Get/Set the amount to indent for each level of the tree
        /// </summary>
        [Category("Layout")]
        [DefaultValue(DefaultIndentWidth)]
        [Description("The amount to indent for each level of the tree")] 
        [Localizable(true)]
        public virtual int IndentWidth
        {
            get { return _indentWidth; }
            set 
            {
                if (value < 0) throw new ArgumentOutOfRangeException("IndentWidth", "IndentWidth must be a positive value");
                if (_indentWidth != value)
                {
                    _indentWidth = value;
                    PerformLayout();
                }
            }
        }

        /// <summary>
        /// The offset of icon/text from the default location specified by <see cref="IndentWidth"/>
        /// </summary>
        /// <remarks>
        /// By default the text/icon of a row is positioned to align with the location of the connections
        /// and expansion indicators of its child rows (specified by <see cref="IndentWidth"/>.  This
        /// property allows you to vary this.   The value may be positive or negative and shifts the location
        /// of the row icon/text relative to its child connections.
        /// </remarks>
        [Category("Layout")]
        [DefaultValue(0)]
        [Description("The offset of row icon/text from the default location specified by IndentWidth")]
        [Localizable(true)]
        public virtual int IndentOffset
        {
            get { return _indentOffset; }
            set
            {
                if (_indentOffset != value)
                {
                    _indentOffset = value;
                    PerformLayout();
                }
            }
        }

        /// <summary>
        /// Return the cumulative width of all the currently displayed columns
        /// </summary>
        [Browsable(false)]
        public virtual int TotalColumnWidth
        {
            get
            {
                int width = 0; 
                foreach (Column column in Columns)
                {
                    if (column.Visible && column != _prefixColumn)
                    {
                        width += column.Width;
                    }
                }
                return width;
            }
        }

        /// <summary>
        /// Set/Get the current horizontal scrolling offset in pixels.
        /// </summary>
        [Browsable(false),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual int HorzScrollOffset
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
        /// Get/Set the width of the row headers
        /// </summary>
        [Category("Behavior"),
        DefaultValue(true),
        Description("Should the displayed rows scroll as the vertical scroll thumb is moved")] 
        public virtual bool TrackVertScroll
        {
            get { return _trackVertScroll; }
            set { _trackVertScroll = value; }
        }


        #endregion

        #region Appearance

        /// <summary>
        /// Set the default style properties, icons and line style based to the values for the given template 
        /// </summary>
        /// <remarks>
        /// This provides a simple mechanism of changing the overall appearance of your application with one
        /// single setting.   Individual styles and properties can still be overridden to provide custom look
        /// and feel.   <see cref="NS.StyleTemplate.ClassicXP"/> to achieve the same default 
        /// appearance as previous
        /// versions of Virtual Tree.
        /// </remarks>
        [Category("Appearance")]
        [DefaultValue(StyleTemplate.ClassicXP)]
        [Description("Set the default style properties, icons and line style to the values for the given template")]
        [RefreshProperties(RefreshProperties.Repaint)]
        public StyleTemplate StyleTemplate
        {
            get { return _styleTemplate; }
            set
            {
                _styleTemplate = value;
                InitializeStyles();
                PerformLayout();
            }
        }

        #region Icons

        /// <summary>
        /// Set/Get the icon that the tree uses to allow the user to collapse a row
        /// </summary>
        [Category("Appearance"),
        Description("The icon displayed by the tree to allow the user to collapse a row")] 
        public virtual Icon CollapseIcon
        {
            get { return (_collapseIcon == null) ? _defaultCollapseIcon : _collapseIcon; }
            set 
            { 
                _collapseIcon = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Called by framework to determine whether the icon should be serialised
        /// </summary>
        /// <returns>True if the icon is not the default</returns>
        private bool ShouldSerializeCollapseIcon()
        {
            return _collapseIcon != null;
        }

        /// <summary>
        /// Called by framework to reset the default icon
        /// </summary>
        public virtual void ResetCollapseIcon()
        {
            _collapseIcon = null;
        }

        /// <summary>
        /// Set/Get the icon that the tree uses to allow the user to expand a row
        /// </summary>
        [Category("Appearance"),
        Description("The icon displayed by the tree to allow the user to expand a row")] 
        public virtual Icon ExpandIcon
        {
            get 
            { 
                return (_expandIcon == null) ? _defaultExpandIcon : _expandIcon; 
            }
            set 
            { 
                _expandIcon = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Called by framework to determine whether the icon should be serialised
        /// </summary>
        /// <returns>True if the icon is not the default</returns>
        private bool ShouldSerializeExpandIcon()
        {
            return _expandIcon != null;
        }

        /// <summary>
        /// Called by framework to reset the default icon
        /// </summary>
        public virtual void ResetExpandIcon()
        {
            _expandIcon = null;
        }

        /// <summary>
        /// Set/Get the icon displayed for columns sorted in ascending order
        /// /// </summary>
        [Category("Appearance"),
        Description("The icon displayed for columns sorted in ascending order")] 
        public virtual Icon SortAscendingIcon
        {
            get { return _sortAscendingIcon; }
            set 
            { 
                if (value == null) 
                    value = _defaultSortAscendingIcon;
                _sortAscendingIcon = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Called by framework to determine whether the icon should be serialised
        /// </summary>
        /// <returns>True if the icon is not the default</returns>
        private bool ShouldSerializeSortAscendingIcon()
        {
            return _sortAscendingIcon != _defaultSortAscendingIcon;
        }

        /// <summary>
        /// Called by framework to reset the default icon
        /// </summary>
        public virtual void ResetSortAscendingIcon()
        {
            _sortAscendingIcon = _defaultSortAscendingIcon;
        }

        /// <summary>
        /// Set/Get the icon displayed for columns sorted in descending order
        /// /// </summary>
        [Category("Appearance"),
        Description("The icon displayed for columns sorted in descending order")] 
        public virtual Icon SortDescendingIcon
        {
            get { return _sortDescendingIcon; }
            set 
            { 
                if (value == null) 
                    value = _defaultSortDescendingIcon;
                _sortDescendingIcon = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Called by framework to determine whether the icon should be serialised
        /// </summary>
        /// <returns>True if the icon is not the default</returns>
        private bool ShouldSerializeSortDescendingIcon()
        {
            return _sortDescendingIcon != _defaultSortDescendingIcon;
        }

        /// <summary>
        /// Called by framework to reset the default icon
        /// </summary>
        public virtual void ResetSortDescendingIcon()
        {
            _sortDescendingIcon = _defaultSortDescendingIcon;
        }

        /// <summary>
        /// Set/Get the icon displayed in the RowHeader when the <see cref="Row"/> has
        /// focus
        /// </summary>
        [Category("Appearance"),
        Description("The icon displayed in the row header when a row has focus")] 
        public virtual Icon FocusIcon
        {
            get { return _focusIcon; }
            set 
            { 
                _focusIcon = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Called by framework to determine whether the icon should be serialised
        /// </summary>
        /// <returns>True if the icon is not the default</returns>
        private bool ShouldSerializeFocusIcon()
        {
            return _focusIcon != _defaultFocusIcon;
        }

        /// <summary>
        /// Called by framework to reset the default icon
        /// </summary>
        public virtual void ResetFocusIcon()
        {
            _focusIcon = _defaultFocusIcon;
        }

        /// <summary>
        /// Set/Get the icon displayed in the RowHeader and Cells to indicate an error in the
        /// underlying data
        /// </summary>
        [Category("Appearance"),
        Description("The icon displayed in the row header and cells to indicate an error in the data")] 
        public virtual Icon ErrorIcon
        {
            get { return _errorIcon; }
            set 
            { 
                if (value == null)
                    throw new ArgumentNullException("ErrorIcon");
                if (value != _errorIcon)
                {
                    _errorIcon = value;
                    Invalidate();
                }
            }
        }

        /// <summary>
        /// Called by framework to determine whether the icon should be serialised
        /// </summary>
        /// <returns>True if the icon is not the default</returns>
        private bool ShouldSerializeErrorIcon()
        {
            return _errorIcon != _defaultErrorIcon;
        }

        /// <summary>
        /// Called by framework to reset the default icon
        /// </summary>
        public virtual void ResetErrorIcon()
        {
            _errorIcon = _defaultErrorIcon;
        }


        /// <summary>
        /// Set/Get the icon displayed in the Column header to indicate that a column is pinned
        /// </summary>
        [Category("Appearance"),
        Description("The icon displayed in the Column header to indicate that a column is pinned")] 
        public virtual Icon PinIcon
        {
            get { return _pinIcon; }
            set 
            { 
                if (value == null)
                    throw new ArgumentNullException("PinIcon");
                if (value != _pinIcon)
                {
                    _pinIcon = value;
                    Invalidate();
                }
            }
        }

        /// <summary>
        /// Called by framework to determine whether the icon should be serialised
        /// </summary>
        /// <returns>True if the icon is not the default</returns>
        private bool ShouldSerializePinIcon()
        {
            return _pinIcon != _defaultPinIcon;
        }

        /// <summary>
        /// Called by framework to reset the default icon
        /// </summary>
        public virtual void ResetPinIcon()
        {
            _pinIcon = _defaultPinIcon;
        }


        #endregion


        #region Header Styles

        /// <summary>
        /// The drawing attributes to use for headers
        /// </summary>
        /// <remarks>
        /// <para>
        /// If <see cref="UseThemedHeaders"/> is true then the background of the headers
        /// are drawn using the current XP Theme rather than the style background properties.
        /// </para>
        /// <para>
        /// The set accessor sets the <see cref="Style.Delta"/> for the style but does not 
        /// change the style reference or style hierarchy.
        /// </para>
        /// </remarks>
        [Category("Appearance")]
        [Description("The drawing attributes to use for headers")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style HeaderStyle
        {
            get { return _headerStyle; }
            set
            {
                if (value == null) throw new ArgumentNullException();
                _headerStyle.Delta = value.Delta;
            }
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
        /// The drawing attributes to use for headers when pressed
        /// </summary>
        /// <remarks>
        /// <para>
        /// If <see cref="UseThemedHeaders"/> is true then the background of the headers
        /// are drawn using the current XP Theme rather than the style background properties.
        /// </para>
        /// <para>
        /// The set accessor sets the <see cref="Style.Delta"/> for the style but does not 
        /// change the style reference or style hierarchy.
        /// </para>
        /// </remarks>
        [Category("Appearance")]
        [Description("The drawing attributes to use for headers when pressed")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style HeaderPressedStyle
        {
            get { return _headerPressedStyle; }
            set
            {
                if (value == null) throw new ArgumentNullException();
                _headerPressedStyle.Delta = value.Delta;
            }
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
        public virtual void ResetHeaderPressedStyle()
        {
            _headerPressedStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use for headers when mouse is over the header
        /// </summary>
        /// <remarks>
        /// <para>
        /// If <see cref="UseThemedHeaders"/> is true then the background of the headers
        /// are drawn using the current XP Theme rather than the style background properties.
        /// </para>
        /// <para>
        /// The set accessor sets the <see cref="Style.Delta"/> for the style but does not 
        /// change the style reference or style hierarchy.
        /// </para>
        /// </remarks>
        [Category("Appearance")]
        [Description("The drawing attributes to use for headers when mouse is over the header")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style HeaderHotStyle
        {
            get { return _headerHotStyle; }
            set
            {
                if (value == null) throw new ArgumentNullException();
                _headerHotStyle.Delta = value.Delta;
            }
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
        public virtual void ResetHeaderHotStyle()
        {
            _headerHotStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use for headers when dragging and dropping columns
        /// </summary>
        /// <remarks>
        /// <para>
        /// If <see cref="UseThemedHeaders"/> is true then the background of the headers
        /// are drawn using the current XP Theme rather than the style background properties.
        /// </para>
        /// <para>
        /// The set accessor sets the <see cref="Style.Delta"/> for the style but does not 
        /// change the style reference or style hierarchy.
        /// </para>
        /// </remarks>
        [Category("Appearance")]
        [Description("The drawing attributes to use for headers when dragging and dropping columns")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style HeaderDropStyle
        {
            get { return _headerDropStyle; }
            set
            {
                if (value == null) throw new ArgumentNullException();
                _headerDropStyle.Delta = value.Delta;
            }
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
        public virtual void ResetHeaderDropStyle()
        {
            _headerDropStyle.Reset();
        }
  
        /// <summary>
        /// The drawing attributes to use when printing headers
        /// </summary>
        /// <remarks>
        /// The set accessor sets the <see cref="Style.Delta"/> for the style but does not 
        /// change the style reference or style hierarchy.
        /// </remarks>
        /// <seealso href="XtraPrinting.html">Printing</seealso>
        [Category("Appearance")]
        [Description("The drawing attributes to use when printing headers")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style HeaderPrintStyle
        {
            get { return _headerPrintStyle; }
            set
            {
                if (value == null) throw new ArgumentNullException();
                _headerPrintStyle.Delta = value.Delta;
            }
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
        public virtual void ResetHeaderPrintStyle()
        {
            _headerPrintStyle.Reset();
        }

        #endregion

        #region RowStyles

        /// <summary>
        /// The drawing attributes to use for rows
        /// </summary>
        /// <remarks>
        /// The set accessor sets the <see cref="Style.Delta"/> for the style but does not 
        /// change the style reference or style hierarchy.
        /// </remarks>
        [Category("Appearance")]
        [Description("The drawing attributes to use for rows")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style RowStyle
        {
            get { return _rowStyle; }
            set 
            {
                if (value == null) throw new ArgumentNullException();
                _rowStyle.Delta = value.Delta; 
            }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeRowStyle()
        {
            return _rowStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetRowStyle()
        {
            _rowStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use for odd rows
        /// </summary>
        /// <remarks>
        /// The set accessor sets the <see cref="Style.Delta"/> for the style but does not 
        /// change the style reference or style hierarchy.
        /// </remarks>
        [Category("Appearance")]
        [Description("The drawing attributes to use for odd rows")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style RowOddStyle
        {
            get { return _rowOddStyle; }
            set
            {
                if (value == null) throw new ArgumentNullException();
                _rowOddStyle.Delta = value.Delta;
            }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeRowOddStyle()
        {
            return _rowOddStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetRowOddStyle()
        {
            _rowOddStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use for even rows
        /// </summary>
        /// <remarks>
        /// The set accessor sets the <see cref="Style.Delta"/> for the style but does not 
        /// change the style reference or style hierarchy.
        /// </remarks>
        [Category("Appearance")]
        [Description("The drawing attributes to use for even rows")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style RowEvenStyle
        {
            get { return _rowEvenStyle; }
            set
            {
                if (value == null) throw new ArgumentNullException();
                _rowEvenStyle.Delta = value.Delta;
            }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeRowEvenStyle()
        {
            return _rowEvenStyle.ShouldSerialize();
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetRowEvenStyle()
        {
            _rowEvenStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use for selected rows
        /// </summary>
        /// <remarks>
        /// <para>
        /// The row selection style is applied after all other styles - thus any properties
        /// set to non-default values in the RowSelectedStyle will override styles set using
        /// RowBindings or CellBindings
        /// </para>
        /// <para>
        /// The set accessor sets the <see cref="Style.Delta"/> for the style but does not 
        /// change the style reference or style hierarchy.
        /// </para>
        /// </remarks>
        [Category("Appearance")]
        [Description("The drawing attributes to use for selected rows")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style RowSelectedStyle
        {
            get { return _rowSelectedStyle; }
            set
            {
                if (value == null) throw new ArgumentNullException();
                _rowSelectedStyle.Delta = value.Delta;
            }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeRowSelectedStyle()
        {
            return _rowSelectedStyle.ShouldSerialize();
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetRowSelectedStyle()
        {
            _rowSelectedStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use for selected rows when the controls does not have focus
        /// </summary>
        /// <remarks>
        /// The set accessor sets the <see cref="Style.Delta"/> for the style but does not 
        /// change the style reference or style hierarchy.
        /// </remarks>
        [Category("Appearance")]
        [Description("The drawing attributes to use for selected rows when the control does not have focus")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style RowSelectedUnfocusedStyle
        {
            get { return _rowSelectedUnfocusedStyle; }
            set
            {
                if (value == null) throw new ArgumentNullException();
                _rowSelectedUnfocusedStyle.Delta = value.Delta;
            }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeRowSelectedUnfocusedStyle()
        {
            return _rowSelectedUnfocusedStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetRowSelectedUnfocusedStyle()
        {
            _rowSelectedUnfocusedStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use when printing rows
        /// </summary>
        /// <remarks>
        /// The set accessor sets the <see cref="Style.Delta"/> for the style but does not 
        /// change the style reference or style hierarchy.
        /// </remarks>
        /// <seealso href="XtraPrinting.html">Printing</seealso>
        [Category("Appearance")]
        [Description("The drawing attributes to use when printing rows")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style RowPrintStyle
        {
            get { return _rowPrintStyle; }
            set
            {
                if (value == null) throw new ArgumentNullException();
                _rowPrintStyle.Delta = value.Delta;
            }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeRowPrintStyle()
        {
            return _rowPrintStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetRowPrintStyle()
        {
            _rowPrintStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use when printing odd rows
        /// </summary>
        /// <remarks>
        /// The set accessor sets the <see cref="Style.Delta"/> for the style but does not 
        /// change the style reference or style hierarchy.
        /// </remarks>
        /// <seealso href="XtraPrinting.html">Printing</seealso>
        [Category("Appearance")]
        [Description("The drawing attributes to use when printing odd rows")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style RowPrintOddStyle
        {
            get { return _rowPrintOddStyle; }
            set
            {
                if (value == null) throw new ArgumentNullException();
                _rowPrintOddStyle.Delta = value.Delta;
            }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeRowPrintOddStyle()
        {
            return _rowPrintOddStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetRowPrintOddStyle()
        {
            _rowPrintOddStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use when printing even rows
        /// </summary>
        /// <remarks>
        /// The set accessor sets the <see cref="Style.Delta"/> for the style but does not 
        /// change the style reference or style hierarchy.
        /// </remarks>
        /// <seealso href="XtraPrinting.html">Printing</seealso>
        [Category("Appearance")]
        [Description("The drawing attributes to use when printing even rows")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style RowPrintEvenStyle
        {
            get { return _rowPrintEvenStyle; }
            set
            {
                if (value == null) throw new ArgumentNullException();
                _rowPrintEvenStyle.Delta = value.Delta;
            }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeRowPrintEvenStyle()
        {
            return _rowPrintEvenStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetRowPrintEvenStyle()
        {
            _rowPrintEvenStyle.Reset();
        }

        #endregion

        /// <summary>
        /// The style of line to draw connections between rows
        /// </summary>
        [Category("Appearance")]
        [Description("The style of line to draw connections between rows")] 
        public virtual LineStyle LineStyle
        {
            get 
            {
                return (_lineStyle == LineStyle.Default) ? _defaultLineStyle : _lineStyle;
            }
            set 
            {
                _lineStyle = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeLineStyle()
        {
            return _lineStyle != LineStyle.Default;
        }

        /// <summary>
        /// Called by framework to reset the line style to the default
        /// </summary>
        private void ResetLineStyle()
        {
            _lineStyle = LineStyle.Default;
        }

        /// <summary>
        /// The color to draw connections between rows
        /// </summary>
        [Category("Appearance")]
        [Description("The color to draw connections between rows")]
        public virtual Color LineColor
        {
            get { return (_lineColor == Color.Empty) ? _defaultLineColor : _lineColor; }
            set 
            {
                _lineColor = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Called by framework to determine whether the line color should be serialised
        /// </summary>
        /// <returns>True if the line color should be serialized</returns>
        private bool ShouldSerializeLineColor()
        {
            return _lineColor != Color.Empty;
        }

        /// <summary>
        /// Called by framework to reset the line color to the default
        /// </summary>
        private void ResetLineColor()
        {
            _lineColor = Color.Empty;
        }

        /// <summary>
        /// The mode to use to draw the background image
        /// </summary>
        [Category("Appearance"),
        Description("The mode to use to draw the background image"),
        DefaultValue(ImageDrawMode.Tile)] 
        public virtual ImageDrawMode BackgroundImageMode
        {
            get { return _backgroundImageMode; }
            set 
            {
                _backgroundImageMode = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Should the background image be printed
        /// </summary>
        /// <seealso href="XtraPrinting.html">Printing</seealso>
        [Category("Appearance"),
        Description("Should the background image be printed"),
        DefaultValue(true)] 
        public virtual bool PrintBackgroundImage
        {
            get { return _printBackgroundImage; }
            set { _printBackgroundImage = value; }
        }

        /// <summary>
        /// Should the control use XP themes (Visual Styles) to draw the column and row headers
        /// </summary>
        /// <remarks>
        /// If set to true then the current XP Theme is used to row and column headers
        /// and the styles set for these are ignored.
        /// </remarks>
        [Category("Appearance")]
        [Description("Should the control use themes (Visual Styles) to draw the column and row headers")]
        public bool UseThemedHeaders
        {
            get 
            {
                bool result = _useThemedHeaders;
                if (_useDefaultThemedHeaders)
                    result = _styleTemplate == StyleTemplate.ClassicXP || _styleTemplate == StyleTemplate.XP;
                return result && VisualStyleRenderer.IsSupported; 
            }
            set
            {
                _useThemedHeaders = value;
                _useDefaultThemedHeaders = false;
                Invalidate();
            }
        }

        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeUseThemedHeaders()
        {
            return !_useDefaultThemedHeaders;
        }

        /// <summary>
        /// Called by framework to reset the property to the default
        /// </summary>
        private void ResetThemedHeaders()
        {
            _useDefaultThemedHeaders = true;
        }

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
        /// Should the control act as a drag and drop target
        /// </summary>
        /// <remarks>
        /// Overridden to set the default value to true
        /// </remarks>
        [DefaultValue(true)]
        public override bool AllowDrop
        {
            get { return base.AllowDrop; }
            set { base.AllowDrop = value; }
        }

        #endregion

        #region XML Serialization

        /// <summary>
        /// Write the end-user customizable properties of the tree (eg column positions, widths,
        /// sorting etc) out to XML
        /// </summary>
        /// <param name="writer">The writer to write the properties to</param>
        /// <param name="elementName">The name of the enclosing element tag</param>
        public virtual void WriteXml(XmlWriter writer, string elementName)
        {
            writer.WriteStartElement(elementName);
            Columns.WriteXml(writer, "Columns");
            writer.WriteElementString("SortColumn", (SortColumn == null) ? "" : SortColumn.Name);
            writer.WriteElementString("RowHeight", RowHeight.ToString());
            writer.WriteElementString("AutoFitColumns", AutoFitColumns.ToString());
            writer.WriteEndElement();
        }

        /// <summary>
        /// Read the end-user customizable attributes of the tree (eg column positions, widths,
        /// sorting etc) from XML
        /// </summary>
        /// <param name="reader">The reader to read the properties from</param>
        /// <remarks>
        /// This method reads the attributes of columns that are currently in the collection from
        /// the given XML reader.   It will not create columns if they don't already exist in
        /// the collection. 
        /// </remarks>
        public virtual void ReadXml(XmlReader reader)
        {
            SuspendLayout();
            try
            {
                reader.ReadStartElement();
                Columns.ReadXml(reader);
                string sortColumnName = reader.ReadElementString("SortColumn");
                SortColumn = Columns[sortColumnName];
                if (reader.IsStartElement("RowHeight"))
                {
                    RowHeight = int.Parse(reader.ReadElementString("RowHeight"));
                }
                if (reader.IsStartElement("AutoFitColumns"))
                {
                    AutoFitColumns = bool.Parse(reader.ReadElementString("AutoFitColumns"));
                }
                reader.ReadEndElement();
            }
            finally
            {
                ResumeLayout(false);
                PerformLayout();
            }
        }


        #endregion

        #endregion

        #region Local Methods

        #region Data Binding 
      
        /// <summary>
        /// The <see cref="RowBinding"/> associated with a given row is cached to
        /// improve performance.  This method clears the cached RowBinding. 
        /// </summary>
        protected virtual void ClearCachedRowBindings()
        {
            _cachedBinding = null;
            _cachedBindingRow = null;
        }

        /// <summary>
        /// The <see cref="RowData"/> associated with currently displayed rows is cached to
        /// improve performance.  This method clears the cached data.
        /// </summary>
        protected virtual void ClearCachedRowData()
        {
            _cachedRowData.Clear();
        }

        /// <summary>
        /// Returns the first RowBinding from the <see cref="RowBindings"/> collection that binds to 
        /// the given item.
        /// </summary>
        /// <remarks>
        /// This method is called by <see cref="GetBindingForItem"/> when the <see cref="GetBinding"/>
        /// event is not handled by a client.  It implements the default logic for selecting a RowBinding
        /// by locating the first RowBinding in the <see cref="RowBindings"/> collection that returns true from
        /// the <see cref="RowBinding.BindsTo(object)"/> function.  To programmatically select the RowBinding to
        /// use for a given item you can override this method or handle the
        /// <see cref="GetBinding"/> event.  The row parameter may be null if the row that the
        /// item belongs to has not yet been established - typically when finding items.
        /// </remarks>
        /// <param name="item">The item to get the binding for</param>
        /// <param name="row">The row the item belongs to (if known)</param>        
        /// <returns>The first RowBinding that binds to the given item/row</returns>
        /// <seealso cref="GetBindingForItem"/>
        protected virtual RowBinding GetRowBinding(object item, Row row)
        {
            foreach (RowBinding binding in _rowBindings)
            {
                if (binding.BindsTo(item, row)) 
                    return binding;
            }
            return null;
        }

        /// <summary>
        /// Handle notification of a change to the child items of the given row via the <see cref="IBindingList.ListChanged"/>
        /// event.
        /// </summary>
        /// <param name="row">The row whose child items have changed</param>
        /// <param name="sender">The sender of the event</param>
        /// <param name="e">The ListChanged parameters</param>
        internal protected virtual void OnRowChildItemsChanged(Row row, object sender, ListChangedEventArgs e)
        {
            if (DataUpdateSuspended)
            {
                if (e.ListChangedType == ListChangedType.ItemChanged)
                {
                    _updateRowDataRequired = true;
                }
                else
                {
                    _updateRowsRequired = true;
                }
            }
            else
            {
                if (e.ListChangedType == ListChangedType.ItemChanged)
                {
                    Row childRow = row.ChildRowByIndex(e.NewIndex);
                    if (childRow != null)
                        UpdateRowData(childRow);
                }
                else
                {
                    row.UpdateChildren(false, false);
                }
            }
        }

        /// <summary>
        /// Bind the root row to the current <see cref="DataSource"/>.  
        /// </summary>
        protected virtual void BindDataSource()
        {
            SuspendLayout();

            try
            {

                // Delete the previous rows if any
                //
                if (_rootRow != null)
                {
                    _rootRow.Dispose();
                    _rootRow = null;
                    _topRow = null;
                    _focusRow = null;
                }

                // reset the horizontal scroll
                //
                _horzScrollOffset = 0;
                HorzScrollBar.Value = 0;

                if (_dataSource == null)
                {
                    _rootRow = null;
                }
                else
                {

#if CHECK_PUBLIC_KEY

                    // Test that the virtual tree assembly is signed with the Infralution strong name
                    // This is to make it just a little harder for the casual hacker
                    //
                    byte[] token = Assembly.GetExecutingAssembly().GetName().GetPublicKeyToken();
                    byte[] requiredToken = { 0x3E, 0x7E, 0x8E, 0x37, 0x44, 0xA5, 0xC1, 0x3F };
                    if (ArrayUtilities.Equals(token, requiredToken))
#endif
                    {
                        object rootItem = _dataSource;

                        // If the DataSource is a VS2005 BindingSource check whether the underlying
                        // DataSource is a DataSet or DataTable or DataView and if so bind directly to it
                        //
                        if (rootItem is BindingSource)
                        {
                            BindingSource bs = rootItem as BindingSource;
                            if (bs.DataSource is DataSet)
                            {
                                DataSet dataSet = bs.DataSource as DataSet;
                                rootItem = dataSet.Tables[bs.DataMember];
                            }
                            else if (bs.DataSource is DataView || bs.DataSource is DataTable)
                            {
                                rootItem = bs.DataSource;
                            }
                        }
                        if (rootItem is DataTable)
                            rootItem = ((DataTable)rootItem).DefaultView;
                        _rootRow = CreateRow(rootItem, null, 0, 0);
                    }

                    // if we are in design mode and don't have any row bindings then auto generate them
                    //
                    if (DesignMode && _rowBindings.Count == 0)
                    {
                        this.AutoGenerateBindings();
                    }
                }
            }
            finally
            {
                ResumeLayout(false);
                PerformLayout();
            }
        }

        /// <summary>
        /// Raises the <see cref="GetRowData"/> event to get the data describing how a given row should be 
        /// displayed by the tree.  
        /// </summary>
        /// <remarks>
        /// If this event is not handled by a client then this method 
        /// will use DataBinding to get the required information.  Override this method if you 
        /// wish to programmatically control the way in which the tree binds to the data source. 
        /// </remarks>
        /// <param name="row">The row to get the data for</param>
        /// <param name="rowData">The data for the given row</param>
        protected virtual void OnGetRowData(Row row, RowData rowData) 
        {
            // check if there is an event handler overriding databindings
            //
            if (GetRowData != null)
            {
                GetRowDataEventArgs e = new GetRowDataEventArgs(row, rowData);
                GetRowData(this, e);
            }
            else  // use databinding
            {
                RowBinding binding = GetBindingForRow(row);
                if (binding != null)
                {
                    binding.GetRowData(row, rowData);
                }
            }
        }

        /// <summary>
        /// Create new cell data object for the given column
        /// </summary>
        /// <param name="column">The column the cell data is for</param>
        /// <returns>The new cell data</returns>
        /// <remarks>
        /// This method can be overriden to create custom CellData objects (using a derived class)
        /// </remarks>
        internal protected virtual CellData CreateCellData(Column column)
        {
            return new CellData(column);
        }

        /// <summary>
        /// Raises the <see cref="GetCellData"/> event to get the data describing how a given cell (row/column) 
        /// should be displayed by the tree.  
        /// </summary>
        /// <remarks>
        /// If this event is not handled by a client then this method
        /// will use DataBinding to get the required information. Override this method if you wish to 
        /// programmatically control the way in which the tree binds to the data source. 
        /// </remarks>
        /// <seealso href="XtraProgrammaticBinding.html">Programmatic Data Binding</seealso>
        /// <param name="row">The row the cell belongs to</param>
        /// <param name="column">The column the cell belongs to</param>
        /// <param name="cellData">The data to be displayed in the cell</param>
        internal protected virtual void OnGetCellData(Row row, Column column, CellData cellData) 
        {
            // check if there is an event handler overriding databindings
            //
            if (GetCellData != null)
            {
                GetCellDataEventArgs e = new GetCellDataEventArgs(row, column, cellData);
                GetCellData(this, e);
            }
            else  // use databinding
            {
                RowBinding binding = GetBindingForRow(row);

                // if there is no row binding for the item then
                // generate a cell binding for the main column only
                //
                if (binding == null)
                {
                    if (column == GetMainColumn())
                    {
                        cellData.Value = row.Item;
                    }
                }
                else
                {
                    binding.GetCellData(row, column, cellData);
                }
            }
        }


        /// <summary>
        /// Raises the <see cref="GetToolTipCellData"/> event to update the CellData before
        /// the ToolTip is displayed for a given cell (row/column) 
        /// </summary>
        /// <remarks>
        /// Override this method if you wish to programmatically set the <see cref="CellData.ToolTip"/> 
        /// on demand (as the user hovers over the cell) rather than setting it when the cell is first
        /// displayed. This can be useful if generating the ToolTip text is computationally expensive.
        /// </remarks>
        /// <param name="row">The row the cell belongs to</param>
        /// <param name="column">The column the cell belongs to</param>
        /// <param name="cellData">The cell data to update</param>
        /// <seealso href="XtraProgrammaticBinding.html">Programmatic Data Binding</seealso>
        internal protected virtual void OnGetToolTipCellData(Row row, Column column, CellData cellData)
        {
            // check if there is an event handler 
            //
            if (GetToolTipCellData != null)
            {
                GetCellDataEventArgs e = new GetCellDataEventArgs(row, column, cellData);
                GetToolTipCellData(this, e);
            }
        }
        
        /// <summary>
        /// Raises the <see cref="GetChildPolicy"/> event to get child policy to use for the given row..
        /// </summary>
        /// <remarks>
        /// If this event is not handled by a client then this method will 
        /// use DataBinding to determine the ChildPolicy.   Override this method if you wish to programmatically 
        /// set the child policy.
        /// </remarks>
        /// <seealso href="XtraProgrammaticBinding.html">Programmatic Data Binding</seealso>
        /// <param name="row">The row to get the child policy for</param>
        /// <returns>The child policy to use for this row</returns>
        internal protected virtual RowChildPolicy GetChildPolicyForRow(Row row)
        {
            // check if there is an event handler overriding databindings
            //
            if (GetChildPolicy != null)
            {
                GetChildPolicyEventArgs e = new GetChildPolicyEventArgs(row);
                GetChildPolicy(this, e);
                return e.ChildPolicy;
            }
            else  // use databinding
            {
                RowBinding binding = GetBindingForRow(row);
                if (binding == null) return RowChildPolicy.Normal;
                return binding.GetChildPolicy(row);
            }
        }

        /// <summary>
        /// Raises the <see cref="GetChildren"/> event to locate the children of a given row.   
        /// </summary>
        /// <remarks>
        /// If this event is not handled by a client then this method will use DataBinding to locate the 
        /// children. Override this method if you wish to programmatically setup the relationships between 
        /// items in the tree instead of using databinding to do this.
        /// </remarks>
        /// <seealso href="XtraProgrammaticBinding.html">Programmatic Data Binding</seealso>
        /// <param name="row">The row to get the children for</param>
        /// <returns>The list of child items for this row (may be null)</returns>
        internal protected virtual IList GetChildrenForRow(Row row)
        {
            // check if there is an event handler overriding databindings
            //
            if (GetChildren != null)
            {
                GetChildrenEventArgs e = new GetChildrenEventArgs(row);
                GetChildren(this, e);
                return e.Children;
            }

            // try using row bindings
            //
            RowBinding binding = GetBindingForRow(row);
            if (binding != null) 
                return binding.GetChildrenForRow(row);

            // there are no matching bindings so check if the item is a list or list source
            //
            if (row.Item is IList)
                return row.Item as IList;
            if (row.Item is IListSource)
                return (row.Item as IListSource).GetList();
            return null;
        }

        /// <summary>
        /// Raises the <see cref="GetParent"/> event to locate the parent of a given item.   
        /// </summary>
        /// <remarks>
        /// <para>
        /// If this event is not handled by a client then this method will use DataBinding to 
        /// determine the parent.   Override this method if you wish to programmatically setup the relationships 
        /// between items in the tree instead of using databinding to do this.
        /// </para>
        /// <para>
        /// This method is only used if the client application uses the <see cref="FindRow(object)"/> method to locate
        /// the Row in the tree corresponding to a given item or uses the SelectedItems property
        /// to set the selected rows.   These methods provide a simpler mechanism for selecting and
        /// identifying rows in the tree but require that every item in the must have a single parent
        /// ie an item can only appear once in the hierarchy.
        /// </para>
        /// </remarks>
        /// <seealso href="XtraProgrammaticBinding.html">Programmatic Data Binding</seealso>
        /// <param name="item">The item whose parent should be returned</param>
        /// <returns>The parent of the given item.</returns>
        internal protected virtual object GetParentForItem(object item)
        {
            // check if there is an event handler overriding databindings
            //
            if (GetParent != null)
            {
                GetParentEventArgs e = new GetParentEventArgs(item);
                GetParent(this, e);
                return e.Parent;
            }
            else // use data binding
            {
                RowBinding binding = GetBindingForItem(item);
                if (binding == null) return null;
                return binding.GetParentForItem(item);
            }
        }

        /// <summary>
        /// Returns the Context Menu to use for the given row.  Raises the <see cref="GetContextMenuStrip"/> event 
        /// to allow clients to handle this programmatically.    
        /// </summary>
        /// <remarks>
        /// If this event is not handled by a client then the tree will attempt 
        /// to use DataBinding to determine the ContextMenuStrip to display.   
        /// Override this method if you wish to programmatically set the ContextMenu.
        /// </remarks>
        /// <param name="row">The row to get the child policy for</param>
        /// <returns>The context menu strip to use for this row</returns>
        internal protected virtual ContextMenuStrip GetContextMenuStripForRow(Row row)
        {

            // check if there is an event handler overriding databindings
            //
            if (GetContextMenuStrip != null)
            {
                GetContextMenuStripEventArgs e = new GetContextMenuStripEventArgs(row);
                e.ContextMenuStrip = this.ContextMenuStrip;
                GetContextMenuStrip(this, e);
                return e.ContextMenuStrip;
            }
            else  // use databinding
            {
                RowBinding binding = GetBindingForRow(row);
                ContextMenuStrip menu = null;
                if (binding != null) 
                    menu = binding.GetContextMenuStrip(row);
                if (menu == null) 
                    menu = this.ContextMenuStrip;
                return menu;
            }
        }

        /// <summary>
        /// Returns true if the given column is supported in the current state of the tree.  
        /// Raises the <see cref="GetColumnInContext"/> event to allow clients to handle this programmatically.    
        /// </summary>
        /// <remarks>
        /// If this event is not handled by a client then the tree will attempt to use DataBinding 
        /// to determine the whether the column is in context.  
        /// Override this method if you wish to programmatically determine column context sensitivity.
        /// </remarks>
        /// <param name="column">The column to be displayed (or not)</param>
        /// <returns>True if the column is supported in the current context</returns>
        /// <seealso cref="ContextRow"/>
        internal protected virtual bool ColumnInContext(Column column)
        {
            // check if there is an event handler overriding databindings
            //
            if (GetColumnInContext != null)
            {
                GetColumnInContextEventArgs e = new GetColumnInContextEventArgs(column);
                GetColumnInContext(this, e);
                return e.InContext;
            }
            else  // use databinding
            {
                bool inContext = false;
                if (ContextRow != null)
                {
                    RowBinding binding = GetBindingForRow(ContextRow);
                    if (binding != null)
                    {
                        inContext = binding.SupportsColumn(column);
                    }
                }
                return inContext;
            }
        }

        /// <summary>
        /// Set the value of the cell specified by the given row/column in the underlying datasource.
        /// Raises the <see cref="SetCellValue"/> event to allow clients to handle this programmatically.    
        /// </summary>
        /// <remarks>
        /// If this event is not handled by a client then the tree will attempt to use DataBinding 
        /// to set the cell value.  Override this method if you wish to programmatically handle
        /// setting cell values when they change.
        /// </remarks>
        /// <seealso href="XtraProgrammaticBinding.html">Programmatic Data Binding</seealso>
        /// <param name="row">The row the cell belongs to</param>
        /// <param name="column">The column the cell belongs to</param>
        /// <param name="oldValue">The old value of the cell (prior to editing)</param>
        /// <param name="newValue">The new value of the cell</param>
        /// <returns>True if the value was successfully changed</returns>
        internal protected virtual bool SetValueForCell(Row row, Column column, 
            object oldValue, object newValue)
        {
            // check if there is an event handler overriding databindings
            //
            if (SetCellValue != null)
            {
                SetCellValueEventArgs e = new SetCellValueEventArgs(row, column, oldValue, newValue);
                SetCellValue(this, e);
                return !e.Cancel;
            }
            else  // use databinding
            {
                RowBinding binding = GetBindingForRow(row);
                if (binding != null)
                {
                    return binding.SetCellValue(row, column, oldValue, newValue);
                }
                return false;
            }
        }

        /// <summary>
        /// Raises the <see cref="GetAllowRowDrag"/> event to determine whether the given row can be dragged.  
        /// </summary>
        /// <remarks>
        /// If this event is not handled by a client then this method will use DataBinding to 
        /// determine whether the row can be dragged.  
        /// Override this method if you wish to programmatically set whether rows can be dragged.
        /// </remarks>
        /// <param name="row">The row begin dragged</param>
        /// <returns>True if the row can be dragged</returns>
        internal protected virtual bool AllowRowDrag(Row row)
        {
            // check if there is an event handler overriding databindings
            //
            if (GetAllowRowDrag != null)
            {
                GetAllowRowDragEventArgs e = new GetAllowRowDragEventArgs(row);
                GetAllowRowDrag(this, e);
                return e.AllowDrag;
            }
            else  // use databinding
            {
                RowBinding binding = GetBindingForRow(row);
                if (binding == null) return false;
                return binding.GetAllowDrag(row);
            }
        }

        /// <summary>
        /// Raises the <see cref="GetAllowedRowDropLocations"/> event to determine the allowed drop
        /// locations for the given row.  
        /// </summary>
        /// <remarks>
        /// If this event is not handled by a client then 
        /// this method will use DataBinding to determine the allowed drop locations.  
        /// Override this method if you wish to programmatically set the allowed drop
        /// locations
        /// </remarks>
        /// <param name="row">The row to get the allowed drop locations for</param>
        /// <param name="data">The data being dropped</param>
        /// <returns>Combination of the allowed drop locations</returns>
        internal protected virtual RowDropLocation AllowedRowDropLocations(Row row, IDataObject data)
        {
            // check if there is an event handler overriding databindings
            //
            if (GetAllowedRowDropLocations != null)
            {
                GetAllowedRowDropLocationsEventArgs e = new GetAllowedRowDropLocationsEventArgs(row, data);
                GetAllowedRowDropLocations(this, e);
                return e.AllowedDropLocations;
            }
            else  // use databinding
            {
                RowBinding binding = GetBindingForRow(row);
                if (binding == null) return RowDropLocation.None;
                return binding.GetAllowedDropLocations(row, data);
            }
        }

        /// <summary>
        /// Raises the GetRowDropEffect event the type of drop operation to perform for the given row, 
        /// drop location and data.  
        /// </summary>
        /// <remarks>
        /// If this event is not handled by a client then this method will use DataBinding 
        /// to determine the DropEffect to use. Override this method if you wish to programmatically set the 
        /// drop effect.  The drop effect is typically dependent on the current modifier keys ie 
        /// Keys.Control sets the effect to Copy.
        /// </remarks>
        /// <param name="row">
        /// The row to get the drop effect for. May be null when dropping data onto the empty space
        /// below the rows.
        /// </param>
        /// <param name="dropLocation">The location of the drop</param>
        /// <param name="data">The data being dropped</param>
        /// <returns>The allowed drop effect</returns>
        internal protected virtual DragDropEffects RowDropEffect(Row row, 
            RowDropLocation dropLocation, 
            IDataObject data)
        {
            // check if there is an event handler overriding databindings
            //
            if (GetRowDropEffect != null)
            {
                GetRowDropEffectEventArgs e = new GetRowDropEffectEventArgs(row, dropLocation, data);
                GetRowDropEffect(this, e);
                return e.DropEffect;
            }
            else  // use databinding
            {
                RowBinding binding = GetBindingForRow(row);
                if (binding == null) return DragDropEffects.None;
                return binding.GetDropEffect(row, dropLocation, data);
            }
        }

        /// <summary>
        /// Raises the <see cref="RowDrop"/> event to handle dropping a dragged item onto a row.  
        /// </summary>
        /// <remarks>
        /// If this event is not handled by a client then this method will use DataBinding 
        /// if possible to handle the drop operation. 
        /// Override this method if you wish to programmatically handle dropping of dragged items
        /// </remarks>
        /// <param name="row">The row being dropped on.  May be null when dropping data onto
        /// the empty space below the rows.</param>
        /// <param name="dropLocation">The location of the drop</param>
        /// <param name="data">The data being dropped</param>
        /// <param name="dropEffect">The type of drop being performed</param>
        internal protected virtual void OnRowDrop(Row row, 
            RowDropLocation dropLocation, 
            IDataObject data,
            DragDropEffects dropEffect)
        {
            // handle possible exceptions in the drop method because these are swallowed
            // by the runtime otherwise with no warning to the user
            //
            try
            {

                // check if there is an event handler overriding databindings
                //
                if (RowDrop != null)
                {
                    RowDropEventArgs e = new RowDropEventArgs(row, dropLocation, data, dropEffect);
                    RowDrop(this, e);
                }
                else  // use databinding
                {
                    RowBinding binding = GetBindingForRow(row);
                    if (binding != null) 
                    {
                        binding.OnDrop(row, dropLocation, data, dropEffect);
                    }
                }
            }
            catch (Exception ex)
            {
                String msg = String.Format("Unhandled Exception in OnRowDrop\nException: {0}\nMessage: {1}\nStack:\n{2}",
                                            ex.GetType().Name, ex.Message, ex.StackTrace);
                DisplayErrorMessage(msg);
                throw;
            }
        }

        /// <summary>
        /// Outputs an error message to the Trace console 
        /// </summary>
        /// <remarks>
        /// If <see cref="SuppressBindingExceptions"/> is set to true then this method can be overridden
        /// to display an error message to the user when an exception or error occurs.  By default this
        /// method writes the error message to the trace console.
        /// </remarks>
        /// <param name="message">The message to display</param>
        internal protected virtual void DisplayErrorMessage(string message)
        {
            Trace.WriteLine(message, "VirtualTree");
        }

        #endregion

        #region Data Binding Auto Generation 

        /// <summary>
        /// Find the column to representing the given data field
        /// </summary>
        /// <param name="dataField">The data field to find the column for</param>
        /// <returns>The column that was autogenerated for the given data field</returns>
        protected Column FindColumnForDataField(string dataField)
        {
            foreach (Column column in Columns)
            {
                if (column.DataField == dataField) return column;
            }
            return null;
        }

        /// <summary>
        /// Generate a column to represent the main column - or return the existing column
        /// </summary>
        /// <returns>The main column to use</returns>
        protected virtual Column AutoGenerateMainColumn()
        {
            const string dataField = "__MAIN__";
            Column column = FindColumnForDataField(dataField);
            if (column == null)
            {
                column = CreateColumn();
                column.Caption = "Item";
                column.DataField = dataField;
                column.AutoSizePolicy = ColumnAutoSizePolicy.AutoIncrease;
                column.Movable = false;
                column.Hidable = false;
                Columns.Insert(0,column);
                if (this.Container != null)
                {
                    ComponentUtilities.AddComponent(Container, column, "colMain");
                }
            }
            return column;
        }

        /// <summary>
        /// Generate a column to represent the given data column - or return the existing column
        /// </summary>
        /// <param name="dataField">The name of the data field the column is to display</param>
        /// <param name="caption">The caption for the column</param>
        /// <returns>The column used to display the given data field</returns>
        protected virtual Column AutoGenerateColumn(string dataField, string caption)
        {           
            // find the column if it exists
            //
            Column column = FindColumnForDataField(dataField);

            // create a new column if necessary
            //
            if (column == null)
            {
                column = CreateColumn();
                column.DataField = dataField;
                column.Caption = caption;
                Columns.Add(column);
                if (this.Container != null)
                {
                    ComponentUtilities.AddComponent(Container, column, "col" + dataField);
                }
            }
            return column;
        }

        /// <summary>
        /// Find the existing DataRowRowBinding for the given data table.
        /// </summary>
        /// <param name="dataTable">The data table to find the binding for</param>
        /// <returns>The DataRowRowBinding if any</returns>
        internal protected DataRowRowBinding FindDataRowBinding(DataTable dataTable)
        {
            foreach (RowBinding binding in RowBindings)
            {
                if (binding is DataRowRowBinding)
                {
                    if ((binding as DataRowRowBinding).Table == dataTable.TableName)
                        return binding as DataRowRowBinding;
                }
            }
            return null;
        }

        /// <summary>
        /// Generate a cell binding for the given column or return the existing cell binding
        /// </summary>
        /// <param name="rowBinding">The row binding to generate the cell binding for</param>
        /// <param name="column">The column to generate the binding for</param>
        /// <param name="dataColumn">The data column to bind to</param>
        /// <returns>The cell binding</returns>
        protected virtual DataRowCellBinding AutoGenerateDataRowCellBinding(DataRowRowBinding rowBinding, Column column, DataColumn dataColumn)
        {
            DataRowCellBinding cellBinding = (DataRowCellBinding)rowBinding.CellBindings[column];
            if (cellBinding == null)
            {
                cellBinding = new DataRowCellBinding(column, dataColumn.ColumnName);
                rowBinding.CellBindings.Add(cellBinding);
                cellBinding.Field = dataColumn.ColumnName;
            }
            return cellBinding;
        }

        /// <summary>
        /// Generate a new DataRowRowBinding for the given table - or return the existing binding
        /// </summary>
        /// <param name="dataTable">The table to generate the binding for</param>
        /// <returns>The DataRowRowBinding for the table</returns>
        protected virtual DataRowRowBinding AutoGenerateDataRowBinding(DataTable dataTable)
        {
            // locate the existing row binding (if any)
            //
            DataRowRowBinding rowBinding = FindDataRowBinding(dataTable);

            // add a new row binding if necessary
            //
            if (rowBinding == null)
            {
                rowBinding = new DataRowRowBinding();
                rowBinding.Table = dataTable.TableName;
                if (dataTable.ChildRelations.Count > 0)
                    rowBinding.ChildRelation = dataTable.ChildRelations[0].RelationName;
                if (dataTable.ParentRelations.Count == 1)
                    rowBinding.ParentRelation = dataTable.ParentRelations[0].RelationName;
                RowBindings.Add(rowBinding);
                rowBinding.Name = "rowBinding" + dataTable.TableName;
                if (this.Container != null)
                {
                    ComponentUtilities.AddComponent(Container, rowBinding, rowBinding.Name);
                }
            }

            // generate the cell binding for the main column
            //
            if (dataTable.Columns.Count > 0)
            {
                Column column = AutoGenerateMainColumn();
                AutoGenerateDataRowCellBinding(rowBinding, column, dataTable.Columns[0]);
            }

            // generate the columns and cell bindings for this table
            //
            foreach (DataColumn dataColumn in dataTable.Columns)
            {
                Column column = AutoGenerateColumn(dataColumn.ColumnName, dataColumn.Caption);
                AutoGenerateDataRowCellBinding(rowBinding, column, dataColumn);
            }

            // generate the row bindings for any child tables
            //
            DataRelation childRelation = dataTable.ChildRelations[rowBinding.ChildRelation];
            if (childRelation != null)
            {
                // only recurse to generate bindings for child tables if they don't already
                // exist
                //
                if (FindDataRowBinding(childRelation.ChildTable) == null)
                {
                    AutoGenerateDataRowBinding(childRelation.ChildTable);
                }
            }
            return rowBinding;
        }

        /// <summary>
        /// Generate a new DataViewRowBinding for the given table - or return the existing binding
        /// </summary>
        /// <param name="dataTable">The table to generate the binding for</param>
        /// <returns>The DataViewRowBinding for the table</returns>
        protected virtual DataViewRowBinding AutoGenerateDataViewBinding(DataTable dataTable)
        {
            // locate the existing row binding (if any)
            //
            DataViewRowBinding rowBinding = null;
            foreach (RowBinding binding in RowBindings)
            {
                if (binding is DataViewRowBinding)
                {
                    if ((binding as DataViewRowBinding).Table == dataTable.TableName)
                    {
                        rowBinding = (binding as DataViewRowBinding);
                    }
                }
            }

            // create a new one if necessary
            //
            if (rowBinding == null)
            {
                rowBinding = new DataViewRowBinding(dataTable.TableName);
                RowBindings.Add(rowBinding);
                rowBinding.Name = "viewBinding" + dataTable.TableName;
                if (this.Container != null)
                {
                    ComponentUtilities.AddComponent(Container, rowBinding, rowBinding.Name);
                }
            }

            // create a cell binding for the main column
            //
            Column mainColumn = AutoGenerateMainColumn();
            CellBinding cellBinding = rowBinding.CellBindings[mainColumn];
            if (cellBinding == null)
            {
                cellBinding = new DataViewCellBinding(mainColumn);
                rowBinding.CellBindings.Add(cellBinding);
            }
            return rowBinding;
        }

        /// <summary>
        /// Auto Generate columns and bindings for a DataTable
        /// </summary>
        /// <param name="dataTable">The datatable to generate bindings for</param>
        protected virtual void AutoGenerateDataTableBindings(DataTable dataTable)
        {
            AutoGenerateDataViewBinding(dataTable);
            AutoGenerateDataRowBinding(dataTable);            
        }

        /// <summary>
        /// Auto Generate columns and bindings for a Dataset
        /// </summary>
        /// <param name="dataset"></param>
        protected virtual void AutoGenerateDataSetBindings(DataSet dataset)
        {
            // locate the existing dataset binding (if any)
            //
            DataSetRowBinding rowBinding = null;
            foreach (RowBinding binding in RowBindings)
            {
                if (binding is DataSetRowBinding)
                {
                    rowBinding = (binding as DataSetRowBinding);
                }
            }

            // add a dataset binding if one doesn't exist
            //
            if (rowBinding == null)
            {
                rowBinding = new DataSetRowBinding();
                RowBindings.Add(rowBinding);
                if (this.Container != null)
                {
                    ComponentUtilities.AddComponent(Container, rowBinding, "dataSetBinding");
                }
            }

            // create a cell binding for the main column
            //
            Column mainColumn = AutoGenerateMainColumn();
            CellBinding cellBinding = rowBinding.CellBindings[mainColumn];
            if (cellBinding == null)
            {
                cellBinding = new DataSetCellBinding(mainColumn);
                rowBinding.CellBindings.Add(cellBinding);
            }

            // generate view and row bindings for each table in the dataset
            //
            foreach(DataTable table in dataset.Tables)
            {
                AutoGenerateDataViewBinding(table);
            }
            foreach(DataTable table in dataset.Tables)
            {
                AutoGenerateDataRowBinding(table);            
            }
        }

        /// <summary>
        /// Find the existing ObjectRowBinding for the given object type.
        /// </summary>
        /// <param name="type">The object type to find the binding for</param>
        /// <returns>The ObjectRowBinding if any</returns>
        protected ObjectRowBinding FindObjectRowBinding(Type type)
        {
            foreach (RowBinding binding in RowBindings)
            {
                if (binding is ObjectRowBinding)
                {
                    if ((binding as ObjectRowBinding).TypeName == type.FullName)
                        return binding as ObjectRowBinding;
                }
            }
            return null;
        }

        /// <summary>
        /// Find the existing ObjectRowBinding for the given typed list.
        /// </summary>
        /// <param name="typedListName">The typed list the object binding is for</param>
        /// <returns>The ObjectRowBinding if any</returns>
        protected ObjectRowBinding FindTypedListObjectRowBinding(string typedListName)
        {
            foreach (RowBinding binding in RowBindings)
            {
                if (binding is ObjectRowBinding)
                {
                    if ((binding as ObjectRowBinding).TypedListName == typedListName)
                        return binding as ObjectRowBinding;
                }
            }
            return null;
        }

        /// <summary>
        /// Creates a new ObjectRowBinding for use when auto generating bindings
        /// </summary>
        /// <remarks>
        /// Override this method if you have derived a new ObjectRowBinding class that
        /// you wish the auto generator to use.
        /// </remarks>
        /// <param name="type"></param>
        /// <returns>A new ObjectRowBinding</returns>
        protected virtual ObjectRowBinding CreateObjectRowBinding(Type type)
        {
            return new ObjectRowBinding(type);
        }

        /// <summary>
        /// Auto Generate the object binding for the given type
        /// </summary>
        /// <param name="type">The type to generate binding for</param>
        protected virtual void AutoGenerateObjectBindings(Type type)
        {

            // locate the existing row binding (if any)
            //
            ObjectRowBinding rowBinding = FindObjectRowBinding(type);

            // add a binding if one doesn't exist
            //
            if (rowBinding == null)
            {
                rowBinding = CreateObjectRowBinding(type);
                RowBindings.Add(rowBinding);
                if (this.Container != null)
                {
                    ComponentUtilities.AddComponent(Container, rowBinding, "rowBinding" + type.Name);
                }
            }
           
            // create a cell binding for the main column
            //
            Column mainColumn = AutoGenerateMainColumn();
            ObjectCellBinding mainCellBinding = rowBinding.CellBinding(mainColumn);
            if (mainCellBinding == null)
            {
                mainCellBinding = rowBinding.CreateCellBinding() as ObjectCellBinding;
                mainCellBinding.Column  = mainColumn;
                rowBinding.CellBindings.Add(mainCellBinding);
            }

            // if this is a list with a typed indexer than create object bindings
            // for the contained type 
            //
            if (type.GetInterface ("IList") != null)
            {
                rowBinding.ChildProperty = "this";
                PropertyInfo indexer = type.GetProperty("Item", new Type[] { typeof(int) });
                if (indexer != null && indexer.PropertyType != typeof(object))
                {
                    if (FindObjectRowBinding(indexer.PropertyType) == null)
                    {
                        AutoGenerateObjectBindings(indexer.PropertyType);
                    }
                }
            }
            else
            {
                // generate columns and cell bindings for each property of the object
                //
                PropertyInfo[] properties = type.GetProperties();
                foreach (PropertyInfo property in properties)
                {
                    Column column = AutoGenerateColumn(property.Name, property.Name);
                    ObjectCellBinding cellBinding = rowBinding.CellBinding(column);
                    if (cellBinding == null)
                    {
                        cellBinding = rowBinding.CreateCellBinding() as ObjectCellBinding;
                        cellBinding.Column = column;
                        cellBinding.Field = property.Name;
                        rowBinding.CellBindings.Add(cellBinding);
                    }
                }
            }
        }

        /// <summary>
        /// Auto Generate the object bindings for the given typed list
        /// </summary>
        /// <param name="typedList">The typedList to generate bindings for</param>
        /// <returns>The object row binding for the given type</returns>
        protected virtual void AutoGenerateTypedListBindings(ITypedList typedList)
        {

            // create a binding for the typed List
            //
            System.Type type = typedList.GetType();
            ObjectRowBinding rowBinding = FindObjectRowBinding(type);

            // add a binding if one doesn't exist
            //
            if (rowBinding == null)
            {
                rowBinding = CreateObjectRowBinding(type);
                rowBinding.ChildProperty = "this";
                RowBindings.Add(rowBinding);
                if (this.Container != null)
                {
                    ComponentUtilities.AddComponent(Container, rowBinding, "rowBinding" + type.Name);
                }
            }
           
            // create a cell binding for the main column
            //
            Column mainColumn = AutoGenerateMainColumn();
            ObjectCellBinding mainCellBinding = rowBinding.CellBinding(mainColumn);
            if (mainCellBinding == null)
            {
                mainCellBinding = rowBinding.CreateCellBinding() as ObjectCellBinding;
                mainCellBinding.Column  = mainColumn;
                rowBinding.CellBindings.Add(mainCellBinding);
            }

            // find the existing row binding for the TypedList items
            //
            string typeListName = typedList.GetListName(null);
            rowBinding = FindTypedListObjectRowBinding(typeListName);

            // add a binding if one doesn't exist
            //
            if (rowBinding == null)
            {
                rowBinding = CreateObjectRowBinding(null);
                rowBinding.TypedListName = typeListName;
                RowBindings.Add(rowBinding);
                if (this.Container != null)
                {
                    ComponentUtilities.AddComponent(Container, rowBinding, "rowBinding" + typeListName);
                }
            }

            PropertyDescriptorCollection properties = typedList.GetItemProperties(null);

            // create a cell binding for the main column - bound by default to the first
            // property
            //
            mainCellBinding = rowBinding.CellBinding(mainColumn);
            if (mainCellBinding == null)
            {
                mainCellBinding = rowBinding.CreateCellBinding() as ObjectCellBinding;
                mainCellBinding.Column = mainColumn;
                if (properties.Count > 0)
                    mainCellBinding.Field = properties[0].Name;
                rowBinding.CellBindings.Add(mainCellBinding);
            }

            // generate columns and cell bindings for each property of the object
            //
            foreach (PropertyDescriptor property in properties)
            {
                Column column = AutoGenerateColumn(property.Name, property.Name);
                ObjectCellBinding cellBinding = rowBinding.CellBinding(column);
                if (cellBinding == null)
                {
                    cellBinding = rowBinding.CreateCellBinding() as ObjectCellBinding;
                    cellBinding.Column = column;
                    cellBinding.Field = property.Name;
                    rowBinding.CellBindings.Add(cellBinding);
                }
            }
        }

        #endregion

        #region Widgets/Painting


        /// <summary>
        /// Start editing the given cellWidegt
        /// </summary>
        /// <param name="cellWidget">The cell widget to start editing</param>
        internal protected virtual void StartEdit(CellWidget cellWidget)
        {
            if (cellWidget != _editWidget)
            {
                if (CompleteEdit())
                {
                    if (cellWidget != null)
                    {
                        // ensure that the row containing the edit cell is selected
                        //
                        if (!cellWidget.Row.Selected)
                        {
                            SetSelectedRow(cellWidget.Row, cellWidget.EnsureVisibleOnFocus);
                        }
                        if (SelectionMode == SelectionMode.Cell && cellWidget.Column.Selectable)
                        {
                            SelectedColumn = cellWidget.Column;
                        }
                        cellWidget.StartEdit();
                    }
                    _editWidget = cellWidget;
                }
            }        
        }

        /// <summary>
        /// Return the PanelWidget used to display row headers and pinned columns.
        /// </summary>
        internal protected PanelWidget PinnedPanel
        {
            get { return _pinnedPanel; }
        }

        /// <summary>
        /// Return the PanelWidget used to display scrollable columns.
        /// </summary>
        protected PanelWidget ScrollablePanel
        {
            get { return _scrollablePanel; }
        }

        /// <summary>
        /// Returns true if the RowWidget has been created for the given row
        /// </summary>
        /// <param name="row">The row to check for</param>
        /// <returns>True if RowWidget has been created</returns>
        protected virtual bool RowWidgetCreated(Row row)
        {
            return PinnedPanel.RowWidgetCreated(row);
        }

        /// <summary>
        /// Create the panel widget used to display the header columns or the scrollable columns
        /// </summary>
        /// <remarks>
        /// To change the appearance or behaviour of this type of widget derive a new widget
        /// class and override the appropriate methods.  You can then either override this method or 
        /// set the corresponding WidgetCreator delegate property to create widgets of the derived class.
        /// </remarks>
        /// <returns>A new PanelWidget</returns>
        internal protected virtual PanelWidget CreatePanelWidget()
        {
            return (PanelWidgetCreator == null) ?
                 new PanelWidget(this) : PanelWidgetCreator(this);
        }

        /// <summary>
        /// Create the header widget used to display column headers
        /// </summary>
        /// <param name="panelWidget">The PanelWidget that the widget is to belong to</param>
        /// <remarks>
        /// To change the appearance or behaviour of this type of widget derive a new widget
        /// class and override the appropriate methods.  You can then either override this method or 
        /// set the corresponding WidgetCreator delegate property to create widgets of the derived class.
        /// </remarks>
        /// <returns>A widget used to display the column header region</returns>
        internal protected virtual HeaderWidget CreateHeaderWidget(PanelWidget panelWidget)
        {
            return (HeaderWidgetCreator == null) ?
                 new HeaderWidget(panelWidget) : HeaderWidgetCreator(panelWidget);
        }

        /// <summary>
        /// Create a column header widget used to display a particular column
        /// </summary>
        /// <remarks>
        /// To change the appearance or behaviour of this type of widget derive a new widget
        /// class and override the appropriate methods.  You can then either override this method or 
        /// set the corresponding WidgetCreator delegate property to create widgets of the derived class.
        /// </remarks>
        /// <param name="parentWidget">The parent widget</param>
        /// <param name="column">The column the widget is to display</param>
        /// <returns>A widget used to display the header for a column</returns>
        internal protected virtual ColumnHeaderWidget CreateColumnHeaderWidget(
            Widget parentWidget, 
            Column column)
        {
            return (ColumnHeaderWidgetCreator == null) ?
                new ColumnHeaderWidget(parentWidget, column) : ColumnHeaderWidgetCreator(parentWidget, column);
        }

        /// <summary>
        /// Create a widget used to handle resizing columns.
        /// </summary>
        /// <remarks>
        /// To change the appearance or behaviour of this type of widget derive a new widget
        /// class and override the appropriate methods.  You can then either override this method or 
        /// set the corresponding WidgetCreator delegate property to create widgets of the derived class.
        /// </remarks>
        /// <param name="columnHeaderWidget">The ColumnHeaderWidget the divider belongs to</param>
        /// <returns>A widget used to handle resizing columns</returns>
        internal protected virtual ColumnDividerWidget CreateColumnDividerWidget(ColumnHeaderWidget columnHeaderWidget)
        {
            return (ColumnDividerWidgetCreator == null) ? 
                new ColumnDividerWidget(columnHeaderWidget) : ColumnDividerWidgetCreator(columnHeaderWidget);
        }

        
        /// <summary>
        /// Create a RowWidget to display the given row and add it to the control's list of 
        /// widgets
        /// </summary>
        /// <remarks>
        /// To change the appearance or behaviour of this type of widget derive a new widget
        /// class and override the appropriate methods.  You can then either override this method or 
        /// set the corresponding WidgetCreator delegate property to create widgets of the derived class.
        /// </remarks>
        /// <param name="row">The row the widget is to display</param>
        /// <param name="panelWidget">The panelWidget that the row widget belongs to (may be null)</param>
        /// <returns>A widget used to display the given row</returns>
        internal protected virtual RowWidget CreateRowWidget(PanelWidget panelWidget, Row row)
        {
            if (RowWidgetCreator == null) 
            {
                RowBinding binding = GetBindingForRow(row);
                return (binding == null) ?
                    new RowWidget(panelWidget, row) : binding.CreateRowWidget(panelWidget, row);
            }
            else
            {
                return RowWidgetCreator(panelWidget, row);
            }
        }
        
        /// <summary>
        /// Create a widget used to display the header for a row.
        /// </summary>
        /// <remarks>
        /// To change the appearance or behaviour of this type of widget derive a new widget
        /// class and override the appropriate methods.  You can then either override this method or 
        /// set the corresponding WidgetCreator delegate property to create widgets of the derived class.
        /// </remarks>
        /// <param name="rowWidget">The RowWidget the header belongs to</param>
        /// <returns>A widget used to display the header for the given row</returns>
        internal protected virtual RowHeaderWidget CreateRowHeaderWidget(RowWidget rowWidget)
        {
            return (RowHeaderWidgetCreator == null) ? 
                new RowHeaderWidget(rowWidget) : RowHeaderWidgetCreator(rowWidget);
        }

        /// <summary>
        /// Create a widget used to handle resizing rows.
        /// </summary>
        /// <remarks>
        /// To change the appearance or behaviour of this type of widget derive a new widget
        /// class and override the appropriate methods.  You can then either override this method or 
        /// set the corresponding WidgetCreator delegate property to create widgets of the derived class.
        /// </remarks>
        /// <param name="rowHeaderWidget">The RowHeaderWidget the divider belongs to</param>
        /// <returns>A widget used to handle resizing rows</returns>
        internal protected virtual RowDividerWidget CreateRowDividerWidget(RowHeaderWidget rowHeaderWidget)
        {
            return (RowDividerWidgetCreator == null) ?
                new RowDividerWidget(rowHeaderWidget) : RowDividerWidgetCreator(rowHeaderWidget);
        }

        /// <summary>
        /// Create a widget used to display connections for a row.
        /// </summary>
        /// <remarks>
        /// To change the appearance or behaviour of this type of widget derive a new widget
        /// class and override the appropriate methods.  You can then either override this method or 
        /// set the corresponding WidgetCreator delegate property to create widgets of the derived class.
        /// </remarks>
        /// <param name="rowWidget">The rowWidget the expansion widget belongs to</param>
        /// <returns>A widget used to display connections for the given row</returns>
        internal protected virtual ExpansionWidget CreateExpansionWidget(RowWidget rowWidget)
        {
            return (ExpansionWidgetCreator == null) ?
                new ExpansionWidget(rowWidget) : ExpansionWidgetCreator(rowWidget);
        }

        /// <summary>
        /// Create a cell widget used to display a cell for a particular row/column
        /// </summary>
        /// <remarks>
        /// To change the appearance or behaviour of this type of widget derive a new widget
        /// class and override the appropriate methods.  You can then either override this method or 
        /// set the corresponding WidgetCreator delegate property to create widgets of the derived class.
        /// </remarks>
        /// <param name="rowWidget">The rowWidget that this cell widget belongs to</param>
        /// <param name="column">The column the widget is to display</param>
        /// <returns>A widget used to display the header for a column</returns>
        internal protected virtual CellWidget CreateCellWidget(RowWidget rowWidget, Column column)
        {
            return (CellWidgetCreator == null) ?
                new CellWidget(rowWidget, column) : CellWidgetCreator(rowWidget, column);
        }

        /// <summary>
        /// Return the actual display area of the control minus scrollbars and headers
        /// </summary>
        [Browsable(false)]
        public override Rectangle DisplayRectangle
        {
            get
            {
                return new Rectangle(DisplayLeft, DisplayTop, DisplayWidth, DisplayHeight);
            }
        }

        /// <summary>
        /// Return the height of the row display area
        /// </summary>
        [Browsable(false)]
        public virtual int DisplayHeight
        {
            get
            {
                int height = ClientSize.Height;
                if (ShowColumnHeaders)
                {
                    height -= HeaderHeight;
                }
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
        public virtual int DisplayWidth
        {
            get
            {
                int width = ClientSize.Width;
                if (ShowVertScroll)
                    width -= VertScrollBar.Width;
                return Math.Max(0, width);
            }
        }

        /// <summary>
        /// Gets the width of the scrollable area 
        /// </summary>
        [Browsable(false)]
        public virtual int ScrollableDisplayWidth
        {
            get
            {
                int width = ClientSize.Width;
                if (ShowVertScroll)
                    width -= VertScrollBar.Width;
                width -= PinnedPanel.TotalColumnWidth;
                return Math.Max(0, width);
            }
        }

        /// <summary>
        /// Returns the location of the left of the row display area
        /// </summary>
        [Browsable(false)]
        protected virtual int DisplayLeft
        {
            get 
            { 
                if (this.RightToLeft == RightToLeft.Yes)
                    return  (this.ShowVertScroll) ? VertScrollBar.Width : 0;
                else
                    return  0;
            }
        }

        /// <summary>
        /// Returns the location of the top of the row display area
        /// </summary>
        [Browsable(false)]
        protected virtual int DisplayTop
        {
            get { return (this.ShowColumnHeaders) ? _headerHeight : 0; }
        }

        /// <summary>
        /// Update the size/position of widgets when control size or horizontal scroll changes
        /// </summary>
        protected override void LayoutWidgets()
        {
            Rectangle bounds;
            int pinnedPanelWidth = PinnedPanel.TotalColumnWidth;
            bounds = new Rectangle(0,0, pinnedPanelWidth, Height);
            bounds = RtlTranslateRect(bounds);
            PinnedPanel.ClipBounds = bounds;
            PinnedPanel.Bounds = bounds;

            int scrollablePanelWidth = DisplayWidth - pinnedPanelWidth;
            int rowOffset = HorzScrollOffset;
            int rowWidth = Math.Max(scrollablePanelWidth, ScrollablePanel.TotalColumnWidth);          

            // set the ClipBounds first so that they are set by the time the child widget OnLayout
            // is called            
            //
            bounds = new Rectangle(pinnedPanelWidth, 0, scrollablePanelWidth, Height);
            ScrollablePanel.ClipBounds = RtlTranslateRect(bounds);
            bounds = new Rectangle(pinnedPanelWidth - rowOffset, 0, rowWidth, Height);
            ScrollablePanel.Bounds = RtlTranslateRect(bounds);

            // if the current edit widget is offscreen then call OnLayout to make it invisible
            //
            if (EditWidget != null && !EditWidget.Row.Visible)
            {
                EditWidget.RowWidget.OnLayout();
            }
        }

        /// <summary>
        /// Return the automatic width to use for the given column
        /// </summary>
        /// <param name="column">The column to get the width for</param>
        /// <returns>The width of the column</returns>
        internal protected virtual int GetOptimalColumnWidth(Column column)
        {
            int width;
            if (column.Pinned)
                width = PinnedPanel.GetOptimalColumnWidth(column);
            else
                width = ScrollablePanel.GetOptimalColumnWidth(column);
            return width;
        }

        /// <summary>
        /// Return the optimal height to use for the given row
        /// </summary>
        /// <param name="row">The row to get the optimal height for</param>
        /// <returns>The height of the row</returns>
        /// <remarks>
        /// If the row is not currently displayed then this will simply return
        /// the default <see cref="RowHeight"/>.
        /// </remarks>
        internal protected virtual int GetOptimalRowHeight(Row row)
        {
            int pinnedHeight = PinnedPanel.GetOptimalRowHeight(row);
            int scrollableHeight = ScrollablePanel.GetOptimalRowHeight(row);
            return Math.Max(pinnedHeight, scrollableHeight);
        }

        /// <summary>
        /// Update the heights of rows which have auto fit height set to true
        /// </summary>
        private void UpdateRowHeights()
        { 
            PinnedPanel.UpdateAutoFitRowHeights();
            ScrollablePanel.UpdateAutoFitRowHeights();
            UpdateNumVisibleRows();
        }

        /// <summary>
        /// Calculate the number of visible rows
        /// </summary>
        protected virtual void UpdateNumVisibleRows()
        {
            int displayHeight = DisplayHeight;
            int height = 0;
            int numRows = 0;
            foreach (RowWidget rowWidget in ScrollablePanel.RowWidgets)
            {
                // get the height of the row (check for user adjusted heights first)
                //
                int rowHeight = GetUserRowHeight(rowWidget.Row);
                if (rowHeight == 0)
                    rowHeight = rowWidget.RowData.Height;
                height += rowHeight;
                if (height > displayHeight) break;
                numRows++;
            }
            _numVisibleRows = numRows;
        }

        /// <summary>
        /// Reset the <see cref="RowData.Height"/> for all AutoFitHeight row widgets
        /// </summary>
        private void ResetAutoFitRowHeights()
        {
            foreach (RowWidget rowWidget in ScrollablePanel.RowWidgets)
            {
                RowData rowData = rowWidget.RowData;
                if (rowData.AutoFitHeight)
                    rowData.Height = MinRowHeight;
            }
        }

        /// <summary>
        /// Update the width of columns
        /// </summary>
        private void UpdateColumnWidths()
        {
            Columns.SuspendChangeNotification();
            try
            {
                // adjust the pinned column widths first
                //
                foreach (Column column in PinnedPanel.Columns)
                {
                    switch (column.AutoSizePolicy)
                    {
                        case ColumnAutoSizePolicy.AutoIncrease:
                            column.Width = Math.Max(PinnedPanel.GetOptimalColumnWidth(column), column.Width);
                            break;
                        case ColumnAutoSizePolicy.AutoSize:
                            column.Width = PinnedPanel.GetOptimalColumnWidth(column);
                            break;
                    }
                }

                // now get the minimum widths for the scrollable columns
                //
                Hashtable autoMinWidth = new Hashtable();
                _minScrollableWidth = 0;
                float totalAutoFitWeight = 0;
                foreach (Column column in ScrollablePanel.Columns)
                {
                    int minWidth = 0;
                    switch (column.AutoSizePolicy)
                    {
                        case ColumnAutoSizePolicy.Manual:
                            minWidth = (column.Resizable) ? column.MinWidth : column.Width;
                            break;
                        case ColumnAutoSizePolicy.AutoIncrease:
                            minWidth = ScrollablePanel.GetOptimalColumnWidth(column);
                            if (!AutoFitColumns)
                            {
                                column.Width = Math.Max(minWidth, column.Width);
                            }
                            break;
                        case ColumnAutoSizePolicy.AutoSize:
                            minWidth = ScrollablePanel.GetOptimalColumnWidth(column);
                            column.Width = minWidth;
                            break;
                    }

                    if (AutoFitColumns)
                    {
                        if (column.AutoFit)
                        {
                            totalAutoFitWeight += column.AutoFitWeight;
                        }
                        autoMinWidth[column] = minWidth;
                    }
                    _minScrollableWidth += minWidth;
                }

                if (AutoFitColumns)
                {
                    int scrollableWidth = ScrollableDisplayWidth;
                    int availableWidth = scrollableWidth - _minScrollableWidth;
                    int remainingAvailableWidth = availableWidth;
                    foreach (Column column in ScrollablePanel.Columns)
                    {
                        if (column.AutoFit)
                        {
                            int extraWidth = 0;
                            if (remainingAvailableWidth > 0 && totalAutoFitWeight > 0)
                            {
                                extraWidth = (int)(availableWidth * column.AutoFitWeight / totalAutoFitWeight);
                                if (extraWidth > remainingAvailableWidth)
                                    extraWidth = remainingAvailableWidth;
                                remainingAvailableWidth -= extraWidth;
                            }
                            column.Width = (int)autoMinWidth[column] + extraWidth;
                        }
                    }
                }
            }
            finally
            {
                Columns.ResumeChangeNotification(false);
            }
        }

        /// <summary>
        /// Paints the background of the control (within the <see cref="OnPaint"/> event)
        /// </summary>
        /// <param name="e">The <see cref="OnPaint"/> event argument</param>
        /// <remarks>
        /// All VirtualTree painting is done in the <see cref="OnPaint"/> method to enable 
        /// flickerless updates using double buffering.  This method is called by 
        /// <see cref="OnPaint"/> to paint the background.
        /// </remarks>
        protected virtual void PaintBackground(PaintEventArgs e)
        {
            RowStyle.FillRectangle(e.Graphics, e.ClipRectangle);
        }

        /// <summary>
        /// Paints the background image to the specified rectangle of the given context
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        /// <param name="rect">The rectangle to paint to</param>
        protected virtual void PaintBackgroundImage(Graphics graphics, Rectangle rect)
        {
            if (BackgroundImage == null) return;
            DrawingUtilities.DrawImage(graphics, BackgroundImage, rect, BackgroundImageMode);
        }

        /// <summary>
        /// Paint the focus indicator for the tree
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        /// <param name="style">The style to use</param>
        /// <param name="rect">The rectangle to draw the focus indicator in</param>
        internal protected virtual void PaintFocusIndicator(Graphics graphics, Style style, Rectangle rect)
        {
            ControlPaint.DrawFocusRectangle(graphics, rect, style.ForeColor, style.BackColor);
        }

        /// <summary>
        /// Performs all painting for the Virtual Tree control
        /// </summary>
        /// <param name="e"></param>
        /// <remarks>
        /// Calls <see cref="PaintBackground"/> and <see cref="PaintBackgroundImage"/> to
        /// paint the background of the control then calls <see cref="WidgetControl.OnPaint"/>
        /// to paint the <see cref="Widget">Widgets</see>.  Finally it calls 
        /// <see cref="PaintFocusIndicator"/> (if required) to paint the focus indicator.
        /// </remarks>
        protected override void OnPaint(PaintEventArgs e)
        {
            // paint the background and image (if any)
            //
            PaintBackground(e);
            PaintBackgroundImage(e.Graphics, DisplayRectangle);

            // draw the widgets
            //
            base.OnPaint (e);

            // Reset the clipping region (since some of the widgets use SetClip) 
            // If we don't do this we get some very strange interactions
            // with other double buffered controls on the same form.
            //
            DrawingUtilities.ResetClip(e.Graphics);

            // paint the focus indicator - if the selection mode is MainCellText then
            // the focus indicator is painted by the CellWidget
            //
            if (FocusRow != null && FocusRow.Visible && ShowFocusIndicator 
                && SelectionMode != SelectionMode.MainCellText)
            {
                if (SelectionMode == SelectionMode.Cell && SelectedColumn != null)
                {
                    CellWidget cellWidget = this.GetCellWidget(FocusRow, SelectedColumn);
                    if (cellWidget != null)
                    {
                        Style style = cellWidget.GetActiveStyle();
                        Rectangle cellBounds = cellWidget.Bounds;
                        if (!SelectedColumn.Pinned)
                        {
                            DrawingUtilities.SetClip(e.Graphics, ScrollablePanel.ClipBounds);
                        }
                        PaintFocusIndicator(e.Graphics, style, cellBounds);
                        DrawingUtilities.ResetClip(e.Graphics);
                    }
                }
                else  // full row selection
                {
                    RowWidget rowWidget = PinnedPanel.GetRowWidget(FocusRow);
                    if (rowWidget != null)
                    {
                        Style style = rowWidget.GetActiveStyle();
                        Rectangle rowBounds = rowWidget.Bounds;
                        Rectangle displayBounds = DisplayRectangle;
                        if (ShowRowHeaders)
                        {
                            displayBounds.Width -= RowHeaderWidth;
                            if (RightToLeft == RightToLeft.No)
                            {
                                displayBounds.X += RowHeaderWidth;
                            }
                        }
                        
                        Rectangle focusBounds = new Rectangle(displayBounds.X, rowBounds.Y, displayBounds.Width, rowBounds.Height);
                        PaintFocusIndicator(e.Graphics, style, focusBounds);
                    }
                }
            }

        }

        /// <summary>
        /// Overridden to prevent the base Control class painting the background.
        /// </summary>
        /// <remarks>
        /// All VirtualTree painting is done in the <see cref="OnPaint"/> method to enable 
        /// flickerless updates using double buffering.  To paint the control background override
        /// the <see cref="PaintBackground"/> method instead
        /// </remarks>
        /// <param name="pevent"></param>
        protected override void OnPaintBackground(PaintEventArgs pevent)
        {
        }

        #endregion

        #region Styles

        /// <summary>
        /// Return the default style 
        /// </summary>
        protected Style DefaultStyle
        {
            get { return _defaultStyle; }
        }

        /// <summary>
        /// Update the default style properties when the controls properties change
        /// </summary>
        protected virtual void UpdateDefaultStyle()
        {
            DefaultStyle.BackColor = BackColor;
            DefaultStyle.ForeColor = ForeColor;
            DefaultStyle.BorderColor = ForeColor;
            DefaultStyle.Font = Font;
            DefaultStyle.BorderWidth = 0;
            DefaultStyle.VertAlignment = StringAlignment.Center;
            DefaultStyle.HorzAlignment = StringAlignment.Center;
            DefaultStyle.RightToLeft = this.RightToLeft;
        }

        /// <summary>
        /// Intialize the default settings for the classic template
        /// </summary>
        private void InitializeClassicStyleTemplate()
        {
            _defaultHeaderStyle.BorderStyle = Border3DStyle.Raised;
            _defaultHeaderStyle.BorderWidth = 2;
            _defaultHeaderStyle.BorderColor = SystemColors.ControlDark;
            _defaultHeaderPressedStyle.BorderStyle = Border3DStyle.Sunken;
            _defaultHeaderHotStyle.BackColor = SystemColors.ControlLightLight;
            _defaultHeaderHotStyle.ForeColor = SystemColors.ControlText;
            _defaultHeaderDropStyle.BackColor = SystemColors.ControlLightLight;
            _defaultHeaderDropStyle.ForeColor = SystemColors.ControlText;

            _defaultHeaderPrintStyle.BorderStyle = Border3DStyle.Adjust;
            _defaultHeaderPrintStyle.BorderWidth = 2;
            _defaultHeaderPrintStyle.BorderColor = Color.White;
            _defaultHeaderPrintStyle.BackColor = Color.LightGray;
            _defaultHeaderPrintStyle.ForeColor = Color.Black;

            _defaultRowStyle.BorderStyle = Border3DStyle.Adjust;
            _defaultRowStyle.BorderColor = SystemColors.ControlLight;
            _defaultRowStyle.BackColor = SystemColors.Window;
            _defaultRowStyle.ForeColor = SystemColors.WindowText;
            _defaultRowStyle.HorzAlignment = StringAlignment.Near;

            _defaultRowSelectedStyle.BackColor = SystemColors.Highlight;
            _defaultRowSelectedStyle.ForeColor = SystemColors.HighlightText;
            _defaultRowSelectedStyle.BorderColor = SystemColors.Highlight;

            _defaultRowSelectedUnfocusedStyle.BackColor = SystemColors.ControlDark;
            _defaultRowSelectedUnfocusedStyle.ForeColor = SystemColors.ControlLightLight;
            _defaultRowSelectedUnfocusedStyle.BorderColor = SystemColors.ControlDark;

            _defaultRowPrintStyle.BackColor = Color.White;
            _defaultRowPrintStyle.ForeColor = Color.Black;

            _defaultCollapseIcon = Resources.CollapseIcon;
            _defaultExpandIcon = Resources.ExpandIcon;
            _defaultLineStyle = LineStyle.Solid;
        }

        /// <summary>
        /// Intialize the default settings for the classic XP template
        /// </summary>
        private void InitializeClassicXPStyleTemplate()
        {
            InitializeClassicStyleTemplate();
        }

        /// <summary>
        /// Intialize the default settings for the XP template
        /// </summary>
        private void InitializeXPStyleTemplate()
        {
            InitializeClassicStyleTemplate();
            _defaultLineStyle = LineStyle.None;
        }

        /// <summary>
        /// Intialize the default settings for the Vista template
        /// </summary>
        private void InitializeVistaStyleTemplate()
        {
            Color highlightBackColor = Color.White;
            Color highlightGradientColor = Color.FromArgb(214, 239, 252);
            Color highlightBorderColor = Color.FromArgb(153, 222, 253);
            
            Color highlightUBackColor = Color.White;
            Color highlightUGradientColor = Color.LightGray;
            Color highlightUBorderColor = Color.DarkGray;

            _defaultHeaderStyle.BorderStyle = Border3DStyle.Adjust;
            _defaultHeaderStyle.BorderWidth = 1;
            _defaultHeaderStyle.BorderColor = highlightUBorderColor;
            _defaultHeaderStyle.BackColor = highlightUBackColor;
            _defaultHeaderStyle.GradientColor = highlightUGradientColor;
            _defaultHeaderStyle.ForeColor = Color.Black;

            _defaultHeaderHotStyle.BackColor = highlightBackColor;
            _defaultHeaderHotStyle.BorderColor = highlightBorderColor;
            _defaultHeaderHotStyle.GradientColor = highlightGradientColor;

            _defaultHeaderPressedStyle.BackColor = highlightBackColor;
            _defaultHeaderPressedStyle.BorderColor = highlightBorderColor;
            _defaultHeaderPressedStyle.GradientColor = highlightBorderColor;

            _defaultHeaderDropStyle.BackColor = highlightBackColor;
            _defaultHeaderDropStyle.BorderColor = highlightBorderColor;
            _defaultHeaderDropStyle.GradientColor = highlightGradientColor;

            _defaultRowStyle.BorderStyle = Border3DStyle.Adjust;
            _defaultRowStyle.ForeColor = Color.Black;
            _defaultRowStyle.BackColor = Color.White;
            _defaultRowStyle.BorderColor = Color.White;
            _defaultRowStyle.BorderWidth = 1;
            _defaultRowStyle.HorzAlignment = StringAlignment.Near;

            _defaultRowSelectedStyle.BackColor = highlightBackColor;
            _defaultRowSelectedStyle.BorderColor = highlightBorderColor;
            _defaultRowSelectedStyle.GradientColor = highlightGradientColor;
            _defaultRowSelectedStyle.BorderRadius = 3;
            _defaultRowSelectedStyle.BorderStyle = Border3DStyle.Flat;
            _defaultRowSelectedStyle.BorderSide = Border3DSide.All;

            _defaultRowSelectedUnfocusedStyle.BackColor = highlightUBackColor;
            _defaultRowSelectedUnfocusedStyle.BorderColor = highlightUBorderColor;
            _defaultRowSelectedUnfocusedStyle.GradientColor = highlightUGradientColor;

            _defaultHeaderPrintStyle.BorderStyle = Border3DStyle.Adjust;
            _defaultHeaderPrintStyle.BorderWidth = 2;
            _defaultHeaderPrintStyle.BorderColor = Color.White;
            _defaultHeaderPrintStyle.BackColor = Color.LightGray;
            _defaultHeaderPrintStyle.ForeColor = Color.Black;

            _defaultRowPrintStyle.BackColor = Color.White;
            _defaultRowPrintStyle.ForeColor = Color.Black;

            _defaultCollapseIcon = Resources.CollapseVistaIcon;
            _defaultExpandIcon = Resources.ExpandVistaIcon;
            _defaultLineStyle = LineStyle.None;
        }

        /// <summary>
        /// Create the styles used by the control and attach event handlers
        /// </summary>
        private void CreateStyles()
        {
            _defaultStyle = new Style();

            _defaultHeaderStyle = new Style(_defaultStyle);
            _headerStyle = new Style(_defaultHeaderStyle);
            _headerStyle.PropertyChanged += new PropertyChangedEventHandler(OnHeaderStyleChanged);
            
            _defaultHeaderPressedStyle = new Style(_headerStyle);
            _headerPressedStyle = new Style(_defaultHeaderPressedStyle);
            _headerPressedStyle.PropertyChanged += new PropertyChangedEventHandler(OnHeaderStyleChanged);

            _defaultHeaderHotStyle = new Style(_headerStyle);
            _headerHotStyle = new Style(_defaultHeaderHotStyle);
            _headerHotStyle.PropertyChanged += new PropertyChangedEventHandler(OnHeaderStyleChanged);

            _defaultHeaderDropStyle = new Style(_headerStyle);
            _headerDropStyle = new Style(_defaultHeaderDropStyle);
            _headerDropStyle.PropertyChanged += new PropertyChangedEventHandler(OnHeaderStyleChanged);

            _defaultHeaderPrintStyle = new Style(_headerStyle);
            _headerPrintStyle = new Style(_defaultHeaderPrintStyle);

            _defaultRowStyle = new Style(_defaultStyle);
            _rowStyle = new Style(_defaultRowStyle);
            _rowStyle.PropertyChanged += new PropertyChangedEventHandler(OnRowStyleChanged);

            // the defaults for odd/even row styles are the same as for row
            //
            _rowOddStyle = new Style(_rowStyle);
            _rowOddStyle.PropertyChanged += new PropertyChangedEventHandler(OnRowStyleChanged);

            _rowEvenStyle = new Style(_rowStyle);
            _rowEvenStyle.PropertyChanged += new PropertyChangedEventHandler(OnRowStyleChanged);

            _defaultRowSelectedStyle = new Style(_rowStyle);
            _rowSelectedStyle = new Style(_defaultRowSelectedStyle);
            _rowSelectedStyle.PropertyChanged += new PropertyChangedEventHandler(OnRowStyleChanged);

            _defaultRowSelectedUnfocusedStyle = new Style(_rowSelectedStyle);
            _rowSelectedUnfocusedStyle = new Style(_defaultRowSelectedUnfocusedStyle);
            _rowSelectedUnfocusedStyle.PropertyChanged += new PropertyChangedEventHandler(OnRowStyleChanged);

            _defaultRowPrintStyle = new Style(_rowStyle);
            _rowPrintStyle = new Style(_defaultRowPrintStyle);

            _defaultRowPrintEvenStyle = new Style(_rowPrintStyle, _rowEvenStyle.Delta);
            _rowPrintEvenStyle = new Style(_defaultRowPrintEvenStyle);

            _defaultRowPrintOddStyle = new Style(_rowPrintStyle, _rowOddStyle.Delta);
            _rowPrintOddStyle = new Style(_defaultRowPrintOddStyle);

        }

        /// <summary>
        /// Create the styles used by the control and initialize them
        /// </summary>
        protected virtual void InitializeStyles()
        {
            UpdateDefaultStyle();
                      
            _defaultHeaderStyle.Reset();
            _defaultHeaderPressedStyle.Reset();
            _defaultHeaderHotStyle.Reset();
            _defaultHeaderDropStyle.Reset();
            _defaultHeaderPrintStyle.Reset();
            _defaultRowStyle.Reset();
            _defaultRowSelectedStyle.Reset();
            _defaultRowSelectedUnfocusedStyle.Reset();
            _defaultRowPrintStyle.Reset();
            _defaultRowPrintEvenStyle.Reset();
            _defaultRowPrintOddStyle.Reset();

            switch (StyleTemplate)
            {
                case StyleTemplate.Classic:
                    InitializeClassicStyleTemplate();
                    break;
                case StyleTemplate.ClassicXP:
                    InitializeClassicXPStyleTemplate();
                    break;
                case StyleTemplate.XP:
                    InitializeXPStyleTemplate();
                    break;
                case StyleTemplate.Vista:
                    InitializeVistaStyleTemplate();
                    break;
                default:
                    InitializeClassicXPStyleTemplate();
                    break;
            }
        }

        /// <summary>
        /// Handle a change to a header style
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnHeaderStyleChanged(object sender, PropertyChangedEventArgs e)
        {
            Invalidate();
        }

        /// <summary>
        /// Handle a change to a row style
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnRowStyleChanged(object sender, PropertyChangedEventArgs e)
        {
            Invalidate();
        }

        /// <summary>
        /// Updates the default style
        /// </summary>
        /// <param name="e"></param>
        protected override void OnBackColorChanged(EventArgs e)
        {
            base.OnBackColorChanged (e);
            UpdateDefaultStyle();
        }

        /// <summary>
        /// Update the default style
        /// </summary>
        /// <param name="e"></param>
        protected override void OnFontChanged(EventArgs e)
        {
            base.OnFontChanged (e);
            UpdateDefaultStyle();
        }

        /// <summary>
        /// Updates the default style
        /// </summary>
        /// <param name="e"></param>
        protected override void OnForeColorChanged(EventArgs e)
        {
            base.OnForeColorChanged (e);
            UpdateDefaultStyle();
        }

        #endregion

        #region Control Layout and Intialization

        /// <summary>
        /// Get the panel the contains the vertical scrollbar 
        /// </summary>
        protected Panel VertScrollBarPanel
        {
            get { return _vertScrollPanel; }
        }
 
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
                if (value)
                    HorzScrollBar.BringToFront();
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
                VertScrollBarPanel.Visible = value;
                if (value)
                    VertScrollBarPanel.BringToFront();
            }
        }

        /// <summary>
        /// Positions the scrollbars and updates the visible widgets.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLayout(LayoutEventArgs e)
        {
            // ignore layout events that are generated by moving the active editor control
            // 
            if (e.AffectedControl == this || e.AffectedControl == null)
            {           

                int clientHeight = ClientSize.Height;
                int clientWidth = ClientSize.Width;

                if (clientHeight <= 0 || clientWidth <= 0)
                    return;

                // prevent data updates caused by ListChanged events from
                // being processed while we do the layout - this prevents flicker
                // in the case when a data source fires a ListChange event
                //
                SuspendDataUpdate();                
                _clipControl.SuspendLayout();
                bool dataUpdateRequired = false;
                
                try
                {

                    // ensure that the focus and context rows are set if possible
                    //
                    if (_focusRow == null)
                    {
                        _focusRow = TopRow;
                    }
                    if (_contextRow == null)
                    {
                        _contextRow = _focusRow;
                    }

                    UpdateColumnsContext();
                    UpdateWidgets();

                    // Reset the heights of any auto fit height rows
                    //
                    ResetAutoFitRowHeights();

                    // Do an initial layout of widgets so that cell widgets are created as required
                    // and we can determine auto calculated row heights and column widths
                    // Suspend layout of editor controls while we do this to avoid flicker
                    //
                    EditorLayoutSuspended = true;
                    try
                    {
                        LayoutWidgets();
                    }
                    finally
                    {
                        EditorLayoutSuspended = false;
                    }

                    UpdateColumnWidths();
                    UpdateRowHeights();
                    
                    UpdateScrollBars();
                    
                    // layout widgets again now row heights and column widths have been calculated
                    //
                    LayoutWidgets();

                    // adjust the top row if required to display the maximum number of rows
                    //
                    AutoAdjustTopRow();
                    
                    Rectangle bounds;
                    int pinnedWidth = PinnedPanel.Bounds.Width;
                    bounds = new Rectangle(pinnedWidth, 0, clientWidth - pinnedWidth, clientHeight);
                    _clipControl.Bounds = RtlTranslateRect(bounds);
                    _clipControl.Visible = true;

                    // position the scrollbars
                    //
                    int scrollHeight = (ShowHorzScroll) ?  clientHeight - HorzScrollBar.Height : clientHeight;
                    int scrollWidth = (ShowVertScroll) ? clientWidth - VertScrollBar.Width : clientWidth;
                    bounds = new Rectangle(clientWidth - VertScrollBar.Width, 0, VertScrollBar.Width, clientHeight);
                    bounds = RtlTranslateRect(bounds);
                    VertScrollBarPanel.SetBounds(bounds.X, bounds.Y, bounds.Width, bounds.Height);
                    VertScrollBar.Height = scrollHeight;

                    bounds = new Rectangle(0, clientHeight - HorzScrollBar.Height, scrollWidth, HorzScrollBar.Height);
                    bounds = RtlTranslateRect(bounds);
                    HorzScrollBar.SetBounds(bounds.X, bounds.Y, bounds.Width, bounds.Height);     

                    // if row caching is not enabled then dispose of any rows that are no
                    // longer required to support the current display
                    //
                    if (!_enableRowCaching && _rootRow != null)
                    {
                        _rootRow.DisposeUnusedRows();
                    }

                    Invalidate();
                }
                finally
                {
                    _clipControl.ResumeLayout();
                    dataUpdateRequired = _updateRowDataRequired || _updateRowsRequired;
                    ResumeDataUpdate();
                }

                // if the data was updated then we need to do a layout again 
                //
                if (!DataUpdateSuspended && dataUpdateRequired)
                {
                    OnLayout(e);
                }

                // a bug in .NET 2.0 means that the cachedLayoutEvents are not cleared
                // unless we do this.  This means that future calls to PerformLayout()
                // will have incorrect AffectedControl
                //
                base.PerformLayout();

            }
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
            UpdateStyles();
        }

        /// <summary>
        /// Handles the WM_GETDLGCODE and WM_CONTEXTMENU messages 
        /// </summary>
        /// <param name="m"></param>
        protected override void WndProc(ref Message m)
        {
            const int WM_GETDLGCODE = 0x0087;
            const int WM_CONTEXTMENU = 0x007B;
            const int DLGC_WANTARROWS = 0x001;
            const int DLGC_WANTCHARS = 0x080;

            if (m.Msg == WM_CONTEXTMENU)
            {
                // disable WM_CONTEXTMENU - we'll show the menu ourselves
            }
            else
            {
                base.WndProc(ref m);
            }

            // Handle the WM_GETDLGCODE message to get windows to send ArrowKey messages to the control            
            //
            if (m.Msg == WM_GETDLGCODE)
            {
                m.Result = new IntPtr(DLGC_WANTARROWS | DLGC_WANTCHARS);
            }
        }

        /// <summary>
        /// Call <see cref="Control.PerformLayout()"/> when RightToLeft is changed.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnRightToLeftChanged(EventArgs e)
        {
            base.OnRightToLeftChanged (e);
            UpdateDefaultStyle();
            PerformLayout();
        }

        /// <summary>
        /// Is layout of editor controls currently suspended
        /// </summary>
        /// <remarks>
        /// Layout of editor controls is suspended by <see cref="OnLayout"/> while doing
        /// a first pass layout of the child widgets.  This reduces flicker of the editor controls.
        /// </remarks>
        internal protected bool EditorLayoutSuspended
        {
            get { return _editorLayoutSuspended; }
            set { _editorLayoutSuspended = value; }
        }

        /// <summary>
        /// Add an editor control as a child control for the given CellWidget.
        /// </summary>
        /// <param name="cellWidget">The cell widget the editor belongs to</param>
        /// <param name="control">The editor control</param>
        internal void ManageEditorControl(CellWidget cellWidget, Control control)
        {
            if (cellWidget.Column.Pinned)
            {
                control.Parent = this;
            }
            else
            {
                control.Parent = _clipControl;
            }
        }

        #endregion
        
        #region Columns

        /// <summary>
        /// Create a new Column object for Virtual Tree
        /// </summary>
        /// <remarks>
        /// This method can be overridden to allow creation of custom Column objects 
        /// (using a derived class)
        /// </remarks>
        /// <returns>A new Column object</returns>
        public virtual Column CreateColumn()
        {
            return new Column();
        }

        /// <summary>
        /// Return offset of the given scrollable column from the left of the scrollable panel
        /// </summary>
        /// <param name="column">The column to get the offsaet of</param>
        /// <returns>The offset of the column</returns>
        protected int GetScrollableColumnOffset(Column column)
        {
            int result = 0;
            foreach (Column c in ScrollablePanel.Columns)
            {
                if (c == column) return result;
                    result += c.Width;
            }
            return 0;
        }

        /// <summary>
        /// Returns the column to use as the main column.
        /// </summary>
        /// <remarks>
        /// Returns <see cref="MainColumn"/> if non-null or else
        /// the first visible column
        /// </remarks>
        /// <returns>The column to use as the main column</returns>
        internal protected Column GetMainColumn()
        {
            if (_mainColumn != null) return _mainColumn;
            if (_pinnedPanel.Columns != null)
            {
                if (_pinnedPanel.Columns.Count > 0)
                    return _pinnedPanel.Columns[0];
                if (_scrollablePanel.Columns.Count > 0)
                    return _scrollablePanel.Columns[0];
            }
            return null;
        }

        /// <summary>
        /// Return the minimum allowed width for the given column
        /// </summary>
        /// <param name="column">The column to get the minimum width for</param>
        /// <returns>The minimum allowed width</returns>
        /// <remarks>
        /// For <see cref="ColumnAutoSizePolicy.Manual"/> this just the <see cref="Column.MinWidth"/>
        /// for autosize columns this calls <see cref="GetOptimalColumnWidth"/>
        /// </remarks>
        protected int GetMinColumnWidth(Column column)
        {
            if (column.AutoSizePolicy == ColumnAutoSizePolicy.Manual)
                return column.MinWidth;
            else
                return GetOptimalColumnWidth(column);
        }

        /// <summary>
        /// Adjust the <see cref="Column.AutoFitWeight"/> for each of the visible, non-pinned columns
        /// to attempt to change the width of the given column to the specified value. 
        /// </summary>
        /// <param name="column">The primary column that we are adjusting the width of</param>
        /// <param name="requiredWidth">The required width for the column</param>
        /// <remarks>This method is only valid when <see cref="AutoFitColumns"/> is set to true.</remarks>
        internal protected virtual void AdjustColumnAutoFitWeights(Column column, int requiredWidth)
        {
            if (!AutoFitColumns)
                throw new InvalidOperationException("This operation is only valid when AutoFitColumns = true");
           
            Columns.SuspendChangeNotification();

            // find the total weight of all the other columns
            //
            float totalPrevWeight = 0;
            foreach (Column c in ScrollablePanel.Columns)
            {
                if (c != column && c.AutoFit)
                {
                    totalPrevWeight += c.AutoFitWeight;
                }
            }
 
            // now calculate the weight of the column to make its width the correct size
            //
            const float totalWeight = 100.0F;
            const float minWeight = 0.01F;
            float remainingWeight = 0;
            int availableWidth = ScrollableDisplayWidth - MinScrollableWidth; 
            int minColumnWidth = GetMinColumnWidth(column);
            int requiredExtraWidth = Math.Max(requiredWidth - minColumnWidth, 0);
            
            if (availableWidth < requiredExtraWidth)
            {
                column.AutoFitWeight = totalWeight - 1.0F;
            }
            else
            {
                column.AutoFitWeight = Math.Max(totalWeight * requiredExtraWidth / availableWidth, minWeight);
            }

            // calculate the weights of the other columns to add up to the remaining weight
            // based on their previous weight as a proportion of the total previous weight
            //
            remainingWeight = totalWeight - column.AutoFitWeight;            
            foreach (Column c in ScrollablePanel.Columns)
            {
                if (c != column && c.AutoFit)
                {
                    if (totalPrevWeight == 0)
                        c.AutoFitWeight = minWeight;
                    else
                        c.AutoFitWeight = remainingWeight * c.AutoFitWeight / totalPrevWeight;
                }
            }
            Columns.ResumeChangeNotification(true);
        }

        /// <summary>
        /// Raises the SortColumnChanged Event.  This allows clients to handle
        /// changes to the sort column or order programatically instead of using
        /// databinding to do this. 
        /// </summary>
        internal protected virtual void OnSortColumnChanged()
        {
            try
            {
                SuspendLayout();
                if (SortColumnChanged != null)
                {
                    SortColumnChanged(this, new System.EventArgs());
                }
                int index = TopRowIndex;
                UpdateRows(true);
                TopRowIndex = index;
            }
            finally
            {
                ResumeLayout(false);
                PerformLayout();
            }

            if (SelectedRow != null)
            {
                SelectedRow.EnsureVisible();
            }
        }

        /// <summary>
        /// Handle a change to the columns for this tree
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnColumnsChanged(object sender, ListChangedEventArgs e)
        {
            PerformLayout();
        }


        #endregion

        #region CellEvents

        /// <summary>
        /// Invokes the <see cref="CellMouseDown"/> event.
        /// </summary>
        /// <param name="sender">The <see cref="CellWidget"/> that received the mouse down event</param>
        /// <param name="e">The event</param>
        /// <remarks>
        /// Provides a convenient means to allow you to add additional behaviour to the 
        /// MouseDown event.  If you wish to override the default CellWidget MouseDown 
        /// behaviour then you need derive a new CellWidget class and override the 
        /// <see cref="CellWidget.OnMouseDown"/> method.
        /// </remarks>
        internal protected virtual void OnCellMouseDown(CellWidget sender, MouseEventArgs e)
        {
            if (CellMouseDown != null)
            {
                CellMouseDown(sender, e);
            }
        }

        /// <summary>
        /// Invokes the <see cref="CellMouseUp"/> event.
        /// </summary>
        /// <param name="sender">The <see cref="CellWidget"/> that received the mouse up event</param>
        /// <param name="e">The event</param>
        /// <remarks>
        /// Provides a convenient means to allow you to add additional behaviour to the 
        /// MouseUp event.  If you wish to override the default CellWidget MouseUp 
        /// behaviour then you need derive a new CellWidget class and override the 
        /// <see cref="CellWidget.OnMouseUp"/> method.
        /// </remarks>
        internal protected virtual void OnCellMouseUp(CellWidget sender, MouseEventArgs e)
        {
            if (CellMouseUp != null)
            {
                CellMouseUp(sender, e);
            }
        }

        /// <summary>
        /// Invokes the <see cref="CellClick"/> event.
        /// </summary>
        /// <param name="sender">The <see cref="CellWidget"/> that received the click event</param>
        /// <param name="e">The event</param>
        /// <remarks>
        /// Provides a convenient means to allow you to add additional behaviour to the 
        /// Click event.  If you wish to override the default CellWidget Click 
        /// behaviour then you need derive a new CellWidget class and override the 
        /// <see cref="CellWidget.OnClick"/> method.
        /// </remarks>
        internal protected virtual void OnCellClick(CellWidget sender, EventArgs e)
        {
            if (CellClick != null)
            {
                CellClick(sender, e);
            }
        }

        /// <summary>
        /// Invokes the <see cref="CellDoubleClick"/> event.
        /// </summary>
        /// <param name="sender">The <see cref="CellWidget"/> that received the double click event</param>
        /// <param name="e">The event</param>
        /// <remarks>
        /// Provides a convenient means to allow you to add additional behaviour to the 
        /// Click event.  If you wish to override the default CellWidget DoubleClick 
        /// behaviour then you need derive a new CellWidget class and override the 
        /// <see cref="CellWidget.OnDoubleClick"/> method.
        /// </remarks>
        internal protected virtual void OnCellDoubleClick(CellWidget sender, EventArgs e)
        {
            if (CellDoubleClick != null)
            {
                CellDoubleClick(sender, e);
            }
        }

        /// <summary>
        /// Invokes the <see cref="RowExpand"/> event.
        /// </summary>
        /// <param name="row">The row being expanded</param>
        internal protected virtual void OnRowExpand(Row row)
        {                 
            if (RowExpand != null)
            {
                RowExpand(this, new RowEventArgs(row));
            }
        }

        /// <summary>
        /// Invokes the <see cref="RowCollapse"/> event.
        /// </summary>
        /// <param name="row">The row being collapsed</param>
        internal protected virtual void OnRowCollapse(Row row)
        {
            if (RowCollapse != null)
            {
                RowCollapse(this, new RowEventArgs(row));
            }
        }

        #endregion

        #region Keyboard Event Handling

        /// <summary>
        /// Is the keyboard currently being used to move the focus within Virtual Tree
        /// </summary>
        protected virtual bool KeyboardNavigationInUse
        {
            get { return _keyboardNavigationInUse; }
            set { _keyboardNavigationInUse = value; }
        }

        /// <summary>
        /// Handle Tab Navigation into/out of the VirtualTree
        /// </summary>
        /// <param name="directed"></param>
        /// <param name="forward"></param>
        protected override void Select(bool directed, bool forward)
        {
            base.Select (directed, forward);
            if (directed && EditOnKeyboardFocus)
            {
                if (forward)
                {
                    SelectedRow = GetRow(FirstRowIndex);
                    EditFirstCellInFocusRow();
                }
                else
                {
                    SelectedRow = GetRow(LastRowIndex);
                    EditLastCellInFocusRow();
                }
            }
        }

        /// <summary>
        /// Return the first CellWidget within the given row that meets the given criteria
        /// </summary>
        /// <param name="row">The row to look in</param>
        /// <param name="editable">Does the cell widget have to be editable</param>
        /// <param name="selectable">Does the cell widget have to be selectable</param>
        /// <returns>The first cell widget within the row that meets the criteria if any </returns>
        /// <remarks>
        /// The method checks both the <see cref="PinnedPanel"/> and the <see cref="ScrollablePanel"/>
        /// to find the first <see cref="CellWidget"/> within the given row that meets the criteria.
        /// </remarks>
        protected virtual CellWidget GetFirstCellWidgetInRow(Row row, bool selectable, bool editable)
        {
            RowWidget rowWidget = PinnedPanel.GetRowWidget(row);
            CellWidget cellWidget = null;
            if (rowWidget != null)
            {
                cellWidget = rowWidget.GetFirstCellWidget(selectable, editable);
            }

            if (cellWidget == null)
            {
                rowWidget = ScrollablePanel.GetRowWidget(row);
                if (rowWidget != null)
                {
                    cellWidget = rowWidget.GetFirstCellWidget(selectable, editable);
                }
            }
            return cellWidget;
        }


        /// <summary>
        /// Return the next CellWidget within the given row starting from the given column that 
        /// meets the given criteria
        /// </summary>
        /// <param name="column">The column to start from</param>
        /// <param name="row">The row to look in</param>
        /// <param name="editable">Does the cell widget have to be editable</param>
        /// <param name="selectable">Does the cell widget have to be selectable</param>
        /// <returns>The next cell widget within the row that meets the criteria if any </returns>
        /// <remarks>
        /// The method checks both the <see cref="PinnedPanel"/> and the <see cref="ScrollablePanel"/>
        /// to find the next <see cref="CellWidget"/> within the given row that meets the criteria.
        /// </remarks>
        protected virtual CellWidget GetNextCellWidgetInRow(Row row, Column column, 
                                                            bool selectable, bool editable)
        {
            RowWidget rowWidget = null;
            CellWidget cellWidget = null;
            if (column.Pinned)
            {
                rowWidget = PinnedPanel.GetRowWidget(row);
                cellWidget = rowWidget.GetNextCellWidget(column, selectable, editable);
                if (cellWidget == null)
                {
                    rowWidget = ScrollablePanel.GetRowWidget(row);
                    cellWidget = rowWidget.GetFirstCellWidget(selectable, editable);
                }
            }
            else
            {
                rowWidget = ScrollablePanel.GetRowWidget(row);
                cellWidget = rowWidget.GetNextCellWidget(column, selectable, editable);
            }
            return cellWidget;
        }

        /// <summary>
        /// Return the prior CellWidget within the given row that meets the given criteria,
        /// starting from the given column
        /// </summary>
        /// <param name="row">The row to look in</param>
        /// <param name="column">The column to start from</param>
        /// <param name="editable">Does the cell widget have to be editable</param>
        /// <param name="selectable">Does the cell widget have to be selectable</param>
        /// <returns>The prior cell widget within the row that meets the criteria if any </returns>
        /// <remarks>
        /// The method checks both the <see cref="PinnedPanel"/> and the <see cref="ScrollablePanel"/>
        /// to find the prior <see cref="CellWidget"/> within the given row that meets the criteria.
        /// </remarks>
        protected virtual CellWidget GetPriorCellWidgetInRow(Row row, Column column,
                                                             bool selectable, bool editable)
        {
            RowWidget rowWidget = null;
            CellWidget cellWidget = null;
            if (column.Pinned)
            {
                rowWidget = PinnedPanel.GetRowWidget(row);
                cellWidget = rowWidget.GetPriorCellWidget(column, selectable, editable);
            }
            else
            {
                rowWidget = ScrollablePanel.GetRowWidget(row);
                cellWidget = rowWidget.GetPriorCellWidget(column, selectable, editable);
                if (cellWidget == null)
                {
                    rowWidget = PinnedPanel.GetRowWidget(row);
                    cellWidget = rowWidget.GetLastCellWidget(selectable, editable);
                }
            }
            return cellWidget;
        }

        /// <summary>
        /// Return the last CellWidget within the given row that meets the given criteria
        /// </summary>
        /// <param name="row">The row to look in</param>
        /// <param name="editable">Does the cell widget have to be editable</param>
        /// <param name="selectable">Does the cell widget have to be selectable</param>
        /// <returns>The last cell widget within the row that meets the given criteria if any </returns>
        /// <remarks>
        /// The method checks both the <see cref="PinnedPanel"/> and the <see cref="ScrollablePanel"/>
        /// to find the last <see cref="CellWidget"/> within the given row that meets the criteria.
        /// </remarks>
        protected virtual CellWidget GetLastCellWidgetInRow(Row row, bool selectable, bool editable)
        {
            RowWidget rowWidget = ScrollablePanel.GetRowWidget(row);
            CellWidget cellWidget = null;
            if (rowWidget != null)
            {
                cellWidget = rowWidget.GetLastCellWidget(selectable, editable);
            }

            if (cellWidget == null)
            {
                rowWidget = PinnedPanel.GetRowWidget(row);
                if (rowWidget != null)
                {
                    cellWidget = rowWidget.GetLastCellWidget(selectable, editable);
                }
            }
            return cellWidget;
        }

        /// <summary>
        /// Return the CellWidget for the given row and column (if any)
        /// </summary>
        /// <param name="row">The row the cell widget belongs to</param>
        /// <param name="column">The column of the cell widget</param>
        /// <returns>The cell widget if any for the given row/column </returns>
        protected virtual CellWidget GetCellWidget(Row row, Column column)
        {
            RowWidget rowWidget = null;
            if (column.Pinned)
            {
                rowWidget = PinnedPanel.GetRowWidget(row);
            }
            else
            {
                rowWidget = ScrollablePanel.GetRowWidget(row);
            }
            return rowWidget.GetCellWidget(column);
        }

        /// <summary>
        /// Return the first editable CellWidget within the given row
        /// </summary>
        /// <param name="row">The row to look in</param>
        /// <returns>The first editable cell widget within the row if any </returns>
        /// <remarks>
        /// The method checks both the <see cref="PinnedPanel"/> and the <see cref="ScrollablePanel"/>
        /// to find the first <see cref="CellWidget"/> within the given row that is editable.
        /// </remarks>
        protected virtual CellWidget GetFirstEditableCellWidgetInRow(Row row)
        {
            return GetFirstCellWidgetInRow(row, false, true);
        }

        /// <summary>
        /// Return the next editable CellWidget within the given row starting from the given column
        /// </summary>
        /// <param name="column">The column to start from</param>
        /// <param name="row">The row to look in</param>
        /// <returns>The next editable cell widget within the row if any </returns>
        /// <remarks>
        /// The method checks both the <see cref="PinnedPanel"/> and the <see cref="ScrollablePanel"/>
        /// to find the next <see cref="CellWidget"/> within the given row that is editable.
        /// </remarks>
        protected virtual CellWidget GetNextEditableCellWidgetInRow(Row row, Column column)
        {
            return GetNextCellWidgetInRow(row, column, false, true);
        }

        /// <summary>
        /// Return the prior editable CellWidget within the given row starting from the given column
        /// </summary>
        /// <param name="column">The column to start from</param>
        /// <param name="row">The row to look in</param>
        /// <returns>The prior editable cell widget within the row if any </returns>
        /// <remarks>
        /// The method checks both the <see cref="PinnedPanel"/> and the <see cref="ScrollablePanel"/>
        /// to find the prior <see cref="CellWidget"/> within the given row that is editable.
        /// </remarks>
        protected virtual CellWidget GetPriorEditableCellWidgetInRow(Row row, Column column)
        {
            return GetPriorCellWidgetInRow(row, column, false, true);
        }

        /// <summary>
        /// Return the last editable CellWidget within the given row 
        /// </summary>
        /// <param name="row">The row to look in</param>
        /// <returns>The last editable cell widget within the row if any </returns>
        /// <remarks>
        /// The method checks both the <see cref="PinnedPanel"/> and the <see cref="ScrollablePanel"/>
        /// to find the last <see cref="CellWidget"/> within the given row that is editable.
        /// </remarks>
        protected virtual CellWidget GetLastEditableCellWidgetInRow(Row row)
        {
            return GetLastCellWidgetInRow(row, false, true);
        }

        /// <summary>
        /// Edit the next cell widget in the tab order
        /// </summary>
        /// <param name="currentCell">The current cell being edited (if any)</param>
        /// <returns>True if there was a widget to move to</returns>
        protected virtual bool EditNext(CellWidget currentCell)
        {
            CellWidget cellWidget = null;
            if (currentCell != null)
            {
                cellWidget = GetNextEditableCellWidgetInRow(currentCell.Row, currentCell.Column);
            }
            if (cellWidget == null)
            {
                if (MoveFocusRow(1, KeyboardSelectionMode.MoveSelection))
                {
                    cellWidget = GetFirstEditableCellWidgetInRow(FocusRow);
                }
            }
            if (cellWidget != null)
            {
                EnsureColumnVisible(cellWidget.Column);
            }
            StartEdit(cellWidget);        
            return (cellWidget != null);
        }

        /// <summary>
        /// Edit the previous cell widget in the tab order
        /// </summary>
        /// <param name="currentCell">The current cell being edited (if any)</param>
        /// <returns>True if there was a widget to move to</returns>
        protected virtual bool EditPrior(CellWidget currentCell)
        {

            CellWidget cellWidget = null;
            if (currentCell != null)
            {
                cellWidget = GetPriorEditableCellWidgetInRow(currentCell.Row, currentCell.Column);
            }
            if (cellWidget == null)
            {
                if (MoveFocusRow(-1, KeyboardSelectionMode.MoveSelection))
                {
                    cellWidget = GetLastEditableCellWidgetInRow(FocusRow);
                }
            }
            if (cellWidget != null)
            {
                EnsureColumnVisible(cellWidget.Column);
            }
            StartEdit(cellWidget);
            return (cellWidget != null);
        }

        /// <summary>
        /// Select the next cell in the current row when <see cref="SelectionMode"/> is Cell
        /// </summary>
        /// <returns>True if there was a cell to select</returns>
        public virtual bool SelectNextCell()
        {
            if (!CompleteEdit()) return false;

            CellWidget cellWidget = null;
            if (SelectedColumn != null && FocusRow != null)
            {
                cellWidget = GetNextCellWidgetInRow(FocusRow, SelectedColumn, true, false);
            }
            if (cellWidget != null)
            {
                SelectedColumn = cellWidget.Column;
                EnsureColumnVisible(cellWidget.Column);
            }
            return (cellWidget != null);
        }

        /// <summary>
        /// Select the prior cell in the current row when <see cref="SelectionMode"/> is Cell
        /// </summary>
        /// <returns>True if there was a cell to select</returns>
        public virtual bool SelectPriorCell()
        {
            if (!CompleteEdit()) return false;

            CellWidget cellWidget = null;
            if (SelectedColumn != null && FocusRow != null)
            {
                cellWidget = GetPriorCellWidgetInRow(FocusRow, SelectedColumn, true, false);
            }
            if (cellWidget != null)
            {
                SelectedColumn = cellWidget.Column;
                EnsureColumnVisible(cellWidget.Column);
            }
            return (cellWidget != null);
        }

        /// <summary>
        /// Select the next row (from the current focus row)
        /// </summary>
        /// <returns>True if there was a row to select</returns>
        public virtual bool SelectNextRow()
        {
            return MoveFocusRow(1, KeyboardSelectionMode.MoveSelection);
        }

        /// <summary>
        /// Select the previous row (from the current focus row)
        /// </summary>
        /// <returns>True if there was a row to select</returns>
        public virtual bool SelectPriorRow()
        {
            return MoveFocusRow(-1, KeyboardSelectionMode.MoveSelection);
        }

        /// <summary>
        /// Process the Left Arrow Command Key when there is no active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessLeftCmdKey(Keys modifiers)
        {
            Row row = FocusRow;
            if (row == null) return false;
            if (SelectionMode == SelectionMode.Cell)
            {
                KeyboardNavigationInUse = true;
                SelectPriorCell();
            }
            else
            {
                if (row.Expanded && row.NumChildren > 0)
                {
                    row.Collapse();
                }
                else if (row.ParentRow != null)
                {
                    Row parentRow = row.ParentRow;
                    if (this.ShowRootRow || parentRow != this.RootRow)
                    {
                        this.SelectedRow = parentRow;
                    }
                }
            }
            return true;
        }

        /// <summary>
        /// Process the Right Arrow Command Key when there is no active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessRightCmdKey(Keys modifiers)
        {
            Row row = FocusRow;
            if (row == null) return false;
            if (SelectionMode == SelectionMode.Cell)
            {
                KeyboardNavigationInUse = true;
                SelectNextCell();
            }
            else
            {
                if (row.Expanded)
                {
                    Row childRow = row.ChildRowByIndex(0);
                    if (childRow != null)
                    {
                        this.SelectedRow = childRow;
                    }
                }
                else
                {
                    row.Expand();
                }
            }
            return true;
        }

        /// <summary>
        /// Process the Add Command Key when there is no active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessAddCmdKey(Keys modifiers)
        {
            if (FocusRow == null) return false;
            FocusRow.Expand();
            return true;
        }

        /// <summary>
        /// Process the Subtract Command Key when there is no active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessSubtractCmdKey(Keys modifiers)
        {
            if (FocusRow != null)
                FocusRow.Collapse();
            return true;
        }

        /// <summary>
        /// Return the keyboard selection mode to use for the given set of keys
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>The keyboard selection mode to use</returns>
        protected virtual KeyboardSelectionMode GetKeyboardSelectionMode(Keys modifiers)
        {
            modifiers &= Keys.Modifiers;
            switch (modifiers)
            {
                case Keys.Shift:
                    return KeyboardSelectionMode.SelectRange;
                case Keys.Control:
                    return KeyboardSelectionMode.MoveAnchor;
                case Keys.Control | Keys.Shift:
                    return KeyboardSelectionMode.ExtentSelectRange;
                default:
                    return KeyboardSelectionMode.MoveSelection;
            }
        }

        /// <summary>
        /// Process the Down Arrow Key Command when there is no active editor
        /// </summary>
        /// <param name="modifiers">The modifiers pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessDownCmdKey(Keys modifiers)
        {
            KeyboardNavigationInUse = true;
            KeyboardSelectionMode selectionMode = GetKeyboardSelectionMode(modifiers);
            return MoveFocusRow(1, selectionMode);
        }

        /// <summary>
        /// Process the Up Arrow Key Command when there is no active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessUpCmdKey(Keys modifiers)
        {
            KeyboardNavigationInUse = true;
            KeyboardSelectionMode selectionMode = GetKeyboardSelectionMode(modifiers);
            return MoveFocusRow(-1, selectionMode);
        }

        /// <summary>
        /// Process the PageDown Key Command when there is no active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessPageDownCmdKey(Keys modifiers)
        {
            KeyboardNavigationInUse = true;
            KeyboardSelectionMode selectionMode = GetKeyboardSelectionMode(modifiers);
            return MoveFocusRow(NumVisibleRows, selectionMode);
        }

        /// <summary>
        /// Process the PageUp Key Command when there is no active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessPageUpCmdKey(Keys modifiers)
        {
            KeyboardNavigationInUse = true;
            KeyboardSelectionMode selectionMode = GetKeyboardSelectionMode(modifiers);
            return MoveFocusRow(-NumVisibleRows, selectionMode);
        }

        /// <summary>
        /// Process the Home Key Command when there is no active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessHomeCmdKey(Keys modifiers)
        {
            KeyboardNavigationInUse = true;
            KeyboardSelectionMode selectionMode = GetKeyboardSelectionMode(modifiers);
            return MoveFocusRow(-LastRowIndex, selectionMode);
        }

        /// <summary>
        /// Process the End Key Command when there is no active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessEndCmdKey(Keys modifiers)
        {
            KeyboardNavigationInUse = true;
            KeyboardSelectionMode selectionMode = GetKeyboardSelectionMode(modifiers);
            return MoveFocusRow(LastRowIndex, selectionMode);
        }

        /// <summary>
        /// Process the Space Key Command when there is no active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessSpaceCmdKey(Keys modifiers)
        {
            bool handled = false;

            // if edit on key press is true then require control-space for selection
            if (!EditOnKeyPress || modifiers == Keys.Control)
            {
                if (FocusRow != null)
                {
                    ExtendSelectRow(FocusRow);
                    handled = true;
                }
            }
            return handled;
        }

        /// <summary>
        /// Process the Enter Key when there is no active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessEnterCmdKey(Keys modifiers)
        {
            return true;
        }

        /// <summary>
        /// Process the Escape Key when there is no active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessEscapeCmdKey(Keys modifiers)
        {
            return true;
        }

        /// <summary>
        /// Process the F2 Key Command when there is no active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessF2CmdKey(Keys modifiers)
        {
            if (SelectionMode == SelectionMode.Cell && SelectedColumn != null)
                return EditCurrentCellInFocusRow();
            else
                return EditFirstCellInFocusRow();
        }

        /// <summary>
        /// Process the Apps Key Command when there is no active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessAppsCmdKey(Keys modifiers)
        {
            Row row = FocusRow;
            Point position = new Point(0, DisplayTop);
            if (row != null)
            {
                row.EnsureVisible();
                RowWidget widget = PinnedPanel.GetRowWidget(row);
                if (widget != null)
                {
                    position = widget.Bounds.Location;
                }
            }
            position.Offset(20, 20);
            ShowRowContextMenu(FocusRow, position);
            return true;
        }

        /// <summary>
        /// Use <see cref="SendKeys"/> to send a single character to the active control 
        /// </summary>
        /// <param name="c">The character to send</param>
        protected virtual void SendKey(char c)
        {
            string s = null;
            switch (c)
            {
                case '+':
                case '^':
                case '%':
                case '~':
                case '*':
                case '&':
                case '(':
                case ')':
                case '{':
                case '}':
                case '[':
                case ']':
                    s = "{" + c + "}";
                    break;
                default:
                    s = new string(c, 1);
                    break;
            }

            // compensate for Caps Lock because the target control will translate the key to upper case
            // if caps lock is on
            //
            if (IsKeyLocked(Keys.CapsLock))
            {
                s = s.ToLower(InputLanguage.CurrentInputLanguage.Culture);
            }
            SendKeys.Send(s);
        }

        /// <summary>
        /// Overridden to handle key press events to optionally initiate cell editing.
        /// </summary>
        /// <remarks>
        /// If <see cref="EditOnKeyPress"/> is true then this event handler will start
        /// a cell edit.
        /// </remarks>
        /// <param name="e"></param>
        protected override void OnKeyPress(KeyPressEventArgs e)
        {
            base.OnKeyPress(e);
            if (e.Handled) return;
            bool controlPressed = (ModifierKeys & Keys.Control) != 0;
            bool altPressed = (ModifierKeys & Keys.Alt) != 0;
            if (EditOnKeyPress && !controlPressed && !altPressed)
            {
                if (SelectedRows.Count == 1)
                {
                    if (SelectionMode == SelectionMode.Cell)
                    {
                        if (EditCurrentCellInFocusRow())
                        {
                            SendKey(e.KeyChar);
                        }
                    }
                    else
                    {
                        if (EditFirstCellInFocusRow())
                        {
                            SendKey(e.KeyChar);
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Process key down commands when there is no active editor
        /// </summary>
        /// <param name="keys">The keys pressed</param>
        /// <returns>True if the keys were handled</returns>
        /// <remarks>
        /// Handles keyboard navigation when there is no active editor
        /// </remarks>
        /// <seealso cref="ProcessEditCmdKeys"/>
        protected virtual bool ProcessNormalCmdKeys(Keys keys)
        {
            Keys baseKey = keys & ~Keys.Modifiers;
            Keys modifiers = keys & Keys.Modifiers;
        switch (baseKey)
            {
                case Keys.Escape:
                    return ProcessEscapeCmdKey(modifiers);
                case Keys.Enter:
                    return ProcessEnterCmdKey(modifiers);
                case Keys.Left:
                    // flip left/right keys for right to left layout
                    //
                    if (this.RightToLeft == RightToLeft.Yes)
                        return ProcessRightCmdKey(modifiers);
                    else
                        return ProcessLeftCmdKey(modifiers);
                case Keys.Right:
                    // flip left/right keys for right to left layout
                    //
                    if (this.RightToLeft == RightToLeft.Yes)
                        return ProcessLeftCmdKey(modifiers);
                    else
                        return ProcessRightCmdKey(modifiers);
                case Keys.Subtract:
                    return ProcessSubtractCmdKey(modifiers);
                case Keys.Add:
                    return ProcessAddCmdKey(modifiers);
                case Keys.Up:
                    return ProcessUpCmdKey(modifiers);
                case Keys.Down:
                    return ProcessDownCmdKey(modifiers);
                case Keys.PageUp:
                    return ProcessPageUpCmdKey(modifiers);
                case Keys.PageDown:
                    return ProcessPageDownCmdKey(modifiers);
                case Keys.Home:
                    return ProcessHomeCmdKey(modifiers);
                case Keys.End:
                    return ProcessEndCmdKey(modifiers);
                case Keys.Space:
                    return ProcessSpaceCmdKey(modifiers);
                case Keys.F2:
                    return ProcessF2CmdKey(modifiers);
                case Keys.Apps:
                    return ProcessAppsCmdKey(modifiers); 
            }
            return false;
        }

        /// <summary>
        /// Process the Escape Key Command when there is an active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessEditEscapeCmdKey(Keys modifiers)
        {
            AbandonEdit();
            return true;
        }

        /// <summary>
        /// Process the Enter Key Command when there is an active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessEditEnterCmdKey(Keys modifiers)
        {
            CompleteEdit();
            return true;
        }

        /// <summary>
        /// Process the Tab Key Command when there is an active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessEditTabCmdKey(Keys modifiers)
        {
            CellWidget currentCell = EditWidget;

            // Finish editing the current cell.  If we can't then swallow the
            // tab key by returning true
            //
            if (!CompleteEdit()) return true;

            KeyboardNavigationInUse = true;
            if (modifiers == Keys.Shift)
                return EditPrior(currentCell);
            else
                return EditNext(currentCell);
        }

        /// <summary>
        /// Process the Down Key Command when there is an active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessEditDownCmdKey(Keys modifiers)
        {
            CellEditor editor = EditWidget.CellData.Editor;
            bool handled = false;
            if (modifiers == Keys.Control)
            {
                KeyboardNavigationInUse = true;
                EditNextCellInColumn();
                handled = true;
            }
            else
            {
                if (!editor.UsesUpDownKeys)
                {
                    if (CompleteEdit())
                    {
                        Focus(); // restore focus to the main control
                        KeyboardNavigationInUse = true;
                        SelectNextRow();
                    }
                    handled = true;
                }
            }
            return handled;
        }

        /// <summary>
        /// Process the Up Key Command when there is an active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessEditUpCmdKey(Keys modifiers)
        {
            CellEditor editor = EditWidget.CellData.Editor;
            bool handled = false;
            if (modifiers == Keys.Control)
            {
                if (CompleteEdit())
                {
                    KeyboardNavigationInUse = true;
                    EditPriorCellInColumn();
                }
                handled = true;
            }
            else
            {
                if (!editor.UsesUpDownKeys)
                {
                    if (CompleteEdit())
                    {
                        Focus(); // restore focus to the main control
                        KeyboardNavigationInUse = true;
                        SelectPriorRow();
                    }
                    handled = true;
                }
            }
            return handled;
        }

        /// <summary>
        /// Process the Left Arrow Command Key when there is an active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessEditLeftCmdKey(Keys modifiers)
        {
            CellEditor editor = EditWidget.CellData.Editor;
            bool handled = false;
            if (modifiers == Keys.Control)
            {
                KeyboardNavigationInUse = true;
                EditPriorCell();
                handled = true;
            }
            else
            {
                if (!editor.UsesLeftRightKeys)
                {
                    if (CompleteEdit())
                    {
                        Focus(); // restore focus to the main control
                        KeyboardNavigationInUse = true;
                        SelectPriorCell();
                    }
                    handled = true;
                }
            }
            return handled;
        }

        /// <summary>
        /// Process the Right Arrow Command Key when there is an active editor
        /// </summary>
        /// <param name="modifiers">The modifier keys pressed</param>
        /// <returns>True if the key was handled</returns>
        protected virtual bool ProcessEditRightCmdKey(Keys modifiers)
        {
            CellEditor editor = EditWidget.CellData.Editor;
            bool handled = false;
            if (modifiers == Keys.Control)
            {
                KeyboardNavigationInUse = true;
                EditNextCell();
                handled = true;
            }
            else
            {
                if (!editor.UsesLeftRightKeys)
                {
                    if (CompleteEdit())
                    {
                        Focus();  // restore focus to the main control
                        KeyboardNavigationInUse = true;
                        SelectNextCell();
                    }
                    handled = true;
                }
            }
            return handled;
        }

        /// <summary>
        /// Process command keys when there is an active editor
        /// </summary>
        /// <param name="keys">The keys to process</param>
        /// <returns>True if the keys were handled</returns>
        /// <remarks>
        /// Handles Escape Key to abandon editing, Enter Key to complete editing and
        /// Tab Key to navigate to the next editable cell. 
        /// </remarks>
        /// <seealso cref="ProcessNormalCmdKeys"/>
        protected virtual bool ProcessEditCmdKeys(Keys keys)
        {
            Keys baseKey = keys & ~Keys.Modifiers;
            Keys modifiers = keys & Keys.Modifiers;
            switch (baseKey)
            {
                case Keys.Escape:
                    return ProcessEditEscapeCmdKey(modifiers);
                case Keys.Enter:
                    return ProcessEditEnterCmdKey(modifiers);
                case Keys.Tab:
                    return ProcessEditTabCmdKey(modifiers);
                case Keys.Down:
                    return ProcessEditDownCmdKey(modifiers);
                case Keys.Up:
                    return ProcessEditUpCmdKey(modifiers);
                case Keys.Left:
                    // flip right/left keys for right to left layout
                    if (this.RightToLeft == RightToLeft.Yes)
                        return ProcessEditRightCmdKey(modifiers);
                    else
                        return ProcessEditLeftCmdKey(modifiers);
                case Keys.Right:
                    // flip right/left keys for right to left layout
                    if (this.RightToLeft == RightToLeft.Yes)
                        return ProcessEditLeftCmdKey(modifiers);
                    else
                        return ProcessEditRightCmdKey(modifiers);
            }
            return false;
        }

        /// <summary>
        /// Handles keyboard navigation within the tree
        /// </summary>
        /// <param name="msg"></param>
        /// <param name="keys"></param>
        /// <returns></returns>
        protected override bool ProcessCmdKey(ref Message msg, Keys keys)
        {
            const int WM_KEYDOWN = 0x100;
            
            bool handled = false;
            if (msg.Msg == WM_KEYDOWN)
            {
                // if there is no active edit widget then handle arrow keys etc for nav
                //
                if (this.EditWidget == null)
                {
                    handled = ProcessNormalCmdKeys(keys);
                }
                else
                {
                    handled = ProcessEditCmdKeys(keys);
                }
            }

            if (handled)
            {
                Update();  // force any paint events to be processed
                return true;
            }
            return base.ProcessCmdKey(ref msg, keys);     
        }
          
        #endregion

        #region Selection and Focus Management


        /// <summary>
        /// Get/Set the current select anchor row (used for extended selection)
        /// </summary>
        internal virtual protected Row AnchorRow
        {
            get { return _anchorRow; }
            set { _anchorRow = value; }
        }

        /// <summary>
        /// Raises the SelectionChanging Event.  Called prior to the SelectedRows being changed by the user.
        /// </summary>
        /// <remarks>
        /// This method is not called if the SelectedRows are changed programmatically. 
        /// </remarks>
        /// <param name="startRow">The first row affected by the change</param>
        /// <param name="endRow">The last row affected by the change</param>
        /// <param name="change">The type of selection change</param>
        /// <returns>True if the selection should be allowed to change</returns>
        protected virtual bool AllowSelectionChange(Row startRow, Row endRow, SelectionChange change)
        {
            if (SelectionChanging != null)
            {
                SelectionChangingEventArgs e = new SelectionChangingEventArgs(startRow, endRow, change);
                SelectionChanging(this, e);
                return !e.Cancel;
            }
            return true;
        }

        /// <summary>
        /// Raises the SelectionChanged Event.  
        /// </summary>
        protected virtual void OnSelectionChanged()
        {
            if (SelectionChanged != null)
            {
                SelectionChanged(this, new System.EventArgs());
            }
        }

        /// <summary>
        /// Handle a change to the selected rows.  Invalidates the control and calls 
        /// OnSelectionChanged
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnSelectedRowsChanged(object sender, ListChangedEventArgs e)
        {
            Invalidate();
            OnSelectionChanged();
        }

        /// <summary>
        /// Raises the OnFocusRowChanged Event.  Called when the FocusRow property has changed.
        /// </summary>
        protected virtual void OnFocusRowChanged()
        {
            if (FocusRowChanged != null)
            {
                FocusRowChanged(this, new System.EventArgs());
            }
        }

        /// <summary>
        /// Move the current focus row the number of rows indicated by rowOffset. 
        /// </summary>
        /// <param name="rowOffset">The number/direction in which to move focus</param>
        /// <param name="mode">Determines the effect of this operation on selection</param>
        /// <returns>True if the focus was moved</returns>
        protected virtual bool MoveFocusRow(int rowOffset, KeyboardSelectionMode mode)
        {

            // if there is no focus row then default to the top visible row
            //
            int rowIndex = TopRowIndex;
            if (FocusRow != null)
            {
                rowIndex = FocusRow.RowIndex;
            }           
            rowIndex += rowOffset;
            if (rowIndex < FirstRowIndex) rowIndex = FirstRowIndex;
            if (rowIndex > LastRowIndex) rowIndex = LastRowIndex;

            // Get the new focus row
            //
            Row focusRow = GetRow(rowIndex);
            bool focusChanged = (focusRow != null && focusRow != FocusRow);
            if (focusChanged) 
            {
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
                CompleteEdit();
            }
            return focusChanged;
        }

        /// <summary>
        /// Is the given row selected - this checks both the SelectedRows list and the DragSelection
        /// list to determine the current selection state of the row.
        /// </summary>
        /// <param name="row">The row to determine the selection state of</param>
        /// <returns>Returns true if the row is selected</returns>
        protected internal bool Selected(Row row)
        {
            bool selected = SelectedRows.Contains(row);
            if (_dragSelection != null && _anchorRow != null)
            {
                bool inDragSelection = _dragSelection.Contains(row);
                if (SelectedRows.Contains(_anchorRow))
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
        internal protected virtual void SelectRow(Row row)
        {
            if (AllowSelectionChange(row, row, SelectionChange.ClearAndAdd))
            {
                AnchorRow = row;
                SelectedRows.Set(row);
            }
        }

        /// <summary>
        /// Extends the current selection by adding the given row to the selection set.
        /// If the row is already selected then it removed from the selection set.
        /// </summary>
        /// <param name="row">The row to add to the selection</param>
        internal protected virtual void ExtendSelectRow(Row row)
        {
            if (AllowMultiSelect)
            {
                if (SelectedRows.Contains(row))
                {
                    if (AllowSelectionChange(row, row, SelectionChange.Remove))
                    {
                        AnchorRow = row;
                        SelectedRows.Remove(row);
                    }
                }
                else
                {
                    if (AllowSelectionChange(row, row, SelectionChange.Add))
                    {
                        AnchorRow = row;
                        SelectedRows.Add(row);
                    }
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
        internal protected virtual void SelectRowRange(Row toRow)
        {
            if (AnchorRow == null || !AllowMultiSelect)
            {
                SelectRow(toRow);
            }
            else
            {
                if (AllowSelectionChange(AnchorRow, toRow, SelectionChange.ClearAndAdd))
                {
                    SelectedRows.Set(GetRows(AnchorRow, toRow));
                }
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
        internal protected virtual void ExtendSelectRowRange(Row toRow)
        {
            if (AnchorRow == null || !AllowMultiSelect)
            {
                SelectRow(toRow);
            }
            else
            {
                if (SelectedRows.Contains(AnchorRow))
                {
                    if (AllowSelectionChange(AnchorRow, toRow, SelectionChange.Add))
                    {
                        SelectedRows.Add(GetRows(AnchorRow, toRow));
                    }
                }
                else
                {
                    if (AllowSelectionChange(AnchorRow, toRow, SelectionChange.Remove))
                    {
                        SelectedRows.Remove(GetRows(AnchorRow, toRow));
                    }
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
        protected virtual void KeyboardExtendSelectRowRange(Row toRow)
        {
            if (AnchorRow == null || !AllowMultiSelect)
            {
                SelectRow(toRow);
            }
            else
            {
                if (AllowSelectionChange(AnchorRow, toRow, SelectionChange.Add))
                {
                    SelectedRows.Add(GetRows(AnchorRow, toRow));
                }
            }
        }

        /// <summary>
        /// Handle the user dragging selection over the given row.
        /// </summary>
        /// <param name="row">The row that selection is being dragged over</param>
        internal protected virtual void DragSelectRow(Row row)
        {
            if (AllowMultiSelect && AnchorRow != null)
            {
                // if there isn't already a drag selection in progress
                //
                if (_dragSelection == null)
                {
                    // if this row isn't the selection anchor then start a new drag selection
                    //
                    if (row != AnchorRow)
                    {
                        _dragSelection = GetRows(AnchorRow, row);
                        Invalidate();
                    }
                }
                else
                {
                    // if row which the mouse is over has changed since the last time the
                    // drag Selection list was updated then update the drag selection list.
                    //
                    if (row != _dragSelection[_dragSelection.Count - 1])
                    {
                        _dragSelection = GetRows(AnchorRow, row);
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
        /// Completes a drag selection by adding/removing the drag selected rows to the SelectedRows list
        /// </summary>
        internal protected virtual void CompleteDragSelection()
        {
            if (_dragSelection != null && _dragSelection.Count > 0)
            {
                Row fromRow = (Row)_dragSelection[0];
                Row toRow = (Row)_dragSelection[_dragSelection.Count - 1];
                if (SelectedRows.Contains(AnchorRow))
                {
                    if (AllowSelectionChange(fromRow, toRow, SelectionChange.Add))
                    {
                        SelectedRows.Add(_dragSelection);
                    }
                }
                else
                {
                    if (AllowSelectionChange(fromRow, toRow, SelectionChange.Add))
                    {
                        SelectedRows.Remove(_dragSelection);
                    }
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
                    if (AllowSelectionChange(null, null, SelectionChange.Clear))
                    {
                        AnchorRow = null;
                        FocusRow = null;
                        SelectedRows.Clear();
                    }
                }
            }
        }

        /// <summary>
        /// Invalidates the control to update the focus indicator
        /// </summary>
        /// <param name="e"></param>
        protected override void OnEnter(EventArgs e)
        {
            base.OnEnter(e);
            Invalidate();

            // Attach to the parent (and grandparent) controls Enter/Leave events.
            // We have to do this because it is the only way to be notified of when ContainsFocus
            // changes (and so we need to repaint the focus indicator).
            //
            EventHandler parentLeaveHandler = new EventHandler(OnParentLeave);
            EventHandler parentEnterHandler = new EventHandler(OnParentEnter);
            for (Control parent = this.Parent; parent != null; parent = parent.Parent) 
            { 
                // note that for forms the Leave/Enter events are suppressed and Deactivate/Activated are
                // used instead
                //
                if (parent is Form)
                {
                    ((Form)parent).Activated += parentEnterHandler;
                    ((Form)parent).Deactivate += parentLeaveHandler;
                }
                else
                {
                    parent.Leave += parentLeaveHandler; 
                    parent.Enter += parentEnterHandler; 
                }
            } 
        }

        /// <summary>
        /// Invalidates the control to update the focus indicator
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLeave(EventArgs e)
        {
            base.OnLeave (e);
            CompleteEdit();
            Invalidate();

            // Detach from the parent (and grandparent) controls Enter/Leave events.
            //
            EventHandler parentLeaveHandler = new EventHandler(OnParentLeave);
            EventHandler parentEnterHandler = new EventHandler(OnParentEnter);
            for (Control parent = this.Parent; parent != null; parent = parent.Parent) 
            { 
                if (parent is Form)
                {
                    ((Form)parent).Activated -= parentEnterHandler;
                    ((Form)parent).Deactivate -= parentLeaveHandler;
                }
                else
                {
                    parent.Leave -= parentLeaveHandler; 
                    parent.Enter -= parentEnterHandler; 
                }
            } 
        }

        /// <summary>
        /// Handle focus leaving main control
        /// </summary>
        /// <remarks>
        /// Calls Invalidate to update the control focus indicator.  Handles case where
        /// focus leaves main control to enter an editor control.
        /// </remarks>
        /// <param name="e"></param>
        protected override void OnLostFocus(EventArgs e)
        {
            base.OnLostFocus (e);
            Invalidate();
        }


        /// <summary>
        /// Handle focus leaving the parent control.  
        /// </summary>
        /// <remarks>
        /// Calls Invalidate to update the control focus indicator
        /// </remarks>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnParentLeave(object sender, EventArgs e)
        {
            Invalidate();
        }

        /// <summary>
        /// Handle parent control gaining focus
        /// </summary>
        /// <remarks>
        /// Calls Invalidate to update the control focus indicator
        /// </remarks>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnParentEnter(object sender, EventArgs e)
        {
            Invalidate();
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

        #region Row Management

        /// <summary>
        /// Create a new Row object for the given item
        /// </summary>
        /// <remarks>
        /// This method can be overriden to create custom Row objects (using a derived class)
        /// </remarks>
        /// <returns>A new Row object</returns>
        internal protected virtual Row CreateRow(object item, Row parentRow, int childIndex, int rowIndex)
        {
            return new Row(this, item, parentRow, childIndex, rowIndex);
        }

        /// <summary>
        /// Populates the given row table with row entries from the tree that fall within the given 
        /// absolute start/end row indices.
        /// </summary>
        /// <remarks>Handles case where startIndex > endIndex by swapping arguments</remarks>
        /// <param name="startIndex">Specifies the starting absolute row index</param>
        /// <param name="endIndex">Specifies the ending absolute row index</param>
        /// <param name="rows">Collection of rows indexed by absolute row index updated by this method</param>
        protected void GetRows(int startIndex, int endIndex, Hashtable rows)
        {
            if (_rootRow != null)
            {
                _rootRow.GetRows(startIndex, endIndex, rows);
            }
        }

        /// <summary>
        /// Updates the columns InContext property
        /// </summary>
        protected virtual void UpdateColumnsContext()
        {
            foreach (Column column in Columns)
            {
                if (column.ContextSensitive)
                {
                    column.InContext = ColumnInContext(column);
                }
                else
                {
                    column.InContext = true;
                }
            }
        }

        /// <summary>
        /// Returns the maximum number of rows that can be displayed given the current
        /// <see cref="MinRowHeight"/> for the tree.
        /// </summary>
        /// <remarks>
        /// This property returns the maximum number of rows that could be displayed if each
        /// row was the minimum height.  <see cref="NumVisibleRows"/> returns the actual number
        /// of rows currently displayed.
        /// </remarks>
        internal protected int MaxVisibleRows
        {
            get
            {
                return ((Height - DisplayTop) / MinRowHeight);
            }
        }

        /// <summary>
        /// Updates the _visibleRows to hold the currently visible rows 
        /// </summary>
        private void UpdateVisibleRows()
        {

            _visibleRows.Clear();

            // Check if the root row has been created
            //
            if (_rootRow != null)
            {           
                int topIndex = TopRowIndex;
                int bottomIndex = Math.Min(topIndex + MaxVisibleRows, LastRowIndex);
                GetRows(topIndex, bottomIndex, _visibleRows);
            }              
        }

        /// <summary>
        /// Return the visible <see cref="Row"/> at the given absolute row index
        /// </summary>
        /// <param name="index">The absolute row index of the row to be returned</param>
        /// <returns>The row object at the given index</returns>
        /// <remarks>
        /// This method only returns rows if they are currently visible.  
        /// </remarks>
        internal protected Row GetVisibleRow(int index)
        {
            return (Row)_visibleRows[index];
        }

        /// <summary>
        /// Create a new RowData object for the Tree. 
        /// </summary>
        /// <returns>A new RowData object</returns>
        /// <remarks>
        /// This method can be overriden to create custom RowData objects (using a derived class)
        /// </remarks>
        protected virtual RowData CreateRowData()
        {
            return new RowData(this);
        }

        /// <summary>
        /// Return the <see cref="RowData"/> for the given row
        /// </summary>
        /// <param name="row">The row to get the RowData for</param>
        /// <returns>The RowData for the row</returns>
        /// <remarks>
        /// Attempts to get the data from cache otherwise calls <see cref="OnGetRowData"/> to
        /// load the data 
        /// </remarks>
        internal protected virtual RowData GetDataForRow(Row row)
        {
            RowData rowData = (RowData)_cachedRowData[row];
            if (rowData == null)
            {
                rowData = CreateRowData();
                OnGetRowData(row, rowData);
                _cachedRowData[row] = rowData;
            }
            return rowData;
        }

        /// <summary>
        /// Update the displayed widgets following a change to the displayed columns or rows 
        /// </summary>
        internal protected virtual void UpdateWidgets()
        {
            // force the row data to be requeried
            //
            ClearCachedRowData();
            PinnedPanel.Columns = this.Columns.GetVisiblePinnedColumns();
            ScrollablePanel.Columns = this.Columns.GetVisibleScrollableColumns();
            UpdateVisibleRows();
            PinnedPanel.ShowRowHeaders = this.ShowRowHeaders;
            PinnedPanel.UpdateWidgets();
            ScrollablePanel.UpdateWidgets();
        }

        /// <summary>
        /// Notifies the tree that a row has been disposed.  This enables the tree to remove any references
        /// to the row eg in selection
        /// </summary>
        /// <param name="row"></param>
        internal protected virtual void RowDisposed(Row row)
        {
            if (!IsDisposed)
            {
                SelectedRows.Remove(row);
                if (row == _topRow)
                {
                    TopRow = null;
                }
                if (row == _focusRow)
                {
                    FocusRow = null;
                }
                if (row == _anchorRow)
                {
                    AnchorRow = null;
                }
                if (row == EditRow)
                {
                    AbandonEdit();
                }
            }
        }

        /// <summary>
        /// Recalculates the row indices of all rows in the tree.
        /// </summary>
        protected void ReindexRows()
        {
            if (_rootRow != null)
            {
                _rootRow.Reindex(0);   
            }
        }

        /// <summary>
        /// Handle an error in the way the parent-child relationships have been defined for items in the tree.
        /// </summary>
        /// <param name="error">A description of the nature of the error</param>
        /// <param name="path">The tree path to the row</param>
        protected virtual void HandleParentingError(string error, IList path)
        {            
            string pathText = null;
            int count = 0;
            foreach (object item in path)
            {
                if (pathText == null)
                    pathText = item.ToString();
                else
                    pathText = string.Format("{0}, {1}", pathText, item.ToString());
                if (count++ > 5)
                {
                    pathText += "...";
                    break;
                }
            }
            DisplayErrorMessage(string.Format(Resources.ParentingErrorMsg, error, pathText));
        }


        #endregion

        #region Scrolling

        /// <summary>
        /// Calculates the number of visible rows and sets the vertical scrolling parameters.
        /// </summary>
        /// <returns>Returns true if the visibility of the vertical scrollbar is changed.</returns>
        private bool UpdateVertScrollBar()
        {

            bool showScroll = false;
            int scrollPosition = TopRowIndex;
            
            VertScrollBar.Minimum = FirstRowIndex;
            VertScrollBar.Maximum = LastRowIndex;
            
            if (scrollPosition > VertScrollBar.Maximum) scrollPosition = VertScrollBar.Maximum;
            if (scrollPosition < VertScrollBar.Minimum) scrollPosition = VertScrollBar.Minimum;
            VertScrollBar.Value = scrollPosition;
 
            if (_numVisibleRows > 0 && 
                (TopRowIndex > VertScrollBar.Minimum || 
                 BottomRowIndex < VertScrollBar.Maximum))
            {
                VertScrollBar.LargeChange = _numVisibleRows;
                showScroll = true;
            }
            else
            {
                VertScrollBar.LargeChange = VertScrollBar.Maximum;
            }

            // determine whether the visibility of the scroll bar has changed
            //
            bool visibilityChanged = (showScroll != ShowVertScroll);
            ShowVertScroll = showScroll;
            return visibilityChanged;
        }

        /// <summary>
        /// Calculates the horizontal scrolling parameters.
        /// </summary>
        /// <returns>Returns true if the visibility of the horizontal scrollbar is changed.</returns>
        private bool UpdateHorzScrollBar()
        {
            int displayWidth = Math.Max(ScrollableDisplayWidth, 1);
            int totalWidth = ScrollablePanel.TotalColumnWidth;
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
        /// Return the total minimum widths of all scrollable columns
        /// </summary>
        internal protected int MinScrollableWidth
        {
            get { return _minScrollableWidth; }
        }

        /// <summary>
        /// Update the horizontal and vertical scroll bar positions
        /// </summary>
        protected virtual void UpdateScrollBars()
        {
            if (_rootRow == null) 
            {
                VertScrollBarPanel.Visible = false;
                UpdateHorzScrollBar();
            }
            else
            {
                // handle the interaction between the horizontal and vertical scrollbar visibilty
                // by recalculating the parameters if the visibility is changed
                //
                if (UpdateVertScrollBar())
                {
                    if (AutoFitColumns)
                        UpdateColumnWidths();
                }
                if (UpdateHorzScrollBar())
                {
                    // if the visibility of the horz scroll changed then we need to recalculate
                    // the vertical scrolling parameters
                    //
                    UpdateNumVisibleRows();
                    if (UpdateVertScrollBar())
                    {
                        if (AutoFitColumns)
                        {
                            UpdateColumnWidths();
                        }

                        // if the visibility of the vert scroll changed then we need to 
                        // recalculate the horizontal parameters
                        //
                        UpdateHorzScrollBar();
                    }
                }
            }
        }

        /// <summary>
        /// Adjust the top index to ensure that the maximum number of rows are displayed
        /// </summary>
        /// <remarks>
        /// If the last row is currently displayed but there is a sufficient space below it
        /// to display an extra row above then change the TopRow.  This method is called
        /// from within OnLayout so changing the TopRow will not result in a new Layout event.
        /// Instead this method calls OnLayout programmatically if required.
        /// </remarks>
        protected virtual void AutoAdjustTopRow()
        {
            if (TopRowIndex > FirstRowIndex)
            {
                // check first that the last row hasn't been scrolled off the screen
                //
                if (TopRowIndex > LastRowIndex)
                {
                    TopRow = BottomRow;
                    OnLayout(new LayoutEventArgs(this, null));
                }
                else if (BottomRowIndex == LastRowIndex)
                {
                    // otherwise check for a gap at the bottom
                    //
                    RowWidget bottomWidget = ScrollablePanel.GetRowWidget(BottomRow);
                    if (bottomWidget != null)
                    {
                        int gap = DisplayTop + DisplayHeight - bottomWidget.Bounds.Bottom;

                        // Check if there is any possibility of fitting another row in the gap
                        //
                        if (gap > this.MinRowHeight)
                        {
                            // now do a more precise check on whether the new top row would fit
                            //
                            int newTopIndex = TopRowIndex - 1;
                            Row newTopRow = GetRow(newTopIndex);
                            int rowHeight = GetUserRowHeight(newTopRow);
                            if (rowHeight == 0)
                            {
                                RowData rowData = GetDataForRow(newTopRow);
                                if (rowData.AutoFitHeight)
                                {
                                    rowData.Height = GetOptimalRowHeight(newTopRow);
                                }
                                rowHeight = rowData.Height;
                            }
                            if (gap > rowHeight)
                            {
                                TopRow = newTopRow;
                                OnLayout(new LayoutEventArgs(this, null));
                            }
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Handle a vertical scrolling event.  Updates the index of the top row displayed by
        /// the tree and calls PerformLayout to display the new range of rows.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnVerticalScroll(object sender, ScrollEventArgs e)
        {
            Focus();
            switch (e.Type)
            {
                case ScrollEventType.First:
                case ScrollEventType.Last:
                case ScrollEventType.SmallDecrement:
                case ScrollEventType.SmallIncrement:
                case ScrollEventType.LargeDecrement:
                case ScrollEventType.LargeIncrement:
                    // update the top row 
                    //
                    TopRowIndex = e.NewValue;
                    Update();  // force paint event to be processed for smooth scrolling
                    break;
                case ScrollEventType.ThumbTrack:
                    
                    if (_trackVertScroll)
                    {
                        // update the top row 
                        //
                        TopRowIndex = e.NewValue;
                        Update();  // force paint event to be processed for smooth scrolling
                    }
                    break;
                case ScrollEventType.EndScroll:
                    if (TopRowIndex != e.NewValue)
                    {
                        TopRowIndex = e.NewValue;
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
            Focus();
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
                TopRowIndex -= WheelDelta;
            }
            else
            {
                int index = BottomRowIndex + WheelDelta;
                if (index > LastRowIndex) index = LastRowIndex;
                BottomRowIndex = index;
            }

            // force the paint events to be processed immediately
            //
            Update();
        }

        #endregion

        #region Context Menus
       

        /// <summary>
        /// Add the component to the container ensuring that the name is unique
        /// </summary>
        /// <param name="component">The component to add</param>
        /// <param name="name">The base name of the component</param>
        protected void AddToContainer(Component component, string name)
        {
            string uniqueName = name;
            int index = 1;
            while (Container.Components[uniqueName] != null)
            {
                uniqueName = name + index++;
            }
            Container.Add(component, uniqueName);
        }

        /// <summary>
        /// Set/Get the Sort Ascending menu item
        /// </summary>
        protected ToolStripMenuItem SortAscendingMenuItem
        {
            get { return _sortAscendingMenuItem; }
            set { _sortAscendingMenuItem = value; }
        }

        /// <summary>
        /// Set/Get the Sort Descending menu item
        /// </summary>
        protected ToolStripMenuItem SortDescendingMenuItem
        {
            get { return _sortDescendingMenuItem; }
            set { _sortDescendingMenuItem = value; }
        }

        /// <summary>
        /// Set/Get the Best Fit menu item
        /// </summary>
        protected ToolStripMenuItem BestFitMenuItem
        {
            get { return _bestFitMenuItem; }
            set { _bestFitMenuItem = value; }
        }

        /// <summary>
        /// Set/Get the Best Fit menu item
        /// </summary>
        protected ToolStripMenuItem BestFitAllMenuItem
        {
            get { return _bestFitAllMenuItem; }
            set { _bestFitAllMenuItem = value; }
        }

        /// <summary>
        /// Set/Get the Auto Fit menu item
        /// </summary>
        protected ToolStripMenuItem AutoFitMenuItem
        {
            get { return _autoFitMenuItem; }
            set { _autoFitMenuItem = value; }
        }

        /// <summary>
        /// Set/Get the Pinned menu item
        /// </summary>
        protected ToolStripMenuItem PinnedMenuItem
        {
            get { return _pinnedMenuItem; }
            set { _pinnedMenuItem = value; }
        }

        /// <summary>
        /// Set/Get the Customize menu item
        /// </summary>
        protected ToolStripMenuItem CustomizeMenuItem
        {
            get { return _customizeMenuItem; }
            set { _customizeMenuItem = value; }
        }

        /// <summary>
        /// Set/Get the Show/Hide Columns menu item
        /// </summary>
        protected ToolStripMenuItem ShowColumnsMenuItem
        {
            get { return _showColumnsMenuItem; }
            set { _showColumnsMenuItem = value; }
        }

        /// <summary>
        /// Add the menu item with the given name to the menu (an optionally the component container)
        /// </summary>
        /// <param name="menu">The menu to add to</param>
        /// <param name="name">The name of the item (the menu item tag)</param>
        /// <param name="text">The text of the menu item</param>
        /// <param name="iconName">The name of the icon resource</param>
        /// <param name="addToContainer">Should the item be added to the container</param>
        /// <returns>The new menu item</returns>
        private ToolStripMenuItem AddMenuItem(ContextMenuStrip menu, 
                                              string name, string text, string iconName,
                                              bool addToContainer)
        {
            Image image = null;
            if (iconName != null)
            {
                Icon icon = Resources.ResourceManager.GetObject(iconName) as Icon;
                image = icon.ToBitmap();
            }
            ToolStripMenuItem item = new ToolStripMenuItem(text, image);
            item.Tag = name;
            menu.Items.Add(item);
            if (addToContainer)
            {
                AddToContainer(item, name);
            }
            return item;
        }

        /// <summary>
        /// Add a separator item to the menu
        /// </summary>
        /// <param name="menu">The menu to add to</param>
        /// <param name="name">The name of the separator</param>
        /// <param name="addToContainer">Should the item be added to the container</param>
        private void AddSeparatorMenuItem(ContextMenuStrip menu, string name, bool addToContainer)
        {
            ToolStripItem item = new ToolStripSeparator();
            menu.Items.Add(item);
            if (addToContainer)
            {
                AddToContainer(item, name);
            }
        }

        /// <summary>
        /// Create the header context menus for this tree
        /// </summary>
        /// <param name="addToContainer">If true the menu components are added to the code container</param>
        public virtual ContextMenuStrip CreateHeaderContextMenu(bool addToContainer)
        {
            ContextMenuStrip menu = new ContextMenuStrip();

            if (addToContainer)
            {
                AddToContainer(menu, "headerContextMenu");
            }
            AddMenuItem(menu, "sortAscendingMenuItem", Resources.SortAscendingMenuText, "SortAscendingMenuIcon", addToContainer);
            AddMenuItem(menu, "sortDescendingMenuItem", Resources.SortDescendingMenuText, "SortDescendingMenuIcon", addToContainer);
            AddSeparatorMenuItem(menu, "separator1MenuItem", addToContainer);
            AddMenuItem(menu, "bestFitMenuItem", Resources.BestFitMenuText, "BestFitMenuIcon", addToContainer);
            AddMenuItem(menu, "bestFitAllMenuItem", Resources.BestFitAllMenuText, "BestFitAllMenuIcon", addToContainer);
            AddMenuItem(menu, "autoFitMenuItem", Resources.AutoFitMenuText, "AutoFitMenuIcon", addToContainer);
            AddSeparatorMenuItem(menu, "separator2MenuItem", addToContainer);
            if (AllowUserPinnedColumns)
            {
                AddMenuItem(menu, "pinnedMenuItem", Resources.PinnedMenuText, "PinnedMenuIcon", addToContainer);
                AddSeparatorMenuItem(menu, "separator3MenuItem", addToContainer);
            }
            AddMenuItem(menu, "showColumnsMenuItem", Resources.ShowColumnsText, null, addToContainer);
            AddMenuItem(menu, "customizeMenuItem", Resources.CustomizeColumnsMenuText, "CustomizeColumnsMenuIcon", addToContainer);
            return menu;
        }

        /// <summary>
        /// Find the header menu tem with the given name
        /// </summary>
        /// <param name="name">The string tag that identifies the menu item</param>
        /// <returns>The menu item</returns>
        protected virtual ToolStripMenuItem FindHeaderMenuItem(string name)
        {
            foreach (ToolStripItem item in HeaderContextMenu.Items)
            {
                if ((item.Tag as string) == name && item is ToolStripMenuItem)
                {
                    return item as ToolStripMenuItem;
                }
            }
            return null;
        }

        /// <summary>
        /// Locate the default menu items in a custom header menu and hook up the
        /// event handlers
        /// </summary>
        protected virtual void HookHeaderContextMenuItems()
        {
            // unhook first from the previous context menu item if any
            //
            if (SortAscendingMenuItem != null)
                SortAscendingMenuItem.Click -= new EventHandler(OnSortAscendingMenuClicked);
            SortAscendingMenuItem = FindHeaderMenuItem("sortAscendingMenuItem");
            if (SortAscendingMenuItem != null)
                SortAscendingMenuItem.Click += new EventHandler(OnSortAscendingMenuClicked);
            
            if (SortDescendingMenuItem != null)
                SortDescendingMenuItem.Click -= new EventHandler(OnSortDescendingMenuClicked);
            SortDescendingMenuItem = FindHeaderMenuItem("sortDescendingMenuItem"); 
            if (SortDescendingMenuItem != null)
                SortDescendingMenuItem.Click += new EventHandler(OnSortDescendingMenuClicked);

            if (BestFitMenuItem != null)
                BestFitMenuItem.Click -= new EventHandler(OnBestFitMenuClicked);
            BestFitMenuItem = FindHeaderMenuItem("bestFitMenuItem");
            if (BestFitMenuItem != null)
                BestFitMenuItem.Click += new EventHandler(OnBestFitMenuClicked);

            if (BestFitAllMenuItem != null)
                BestFitAllMenuItem.Click -= new EventHandler(OnBestFitAllMenuClicked);
            BestFitAllMenuItem = FindHeaderMenuItem("bestFitAllMenuItem");
            if (BestFitAllMenuItem != null)
                BestFitAllMenuItem.Click += new EventHandler(OnBestFitAllMenuClicked);

            if (AutoFitMenuItem != null)
                AutoFitMenuItem.Click -= new EventHandler(OnAutoFitMenuClicked);
            AutoFitMenuItem = FindHeaderMenuItem("autoFitMenuItem");
            if (AutoFitMenuItem != null)
                AutoFitMenuItem.Click += new EventHandler(OnAutoFitMenuClicked);

            if (PinnedMenuItem != null)
                PinnedMenuItem.Click -= new EventHandler(OnPinnedMenuClicked);
            PinnedMenuItem = FindHeaderMenuItem("pinnedMenuItem");
            if (PinnedMenuItem != null)
                PinnedMenuItem.Click += new EventHandler(OnPinnedMenuClicked);

            if (ShowColumnsMenuItem != null)
            {
                ShowColumnsMenuItem.DropDownOpened -= new EventHandler(OnShowColumnsDropDownOpened);
                ShowColumnsMenuItem.DropDownItemClicked -= new ToolStripItemClickedEventHandler(OnShowColumnsDropDownClicked);
                ShowColumnsMenuItem.DropDown.Closing -= new ToolStripDropDownClosingEventHandler(OnShowColumnsDropDownClosing);
            }
            ShowColumnsMenuItem = FindHeaderMenuItem("showColumnsMenuItem");
            if (ShowColumnsMenuItem != null)
            {
                ShowColumnsMenuItem.DropDownOpened += new EventHandler(OnShowColumnsDropDownOpened);
                ShowColumnsMenuItem.DropDownItemClicked += new ToolStripItemClickedEventHandler(OnShowColumnsDropDownClicked);
                ShowColumnsMenuItem.DropDown.Closing += new ToolStripDropDownClosingEventHandler(OnShowColumnsDropDownClosing);
                ShowColumnsMenuItem.DropDownItems.Add("");
            }

            if (CustomizeMenuItem != null)
                CustomizeMenuItem.Click -= new EventHandler(OnCustomizeMenuClicked);
            CustomizeMenuItem = FindHeaderMenuItem("customizeMenuItem");
            if (CustomizeMenuItem != null)
                CustomizeMenuItem.Click += new EventHandler(OnCustomizeMenuClicked);

        }

        /// <summary>
        /// Prevent the ShowColumns drop down from closing when the user checks/unchecks an item
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnShowColumnsDropDownClosing(object sender, ToolStripDropDownClosingEventArgs e)
        {
            if (e.CloseReason == ToolStripDropDownCloseReason.ItemClicked)
            {
                e.Cancel = true;
            }
        }

        /// <summary>
        /// Handle the user selecting show/hide for a column
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnShowColumnsDropDownClicked(object sender, ToolStripItemClickedEventArgs e)
        {
            Column column = e.ClickedItem.Tag as Column;
            column.Active = !column.Active;
        }

        /// <summary>
        /// Add a menu item for the given column to the collection
        /// </summary>
        /// <param name="column"></param>
        /// <param name="items"></param>
        protected virtual void AddColumnMenuItem(Column column, ToolStripItemCollection items)
        {
            if (column.InContext && !column.PrefixColumn && column.Hidable)
            {
                ToolStripMenuItem item = new ToolStripMenuItem();
                item.Tag = column;
                item.Checked = column.Visible;
                item.CheckOnClick = true;
                if (string.IsNullOrEmpty(column.Caption))
                    item.Text = column.ToolTip;
                else
                    item.Text = column.Caption;
                ShowColumnsMenuItem.DropDownItems.Add(item);
            }
        }

        /// <summary>
        /// Add DropDownItems to the Show Columns menu item for each of the columns
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnShowColumnsDropDownOpened(object sender, EventArgs e)
        {
            ShowColumnsMenuItem.DropDown.SuspendLayout();
            try
            {
                ToolStripItemCollection items = ShowColumnsMenuItem.DropDownItems;
                items.Clear();
                
                // ensure pinned columns are displayed before unpinned columns
                //
                foreach (Column column in Columns)
                {
                    if (column.Pinned)
                    {
                        AddColumnMenuItem(column, items);
                    }
                }
                foreach (Column column in Columns)
                {
                    if (!column.Pinned)
                    {
                        AddColumnMenuItem(column, items);
                    }
                }
            }
            finally
            {
                ShowColumnsMenuItem.DropDown.ResumeLayout();
            }
        }

        /// <summary>
        /// Handle a click event for the sort ascending context menu
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        public virtual void OnSortAscendingMenuClicked(object sender, EventArgs e)
        {
            if (ContextMenuColumn != null)
            {
                SuspendLayout();
                try
                {
                    SortColumn = ContextMenuColumn;
                    SortColumn.SortDirection = ListSortDirection.Ascending;
                }
                finally
                {
                    ResumeLayout(false);
                    PerformLayout();
                }
            }
        }

        /// <summary>
        /// Handle a click event for the sort descending context menu
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnSortDescendingMenuClicked(object sender, EventArgs e)
        {
            if (ContextMenuColumn != null)
            {
                SuspendLayout();
                try
                {
                    SortColumn = ContextMenuColumn;
                    SortColumn.SortDirection = ListSortDirection.Descending;
                }
                finally
                {
                    ResumeLayout(false);
                    PerformLayout();
                }
            }
        }

        /// <summary>
        /// Handle a click event for the best fit context menu
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnBestFitMenuClicked(object sender, EventArgs e)
        {
            SetBestFitWidth(ContextMenuColumn);
        }

        /// <summary>
        /// Handle a click event for the best fit all context menu
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnBestFitAllMenuClicked(object sender, EventArgs e)
        {
            SetBestFitWidthAllColumns();
        }

        /// <summary>
        /// Handle a click event for the Auto Fit context menu
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnAutoFitMenuClicked(object sender, EventArgs e)
        {
            AutoFitColumns = !AutoFitColumns;
        }

        /// <summary>
        /// Handle a click event for the Pinned context menu
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnPinnedMenuClicked(object sender, EventArgs e)
        {
            ContextMenuColumn.Pinned = !ContextMenuColumn.Pinned;
        }

        /// <summary>
        /// Handle a click event for the customize context menu
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnCustomizeMenuClicked(object sender, EventArgs e)
        {
            ShowColumnCustomizeForm();
        }

        /// <summary>
        /// Handle a change to the <seealso cref="CultureManager.ApplicationUICulture"/>
        /// </summary>
        /// <param name="newCulture">The new culture</param>
        protected virtual void OnApplicationUICultureChanged(CultureInfo newCulture)
        {
            // if we are using the default header context menu then force it
            // to be recreated
            //
            if (_defaultHeaderContextMenu)
            {
                _headerContextMenu = null;
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
            else if (point.Y < DisplayTop)
            {
                offset = DisplayTop - point.Y;
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
        /// or last displayed rows.   The event scrolls the tree in the direction that the
        /// user is dragging to enable drag selection select elements outside the currently
        /// displayed rows.
        /// </remarks>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnDragSelectTimerTick(object sender, EventArgs e)
        {
            Point point = PointToClient(MousePosition);
            Row row = null;
            if (point.Y > HorzScrollBar.Top)
            {
                BottomRowIndex++;
                row = GetRow(BottomRowIndex);
            }
            else if (point.Y < DisplayTop)
            {
                TopRowIndex--;
                row = _topRow;
            }

            if (row == null)
            {
                StopDragSelectTimer();
            }
            else
            {
                DragSelectRow(row);
            }
        }

        /// <summary>
        /// Scroll the control horizontally depending on the location of the mouse
        /// </summary>
        /// <param name="mousePosition">The mouse position in client coords</param>
        protected virtual void HorizontalDragScroll(Point mousePosition)
        {
            int pinnedWidth = PinnedPanel.TotalColumnWidth;
            int scrollableWidth = Width - pinnedWidth;
            int scrollZone = scrollableWidth / 4;
            int offset = (RightToLeft == RightToLeft.Yes) ? -10 : 10;
            if (mousePosition.X < pinnedWidth + scrollZone) 
            {
                HorzScrollOffset -= offset; 
            } 
            else if (mousePosition.X > Right - scrollZone) 
            {
                HorzScrollOffset += offset; 
            }
        }

        /// <summary>
        /// Calls <see cref="HorizontalDragScroll"/> to handle auto scrolling the control horizontally
        /// </summary>
        /// <param name="e"></param>
        protected override void OnDragOver(DragEventArgs e)
        {
            base.OnDragOver (e);
            if (this.ShowHorzScroll)
            {
                HorizontalDragScroll(PointToClient(new Point(e.X, e.Y)));
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
        /// Start initialization of the virtual tree.
        /// </summary>
        public void BeginInit()
        {
            SuspendLayout();
            _initializing = true;
        }

        /// <summary>
        /// End initialization of the virtual tree
        /// </summary>
        public void EndInit()
        {
            _initializing = false;
            BindDataSource();
            ResumeLayout(false);
            PerformLayout();
            if (_headerContextMenu != null)
            {
                HookHeaderContextMenuItems();
            }
        }

 

        #endregion



    }
}
