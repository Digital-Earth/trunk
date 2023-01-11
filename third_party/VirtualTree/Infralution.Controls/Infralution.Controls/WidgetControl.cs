#region File Header
//
//      FILE:   WidgetControl.cs.
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
using System.Windows.Forms;
using System.Collections;
using System.Drawing;
using System.ComponentModel;
using System.Diagnostics;
namespace Infralution.Controls
{
	/// <summary>
	/// Defines a base control class for controls which use widgets (light weight visual elements)
	/// to perform painting and mouse event handling.
	/// </summary>
    /// <remarks>
    /// Building complex composite controls using standard <see cref="Control">Controls</see>  
    /// uses a lot of system resources and is somewhat unweildy.   This class facilitates the
    /// building of complex controls with a minimum of resources. 
    /// </remarks>
    /// <seealso cref="Widget"/>
    [ToolboxItem(false)]
    public class WidgetControl : BorderedControl
    {
        #region Member Variables

        /// <summary>
        /// List of currently displayed widgets (user interface elements)
        /// </summary>
        private WidgetList _widgets = new WidgetList();

        /// <summary>
        /// The widget that currently is capturing mouse input
        /// </summary>
        private Widget _mouseCaptureWidget;

        /// <summary>
        /// The widget that the mouse was over when clicked
        /// </summary>
        private Widget _mouseDownWidget;

        /// <summary>
        /// The widget under the current mouse position
        /// </summary>
        private Widget _hotWidget;

        /// <summary>
        /// The widget receiving drag events
        /// </summary>
        private Widget _dragTargetWidget;

        /// <summary>
        /// The widget that is the source of a drag drop operation
        /// </summary>
        private Widget _dragSourceWidget;

        /// <summary>
        /// Cursors for drag/drop operations
        /// </summary>
        private Cursor _dragMoveCursor;
        private Cursor _dragCopyCursor;
        private Cursor _dragNoneCursor;
        private Cursor _dragLinkCursor;
        private Cursor _dragScrollCursor;

        /// <summary>
        /// The location at which the mouse was pressed.
        /// </summary>
        private Point _mouseDownLocation;

        /// <summary>
        /// The buttons that were pressed last last
        /// </summary>
        private MouseButtons _mouseDownButton = MouseButtons.None;

        /// <summary>
        /// The number of pixels that the mouse must be moved with the button held down
        /// to initiate a drag
        /// </summary>
        private int _dragSensitivity = _defaultDragSensitivity;
        private const int _defaultDragSensitivity = 5;

        #endregion

        #region Public Interface

        /// <summary>
        /// Return the the topmost Widget that contains the given point
        /// </summary>
        /// <param name="x"></param>
        /// <param name="y"></param>
        /// <returns>The topmost Widget that contains the given point</returns>
        public virtual Widget GetWidget(int x, int y)
        {     
            // iterate through the list in reverse order to find the topmost widget that is hit
            //
            for (int i = _widgets.Count - 1; i >= 0; i--)
            {
                Widget widget = (Widget)_widgets[i];
                Widget hitWidget = widget.GetHitWidget(x, y);
                if (hitWidget != null) return hitWidget;
            }
            return null;
        }

        /// <summary>
        /// Start a drag/drop operation for the given widget.  This widget is set as the
        /// DragSource and GiveDragFeedback events are routed to this widget.
        /// </summary>
        /// <param name="widget">The widget starting the drag/drop</param>
        /// <param name="data">The data to pass</param>
        /// <param name="allowedEffects">The allowed effects</param>
        public virtual void DoDragDrop(Widget widget, 
                                       object data, 
                                       DragDropEffects allowedEffects)
        {

            Cursor oldCursor = this.Cursor;

            // release the current MouseCaptureWidget - because if the drag drop was started
            // as a result of a button down event (which it typically is) then we won't
            // receive a button up event to release the MouseCaptureWidget
            //
            MouseCaptureWidget = null;
            DragSourceWidget = widget;
            base.DoDragDrop(data, allowedEffects);
            DragSourceWidget = null;
            Cursor = oldCursor;
        }

        /// <summary>
        /// Set/Get the cursor to use for drag drop move operations
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual Cursor DragMoveCursor
        {
            get { return _dragMoveCursor; }
            set { _dragMoveCursor = value; }
        }

        /// <summary>
        /// Set/Get the cursor to use for drag drop copy operations
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual Cursor DragCopyCursor
        {
            get { return _dragCopyCursor; }
            set { _dragCopyCursor = value; }
        }

        /// <summary>
        /// Set/Get the cursor to use for drag drop link operations
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual Cursor DragLinkCursor
        {
            get { return _dragLinkCursor; }
            set { _dragLinkCursor = value; }
        }

        /// <summary>
        /// Set/Get the cursor to use for drag drop operations when the target
        /// does not accept the dropped item
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual Cursor DragNoneCursor
        {
            get { return _dragNoneCursor; }
            set { _dragNoneCursor = value; }
        }

        /// <summary>
        /// Set/Get the cursor to use for drag drop scroll operations
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public virtual Cursor DragScrollCursor
        {
            get { return _dragScrollCursor; }
            set { _dragScrollCursor = value; }
        }

        /// <summary>
        /// Get the mouse location when the mouse was last pressed.  
        /// </summary>
        /// <remarks> 
        /// This is often useful in determining whether the user is starting a drag
        /// operation or is just pressing the button (with some minor movement)
        /// </remarks>
        [Browsable(false)]
        public virtual Point MouseDownLocation
        {
            get { return _mouseDownLocation; }
        }
        
        /// <summary>
        /// Get the mouse button used when the mouse was last pressed.  
        /// </summary>
        /// <remarks>
        /// This is useful when overriding <see cref="Widget.OnClick"/> and 
        /// <see cref="Widget.OnDoubleClick"/> to determine the mouse button that was
        /// clicked.
        /// </remarks>
        [Browsable(false)]
        public virtual MouseButtons MouseDownButton
        {
            get { return _mouseDownButton; }
        }

        /// <summary>
        /// Determine if the user is dragging the mouse from the location that it was pressed
        /// </summary>
        [Browsable(false)]
        public virtual bool MouseDragging
        {
            get
            {
                if (MouseCaptureWidget == null) return false;
                Point point =  PointToClient(MousePosition);
                return Math.Abs(MouseDownLocation.X - point.X) > DragSensitivity ||
                       Math.Abs(MouseDownLocation.Y - point.Y) > DragSensitivity;
            }
        }

        /// <summary>
        /// The number of pixels that the mouse must be moved with the button held down
        /// to initiate a drag
        /// </summary>
        /// <seealso cref="MouseDragging"/>
        [Category("Behavior")]
        [Description("The number of pixels that the mouse must be moved with the button held down to initiate a drag")]
        [DefaultValue(_defaultDragSensitivity)]
        public virtual int DragSensitivity
        {
            get { return _dragSensitivity; }
            set 
            {
                if (value < 1)
                    throw new ArgumentOutOfRangeException("value", "DragSensitivity must be greater than zero");
                _dragSensitivity = value; 
            }
        }


        #endregion

        #region Local Accessors

        /// <summary>
        /// Return the list of widgets parented by this control
        /// </summary>
        protected virtual WidgetList Widgets
        {
            get { return _widgets; }
        }

        /// <summary>
        /// Get/Set the widget that is capturing mouse events for the control
        /// </summary>
        /// <remarks>
        /// This is typically set to the widget that receives the MouseDown event
        /// and then cleared when the MouseUp event is received.  If a MouseCaptureWidget
        /// is set then all MouseMove and MouseUp events are sent to that widget. 
        /// </remarks>
        protected virtual Widget MouseCaptureWidget
        {
            get { return _mouseCaptureWidget; }
            set { _mouseCaptureWidget = value; }
        }

        /// <summary>
        /// Get/Set the widget that last received a MouseDown event (if any)
        /// </summary>
        protected virtual Widget MouseDownWidget
        {
            get { return _mouseDownWidget; }
            set { _mouseDownWidget = value; }
        }

        /// <summary>
        /// Get/Set the widget that the mouse is currently over
        /// </summary>
        protected virtual Widget HotWidget
        {
            get { return _hotWidget; }
            set { _hotWidget = value; }
        }

        /// <summary>
        /// Get/Set the widget that is currently receiving drag events
        /// </summary>
        protected virtual Widget DragTargetWidget
        {
            get { return _dragTargetWidget; }
            set { _dragTargetWidget = value; }
        }

        /// <summary>
        /// Set/Get the widget that is the source (ie initiated) for a drag drop operation
        /// </summary>
        protected virtual Widget DragSourceWidget
        {
            get { return _dragSourceWidget; }
            set { _dragSourceWidget = value; }
        }

        #endregion

        #region Mouse Event Handling

        /// <summary>
        /// Handle MouseDown events for the Widget control.  Sets the <see cref="MouseDownWidget"/>
        /// and  <see cref="MouseCaptureWidget"/> to the widget under the cursor and routes the event 
        /// to the widget.  All further mouse events are routed to the <see cref="MouseCaptureWidget"/> 
        /// until a button up event is received.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseDown(MouseEventArgs e)
        {
            _mouseDownLocation = new Point(e.X, e.Y);
            _mouseDownButton = e.Button;
            base.OnMouseDown (e);
            this.Focus();

            Widget widget = GetWidget(e.X, e.Y);
            MouseDownWidget = widget;
            MouseCaptureWidget = widget;
            if (widget != null)
            {
                widget.OnMouseDown(e);
            }
        }

        /// <summary>
        /// Handles MouseUp events for the Widget control.  The event is routed
        /// to the current <see cref="MouseCaptureWidget"/>.   The <see cref="MouseCaptureWidget"/>
        /// is set to null/nothing on completion.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseUp(MouseEventArgs e)
        {
            base.OnMouseUp (e);
            Widget widget = MouseCaptureWidget;
            if (widget != null)
            {
                widget.OnMouseUp(e);
            }
            MouseCaptureWidget = null;
        }

        /// <summary>
        /// Handles MouseMove events for the Widget control.  If there is a 
        /// <see cref="MouseCaptureWidget"/> then the event is routed to it.  Otherwise the 
        /// <see cref="HotWidget"/> is set to the widget under the cursor (if any) and the event is 
        /// routed to it.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseMove(MouseEventArgs e)
        {
            base.OnMouseMove (e);

            // If there is a MouseCaptureWidget (ie the user clicked on a widget
            // and hasn't yet released - then funnel all mouse move events to that widget
            // Otherwise funnel the event to the widget the mouse is currently over.
            //
            Widget widget = MouseCaptureWidget;
            if (widget == null)
            {
                widget = GetWidget(e.X, e.Y);
                if (widget != HotWidget)
                {
                    if (HotWidget != null)
                    {
                        HotWidget.OnMouseLeave(e);
                    }
                    HotWidget = widget;
                    if (widget != null)
                    {
                        widget.OnMouseEnter(e);
                    }
                }
            }

            if (widget != null)
            {
                widget.OnMouseMove(e);
            }
        }

        /// <summary>
        /// Handles OnMouseLeave events for the Widget control.  The event is routed
        /// to the current <see cref="HotWidget"/>.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseLeave(EventArgs e)
        {
            base.OnMouseLeave (e);
            if (HotWidget != null)
            {
                HotWidget.OnMouseLeave(e);
                HotWidget = null;
            }
        }

        /// <summary>
        /// Handles Click events for the Widget control and routes them to the appropriate Widget
        /// </summary>
        /// <remarks>
        /// Click events are only passed to a Widget if the mouse is over the same widget as when
        /// the mouse was clicked.  This allows the standard windows behaviour of sliding off a control
        /// when the mouse is pressed not activating the click event. 
        /// </remarks>
        /// <param name="e"></param>
        protected override void OnClick(EventArgs e)
        {
            base.OnClick(e);
            Point point = this.PointToClient(Control.MousePosition);
            Widget widget = GetWidget(point.X, point.Y);
            if (widget != null && widget == MouseDownWidget)
            {
                widget.OnClick(e);
            }
        }

        /// <summary>
        /// Handles DoubleClick events for the Widget control.  
        /// </summary>
        /// <param name="e"></param>
        protected override void OnDoubleClick(EventArgs e)
        {
            base.OnDoubleClick (e);
            Point point = PointToClient(MousePosition);
            Widget widget = GetWidget(point.X, point.Y);
            if (widget != null)
            {
                widget.OnDoubleClick(e);
            }
        }

        #endregion

        #region Painting

        /// <summary>
        /// Paint the control widgets in the given region of a graphics context
        /// </summary>
        /// <param name="graphics">The graphics context to paint to</param>
        /// <param name="clipRectangle">The clipping rectangle</param>
        protected virtual void PaintWidgets(Graphics graphics, Rectangle clipRectangle)
        {
            // avoid any potential reentrancy problems associated with iterators
            //
            for (int i = 0; i < _widgets.Count; i++)
            {
                Widget widget = _widgets[i];
                if (widget.PaintRequired(clipRectangle))
                {
                    widget.OnPaint(graphics);
                }
            }
        }

        /// <summary>
        /// Calls PaintWidgets.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);
            PaintWidgets(e.Graphics, e.ClipRectangle);
         }

        /// <summary>
        /// Update the position and size of the widgets managed by this control.
        /// </summary>
        protected virtual void LayoutWidgets()
        {
        }

        /// <summary>
        /// Calls LayoutWidgets
        /// </summary>
        /// <param name="levent"></param>
        protected override void OnLayout(LayoutEventArgs levent)
        {
            base.OnLayout (levent);
            LayoutWidgets();
            Invalidate();
        }

 
        /// <summary>
        /// Return a client x coordinate adjusted for <see cref="RightToLeft"/> positioning.
        /// </summary>
        /// <param name="x">The coordinate to adjust</param>
        /// <returns>The adjusted coordinate</returns>
        /// <remarks>
        /// If <see cref="RightToLeft"/> is set then this adjusts the coordinate to give right to left
        /// layout
        /// </remarks>
        protected virtual int RtlTranslateX(int x)
        {
            return DrawingUtilities.RtlTranslateX(this, x);
        }

        /// <summary>
        /// Return a point in client coordinates adjusted for <see cref="RightToLeft"/> positioning.
        /// </summary>
        /// <param name="p">The point to adjust</param>
        /// <returns>The adjusted point</returns>
        /// <remarks>
        /// If <see cref="RightToLeft"/> is set then this adjusts the point to give right to left
        /// layout
        /// </remarks>
        protected virtual Point RtlTranslatePoint(Point p)
        {
            return DrawingUtilities.RtlTranslatePoint(this, p);
        }

        /// <summary>
        /// Return a rectangle in client coordinates adjusted for <see cref="RightToLeft"/> positioning.
        /// </summary>
        /// <param name="r">The rectangle to adjust</param>
        /// <returns>The adjusted rectangle</returns>
        /// <remarks>
        /// If <see cref="RightToLeft"/> is set then this adjusts the rectangle to give right to left
        /// layout
        /// </remarks>
        protected virtual Rectangle RtlTranslateRect(Rectangle r)
        {
            return DrawingUtilities.RtlTranslateRect(this, r);
        }


        #endregion

        #region Drag and Drop

        /// <summary>
        /// Routes the event to the widget under the cursor.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnDragEnter(DragEventArgs e)
        {
            base.OnDragEnter (e);
            Point point = PointToClient(MousePosition);
            DragTargetWidget = GetWidget(point.X, point.Y);
            if (DragTargetWidget != null)
            {
                DragTargetWidget.OnDragEnter(e);
            }
        }

        /// <summary>
        /// Routes the event to the widget under the cursor.  If the
        /// widget has changed then this will call OnDragLeave/OnDragEnter on the
        /// relevant widgets.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnDragOver(DragEventArgs e)
        {
            base.OnDragOver (e);
            Point point = PointToClient(MousePosition);
            Widget widget = GetWidget(point.X, point.Y);
            if (DragTargetWidget != widget)
            {
                if (DragTargetWidget != null)
                {
                    DragTargetWidget.OnDragLeave(e);
                }
                DragTargetWidget = widget;
                if (widget != null)
                {
                    widget.OnDragEnter(e);
                }
            }
            else
            {
                if (widget != null)
                {
                    widget.OnDragOver(e);
                }
            }
        }

        /// <summary>
        /// Routes the event to the widget under the cursor.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnDragLeave(EventArgs e)
        {
            base.OnDragLeave (e);
            if (DragTargetWidget != null)
            {
                DragTargetWidget.OnDragLeave(e);
            }
            DragTargetWidget = null;
        }

        /// <summary>
        /// Routes the event to the widget under the cursor.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnDragDrop(DragEventArgs e)
        {
            base.OnDragDrop (e);
            if (DragTargetWidget != null)
            {
                DragTargetWidget.OnDragDrop(e);
            }
        }

        /// <summary>
        /// Handles drag feedback for the control.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnGiveFeedback(GiveFeedbackEventArgs e)
        {
            base.OnGiveFeedback(e);
            Cursor cursor = null;
            switch (e.Effect)
            {
                case DragDropEffects.Copy:
                    cursor = DragCopyCursor;
                    break;
                case DragDropEffects.Link:
                    cursor = DragLinkCursor;
                    break;
                case DragDropEffects.Move:
                    cursor = DragMoveCursor;
                    break;
                case DragDropEffects.None:
                    cursor = DragNoneCursor;
                    break;
                case DragDropEffects.Scroll:
                    cursor = DragScrollCursor;
                    break;
            }

            if (cursor == null)
            {
                e.UseDefaultCursors = true;
            }
            else
            {
                Cursor.Current = cursor;
                e.UseDefaultCursors = false;
            }

            // give the source widget a chance to handle feedback
            //
            if (DragSourceWidget != null)
            {
                DragSourceWidget.OnGiveFeedback(e);
            }

        }

        #endregion

	}
}
