#region File Header
//
//      FILE:   ExpansionIndicator.cs.
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
using NS=Infralution.Controls.VirtualTree;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a <see cref="Widget"/> for displaying the expansion indicator for a <see cref="NS.Row"/> of a 
    /// <see cref="VirtualTree"/> 
    /// </summary>
    /// <remarks>
    /// <para>
    /// This class defines the visual appearance and behaviour of the row expansion indicator for a <see cref="NS.VirtualTree"/>.
    /// It draws the indicator based on the current state of the <see cref="NS.Row"/> using the 
    /// <see cref="NS.VirtualTree.ExpandIcon"/> and <see cref="NS.VirtualTree.CollapseIcon"/>.
     ///</para>
    /// <para>
    /// This class can be extended to customize the appearance and/or behaviour of the expansion indicator.  To
    /// have the <see cref="NS.VirtualTree"/> use the derived class you must override the 
    /// <see cref="VirtualTree.CreateExpansionWidget"/> method.
    /// </para>
    /// </remarks>
    /// <seealso cref="NS.VirtualTree"/>
    /// <seealso cref="NS.Row"/>
    /// <seealso cref="NS.RowWidget"/>
    public class ExpansionWidget : Widget
    {

        #region Member Variables

        /// <summary>
        /// The row widget the expansion widget belongs to
        /// </summary>
        private RowWidget _rowWidget;

        /// <summary>
        /// The location to draw the centre of the expansion widget at
        /// </summary>
        private Point _location = Point.Empty;

        #endregion

        #region Public Interface

        /// <summary>
        /// Creates a new widget.
        /// </summary>
        /// <param name="rowWidget">The row Widget the expansion widget belongs to</param>
        public ExpansionWidget(RowWidget rowWidget)
            : base(rowWidget.Tree)
        {
            _rowWidget = rowWidget;
        }

        /// <summary>
        /// The row widget this widget belongs to
        /// </summary>
        public RowWidget RowWidget
        {
            get { return _rowWidget; }
        }

        /// <summary>
        /// The row associated with the widget
        /// </summary>
        public Row Row
        {
            get { return _rowWidget.Row; }
        }
  
        /// <summary>
        /// The Tree control the widget is for
        /// </summary>
        public VirtualTree Tree
        {
            get { return (WidgetControl as VirtualTree); }
        }

        /// <summary>
        /// Set/Get the location of the centre of the expansion indicator
        /// </summary>
        public Point Location
        {
            get { return _location; }
            set 
            {
                _location = value;
                Icon icon = (Row.Expanded) ? Tree.CollapseIcon : Tree.ExpandIcon;
                Bounds = new Rectangle(_location.X - icon.Width / 2, 
                                       _location.Y - icon.Height / 2, 
                                       icon.Width, icon.Height);
            }
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Update the bounds of this widget
        /// </summary>
        public override void OnLayout()
        {
        }

        /// <summary>
        /// Handle painting for this widget
        /// </summary>
        /// <param name="graphics"></param>
        public override void OnPaint(Graphics graphics)
        {
            Icon icon = (Row.Expanded) ? Tree.CollapseIcon : Tree.ExpandIcon;
            DrawingUtilities.DrawIcon(graphics, icon, Bounds.X, Bounds.Y, false, RightToLeft == RightToLeft.Yes);
        }

        /// <summary>
        /// Handle printing for this widget
        /// </summary>
        /// <param name="graphics">The graphics context to print to</param>
        public override void OnPrint(Graphics graphics)
        {
            Icon icon = (Row.Expanded) ? Tree.CollapseIcon : Tree.ExpandIcon;
            DrawingUtilities.DrawIcon(graphics, icon, Bounds.X, Bounds.Y, true, RightToLeft == RightToLeft.Yes);
        }

        /// <summary>
        /// Handle MouseDown events for this Widget
        /// </summary>
        /// <param name="e"></param>
        public override void OnMouseDown(MouseEventArgs e)
        {
            if (Row.Expanded)
                RowWidget.CollapseRow();
            else 
                RowWidget.ExpandRow();
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
