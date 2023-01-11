#region File Header
//
//      FILE:   RowData.cs.
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
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines the visual properties and relationships for a <see cref="Row"/> within a <see cref="VirtualTree"/>.
    /// </summary>
    /// <remarks>
    /// The visual properties and relationships for a <see cref="Row"/> can be set by:
    /// <list type="bullet">
    /// <item>Setting the <see cref="RowBinding"/> properties in the <see cref="VirtualTree"/> designer</item>
    /// <item>Overriding the <see cref="VirtualTree.OnGetRowData"/> method</item>
    /// <item>Handling the <see cref="VirtualTree.GetRowData"/> event</item>
    /// <item>A combination of the above</item>
    /// </list>
    /// </remarks>
    /// <seealso cref="VirtualTree.OnGetRowData"/>
    /// <seealso cref="VirtualTree.GetRowData"/>
    /// <seealso cref="RowBinding"/>
    public class RowData 
    {
        #region Member Variables

        private string _error;
        private Icon  _icon = null;
        private Icon  _expandedIcon = null;
        private int   _iconSize = 16;
        private Style _oddStyle;
        private Style _evenStyle;
        private Style _printOddStyle;
        private Style _printEvenStyle;
        private int   _height;  
        private bool  _autoFitHeight = false; 
        private bool  _resizable = true;
        private bool  _showPrefixColumn = true;

        #endregion

        #region Public Interface

        /// <summary>
        /// Create a new row data object for the given tree
        /// </summary>
        /// <param name="tree">The tree used to intialize the styles</param>
        public RowData(VirtualTree tree)
        {
            _height = tree.RowHeight;
            _oddStyle = tree.RowOddStyle;
            _evenStyle = tree.RowEvenStyle;
            _printOddStyle = tree.RowPrintOddStyle;
            _printEvenStyle = tree.RowPrintEvenStyle;
        }

        /// <summary>
        /// Set/Get the error string associated with the row. 
        /// </summary>
        /// <remarks>
        /// If set then an error icon is displayed in the row header (if displayed) 
        /// and a tooltip with this string is displayed when the mouse is hovered over the 
        /// row header. 
        /// </remarks>
        public string Error
        {
            get { return _error; }
            set { _error = value; }
        }

        /// <summary>
        /// Set/get the icon to display for the <see cref="Row"/>.  
        /// </summary>
        public Icon Icon
        {
            get { return _icon; }
            set { _icon = value; }
        }

        /// <summary>
        /// Set/Get the icon to display for the <see cref="Row"/> when expanded (if different to <see cref="Icon"/>).
        /// </summary>
        /// <remarks>If the expanded icon is not set then the normal icon is used</remarks>
         public Icon ExpandedIcon
        {
            get { return _expandedIcon; }
            set { _expandedIcon = value; }
        }

        /// <summary>
        /// Set/Get the size of the icon to display.
        /// </summary>
        /// <remarks>
        /// This lets you select the size of the icon to display where an icon contains
        /// multiple image sizes.
        /// </remarks>
        public virtual int IconSize
        {
            get { return _iconSize; }
            set 
            {
                if (_iconSize <= 0) 
                    throw new ArgumentOutOfRangeException("IconSize", "Value must be non-zero");
                _iconSize = value; 
            }
        }

        /// <summary>
        /// Set/Get the height in pixels for this row. 
        /// </summary>
        /// <remarks>
        /// If <see cref="AutoFitHeight"/> is true then this is calculated automatically
        /// from the contents of the row.
        /// </remarks>
        public int Height
        {
            get { return _height; }
            set { _height = value; }
        }

        /// <summary>
        /// Should the height for this row be calculated automatically from the contents
        /// of the row
        /// </summary>
        public bool AutoFitHeight
        {
            get { return _autoFitHeight; }
            set { _autoFitHeight = value; }
        }

        /// <summary>
        /// Can the user change the height of this row by dragging the RowHeader dividers
        /// </summary>
        /// <remarks>
        /// <see cref="VirtualTree.AllowRowResize"/> must be set to true to allow any resizing of rows
        /// </remarks>
        /// <seealso cref="VirtualTree.AllowRowResize"/> 
        public virtual bool Resizable
        {
            get { return _resizable; }
            set { _resizable = value; }
        }

        /// <summary>
        /// Should the <see cref="VirtualTree.PrefixColumn"/> (if defined) be shown for this row
        /// </summary>
        public bool ShowPrefixColumn
        {
            get { return _showPrefixColumn; }
            set { _showPrefixColumn = value; }
        }

        /// <summary>
        /// Set/Get the style to use for drawing odd <see cref="Row">Rows</see>
        /// </summary>
        public Style OddStyle
        {
            get { return _oddStyle; }
            set 
            {
                if (value == null) throw new ArgumentNullException("value");
                _oddStyle = value; 
            }
        }

        /// <summary>
        /// Set/Get the style to use for drawing even <see cref="Row">Rows</see>
        /// </summary>
        public Style EvenStyle
        {
            get { return _evenStyle; }
            set 
            {
                if (value == null) throw new ArgumentNullException("value");
                _evenStyle = value; 
            }
        }

        /// <summary>
        /// Set/Get the style to use for printing odd <see cref="Row">Rows</see>
        /// </summary>
        public Style PrintOddStyle
        {
            get { return _printOddStyle; }
            set 
            {
                if (value == null) throw new ArgumentNullException("value");
                _printOddStyle = value; 
            }
        }

        /// <summary>
        /// Set/Get the style to use for printing even <see cref="Row">Rows</see>
        /// </summary>
        public Style PrintEvenStyle
        {
            get { return _printEvenStyle; }
            set 
            {
                if (value == null) throw new ArgumentNullException("value");
                _printEvenStyle = value; 
            }
        }

        /// <summary>
        /// Return the icon to use converting its size if necessary
        /// </summary>
        /// <returns>The icon to use</returns>
        public Icon GetIcon()
        {
            if (_icon != null)
            {
                if (_icon.Width != _iconSize)
                {
                    int height = (int) (_iconSize * (float)_icon.Height / (float)_icon.Width);
                    _icon = new Icon(_icon, _iconSize, height);
                }
            }
            return _icon;
        }

        /// <summary>
        /// Return the icon to use for expanded rows converting its size if necessary
        /// </summary>
        /// <returns>The icon to use</returns>
        public Icon GetExpandedIcon()
        {
            if (_expandedIcon == null) return GetIcon();

            if (_expandedIcon.Width != _iconSize)
            {
                int height = (int) (_iconSize * (float)_expandedIcon.Height / (float)_expandedIcon.Width);
                _expandedIcon = new Icon(_expandedIcon, _iconSize, height);
            }
            return _expandedIcon;
        }

        #endregion

    }

}
