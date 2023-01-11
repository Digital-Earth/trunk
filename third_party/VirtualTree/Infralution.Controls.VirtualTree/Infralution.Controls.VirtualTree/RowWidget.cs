#region File Header
//
//      FILE:   RowWidget.cs.
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
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Windows.Forms;
using System.Collections;
using System.Collections.Generic;
using NS=Infralution.Controls.VirtualTree;
using System.Diagnostics;
using Infralution.Controls.VirtualTree.Properties;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a <see cref="Widget"/> for displaying <see cref="NS.Row">Rows</see> of a <see cref="VirtualTree"/>. 
    /// </summary>
    /// <remarks>
    /// <para>
    /// This class defines the visual appearance and behaviour of rows within a <see cref="VirtualTree"/>.
    /// The class uses the <see cref="VirtualTree.GetRowData">VirtualTree.GetRowData</see> method to 
    /// get the <see cref="NS.RowData"/> which defines the information to be displayed.
    /// </para>
    /// <para>
    /// The class creates a <see cref="NS.CellWidget"/> for each visible <see cref="NS.Column"/> to display
    /// the <see cref="CellData"/>.  It also uses an <see cref="NS.ExpansionWidget"/> to handle the display
    /// a behaviour of the expansion indicator for the <see cref="NS.Row"/>
    /// </para>
    /// <para>
    /// This class can be extended to customize the appearance and/or behaviour of some or all rows.  To
    /// have the <see cref="NS.VirtualTree"/> use the derived class you must override the 
    /// <see cref="VirtualTree.CreateRowWidget"/> method.
    /// </para>
    /// </remarks>
    /// <seealso cref="NS.VirtualTree"/>
    /// <seealso cref="NS.Row"/>
    /// <seealso cref="NS.CellWidget"/>
    /// <seealso cref="NS.ExpansionWidget"/>
    public class RowWidget : Widget
    {
        #region Member Variables

        /// <summary>
        /// The row the widget is associated with
        /// </summary>
        private Row _row;

        /// <summary>
        /// The row data for this widget
        /// </summary>
        private RowData _rowData;
        
        /// <summary>
        /// The expansion widget for this row
        /// </summary>
        private ExpansionWidget _expansionWidget;

        /// <summary>
        /// The row header widget for this row
        /// </summary>
        private RowHeaderWidget _rowHeaderWidget;

        /// <summary>
        /// The columns the widget is to display
        /// </summary>
        private SimpleColumnList _columns;

        /// <summary>
        /// Specifies the column traversal order for editing (includes prefix column if present)
        /// </summary>
        private SimpleColumnList _traversalColumns;

        /// <summary>
        /// The column to display the connections in
        /// </summary>
        private Column _mainColumn;

        /// <summary>
        /// Should the row header be displayed
        /// </summary>
        private bool _showRowHeader = false;

        /// <summary>
        /// The cell widgets for this row indexed by column
        /// </summary>
        private Dictionary<Column, CellWidget> _cellWidgets = new Dictionary<Column, CellWidget>();

        /// <summary>
        /// Is the row currently being dragged
        /// </summary>
        private bool _dragging = false;

        /// <summary>
        /// Is the row currently being dropped on
        /// </summary>
        private bool _dropping = false;

        /// <summary>
        /// Should the row be selected on mouse up
        /// </summary>
        private bool _selectOnMouseUp = false;

        /// <summary>
        /// The current drop location for this row
        /// </summary>
        private RowDropLocation _dropLocation = RowDropLocation.None;

        /// <summary>
        /// Timer used to autoscroll the tree when dropping
        /// </summary>
        private Timer _dropScrollTimer;

        /// <summary>
        /// Timer used to expand the row when the user hovers over the row when dropping
        /// </summary>
        private Timer _dropExpandTimer;

        //  Icons for drag/drop
        // 
        private static Icon _dropBelowIcon;
        private static Icon _dropAboveIcon;
        private static Icon _dropOnIcon;
        private static Icon _dropNoneIcon;

        private static Cursor _moveCursor;
        private static Cursor _copyCursor;

        #endregion

        #region Public Interface

        /// <summary>
        /// Creates a new widget.
        /// </summary>
        /// <param name="panelWidget">The panel widget that the row widget belongs to (may be null)</param>
        /// <param name="row">The row the widget is to display</param>
        public RowWidget(PanelWidget panelWidget, Row row)
            : base(panelWidget)
        {
            _row = row;
            _rowData = Tree.GetDataForRow(row);       
            Debug.Assert(_rowData != null);
        }

        /// <summary>
        /// Return the PanelWidget the widget belongs to
        /// </summary>
        public PanelWidget PanelWidget
        {
            get { return (PanelWidget)ParentWidget; }
        }

        /// <summary>
        /// Return the tree the widget is associated with
        /// </summary>
        public VirtualTree Tree
        {
            get { return _row.Tree; }
        }

        /// <summary>
        /// The row the widget is associated with
        /// </summary>
        public Row Row
        {
            get { return _row; }
        }

        /// <summary>
        /// The columns that the widget is to display
        /// </summary>
        public SimpleColumnList Columns
        {
            get { return _columns; }
            set 
            { 
                _columns = value;

                // force the traversal columns to be recalcuated
                //
                _traversalColumns = null;
            }
        }

        /// <summary>
        /// The column used to display icons and connections
        /// </summary>
        public Column MainColumn 
        {
            get { return _mainColumn; }
            set { _mainColumn = value; }
        }

        /// <summary>
        /// Should this row widget display the row header
        /// </summary>
        public bool ShowRowHeader 
        {
            get { return _showRowHeader; }
            set { _showRowHeader = value; }
        }

        /// <summary>
        /// Return the data for this row widget 
        /// </summary>
        public RowData RowData
        {
            get { return _rowData; }
        }

        /// <summary>
        /// Force the RowWidget to update its RowData and the CellData for each of its CellWidgets
        /// </summary>
        public virtual void UpdateData()
        {
            if (!Row.Disposed)
            {
                _rowData = Tree.GetDataForRow(Row);
                Debug.Assert(_rowData != null);

                // Call UpdateData for each cell widget
                //
                foreach (CellWidget cellWidget in _cellWidgets.Values)
                {
                    // don't update the cell widget if the column is no longer part of the tree
                    //
                    if (cellWidget.Column.Tree == Tree)
                    {
                        cellWidget.UpdateData();
                    }
                }        
            }
        }

        /// <summary>
        /// Get the cell widget belonging to this row for the given column
        /// </summary>
        /// <param name="column">The column to get the cell widget for</param>
        /// <returns>The cell widget for the given column</returns>
        public virtual CellWidget GetCellWidget(Column column)
        {
            CellWidget widget = null;
            if (!_cellWidgets.TryGetValue(column, out widget))
            {
                widget = Tree.CreateCellWidget(this, column);
                _cellWidgets[column] = widget;
            }
            return widget;
        }

        /// <summary>
        /// Return the expansion widget for this row
        /// </summary>
        /// <remarks>
        /// Creates the expansion widget on demand.
        /// </remarks>
        public virtual ExpansionWidget ExpansionWidget
        {
            get 
            { 
                if (_expansionWidget == null)
                {
                    _expansionWidget = Tree.CreateExpansionWidget(this);
                }
                return _expansionWidget; 
            }
        }
        
        /// <summary>
        /// Return the header widget for this row
        /// </summary>
        /// <remarks>
        /// Creates the widget on demand.
        /// </remarks>
        public virtual RowHeaderWidget RowHeaderWidget
        {
            get 
            { 
                if (_rowHeaderWidget == null)
                {
                    _rowHeaderWidget = Tree.CreateRowHeaderWidget(this);
                }
                return _rowHeaderWidget; 
            }
        }


        /// <summary>
        /// Return the first cell widget that meets the given criteria 
        /// </summary>
        /// <param name="editable">Does the cell widget have to be editable</param>
        /// <param name="selectable">Does the cell widget have to be selectable</param>
        /// <returns>The cell widget or null if no cells meet the criteria</returns>
        public virtual CellWidget GetFirstCellWidget(bool selectable, bool editable)
        {
            foreach (Column column in TraversalColumns)
            {
                CellWidget cellWidget = GetCellWidget(column);
                if ((!editable || cellWidget.Editable) && 
                    (!selectable || cellWidget.Selectable))
                    return cellWidget;
            }
            return null;
        }

        /// <summary>
        /// Return the prior cell widget before the given column that meets the given criteria
        /// </summary>
        /// <param name="column">The column before which to locate a cell widget</param>
        /// <param name="editable">Does the cell widget have to be editable</param>
        /// <param name="selectable">Does the cell widget have to be selectable</param>
        /// <returns>The cell widget or null if no cells meet the criteria</returns>
        public virtual CellWidget GetPriorCellWidget(Column column, bool selectable, bool editable)
        {
            SimpleColumnList columns = TraversalColumns;
            int startIndex = columns.IndexOf(column);
            if (startIndex > 0)
            {
                for (int i = startIndex - 1; i >= 0; i--)
                {
                    CellWidget cellWidget = GetCellWidget(columns[i]);
                    if ((!editable || cellWidget.Editable) &&
                        (!selectable || cellWidget.Selectable))
                        return cellWidget;
                }
            }
            return null;
        }

        /// <summary>
        /// Return the next cell widget following the given column that meets the given criteria
        /// </summary>
        /// <param name="editable">Does the cell widget have to be editable</param>
        /// <param name="selectable">Does the cell widget have to be selectable</param>
        /// <param name="column">The column after which to locate a cell widget</param>
        /// <returns>The cell widget or null if no cells meet the criteria</returns>
        public virtual CellWidget GetNextCellWidget(Column column, bool selectable, bool editable)
        {
            SimpleColumnList columns = TraversalColumns;
            int startIndex = columns.IndexOf(column);
            if (startIndex >= 0)
            {
                for (int i = startIndex + 1; i < columns.Count; i++)
                {
                    CellWidget cellWidget = GetCellWidget(columns[i]);
                    if ((!editable || cellWidget.Editable) &&
                        (!selectable || cellWidget.Selectable))
                        return cellWidget;
                }
            }
            return null;
        }

        /// <summary>
        /// Return the last cell widget in the row that meets the given criteria
        /// </summary>
        /// <param name="editable">Does the cell widget have to be editable</param>
        /// <param name="selectable">Does the cell widget have to be selectable</param>
        /// <returns>The cell widget or null if no cells meet the criteria</returns>
        public virtual CellWidget GetLastCellWidget(bool selectable, bool editable)
        {
            SimpleColumnList columns = TraversalColumns;
            for (int i = columns.Count - 1; i >= 0; i--)
            {
                CellWidget cellWidget = GetCellWidget(columns[i]);
                if ((!editable || cellWidget.Editable) &&
                    (!selectable || cellWidget.Selectable))
                    return cellWidget;
            }
            return null;
        }

        /// <summary>
        /// Return the optimal height for this row to fit data content
        /// </summary>
        /// <param name="graphics">The context being drawn to</param>
        /// <returns>The optimal height in for the row pixels</returns>
        public virtual int GetOptimalRowHeight(Graphics graphics)
        {
            int height = Tree.MinRowHeight;
            foreach (Column column in _columns)
            {
                CellWidget cellWidget = GetCellWidget(column);
                int cellHeight = cellWidget.GetOptimalHeight(graphics);
                if (cellHeight > height)
                    height = cellHeight;
            }
            int maxHeight = Tree.MaxRowHeight;
            if (maxHeight != 0)
            {
                height = Math.Min(height, maxHeight);
            }
            return height;
        }

        /// <summary>
        /// Return the optimal width (to fit data content) for the given column
        /// </summary>
        /// <param name="column">The column to get the optimal width for</param>
        /// <param name="graphics">The graphics context to use to determine the width</param>
        /// <returns>The width of the column</returns>
        public virtual int GetOptimalColumnWidth(Column column, Graphics graphics)
        {
            CellWidget cellWidget = GetCellWidget(column);
            if (cellWidget == null) return 0;
            return cellWidget.GetOptimalWidth(graphics);
        }

 
        #endregion

        #region Local Methods/Overrides

        /// <summary>
        /// Returns the columns (including the prefix column) in the editing traversal order
        /// </summary>
        protected SimpleColumnList TraversalColumns
        {
            get
            {
                if (_traversalColumns == null && _columns != null)
                {
                    _traversalColumns = _columns;
                    Column prefixColumn = Tree.PrefixColumn;
                    if (prefixColumn != null)
                    {
                        int index = _columns.IndexOf(MainColumn);
                        if (index >= 0)
                        {
                            // insert the prefix column into the collection at the 
                            // correct index
                            //
                            _traversalColumns = new SimpleColumnList();
                            int i = 0;
                            foreach (Column column in _columns)
                            {
                                if (index == i)
                                {
                                    _traversalColumns.Add(prefixColumn);
                                }
                                _traversalColumns.Add(column);
                                i++;
                            }
                        }
                    }
                }
                return _traversalColumns;
            }
        }

        /// <summary>
        /// The currently active <see cref="CellWidget">CellWidgets</see> for this RowWidget
        /// </summary>
        protected IEnumerable<CellWidget> CellWidgets
        {
            get { return _cellWidgets.Values; }
        }

        /// <summary>
        /// Dispose of the cell widgets
        /// </summary>
        /// <param name="disposing"></param>
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                foreach (CellWidget cellWidget in _cellWidgets.Values)
                {
                    cellWidget.Dispose();
                }
                _cellWidgets.Clear();

                if (_expansionWidget != null)
                {
                    _expansionWidget.Dispose();
                    _expansionWidget = null;
                }
                if (_rowHeaderWidget != null)
                {
                    _rowHeaderWidget.Dispose();
                    _rowHeaderWidget = null;
                }
            }
        }

        /// <summary>
        /// Should this row be selected when the mouse button is released
        /// </summary>
        protected bool SelectOnMouseUp
        {
            get { return _selectOnMouseUp; }
            set { _selectOnMouseUp = value; }
        }

        /// <summary>
        /// Is the row being dragged
        /// </summary>
        protected bool Dragging
        {
            get { return _dragging; }
            set { _dragging = value; }
        }

        /// <summary>
        /// Is the row being dropped on
        /// </summary>
        protected bool Dropping
        {
            get { return _dropping; }
            set 
            { 
                _dropping = value; 
                Invalidate();
            }
        }

        /// <summary>
        /// The current drop location for this row
        /// </summary>
        public RowDropLocation DropLocation
        {
            get { return _dropLocation; }
            set
            {
                if (_dropLocation != value)
                {
                    _dropLocation = value;
                    Invalidate();                     
                }
            }
        }

        /// <summary>
        /// Return the style to use for unselected rows
        /// </summary>
        protected virtual Style GetUnselectedStyle()
        {
            return ((Row.RowIndex % 2) == 0) ? _rowData.EvenStyle : _rowData.OddStyle;
        }

        /// <summary>
        /// Return the style to use for selected cells
        /// </summary>
        protected virtual Style GetSelectedStyle()
        {
            Style style = GetUnselectedStyle();
            VirtualTree tree = Tree;
            if (tree.ContainsFocus)
            {
                style = tree.RowSelectedStyle.Copy(tree.RowStyle, style);
            }
            else
            {
                style = tree.RowSelectedUnfocusedStyle.Copy(tree.RowStyle, style);
            }
            return style;
        }

        /// <summary>
        /// Return the style to use based on the current state
        /// </summary>
        public virtual Style GetActiveStyle()
        {
            if (Row.Selected && Tree.SelectedColumn == null &&
                Tree.SelectionMode != SelectionMode.MainCellText)
                return GetSelectedStyle();
            else
                return GetUnselectedStyle();
        }

        /// <summary>
        /// Update the bounds of this widget when layout is changed
        /// </summary>
        public override void OnLayout()
        {
            Dictionary<CellWidget, CellWidget> activeCellWidgets = new Dictionary<CellWidget, CellWidget>();
            Rectangle panelClip = PanelClipBounds();
            Rectangle cellBounds = Bounds;     
            ChildWidgets.Clear();        

            // add the row header if any
            //
            if (ShowRowHeader)
            {
                cellBounds.Width = Tree.RowHeaderWidth;
                RowHeaderWidget.Bounds = RtlTranslateRect(cellBounds);
                ChildWidgets.Add(RowHeaderWidget);
                cellBounds.X += cellBounds.Width;
            }

            if (_columns != null)
            {
                foreach(Column column in _columns)
                {
                    cellBounds.Width = column.Width;
                    Rectangle rtlCellBounds = RtlTranslateRect(cellBounds);

                    // check the cell widget is visible before creating/adding it
                    //
                    if (rtlCellBounds.Width > 0 &&
                        rtlCellBounds.Left < panelClip.Right &&
                        rtlCellBounds.Right > panelClip.Left)
                    {
                        CellWidget cellWidget = GetCellWidget(column);
                        activeCellWidgets.Add(cellWidget, cellWidget);
                        cellWidget.Bounds = rtlCellBounds;
                        ChildWidgets.Add(cellWidget);
                    
                        if (column == _mainColumn)
                        {

                            // add the prefix cell widget if any
                            //
                            Column prefixColumn = Tree.PrefixColumn;
                            if (_rowData.ShowPrefixColumn && prefixColumn != null && prefixColumn.Visible)
                            {
                                Rectangle prefixBounds = cellWidget.GetPrefixBounds(prefixColumn);

                                // check the widget is visible before adding it
                                //
                                if (prefixBounds.Width > 0 &&
                                    prefixBounds.Left < panelClip.Right &&
                                    prefixBounds.Right > panelClip.Left)
                                {
                                    CellWidget prefixWidget = GetCellWidget(prefixColumn);

                                    // if both the cell editor and value are null then don't display
                                    // the prefix 
                                    //
                                    CellData cellData = prefixWidget.CellData;
                                    if (cellData.Editor != null || cellData.Value != null)
                                    {
                                        activeCellWidgets.Add(prefixWidget, prefixWidget);
                                        prefixWidget.Bounds = prefixBounds;
                                        ChildWidgets.Add(prefixWidget);

                                        // set the width to reserve for the prefix widget
                                        //
                                        cellWidget.PrefixWidth = prefixBounds.Width;
                                    }
                                }
                            }
                        }
                    }
                    cellBounds.X += cellBounds.Width;

                    // exit the loop once we have laid out the last visible cell
                    //
                    if (RightToLeft == RightToLeft.Yes)
                    {
                        if (rtlCellBounds.Right < panelClip.Left)
                            break;
                    }
                    else
                    {
                        if (rtlCellBounds.Left > panelClip.Right)
                            break;
                    }

                }

            }

            // Park widgets that are not active so that their editor (if any) is hidden
            //
            foreach (CellWidget cellWidget in _cellWidgets.Values)
            {
                if (cellWidget.Bounds != Rectangle.Empty)
                {
                    if (!activeCellWidgets.ContainsKey(cellWidget))
                    {
                        cellWidget.Bounds = Rectangle.Empty;
                    }
                }
            }
        }

        /// <summary>
        /// Return the x coordinate to paint the drop locator at
        /// </summary>
        protected virtual int DropIndicatorX
        {
            get 
            {
                return (Tree.ShowRowHeaders) ? Tree.RowHeaderWidth : 0;
            }
        }

        /// <summary>
        /// Paint the drop indicator for the current drop location
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        protected virtual void PaintDropNoneIndicator(Graphics graphics)
        {
            if (_dropNoneIcon == null)
            {
                _dropNoneIcon = Resources.DropNoneIcon; 
            }
            int y = Bounds.Y + (Bounds.Height - _dropNoneIcon.Height) / 2;
            Rectangle bounds = new Rectangle(DropIndicatorX, y, _dropNoneIcon.Width, _dropNoneIcon.Height);            
            bounds = DrawingUtilities.RtlTranslateRect(WidgetControl, bounds);
            DrawingUtilities.DrawIcon(graphics, _dropNoneIcon, bounds.X, bounds.Y, false, WidgetControl.RightToLeft == RightToLeft.Yes);
        }

        /// <summary>
        /// Paint the drop indicator for the current drop location
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        protected virtual void PaintDropOnIndicator(Graphics graphics)
        {   
            if (_dropOnIcon == null)
            {
                _dropOnIcon = Resources.DropOnIcon;
            }
            int y = Bounds.Y + (Bounds.Height - _dropOnIcon.Height) / 2;
            Rectangle bounds = new Rectangle(DropIndicatorX, y, _dropOnIcon.Width, _dropOnIcon.Height);
            bounds = DrawingUtilities.RtlTranslateRect(WidgetControl, bounds);
            DrawingUtilities.DrawIcon(graphics, _dropOnIcon, bounds.X, bounds.Y, false, WidgetControl.RightToLeft == RightToLeft.Yes);
        }

        /// <summary>
        /// Paint the drop indicator for the current drop location
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        protected virtual void PaintDropAboveIndicator(Graphics graphics)
        {      
            if (_dropAboveIcon == null)
            {
                _dropAboveIcon = Resources.DropAboveIcon;
            }
            Rectangle bounds = new Rectangle(DropIndicatorX, Bounds.Y, _dropAboveIcon.Width, _dropAboveIcon.Height);
            bounds = DrawingUtilities.RtlTranslateRect(WidgetControl, bounds);
            DrawingUtilities.DrawIcon(graphics, _dropAboveIcon, bounds.X, bounds.Y, false, WidgetControl.RightToLeft == RightToLeft.Yes);
        }

        /// <summary>
        /// Paint the drop indicator for the current drop location
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        protected virtual void PaintDropBelowIndicator(Graphics graphics)
        {   
            if (_dropBelowIcon == null)
            {
                _dropBelowIcon = Resources.DropBelowIcon;
            }
            int y = Bounds.Y + Bounds.Height - _dropBelowIcon.Height;
            Rectangle bounds = new Rectangle(DropIndicatorX, y, _dropBelowIcon.Width, _dropBelowIcon.Height);
            bounds = DrawingUtilities.RtlTranslateRect(WidgetControl, bounds);
            DrawingUtilities.DrawIcon(graphics, _dropBelowIcon, bounds.X, bounds.Y, false, WidgetControl.RightToLeft == RightToLeft.Yes);
        }

        /// <summary>
        /// Paint the drop indicator for the current drop location
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        protected virtual void PaintDropIndicator(Graphics graphics)
        {        
            switch(DropLocation)
            {
                case RowDropLocation.OnRow:
                    PaintDropOnIndicator(graphics);
                    break;
                case RowDropLocation.AboveRow:
                    PaintDropAboveIndicator(graphics);
                    break;
                case RowDropLocation.BelowRow:
                    PaintDropBelowIndicator(graphics);
                    break;
                case RowDropLocation.None:
                    PaintDropNoneIndicator(graphics);
                    break;
            }
        }

        /// <summary>
        /// Return the clipping bounds for the panel containing the row
        /// </summary>
        /// <returns></returns>
        public virtual Rectangle PanelClipBounds()
        {
            return PanelWidget.ClipBounds;
        }

        /// <summary>
        /// Set the clipping region to restrict drawing to the row bounds
        /// </summary>
        /// <param name="graphics">The graphics context</param>
        /// <param name="style">The style to use</param>
        protected virtual void SetRowClip(Graphics graphics, Style style)
        {
            // when using adjust border style set the clip rectangle so that
            // we don't draw over rows above us - this stops us screwing
            // up selected rows above this row
            //
            Rectangle clipBounds = Bounds;
            if (style.BorderStyle == Border3DStyle.Adjust)
            {
                int borderWidth = style.BorderWidth;
                clipBounds.Height += borderWidth;
                clipBounds.Width += borderWidth;
            }
            clipBounds = RtlTranslateRect(clipBounds);
            clipBounds.Intersect(PanelClipBounds());
            DrawingUtilities.SetClip(graphics, clipBounds);
        }

        /// <summary>
        /// Set the clipping region to restrict drawing to the row bounds
        /// </summary>
        /// <param name="graphics">The graphics context</param>
        public virtual void SetRowClip(Graphics graphics)
        {
            SetRowClip(graphics, GetActiveStyle());
        }

        /// <summary>
        /// Print the row widget to the given graphics context
        /// </summary>
        /// <param name="graphics">The context to print to</param>
        public override void OnPrint(Graphics graphics)
        {
            Style style = ((Row.RowIndex % 2) == 0) ? RowData.PrintEvenStyle : RowData.PrintOddStyle;

            SetRowClip(graphics, style); 
            PaintBackground(graphics, style, false);

            // print the child widgets
            //
            base.OnPrint(graphics);         
  
        }

        /// <summary>
        /// Paint the background and border of the row
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        /// <param name="style">The style to use</param>
        /// <param name="printing">Is it a printing context</param>
        protected virtual void PaintBackground(Graphics graphics, Style style, bool printing)
        {
            // paint the background for the row
            //
            Rectangle bounds = Bounds;
            if (this.ShowRowHeader)
            {
                int rowHeaderWidth = Tree.RowHeaderWidth;
                bounds.X += rowHeaderWidth;
                bounds.Width -= rowHeaderWidth;
            }
            bounds = RtlTranslateRect(bounds);
            style.DrawBackground(graphics, bounds);
            style.DrawBorder(graphics, bounds);
        }

        /// <summary>
        /// Handle painting for this widget
        /// </summary>
        /// <param name="graphics"></param>
        public override void OnPaint(Graphics graphics)
        {
            VirtualTree tree = Tree;           
            Style style = GetActiveStyle(); 
            SetRowClip(graphics, style);
            PaintBackground(graphics, style, false);

            // Paint the child (cellWidgets) 
            //
            base.OnPaint(graphics);

            // paint the drop indicator
            //
            if (Dropping)
            {
                PaintDropIndicator(graphics);
            }
            
            // reset the clip area to the entire panel
            //
            DrawingUtilities.SetClip(graphics, PanelWidget.ClipBounds);
        }

        /// <summary>
        /// Handle mouse selection for this row
        /// </summary>
        /// <param name="e">The original mouse event</param>
        protected virtual void OnMouseSelection(MouseEventArgs e)
        {
            switch (Control.ModifierKeys)
            {
                case Keys.Control:
                    Tree.ExtendSelectRow(Row);
                    break;
                case Keys.Shift:
                    Tree.SelectRowRange(Row);
                    break;
                case Keys.Control | Keys.Shift:
                    Tree.ExtendSelectRowRange(Row);
                    break;
                default:
                    Tree.SelectRow(Row);
                    break;
            }
            Tree.FocusRow = Row;
        }


        /// <summary>
        /// Handle a left mouse down event.
        /// </summary>
        /// <param name="enableDragSelect">Should drag selection be allowed</param>
        /// <param name="e">The mouse event details.</param>
        public virtual void OnLeftMouseDown(MouseEventArgs e, bool enableDragSelect)
        {
            if (Row.Selected && !enableDragSelect)
            {
                SelectOnMouseUp = true;
            }
            else
            {
                OnMouseSelection(e);
            }
        }

        /// <summary>
        /// Handle a right mouse down event
        /// </summary>
        /// <param name="e">The mouse event details.</param>
        public virtual void OnRightMouseDown(MouseEventArgs e)
        {
            if (!Row.Selected)
            {
                Tree.FocusRow = Row;
                Tree.SelectRow(Row);
            }
            Tree.ShowRowContextMenu(Row);
        }

        /// <summary>
        /// Handle MouseDown events for this Widget
        /// </summary>
        /// <param name="enableDragSelection">Should drag selection be allowed</param>
        /// <param name="e"></param>
        public virtual void OnMouseDown(MouseEventArgs e, bool enableDragSelection)
        {
            Tree.CompleteEdit();
            switch (e.Button)
            {
                case MouseButtons.Left:
                    OnLeftMouseDown(e, enableDragSelection);
                    break;
                case MouseButtons.Right:
                    OnRightMouseDown(e);
                    break;
            }
        }

        /// <summary>
        /// Handle MouseDown events for this Widget
        /// </summary>
        /// <param name="e"></param>
        public override void OnMouseDown(MouseEventArgs e)
        {
            Tree.SelectedColumn = null;
            OnMouseDown(e, Tree.EnableDragSelect);
        }

        /// <summary>
        /// Handle MouseUp events for this Widget.
        /// </summary>
        /// <param name="enableDragSelect">Should drag selection be allowed</param>
        /// <param name="e"></param>
        public virtual void OnMouseUp(MouseEventArgs e, bool enableDragSelect)
        {
            if (enableDragSelect)
            {
                // ensure the drag timer is stopped
                //
                Tree.StopDragSelectTimer();
                if (e.Button == MouseButtons.Left)
                {
                    Tree.CompleteDragSelection();
                }
            }
            else if (SelectOnMouseUp)
            {
                OnMouseSelection(e);
                SelectOnMouseUp = false;
            }
        }

        /// <summary>
        /// Handle MouseUp events for this Widget.
        /// </summary>
        /// <param name="e"></param>
        public override void OnMouseUp(MouseEventArgs e)
        {
            OnMouseUp(e, Tree.EnableDragSelect);
        }

        /// <summary>
        /// Handle mouse movement when drag selecting rows
        /// </summary>
        /// <param name="e"></param>
        protected virtual void OnMouseDragSelect(MouseEventArgs e)
        {
            // get the row widget which the mouse is now over
            //
            Widget widget = Tree.GetWidget(e.X, e.Y);
            if (widget != null)
            {
                RowWidget rowWidget = null;
                if (widget is RowWidget)
                    rowWidget = (widget as RowWidget);
                else if (widget is CellWidget)
                    rowWidget = (widget as CellWidget).RowWidget;
                else if (widget is RowHeaderWidget)
                    rowWidget = (widget as RowHeaderWidget).RowWidget;

                if (rowWidget != null)
                {
                    Tree.DragSelectRow(rowWidget.Row);
                }
            }
            Tree.StartDragSelectTimer();
        }
 
        /// <summary>
        /// Handle MouseMove events for this Widget
        /// </summary>
        /// <param name="e"></param>
        /// <param name="enableDragSelect">Should drag selection be allowed</param>
        public virtual void OnMouseMove(MouseEventArgs e, bool enableDragSelect)
        {
            if (WidgetControl.MouseDragging && Control.MouseButtons == MouseButtons.Left)
            {
                if (enableDragSelect)
                {
                    OnMouseDragSelect(e);
                }
                else
                {
                    if (Tree.AllowRowDrag(Row) && Tree.SelectedRows.Count > 0)
                    {
                        DoDragDrop();
                    }
                }
            }
        }

        /// <summary>
        /// Handle MouseMove events for this Widget
        /// </summary>
        /// <param name="e"></param>
        public override void OnMouseMove(MouseEventArgs e)
        {
            OnMouseMove(e, Tree.EnableDragSelect);
        }

        /// <summary>
        /// Handle Double Click events for this Widget.
        /// </summary>
        /// <param name="e"></param>
        public override void OnDoubleClick(EventArgs e)
        {
            if (Tree.ExpandOnDoubleClick)
            {
                if (Row.Expanded)
                    CollapseRow();
                else
                    ExpandRow();
            }
        }

        /// <summary>
        /// Return the maximum height that the drag cursor is allowed to be 
        /// </summary>
        /// <remarks>
        /// When dragging multiple rows the drag cursor displays the rows being dragged.
        /// However displaying a drag image containing all the selected rows may require
        /// a cursor larger than the system can support - this property limits the size
        /// of the drag cursor created.  The default value is 200.
        /// </remarks>
        protected virtual int MaxDragCursorHeight
        {
            get { return 200; }
        }

        /// <summary>
        /// Dispose of the drag cursors.
        /// </summary>
        protected virtual void DestroyDragCursors()
        {
            if (WidgetControl.DragNoneCursor != null)
            {
                DrawingUtilities.DestroyCursor(WidgetControl.DragMoveCursor);
                WidgetControl.DragMoveCursor = null;
                DrawingUtilities.DestroyCursor(WidgetControl.DragCopyCursor);
                WidgetControl.DragCopyCursor = null;
                DrawingUtilities.DestroyCursor(WidgetControl.DragNoneCursor);
                WidgetControl.DragNoneCursor = null;
            }
        }

        /// <summary>
        /// Create the drag cursor used for the row.
        /// </summary>
        protected virtual void SetDragCursors()
        {
            if (!DrawingUtilities.LargeCursorsSupported) return;

            int height = Math.Min(Bounds.Height * Tree.SelectedRows.Count, MaxDragCursorHeight);
            int width = Tree.ClientSize.Width;
            using (Bitmap bitmap = new Bitmap(width, height))
            {
                using (Graphics graphics = Graphics.FromImage(bitmap))
                {
                    PanelWidget panelWidget = Tree.CreatePanelWidget();
                    panelWidget.Bounds = new Rectangle(0, 0, width, height); ;
                    panelWidget.Printing = true;
                    Rectangle bounds = new Rectangle(0, 0, width, Bounds.Height);
                    foreach (Row row in Tree.SelectedRows)
                    {
                        RowWidget widget = Tree.CreateRowWidget(panelWidget, row);
                        widget.Columns = Tree.Columns.GetVisibleColumns();
                        widget.MainColumn = MainColumn;
                        widget.Bounds = bounds;
                        widget.OnPaint(graphics);
                        bounds.Y += bounds.Height;
                        if (bounds.Y > height)
                            break;
                    }
                }

                if (_moveCursor == null)
                {
                    _moveCursor = new Cursor(typeof(VirtualTree), "Cursors.Move.cur");
                    _copyCursor = new Cursor(typeof(VirtualTree), "Cursors.Copy.cur");
                }
                WidgetControl.DragMoveCursor =
                    DrawingUtilities.CreateCursor(bitmap, 0.5F, 0, 0, _moveCursor, 16, 16, WidgetControl.RightToLeft);
                WidgetControl.DragCopyCursor =
                    DrawingUtilities.CreateCursor(bitmap, 0.5F, 0, 0, _copyCursor, 16, 16, WidgetControl.RightToLeft);
                WidgetControl.DragNoneCursor =
                    DrawingUtilities.CreateCursor(bitmap, 0.5F, 10, 10, Cursors.No, 16, 16, WidgetControl.RightToLeft);
            }
        }

        /// <summary>
        /// Start a drag/drop operation for this row
        /// </summary>
        protected virtual void DoDragDrop()
        {
            SelectOnMouseUp = false;
            Dragging = true;
            Tree.CompleteEdit();
            SetDragCursors();
            WidgetControl.DoDragDrop(this, Tree.SelectedRows.GetRows(), DragDropEffects.All);
            DestroyDragCursors();
            Dragging = false;
        }

        /// <summary>
        /// Expand the row and scroll children into view if necessary
        /// </summary>
        internal protected virtual void ExpandRow()
        {
            Row.Expand();
        }

        /// <summary>
        /// Collapse the row
        /// </summary>
        internal protected virtual void CollapseRow()
        {
            Row.Collapse();
        }

        /// <summary>
        /// Return the interval in milliseconds to use for drop scrolling 
        /// </summary>
        protected virtual int DropScrollTimerInterval
        {
            get { return Tree.DropScrollTimerInterval; }
        }

        /// <summary>
        /// Start the drop scroll timer if this row is the first or last displayed.  This enables the
        /// tree to be scrolled when dropping 
        /// </summary>
        protected virtual void StartDropScrollTimer()
        {
            if (Row.RowIndex >= Tree.BottomRowIndex || Row.RowIndex <= Tree.TopRowIndex)
            {
                if (_dropScrollTimer == null)
                {
                    _dropScrollTimer = new Timer();
                    _dropScrollTimer.Interval = DropScrollTimerInterval;
                    _dropScrollTimer.Tick += new EventHandler(OnDropScrollTimerTick);
                    _dropScrollTimer.Start();
                }
            }
        }

        /// <summary>
        /// Stop the current drop scroll timer
        /// </summary>
        protected virtual void StopDropScrollTimer()
        {
            if (_dropScrollTimer != null)
            {
                _dropScrollTimer.Dispose();
                _dropScrollTimer = null;
            }
        }

        /// <summary>
        /// Scroll the tree up or down (depending on the position of this row) to enable the
        /// use to drop an item beyond the currently displayed rows
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnDropScrollTimerTick(object sender, EventArgs e)
        {
            if (Row.RowIndex >= Tree.BottomRowIndex && Row.RowIndex < Tree.LastRowIndex)
            {
                Tree.TopRowIndex += 1;
            }
            else if (Row.RowIndex <= Tree.TopRowIndex)
            {
                Tree.TopRowIndex -= 1;
            }
            StopDropScrollTimer();
        }

        /// <summary>
        /// Return the interval in milliseconds to use for drop expanding 
        /// </summary>
        protected virtual int DropExpandTimerInterval
        {
            get { return Tree.DropExpandTimerInterval; }
        }

        /// <summary>
        /// Starts a timer which will expand this row if the timer occurs
        /// </summary>
        protected virtual void StartDropExpandTimer()
        {
            if (Row.ShowExpansionIndicator && ! Row.Expanded)
            {
                _dropExpandTimer = new Timer();
                _dropExpandTimer.Interval = DropExpandTimerInterval;
                _dropExpandTimer.Tick += new EventHandler(OnDropExpandTimerTick);
                _dropExpandTimer.Start();
            }
        }

        /// <summary>
        /// Stop the current drop scroll timer
        /// </summary>
        protected virtual void StopDropExpandTimer()
        {
            if (_dropExpandTimer != null)
            {
                _dropExpandTimer.Dispose();
                _dropExpandTimer = null;
            }
        }

        /// <summary>
        /// Expand this row when the user hovers over it during a drag/drop operation
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnDropExpandTimerTick(object sender, EventArgs e)
        {
            if (!Row.Disposed)
                ExpandRow();
            StopDropExpandTimer();
        }

        /// <summary>
        /// Return the number pixels at the top and bottom of the row which
        /// are used for the Above and Below Drop Locations
        /// </summary>
        protected virtual int AboveBelowDropZone
        {
            get { return 3; }
        }

        /// <summary>
        /// Set the current DropLocation based on the position of the mouse and
        /// the allowed drop locations for the row
        /// </summary>
        protected virtual void UpdateDropLocation(DragEventArgs e)
        {
            RowDropLocation allowedLocations = Tree.AllowedRowDropLocations(Row, e.Data);
            Point position = WidgetControl.PointToClient(Control.MousePosition);  

            // calculate the offset used for check above/below drop zone
            // If dropping on the row is not allowed then the above/below zones
            // are just half the row height
            //
            int zone = ((allowedLocations & RowDropLocation.OnRow) != 0) 
                            ? AboveBelowDropZone : Bounds.Height / 2 + 1;

            if (((allowedLocations & RowDropLocation.AboveRow) != 0) &&
                (position.Y < Bounds.Y + zone))
                DropLocation = RowDropLocation.AboveRow;
            else if (((allowedLocations & RowDropLocation.BelowRow) != 0)&&
                     (position.Y > Bounds.Bottom - zone))
                DropLocation = RowDropLocation.BelowRow;
            else if ((allowedLocations & RowDropLocation.OnRow) != 0)
                DropLocation = RowDropLocation.OnRow;
            else
                DropLocation = RowDropLocation.None;
        }

        /// <summary>
        /// Handle dragging of data over this row
        /// </summary>
        /// <param name="e"></param>
        public override void OnDragEnter(DragEventArgs e)
        {
            Dropping = true;
            UpdateDropLocation(e);
            if (DropLocation == RowDropLocation.None)
                e.Effect = DragDropEffects.None;
            else
                e.Effect = Tree.RowDropEffect(Row, DropLocation, e.Data);
            StartDropExpandTimer();
            StartDropScrollTimer();
        }

        /// <summary>
        /// Update the current drag location
        /// </summary>
        /// <param name="e"></param>
        public override void OnDragOver(DragEventArgs e)
        {
            UpdateDropLocation(e);
            if (DropLocation == RowDropLocation.None)
                e.Effect = DragDropEffects.None;
            else
                e.Effect = Tree.RowDropEffect(Row, DropLocation, e.Data);
        }

        /// <summary>
        /// Handle drag leaving this widget
        /// </summary>
        /// <param name="e"></param>
        public override void OnDragLeave(EventArgs e)
        {
            DropLocation = RowDropLocation.None;
            StopDropExpandTimer();
            StopDropScrollTimer();
            Dropping = false;
        }

        /// <summary>
        /// Handle dropping of data on this row
        /// </summary>
        /// <param name="e"></param>
        public override void OnDragDrop(DragEventArgs e)
        {
            if (DropLocation != RowDropLocation.None)
            {
                e.Effect = Tree.RowDropEffect(Row, DropLocation, e.Data);
                if (e.Effect != DragDropEffects.None)
                {
                    Tree.OnRowDrop(Row, DropLocation, e.Data, e.Effect);
                }
            }
            StopDropExpandTimer();
            StopDropScrollTimer();
            Dropping = false;
        }
        
        #endregion

    }

}
