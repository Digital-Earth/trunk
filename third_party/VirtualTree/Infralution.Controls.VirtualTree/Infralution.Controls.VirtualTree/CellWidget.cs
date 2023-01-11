 #region File Header
//
//      FILE:   CellWidget.cs.
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
using System.Drawing.Design;
using System.Windows.Forms;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing.Drawing2D;
using Infralution.Controls;
using Infralution.Common;
using Infralution.RichText;
using NS=Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a <see cref="Widget"/> for displaying a single cell (defined by a <see cref="NS.Row"/> and
    /// <see cref="NS.Column"/>) of a <see cref="VirtualTree"/>. 
    /// </summary>
    /// <remarks>
    /// <para>
    /// This class defines the visual appearance and behaviour of cells within a <see cref="VirtualTree"/>.
    /// The constructor uses the <see cref="VirtualTree.GetCellData">VirtualTree.GetCellData</see> method to 
    /// get the <see cref="NS.CellData"/> which defines the information to be displayed.
    /// </para>
    /// <para>
    /// This class can be extended to customize the appearance and/or behaviour of some or all cells.  To
    /// have the <see cref="NS.VirtualTree"/> use the derived class you must override the 
    /// <see cref="VirtualTree.CreateCellWidget"/> method.
    /// </para>
    /// </remarks>
    /// <seealso cref="NS.VirtualTree"/>
    /// <seealso cref="NS.Row"/>
    /// <seealso cref="NS.Column"/>
    /// <seealso cref="NS.RowWidget"/>
    public class CellWidget : Widget, ITypeDescriptorContext
    {

        #region Member Variables

        /// <summary>
        /// Border used for layout
        /// </summary>
        const int _border = 2;

        /// <summary>
        /// Amount of the cell widget reserved for drawing icon and connections etc 
        /// </summary>
        private int _prefixWidth = 0;

        /// <summary>
        /// The RowWidget this widget is part of
        /// </summary>
        private RowWidget _rowWidget;

        /// <summary>
        /// The column that this cell widget is to display
        /// </summary>
        private Column _column;

        /// <summary>
        /// The active editor control (if any)
        /// </summary>
        private Control _editorControl;

        /// <summary>
        /// The current editor (if any)
        /// </summary>
        private CellEditor _editor;

        /// <summary>
        /// Are we currently editing?
        /// </summary>
        private bool _editing = false;

        /// <summary>
        /// The previous editor control (if any)
        /// </summary>
        private Control _prevEditorControl;

        /// <summary>
        /// The previous editor (if any)
        /// </summary>
        private CellEditor _prevEditor;

        /// <summary>
        /// The cell data to be displayed
        /// </summary>
        private CellData _cellData;

        /// <summary>
        /// Timer used to initiate editing when SelectBeforeEdit is true
        /// </summary>
        private Timer _editTimer;

        /// <summary>
        /// Renderer for RTF text
        /// </summary>
        static private RichTextRenderer _richTextRenderer;

        #endregion

        #region Public Interface

        /// <summary>
        /// Creates a new widget.
        /// </summary>
        /// <param name="rowWidget">The Row Widget this widget belongs to</param>
        /// <param name="column">The column the cell is to display</param>
        public CellWidget(RowWidget rowWidget, Column column)
            : base(rowWidget.WidgetControl)
        {
            _rowWidget = rowWidget;
            _column = column;
        }

        /// <summary>
        /// Return the tree the widget is associated with
        /// </summary>
        public VirtualTree Tree
        {
            get { return _rowWidget.Tree; }
        }

        /// <summary>
        /// The Row Widget this cell widget belongs to
        /// </summary>
        public RowWidget RowWidget
        {
            get { return _rowWidget; }
        }

        /// <summary>
        /// The row the widget is associated with
        /// </summary>
        public Row Row
        {
            get { return _rowWidget.Row; }
        }

        /// <summary>
        /// The column the widget is to display
        /// </summary>
        public Column Column
        {
            get { return _column; }
        }

        /// <summary>
        /// Return the data for this cell widget
        /// </summary>
        /// <remarks>
        /// The cell data is loaded on demand the first time this method is called
        /// </remarks>
        public CellData CellData
        {
            get 
            {
                if (_cellData == null)
                {
                    LoadCellData();
                }
                return _cellData; 
            }
        }

        /// <summary>
        /// Force the CellWidget to update its CellData
        /// </summary>
        public virtual void UpdateData()
        {
            if (_cellData != null)
            {
                LoadCellData();
            }
        }

        /// <summary>
        /// Can this cell be edited
        /// </summary>
        public virtual bool Editable
        {
            get 
            {
                return (CellData.Editor != null);
            }
        }

        /// <summary>
        /// Can this cell be selected
        /// </summary>
        public virtual bool Selectable
        {
            get
            {
                return Column.Selectable && !(RowWidget is SpanningRowWidget);
            }
        }

        /// <summary>
        /// Is this cell currently being edited
        /// </summary>
        public bool Editing
        {
            get { return _editing; }
        }

        /// <summary>
        /// The active editor control (if any)
        /// </summary>
        public Control EditorControl
        {
            get { return _editorControl; }
        }

        /// <summary>
        /// Return the optimal width for this cell (to fit the data)
        /// </summary>
        /// <param name="graphics">The graphics context to use to determine the width</param>
        /// <returns>The optimal width for the cell</returns>
        public virtual int GetOptimalWidth(Graphics graphics)
        {
            int width = EditOffset;
            Style style = GetUnselectedStyle();
            CellData cellData = CellData;
            if (cellData.ShowText)
            {
                int textWidth = 0;
                if (cellData.IsRichTextFormat)
                {
                    textWidth = GetRichTextWidth(graphics);
                }
                else
                {
                    Size size = style.MeasureString(graphics, Text);
                    textWidth = size.Width;
                }
                width += textWidth;
                if (ShowPreview)
                {
                    width += cellData.PreviewSize.Width;
                }
                if (ShowErrorIndicator)
                {
                    width += Tree.ErrorIcon.Width + _border;
                }
            }
            width += 2 * style.BorderWidth + 2;
            width = Math.Max(width, _column.MinWidth); 
            return width;
        }

        /// <summary>
        /// Return the optimal height for this cell for the given width
        /// </summary>
        /// <param name="graphics">The graphics context to use to determine the width</param>
        /// <param name="width">The width available for the cell</param>
        /// <returns>The optimal height for the cell</returns>
        public virtual int GetOptimalHeight(Graphics graphics, int width)
        {
            int height = 0;

            Style style = GetActiveStyle();
            CellData cellData = CellData;
            if (cellData.ShowText)
            {
                if (ShowPreview)
                {
                    height = cellData.PreviewSize.Height;
                    width -= cellData.PreviewSize.Width;
                }
                int textHeight = 0;
                if (cellData.IsRichTextFormat)
                {
                    textHeight = GetRichTextHeight(graphics, width);
                }
                else
                {
                    Size size = style.MeasureString(graphics, Text, width);
                    textHeight = size.Height;
                }
                height = Math.Max(height, textHeight);
            }
            else
            {
                height = cellData.PreviewSize.Height;
            }
            height += 2 * style.BorderWidth + 2;
            return height;
        }

        /// <summary>
        /// Return the optimal height for this cell for its current width
        /// </summary>
        /// <param name="graphics">The graphics context to use to determine the width</param>
        /// <returns>The optimal height for the cell</returns>
        public virtual int GetOptimalHeight(Graphics graphics)
        {
            return GetOptimalHeight(graphics, Column.Width - EditOffset);
        }

        /// <summary>
        /// Return the style to use for unselected cells
        /// </summary>
        public virtual Style GetUnselectedStyle()
        {
            CellData cellData = CellData;
            return ((Row.RowIndex % 2) == 0) ? cellData.EvenStyle : cellData.OddStyle;
        }

        /// <summary>
        /// Return the style to use for selected cells
        /// </summary>
        public virtual Style GetSelectedStyle()
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
        /// Return the style to use based on the current state and selection mode
        /// </summary>
        public virtual Style GetActiveStyle()
        {
            bool showSelected = false;
            switch (Tree.SelectionMode)
            {
                case SelectionMode.FullRow:
                    showSelected = Row.Selected;
                    break;
                case SelectionMode.Cell:
                    Column selectedColumn = Tree.SelectedColumn;
                    showSelected = Row.Selected && (selectedColumn == null || selectedColumn == this.Column);
                    break;
                case SelectionMode.MainCellText:
                    showSelected = Row.Selected && (Column == RowWidget.MainColumn);
                    break;
            }
            return (showSelected) ? GetSelectedStyle() : GetUnselectedStyle();
        }

        /// <summary>
        /// Get the bounds to use for the prefix column and return them
        /// </summary>
        /// <returns>The bounds for the prefix cell widget</returns>
        public virtual Rectangle GetPrefixBounds(Column prefixColumn)
        {
            int prefixOffset = PrefixOffset;
            Rectangle prefixBounds = Bounds;
            prefixBounds.X += prefixOffset;
            prefixBounds.Width -= prefixOffset;
            if (prefixColumn.Width < prefixBounds.Width)
            {
                prefixBounds.Width = prefixColumn.Width;
            }
            return RtlTranslateRect(prefixBounds);
        }

        /// <summary>
        /// Get/Set the width of the cell reserved to display the prefix cell
        /// </summary>
        public int PrefixWidth
        {
            get { return _prefixWidth; }
            set { _prefixWidth = value; }
        }


        #endregion

        #region Local Methods

        /// <summary>
        /// Load or reload the data for this cell
        /// </summary>
        private void LoadCellData()
        {
            VirtualTree tree = Tree;
            CellData newData = tree.CreateCellData(_column);
            tree.OnGetCellData(Row, _column, newData);

            // If there is an active editor (and it is unchanged) then set the cell value in the editor
            //
            if (_cellData != null && newData.Editor == _editor)
            {
                _cellData = newData;
                _editor = _cellData.Editor;
                if (_editorControl != null)
                {
                    _editor.SetValue(this, _editorControl, _cellData.Value);
                }
            }
            else
            {
                // The editor has been changed
                //
                if (_editorControl != null)
                {
                    // Can't dispose of the old control because we might be in an event handler for that
                    // control and so encounter recursion issues.  Instead store a reference to it so we
                    // dispose of it later.  
                    //
                    if (_prevEditorControl != null)
                    {
                        DisposeEditorControl(_prevEditor, _prevEditorControl);
                    }
                    _prevEditor = _editor;
                    _prevEditorControl = _editorControl;
                    _editorControl.Visible = false;
                    _editorControl = null;
                }
                _cellData = newData;
                _editor = _cellData.Editor;

                // if display mode is always or we are currently editing then we need to 
                // recreate the editor
                //
                if (_editor != null)
                {
                    if (_editor.DisplayMode == CellEditorDisplayMode.Always || Editing)
                    {
                        // handle case when dragging and dropping and printing that we
                        // don't have a widget control
                        //
                        if (!RowWidget.PanelWidget.Printing)
                        {
                            CreateEditorControl();
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Return the Rich Text Renderer for this widget
        /// </summary>
        protected RichTextRenderer RichTextRenderer
        {
            get
            {
                if (_richTextRenderer == null)
                {
                    _richTextRenderer = new RichTextRenderer();
                }
                Style style = GetActiveStyle();
                if (style.RightToLeft == RightToLeft.Yes)
                    _richTextRenderer.HorizontalAlignment = StringAlignment.Far;
                else
                    _richTextRenderer.HorizontalAlignment = StringAlignment.Near;
                _richTextRenderer.VerticalAlignment = style.VertAlignment;
                _richTextRenderer.WordWrap = style.WordWrap;
                _richTextRenderer.Font = style.Font;
                _richTextRenderer.TextColor = style.ForeColor;
                _richTextRenderer.Text = Text;
                return _richTextRenderer;
            }
        }

        /// <summary>
        /// Start a timer to initiate an edit on this cell after the given period
        /// </summary>
        protected void StartEditTimer(int interval)
        {
            if (_editTimer == null)
            {
                _editTimer = new Timer();
                _editTimer.Interval = interval;
                _editTimer.Tick += new EventHandler(OnEditTimerTick);
                _editTimer.Start();
            }
        }

        /// <summary>
        /// Stop the current edit timer (if any)
        /// </summary>
        protected void StopEditTimer()
        {
            if (_editTimer != null)
            {
                _editTimer.Stop();
                _editTimer = null;
            }
        }

        /// <summary>
        /// Dispose of editor control if any and any other resources
        /// </summary>
        protected override void Dispose(bool disposing)
        {
            if (_editorControl != null)
            {
                DisposeEditorControl(_editor, _editorControl);
                _editorControl = null;
            }
            if (_prevEditorControl != null)
            {
                DisposeEditorControl(_prevEditor, _prevEditorControl);
                _prevEditorControl = null;
            }
        }


        /// <summary>
        /// Updates the position of the cell editor
        /// </summary>
        public override void OnLayout()
        {
            ChildWidgets.Clear();
            if (this.Column == RowWidget.MainColumn)
            {
                // Add the expansion widget if visible
                //
                if (Row.ShowExpansionIndicator)
                {
                    Point location = new Point(ConnectX(Row.Level - 1), ConnectY);
                    ExpansionWidget expansionWidget = RowWidget.ExpansionWidget;
                    expansionWidget.Location = RtlTranslatePoint(location);
                    Rectangle expBounds = expansionWidget.Bounds;

                    // only add the expansion widget if it is visible and fully 
                    // within the main cell bounds
                    //
                    if (expBounds.Width > 0 &&
                        RtlTranslateRect(Bounds).Contains(expBounds))
                    {
                        ChildWidgets.Add(expansionWidget);
                    }
                }
            }

            if (_editorControl != null)
            {
                LayoutEditor();
            }

            // force the loading of cell data if not already loaded
            //
            if (_cellData == null)
            {
                LoadCellData();
            }
        }

        /// <summary>
        /// Returns true if a preview should be shown for this cell
        /// </summary>
        /// <remarks>
        /// Returns true if <see cref="NS.CellData.ShowPreview"/> is true and that the 
        /// <see cref="NS.CellData.TypeEditor"/> supports painting
        /// </remarks>
        protected virtual bool ShowPreview
        {
            get 
            {
                CellData cellData = CellData;
                return cellData.ShowPreview && cellData.TypeEditor != null &&
                       cellData.TypeEditor.GetPaintValueSupported(this);
            }
        }

        /// <summary>
        /// Returns true if the error indicator should be shown for this cell
        /// </summary>
        /// <remarks>
        /// Returns true if <see cref="NS.CellData.Error"/> is not null or blank
        /// </remarks>
        protected virtual bool ShowErrorIndicator
        {
            get 
            { 
                return CellData.Error != null;
            }
        }


        /// <summary>
        /// Return the offset of the icon in the main column
        /// </summary>
        protected virtual int IconOffset
        {
            get
            {
                return Tree.IndentWidth * Row.Level + Tree.IndentOffset;
            }
        }

        /// <summary>
        /// Return the width reserved for the icon
        /// </summary>
        protected virtual int IconWidth
        {
            get
            {
                RowData rowData = RowWidget.RowData;
                if (rowData.Icon != null)
                {
                    return rowData.IconSize + 1;
                }
                return 0;
            }
        }

        /// <summary>
        /// Return the number of pixels to indent the prefix widget from the row icon
        /// </summary>
        protected virtual int PrefixIndent
        {
            get { return 5; }
        }

        /// <summary>
        /// Return the offset of prefix cell widget within the widget
        /// </summary>
        protected virtual int PrefixOffset
        {
            get
            {
                return IconOffset + IconWidth + PrefixIndent;
            }
        }

        /// <summary>
        /// Return the offset of the edit area within the widget
        /// </summary>
        protected virtual int EditOffset
        {
            get
            {
                if (this.Column == RowWidget.MainColumn)
                {
                    if (PrefixWidth > 0)
                        return PrefixOffset + PrefixWidth;
                    else
                        return IconOffset + IconWidth;
                }
                else
                {
                    return 0;
                }
            }
        }

        /// <summary>
        /// Return the bounds of the CellWidget less the Prefix area used for displaying connections etc
        /// </summary>
        /// <remarks>
        /// Note that the bounds have not been corrected for the <see cref="RightToLeft"/> 
        /// state - you should call <see cref="Widget.RtlTranslateRect(Rectangle)"/>
        /// before using the bounds to draw.
        /// </remarks>
        protected virtual Rectangle GetEditBounds()
        {
            Rectangle bounds = Bounds;
            int editOffset = EditOffset;
            bounds.Width -= editOffset;
            bounds.X += editOffset; 
            return bounds;
        }
 
        /// <summary>
        /// Return the bounds to use for the cell editor control 
        /// </summary>
        /// <remarks>
        /// The returned bounds have been corrected for the <see cref="RightToLeft"/> 
        /// state.  If the cell is for a scrollable column then the bounds are adjusted 
        /// to account for the fact that the control is parented by the clip control 
        /// </remarks>
        protected virtual Rectangle GetRtlEditorBounds()
        {
            Style style = GetUnselectedStyle();
            Rectangle bounds = RtlTranslateRect(style.BorderRect(GetEditBounds()));
            VirtualTree tree = Tree;
            if (!Column.Pinned)
            {
                if (tree.RightToLeft != RightToLeft.Yes)
                    bounds.X -= tree.PinnedPanel.Bounds.Width;
            }
            return bounds;
        }
 
        /// <summary>
        /// Return the bounds of the preview and text regions for the cell
        /// </summary>
        /// <returns>The rectangle to draw the preview and text to</returns>
        /// <remarks>
        /// This excludes the area used to display the error indicator (if any).   Note that the bounds have not been
        /// corrected for the <see cref="RightToLeft"/> state - you should call <see cref="Widget.RtlTranslateRect(Rectangle)"/>
        /// before using the bounds to draw.
        /// </remarks>
        protected virtual Rectangle GetContentBounds()
        {
            Rectangle bounds = GetEditBounds();
            if (ShowErrorIndicator)
            {
                int errorWidth = Tree.ErrorIcon.Width + _border;
                bounds.X += errorWidth;
                bounds.Width -= errorWidth;
            }
            return bounds;
        }

        /// <summary>
        /// Return the bounds of the preview region for the cell
        /// </summary>
        /// <returns>The rectangle to draw the preview to</returns>
        /// <remarks>
        /// Takes into account the specified <see cref="NS.CellData.PreviewSize"/> and whether
        /// <see cref="NS.CellData.ShowText"/> is true.  Note that the bounds have not been
        /// corrected for the <see cref="RightToLeft"/> state - you should call <see cref="Widget.RtlTranslateRect(Rectangle)"/>
        /// before using the bounds to draw.
        /// </remarks>
        protected virtual Rectangle GetPreviewBounds()
        {
            Rectangle contentBounds = GetContentBounds();
            Rectangle bounds = contentBounds;
            CellData cellData = CellData;
            if (cellData.ShowText)
            {
                bounds.Width = Math.Min(contentBounds.Width, cellData.PreviewSize.Width);
            }
            bounds.X += _border;
            bounds.Width -= 2 * _border;
            bounds.Height = Math.Min(bounds.Height - 1, cellData.PreviewSize.Height);
            bounds.Y = bounds.Y + (contentBounds.Height - bounds.Height) / 2;
            return bounds;
        }

        /// <summary>
        /// Return the bounds of the text region for the cell
        /// </summary>
        /// <returns>The rectangle to draw the text to</returns>
        /// <remarks>
        /// Note that the bounds have not been corrected for the <see cref="RightToLeft"/> state 
        /// - you should call <see cref="Widget.RtlTranslateRect(Rectangle)"/> before using the bounds to draw.
        /// </remarks>
        protected virtual Rectangle GetTextBounds()
        {
            Rectangle bounds = GetContentBounds();            
            if (ShowPreview)
            {
                Rectangle previewBounds = GetPreviewBounds();
                bounds.X = previewBounds.Right + _border;
                bounds.Width = bounds.Width - previewBounds.Width - _border;
            }
            return bounds;
        }

        /// <summary>
        /// Return the width of cell text when using RichText
        /// </summary>
        /// <param name="graphics">The graphics context</param>
        /// <returns>The width in pixels</returns>
        protected virtual int GetRichTextWidth(Graphics graphics)
        {
            return RichTextRenderer.GetNaturalWidth(graphics);
        }

        /// <summary>
        /// Return the height of cell text when using RichText
        /// </summary>
        /// <param name="graphics">The graphics context</param>
        /// <param name="width">The width of the cell</param>
        /// <returns>The height in pixels</returns>
        protected virtual int GetRichTextHeight(Graphics graphics, int width)
        {
            return RichTextRenderer.GetNaturalHeight(graphics, width);
        }

        /// <summary>
        /// Paint the error indicator for the cell
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        /// <param name="style">The style to use</param>
        /// <param name="printing">Is it a printing context</param>
        protected virtual void PaintErrorIndicator(Graphics graphics, Style style, bool printing)
        {
            Icon icon = Tree.ErrorIcon;
            Rectangle bounds = GetEditBounds();
            bounds.X += style.BorderWidth;
            bounds.Y += (bounds.Height - icon.Height) / 2;
            bounds.Width = icon.Width;
            bounds.Height = icon.Height;
            bounds = RtlTranslateRect(bounds);
            DrawingUtilities.DrawIcon(graphics, icon, bounds.X, bounds.Y, printing);
        }

        /// <summary>
        /// Paint the preview for the cell
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        /// <param name="style">The style to use</param>
        /// <param name="printing">Is it a printing context</param>
        protected virtual void PaintPreview(Graphics graphics, Style style, bool printing)
        {
            CellData cellData = CellData;
            Rectangle bounds = GetPreviewBounds();
            bounds = RtlTranslateRect(GetPreviewBounds());

            // This piece of magic is needed because of a bug in the standard UITypeEditor for
            // Icons.  The PaintValue method simply ignores the X value of the bounds and draws
            // at zero
            //
            UITypeEditor typeEditor = cellData.TypeEditor;
            if (typeEditor.GetType().FullName == "System.Drawing.Design.IconEditor")
            {
                graphics.TranslateTransform(bounds.Left, 0);
                bounds.X = 0;
            }
            PaintValueEventArgs pve = new PaintValueEventArgs(this, cellData.Value, graphics, bounds);
            typeEditor.PaintValue(pve);
            if (cellData.DrawPreviewBorder)
            {
                using (Pen pen = new Pen(style.ForeColor))
                {
                    graphics.DrawRectangle(pen, bounds);
                }
            }
            
            if (bounds.X == 0)
            {
                graphics.ResetTransform();
            }
        }

        /// <summary>
        /// Return the x coordinate to draw the vertical connection for the given indent level
        /// </summary>
        /// <param name="indentLevel">The level of the connection within the tree heirarchy</param>
        /// <returns>The x location to paint at</returns>
        protected virtual int ConnectX(int indentLevel)
        {
            return Bounds.X + (indentLevel * Tree.IndentWidth) + 8;
        }

        /// <summary>
        /// Return the y coordinate to draw the horizontal connection at
        /// </summary>
        protected virtual int ConnectY
        {
            get
            {
                Style style = this.GetActiveStyle();
                int y = Bounds.Y;
                switch (style.VertAlignment)
                {
                    case StringAlignment.Near:
                        y += RowWidget.RowData.IconSize / 2;
                        break;
                    case StringAlignment.Far:
                        y += Bounds.Height - RowWidget.RowData.IconSize / 2;
                        break;
                    case StringAlignment.Center:
                        y += Bounds.Height / 2;
                        break;
                }
                return y;
            }
        }

        /// <summary>
        /// Paint the vertical connections for the given row.  Recursively paints the 
        /// vertical child connections for the given row.
        /// </summary>
        /// <param name="graphics">The graphics context to use</param>
        /// <param name="pen">The pen to use to draw the connections</param>
        /// <param name="row">The row to paint connections for</param>
        protected virtual void PaintVerticalConnections(Graphics graphics, Pen pen, Row row)
        {
            int topIndex = Tree.TopRowIndex;

            // if this is not the root row
            //
            if (row.ParentRow != null)
            {
                // recursively paint the connections for the rows ancestors
                //
                PaintVerticalConnections(graphics, pen, row.ParentRow);
            }

            // paint the vertical connection from this row to its direct expanded children
            //
            if (row.Expanded && row.NumChildren > 0 && row.RowIndex >= Tree.FirstRowIndex)
            {
                if (row.LastChildRowIndex > Row.RowIndex)
                {
                    int x = RtlTranslateX(ConnectX(row.Level));
                    graphics.DrawLine(pen, x, Bounds.Top, x, Bounds.Bottom);
                }
                else if (row.LastChildRowIndex == Row.RowIndex)
                {
                    int x = RtlTranslateX(ConnectX(row.Level));
                    graphics.DrawLine(pen, x, Bounds.Top, x, ConnectY);
                }
            }
        }

        /// <summary>
        /// Create the pen used to draw the lines between connections
        /// </summary>
        /// <returns></returns>
        protected virtual Pen CreateConnectionPen()
        {
            Pen pen = null;
            switch (Tree.LineStyle)
            {
                case LineStyle.None:
                    break;
                case LineStyle.Solid:
                    pen = new Pen(Tree.LineColor);
                    break;
                case LineStyle.Dot:
                    pen = new Pen(Tree.LineColor);
                    pen.DashStyle = DashStyle.Dot;
                    break;
                case LineStyle.Dash:
                    pen = new Pen(Tree.LineColor);
                    pen.DashStyle = DashStyle.Dash;
                    break;
                case LineStyle.DashDot:
                    pen = new Pen(Tree.LineColor);
                    pen.DashStyle = DashStyle.DashDot;
                    break;
                case LineStyle.DashDotDot:
                    pen = new Pen(Tree.LineColor);
                    pen.DashStyle = DashStyle.DashDotDot;
                    break;
            }
            return pen;
        }

        /// <summary>
        /// Paint the connections for this row
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        /// <param name="printing">Is the context a printer context</param>
        protected virtual void PaintConnections(Graphics graphics, bool printing)
        {
            Pen pen = CreateConnectionPen();
            if (pen == null) return;


            // draw the horizontal connector to the parent
            //
            if (Row.Level > 0)
            {
                int topLevel = (Tree.ShowRootRow) ? 0 : 1;
                if (Row.Level > topLevel || Row.ShowExpansionIndicator)
                {
                    int y = ConnectY;
                    int x1 = ConnectX(Row.Level - 1);
                    int x2 = x1 + Tree.IndentWidth + Tree.IndentOffset - 8;
                    graphics.DrawLine(pen, RtlTranslateX(x1), y, RtlTranslateX(x2), y);
                }
            }
            if (Row.ParentRow != null)
            {
                PaintVerticalConnections(graphics, pen, Row.ParentRow);
            }

            pen.Dispose();
        }

        /// <summary>
        /// Paint the row icon at the given location
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        /// <param name="printing">Is it a printing context?</param>
        protected virtual void PaintIcon(Graphics graphics, bool printing)
        {
            RowData rowData = RowWidget.RowData;
            Icon icon = (Row.Expanded) ? rowData.GetExpandedIcon() : rowData.GetIcon();
            if (icon != null)
            {
                int y = ConnectY - icon.Height / 2;
                int x = RtlTranslateX(Bounds.X + IconOffset);
                if (WidgetControl.RightToLeft == RightToLeft.Yes)
                {
                    x -= icon.Width;
                }
                DrawingUtilities.DrawIcon(graphics, icon, x, y, printing);
            }
        }

        /// <summary>
        /// Set the clipping region of the given graphics context to restrict all drawing
        /// within this column
        /// </summary>
        /// <param name="graphics">The context to set the clipping region for</param>
        /// <remarks>
        /// This is used when drawing connections and icons for the tree
        /// </remarks>
        protected virtual void SetColumnClip(Graphics graphics)
        {
            Rectangle clipBounds = Bounds;
            clipBounds.Intersect(RowWidget.PanelClipBounds());
            DrawingUtilities.SetClip(graphics, clipBounds);
        }

        /// <summary>
        /// Return the string representation of the CellDataValue
        /// </summary>
        protected virtual string Text
        {
            get 
            {
                return CellData.GetText();
            }
        }

        /// <summary>
        /// Return the actual bounds of the given text - accounting for alignment and text length
        /// </summary>
        /// <param name="graphics">The graphics context</param>
        /// <param name="bounds">The rectangle that the text is drawn into</param>
        /// <param name="style">The current style</param>
        /// <param name="text">The text to be drawn</param>
        /// <remarks>
        /// <see cref="GetTextBounds"/> returns the area of the cell that the text is drawn into.  This method
        /// returns the area within that rectangle that the text actually occupies
        /// </remarks>
        protected virtual Rectangle GetActualTextBounds(Graphics graphics, Rectangle bounds, Style style, String text)
        {
            int width = style.BorderWidth * 2;
            if (CellData.IsRichTextFormat)
            {
                width = GetRichTextWidth(graphics);
            }
            else
            {
                Size size = style.MeasureString(graphics, text, bounds.Width);
                width = size.Width + 1;  // allow some additional space - required for "/" characters
            }
            width = Math.Min(width, bounds.Width);
            Rectangle result = new Rectangle(bounds.X, bounds.Y, width,  bounds.Height);
            switch (style.HorzAlignment)
            {
                case StringAlignment.Center:
                    result.X = bounds.X + (bounds.Width - width) / 2;
                    break;
                case StringAlignment.Far:
                    result.X = bounds.Right - width;
                    break;
            }
            result = RtlTranslateRect(result);
            return result;
        }

        /// <summary>
        /// Paint the background for selected cell text when the <see cref="SelectionMode"/> is set to 
        /// <see cref="SelectionMode.MainCellText"/> 
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="bounds">The bounds of text</param>
        /// <param name="style">The style to use</param>
        protected virtual void PaintSelectedTextBackground(Graphics graphics, Rectangle bounds, Style style)
        {
            style.DrawBackground(graphics, bounds);
            style.DrawBorder(graphics, bounds);
        }

        /// <summary>
        /// Paint the rich text to the given context
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        /// <param name="bounds">The bounds to paint the text in</param>
        protected virtual void PaintRichText(Graphics graphics, Rectangle bounds)
        {
            RichTextRenderer.Draw(graphics, bounds);
        }

        /// <summary>
        /// Paint the text for the cell
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        /// <param name="bounds">The rectangle to paint the text with</param>
        /// <param name="style">The style to use</param>
        /// <param name="text">The text to paint</param>
        /// <param name="printing">Is the context a printer context</param>
        protected virtual void PaintText(Graphics graphics, Rectangle bounds, Style style, string text, bool printing)
        {
            bounds = RtlTranslateRect(bounds);
            if (CellData.IsRichTextFormat)
            {
                PaintRichText(graphics, bounds);
            }
            else
            {
                style.DrawString(graphics, text, bounds, printing);
            }
        }

        /// <summary>
        /// Paint the foreground of the cell
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        /// <param name="style">The style to use</param>
        /// <param name="printing">Is it a printing context</param>
        protected virtual void PaintForeground(Graphics graphics, Style style, bool printing)
        {
            if (ShowErrorIndicator)
            {
                PaintErrorIndicator(graphics, style, printing);
            }           
            if (ShowPreview)
            {
                PaintPreview(graphics, style, printing);
            }
            if (CellData.ShowText)
            {
                string text = Text;
                Rectangle textBounds = GetTextBounds();

                if (Tree.SelectionMode == SelectionMode.MainCellText &&
                    RowWidget.MainColumn == Column && !printing)
                {
                    if (Row.Selected || Row.HasFocus)
                    {
                        Rectangle selectBounds = GetActualTextBounds(graphics, textBounds, style, text);
                        selectBounds.Inflate(style.BorderWidth, 0);
                        if (Row.Selected)
                        {
                            PaintSelectedTextBackground(graphics, selectBounds, style);
                        }
                        if (Row.HasFocus && Tree.ShowFocusIndicator)
                        {
                            Tree.PaintFocusIndicator(graphics, style, selectBounds);
                        }
                    }
                }
                PaintText(graphics, textBounds, style, text, printing);
            }
        }

        /// <summary>
        /// Paint the background and border of the cell
        /// </summary>
        /// <remarks>
        /// The background for the cell is only painted if the BackColor is different to that of the row
        /// </remarks>
        /// <param name="graphics">The context to paint to</param>
        /// <param name="rowStyle">The style of the parent row</param>
        /// <param name="cellStyle">The style for this cell</param>
        /// <param name="printing">Is it a printing context</param>
        protected virtual void PaintBackground(Graphics graphics, 
                                               Style rowStyle, 
                                               Style cellStyle, 
                                               bool printing)
        {
            Rectangle bounds = Bounds; 
            if (rowStyle.BackColor != cellStyle.BackColor)
            {
                cellStyle.DrawBackground(graphics, bounds);
            }

            // ensure the border is shown when on the edge of the pinned panel
            //
            if (rowStyle.BorderStyle == Border3DStyle.Adjust && Column.Pinned)
            {
                if (bounds.Right >= RowWidget.Bounds.Right && Tree.HorzScrollOffset > 0)
                {
                    bounds.Width--;
                }
            }

            // only draw the cell border if the row is not selected 
            //
            bool rowSelected = Row.Selected && (Tree.SelectedColumn == null);
            if (!rowSelected)
            {
                cellStyle.DrawBorder(graphics, bounds);
            }
        }

        /// <summary>
        /// Handle painting for this widget
        /// </summary>
        /// <param name="graphics"></param>
        public override void OnPaint(Graphics graphics)
        {
            Style rowStyle = RowWidget.GetActiveStyle();
            Style foreStyle = GetActiveStyle();
            Style backStyle = foreStyle;

            // if selection mode is main cell text then always use the unselected style
            // for the background
            //
            if (Tree.SelectionMode == SelectionMode.MainCellText)
            {
                backStyle = GetUnselectedStyle();
            }

            // don't paint the background for prefix columns
            //
            if (!Column.PrefixColumn)
            {
                PaintBackground(graphics, rowStyle, backStyle, false);
            }

            // if this is the main column then paint the icon and connections
            //
            if (this.Column == RowWidget.MainColumn)
            {
                // set the clipping region to the width of this column - this ensures
                // we don't draw icons outside of the column
                //
                SetColumnClip(graphics);
                PaintConnections(graphics, false);
                PaintIcon(graphics, false);

                // reset the clip region for the row
                //
                RowWidget.SetRowClip(graphics); 
            }

            // only paint the foreground if the editor control is not shown
            //
            if (_editorControl == null || !_editorControl.Visible)
            {
                PaintForeground(graphics, foreStyle, false);   
            }
            else
            {
                if (Editing)
                    foreStyle = GetUnselectedStyle();
                _editor.SetControlStyle(_editorControl, foreStyle);      
            }

            // Paint the child widgets if any (expansion widget) 
            //
            base.OnPaint(graphics);

        }

        /// <summary>
        /// Handle printing for this widget
        /// </summary>
        /// <param name="graphics">The graphics context to print to</param>
        public override void OnPrint(Graphics graphics)
        {
            Style rowStyle, cellStyle;
            CellData cellData = CellData;
            if ((Row.RowIndex % 2) == 0)
            {
                rowStyle = RowWidget.RowData.PrintEvenStyle;
                cellStyle = cellData.PrintEvenStyle;
            }
            else
            {
                rowStyle = RowWidget.RowData.PrintOddStyle;
                cellStyle = cellData.PrintOddStyle;
            }

            // don't paint the background for prefix columns
            //
            if (!Column.PrefixColumn)
            {
                PaintBackground(graphics, rowStyle, cellStyle, true);
            }

            // if this is the main column then print the icon and connections
            //
            if (this.Column == RowWidget.MainColumn)
            {
                // set the clipping region to the width of this column - this ensures
                // we don't draw icons outside of the column
                //
                SetColumnClip(graphics);
                PaintConnections(graphics, true);
                PaintIcon(graphics, true);

                // reset the clip region for the row
                //
                RowWidget.SetRowClip(graphics);
            }

            PaintForeground(graphics, cellStyle, true);

            // Paint the child widgets if any (expansion widget) 
            //
            base.OnPrint(graphics);
        }

        /// <summary>
        /// Return true if the given text fits in the given rectangle
        /// </summary>
        /// <param name="graphics">The graphics context</param>
        /// <param name="style">The style used to draw the text</param>
        /// <param name="bounds">The bounds of the rectangle</param>
        /// <param name="text">The text to be displayed</param>
        /// <returns>True if the text fits without clipping</returns>
        protected virtual bool TextFits(Graphics graphics, Style style, Rectangle bounds, string text)
        {
            if (CellData.IsRichTextFormat)
            {
                int height = GetRichTextHeight(graphics, bounds.Width);
                return height < bounds.Height;
            }
            else
            {
                return style.StringFits(graphics, text, bounds);
            }
        }

        /// <summary>
        /// Return the tooltip text to display for this cell (if any)
        /// </summary>
        /// <returns>The tooltip to display or null if no tooltip should be displayed</returns>
        protected virtual string GetToolTipText()
        {
            CellData cellData = CellData;

            // give the user a chance to dynamically set the tooltip
            //
            Tree.OnGetToolTipCellData(Row, Column, cellData);

            if (cellData.Error != null)
            {
                return cellData.GetErrorText();
            }

            String tooltip = null;
            if (cellData.AlwaysDisplayToolTip)
            {
                tooltip = cellData.GetToolTipText();
            }
            else
            {
                using (Graphics graphics = WidgetControl.CreateGraphics())
                {
                    if (!TextFits(graphics, GetActiveStyle(), GetTextBounds(), Text))
                    {
                        tooltip = cellData.GetToolTipText();
                    }
                }
            }
            return tooltip;
        }

        /// <summary>
        /// Called when the mouse is first moved over the widget
        /// </summary>
        public override void OnMouseEnter(MouseEventArgs e)
        {
            string toolTipText = GetToolTipText();
            if (toolTipText != null)
            {
                Tree.ShowToolTip(toolTipText);
            }
        }

        /// <summary>
        /// Called when the mouse is leaves the widget
        /// </summary>
        public override void OnMouseLeave(EventArgs e)
        {
            Tree.HideToolTip();
        }

        /// <summary>
        /// Handle the edit timer (when <see cref="VirtualTree.SelectBeforeEdit"/> is true)
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnEditTimerTick(object sender, EventArgs e)
        {
            _editTimer.Stop();
            _editTimer = null;
            if (!WidgetControl.MouseDragging)
            {
                Tree.StartEdit(this);
            }
        }

        /// <summary>
        /// Handle a left mouse down event.
        /// </summary>
        /// <param name="e">The mouse event details.</param>
        protected virtual void OnLeftMouseDown(MouseEventArgs e)
        {
            // remember if the row/cell was selected before calling RowWidget.OnMouseDown 
            // which will select it
            //
            bool preSelected = Row.Selected && (Tree.SelectedRows.Count == 1);
            if (Tree.SelectionMode == SelectionMode.Cell && Column.Selectable)
            {
                preSelected &= (Tree.SelectedColumn == Column);
            }

            RowWidget.OnLeftMouseDown(e, Tree.EnableDragSelect);
            if (Tree.SelectionMode == SelectionMode.Cell)
            {
                Tree.SelectedColumn = (Selectable) ? Column : null;
            }
            if (Editable)
            {
                if (Tree.SelectBeforeEdit)
                {
                    if (preSelected)
                    {
                        StartEditTimer(SystemInformation.DoubleClickTime);
                    }
                }
                else
                {
                    Tree.StartEdit(this);
                }
            }
        }

        /// <summary>
        /// Handle a right mouse down event
        /// </summary>
        /// <param name="e">The mouse event details.</param>
        protected virtual void OnRightMouseDown(MouseEventArgs e)
        {
            if (Tree.SelectionMode == SelectionMode.Cell)
            {
                Tree.SelectedColumn = (Selectable) ? Column : null;
            }
            RowWidget.OnRightMouseDown(e);
        }

        /// <summary>
        /// Handle MouseDown events for this Widget
        /// </summary>
        /// <param name="e"></param>
        public override void OnMouseDown(MouseEventArgs e)
        {
            Tree.CompleteEdit();
            Tree.OnCellMouseDown(this, e);
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
            Tree.OnCellMouseUp(this, e);
            RowWidget.OnMouseUp(e);
        }

        /// <summary>
        /// Handle MouseMove events for this Widget
        /// </summary>
        /// <param name="e"></param>
        public override void OnMouseMove(MouseEventArgs e)
        {
            RowWidget.OnMouseMove(e);
        }

        /// <summary>
        /// Handle Double Click events for this Widget.
        /// </summary>
        /// <param name="e"></param>
        public override void OnDoubleClick(EventArgs e)
        {
            StopEditTimer();
            Tree.OnCellDoubleClick(this, e);
            if (Editable && Tree.EditOnDoubleClick)
            {
                Tree.StartEdit(this);
            }
            else
            {
                RowWidget.OnDoubleClick(e);
            }
        }

        /// <summary>
        /// Handle Click events for this Widget.
        /// </summary>
        /// <param name="e"></param>
        public override void OnClick(EventArgs e)
        {
            Tree.OnCellClick(this, e);
        }

        /// <summary>
        /// Handle dragging of another row over this row
        /// </summary>
        /// <param name="e"></param>
        public override void OnDragEnter(DragEventArgs e)
        {
            RowWidget.OnDragEnter(e);
        }

        /// <summary>
        /// Handle drag leaving this widget
        /// </summary>
        /// <param name="e"></param>
        public override void OnDragLeave(EventArgs e)
        {
            RowWidget.OnDragLeave(e);
        }

        /// <summary>
        /// Handle drag over this widget
        /// </summary>
        /// <param name="e"></param>
        public override void OnDragOver(DragEventArgs e)
        {
            RowWidget.OnDragOver(e);
        }


        /// <summary>
        /// Handle dropping another row on this row
        /// </summary>
        /// <param name="e"></param>
        public override void OnDragDrop(DragEventArgs e)
        {
            RowWidget.OnDragDrop(e);
        }

        /// <summary>
        /// Creates the editor control for this widget
        /// </summary>
        protected virtual void CreateEditorControl()
        {       
            Debug.Assert(_editor != null);
            Debug.Assert(_editorControl == null);
            CellData cellData = CellData;
            _editorControl = _editor.GetControl(this);
            
            // if the control is a universal edit box then set the
            // type editor and converter to use
            //
            if (_editorControl is UniversalEditBox)
            {
                UniversalEditBox editBox = _editorControl as UniversalEditBox;
                editBox.ValueOwner = Row.Item;
                editBox.Converter = cellData.TypeConverter;
                editBox.Editor = cellData.TypeEditor;
            }
            _editor.SetValue(this, _editorControl, cellData.Value);
            _editorControl.GotFocus += new EventHandler(OnEditorGotFocus);
            _editor.AddUpdateValueHandler(_editorControl, new EventHandler(OnEditorValueChanged));
            _editorControl.Visible = false;
            _editorControl.TabStop = false;
        }

        /// <summary>
        /// Disposes of the current editor control
        /// </summary>
        protected virtual void DisposeEditorControl(CellEditor editor, Control control)
        {
            Debug.Assert(control != null);
            Debug.Assert(editor != null);

            // remove event handlers
            //
            control.GotFocus -= new EventHandler(OnEditorGotFocus);
            editor.RemoveUpdateValueHandler(control, new EventHandler(OnEditorValueChanged));

            // remove the control from the parent
            //
            control.Visible = false;
            control.Parent = null;

            // release the editor control back to the cache
            //
            editor.FreeControl(control);
        }

        /// <summary>
        /// Update the size and position of the <see cref="CellEditor"/> <see cref="Control"/> to match this 
        /// cell widget.
        /// </summary>
        protected virtual void LayoutEditor()
        {
            if (_editorControl != null) 
            {
                bool showControl = Row.Visible && Column.Visible && Bounds != Rectangle.Empty;
                if (_editor.DisplayMode == CellEditorDisplayMode.OnEdit)
                {
                    showControl = showControl && Editing;
                }

                // ensure the editors RightToLeft matches the cell
                //
                _editorControl.RightToLeft = GetActiveStyle().RightToLeft;
                if (showControl)
                {
                    if (!Tree.EditorLayoutSuspended)
                    {
                        Tree.ManageEditorControl(this, _editorControl);
                        _editor.LayoutControl(_editorControl, GetRtlEditorBounds(), showControl);
                    }
                }
                else
                {
                    _editor.LayoutControl(_editorControl, GetRtlEditorBounds(), showControl);
                }
            }
        }

        /// <summary>
        /// Handle a change to the value being edited
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnEditorValueChanged(object sender, System.EventArgs e)
        {
            // check that the editor has not been removed
            //
            if (_editor != null)
            {
                CellData cellData = CellData;
                object newValue = _editor.GetValue(this, _editorControl);
                if (ValueChanged(cellData.Value, newValue))
                {
                    try
                    {
                        if (Tree.SetValueForCell(Row, Column, cellData.Value, newValue))
                        {
                            // update the data for the cell
                            //
                            Tree.OnGetCellData(Row, _column, cellData);

                            // changing the cell value may change the sort order - so lets keep this cell
                            // visible
                            //
                            Row.EnsureVisible();
                        }
                        else
                        {
                            // reset the value displayed in the editor to the original value
                            //
                            _editor.SetValue(this, _editorControl, cellData.Value);
                        }
                    }
                    catch
                    {
                        // reset the value displayed in the editor to the original value before passing the exception upwards
                        //
                        _editor.SetValue(this, _editorControl, cellData.Value);
                        throw;
                    }
                }
            }
        }

        /// <summary>
        /// Ensure that this widget is the current EditWidget when the editor control
        /// receives focus
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnEditorGotFocus(object sender, EventArgs e)
        {
            Tree.StartEdit(this);
        }

        /// <summary>
        /// Determine if an edited value has changed 
        /// </summary>
        /// <param name="oldValue">The old Value</param>
        /// <param name="newValue">The new Value</param>
        /// <returns>True if the values are different</returns>
        private bool ValueChanged(object oldValue, object newValue)
        {
            if (oldValue == null) return (newValue != null);
            return !oldValue.Equals(newValue);
        }

        /// <summary>
        /// Called by VirtualTree to start editing this cell
        /// </summary>
        public virtual void StartEdit()
        {
            if (_editor != null && !RowWidget.PanelWidget.Printing)
            {
                if (_editorControl == null)
                {
                    CreateEditorControl();
                }
                _editing = true;
                LayoutEditor();
                Focus();
            }
        }

        /// <summary>
        /// Should the Virtual Tree ensure CellWidget is fully visible when it gets focus
        /// </summary>
        public bool EnsureVisibleOnFocus
        {
            get { return (_editor == null) ? false : _editor.EnsureVisibleOnFocus; }
        }

        /// <summary>
        /// Set focus to the Cell Editor (if present)
        /// </summary>
        /// <returns>True if successful</returns>
        public virtual bool Focus()
        {
            if (_editorControl != null)
            {
                if (_editorControl.ContainsFocus) return true;
                return _editorControl.Focus();
            }
            return false;
        }

        /// <summary>
        /// Does the CellEditor control (or one of its children) currently have focus
        /// </summary>
        /// <returns></returns>
        public virtual bool ContainsFocus()
        {
            if (_editorControl != null)
            {
                return _editorControl.ContainsFocus;
            }
            return false;
        }

        /// <summary>
        /// Called by VirtualTree to complete editing this cell
        /// </summary>
        public virtual void CompleteEdit()
        {
            if (_editorControl != null)
            {
                if (_editor.DisplayMode == CellEditorDisplayMode.OnEdit)
                {
                    _editorControl.Visible = false;
                }
            }
            _editing = false;
        }

        /// <summary>
        /// Call by VirtualTree to abandon editing this cell
        /// </summary>
        public virtual void AbandonEdit()
        {
            if (_editorControl != null)
            {
                _editor.SetValue(this, _editorControl, CellData.Value);
                if (_editor.DisplayMode == CellEditorDisplayMode.OnEdit)
                {
                    _editorControl.Visible = false;
                }
            }
            _editing = false;
        }

        #endregion

        #region ITypeDescriptorContext Members

        /// <summary>
        /// 
        /// </summary>
        void ITypeDescriptorContext.OnComponentChanged()
        {
        }

        /// <summary>
        /// Returns the container for the sited control
        /// </summary>
        IContainer ITypeDescriptorContext.Container
        {
            get { return this.Tree.Container; }
        }

        /// <summary>
        /// Always returns true
        /// </summary>
        bool ITypeDescriptorContext.OnComponentChanging()
        {
            return true;
        }

        /// <summary>
        /// Return the object that owns the value being edited
        /// </summary>
        object ITypeDescriptorContext.Instance
        {
            get { return Row.Item; }
        }

        /// <summary>
        /// Always returns null
        /// </summary>
        PropertyDescriptor ITypeDescriptorContext.PropertyDescriptor
        {
            get { return null; }
        }

        /// <summary>
        /// If the tree is sited returns then calls through
        /// </summary>
        /// <param name="serviceType"></param>
        /// <returns></returns>
        object IServiceProvider.GetService(Type serviceType)
        {
            return (Tree.Site == null) ? null : Tree.Site.GetService(serviceType);
        }

        #endregion

 
    }

}
