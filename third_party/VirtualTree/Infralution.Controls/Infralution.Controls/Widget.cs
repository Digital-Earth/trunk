#region File Header
//
//      FILE:   Widget.cs.
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
namespace Infralution.Controls
{

    /// <summary>
    /// Defines a base class for supporting light weight (windowless) visual elements which can draw 
    /// themselves on the <see cref="Graphics"/> context of a parent <see cref="WidgetControl"/> 
    /// and respond to mouse events routed to them by the parent control.
    /// </summary>
    /// <remarks>
    /// Building complex composite controls using standard <see cref="Control">Controls</see>  
    /// uses a lot of system resources and is somewhat unweildy.   This class facilitates the
    /// building of complex controls with a minimum of resources. 
    /// </remarks>
    /// <seealso cref="WidgetControl"/>
    public abstract class Widget : IDisposable
    {

        /// <summary>
        /// The control that this widget is associated with
        /// </summary>
        private WidgetControl _control;

        /// <summary>
        /// The parent widget if any
        /// </summary>
        private Widget _parentWidget;

        /// <summary>
        /// The child widgets of this widget (if any)
        /// </summary>
        private WidgetList _childWidgets = new WidgetList();

        /// <summary>
        /// The bounds of this widget
        /// </summary>
        private Rectangle _bounds;
 
        /// <summary>
        /// The right to left layout for this widget
        /// </summary>
        private RightToLeft _rightToLeft = RightToLeft.Inherit;

        /// <summary>
        /// Create a new widget
        /// </summary>
        /// <remarks>
        /// The <see cref="WidgetControl"/> or <see cref="ParentWidget"/> property must
        /// be set before using the control.
        /// </remarks>
        public Widget()
        {
        }

        /// <summary>
        /// Create a new widget for the given control
        /// </summary>
        /// <param name="control">The control the widget is associated with</param>
        public Widget(WidgetControl control)
        {
            _control = control;
        }

        /// <summary>
        /// Create a new widget parented by the given widget
        /// </summary>
        /// <param name="parentWidget">The parent widget</param>
        public Widget(Widget parentWidget)
        {
            ParentWidget = parentWidget;
        }

        /// <summary>
        /// Destructor calls Dispose(false)
        /// </summary>
        ~Widget()
        {
            Dispose(false);
        }

        /// <summary>
        /// Dispose of the widget.
        /// </summary>
        /// <remarks>
        /// Calls Dispose(disposing => true) and then suppresses finalize.
        /// </remarks>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Overridden by derived classes to dispose of resources used by the widget
        /// </summary>
        /// <remarks>
        /// If disposing is false the method is being called by the GC and the 
        /// overriding method should only dispose of unmanaged resources (ie it should
        /// not access other managed objects). 
        /// </remarks>
        /// <param name="disposing">True if not called by the GC</param>
        protected virtual void Dispose(bool disposing)
        {
        }

        /// <summary>
        /// Get/Set the control that this widget is drawn on
        /// </summary>
        public WidgetControl WidgetControl
        {
            get { return _control; }
            set { _control = value; }
        }

        /// <summary>
        /// Get/Set the parent widget for this widget (if any)
        /// </summary>
        public Widget ParentWidget
        {
            get { return _parentWidget; }
            set
            {
                _parentWidget = value;
                if (_parentWidget != null)
                {
                    _control = _parentWidget.WidgetControl;
                }
            }
        }

        /// <summary>
        /// The bounds in painting coordinates of this widget
        /// </summary>
        public Rectangle Bounds
        {
            get { return _bounds; }
            set 
            { 
                _bounds = value;
                OnLayout();
            }
        }

        /// <summary>
        /// Is this widget in a right to left control
        /// </summary>
        public RightToLeft RightToLeft
        {
            get 
            {   
                RightToLeft result = _rightToLeft;
                if (_rightToLeft == RightToLeft.Inherit)
                {
                    if (this._parentWidget != null)
                        result = _parentWidget.RightToLeft;
                    else if (this._control != null)
                        result = _control.RightToLeft;
                }
                return result; 
            }
            set
            {
                _rightToLeft = value;
            }
        }

        /// <summary>
        /// Return a widget x coordinate adjusted for <see cref="RightToLeft"/> positioning.
        /// </summary>
        /// <param name="x">The coordinate to adjust</param>
        /// <returns>The adjusted coordinate</returns>
        /// <remarks>
        /// If <see cref="RightToLeft"/> is set then this adjusts the coordinate to give right to left
        /// layout
        /// </remarks>
        protected virtual int RtlTranslateX(int x)
        {
            return RtlTranslateX(x, this.RightToLeft);
        }

        /// <summary>
        /// Return a point in widget coordinates adjusted for <see cref="RightToLeft"/> positioning.
        /// </summary>
        /// <param name="p">The point to adjust</param>
        /// <returns>The adjusted point</returns>
        /// <remarks>
        /// If <see cref="RightToLeft"/> is set then this adjusts the point to give right to left
        /// layout
        /// </remarks>
        protected virtual Point RtlTranslatePoint(Point p)
        {
            return RtlTranslatePoint(p, this.RightToLeft);
        }

        /// <summary>
        /// Return a rectangle in widget coordinates adjusted for <see cref="RightToLeft"/> positioning.
        /// </summary>
        /// <param name="r">The rectangle to adjust</param>
        /// <returns>The adjusted rectangle</returns>
        /// <remarks>
        /// If <see cref="RightToLeft"/> is set then this adjusts the rectangle to give right to left
        /// layout
        /// </remarks>
        protected virtual Rectangle RtlTranslateRect(Rectangle r)
        {
            return RtlTranslateRect(r, this.RightToLeft);
        }

        /// <summary>
        /// Return a widget x coordinate adjusted for <see cref="RightToLeft"/> positioning.
        /// </summary>
        /// <param name="x">The coordinate to adjust</param>
        /// <returns>The adjusted coordinate</returns>
        /// <param name="rightToLeft">Should layout be right to left</param>
        protected virtual int RtlTranslateX(int x, RightToLeft rightToLeft)
        {
            if (rightToLeft == RightToLeft.Yes)
            {
                x = _bounds.X + _bounds.Width - (x - _bounds.X) - 1;
            }
            return x;
        }

        /// <summary>
        /// Return a point in widget coordinates adjusted for <see cref="RightToLeft"/> positioning.
        /// </summary>
        /// <param name="p">The point to adjust</param>
        /// <param name="rightToLeft">Should layout be right to left</param>
        /// <returns>The adjusted point</returns>
        protected virtual Point RtlTranslatePoint(Point p, RightToLeft rightToLeft)
        {
            p.X = RtlTranslateX(p.X, rightToLeft);
            return p;
        }

        /// <summary>
        /// Return a rectangle in widget coordinates adjusted for <see cref="RightToLeft"/> positioning.
        /// </summary>
        /// <param name="r">The rectangle to adjust</param>
        /// <param name="rightToLeft">Should layout be right to left</param>
        /// <returns>The adjusted rectangle</returns>
        protected virtual Rectangle RtlTranslateRect(Rectangle r, RightToLeft rightToLeft)
        {
            if (rightToLeft == RightToLeft.Yes)
            {
                int x = _bounds.X + _bounds.Width - (r.X - _bounds.X) - r.Width;
                return new Rectangle(x, r.Y, r.Width, r.Height);
            }
            return r;
        }

        /// <summary>
        /// Test whether the given point is within the bounds of this widget
        /// </summary>
        /// <param name="x">The X coord of the point to test</param>
        /// <param name="y">The Y coord of the point to test</param>
        /// <returns>True if the bound is within the bounds of this widget</returns>
        public virtual bool IsHit(float x, float y)
        {
            return (x >= _bounds.Left) && (x < _bounds.Right)&&
                   (y >= _bounds.Top) && (y < _bounds.Bottom);
        }

        /// <summary>
        /// Hit Test this widget and all of its children/decsendants.   Returns the topmost
        /// descendant widget that is hit.
        /// </summary>
        /// <param name="x">The X coord of the point to test</param>
        /// <param name="y">The Y coord of the point to test</param>
        /// <returns>The topmost widget within this widget that is hit (if any)</returns>
        public virtual Widget GetHitWidget(float x, float y)
        {
            Widget hitWidget = null;
            if (IsHit(x, y))
            {
                hitWidget = this;

                // iterate through the list of child widgets in reverse order to 
                // find the topmost widget that is hit
                //
                for (int i = _childWidgets.Count - 1; i >= 0; i--)
                {
                    Widget childWidget = (Widget)_childWidgets[i];
                    Widget widget = childWidget.GetHitWidget(x, y);
                    if (widget != null)
                    {
                        hitWidget = widget;
                        break;
                    }
                }
            }
            return hitWidget;
        }

        /// <summary>
        /// Check if this widget will paint inside the given clip rectange
        /// </summary>
        /// <param name="clipRect">The clipping rectangle for the paint event</param>
        /// <returns>True if the widget requires painting</returns>
        public virtual bool PaintRequired(Rectangle clipRect)
        {            
            return Bounds.IntersectsWith(clipRect);
        }

        /// <summary>
        /// Invalidates the region of the control that this widget is on 
        /// </summary>
        public virtual void Invalidate()
        {
            WidgetControl.Invalidate(Bounds);
        }

        /// <summary>
        /// Force the control to repaint invalidated areas.
        /// </summary>
        public virtual void Update()
        {
            WidgetControl.Update();
        }

        /// <summary>
        /// Return the list of child widgets for this widget
        /// </summary>
        public WidgetList ChildWidgets
        {
            get { return _childWidgets; }
        }

        /// <summary>
        /// Handle layout events for this widget.  The base method simply calls
        /// the OnLayout method for each of the child widgets.
        /// </summary>
        public virtual void OnLayout()
        {
            // avoid any potential reentrancy problems associated with iterators
            //
            for (int i = 0; i < _childWidgets.Count; i++)
            {
                Widget widget = _childWidgets[i];
                widget.OnLayout();
            }
        }

        /// <summary>
        /// Print the contents of this widget to the given graphics context.
        /// </summary>
        /// <remarks>
        /// This method handles printing for the widget.  Typically this may call the same 
        /// methods as OnPaint.  The base method simply calls the OnPrint method for each 
        /// of the child widgets.
        /// </remarks>
        /// <param name="graphics">The graphics context to print to</param>
        public virtual void OnPrint(Graphics graphics)
        {
            // avoid any potential reentrancy problems associated with iterators
            //
            for (int i = 0; i < _childWidgets.Count; i++)
            {
                Widget widget = _childWidgets[i];
                widget.OnPrint(graphics);
            }
        }

        /// <summary>
        /// Handle paint events for this widget. 
        /// </summary>
        /// <remarks>
        /// The base method simply calls the OnPaint method for each of the child widgets.
        /// </remarks>
        /// <param name="graphics">The graphics context to paint to</param>
        public virtual void OnPaint(Graphics graphics)
        {
            // avoid any potential reentrancy problems associated with iterators
            //
            for (int i = 0; i < _childWidgets.Count; i++)
            {
                Widget widget = _childWidgets[i];
                widget.OnPaint(graphics);
            }
        }

        /// <summary>
        /// Called when the mouse is first moved over the widget
        /// </summary>
        public virtual void OnMouseEnter(MouseEventArgs e)
        {
        }

        /// <summary>
        /// Called when the mouse is leaves the widget
        /// </summary>
        public virtual void OnMouseLeave(EventArgs e)
        {
        }

        /// <summary>
        /// Handle MouseDown events for this Widget
        /// </summary>
        /// <param name="e"></param>
        public  virtual void OnMouseDown(MouseEventArgs e)
        {
        }

        /// <summary>
        /// Handle MouseUp events for this Widget.
        /// </summary>
        /// <param name="e"></param>
        public virtual void OnMouseUp(MouseEventArgs e)
        {
        }

        /// <summary>
        /// Handle MouseMove events for this Widget
        /// </summary>
        /// <param name="e"></param>
        public virtual void OnMouseMove(MouseEventArgs e)
        {
        }

        /// <summary>
        /// Handle Click events for this Widget
        /// </summary>
        /// <param name="e"></param>
        public virtual void OnClick(EventArgs e)
        {
        }

        /// <summary>
        /// Handle Double Click events for this Widget.
        /// </summary>
        /// <param name="e"></param>
        public virtual void OnDoubleClick(EventArgs e)
        {
        }

        /// <summary>
        /// Handle Drag Enter events for this widget
        /// </summary>
        /// <param name="e"></param>
        public virtual void OnDragEnter(DragEventArgs e)
        {
            e.Effect = DragDropEffects.None;
        }
 
        /// <summary>
        /// Handle Drag Leave events for this widget
        /// </summary>
        /// <param name="e"></param>
        public virtual  void OnDragLeave(EventArgs e)
        {
        }


        /// <summary>
        /// Handle Drag Over events for this widget
        /// </summary>
        /// <param name="e"></param>
        public virtual void OnDragOver(DragEventArgs e)
        {
        }
 
        /// <summary>
        /// Handle Drag Drop events for this widget
        /// </summary>
        /// <param name="e"></param>
        public virtual  void OnDragDrop(DragEventArgs e)
        {
        }

        /// <summary>
        /// Handle Drag Feedback events for this widget
        /// </summary>
        /// <param name="e"></param>
        public virtual void OnGiveFeedback(GiveFeedbackEventArgs e)
        {
        }

 
    }

}
