#region File Header
//
//      FILE:   PanelWidget.cs.
//
//    AUTHOR:   Grant Frisken
//
// COPYRIGHT:   Copyright 2005 
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
using System.Diagnostics;
using NS = Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines the main <see cref="Widget"/> for managing the display of data for the <see cref="NS.VirtualTree"/> control. 
    /// </summary>
    /// <remarks>
    /// <para>
    /// The <see cref="NS.VirtualTree"/> control uses two PanelWidgets.  One for the scrollable columns and 
    /// another for the non-scrollable header columns.  This widget manages a <see cref="HeaderWidget"/>
    /// to display the column headers and <see cref="RowWidget">RowWidgets</see> to display the row data.
    /// </para>
    /// <para>
    /// This class can be extended to customize the appearance and/or behaviour of the panel.  To
    /// have the <see cref="NS.VirtualTree"/> use the derived class you must override the 
    /// <see cref="VirtualTree.CreatePanelWidget"/> method.
    /// </para>
    /// </remarks>
    /// <seealso cref="NS.VirtualTree"/>
    /// <seealso cref="NS.RowWidget"/>
    /// <seealso cref="NS.HeaderWidget"/>
    public class PanelWidget : Widget
    {
        #region Member Variables

        /// <summary>
        /// The columns the panel is to display
        /// </summary>
        SimpleColumnList _columns;

        /// <summary>
        /// Should this panel display row headers
        /// </summary>
        bool _showRowHeaders = false;

        /// <summary>
        /// Is this panel being used to print
        /// </summary>
        bool _printing = false;

        /// <summary>
        /// The header widget (if present)
        /// </summary>
        private HeaderWidget _headerWidget = null;

        /// <summary>
        /// Current row widgets
        /// </summary>
        private ArrayList _rowWidgets = new ArrayList();

        /// <summary>
        /// Row widgets index by row
        /// </summary>
        private Hashtable _rowWidgetMap = new Hashtable();

        /// <summary>
        /// The clipping bounds for this panel (if any)
        /// </summary>
        private Rectangle _clipBounds = Rectangle.Empty;

        #endregion

        #region Public Interface

        /// <summary>
        /// Creates a new widget.
        /// </summary>
        /// <param name="tree">The VirtualTree control the widget is associated with</param>
        /// <remarks>
        /// The PanelWidget used to display header columns is parented by a separate control to
        /// enable the PanelWidget used to display the scrolling columns to slide underneath.
        /// Thus the parentControl and tree parameters may not be the same.
        /// </remarks>
        public PanelWidget(VirtualTree tree)
            : base(tree)
        {
        }

        /// <summary>
        /// Return the tree the widget is associated with
        /// </summary>
        public VirtualTree Tree
        {
            get { return WidgetControl as VirtualTree; }
        }

        /// <summary>
        /// The columns that the widget is to display
        /// </summary>
        public SimpleColumnList Columns
        {
            get { return _columns; }
            set { _columns = value; }
        }

        /// <summary>
        /// Should this widget display row headers
        /// </summary>
        public bool ShowRowHeaders 
        {
            get { return _showRowHeaders; }
            set { _showRowHeaders = value; }
        }

        /// <summary>
        /// Is this panel being used for printing
        /// </summary>
        public bool Printing 
        {
            get { return _printing; }
            set { _printing = value; }
        }

        /// <summary>
        /// The clipping bounds for this panel.
        /// </summary>
        /// <remarks>
        /// This allows the scrollable panel to be offset when horizontally scrolling 
        /// while still restricting drawing to the scrollable area
        /// </remarks>
        public Rectangle ClipBounds
        {
            get { return _clipBounds; }
            set { _clipBounds = value; }
        }

        /// <summary>
        /// Return the cumulative width of all the columns in the panel (including RowHeaders)
        /// </summary>
        public virtual int TotalColumnWidth
        {
            get
            {
                int width = 0; 
                if (ShowRowHeaders)
                {
                    width = Tree.RowHeaderWidth;
                }
                foreach (Column column in Columns)
                {
                    if (column.Visible && !column.PrefixColumn)
                    {
                        width += column.Width;
                    }
                }
                return width;
            }
        }

        /// <summary>
        /// Return the optimal width to use for the given column
        /// </summary>
        /// <param name="column">The column to get the width for</param>
        /// <returns>The width of the column</returns>
        public virtual int GetOptimalColumnWidth(Column column)
        {
            int width = column.MinWidth;
            using (Graphics graphics = WidgetControl.CreateGraphics())
            {
                foreach (RowWidget rowWidget in _rowWidgets)
                {
                    // check if the row widget is actually visible
                    //
                    if (rowWidget.Bounds.Top < Bounds.Bottom)
                    {
                        int cellWidth = rowWidget.GetOptimalColumnWidth(column, graphics);
                        if (cellWidth > width)
                            width = cellWidth;
                    }
                }
            }
            if (column.MaxAutoSizeWidth != 0)
            {
                width = Math.Min(width, column.MaxAutoSizeWidth);
            }
            return width;
        }

        /// <summary>
        /// Return the optimal height to use for the given row
        /// </summary>
        /// <param name="row">The row to get the height for</param>
        /// <returns>The optimal height for the row</returns>
        public virtual int GetOptimalRowHeight(Row row)
        {
            int height = Tree.MinRowHeight;
            RowWidget rowWidget = GetRowWidget(row);
            if (rowWidget != null)
            {
                using (Graphics graphics = WidgetControl.CreateGraphics())
                {
                    height = rowWidget.GetOptimalRowHeight(graphics);
                }
            }
            return height;
        }


        /// <summary>
        /// Update the <see cref="RowData.Height"/> for those rows which have <see cref="RowData.AutoFitHeight"/> 
        /// to fit the displayed data
        /// </summary>
        public virtual void UpdateAutoFitRowHeights()
        {
            using (Graphics graphics = WidgetControl.CreateGraphics())
            {
                foreach(RowWidget rowWidget in _rowWidgets)
                {
                    // check if the row widget is actually visible
                    //
                    if (rowWidget.Bounds.Top < Bounds.Bottom)
                    {
                        RowData rowData = rowWidget.RowData;
                        if (rowData.AutoFitHeight)
                        {
                            int height = rowWidget.GetOptimalRowHeight(graphics);
                            if (height > rowData.Height)
                                rowData.Height = height;
                        }
                    }
                }
            }
        }


        /// <summary>
        /// Force the panel to refresh the displayed data for all rows following a change to the data source.
        /// </summary>
        public virtual void UpdateRowData()
        {
            foreach (RowWidget rowWidget in _rowWidgets)
            {
                rowWidget.UpdateData();
            }
        }

        /// <summary>
        /// Returns true if the RowWidget for the given row has been created
        /// </summary>
        /// <param name="row">The row to check fo</param>
        /// <returns>True if the row widget has been created</returns>
        public virtual bool RowWidgetCreated(Row row)
        {
            return _rowWidgetMap[row] != null;
        }

        /// <summary>
        /// Return the row widget for the given row
        /// </summary>
        /// <param name="row">The row to get the widget for</param>
        /// <returns>The row widget for the given row</returns>
        public virtual RowWidget GetRowWidget(Row row)
        {
            RowWidget rowWidget = null;
            if (row != null)
            {
                rowWidget = (RowWidget)_rowWidgetMap[row];
                if (rowWidget == null)
                {
                    // if there is no row widget then create one - this allows
                    // us to call GetOptimalRowHeight even when the row is not currently
                    // displayed
                    //
                    rowWidget = Tree.CreateRowWidget(this, row);
                    rowWidget.Columns = _columns;
                    rowWidget.MainColumn = Tree.GetMainColumn();
                    rowWidget.ShowRowHeader = ShowRowHeaders;
                    rowWidget.Bounds = new Rectangle(Bounds.X, Bounds.Y, Bounds.Width, Tree.RowHeight);
                    _rowWidgetMap[row] = rowWidget;
                }
            }
            return rowWidget;
        }

        #endregion

        #region Local Methods/Overrides

        /// <summary>
        /// Dispose of the cell widgets
        /// </summary>
        /// <param name="disposing"></param>
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                foreach (RowWidget rowWidget in _rowWidgets)
                {
                    rowWidget.Dispose();
                }
                _rowWidgets.Clear();
                _rowWidgetMap.Clear();
                if (_headerWidget != null)
                {
                    _headerWidget.Dispose();
                    _headerWidget = null;
                }
            }
        }

        /// <summary>
        /// Return the header widget for the panel
        /// </summary>
        protected HeaderWidget HeaderWidget
        {
            get 
            {
                if (_headerWidget == null)
                    _headerWidget = Tree.CreateHeaderWidget(this);
                return _headerWidget; 
            }
        }

        /// <summary>
        /// Add a widget to display the given row.    
        /// </summary>
        /// <remarks>
        /// Attempts to use a cached row widget if there is one - otherwise calls CreateRowWidget
        /// to create a new row widget
        /// </remarks>
        /// <param name="row">The row to add a widget for</param>
        /// <param name="mainColumn">The main column for the tree</param>
        /// <returns>The row widget</returns>
        private RowWidget AddRowWidget(Row row, Column mainColumn)
        {
            RowWidget rowWidget = (RowWidget)_rowWidgetMap[row];
            if (rowWidget == null)
            {
                rowWidget = Tree.CreateRowWidget(this, row);
                _rowWidgetMap[row] = rowWidget;
            }
            rowWidget.Columns = _columns;
            rowWidget.MainColumn = mainColumn;
            rowWidget.ShowRowHeader = ShowRowHeaders;
            ChildWidgets.Add(rowWidget);
            return rowWidget;
         }
       
        /// <summary>
        /// Add RowWidgets for each of the visible rows reusing cached RowWidgets if possible
        /// </summary>
        private void AddRowWidgets()
        {
            ArrayList newRowWidgets = new ArrayList();
            Hashtable newRowWidgetMap = new Hashtable();
            CellWidget editWidget = Tree.EditWidget;

            // ensure that if there is active edit cell that we retain the row widget in cache
            //
            if (editWidget != null && !editWidget.Row.Disposed)
            {
                Row editRow = editWidget.Row;
                RowWidget rowWidget = GetRowWidget(editRow);
                if (rowWidget != null)
                {
                    newRowWidgetMap[editRow] = rowWidget;
                }
            }

            // Add widgets for each of the possibly visible rows - we don't know
            // which widgets will actually be visible until later because of variable
            // height rows
            //
            int startRow = Tree.TopRowIndex;
            int endRow = Math.Min(startRow + Tree.MaxVisibleRows, Tree.LastRowIndex);
            Column mainColumn = Tree.GetMainColumn();
            for (int i = startRow; i <= endRow; i++)
            {
                Row row = Tree.GetVisibleRow(i);
                if (row != null)
                {
                    Debug.Assert(!row.Disposed);
                    RowWidget rowWidget = AddRowWidget(row, mainColumn);
                    newRowWidgets.Add(rowWidget);
                    newRowWidgetMap[rowWidget.Row] = rowWidget;
                }
            }
        
            // Dispose of RowWidgets that aren't used any more
            //
            foreach (RowWidget rowWidget in _rowWidgets)
            {
                if (!newRowWidgetMap.Contains(rowWidget.Row))
                {
                    rowWidget.Dispose();
                }
            }
            _rowWidgets = newRowWidgets;
            _rowWidgetMap = newRowWidgetMap;
        }

        /// <summary>
        /// Update the displayed widgets following a change to the columns or rows
        /// </summary>
        public virtual void UpdateWidgets()
        {
            ChildWidgets.Clear();
            AddRowWidgets();
            if (Tree.ShowColumnHeaders)
            {
                HeaderWidget.ShowRowHeader = ShowRowHeaders;
                HeaderWidget.Columns = _columns;
                ChildWidgets.Add(HeaderWidget);
            }
        }

        /// <summary>
        /// Return the row widgets for the panel
        /// </summary>
        public IList RowWidgets
        {
            get { return _rowWidgets; }
        }

        /// <summary>
        /// Override paint to set the clipping rectangle when painting graphics
        /// </summary>
        /// <param name="graphics">The graphics context to paint to</param>
        public override void OnPaint(Graphics graphics)
        {
            DrawingUtilities.SetClip(graphics, _clipBounds);
            base.OnPaint (graphics);
        }

        /// <summary>
        /// Layout the child widgets
        /// </summary>
        public override void OnLayout()
        {

            // Set the default clip bounds if they haven't been set
            //
            if (_clipBounds.IsEmpty)
            {
                _clipBounds = Bounds;
            }

            int headerHeight = 0;

            // layout the header widget
            //
            if (Tree.ShowColumnHeaders)
            {
                headerHeight = Tree.HeaderHeight;
                HeaderWidget.Bounds = new Rectangle(Bounds.X, Bounds.Y, Bounds.Width, headerHeight);
            }

            // layout the row widgets
            //
            Rectangle bounds = new Rectangle(Bounds.X, headerHeight, Bounds.Width, Tree.RowHeight);
            foreach(RowWidget rowWidget in _rowWidgets)
            {
                int adjustedHeight = Tree.GetUserRowHeight(rowWidget.Row);
                bounds.Height = (adjustedHeight == 0) ? rowWidget.RowData.Height : adjustedHeight;
                rowWidget.Bounds = bounds;
                bounds.Y += bounds.Height;
            }
        }

        /// <summary>
        /// Display the context menu when right mouse selected over an empty space
        /// </summary>
        /// <param name="e"></param>
        public override void OnMouseDown(MouseEventArgs e)
        {
            base.OnMouseDown (e);
            if (e.Button == MouseButtons.Right)
            {
                if (Tree.ContextMenuStrip != null)
                {
                    Tree.ContextMenuStrip.Show(Tree, e.X, e.Y);
                }
            }
        }

        /// <summary>
        /// Update the current drop effect
        /// </summary>
        /// <param name="e"></param>
        public override void OnDragEnter(DragEventArgs e)
        {
            e.Effect = Tree.RowDropEffect(null, RowDropLocation.EmptyRowSpace, e.Data);
        }

        /// <summary>
        /// Update the current drop effect
        /// </summary>
        /// <param name="e"></param>
        public override void OnDragOver(DragEventArgs e)
        {
            e.Effect = Tree.RowDropEffect(null, RowDropLocation.EmptyRowSpace, e.Data);
        }

        /// <summary>
        /// Handle dropping of data on the panel
        /// </summary>
        /// <param name="e"></param>
        public override void OnDragDrop(DragEventArgs e)
        {
            e.Effect = Tree.RowDropEffect(null, RowDropLocation.EmptyRowSpace, e.Data);
            if (e.Effect != DragDropEffects.None)
            {
                Tree.OnRowDrop(null, RowDropLocation.EmptyRowSpace, e.Data, e.Effect);
            }
        }

        #endregion

    }

}

