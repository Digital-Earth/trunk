#region File Header
//
//      FILE:   ColumnHeaderWidget.cs.
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
using System.Windows.Forms;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Drawing.Imaging;
using System.Windows.Forms.VisualStyles;
using NS=Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a <see cref="Widget"/> for displaying the header information for a <see cref="NS.Column"/> of a 
    /// <see cref="VirtualTree"/> 
    /// </summary>
    /// <remarks>
    /// <para>
    /// This class defines the visual appearance and behaviour of the column headers for a <see cref="VirtualTree"/>.
    /// The widget supports:
    /// <list type = "bullet">
    /// <item>left click to sort the column</item>
    /// <item>drag and drop to move the column</item>
    /// <item>resizing columns using the <see cref="NS.ColumnDividerWidget"/></item>
    /// <item>right click to display the column menu</item>
    /// </list>
    /// </para>
    /// <para>
    /// This class can be extended to customize the appearance and/or behaviour of column headers.  To
    /// have the <see cref="NS.VirtualTree"/> use the derived class you must override the 
    /// <see cref="VirtualTree.CreateColumnHeaderWidget"/> method.
    /// </para>
    /// </remarks>
    /// <seealso cref="NS.VirtualTree"/>
    /// <seealso cref="NS.Column"/>
    /// <seealso cref="NS.HeaderWidget"/>
    /// <seealso cref="NS.ColumnDividerWidget"/>
    public class ColumnHeaderWidget : Widget
    {
        #region Member Variables

        /// <summary>
        /// The column the widget is associated with
        /// </summary>
        private Column _column;

        /// <summary>
        /// Is the mouse currently hovering
        /// </summary>
        private bool _hot = false;

        /// <summary>
        /// Is the column header currently pressed
        /// </summary>
        private bool _pressed = false;

        /// <summary>
        /// Is the column header currently being dragged
        /// </summary>
        private bool _dragging = false;

        /// <summary>
        /// Is this a potential drop site 
        /// </summary>
        private bool _dropping = false;

        /// <summary>
        /// The column divider widget (if any)
        /// </summary>
        private ColumnDividerWidget _dividerWidget;

        /// <summary>
        /// The bounds that the text was drawn to
        /// </summary>
        private Rectangle _textBounds;

        /// <summary>
        /// Overlay cursor for dragging column headers
        /// </summary>
        private static Cursor _arrowCursor;

        #endregion

        #region Public Interface

        /// <summary>
        /// Creates a new widget.
        /// </summary>
        /// <param name="parentWidget">The parent widget this widget belongs to</param>
        /// <param name="column">The column the widget is to display</param>
        public ColumnHeaderWidget(Widget parentWidget, Column column)
            : base(parentWidget)
        {
            _column = column;
            _dividerWidget = Tree.CreateColumnDividerWidget(this);
        }

        /// <summary>
        /// Return the tree the widget is associated with
        /// </summary>
        public VirtualTree Tree
        {
            get { return Column.Tree; }
        }

        /// <summary>
        /// The column the widget is associated with
        /// </summary>
        public Column Column
        {
            get { return _column; }
        }
 
        /// <summary>
        /// The column divider widget (if present)
        /// </summary>
        public ColumnDividerWidget DividerWidget
        {
            get { return _dividerWidget; }
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Paint the background using XP Theme styles
        /// </summary>
        /// <param name="graphics"></param>
        protected virtual void PaintThemedBackground(Graphics graphics)
        {
            VisualStyleElement element;
            if (Pressed)
                element = VisualStyleElement.Header.Item.Normal;
            else if (Hot || Dropping)
                element = VisualStyleElement.Header.Item.Hot;
            else
                element = VisualStyleElement.Header.Item.Normal;

            VisualStyleRenderer renderer = new VisualStyleRenderer(element);
            DrawingUtilities.DrawThemeBackground(WidgetControl, graphics, renderer, Bounds, RightToLeft);
        }

        /// <summary>
        /// Paint the background using standard .NET border styles
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="style">The style parameters to use</param>
        /// <param name="printing">Is it a printing context</param>
        protected virtual void PaintBackground(Graphics graphics, Style style, bool printing)
        {
            Rectangle bounds = Bounds;
            style.DrawBackground(graphics, bounds);

            // ensure the border is shown when on the edge of the pinned panel
            //
            if (style.BorderStyle == Border3DStyle.Adjust && Column.Pinned && ParentWidget != null)
            {
                if (bounds.Right >= ParentWidget.Bounds.Right && Tree.HorzScrollOffset > 0)
                {
                    bounds.Width--;
                }
            }
            style.DrawBorder(graphics, bounds);
        }

        /// <summary>
        /// Paint the foreground of the column header
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="style">The style parameters to use</param>
        /// <param name="printing">Is it a printing context</param>
        protected virtual void PaintForeground(Graphics graphics, Style style, bool printing)
        {
            int border = style.BorderWidth + 1;
            Rectangle bounds = Bounds;
            Icon icon;

            if (Column.Pinned && Column.ShowPinIcon && Column.Active && !printing)
            {
                icon = Tree.PinIcon;
                int iconWidth = icon.Width + border;
                if (iconWidth < Bounds.Width)
                {
                    Rectangle iconBounds = new Rectangle(bounds.X + border + 1, bounds.Y, icon.Width, icon.Height);
                    iconBounds.Y += (bounds.Height - icon.Height) / 2;
                    iconBounds = RtlTranslateRect(iconBounds);
                    DrawingUtilities.DrawIcon(graphics, icon, iconBounds.X, iconBounds.Y, printing);
                    bounds.X += iconWidth + 1;
                    bounds.Width -= iconWidth;
                }
            }

            icon = Column.Icon;
            if (icon != null)
            {
                int iconWidth = icon.Width + border;
                if (bounds.X + iconWidth < Bounds.Right)
                {
                    Rectangle iconBounds = new Rectangle(bounds.X + border, bounds.Y, icon.Width, icon.Height);
                    iconBounds.Y += (bounds.Height - icon.Height) / 2;
                    iconBounds = RtlTranslateRect(iconBounds);
                    DrawingUtilities.DrawIcon(graphics, icon, iconBounds.X, iconBounds.Y, printing);
                    bounds.X += iconWidth;
                    bounds.Width -= iconWidth;
                }
            }
         
            if (Column.Active && Tree.SortColumn == Column)
            {
                icon = (Column.SortDirection == ListSortDirection.Ascending) ?
                    Tree.SortAscendingIcon : Tree.SortDescendingIcon;
                
                Rectangle iconBounds = new Rectangle(bounds.X, bounds.Y, icon.Width, icon.Height);                
                iconBounds.Y += (bounds.Height - icon.Height) / 2;
                iconBounds.X = bounds.Right - border - icon.Width - 1;
                
                // only draw the sort icon if it will fit in the available width
                //
                if (iconBounds.X > bounds.X && !printing)
                {
                    iconBounds = RtlTranslateRect(iconBounds);
                    DrawingUtilities.DrawIcon(graphics, icon, iconBounds.X, iconBounds.Y, printing);
                    bounds.Width -= (icon.Width + border);
                }
            }
            _textBounds = bounds;
            style.DrawString(graphics, Column.Caption, RtlTranslateRect(_textBounds), printing);

        }

        /// <summary>
        /// Update the location of the divider widget (if present)
        /// </summary>
        public override void OnLayout()
        {
            // only add the divider widget if the column is active
            //
            ChildWidgets.Clear();
            if (Column.Active && Column.Resizable && Column.AutoSizePolicy != ColumnAutoSizePolicy.AutoSize)
            {
                const int dividerWidth = 4;
                Rectangle bounds = new Rectangle(Bounds.Right - dividerWidth, Bounds.Top, 
                                                 2 * dividerWidth, Bounds.Height);
                _dividerWidget.Bounds = RtlTranslateRect(bounds);
                ChildWidgets.Add(_dividerWidget);
            }
        }

        /// <summary>
        /// Handle painting for this widget
        /// </summary>
        /// <param name="graphics"></param>
        public override void OnPaint(Graphics graphics)
        {
            Style style;

            if (Pressed)
                style = Column.HeaderPressedStyle;
            else if (Dropping)
                style = Column.HeaderDropStyle;
            else if (Hot)
                style = Column.HeaderHotStyle;
            else
                style = Column.HeaderStyle;

            if (Tree.UseThemedHeaders)
                PaintThemedBackground(graphics);
            else
                PaintBackground(graphics, style, false);
            PaintForeground(graphics, style, false);

        }

        /// <summary>
        /// Handle printing for this widget
        /// </summary>
        /// <param name="graphics">The graphics context to print to</param>
        public override void OnPrint(Graphics graphics)
        {
            Style style = Column.HeaderPrintStyle;
            PaintBackground(graphics, style, true);
            PaintForeground(graphics, style, true);
        }

        /// <summary>
        /// Called when the mouse is first moved over the widget
        /// </summary>
        public override void OnMouseEnter(MouseEventArgs e)
        {
            if (Column.Sortable && Column.Active)
            {
                Hot = true;
            }

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
            Hot = false;
            Tree.HideToolTip();
        }

        /// <summary>
        /// Handle a left mouse down event
        /// </summary>
        /// <param name="e">The mouse event details.</param>
        protected virtual void OnLeftMouseDown(MouseEventArgs e)
        {
            Pressed = true;
        }

        /// <summary>
        /// Handle a right mouse down event
        /// </summary>
        /// <param name="e">The mouse event details.</param>
        protected virtual void OnRightMouseDown(MouseEventArgs e)
        {
            if (Column.Active)
            {
                Tree.ShowHeaderContextMenu(Column);
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
            Pressed = false;
        }

        /// <summary>
        /// Handle sorting the column if sortable
        /// </summary>
        /// <param name="e"></param>
        public override void OnClick(EventArgs e)
        {
            if (WidgetControl.MouseDownButton == MouseButtons.Left)
            {
                if (Column.Sortable && Column.Active)
                {
                    if (Tree.SortColumn == Column)
                    {
                        Column.SortDirection = (Column.SortDirection == ListSortDirection.Ascending) ?
                            ListSortDirection.Descending : ListSortDirection.Ascending;
                    }
                    else
                    {
                        Tree.SortColumn = Column;
                    }
                }
            }
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
                DrawingUtilities.DestroyCursor(WidgetControl.DragNoneCursor);
                WidgetControl.DragNoneCursor = null;
            }
        }

        /// <summary>
        /// Create the drag cursor used for the column header widget.
        /// </summary>
        protected virtual void SetDragCursors()
        {
            if (!DrawingUtilities.LargeCursorsSupported) return;

            Rectangle bounds = Bounds;
            using (Bitmap bitmap = new Bitmap(bounds.Width, bounds.Height))
            {
                using (Graphics graphics = Graphics.FromImage(bitmap))
                {
                    // shift the origin to draw the column header at the bitmap origin
                    //
                    graphics.TranslateTransform(-bounds.X, -bounds.Y);
                    if (RightToLeft == RightToLeft.Yes)
                    {
                        // Shifting the origin for RightToLeft themed backgrounds doesn't work 
                        // because of mirroring - instead force the use non-themed backgrounds
                        //
                        PaintBackground(graphics, Column.HeaderStyle, false);
                        PaintForeground(graphics, Column.HeaderStyle, false);
                    }
                    else
                    {
                        OnPaint(graphics);
                    }
                }

                if (_arrowCursor == null)
                {
                    _arrowCursor = new Cursor(typeof(VirtualTree), "Cursors.Arrow.cur");
                }
                WidgetControl.DragMoveCursor = DrawingUtilities.CreateCursor(bitmap, 0.5F,
                    0, 0, _arrowCursor, 10, 10, WidgetControl.RightToLeft);
                WidgetControl.DragNoneCursor = DrawingUtilities.CreateCursor(bitmap, 0.5F,
                    10, 10, Cursors.No, 16, 16, WidgetControl.RightToLeft);
            }
        }

        /// <summary>
        /// Start a drag/drop operation for this column header
        /// </summary>
        protected virtual void DoDragDrop()
        {
            Dragging = true;
            SetDragCursors();
            
            // Set the format of the data explicitly so that if the column is of a derived
            // class the drop will still work
            //
            DataObject data = new DataObject();
            data.SetData(typeof(Column), Column);
            WidgetControl.DoDragDrop(this, data, DragDropEffects.Move); 
            DestroyDragCursors();
            Dragging = false;
            Pressed = false;
        }

        /// <summary>
        /// Handle MouseMove events for this Widget
        /// </summary>
        /// <param name="e"></param>
        public override void OnMouseMove(MouseEventArgs e)
        {
            if (Pressed)
            {
                if (!Dragging && Column.CanMove)
                {
                    if (WidgetControl.MouseDragging)
                    {
                        DoDragDrop();
                    }
               }
            }
        }

        /// <summary>
        /// Handle dragging of another column over this column
        /// </summary>
        /// <param name="e"></param>
        public override void OnDragEnter(DragEventArgs e)
        {
            e.Effect = DragDropEffects.None;
            if (Column.CanMove && e.Data.GetDataPresent(typeof(Column)))
            {
                Column dropColumn = (Column)e.Data.GetData(typeof(Column));
             
                // check the drop column belongs to this tree
                //
                if (Tree.Columns.Contains(dropColumn))
                {
                    // if this column is inactive only allow drop on it if the
                    // column being dropped can be hidden
                    //
                    if (Column.Active || dropColumn.Hidable)
                    {
                        e.Effect = DragDropEffects.Move;
                        Dropping = true;
                        Update();
                    }
                }
            }
        }

        /// <summary>
        /// Handle drag leaving this widget
        /// </summary>
        /// <param name="e"></param>
        public override void OnDragLeave(EventArgs e)
        {
            if (Dropping)
            {
                Dropping = false;
                Update();
            }
        }

        /// <summary>
        /// Handle dropping another column on this column
        /// </summary>
        /// <param name="e"></param>
        public override void OnDragDrop(DragEventArgs e)
        {
            if (e.Data.GetDataPresent(typeof(Column)))
            {
                Dropping = false;
                Column dropColumn = (Column)e.Data.GetData(typeof(Column));
                int index = Tree.Columns.IndexOf(Column);
                int dropIndex = Tree.Columns.IndexOf(dropColumn);
                if (index >= 0 && dropIndex >= 0)
                {
                    // when dropping non-active columns adjust the index so that
                    // the column is always inserted before the selected column
                    //
                    if (!dropColumn.Active)
                    {
                        if (dropIndex < index)
                            index--;
                    }
                    Tree.SuspendLayout();
                    try
                    {
                        Tree.Columns.SetIndexOf(dropColumn, index);
                        dropColumn.Active = !(WidgetControl is ColumnCustomizeControl);
                        dropColumn.Pinned = Column.Pinned;
                    }
                    finally
                    {
                        Tree.ResumeLayout();
                        Tree.PerformLayout(null, null);
                    }
                }
            }
        }
 
        /// <summary>
        /// Is the mouse currently hovering over the header
        /// </summary>
        protected bool Hot
        {
            get 
            { 
                return _hot; 
            }
            set
            {
                if (_hot != value)
                {
                    _hot = value;
                    Invalidate();                     
                }
            }
        }

        /// <summary>
        /// Has the column header been pressed
        /// </summary>
        protected bool Pressed
        {
            get { return _pressed; }
            set
            {
                if (_pressed != value)
                {
                    _pressed = value;
                    Invalidate();                     
                }
            }
        }

        /// <summary>
        /// Is the column header being dragged
        /// </summary>
        protected bool Dragging
        {
            get { return _dragging; }
            set { _dragging = value; }
        }

        /// <summary>
        /// Should the column header be shown as a potential drop site
        /// </summary>
        protected bool Dropping
        {
            get { return _dropping; }
            set
            {
                if (_dropping != value)
                {
                    _dropping = value;
                    Invalidate();                     
                }
            }
        }

        /// <summary>
        /// Return the tooltip text to display for this cell (if any)
        /// </summary>
        /// <returns>The tooltip to display or null if no tooltip should be displayed</returns>
        protected virtual string GetToolTipText()
        {
            if (_column.ToolTip != null)
            {
                return _column.ToolTip;
            }
            
            using (Graphics graphics = WidgetControl.CreateGraphics())
            {
                Style style = _column.HeaderStyle;
                string text = _column.Caption;
                if (!style.StringFits(graphics, text, _textBounds))
                    return text;
            }
            return null;
        }

        #endregion
    }

}
