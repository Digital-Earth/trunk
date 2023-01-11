#region File Header
//
//      FILE:   StyleDelta.cs.
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
using System.Reflection;
namespace Infralution.Controls
{

    /// <summary>
    /// Encapsulates the differences between a <see cref="Style"/> and its parent style.
    /// </summary>
    /// <remarks>
    /// Typically the end user should not need to use StyleDeltas directly.  They are used
    /// by control authors to build a hierarchy of related styles.
    /// </remarks>
    /// <seealso cref="Style"/>
    public class StyleDelta: INotifyPropertyChanged
    {
        #region Member Variables
 
        private Color _foreColor = Color.Empty;
        private Color _backColor = Color.Empty;
        private Color _gradientColor = Color.Empty;
        private LinearGradientMode _gradientMode = LinearGradientMode.Vertical;
        private bool _defaultGradientMode = true;
        private Color _borderColor = Color.Empty;
        private int   _borderWidth = -1;  
        private Border3DStyle _borderStyle = Border3DStyle.Flat;
        private bool _defaultBorderStyle = true;
        private Border3DSide _borderSide = Border3DSide.All;
        private bool _defaultBorderSide = true;
        private int _borderRadius = -1;
        private Font _font = null;
        private StringAlignment _horzAlignment = StringAlignment.Center;
        private bool _defaultHorzAlignment = true;
        private StringAlignment _vertAlignment = StringAlignment.Center;
        private bool _defaultVertAlignment = true;
        private RightToLeft _rightToLeft = RightToLeft.Inherit;
        private bool _wordWrap = false;
        private bool _defaultWordWrap = true;
        private StringTrimming _stringTrimming = StringTrimming.EllipsisCharacter;
        private bool _defaultStringTrimming = true;
        private byte _alphaBlend = 255;
        private bool _defaultAlphaBlend = true;
        private bool _compatibleTextRendering = _compatibleTextRenderingDefault;
        private bool _defaultCompatibleTextRendering = true;

        private static bool _compatibleTextRenderingDefault = true;

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
        /// Create an empty style delta 
        /// </summary>
        public StyleDelta()
        {
        }

        /// <summary>
        /// The foreground color to use when drawing the item
        /// </summary>
        public Color ForeColor
        {
            get { return _foreColor; }
            set
            {
                _foreColor = value;
                OnPropertyChanged("ForeColor");
            }
        }

        /// <summary>
        /// Reset the property to the default (inherited) value
        /// </summary>
        public void ResetForeColor()
        {
            ForeColor = Color.Empty;
        }
        
        /// <summary>
        /// Should the default (inherited) property value be used
        /// </summary>
        public bool UseDefaultForeColor
        {
            get { return _foreColor == Color.Empty; }
        }
 
        /// <summary>
        /// The background color to use when drawing the item
        /// </summary>
        public Color BackColor
        {
            get { return _backColor; }
            set
            {
                _backColor = value;
                OnPropertyChanged("BackColor");
            }
        }

        /// <summary>
        /// Reset the property to the default (inherited) value
        /// </summary>
        public void ResetBackColor()
        {
            BackColor = Color.Empty;
        }
        
        /// <summary>
        /// Should the default (inherited) property value be used
        /// </summary>
        public bool UseDefaultBackColor
        {
            get { return _backColor == Color.Empty; }
        }

        /// <summary>
        /// The second background color to use when drawing a gradient background
        /// </summary>
        public Color GradientColor
        {
            get { return _gradientColor; }
            set
            {
                _gradientColor = value;
                OnPropertyChanged("GradientColor");
            }
        }

        /// <summary>
        /// Reset the property to the default (inherited) value
        /// </summary>
        public void ResetGradientColor()
        {
            GradientColor = Color.Empty;
        }
        
        /// <summary>
        /// Should the default (inherited) property value be used
        /// </summary>
        public bool UseDefaultGradientColor
        {
            get { return _gradientColor == Color.Empty; }
        }

        /// <summary>
        /// The gradient mode to use when drawing a gradient background  
        /// </summary>
        public LinearGradientMode GradientMode
        {
            get { return _gradientMode; }
            set
            {
                _gradientMode = value;
                _defaultGradientMode = false;
                OnPropertyChanged("GradientMode");
            }
        }

        /// <summary>
        /// Reset the property to the default (inherited) value
        /// </summary>
        public void ResetGradientMode()
        {
            _defaultGradientMode = true;
            OnPropertyChanged("GradientMode");
        }
        
        /// <summary>
        /// Should the default (inherited) property value be used
        /// </summary>
        public bool UseDefaultGradientMode
        {
            get { return _defaultGradientMode; }
        }

        /// <summary>
        /// The alpha blend to use when drawing the background (0 = transparent, 255 = opaque)  
        /// </summary>
        public byte AlphaBlend
        {
            get { return _alphaBlend; }
            set
            {
                _alphaBlend = value;
                _defaultAlphaBlend = false;
                OnPropertyChanged("AlphaBlend");
            }
        }

        /// <summary>
        /// Reset the property to the default (inherited) value
        /// </summary>
        public void ResetAlphaBlend()
        {
            _defaultAlphaBlend = true;
            OnPropertyChanged("AlphaBlend");
        }
        
        /// <summary>
        /// Should the default (inherited) property value be used
        /// </summary>
        public bool UseDefaultAlphaBlend
        {
            get { return _defaultAlphaBlend; }
        }

        /// <summary>
        /// The border color to use when drawing the item
        /// </summary>
        public Color BorderColor
        {
            get { return  _borderColor; }
            set
            {
                _borderColor = value;
                OnPropertyChanged("BorderColor");
            }
        }

        /// <summary>
        /// Reset the property to the default (inherited) value
        /// </summary>
        public void ResetBorderColor()
        {
            BorderColor = Color.Empty;
        }
        
        /// <summary>
        /// Should the default (inherited) property value be used
        /// </summary>
        public bool UseDefaultBorderColor
        {
            get { return _borderColor == Color.Empty; }
        }

        /// <summary>
        /// The width of the border to use when drawing the item
        /// </summary>
        public int BorderWidth
        {
            get {  return (_borderWidth < 0) ? 0 : _borderWidth; }
            set
            {
                _borderWidth = value;
                OnPropertyChanged("BorderWidth");
            }
        }

        /// <summary>
        /// Reset the property to the default (inherited) value
        /// </summary>
        public void ResetBorderWidth()
        {
            BorderWidth = -1;
        }
        
        /// <summary>
        /// Should the default (inherited) property value be used
        /// </summary>
        public bool UseDefaultBorderWidth
        {
            get { return _borderWidth < 0; }
        }


        /// <summary>
        /// The border style to use
        /// </summary>
        public Border3DStyle BorderStyle
        {
            get { return _borderStyle; }
            set
            {
                _borderStyle = value;
                _defaultBorderStyle = false;
                if (value != Border3DStyle.Flat && value != Border3DStyle.Adjust)
                {
                    BorderWidth = 2;
                }
                OnPropertyChanged("BorderStyle");
            }
        }

        /// <summary>
        /// Reset the property to the default (inherited) value
        /// </summary>
        public void ResetBorderStyle()
        {
            _defaultBorderStyle = true;
            OnPropertyChanged("BorderStyle");
        }
        
        /// <summary>
        /// Should the default (inherited) property value be used
        /// </summary>
        public bool UseDefaultBorderStyle
        {
            get { return _defaultBorderStyle; }
        }

        /// <summary>
        /// The sides to draw borders on
        /// </summary>
        public Border3DSide BorderSide
        {
            get { return _borderSide; }
            set
            {
                _borderSide = value;
                _defaultBorderSide = false;
                OnPropertyChanged("BorderSide");
            }
        }

        /// <summary>
        /// Reset the property to the default (inherited) value
        /// </summary>
        public void ResetBorderSide()
        {
            _defaultBorderSide = true;
            OnPropertyChanged("BorderSide");
        }

        /// <summary>
        /// Should the default (inherited) property value be used
        /// </summary>
        public bool UseDefaultBorderSide
        {
            get { return _defaultBorderSide; }
        }

        /// <summary>
        /// The radius to use for rounding the border and background rectangles.
        /// A value of zero indicates normal rectangular borders.
        /// </summary>
        public int BorderRadius
        {
            get { return (_borderRadius < 0) ? 0 : _borderRadius; }
            set
            {
                _borderRadius = value;
                OnPropertyChanged("BorderRadius");
            }
        }

        /// <summary>
        /// Reset the property to the default (inherited) value
        /// </summary>
        public void ResetBorderRadius()
        {
            BorderRadius = -1;
        }

        /// <summary>
        /// Should the default (inherited) property value be used
        /// </summary>
        public bool UseDefaultBorderRadius
        {
            get { return _borderRadius < 0; }
        }

        /// <summary>
        /// The font to use when drawing the item
        /// </summary>
        public Font Font
        {
            get { return _font; }
            set
            {
                _font = value;
                OnPropertyChanged("Font");
            }
        }

        /// <summary>
        /// Reset the property to the default (inherited) value
        /// </summary>
        public void ResetFont()
        {
            Font = null;
        }
        
        /// <summary>
        /// Should the default (inherited) property value be used
        /// </summary>
        public bool UseDefaultFont
        {
            get { return _font == null; }
        }

        /// <summary>
        /// The horizontal alignment to use when drawing text
        /// </summary>
        public StringAlignment HorzAlignment
        {
            get {  return _horzAlignment; }
            set
            {
                _horzAlignment = value;
                _defaultHorzAlignment = false;
                OnPropertyChanged("HorzAlignment");
            }
        }

        /// <summary>
        /// Reset the property to the default (inherited) value
        /// </summary>
        public void ResetHorzAlignment()
        {
            _defaultHorzAlignment = true;
            OnPropertyChanged("HorzAlignment");
        }
        
        /// <summary>
        /// Should the default (inherited) property value be used
        /// </summary>
        public bool UseDefaultHorzAlignment
        {
            get { return _defaultHorzAlignment; }
        }

        /// <summary>
        /// The vertical alignment to use when drawing text
        /// </summary>
        public StringAlignment VertAlignment
        {
            get { return _vertAlignment; }
            set
            {
                _vertAlignment = value;
                _defaultVertAlignment = false;
                OnPropertyChanged("VertAlignment");
            }
        }

        /// <summary>
        /// Reset the property to the default (inherited) value
        /// </summary>
        public void ResetVertAlignment()
        {
            _defaultVertAlignment = true;
            OnPropertyChanged("VertAlignment");
        }
        
        /// <summary>
        /// Should the default (inherited) property value be used
        /// </summary>
        public bool UseDefaultVertAlignment
        {
            get { return _defaultVertAlignment; }
        }

        /// <summary>
        /// Is the layout of this element right to left
        /// </summary>
        public RightToLeft RightToLeft
        {
            get {  return _rightToLeft; }
            set
            {
                _rightToLeft = value;
                OnPropertyChanged("RightToLeft");
            }
        }

        /// <summary>
        /// Reset the property to the default (inherited) value
        /// </summary>
        public void ResetRightToLeft()
        {
            RightToLeft = RightToLeft.Inherit;
        }
    
        /// <summary>
        /// Should the default (inherited) property value be used
        /// </summary>
        public bool UseDefaultRightToLeft
        {
            get { return _rightToLeft == RightToLeft.Inherit; }
        }

        /// <summary>
        /// Should text be word wrapped inside the display rectangle 
        /// </summary>
        public bool WordWrap
        {
            get { return _wordWrap; }
            set
            {
                _wordWrap = value;
                _defaultWordWrap = false;
                OnPropertyChanged("WordWrap");
            }
        }

        /// <summary>
        /// Reset the property to the default (inherited) value
        /// </summary>
        public void ResetWordWrap()
        {
            _defaultWordWrap = true;
            OnPropertyChanged("WordWrap");
        }
        
        /// <summary>
        /// Should the default (inherited) property value be used
        /// </summary>
        public bool UseDefaultWordWrap
        {
            get { return _defaultWordWrap; }
        }

        /// <summary>
        /// How should strings be trimmed if they don't fit the display area 
        /// </summary>
        public StringTrimming StringTrimming
        {
            get { return _stringTrimming; }
            set
            {
                _stringTrimming = value;
                _defaultStringTrimming = false;
                OnPropertyChanged("StringTrimming");
            }
        }

        /// <summary>
        /// Reset the property to the default (inherited) value
        /// </summary>
        public void ResetStringTrimming()
        {
            _defaultStringTrimming = true;
            OnPropertyChanged("StringTrimming");
        }

        /// <summary>
        /// Should the default (inherited) property value be used
        /// </summary>
        public bool UseDefaultStringTrimming
        {
            get { return _defaultStringTrimming; }
        }

        /// <summary>
        /// Should GDI+ be used for rendering text
        /// </summary>
        public bool CompatibleTextRendering
        {
            get { return _compatibleTextRendering; }
            set
            {
                _compatibleTextRendering = value;
                _defaultCompatibleTextRendering = false;
                OnPropertyChanged("CompatibleTextRendering");
            }
        }

        /// <summary>
        /// Reset the property to the default (inherited) value
        /// </summary>
        public void ResetCompatibleTextRendering()
        {
            _defaultCompatibleTextRendering = true;
            OnPropertyChanged("CompatibleTextRendering");
        }

        /// <summary>
        /// Should the default (inherited) property value be used
        /// </summary>
        public bool UseDefaultCompatibleTextRendering
        {
            get { return _defaultCompatibleTextRendering; }
        }

        /// <summary>
        /// Returns true if the delta is empty (ie all values are defaults)
        /// </summary>
        /// <returns></returns>
        public bool IsEmpty()
        {
            return this.UseDefaultBackColor && this.UseDefaultForeColor &&
                   this.UseDefaultGradientColor && this.UseDefaultGradientMode &&
                   this.UseDefaultFont && this.UseDefaultBorderColor &&
                   this.UseDefaultBorderWidth && this.UseDefaultBorderStyle &&
                   this.UseDefaultHorzAlignment && this.UseDefaultVertAlignment &&
                   this.UseDefaultRightToLeft && this.UseDefaultWordWrap && 
                   this.UseDefaultStringTrimming && this.UseDefaultAlphaBlend && 
                   this.UseDefaultBorderSide && this.UseDefaultBorderRadius && this.UseDefaultCompatibleTextRendering;
        }

        /// <summary>
        /// Reset the default values for all properties
        /// </summary>
        public void Reset()
        {
            this.ResetBackColor();
            this.ResetGradientColor();
            this.ResetGradientMode();
            this.ResetAlphaBlend();
            this.ResetForeColor();
            this.ResetBorderColor();
            this.ResetBorderStyle();
            this.ResetFont();
            this.ResetBorderWidth();
            this.ResetHorzAlignment();
            this.ResetVertAlignment();
            this.ResetRightToLeft();
            this.ResetWordWrap();
            this.ResetStringTrimming();
            this.ResetBorderSide();
            this.ResetBorderRadius();
            this.ResetCompatibleTextRendering();
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Set the default value for the <see cref="CompatibleTextRendering"/>
        /// property of new styles
        /// </summary>
        internal static bool CompatibleTextRenderingDefault
        {
            get { return _compatibleTextRenderingDefault; }
            set
            {
                _compatibleTextRenderingDefault = value;
            }
        }

        /// <summary>
        /// Notify clients of a change to the style delta
        /// </summary>
        protected virtual void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

         #endregion

    }

}
