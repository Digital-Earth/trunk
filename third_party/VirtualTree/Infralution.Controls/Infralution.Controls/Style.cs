#region File Header
//
//      FILE:   Style.cs.
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
using System.Drawing.Drawing2D;
using System.Windows.Forms;
using System.ComponentModel;
namespace Infralution.Controls
{

    /// <summary>
    /// Defines a combination of colors (foreground, background, border) and other
    /// parameters used to draw a <see cref="Control"/> or <see cref="Widget"/>.
    /// </summary>
    /// <remarks>
    /// <para>The class provides a mechanism for deriving new styles from a parent style
    /// by specifying the differences from the parent style using a <see cref="StyleDelta"/>.
    /// Changes made to the derived style are stored and serialized in the <see cref="StyleDelta"/>.
    /// </para>
    /// <para>Typically the end user does not need to deal with
    /// the underlying <see cref="StyleDelta"/>.  It can, however, be extracted from the Style
    /// in order to create a new Style which applies the same delta to another parent style.
    /// This is useful for building relationships between styles. For instance a control may
    /// define default styles for a given component, say a Header widget.   The control
    /// would define a HeaderStyle and child styles HeaderPressedStyle and HeaderHotStyle which
    /// specify the default appearance of the header in different states.  If the user changes
    /// a property (eg font) of the default HeaderStyle this is automatically reflected in the child
    /// styles if they do not override this particular property.
    /// </para> 
    /// <para>An even more powerful application of this paradigm would allow the end user to override
    /// the Style properties for individual Header widgets.  To allow ultimate user flexibility the
    /// Header widget would define properties for each of the states:</para>
    /// <list type="definition">
    /// <item><term>HeaderWidget.Style</term><description>parented by Control.HeaderStyle</description></item>
    /// <item><term>HeaderWidget.PressedStyle</term><description>parented by HeaderWidget.Style</description></item>
    /// <item><term>HeaderWidget.HotStyle</term><description>parented by HeaderWidth.Style</description></item>
    /// </list>
    /// <para>If the HeaderWidget.PressedStyle were derived directly from the Control.HeaderPressedStyle then to change 
    /// the font (for instance) of a given header widget the user would be forced to change each of the widget styles.  
    /// However if the HeaderWidget.PressedStyle derived directly from HeaderWidget.Style then changes made by the user to the
    /// default Control.HeaderPressedStyle would have no effect.</para>
    /// <para>To overcome this difficulty the HeaderWidget.PressedStyle is not parented directly by the HeaderWidget.Style. 
    /// Instead an intermediate style is created which applies the <see cref="StyleDelta"/> of the
    /// Control.HeaderPressedStyle to the HeaderWidget.Style.  The HeaderWidget.PressedStyle is then parented
    /// by this intermediate style.  Changes to either the Control.HeaderPressedStyle or HeaderWidget.Style are
    /// now reflected automatically in the HeaderWidget.PressedStyle.   The Style class provides a constructor
    /// that simplifies the building of hierarchies such as this.</para>
    /// </remarks>
    /// <seealso cref="StyleDelta"/>
    [TypeConverter(typeof(ExpandableObjectConverter))]
    public class Style : INotifyPropertyChanged
    {
        #region Member Variables

        private Style _parentStyle = null;
        private StyleDelta _delta;

        #endregion

        #region Public Events

       /// <summary>
        /// Raised when the properties of the column are changed.
        /// </summary>
        [Description("Fired when the properties of the column are changed")]
    	public event PropertyChangedEventHandler PropertyChanged;
        #endregion

        #region Public Interface

        /// <summary>
        /// Create an root style with default values for all properties
        /// </summary>
        public Style()
        {
            _delta = new StyleDelta();
            _delta.PropertyChanged += new PropertyChangedEventHandler(OnPropertyChanged);
        }

        /// <summary>
        /// Create a derived style in which all properties default to given parent style property value
        /// </summary>
        /// <param name="parentStyle">The style to inherit from</param>
        public Style(Style parentStyle)
        {
            _parentStyle = parentStyle;           
            _delta = new StyleDelta();
            _delta.PropertyChanged += new PropertyChangedEventHandler(OnPropertyChanged);
        }

        /// <summary>
        /// Create a derived style with the specified differences to the parent style
        /// </summary>
        /// <param name="parentStyle">The style to inherit from</param>
        /// <param name="delta">A style delta specifying the differences from the parent style</param>
        public Style(Style parentStyle, StyleDelta delta)
        {
            _parentStyle = parentStyle;           
            _delta = delta;
            _delta.PropertyChanged += new PropertyChangedEventHandler(OnPropertyChanged);
        }

        /// <summary>
        /// Create a new root style based on the given style delta
        /// </summary>
        /// <param name="delta">A style delta specifying default properties</param>
        public Style(StyleDelta delta)
        {
            _delta = delta;
            _delta.PropertyChanged += new PropertyChangedEventHandler(OnPropertyChanged);
        }

        /// <summary>
        /// Set/Get the root style of this style.  
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public Style RootStyle
        {
            get 
            {
                Style style = this;
                while (style.ParentStyle != null)
                {
                    style = style.ParentStyle;
                }
                return style;
            }            
        }

        /// <summary>
        /// Copy this style and each of its ancestors up to the given style
        /// </summary>
        /// <param name="ancestorStyle">The style that is the ancestor in the relationship</param>
        /// <param name="newParentStyle">The style to make the parent of the copied hierarchy</param>
        /// <returns>The new style</returns>
        public Style Copy(Style ancestorStyle, Style newParentStyle) 
        {
            Style istyle = this;
            Style newStyle = null;
            Style prevNewStyle = null;
            Style style = null;
            while (istyle != ancestorStyle && istyle != null)
            {   
                // create a copy of this style which uses the same delta
                //
                newStyle = new Style(istyle.Delta);

                // fix the parentage of the previous style up
                //
                if (prevNewStyle != null)
                    prevNewStyle.ParentStyle = newStyle;
                
                // remember the style to return
                //
                if (style == null)
                    style = newStyle;
                prevNewStyle = newStyle;
                istyle = istyle.ParentStyle;
            }
            newStyle.ParentStyle = newParentStyle;
            return style;
        }

        /// <summary>
        /// Copy this style and each of its ancestors up to the given style
        /// </summary>
        /// <param name="ancestorStyle">The style to stop copying at</param>
        /// <returns>A new style hierarchy which uses the same Style Deltas</returns>
        public Style Copy(Style ancestorStyle)
        {
            return Copy(ancestorStyle, null);
        }

        /// <summary>
        /// Copy this style and all of its ancestors
        /// </summary>
        /// <returns>A new style hierarchy which uses the same Style Deltas</returns>
        public Style Copy()
        {
            return Copy(null, null);
        }

        /// <summary>
        /// Set/Get the parent style of this style.  
        /// </summary>
        /// <remarks>
        /// This is an internal method and should not in general be called by application code.
        /// </remarks>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public Style ParentStyle
        {
            get { return _parentStyle; }
            set { _parentStyle = value; }
        }

        /// <summary>
        /// Set/Get the changes applied by this style to its parent style.
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public StyleDelta Delta
        {
            get { return _delta; }
            set
            {
                if (value == null) throw new ArgumentNullException();
                _delta = value;
            }
        }

        /// <summary>
        /// The foreground color to use when drawing the item
        /// </summary>
        [Description("The foreground color to use when drawing the item")]
        [RefreshProperties(RefreshProperties.Repaint)] 
        [Category("Appearance")]
        [NotifyParentProperty(true)]   
        public Color ForeColor
        {
            get 
            { 
                return (Delta.UseDefaultForeColor && ParentStyle != null) 
                    ? ParentStyle.ForeColor : Delta.ForeColor;
            }
            set
            {
                Delta.ForeColor = value;
            }
        }

        /// <summary>
        /// Reset the ForeColor to the default value
        /// </summary>
        public void ResetForeColor()
        {
            Delta.ResetForeColor();
        }
        
        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeForeColor()
        {
            return !Delta.UseDefaultForeColor;
        }
 
        /// <summary>
        /// The background color to use when drawing the item
        /// </summary>
        [Description("The background color to use when drawing the item")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Category("Appearance")]
        [NotifyParentProperty(true)]   
        public Color BackColor
        {
            get 
            { 
                return (Delta.UseDefaultBackColor && ParentStyle != null) 
                    ? ParentStyle.BackColor : Delta.BackColor;
            }
            set
            {
                Delta.BackColor = value;
            }
        }

        /// <summary>
        /// Reset the BackColor to the default value
        /// </summary>
        public void ResetBackColor()
        {
            Delta.ResetBackColor();
        }
        
        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeBackColor()
        {
            return !Delta.UseDefaultBackColor;
        }

        /// <summary>
        /// The second color to use when drawing gradient filled backgrounds
        /// </summary>
        [Description("The second color to use when drawing gradient filled backgrounds")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Category("Appearance")]
        [NotifyParentProperty(true)]   
        public Color GradientColor
        {
            get 
            { 
                return (Delta.UseDefaultGradientColor && ParentStyle != null) 
                    ? ParentStyle.GradientColor : Delta.GradientColor;
            }
            set
            {
                Delta.GradientColor = value;
            }
        }

        /// <summary>
        /// Reset the GradientColor to the default value
        /// </summary>
        public void ResetGradientColor()
        {
            Delta.ResetGradientColor();
        }
        
        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeGradientColor()
        {
            return !Delta.UseDefaultGradientColor;
        }

        /// <summary>
        /// The gradient mode to use when drawing gradient filled backgrounds
        /// </summary>
        /// <seealso cref="GradientColor"/>
        [Description("The gradient mode to use when drawing gradient filled backgrounds")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Category("Appearance")]
        [NotifyParentProperty(true)]   
        public LinearGradientMode GradientMode
        {
            get 
            { 
                return (Delta.UseDefaultGradientMode && ParentStyle != null) 
                    ? ParentStyle.GradientMode : Delta.GradientMode;
            }
            set
            {
                Delta.GradientMode = value;
            }
        }

        /// <summary>
        /// Reset the GradientMode to the default value
        /// </summary>
        public void ResetGradientMode()
        {
            Delta.ResetGradientMode();
        }
        
        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeGradientMode()
        {
            return !Delta.UseDefaultGradientMode;
        }

        /// <summary>
        /// The alpha blending factor to use when drawing the background (0 = transparent, 255 = opaque)  
        /// </summary>
        [Description("The alpha blending factor to use when drawing the background (0 = transparent, 255 = opaque)")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Category("Appearance")]
        [NotifyParentProperty(true)]   
        public byte AlphaBlend
        {
            get 
            { 
                return (Delta.UseDefaultAlphaBlend && ParentStyle != null) 
                    ? ParentStyle.AlphaBlend : Delta.AlphaBlend;
            }
            set
            {
                Delta.AlphaBlend = value;
            }
        }

        /// <summary>
        /// Reset the AlphaBlend to the default value
        /// </summary>
        public void ResetAlphaBlend()
        {
            Delta.ResetAlphaBlend();
        }
        
        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeAlphaBlend()
        {
            return !Delta.UseDefaultAlphaBlend;
        }

        /// <summary>
        /// The border color to use when drawing the item
        /// </summary>
        [Description("The border color to use when drawing the item")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Category("Appearance")]
        [NotifyParentProperty(true)]   
        public Color BorderColor
        {
            get 
            { 
                return (Delta.UseDefaultBorderColor && ParentStyle != null) 
                    ? ParentStyle.BorderColor : Delta.BorderColor;
            }
            set
            {
                Delta.BorderColor = value;
            }
        }

        /// <summary>
        /// Reset the BorderColor to the default value
        /// </summary>
        public void ResetBorderColor()
        {
            Delta.ResetBorderColor();
        }
        
        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeBorderColor()
        {
            return !Delta.UseDefaultBorderColor;
        }

        /// <summary>
        /// The width of the border to use when drawing the item
        /// </summary>
        [Description("The border width to use when drawing the item")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Category("Appearance")]
        [NotifyParentProperty(true)]   
        public int BorderWidth
        {
            get 
            { 
                return (Delta.UseDefaultBorderWidth && ParentStyle != null) 
                    ? ParentStyle.BorderWidth : Delta.BorderWidth;
            }
            set
            {
                Delta.BorderWidth = value;
            }
        }

        /// <summary>
        /// Reset the BorderWidth to the default value
        /// </summary>
        public void ResetBorderWidth()
        {
            Delta.ResetBorderWidth();
        }
        
        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeBorderWidth()
        {
            return !Delta.UseDefaultBorderWidth;
        }

        /// <summary>
        /// The border style to use
        /// </summary>
        [Description("The border style to use to draw border")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Category("Appearance")]
        [NotifyParentProperty(true)]   
        public Border3DStyle BorderStyle
        {
            get 
            { 
                return (Delta.UseDefaultBorderStyle && ParentStyle != null) 
                    ? ParentStyle.BorderStyle : Delta.BorderStyle;
            }
            set
            {
                Delta.BorderStyle = value;
            }
        }

        /// <summary>
        /// Reset the BorderStyle to the default value
        /// </summary>
        public void ResetBorderStyle()
        {
            Delta.ResetBorderStyle();
        }
        
        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeBorderStyle()
        {
            return !Delta.UseDefaultBorderStyle;
        }

        /// <summary>
        /// The sides to draw borders on
        /// </summary>
        [Description("The sides to draw borders on")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Category("Appearance")]
        [NotifyParentProperty(true)]
        public Border3DSide BorderSide
        {
            get
            {
                return (Delta.UseDefaultBorderSide && ParentStyle != null)
                    ? ParentStyle.BorderSide : Delta.BorderSide;
            }
            set
            {
                Delta.BorderSide = value;
            }
        }

        /// <summary>
        /// Reset the BorderSide to the default value
        /// </summary>
        public void ResetBorderSide()
        {
            Delta.ResetBorderSide();
        }

        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeBorderSide()
        {
            return !Delta.UseDefaultBorderSide;
        }

        /// <summary>
        /// The radius to use for rounding the border and background rectangles.
        /// A value of zero indicates normal rectangular borders.
        /// </summary>
        [Description("The radius to use for rounding the border and background rectangles (zero is non-rounded)")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Category("Appearance")]
        [NotifyParentProperty(true)]
        public int BorderRadius
        {
            get
            {
                return (Delta.UseDefaultBorderRadius && ParentStyle != null)
                    ? ParentStyle.BorderRadius : Delta.BorderRadius;
            }
            set
            {
                Delta.BorderRadius = value;
            }
        }

        /// <summary>
        /// Reset the BorderRadius to the default value
        /// </summary>
        public void ResetBorderRadius()
        {
            Delta.ResetBorderRadius();
        }

        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeBorderRadius()
        {
            return !Delta.UseDefaultBorderRadius;
        }

        /// <summary>
        /// The font to use when drawing the item
        /// </summary>
        [Description("The font to use when drawing the item")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Category("Appearance")]
        [NotifyParentProperty(true)]   
        [Localizable(true)]
        public Font Font
        {
            get 
            { 
                return (Delta.UseDefaultFont && ParentStyle != null) 
                    ? ParentStyle.Font : Delta.Font;
            }
            set
            {
                Delta.Font = value;
            }
        }

        /// <summary>
        /// Reset the Font to the default value
        /// </summary>
        public void ResetFont()
        {
            Delta.ResetFont();
        }
        
        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeFont()
        {
            return !Delta.UseDefaultFont;
        }

        /// <summary>
        /// The horizontal alignment to use when drawing text
        /// </summary>
        [Description("The horizontal alignment to use when drawing text")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Category("Appearance")]
        [NotifyParentProperty(true)]   
        [Localizable(true)]
        public StringAlignment HorzAlignment
        {
            get 
            { 
                return (Delta.UseDefaultHorzAlignment && ParentStyle != null) 
                    ? ParentStyle.HorzAlignment : Delta.HorzAlignment;
            }
            set
            {
                Delta.HorzAlignment = value;
            }
        }

        /// <summary>
        /// Reset the Horizontal Alignment to the default value
        /// </summary>
        public void ResetHorzAlignment()
        {
            Delta.ResetHorzAlignment();
        }
        
        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeHorzAlignment()
        {
            return !Delta.UseDefaultHorzAlignment;
        }

        /// <summary>
        /// The vertical alignment to use when drawing text
        /// </summary>
        [Description("The vertical alignment to use when drawing text")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Category("Appearance")]
        [NotifyParentProperty(true)]   
        [Localizable(true)]
        public StringAlignment VertAlignment
        {
            get 
            { 
                return (Delta.UseDefaultVertAlignment && ParentStyle != null) 
                    ? ParentStyle.VertAlignment : Delta.VertAlignment;
            }
            set
            {
                Delta.VertAlignment = value;
            }
        }

        /// <summary>
        /// Reset the Vertical Alignment to the default value
        /// </summary>
        public void ResetVertAlignment()
        {
            Delta.ResetVertAlignment();
        }
        
        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeVertAlignment()
        {
            return !Delta.UseDefaultVertAlignment;
        }

        /// <summary>
        /// Is the layout of this element right to left
        /// </summary>
        [Description("Is the layout of this element right to left")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Category("Appearance")]
        [NotifyParentProperty(true)]   
        [Localizable(true)]
        public RightToLeft RightToLeft
        {
            get 
            { 
                return (Delta.UseDefaultRightToLeft && ParentStyle != null) 
                    ? ParentStyle.RightToLeft : Delta.RightToLeft;
            }
            set
            {
                Delta.RightToLeft = value;
            }
        }

        /// <summary>
        /// Reset the RightToLeft to the default value
        /// </summary>
        public void ResetRightToLeft()
        {
            Delta.ResetRightToLeft();
        }
        
        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeRightToLeft()
        {
            return !Delta.UseDefaultRightToLeft;
        }

        
        /// <summary>
        /// Should text be word wrapped inside the display rectangle
        /// </summary>
        [Description("Should text be word wrapped inside the display rectangle")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Category("Appearance")]
        [NotifyParentProperty(true)]   
        [Localizable(true)]
        public bool WordWrap
        {
            get 
            { 
                return (Delta.UseDefaultWordWrap && ParentStyle != null) 
                    ? ParentStyle.WordWrap : Delta.WordWrap;
            }
            set
            {
                Delta.WordWrap = value;
            }
        }

        /// <summary>
        /// Reset the WordWrap to the default value
        /// </summary>
        public void ResetWordWrap()
        {
            Delta.ResetWordWrap();
        }
        
        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeWordWrap()
        {
            return !Delta.UseDefaultWordWrap;
        }

        /// <summary>
        /// How should strings be trimmed if they don't fit the display area
        /// </summary>
        [Description("How should strings be trimmed if they don't fit the display area")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Category("Appearance")]
        [NotifyParentProperty(true)]
        [Localizable(true)]
        public StringTrimming StringTrimming
        {
            get
            {
                return (Delta.UseDefaultStringTrimming && ParentStyle != null)
                    ? ParentStyle.StringTrimming : Delta.StringTrimming;
            }
            set
            {
                Delta.StringTrimming = value;
            }
        }

        /// <summary>
        /// Reset the StringTrimming to the default value
        /// </summary>
        public void ResetStringTrimming()
        {
            Delta.ResetStringTrimming();
        }

        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeStringTrimming()
        {
            return !Delta.UseDefaultStringTrimming;
        }

        /// <summary>
        /// Should GDI+ be used for rendering text
        /// </summary>
        /// <remarks>
        /// If this property is true then the GDI+ based Graphics.DrawString methods are used for
        /// rendering text otherwise the GDI based TextRenderer methods are used to render text.
        /// In general the GDI methods are faster unless <see cref="WordWrap"/> is turned on.  
        /// With <see cref="WordWrap"/> set to true GDI+ text renderering may be 
        /// significantly faster.  You can set the default value for all new styles created by
        /// setting the static (shared) <see cref="CompatibleTextRenderingDefault"/> property.
        /// </remarks>
        [Description("Should GDI+ be used for rendering text")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Category("Appearance")]
        [NotifyParentProperty(true)]
        [Localizable(true)]
        public bool CompatibleTextRendering
        {
            get
            {
                return (Delta.UseDefaultCompatibleTextRendering && ParentStyle != null)
                    ? ParentStyle.CompatibleTextRendering : Delta.CompatibleTextRendering;
            }
            set
            {
                Delta.CompatibleTextRendering = value;
            }
        }

        /// <summary>
        /// Reset the WordWrap to the default value
        /// </summary>
        public void ResetCompatibleTextRendering()
        {
            Delta.ResetCompatibleTextRendering();
        }

        /// <summary>
        /// Called by framework to determine whether the property should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializeCompatibleTextRendering()
        {
            return !Delta.UseDefaultCompatibleTextRendering;
        }

        /// <summary>
        /// Set the default value for the <see cref="CompatibleTextRendering"/>
        /// property of new styles.
        /// </summary>
        /// <remarks>
        /// For backward compatibility the default value is true
        /// </remarks>
        public static bool CompatibleTextRenderingDefault
        {
            get { return StyleDelta.CompatibleTextRenderingDefault; }
            set
            {
                StyleDelta.CompatibleTextRenderingDefault = value;
            }
        }

        /// <summary>
        /// Returns true if the style requires serialization
        /// </summary>
        /// <returns></returns>
        public bool ShouldSerialize()
        {
            return !Delta.IsEmpty();
        }

        /// <summary>
        /// Reset the default values for all properties
        /// </summary>
        public void Reset()
        {
           Delta.Reset();
        }

        /// <summary>
        /// Return the bounds of a rectangle adjusting for the border width
        /// </summary>
        /// <param name="rect">The rectangle to get the border rectangle for</param>
        /// <returns>The bounds of the inner border</returns>
        public Rectangle BorderRect(Rectangle rect)
        {
            Border3DSide sides = BorderSide;
            int borderWidth = BorderWidth;
            
            if (BorderStyle == Border3DStyle.Adjust)
            {
                borderWidth = borderWidth / 2;
            }

            if ((sides & Border3DSide.Left) == Border3DSide.Left)
            {
                rect.X += borderWidth;
                rect.Width -= borderWidth;
            }
            if ((sides & Border3DSide.Top) == Border3DSide.Top)
            {
                rect.Y += borderWidth;
                rect.Height -= borderWidth;
            }
            if ((sides & Border3DSide.Right) == Border3DSide.Right)
            {
                rect.Width -= borderWidth;
            }
            if ((sides & Border3DSide.Bottom) == Border3DSide.Bottom)
            {
                rect.Height -= borderWidth;
            }
            return rect;
        }

        /// <summary>
        /// Return the bounds of a rectangle adjusting for the border width
        /// </summary>
        /// <param name="rectF">The rectangle to get the border rectangle for</param>
        /// <returns>The bounds of the inner border</returns>
        public RectangleF BorderRect(RectangleF rectF)
        {
            Rectangle rect = BorderRect(new Rectangle((int)rectF.X, (int)rectF.Y, (int)rectF.Width, (int)rectF.Height));
            return new RectangleF(rect.X, rect.Y, rect.Width, rect.Height);
        }


        /// <summary>
        /// Draw the border for the given rectangle using the current style attributes
        /// </summary>
        /// <remarks>
        /// This method provides more control when drawing borders with <see cref="BorderStyle"/>
        /// set to <see cref="Border3DStyle.Adjust"/>.  When using this style the border will be drawn
        /// to overlap the adjacent rectangle so that if both are drawn using the same style then the
        /// width of the border between them is only that of single <see cref="BorderWidth"/>. 
        /// </remarks>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="bounds">The rectangle to draw the border for</param>
        public void DrawBorder(Graphics graphics, Rectangle bounds)
        {
            int borderWidth = BorderWidth;
            if (borderWidth <= 0) return;

            Border3DStyle borderStyle = BorderStyle;
            switch (borderStyle)
            {
                case Border3DStyle.Flat:
                    DrawBorder(graphics, bounds, BorderSide, borderWidth, BorderColor, true);
                    break;
                case Border3DStyle.Adjust:
                    DrawBorder(graphics, bounds, BorderSide, borderWidth, BorderColor, false);
                    break;
                default:
                    Draw3DBorder(graphics, bounds, borderStyle);
                    break;
            }
        }

        /// <summary>
        /// Draw the background for an area using this styles settings
        /// </summary>
        /// <param name="graphics">The context to draw the border to</param>
        /// <param name="bounds">The bounds of the element</param>
        public void DrawBackground(Graphics graphics, Rectangle bounds)
        {
            if (bounds.Width <= 0 || bounds.Height <= 0) return;

            Brush brush;
            byte alpha = AlphaBlend;
            Color color = Color.FromArgb(alpha, BackColor);       
            Color gradientColor = GradientColor;
            if (gradientColor != Color.Empty)
            {
                gradientColor = Color.FromArgb(alpha, gradientColor);
                brush = new LinearGradientBrush(bounds, color, gradientColor, GradientMode);
            }
            else
            {
                brush = new SolidBrush(color);
            }
            int borderRadius = BorderRadius;
            if (borderRadius == 0)
                graphics.FillRectangle(brush, bounds);
            else
            {
                bounds.Width -= 1;
                DrawingUtilities.FillRoundedRect(graphics, brush, null, bounds, borderRadius);
            }
            brush.Dispose();
        }
       
        /// <summary>
        /// Draw a string using this style to the given graphics context.
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="text">The text to draw</param>
        /// <param name="bounds">The bounds of the text rectangle</param>
        /// <param name="printing">Is the text being drawn to a printer context</param>
        public void DrawString(Graphics graphics, String text, Rectangle bounds, bool printing)
        {
            if (CompatibleTextRendering)
                DrawStringGDIPlus(graphics, text, bounds);
            else
                DrawStringGDI(graphics, text, bounds, printing);
        }

        /// <summary>
        /// Return the size needed to display the given string without clipping
        /// </summary>
        /// <param name="graphics">The context being used</param>
        /// <param name="text">The text to find the size of</param>
        /// <returns>The size required to display the given string</returns>
        public Size MeasureString(Graphics graphics, String text)
        {
            return (CompatibleTextRendering) ? 
                MeasureStringGDIPlus(graphics, text) : MeasureStringGDI(graphics, text);
        }


        /// <summary>
        /// Return the size needed to display the given string without clipping
        /// </summary>
        /// <param name="graphics">The context being used</param>
        /// <param name="text">The text to find the size of</param>
        /// <param name="maxWidth">The maximum allowed width for the string</param>
        /// <returns>The size required to display the given string</returns>
        public Size MeasureString(Graphics graphics, String text, int maxWidth)
        {
            return (CompatibleTextRendering) ?
                MeasureStringGDIPlus(graphics, text, maxWidth) : MeasureStringGDI(graphics, text, maxWidth);
        }

        /// <summary>
        /// Return true if the text can be drawn in the given area without clipping
        /// </summary>
        /// <param name="graphics">The context to draw the string to</param>
        /// <param name="text">The text drawn</param>
        /// <param name="bounds">The bounds to test</param>
        /// <returns>True if the text can be drawn without clipping</returns>
        public bool StringFits(Graphics graphics, String text, Rectangle bounds)
        {
            return (CompatibleTextRendering) ?
                StringFitsGDIPlus(graphics, text, bounds) : StringFitsGDI(graphics, text, bounds);
        }

        /// <summary>
        /// Fill the given rectangle with the background color
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="rect">The rectangle to fill</param>
        public void FillRectangle(Graphics graphics, Rectangle rect)
        {
            Brush brush = new SolidBrush(BackColor);
            graphics.FillRectangle(brush, rect);
            brush.Dispose();
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Return the font to use when drawing text to a GDI printer context using <see cref="TextRenderer"/>
        /// </summary>
        /// <param name="graphics">The graphics context</param>
        /// <param name="font">The font</param>
        /// <returns>The scaled printer font</returns>
        protected Font GetPrinterFont(Graphics graphics, Font font)
        {
            return new Font(
               font.FontFamily, (float)font.SizeInPoints / 72f *
               graphics.DpiY, font.Style, GraphicsUnit.Pixel,
               font.GdiCharSet, font.GdiVerticalFont);
        }

        /// <summary>
        /// Draw a string using this style to the given graphics context using GDI
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="text">The text to draw</param>
        /// <param name="bounds">The bounds of the text rectangle</param>
        /// <param name="printing">Is the text being drawn to a printer context</param>
        protected void DrawStringGDI(Graphics graphics, String text, Rectangle bounds, bool printing)
        {
            Rectangle innerBounds = BorderRect(bounds);
            TextFormatFlags flags = GetAlignmentFlags();
            switch (this.StringTrimming)
            {
                case StringTrimming.Character:
                    flags |= TextFormatFlags.NoFullWidthCharacterBreak;
                    break;
                case StringTrimming.EllipsisCharacter:
                    flags |= TextFormatFlags.EndEllipsis;
                    break;
                case StringTrimming.EllipsisPath:
                    flags |= TextFormatFlags.PathEllipsis;
                    break;
                case StringTrimming.EllipsisWord:
                    flags |= TextFormatFlags.WordEllipsis;
                    break;
            }
            Font font = Font;
            if (printing) 
            {
                innerBounds = DrawingUtilities.TransformRect(graphics, CoordinateSpace.Device, CoordinateSpace.Page, innerBounds);
                font = GetPrinterFont(graphics, font);
            }
            TextRenderer.DrawText(graphics, text, font, innerBounds, ForeColor, flags);
        }

        /// <summary>
        /// Return the size needed to display the given string without clipping using GDI
        /// </summary>
        /// <param name="graphics">The context being used</param>
        /// <param name="text">The text to find the size of</param>
        /// <returns>The size required to display the given string</returns>
        protected Size MeasureStringGDI(Graphics graphics, String text)
        {
            return TextRenderer.MeasureText(graphics, text, Font);
        }

        /// <summary>
        /// Return the size needed to display the given string without clipping using GDI
        /// </summary>
        /// <param name="graphics">The context being used</param>
        /// <param name="text">The text to find the size of</param>
        /// <param name="maxWidth">The maximum allowed width for the string</param>
        /// <returns>The size required to display the given string</returns>
        protected Size MeasureStringGDI(Graphics graphics, String text, int maxWidth)
        {
            Size size;
            if (WordWrap)
            {
                Size requiredSize = new Size(maxWidth - 2 * BorderWidth, 0);
                size = TextRenderer.MeasureText(graphics, text, Font, requiredSize, GetAlignmentFlags());
            }
            else
            {
                size = TextRenderer.MeasureText(graphics, text, Font);
            }
            return size;
        }

        /// <summary>
        /// Return true if the text can be drawn in the given area without clipping using GDI
        /// </summary>
        /// <param name="graphics">The context to draw the string to</param>
        /// <param name="text">The text drawn</param>
        /// <param name="bounds">The bounds to test</param>
        /// <returns>True if the text can be drawn without clipping</returns>
        protected bool StringFitsGDI(Graphics graphics, String text, Rectangle bounds)
        {
            Rectangle innerBounds = BorderRect(bounds);
            SizeF size;
            if (WordWrap)
            {
                size = TextRenderer.MeasureText(graphics, text, Font, innerBounds.Size, GetAlignmentFlags());
            }
            else
            {
                size = TextRenderer.MeasureText(graphics, text, Font);
            }
            return size.Width <= innerBounds.Width && size.Height <= innerBounds.Height;
        }

        /// <summary>
        /// Draw a string using this style to the given graphics context using GDI+
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="text">The text to draw</param>
        /// <param name="bounds">The bounds of the text rectangle</param>
        protected void DrawStringGDIPlus(Graphics graphics, String text, Rectangle bounds)
        {
            const int MAX_TEXT_LENGTH = 32000;

            RectangleF innerBounds = BorderRect(bounds);
            StringFormat format = new StringFormat();
            Brush brush = new SolidBrush(ForeColor);
            if (!this.WordWrap)
            {
                format.FormatFlags = StringFormatFlags.NoWrap;
            }
            format.Trimming = this.StringTrimming;
            if (this.RightToLeft == RightToLeft.Yes)
            {
                format.FormatFlags |= StringFormatFlags.DirectionRightToLeft;
            }
            format.Alignment = HorzAlignment;
            format.LineAlignment = VertAlignment;

            // GDI+ DrawString throws an exception if the string is longer than a certain length
            //
            if (text != null && text.Length > MAX_TEXT_LENGTH)
            {
                text = text.Substring(0, MAX_TEXT_LENGTH);
            }

            graphics.DrawString(text, Font, brush, innerBounds, format);
            brush.Dispose();
            format.Dispose();
        }

        /// <summary>
        /// Return the size needed to display the given string without clipping using GDI+
        /// </summary>
        /// <param name="graphics">The context being used</param>
        /// <param name="text">The text to find the size of</param>
        /// <returns>The size required to display the given string</returns>
        protected Size MeasureStringGDIPlus(Graphics graphics, String text)
        {
            SizeF size = graphics.MeasureString(text, Font);
            return new Size((int)size.Width, (int)size.Height);
        }


        /// <summary>
        /// Return the size needed to display the given string without clipping using GDI+
        /// </summary>
        /// <param name="graphics">The context being used</param>
        /// <param name="text">The text to find the size of</param>
        /// <param name="maxWidth">The maximum allowed width for the string</param>
        /// <returns>The size required to display the given string</returns>
        protected Size MeasureStringGDIPlus(Graphics graphics, String text, int maxWidth)
        {
            SizeF size;
            if (WordWrap)
            {
                StringFormat format = new StringFormat();
                format.Alignment = HorzAlignment;
                format.LineAlignment = VertAlignment;
                maxWidth -= 2 * BorderWidth;
                size = graphics.MeasureString(text, Font, maxWidth, format);
            }
            else
            {
                size = graphics.MeasureString(text, Font);
            }
            return new Size((int)size.Width, (int)size.Height);
        }

        /// <summary>
        /// Return true if the text can be drawn in the given area without clipping using GDI+
        /// </summary>
        /// <param name="graphics">The context to draw the string to</param>
        /// <param name="text">The text drawn</param>
        /// <param name="bounds">The bounds to test</param>
        /// <returns>True if the text can be drawn without clipping</returns>
        protected bool StringFitsGDIPlus(Graphics graphics, String text, Rectangle bounds)
        {
            RectangleF innerBounds = BorderRect(bounds);
            SizeF size;
            if (WordWrap)
            {
                StringFormat format = new StringFormat();
                format.Alignment = HorzAlignment;
                format.LineAlignment = VertAlignment;
                size = graphics.MeasureString(text, Font, (int)innerBounds.Width, format);
            }
            else
            {
                size = graphics.MeasureString(text, Font);
            }
            return size.Width <= innerBounds.Width && size.Height <= innerBounds.Height;
        }


        /// <summary>
        /// Convert StringAlignment to TextFormatFlags
        /// </summary>
        /// <returns>The TextFormatFlags to use</returns>
        protected TextFormatFlags GetAlignmentFlags()
        {
            TextFormatFlags flags = TextFormatFlags.Default;
            bool rightToLeft = (this.RightToLeft == RightToLeft.Yes);
            if (rightToLeft)
            {
                flags |= TextFormatFlags.RightToLeft;
            }
            switch (HorzAlignment)
            {
                case StringAlignment.Near:
                    flags |= (rightToLeft) ? TextFormatFlags.Right : TextFormatFlags.Left;
                    break;
                case StringAlignment.Far:
                    flags |= (rightToLeft) ? TextFormatFlags.Left : TextFormatFlags.Right;
                    break;
                case StringAlignment.Center:
                    flags |= TextFormatFlags.HorizontalCenter;
                    break;
            }
            switch (VertAlignment)
            {
                case StringAlignment.Near:
                    flags |= TextFormatFlags.Top;
                    break;
                case StringAlignment.Far:
                    flags |= TextFormatFlags.Bottom;
                    break;
                case StringAlignment.Center:
                    flags |= TextFormatFlags.VerticalCenter;
                    break;
            }
            if (this.WordWrap)
            {
                flags |= TextFormatFlags.WordBreak;
            }
            return flags;
        }

        /// <summary>
        /// Handle notification of changes from the style delta this style uses
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        protected virtual void OnPropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, e);
            }
        }

        /// <summary>
        /// Draw a border using the given parameters
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="bounds">The bounds to draw the border for</param>
        /// <param name="sides">The sides to draw to</param>
        /// <param name="borderWidth">The width of the border</param>
        /// <param name="color">The color of the border</param>
        /// <param name="insetBorder">Should the border be drawn inset</param>
        protected void DrawBorder(Graphics graphics, Rectangle bounds, Border3DSide sides, int borderWidth, 
                                  Color color, bool insetBorder)
        {
            Rectangle innerBounds = bounds;
            if (insetBorder)
            {
                int halfBorder = borderWidth / 2;

                innerBounds.X += halfBorder;
                innerBounds.Y += halfBorder;
                innerBounds.Width -= borderWidth;
                innerBounds.Height -= borderWidth;
            }

            using (Pen pen = new Pen(color, borderWidth))
            {
                pen.Alignment = PenAlignment.Center;
                if (sides == Border3DSide.All)
                {
                    int borderRadius = BorderRadius;
                    if (borderRadius == 0)
                    {
                        graphics.DrawRectangle(pen, innerBounds);
                    }
                    else
                    {
                        DrawingUtilities.DrawRoundedRect(graphics, pen, innerBounds, borderRadius);
                    }
                }
                else
                {
                    bool rightToLeft = (this.RightToLeft == RightToLeft.Yes);
                    if ((sides & Border3DSide.Left) == Border3DSide.Left)
                    {
                        if (rightToLeft)
                            graphics.DrawLine(pen, innerBounds.Right, bounds.Top, innerBounds.Right, bounds.Bottom);
                        else
                            graphics.DrawLine(pen, innerBounds.Left, bounds.Top, innerBounds.Left, bounds.Bottom);
                    }
                    if ((sides & Border3DSide.Right) == Border3DSide.Right)
                    {
                        if (rightToLeft)
                            graphics.DrawLine(pen, innerBounds.Left, bounds.Top, innerBounds.Left, bounds.Bottom);
                        else
                            graphics.DrawLine(pen, innerBounds.Right, bounds.Top, innerBounds.Right, bounds.Bottom);
                    }
                    if ((sides & Border3DSide.Bottom) == Border3DSide.Bottom)
                    {
                        graphics.DrawLine(pen, bounds.Left, innerBounds.Bottom, bounds.Right, innerBounds.Bottom);
                    }
                    if ((sides & Border3DSide.Top) == Border3DSide.Top)
                    {
                        graphics.DrawLine(pen, bounds.Left, innerBounds.Top, bounds.Right, innerBounds.Top);
                    }
                }
            }
        }

        /// <summary>
        /// Draw a 3D border around the given rectangle
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="bounds">The rectangle to draw the border around</param>
        /// <param name="style">The style of border</param>
        protected void Draw3DBorder(Graphics graphics, Rectangle bounds, Border3DStyle style)
        {
            // ControlPaint only works in device coordinates so we need to transform
            //
            bounds = DrawingUtilities.TransformRect(graphics, CoordinateSpace.Device, CoordinateSpace.Page, bounds);
            bounds.Width--;
            bounds.Height--;
            ControlPaint.DrawBorder3D(graphics, bounds, style, BorderSide);
        }

        #endregion

    }

}
