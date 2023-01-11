#region File Header
//
//      FILE:   HeaderWidget.cs.
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
using System.Collections;
using System.Windows.Forms.VisualStyles;
using NS=Infralution.Controls.VirtualTree;
using System.Diagnostics;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a <see cref="Widget"/> for displaying the header area for a <see cref="VirtualTree"/> 
    /// </summary>
    /// <remarks>
    /// <para>
    /// This class defines the visual appearance and behaviour of the column header area for a 
    /// <see cref="VirtualTree"/>.  It draws the background header and creates a child 
    /// <see cref="ColumnHeaderWidget"/> for each active <see cref="Column"/> of the <see cref="VirtualTree"/> 
    /// </para>
    /// <para>
    /// This class can be extended to customize the appearance and/or behaviour of the header area.  To
    /// have the <see cref="NS.VirtualTree"/> use the derived class you must override the 
    /// <see cref="VirtualTree.CreateHeaderWidget"/> method.
    /// </para>
    /// </remarks>
    /// <seealso cref="NS.VirtualTree"/>
    /// <seealso cref="NS.ColumnHeaderWidget"/>
    public class HeaderWidget : Widget
    {

        #region Member Variables

        /// <summary>
        /// The columns the widget is to display
        /// </summary>
        private SimpleColumnList _columns;

        /// <summary>
        /// Is this a potential drop site 
        /// </summary>
        private bool _dropping = false;

        /// <summary>
        /// The column header widgets indexed by column
        /// </summary>
        private Hashtable _columnWidgets = new Hashtable();

        /// <summary>
        /// Should the row header be displayed
        /// </summary>
        private bool _showRowHeader = false;

        #endregion

        #region Public Interface

        /// <summary>
        /// Creates a new widget.
        /// </summary>
        /// <param name="panelWidget">The panel widget the header belongs to</param>
        public HeaderWidget(PanelWidget panelWidget)
            : base(panelWidget)
        {
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
            get { return PanelWidget.Tree; }
        }

        /// <summary>
        /// Should this header widget display the row header
        /// </summary>
        public bool ShowRowHeader 
        {
            get { return _showRowHeader; }
            set { _showRowHeader = value; }
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
        /// Get the column header widget for the given column
        /// </summary>
        /// <param name="column">The column to get the widget for</param>
        /// <returns>The widget for the given column</returns>
        public virtual ColumnHeaderWidget GetColumnHeaderWidget(Column column)
        {
            ColumnHeaderWidget widget = (ColumnHeaderWidget)_columnWidgets[column];
            if (widget == null)
            {
                widget = Tree.CreateColumnHeaderWidget(this, column);
                _columnWidgets[column] = widget;
            }
            return widget;
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Should the header be shown as a potential drop site
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
        /// Paint the background using XP Theme styles
        /// </summary>
        /// <param name="graphics"></param>
        protected virtual void PaintThemedBackground(Graphics graphics)
        {
            Rectangle bounds = Bounds;
            VisualStyleElement element;
            if (Dropping)
                element = VisualStyleElement.Header.Item.Hot;
            else
                element = VisualStyleElement.Header.Item.Normal;

            bounds.Width += 10;
            bounds = RtlTranslateRect(bounds);
            VisualStyleRenderer renderer = new VisualStyleRenderer(element);
            DrawingUtilities.DrawThemeBackground(WidgetControl, graphics, renderer, bounds, RightToLeft);

            if (ShowRowHeader)
            {
                bounds = Bounds;
                bounds.Width = Tree.RowHeaderWidth;
                bounds = RtlTranslateRect(bounds);
                DrawingUtilities.DrawThemeBackground(WidgetControl, graphics, renderer, bounds, RightToLeft);
            }
        }

        /// <summary>
        /// Paint the background using standard .NET border styles
        /// </summary>
        /// <param name="graphics"></param>
        protected virtual void PaintBackground(Graphics graphics)
        {
            Style style = (Dropping) ? Tree.HeaderDropStyle : Tree.HeaderStyle;
            style.DrawBackground(graphics, Bounds);
            style.DrawBorder(graphics, Bounds);
        }

        /// <summary>
        /// Update this widget and its children on layout
        /// </summary>
        public override void OnLayout()
        {
            if (_columns == null) return;

            ChildWidgets.Clear();
            Rectangle bounds = Bounds;
            if (ShowRowHeader)
            {
                bounds.X += Tree.RowHeaderWidth;
                bounds.Width -= Tree.RowHeaderWidth;
            }

            // layout the visible child widgets
            //
            Rectangle panelClip = PanelWidget.ClipBounds;
            foreach (Column column in _columns)
            {
                ColumnHeaderWidget widget = GetColumnHeaderWidget(column);
                bounds.Width = column.Width;
                Rectangle rtlBounds = RtlTranslateRect(bounds);
                if (rtlBounds.Width > 0 && 
                    rtlBounds.Left < panelClip.Right &&
                    rtlBounds.Right > panelClip.Left)
                {
                    widget.Bounds = rtlBounds;
                    ChildWidgets.Add(widget);
                }
                bounds.X += bounds.Width;

                // exit the loop once we have processed the last visible column
                //
                if (RightToLeft == RightToLeft.Yes)
                {
                    if (rtlBounds.Right < panelClip.Left)
                        break;
                }
                else
                {
                    if (rtlBounds.Left > panelClip.Right)
                        break;
                }
            }
        }

        /// <summary>
        /// Handle painting for this widget
        /// </summary>
        /// <param name="graphics">The graphics context to paint to</param>
        public override void OnPaint(Graphics graphics)
        {
            if (Tree.UseThemedHeaders)
                PaintThemedBackground(graphics);
            else
                PaintBackground(graphics);
            base.OnPaint(graphics);
        }

        /// <summary>
        /// Handle printing for this widget
        /// </summary>
        /// <param name="graphics">The graphics context to print to</param>
        public override void OnPrint(Graphics graphics)
        {
            Style style = Tree.HeaderPrintStyle;
            style.DrawBackground(graphics, Bounds);
            style.DrawBorder(graphics, Bounds);

            // print the child (column header) widgets
            //
            base.OnPrint(graphics);
        }

        /// <summary>
        /// Handle a right mouse down event
        /// </summary>
        /// <param name="e">The mouse event details.</param>
        protected virtual void OnRightMouseDown(MouseEventArgs e)
        {
            Tree.ShowHeaderContextMenu(null);
        }

        /// <summary>
        /// Handle a left mouse down event
        /// </summary>
        /// <param name="e">The mouse event details.</param>
        protected virtual void OnLeftMouseDown(MouseEventArgs e)
        {
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
        /// Handle dragging of a column widget past the end of columns
        /// </summary>
        /// <param name="e"></param>
        public override void OnDragEnter(DragEventArgs e)
        {
            if (e.Data.GetDataPresent(typeof(Column)))
            {
                Column dropColumn = (Column)e.Data.GetData(typeof(Column));
                
                // check the drop column belongs to this tree
                //
                if (Tree.Columns.Contains(dropColumn))
                {
                    Point pos = WidgetControl.PointToClient(new Point(e.X, e.Y));
                    if (ShowRowHeader && pos.X < Tree.RowHeaderWidth)
                    {
                        Dropping = Tree.AllowUserPinnedColumns && 
                            (Columns.Count == 0 || Columns[0].Movable);
                    }
                    else
                    {
                        Dropping = true;
                    }
                    if (Dropping)
                    {
                        e.Effect = DragDropEffects.Move;
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
        /// Handle dropping a column on the empty header area
        /// </summary>
        /// <param name="e"></param>
        public override void OnDragDrop(DragEventArgs e)
        {
            if (e.Data.GetDataPresent(typeof(Column)))
            {
                Dropping = false;

                Column dropColumn = (Column)e.Data.GetData(typeof(Column));
                Point pos = WidgetControl.PointToClient(new Point(e.X, e.Y));
                int index = 0;
                if (ShowRowHeader && pos.X < Tree.RowHeaderWidth)
                {
                    index = 0;
                    dropColumn.Pinned = true;
                }
                else
                {
                    index = Tree.Columns.Count - 1;
                }
                Tree.SuspendLayout();
                Tree.Columns.SetIndexOf(dropColumn, index);
                dropColumn.Active = true;
                Tree.ResumeLayout();
                Tree.PerformLayout();
            }
        }

        #endregion
    }
}
