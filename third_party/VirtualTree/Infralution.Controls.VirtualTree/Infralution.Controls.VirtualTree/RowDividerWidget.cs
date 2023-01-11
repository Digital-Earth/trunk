#region File Header
//
//      FILE:   RowDividerWidget.cs.
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
using NS=Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a <see cref="Widget"/> that allows the end user to change the <see cref="VirtualTree.RowHeight">VirtualTree.RowHeight</see>. 
    /// </summary>
    /// <remarks>
    /// Row divider widgets belong to a <see cref="NS.RowHeaderWidget"/>.  They are positioned at the bottom of the
    /// <see cref="NS.RowHeaderWidget"/> but only draw a graphical representation while they are being dragged by the
    /// user.    They allow the user to click and drag them to change the size of the RowHeight the VirtualTree.
    /// </remarks>
    /// <seealso cref="NS.VirtualTree"/>
    /// <seealso cref="NS.RowHeaderWidget"/>
    public class RowDividerWidget : Widget
    {

        /// <summary>
        /// The row header widget parenting this widget
        /// </summary>
        private RowHeaderWidget _rowHeaderWidget;

        /// <summary>
        /// The current position of the divider
        /// </summary>
        private int _position = 0;

        /// <summary>
        /// Has the click already been handled
        /// </summary>
        private bool _clickHandled = false;

        /// <summary>
        /// Creates a new widget.
        /// </summary>
        /// <param name="rowHeaderWidget">The row header widget the divider is for</param>
        public RowDividerWidget(RowHeaderWidget rowHeaderWidget)
            : base(rowHeaderWidget.WidgetControl)
        {
            _rowHeaderWidget = rowHeaderWidget;
        }

        /// <summary>
        /// The row header widget the divider is for
        /// </summary>
        public RowHeaderWidget RowHeaderWidget
        {
            get { return _rowHeaderWidget; }
        }

        /// <summary>
        /// The tree the divider is associated with
        /// </summary>
        public VirtualTree Tree
        {
            get { return RowHeaderWidget.Tree; }
        }

        /// <summary>
        /// The row the divider is associated with
        /// </summary>
        public Row Row
        {
            get { return RowHeaderWidget.RowWidget.Row; }
        }

        /// <summary>
        /// Get the current y position of the divider (when moving)
        /// </summary>
        public int Position
        {
            get { return _position; }
        }

        /// <summary>
        /// Draw (reversibly) the row divider when moving the row divider
        /// </summary>
        protected virtual void DrawDivider()
        {
            if (_position != 0)
            {
                VirtualTree tree = RowHeaderWidget.Tree;
                Point left = new Point(0, _position);
                Point right = new Point(tree.Width, _position);
                ControlPaint.DrawReversibleLine(tree.PointToScreen(left), 
                                                tree.PointToScreen(right), Color.Black);
            }
        }

        /// <summary>
        /// Set the new position of the divider
        /// </summary>
        /// <param name="y">The position of the divider in control coordinates</param>
        protected virtual void SetPosition(int y)
        {
            Rectangle bounds = RowHeaderWidget.Bounds;
            int minY = bounds.Top + Tree.MinRowHeight;
            int maxY = bounds.Top + Tree.MaxRowHeight;
            _position = y;
            if (_position < minY) _position = minY;
            if (_position > maxY) _position = maxY;
        }

        /// <summary>
        /// Called when the mouse is first moved over the widget
        /// </summary>
        public override void OnMouseEnter(MouseEventArgs e)
        {
            WidgetControl.Cursor = Cursors.HSplit;
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
                SetPosition(e.Y);
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
                int height = Position - RowHeaderWidget.Bounds.Top;
                if (height < Tree.MinRowHeight) height = Tree.MinRowHeight;
                if (height > Tree.MaxRowHeight) height = Tree.MaxRowHeight;
                Tree.SuspendLayout();
                try
                {
                    if (Control.ModifierKeys == Keys.Control)
                    {
                        Tree.ClearUserRowHeight(Row);
                        Tree.RowHeight = height;
                    }
                    else if (Control.ModifierKeys == Keys.Shift)
                    {
                        Tree.ClearUserRowHeights();
                        Tree.RowHeight = height;
                    }
                    else
                    {
                        if (Tree.AllowIndividualRowResize)
                        {
                            Tree.SetUserRowHeight(Row, height);
                        }
                        else
                        {
                            Tree.RowHeight = height;
                            Tree.ClearUserRowHeights();
                        }
                    }
                }
                finally
                {
                    Tree.ResumeLayout();
                }
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
            if (Tree.AllowIndividualRowResize)
            {
                int height = Tree.GetOptimalRowHeight(Row);
                Tree.SetUserRowHeight(Row, height);
            }
        }


    }

}
