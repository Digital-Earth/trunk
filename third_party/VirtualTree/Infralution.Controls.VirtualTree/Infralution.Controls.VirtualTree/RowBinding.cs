#region File Header
//
//      FILE:   RowBinding.cs.
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
using System.ComponentModel;
using System.Windows.Forms;
using NS = Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a base class for binding information displayed in a <see cref="Row"/> of a 
    /// <see cref="VirtualTree"/> and relationships between Rows to the <see cref="VirtualTree.DataSource"/>. 
    /// </summary>
    /// <remarks>
    /// <para>
    /// RowBindings are usually created using the <see cref="VirtualTree"/> visual designer.  The user
    /// uses the visual designer to add RowBindings to the <see cref="VirtualTree.RowBindings"/> property 
    /// of the tree.  Typically a RowBinding is added for each type of item to be displayed by the <see cref="VirtualTree"/>.
    /// The <see cref="VirtualTree"/> determines the RowBinding that should be used to display a given item by
    /// calling the <see cref="BindsTo(object)"/> method of each RowBinding in calls the <see cref="VirtualTree.RowBindings"/> 
    /// list.    The <see cref="VirtualTree"/> then calls the <see cref="GetRowData"/>, <see cref="GetCellData"/> and
    /// <see cref="GetChildrenForRow"/> methods on the binding to obtain the information necessary to display the <see cref="Row"/>.
    /// </para>
    /// <para>
    /// Derived classes are typically defined which map <see cref="RowData"/> properties to attributes of the
    /// <see cref="VirtualTree.DataSource"/> for a particular type of DataSource.
    /// This base class provides a mechanism to allow the user to set the <see cref="RowData"/> properties 
    /// that are not typically data driven.  For instance the <see cref="Icon"/> and <see cref="Style"/> of 
    /// for the binding can be set using the <see cref="VirtualTree"/> designer.
    /// </para>
    /// </remarks>
    /// <seealso href="XtraDatasetBinding.html">Data Binding to ADO.NET Datasets and DataViews</seealso>
    /// <seealso href="XtraObjectBinding.html">Data Binding to Object Properties</seealso>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="Row"/>
    /// <seealso cref="NS.CellBinding"/>
    [DesignTimeVisible(false)]
    [ToolboxItem(false)]
    public abstract class RowBinding : Component
    {
        #region Member Variables

        /// <summary>
        /// List of registered row bindings types.
        /// </summary>
        private static ArrayList _registeredRowBindings = new ArrayList();

        /// <summary>
        /// Lookup table for human readable names for rowbinding types
        /// </summary>
        private static Hashtable _registeredRowBindingName = new Hashtable();

        /// <summary>
        /// The tree this binding belongs to
        /// </summary>
        private VirtualTree _tree;     
 
        /// <summary>
        /// The name of this binding - when sited matches that of the site
        /// </summary>
        private string _name;

        /// <summary>
        /// Policy defining how child rows are handled
        /// </summary>
        private RowChildPolicy  _childPolicy = RowChildPolicy.Normal; 
       
        /// <summary>
        /// The icon to display for unexpanded rows
        /// </summary>
        private Icon            _icon = null;

        /// <summary>
        /// The icon to display for expanded rows (if different to unexpanded)
        /// </summary>
        private Icon            _expandedIcon = null;

        /// <summary>
        /// Size of the icon to use
        /// </summary>
        private int             _iconSize = 16;

        /// <summary>
        /// The height for this row - if zero the standard row height for the tree is used
        /// </summary>
        private int             _height = 0;

        /// <summary>
        /// Should the height of this row be automatically calculated by virtual tree
        /// </summary>
        private bool            _autoFitHeight  = false;

        /// <summary>
        /// Can the user resize this row
        /// </summary>
        private bool            _resizable  = true;

        /// <summary>
        /// Should the prefix column be shown for this row
        /// </summary>
        private bool            _showPrefixColumn = true;

        /// <summary>
        /// The context menu to display for this row
        /// </summary>
        private ContextMenuStrip _contextMenuStrip = null;

        /// <summary>
        /// Can rows of this type be dragged
        /// </summary>
        private bool _allowDrag = false;

        /// <summary>
        /// The allowed drop locations for rows
        /// </summary>
        private RowDropLocation _allowedDropLocations = RowDropLocation.None;

        /// <summary>
        /// Should this row display a single cell for the main column which spans all columns
        /// </summary>
        private bool _spanningRow = false;

        /// <summary>
        /// The set of bindings to columns for this row
        /// </summary>
        private CellBindingList _cellBindings;

        /// <summary>
        /// The style for this row
        /// </summary>
        private Style _style = new Style();

        /// <summary>
        /// The style for odd rows
        /// </summary>
        private Style _oddStyle = new Style();

        /// <summary>
        /// The style for even rows
        /// </summary>
        private Style _evenStyle = new Style();

        /// <summary>
        /// The style for selected rows
        /// </summary>
        private Style _selectedStyle = new Style();

        /// <summary>
        /// The style to use when printing this type of row
        /// </summary>
        Style _printStyle = new Style();

        /// <summary>
        /// The style to use when printing even rows 
        /// </summary>
        Style _printEvenStyle = new Style();

        /// <summary>
        /// The style to use when printing odd rows 
        /// </summary>
        Style _printOddStyle = new Style();

        #endregion

        #region Public Events

        /// <summary>
        /// Raised by the binding to obtain the children of the given row.  
        /// </summary>
        /// <remarks>
        /// Handle this event if you want to programmatically set the children for items 
        /// using this binding.
        /// </remarks>
        /// <seealso href="XtraProgrammaticBinding.html">Programmatic Data Binding</seealso>
        [Category("Data"),
        Description("Fired when the tree needs to find the child items for a row")]
        public event GetChildrenHandler GetChildren;

        /// <summary>
        /// Raised by the binding to locate the parent of a given item.   
        /// </summary>
        /// <remarks>
        /// Handle this event if you want to programmatically set the parent for items of 
        /// using this binding.
        /// </remarks>
        /// <seealso href="XtraProgrammaticBinding.html">Programmatic Data Binding</seealso>
        [Category("Data"),
        Description("Fired when the tree needs to find the parent item for a given item in the tree")]
        public event GetParentHandler GetParent;

        #endregion

        #region Public Properties

        /// <summary>
        /// Return the <see cref="VirtualTree"/> this binding belongs to.
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public VirtualTree Tree
        {
            get { return _tree; }
        }

        /// <summary>
        /// The name used to identify this binding. 
        /// </summary>
        /// <remarks>
        /// When the binding is sited at design time the name matches that of the site
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
        /// Defines how the <see cref="VirtualTree"/> should handle loading children for rows of this type.
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(RowChildPolicy.Normal)]
        [Description("Defines how the tree should handle loading children for rows of this type")]   
        public virtual RowChildPolicy ChildPolicy
        {
            get { return _childPolicy; }
            set { _childPolicy = value; }
        }

        /// <summary>
        /// Set/Get the icon to display for rows of this type.
        /// </summary>
        [Category("Appearance")]
        [DefaultValue(null)]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Description("The icon to display for rows of this type")]
        [Localizable(true)]
        public virtual Icon Icon
        {
            get { return _icon; }
            set { _icon = value; }
        }

        /// <summary>
        /// Set/Get the icon to display for rows of this type when the row is expanded.
        /// </summary>
        /// <remarks>If the expanded icon is null then the normal icon is used</remarks>
        [Category("Appearance")] 
        [DefaultValue(null)]
        [Description("The icon to display for expanded rows of this type")]
        [Localizable(true)]
        public virtual Icon ExpandedIcon
        {
            get { return (_expandedIcon == null) ? _icon : _expandedIcon; }
            set { _expandedIcon = value; }
        }

        /// <summary>
        /// Set/Get the size of the icon to display.
        /// </summary>
        /// <remarks>
        /// This lets you select the size of the icon to display where an icon contains
        /// multiple image sizes.
        /// </remarks>
        [Category("Appearance")]
        [DefaultValue(16)]
        [Description("The size of the icon to display")]
        [Localizable(true)]
        public virtual int IconSize
        {
            get { return _iconSize; }
            set 
            {
                if (value <= 0) throw new ArgumentOutOfRangeException("IconSize", "Value must be non-zero");
                _iconSize = value; 
            }
        }

        /// <summary>
        /// Set/Get the height for this row.
        /// </summary>
        /// <remarks>If set to zero than the default height for the tree is used</remarks>
        [Category("Appearance")]
        [Description("The height (in pixels) for rows of this type")]
        [Localizable(true)]
        public virtual int Height
        {
            get { return (_height == 0 && _tree != null) ? _tree.RowHeight : _height; }
            set { _height = value; }
        }

        /// <summary>
        /// Called by framework to determine whether the Height should be serialised
        /// </summary>
        /// <returns>True if the height is not the default</returns>
        private bool ShouldSerializeHeight()
        {
            return (_height != 0);
        }

        /// <summary>
        /// Called by framework to reset the height to the default
        /// </summary>
        private void ResetHeight()
        {
            _height = 0;
        }

        /// <summary>
        /// Should the height for rows of this type be calculated automatically to fit the contents.
        /// </summary>
        /// <remarks>
        /// If set to true then Virtual Tree will automatically calculate the
        /// height of the row required to fit the contents of the cells currently displayed in
        /// the row
        /// </remarks>
        [Category("Appearance")]
        [DefaultValue(false)]
        [Description("Should the height for rows of this type be calculated automatically to fit the contents")]
        [Localizable(true)]
        public virtual bool AutoFitHeight
        {
            get { return _autoFitHeight; }
            set { _autoFitHeight = value; }
        }

        /// <summary>
        /// Can the user resize this type of row by dragging the row header divider
        /// </summary>
        /// <remarks>
        /// <see cref="VirtualTree.AllowRowResize"/> must also be true in order for the user
        /// to be able to resize the row.
        /// </remarks>
        /// <seealso cref="VirtualTree.AllowRowResize"/>
        /// <seealso cref="VirtualTree.AllowIndividualRowResize"/>
        [Category("Behavior")]
        [DefaultValue(true)]
        [Description("Can the user resize this type of row by dragging the row header divider")]
        public virtual bool Resizable
        {
            get { return _resizable; }
            set { _resizable = value; }
        }

        /// <summary>
        /// Should the <see cref="VirtualTree.PrefixColumn"/> (if defined) be shown for this row
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(true)]
        [Description("Should the Prefix Column (if defined) be shown for this row")]
        public bool ShowPrefixColumn
        {
            get { return _showPrefixColumn; }
            set { _showPrefixColumn = value; }
        }

        /// <summary>
        /// Set/Get the <see cref="ContextMenu"/> to display for rows of this type.
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(null)]
        [Description("The context menu to display for rows of this type")]   
        public ContextMenuStrip ContextMenuStrip
        {
            get { return _contextMenuStrip; }
            set { _contextMenuStrip = value; }
        }

        /// <summary>
        /// Set/Get whether the user can drag and drop rows of this type.
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(false)]
        [Description("Can this type of row be dragged and dropped")]   
        public virtual bool AllowDrag
        {
            get { return _allowDrag; }
            set { _allowDrag = value; }
        }

        /// <summary>
        /// Set/Get whether the user can drop items onto rows of this type.
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(false)]
        [Description("Can items be dropped onto this type of row")]   
        public virtual bool AllowDropOnRow
        {
            get { return (_allowedDropLocations & RowDropLocation.OnRow) != 0; }
            set 
            { 
                if (value) 
                    _allowedDropLocations |= RowDropLocation.OnRow;
                else
                    _allowedDropLocations &= ~RowDropLocation.OnRow;
            }
        }

        /// <summary>
        /// Set/Get whether the user can drop items above rows of this type.
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(false)]
        [Description("Can items be dropped above rows of this type")]   
        public virtual bool AllowDropAboveRow
        {
            get { return (_allowedDropLocations & RowDropLocation.AboveRow) != 0; }
            set 
            { 
                if (value) 
                    _allowedDropLocations |= RowDropLocation.AboveRow;
                else
                    _allowedDropLocations &= ~RowDropLocation.AboveRow;
            }
        }

        /// <summary>
        /// Set/Get whether the user can drop items below rows of this type.
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(false)]
        [Description("Can items be dropped below rows of this type")]   
        public virtual bool AllowDropBelowRow
        {
            get { return (_allowedDropLocations & RowDropLocation.BelowRow) != 0; }
            set 
            { 
                if (value) 
                    _allowedDropLocations |= RowDropLocation.BelowRow;
                else
                    _allowedDropLocations &= ~RowDropLocation.BelowRow;
            }
        }

        /// <summary>
        /// Should this row display a single cell (for <see cref="VirtualTree.MainColumn"/> 
        /// thats spans all columns
        /// </summary>
        [Category("Behavior")]
        [DefaultValue(false)]
        [Description("Should this row display a single cell (for the MainColumn) that spans all columns")]
        public virtual bool SpanningRow
        {
            get { return _spanningRow; }
            set { _spanningRow = value; }
        }

        /// <summary>
        /// Get the cell bindings for this row binding
        /// </summary>
        /// <remarks>
        /// <see cref="CellBinding">CellBindings</see> may the information displayed for a given 
        /// <see cref="Column"/> of the <see cref="Row"/> to attributes items from the <see cref="VirtualTree.DataSource"/>.
        /// </remarks>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public virtual CellBindingList CellBindings
        {
            get { return _cellBindings; }
        }

        /// <summary>
        /// Called by framework to determine whether cell bindings should be code serialized
        /// </summary>
        /// <returns>True if the cell bindings aren't empty</returns>
        private bool ShouldSerializeCellBindings()
        {
            return _cellBindings.Count > 0;
        }

        /// <summary>
        /// Return the string to be displayed in the designer for this binding
        /// </summary>
        [Browsable(false)]
        public virtual string DisplayName
        {
            get { return GetType().Name; }
        }

        /// <summary>
        /// The style to draw the row with
        /// </summary>
        /// <remarks>
        /// Use this to set style properties that should be applied to both odd and
        /// even rows.   This style derives from <see cref="NS.VirtualTree.RowStyle">VirtualTree.Style</see>.  
        /// Use <see cref="NS.VirtualTree.RowStyle">VirtualTree.Style</see> to set properties which all rows in the
        /// tree. 
        /// </remarks>
        /// <seealso cref="NS.VirtualTree.RowStyle">VirtualTree.Style</seealso>
        /// <seealso cref="OddStyle"/>
        /// <seealso cref="EvenStyle"/>
        [Category("Appearance")]
        [Description("The style to draw the row with")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public virtual Style Style
        {
            get { return _style; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeStyle()
        {
            return _style.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        private void ResetStyle()
        {
            _style.Reset();
        }

        /// <summary>
        /// The style to draw odd rows with
        /// </summary>
        /// <remarks>
        /// Use this to override style properties that apply only to odd rows.   
        /// OddStyle derives from <see cref="Style"/> and <see cref="NS.VirtualTree.RowOddStyle">VirtualTree.OddStyle</see>.   
        /// Use <see cref="NS.VirtualTree.RowOddStyle">VirtualTree.OddStyle</see> to set properties which apply to all
        /// odd rows.   Use <see cref="Style"/> to set properties which apply to both odd and even rows of this type. 
        /// </remarks>
        /// <seealso cref="Style"/>
        /// <seealso cref="EvenStyle"/>
        /// <seealso cref="NS.VirtualTree.RowOddStyle">VirtualTree.OddStyle</seealso>
        [Category("Appearance")]
        [Description("The style to draw odd rows with")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public virtual Style OddStyle
        {
            get { return _oddStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeOddStyle()
        {
            return _oddStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        private void ResetOddStyle()
        {
            _oddStyle.Reset();
        }

        /// <summary>
        /// The style to draw even rows with
        /// </summary>
        /// <remarks>
        /// Use this to override style properties that apply only to even rows.   
        /// EvenStyle derives from <see cref="Style"/> and <see cref="NS.VirtualTree.RowEvenStyle">VirtualTree.EvenStyle</see>.   
        /// Use <see cref="NS.VirtualTree.RowEvenStyle">VirtualTree.EvenStyle</see> to set properties which apply to all
        /// even rows.   Use <see cref="Style"/> to set properties which apply to both odd and even rows of this type. 
        /// </remarks>
        /// <seealso cref="Style"/>
        /// <seealso cref="OddStyle"/>
        /// <seealso cref="NS.VirtualTree.RowEvenStyle">VirtualTree.EvenStyle</seealso>
        [Category("Appearance")]
        [Description("The style to draw even rows with")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public virtual Style EvenStyle
        {
            get { return _evenStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeEvenStyle()
        {
            return _evenStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        private void ResetEvenStyle()
        {
            _evenStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use when printing 
        /// </summary>
        [Category("Appearance")]
        [Description("The drawing attributes to use when printing")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style PrintStyle
        {
            get { return _printStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializePrintStyle()
        {
            return _printStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetPrintStyle()
        {
            _printStyle.Reset();
        }

 
        /// <summary>
        /// The drawing attributes to use when printing for odd rows
        /// </summary>
        [Category("Appearance")]
        [Description("The drawing attributes to use when printing odd rows")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style PrintOddStyle
        {
            get { return _printOddStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializePrintOddStyle()
        {
            return _printOddStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetPrintOddStyle()
        {
            _printOddStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use when printing even rows
        /// </summary>
        [Category("Appearance")]
        [Description("The drawing attributes to use when printing even rows")]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style PrintEvenStyle
        {
            get { return _printEvenStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializePrintEvenStyle()
        {
            return _printEvenStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetPrintEvenStyle()
        {
            _printEvenStyle.Reset();
        }

        #endregion

        #region Public Methods

        /// <summary>
        /// Initialise a default instance of this class
        /// </summary>
        public RowBinding()
        {
            _cellBindings = new CellBindingList(this);
        }

        /// <summary>
        /// Register a derived row bindings type
        /// </summary>
        /// <remarks>
        /// This method allows derived row binding types to be added using the Row Bindings editor
        /// </remarks>
        /// <param name="rowBindingType">The type to register (must be derived from RowBinding)</param>
        /// <param name="name">The name to display for this type</param>
        static public void RegisterRowBindingType(Type rowBindingType, string name)
        {
            if (!typeof(RowBinding).IsAssignableFrom(rowBindingType))
                throw new ArgumentException("rowBindingType must be derived from RowBinding", "rowBindingType");
            _registeredRowBindings.Add(rowBindingType);
            _registeredRowBindingName[rowBindingType] = name;
        }

        /// <summary>
        /// The registered row binding types.
        /// </summary>
        /// <seealso cref="RegisterRowBindingType"/>
        static public ICollection RegisteredRowBindingTypes
        {
            get { return _registeredRowBindings; }
        }

        /// <summary>
        /// Return the human readable name for a registered row binding type.
        /// </summary>
        /// <seealso cref="RegisterRowBindingType"/>
        static public string RegisteredRowBindingName(Type rowBindingType)
        {
            return _registeredRowBindingName[rowBindingType] as string; 
        }

        /// <summary>
        /// Determines whether this binding applies to the given item.
        /// </summary>
        /// <remarks>
        /// This method is used by <see cref="VirtualTree"/> to determine the RowBinding to use
        /// to display an item in the tree.  The row parameter may be null if the row that the
        /// item belongs to has not yet been established - typically when finding items.
        /// </remarks>
        /// <param name="item">The item to bind</param>
        /// <returns>True if the binding handles the given item</returns>
        public abstract bool BindsTo(object item);

        /// <summary>
        /// Determines whether this binding applies to the given item.
        /// </summary>
        /// <remarks>
        /// This method is used by <see cref="VirtualTree"/> to determine the RowBinding to use
        /// to display an item in the tree.  The row parameter may be null if the row that the
        /// item belongs to has not yet been established - typically when finding items.  The default
        /// implementation simply calls BindsTo(item)
        /// </remarks>
        /// <param name="item">The item to bind</param>
        /// <param name="row">The row the item belongs to (if known)</param>
        /// <returns>True if the binding handles the given item</returns>
        public virtual bool BindsTo(object item, Row row)
        {
            return BindsTo(item);
        }

        /// <summary>
        /// Overridden by derived classes to return a list of children for the item displayed in the given row
        /// </summary>
        /// <param name="row">The row to get the children for</param>
        /// <returns>The list of children for the item displayed in the row</returns>
        /// <remarks>
        /// The default implementation raises the <see cref="GetChildren"/> event to get the list of children
        /// </remarks>
        public virtual IList GetChildrenForRow(Row row)
        {
            IList children = null;
            if (GetChildren != null)
            {
                // prevent recursion if the event handler calls GetChildrenForRow
                //
                GetChildrenHandler getChildren = GetChildren;
                GetChildren = null;
                GetChildrenEventArgs e = new GetChildrenEventArgs(row);
                getChildren(this, e);
                children = e.Children;
                GetChildren = getChildren;
            }
            return children;
        }
       
        /// <summary>
        /// Is there a handler for the <see cref="GetParent"/> event defined
        /// </summary>
        protected bool UseGetChildrenEvent
        {
            get { return GetChildren != null; }
        }
   
        /// <summary>
        /// Returns the parent item for the given item
        /// </summary>
        /// <remarks>
        /// This method allows the <see cref="NS.VirtualTree"/> to locate the row that uniquely corresponds to a
        /// given item in the tree.  It must be implemented if the binding is to support the simplified methods 
        /// for locating and selecting items in the <see cref="VirtualTree"/>.  This includes 
        /// <see cref="NS.VirtualTree.FindRow(object)"/>, <see cref="NS.VirtualTree.SelectedItem"/> and 
        /// <see cref="NS.VirtualTree.SelectedItems"/>.   The default implementation raises the <see cref="GetParent"/>
        /// event to get the parent for this binding.
        /// </remarks>
        /// <param name="item">The item to get the parent for</param>
        /// <returns>The parent item for the given item</returns>
        public virtual object GetParentForItem(object item)
        {
            object parent = null;
            if (GetParent != null)
            {
                GetParentEventArgs e = new GetParentEventArgs(item);
                GetParent(this, e);
                parent = e.Parent;
            }
            return parent;
        }
  
        /// <summary>
        /// Is there a handler for the <see cref="GetParent"/> event defined
        /// </summary>
        protected bool UseGetParentEvent
        {
            get { return GetParent != null; }
        }

        /// <summary>
        /// Return the Child Policy to use for the given row.
        /// </summary>
        /// <param name="row">The row to get the ChildPolicy for</param>
        /// <returns>The child policy to use for this row</returns>
        public virtual RowChildPolicy GetChildPolicy(Row row)
        {
            return _childPolicy;
        }
        
        /// <summary>
        /// Return the context menu to display for the given row
        /// </summary>
        /// <param name="row">The row to get the context menu for</param>
        /// <returns>The context menu to display for the given row</returns>
        public virtual ContextMenuStrip GetContextMenuStrip(Row row)
        {
            return _contextMenuStrip;
        }
 
        /// <summary>
        /// Return true if the given row is allowed to be dragged
        /// </summary>
        /// <param name="row">The row to get the allowed drop locations for</param>
        /// <returns>True if the row is allowed to be dragged</returns>
        public virtual bool GetAllowDrag(Row row)
        {
            return _allowDrag;
        }
       
        /// <summary>
        /// Return the allowed drop locations for the given row
        /// </summary>
        /// <param name="row">The row to get the allowed drop locations for</param>
        /// <param name="data">The data being dropped</param>
        /// <returns>The allowed drop locations</returns>
        public virtual RowDropLocation GetAllowedDropLocations(Row row, IDataObject data)
        {
            return _allowedDropLocations;
        }
 
        /// <summary>
        /// Return the drop effect to use for the given row, drop location and data
        /// </summary>
        /// <param name="row">The row being dropped on</param>
        /// <param name="dropLocation">The location of the drop</param>
        /// <param name="data">The data being dropped</param>
        /// <returns>The allowed drop locations</returns>
        public virtual DragDropEffects GetDropEffect(Row row, 
                                                     RowDropLocation dropLocation, 
                                                     IDataObject data)
        {
            return DragDropEffects.Move;
        }

        /// <summary>
        /// Handle dropping of dragged data onto a row of this type
        /// </summary>
        /// <param name="row">The row being dropped on</param>
        /// <param name="dropLocation">The location of the drop</param>
        /// <param name="data">The data being dropped</param>
        /// <param name="dropEffect">The type of drop operation</param>
        /// <remarks>
        /// The default method is a no-op
        /// </remarks>
        public virtual void OnDrop(Row row, 
                                   RowDropLocation dropLocation, 
                                   IDataObject data,
                                   DragDropEffects dropEffect)
        {
        }
 
        /// <summary>
        /// Populate the row data for this row binding
        /// </summary>
        /// <param name="row">The row to get the data for</param>
        /// <param name="rowData">The data for the row</param>
        public virtual void GetRowData(Row row, RowData rowData)
        {
            rowData.Icon = Icon;
            rowData.ExpandedIcon = ExpandedIcon;
            rowData.IconSize = IconSize;
            rowData.AutoFitHeight = AutoFitHeight;
            rowData.Resizable = Resizable;
            rowData.ShowPrefixColumn = ShowPrefixColumn;
            rowData.Height = Height;
            rowData.OddStyle = OddStyle;
            rowData.EvenStyle = EvenStyle;
            rowData.PrintEvenStyle = PrintEvenStyle;
            rowData.PrintOddStyle = PrintOddStyle;

            IDataErrorInfo errorInfo = row.Item as IDataErrorInfo;
            if (errorInfo != null)
            {
                rowData.Error = errorInfo.Error;
            }
        }

        /// <summary>
        /// Get the <see cref="CellData"/> defining how the given cell within the row should be displayed.
        /// </summary>
        /// <remarks>
        /// Finds the <see cref="CellBinding"/> for the given <see cref="NS.Column"/>
        /// and calls the <see cref="NS.CellBinding.GetCellData"/> method on it.  
        /// </remarks>
        /// <param name="row">The row the cell belongs to</param>
        /// <param name="column">The column the cell belongs to</param>
        /// <param name="cellData">The data to be displayed in the cell</param>
        public virtual void GetCellData(Row row, Column column, CellData cellData)
        {
            CellBinding binding = CellBinding(column);
            if (binding != null)
            {
                binding.GetCellData(row, cellData);
            }
        }

        /// <summary>
        /// Return the CellBinding to use for the given column
        /// </summary>
        /// <param name="column">The column to get the cell binding for</param>
        /// <remarks>
        /// If <see cref="SpanningRow"/> is set to true then the first CellBinding in the
        /// collection is always used.
        /// </remarks>
        /// <returns>The CellBinding to use</returns>
        protected virtual CellBinding CellBinding(Column column)
        {
            if (_cellBindings.Count == 0) return null;
            CellBinding result = _cellBindings[column];

            // for spanning rows we must return a binding if this is not a
            // prefix column
            //
            if (_spanningRow && result == null && !column.PrefixColumn)
            {
                foreach (CellBinding cellBinding in _cellBindings)
                {
                    if (!cellBinding.Column.PrefixColumn)
                    {
                        result = cellBinding;
                        break;
                    }
                }
            }
            return result;
        }

        /// <summary>
        /// Change the value of the cell defined by the given row/column
        /// </summary>
        /// <param name="row">The row the cell belongs to</param>
        /// <param name="column">The column the cell belongs to</param>
        /// <param name="oldValue">The old value of the cell</param>
        /// <param name="newValue">The new value of the cell</param>
        /// <returns>True if the value is successfully changed</returns>
        /// <remarks>
        /// This method allows a row binding to support mapping changes to values
        /// back into the underlying <see cref="VirtualTree.DataSource"/> when the user 
        /// uses the associated <see cref="CellEditor"/> to modify displayed values.   The 
        /// default implementation locates the <see cref="CellBinding"/> for the given <see cref="NS.Column"/>
        /// and calls the <see cref="NS.CellBinding.SetValue"/> method on it.
        /// </remarks>
        public virtual bool SetCellValue(Row row, Column column, 
                                         object oldValue, object newValue)
        {
            CellBinding binding = CellBinding(column);
            if (binding == null) return false;
            return binding.SetValue(row, oldValue, newValue);
        }

        /// <summary>
        /// Does this row binding provide cell data for the given column.
        /// </summary>
        /// <param name="column">The column to check</param>
        /// <returns>True if the binding provides cell data for the given column</returns>
        public virtual bool SupportsColumn(Column column)
        {
            return (_cellBindings[column] != null);
        }

        /// <summary>
        /// Create a new RowWidget to use for rows using this binding
        /// </summary>
        /// <param name="panelWidget">The parent PanelWidget for the RowWidget</param>
        /// <param name="row">The Row for the RowWidget</param>
        /// <returns>A new RowWidget</returns>
        public virtual RowWidget CreateRowWidget(PanelWidget panelWidget, Row row)
        {
            return (_spanningRow) ? 
                new SpanningRowWidget(panelWidget, row) : new RowWidget(panelWidget, row);
        }

        /// <summary>
        /// Create a cell binding of the correct type for this row binding.
        /// </summary>
        /// <returns>A new cell binding for this row binding</returns>
        public abstract CellBinding CreateCellBinding();

        #endregion
 
        #region Internal Methods

        /// <summary>
        /// Attach the binding to a tree.
        /// </summary>
        /// <param name="tree">The tree to attach this binding to</param>
        internal void SetTree(VirtualTree tree)
        {
            _tree = tree;
            InitializeStyles();
        }

        /// <summary>
        /// Setup the relationships between styles
        /// </summary>
        internal protected virtual void InitializeStyles()
        {
            if (_tree == null) return;

            _style.ParentStyle = _tree.RowStyle;
            _oddStyle.ParentStyle = new Style(_tree.RowOddStyle, _style.Delta);
            _evenStyle.ParentStyle = new Style(_tree.RowEvenStyle, _style.Delta);

            _printStyle.ParentStyle = new Style(_tree.RowPrintStyle, _style.Delta);
            _printOddStyle.ParentStyle = _printStyle.Copy(_tree.RowPrintStyle, _tree.RowPrintOddStyle);
            _printEvenStyle.ParentStyle = _printStyle.Copy(_tree.RowPrintStyle, _tree.RowPrintEvenStyle);
     
            // update the cellbinding styles
            //
            foreach (CellBinding cellBinding in CellBindings)
            {
                cellBinding.InitializeStyles();
            }
        }

        /// <summary>
        /// Calls Tree.DisplayErrorMessage to display an error message
        /// </summary>
        /// <param name="message">The message to display</param>
        internal protected virtual void DisplayErrorMessage(string message)
        {
            if (Tree != null)
            {
                Tree.DisplayErrorMessage(message);
            }
        }

        /// <summary>
        /// Should binding exceptions be suppressed
        /// </summary>
        protected bool SuppressBindingExceptions
        {
            get { return (Tree == null) ? false : Tree.SuppressBindingExceptions; }
        }

        #endregion

     }


}
