#region File Header
//
//      FILE:   ColumnDividerWidget.cs.
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
using System.Diagnostics;
using NS=Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a <see cref="Widget"/> that allows the end user to resize a <see cref="NS.Column"/> of a <see cref="VirtualTree"/>. 
    /// </summary>
    /// <remarks>
    /// Column divider widgets belong to a <see cref="NS.ColumnHeaderWidget"/>.  They are positioned to the right of the
    /// <see cref="NS.ColumnHeaderWidget"/> but only draw a graphical representation while they are being dragged by the
    /// user.    They allow the user to click and drag them to change the size of the column header they belong to.
    /// </remarks>
    /// <seealso cref="NS.VirtualTree"/>
    /// <seealso cref="NS.Column"/>
    /// <seealso cref="NS.ColumnHeaderWidget"/>
    public class ColumnDividerWidget : Widget
    {

        /// <summary>
        /// The column header the widget parenting this widget
        /// </summary>
        private ColumnHeaderWidget _columnHeaderWidget;

        /// <summary>
        /// The current position of the divider
        /// </summary>
        private int _position;

        /// <summary>
        /// Has the click already been handled
        /// </summary>
        private bool _clickHandled = false;

        /// <summary>
        /// Creates a new widget.
        /// </summary>
        /// <param name="columnHeaderWidget">The column header widget the divider is for</param>
        public ColumnDividerWidget(ColumnHeaderWidget columnHeaderWidget)
            : base(columnHeaderWidget.WidgetControl)
        {
            _columnHeaderWidget = columnHeaderWidget;
        }

        /// <summary>
        /// The column header widget the divider is for
        /// </summary>
        public ColumnHeaderWidget ColumnHeaderWidget
        {
            get { return _columnHeaderWidget; }
        }

        /// <summary>
        /// The column the divider is for
        /// </summary>
        public Column Column
        {
            get { return _columnHeaderWidget.Column; }
        }

        /// <summary>
        /// The virtual tree the divider is associated with
        /// </summary>
        public VirtualTree Tree
        {
            get { return _columnHeaderWidget.Tree; }
        }

        /// <summary>
        /// Set/Get the current x position of the divider (when moving)
        /// </summary>
        public int Position
        {
            get { return _position; }
        }

        /// <summary>
        /// Return the minimum width the column can be allowed to resize to
        /// </summary>
        /// <returns></returns>
        protected virtual int GetMinColumnWidth()
        {
            Column column = Column;
            int minWidth = column.MinWidth;
            if (column.AutoSizePolicy != ColumnAutoSizePolicy.Manual)
            {
                minWidth = Math.Max(minWidth, Tree.GetOptimalColumnWidth(column));
            }
            return minWidth;
        }

        /// <summary>
        /// Return the maximum width the column can be allowed to resize to
        /// </summary>
        /// <returns></returns>
        protected virtual int GetMaxColumnWidth()
        {
            // if the column is pinned then work out the width required to maintain a minimum sized
            // scrollable area
            //
            if (Column.Pinned)
            {
                const int minScrollableWidth = 100;
                int availableWidth = Tree.ScrollableDisplayWidth - minScrollableWidth;
                if (availableWidth > 0)
                    return Column.Width + availableWidth;
                else
                    return Column.Width;
            }

            if (Tree.AutoFitColumns)
            {
                int availableWidth = Tree.ScrollableDisplayWidth - Tree.MinScrollableWidth;
                if (availableWidth > 0)
                    return Column.MinWidth + availableWidth;
                else
                    return Column.MinWidth;
            }

            return 10000;
        }

        /// <summary>
        /// Set the new position of the divider
        /// </summary>
        /// <param name="x">The position of the divider in control coordinates</param>
        protected virtual void SetPosition(int x)
        {
            Column column = ColumnHeaderWidget.Column;
            int minX, maxX;
            if (this.RightToLeft == RightToLeft.Yes)
            {
                maxX = ColumnHeaderWidget.Bounds.Right - GetMinColumnWidth();
                minX = ColumnHeaderWidget.Bounds.Right - GetMaxColumnWidth();
            }
            else
            {
                minX = ColumnHeaderWidget.Bounds.Left + GetMinColumnWidth();
                maxX = ColumnHeaderWidget.Bounds.Left + GetMaxColumnWidth();
            }
            _position = x;
            if (_position > maxX)
                _position = maxX;
            if (_position < minX)
                _position = minX;
        }

        /// <summary>
        /// Draw the column divider when moving the column divider
        /// </summary>
        protected virtual void DrawDivider()
        {
            if (_position != 0)
            {
                Rectangle bounds = Tree.ClientRectangle;
                Point top = new Point(_position, bounds.Top);
                Point bottom = new Point(_position, bounds.Bottom);
                ControlPaint.DrawReversibleLine(WidgetControl.PointToScreen(top), 
                    WidgetControl.PointToScreen(bottom), Color.Black);
            }
        }

        /// <summary>
        /// Called when the mouse is first moved over the widget
        /// </summary>
        public override void OnMouseEnter(MouseEventArgs e)
        {
            WidgetControl.Cursor = Cursors.VSplit;
        }

        /// <summary>
        /// Called when the mouse is leaves the widget
        /// </summary>
        public override void OnMouseLeave(EventArgs e)
        {
            WidgetControl.Cursor = Cursors.Default;
        }

        /// <summary>
        /// Handle MouseDown events for this Widget
        /// </summary>
        /// <param name="e"></param>
        public override void OnMouseDown(MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                _position = 0;
                _clickHandled = false;
            }
            else if (e.Button == MouseButtons.Right)
            {
                ColumnHeaderWidget.OnMouseDown(e);
            }
        }

        /// <summary>
        /// Handle MouseMove events for this Widget
        /// </summary>
        /// <param name="e"></param>
        public override void OnMouseMove(MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                DrawDivider();
                SetPosition(e.X);
                DrawDivider();
            }
        }

        /// <summary>
        /// Handles case where OnClick is not.called because the mouse has been dragged outside
        /// the control.  Calls <see cref="OnClick"/> in this case.
        /// </summary>
        /// <param name="e"></param>
        public override void OnMouseUp(MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                if (!_clickHandled)
                {
                    OnClick(e);
                }
            }
        }

        /// <summary>
        /// Handle a Click event for this widget
        /// </summary>
        /// <param name="e"></param>
        public override void OnClick(EventArgs e)
        {
            DrawDivider();
            _clickHandled = true;
            if (Position != 0)
            {
                int width;
                if (RightToLeft == RightToLeft.Yes)
                   width = ColumnHeaderWidget.Bounds.Right - Position;
                else
                   width = Position - ColumnHeaderWidget.Bounds.Left;
                
                if (width < ColumnHeaderWidget.Column.MinWidth)
                    width = ColumnHeaderWidget.Column.MinWidth;
                if (!Column.Pinned && Tree.AutoFitColumns)
                    Tree.AdjustColumnAutoFitWeights(Column, width);
                else
                    Column.Width = width;
            }
        }

        /// <summary>
        /// Best fit the column on double click
        /// </summary>
        /// <param name="e"></param>
        public override void OnDoubleClick(EventArgs e)
        {
            DrawDivider();
            _clickHandled = true;
            Tree.SetBestFitWidth(Column);
        }

    }

}
