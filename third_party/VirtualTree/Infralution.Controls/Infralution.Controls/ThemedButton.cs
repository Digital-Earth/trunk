#region File Header
//
//      FILE:   ThemedButton.cs.
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
using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;
using System.Windows.Forms.VisualStyles;

namespace Infralution.Controls
{
	/// <summary>
	/// Defines a base button class that supports drawing using Visual Styles  
	/// </summary>
	/// <remarks>
    /// Defines an owner drawn button class that uses Visual Styles to draw the button.  Unlike
    /// the standard Button class derived classes can specify the Visual Style elements to use 
    /// to draw the button.
	/// </remarks>
    [ToolboxItem(false)]
    public class ThemedButton : Button
	{

        #region Member Variables

        /// <summary>
        /// Is the button currently pressed
        /// </summary>
        private bool _pressed = false;

        /// <summary>
        /// Is the button hot (ie mouse over the button
        /// </summary>
        private bool _hot = false;

        #endregion

        #region Local Types and Methods

        protected enum VisualState : int
        {
            Normal = 1,
            Hot = 2,
            Pressed = 3,
            Disabled = 4,
            Defaulted = 5
        }

        protected virtual VisualStyleElement GetVisualStyleElement(VisualState visualState)
        {
            switch (visualState)
            {
                case VisualState.Normal:
                    return VisualStyleElement.Button.PushButton.Normal;
                case VisualState.Disabled:
                    return VisualStyleElement.Button.PushButton.Disabled;
                case VisualState.Hot:
                    return VisualStyleElement.Button.PushButton.Hot;
                case VisualState.Pressed:
                    return VisualStyleElement.Button.PushButton.Pressed;
                case VisualState.Defaulted:
                    return VisualStyleElement.Button.PushButton.Default;
            }
            return VisualStyleElement.Button.PushButton.Normal;
        }

        /// <summary>
        /// Paint the button background when using Visual Styles
        /// </summary>
        /// <param name="graphics">The graphics to draw to</param>
        /// <param name="visualState">The current visual state of the button</param>
        protected virtual void PaintBackground(Graphics graphics, VisualState visualState)
        { 
            graphics.Clear(BackColor);
            VisualStyleRenderer renderer = new VisualStyleRenderer(GetVisualStyleElement(visualState));
            renderer.DrawBackground(graphics, new Rectangle(0, 0, Width, Height));
        }

        /// <summary>
        /// Paint the button foreground when using Visual Styles
        /// </summary>
        /// <param name="graphics">The graphics to draw to</param>
        /// <param name="visualState">The current visual state of the button</param>
        protected virtual void PaintForeground(Graphics graphics, VisualState visualState)
        {
        }

        /// <summary>
        /// Draw the focus rectangle when using visual styles
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        protected virtual void PaintFocusRectangle(Graphics graphics)
        {
            const int offset = 3; 
            Rectangle rect = new Rectangle(offset, offset, 
                                           Width - 2 * offset, Height - 2 * offset);
            ControlPaint.DrawFocusRectangle(graphics, rect, ForeColor, BackColor);
        }

        /// <summary>
        /// Paints the control
        /// </summary>
        /// <param name="pe"></param>
		protected override void OnPaint(PaintEventArgs pe)
		{
            if (Application.RenderWithVisualStyles)
            {
                VisualState visualState = VisualState.Normal;
                if (!Enabled)
                    visualState = VisualState.Disabled;
                else if (_pressed)
                    visualState = VisualState.Pressed;
                else if (_hot)
                    visualState = VisualState.Hot;
                else if ((this.Focused && this.ShowFocusCues) || this.IsDefault)
                    visualState = VisualState.Defaulted;
                else
                    visualState = VisualState.Normal;

                PaintBackground(pe.Graphics, visualState);
                PaintForeground(pe.Graphics, visualState);

                if (this.Focused && this.ShowFocusCues)
                {
                    PaintFocusRectangle(pe.Graphics);
                }
            }
            else
            {
                base.OnPaint(pe);
            }
		}

        /// <summary>
        /// Handles a mouse down event
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseDown(MouseEventArgs e)
        {
            base.OnMouseDown (e);
            if (e.Button == MouseButtons.Left)
            {
                _pressed = true;
                Invalidate();
            }
        }

        /// <summary>
        /// Handles a mouse up event
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseUp(MouseEventArgs e)
        {
            base.OnMouseUp (e);
            _pressed = false;
            Invalidate();
        }

        /// <summary>
        /// Set the button to hot while the mouse is over the button
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseEnter(EventArgs e)
        {
            base.OnMouseEnter (e);
            _hot = true;
            Invalidate();
        }

        /// <summary>
        /// Set the button to not be hot when the mouse leaves the button
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseLeave(EventArgs e)
        {
            base.OnMouseLeave (e);
            _hot = false;
            Invalidate();
        }

        /// <summary>
        /// Invalidates the control to update the focus indicator
        /// </summary>
        /// <param name="e"></param>
        protected override void OnGotFocus(EventArgs e)
        {
            Invalidate();
            base.OnGotFocus (e);
        }

        /// <summary>
        /// Invalidates the control to update the focus indicator
        /// </summary>
        /// <param name="e"></param>
        protected override void OnLostFocus(EventArgs e)
        {
            Invalidate();
            base.OnLostFocus (e);
        }

        /// <summary>
        /// Invalidates the control when the UICues change
        /// </summary>
        /// <param name="e"></param>
        protected override void OnChangeUICues(UICuesEventArgs e)
        {
            Invalidate();
            base.OnChangeUICues (e);
        }

        #endregion
	}
}
