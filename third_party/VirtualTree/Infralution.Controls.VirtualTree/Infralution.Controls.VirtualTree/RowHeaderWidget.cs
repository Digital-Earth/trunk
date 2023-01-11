 #region File Header
//
//      FILE:   RowHeaderWidget.cs.
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
using System.Windows.Forms.VisualStyles;
using System.ComponentModel;
using System.Diagnostics;
using Infralution.Controls;
using Infralution.Common;
using NS=Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a <see cref="Widget"/> for displaying the header for a row.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This class defines the visual appearance and behaviour of the header region of a row within 
    /// a <see cref="VirtualTree"/>. This class can be extended to customize the appearance and/or behaviour 
    /// of row headers.  To have the <see cref="NS.VirtualTree"/> use the derived class you must override the 
    /// <see cref="VirtualTree.CreateRowHeaderWidget"/> method.
    /// </para>
    /// </remarks>
    /// <seealso cref="NS.VirtualTree"/>
    /// <seealso cref="NS.Row"/>
    /// <seealso cref="NS.RowWidget"/>
    public class RowHeaderWidget : Widget
    {

        #region Member Variables

        /// <summary>
        /// The RowWidget the header belongs to 
        /// </summary>
        private RowWidget _rowWidget;

        /// <summary>
        /// The divider widget for this row
        /// </summary>
        private RowDividerWidget _dividerWidget;

        #endregion

        #region Public Interface

        /// <summary>
        /// Creates a new widget.
        /// </summary>
        /// <param name="rowWidget">The RowWidget the header belongs to</param>
        public RowHeaderWidget(RowWidget rowWidget)
            : base(rowWidget.WidgetControl)
        {
            _rowWidget = rowWidget;
            if (Tree.AllowRowResize && rowWidget.RowData.Resizable)
            {
                _dividerWidget = Tree.CreateRowDividerWidget(this);
                ChildWidgets.Add(_dividerWidget);
            }
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
        /// The row divider widget (if present)
        /// </summary>
        public RowDividerWidget DividerWidget
        {
            get { return _dividerWidget; }
        }


        #endregion

        #region Local Methods

        /// <summary>
        /// Handle painting the focus indicator
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        /// <param name="style">The style to use</param>
        /// <param name="printing">Are we printing?</param>
        protected virtual void PaintFocusIndicator(Graphics graphics, Style style, bool printing)
        {
            Icon focusIcon = Tree.FocusIcon;
            if (focusIcon != null)
            {
                Rectangle bounds = Bounds;
                bounds.X += style.BorderWidth;
                bounds.Y += (bounds.Height - focusIcon.Height) / 2;
                bounds.Width = focusIcon.Width;
                bounds = RtlTranslateRect(bounds);
                DrawingUtilities.DrawIcon(graphics, focusIcon, bounds.X, bounds.Y, printing, RightToLeft == RightToLeft.Yes);
            }
        }

        /// <summary>
        /// Handle painting the error indicator
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        /// <param name="style">The style to use</param>
        /// <param name="printing">Are we printing?</param>
        protected virtual void PaintErrorIndicator(Graphics graphics, Style style, bool printing)
        {
            Icon errorIcon = Tree.ErrorIcon;
            if (errorIcon != null)
            {
                Rectangle bounds = Bounds;
                bounds.X = bounds.Right - errorIcon.Width - style.BorderWidth;;
                bounds.Y += (bounds.Height - errorIcon.Height) / 2;
                bounds.Width = errorIcon.Width;
                bounds = RtlTranslateRect(bounds);
                DrawingUtilities.DrawIcon(graphics, errorIcon, bounds.X, bounds.Y, printing);
            }
        }

        /// <summary>
        /// Paint the foreground of the row header
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        /// <param name="style">The style to use</param>
        /// <param name="printing">Are we printing?</param>
        protected virtual void PaintForeground(Graphics graphics, Style style, bool printing)
        {
            if (!StringUtilities.IsBlank(RowWidget.RowData.Error))
            {
                PaintErrorIndicator(graphics, style, printing);
            }
            if (RowWidget.Row.HasFocus)
            {
                PaintFocusIndicator(graphics, style, printing);
            }
        }

        /// <summary>
        /// Paint the background using XP Theme styles
        /// </summary>
        /// <param name="graphics"></param>
        protected virtual void PaintThemedBackground(Graphics graphics)
        {
            VisualStyleRenderer renderer = new VisualStyleRenderer(VisualStyleElement.Header.Item.Normal);
            DrawingUtilities.DrawThemeBackground(WidgetControl, graphics, renderer, Bounds, RightToLeft);
        }

        /// <summary>
        /// Paint the background and border of the row header
        /// </summary>
        /// <param name="graphics">The context to paint to</param>
        /// <param name="style">The style to use</param>
        /// <param name="printing">Are we printing?</param>
        protected virtual void PaintBackground(Graphics graphics, Style style, bool printing)
        {
            style.DrawBackground(graphics, Bounds);
            style.DrawBorder(graphics, Bounds);
        }

        /// <summary>
        /// Update the location of the divider widget (if present)
        /// </summary>
        public override void OnLayout()
        {
            if (_dividerWidget != null)
            {
                const int dividerHeight = 2;
                _dividerWidget.Bounds = new Rectangle(Bounds.Left, Bounds.Bottom - dividerHeight, 
                                                      Bounds.Width, dividerHeight*2);
            }
        }

        /// <summary>
        /// Handle painting for this widget
        /// </summary>
        /// <param name="graphics"></param>
        public override void OnPaint(Graphics graphics)
        {
            if (Bounds.Width > 0) 
            {
                Style style = Tree.HeaderStyle;
                if (Tree.UseThemedHeaders)
                {
                    PaintThemedBackground(graphics);
                }
                else
                    PaintBackground(graphics, style, false);
                PaintForeground(graphics, style, false);
            }
        }

        /// <summary>
        /// Handle printing for this widget
        /// </summary>
        /// <param name="graphics">The graphics context to print to</param>
        public override void OnPrint(Graphics graphics)
        {
            PaintBackground(graphics, Tree.HeaderPrintStyle, true);
            PaintForeground(graphics, Tree.HeaderPrintStyle, true);
        }

        /// <summary>
        /// Called when the mouse is first moved over the widget
        /// </summary>
        public override void OnMouseEnter(MouseEventArgs e)
        {
            string error = RowWidget.RowData.Error;
            if (!StringUtilities.IsBlank(error))
            {
                Tree.ShowToolTip(error);
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
        /// Handle MouseDown events for this Widget
        /// </summary>
        /// <param name="e"></param>
        public override void OnMouseDown(MouseEventArgs e)
        {
            Tree.SelectedColumn = null;
            RowWidget.OnMouseDown(e, Tree.EnableRowHeaderDragSelect);
        }

        /// <summary>
        /// Handle MouseUp events for this Widget.
        /// </summary>
        /// <param name="e"></param>
        public override void OnMouseUp(MouseEventArgs e)
        {
            RowWidget.OnMouseUp(e, Tree.EnableRowHeaderDragSelect);
        }

        /// <summary>
        /// Handle MouseMove events for this Widget
        /// </summary>
        /// <param name="e"></param>
        public override void OnMouseMove(MouseEventArgs e)
        {
            RowWidget.OnMouseMove(e, Tree.EnableRowHeaderDragSelect);
        }

        /// <summary>
        /// Handle Double Click events for this Widget.
        /// </summary>
        /// <param name="e"></param>
        public override void OnDoubleClick(EventArgs e)
        {
            RowWidget.OnDoubleClick(e);
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


        #endregion

    }

}
