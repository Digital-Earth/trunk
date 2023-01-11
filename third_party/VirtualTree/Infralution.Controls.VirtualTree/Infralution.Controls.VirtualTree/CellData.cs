#region File Header
//
//      FILE:   CellData.cs.
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
using System.ComponentModel;
using System.Drawing.Design;
using Infralution.Common;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines the visual properties of a single cell (defined by a <see cref="Row"/>/<see cref="Column"/>) 
    /// within a <see cref="VirtualTree"/>.
    /// </summary>
    /// <remarks>
    /// The visual properties for a single cell can be set by:
    /// <list type="bullet">
    /// <item>Setting the <see cref="CellBinding"/> properties in the <see cref="VirtualTree"/> designer</item>
    /// <item>Overriding the <see cref="VirtualTree.OnGetCellData"/> method</item>
    /// <item>Handling the <see cref="VirtualTree.GetCellData"/> event</item>
    /// <item>A combination of the above</item>
    /// </list>
    /// </remarks>
    /// <seealso cref="VirtualTree.OnGetCellData"/>
    /// <seealso cref="VirtualTree.GetCellData"/>
    /// <seealso cref="CellBinding"/>
    /// <seealso cref="RowBinding"/>
    public class CellData
    {
        #region Member Variables

        private object  _value = null;
        private string  _format = "{0}";
        private bool _isRichTextFormat = false;
        private string  _error = null;

        private bool _alwaysDisplayToolTip = false;
        private string  _toolTip = null;

        private Style _oddStyle;
        private Style _evenStyle;
        private Style _printOddStyle;
        private Style _printEvenStyle;        
        private CellEditor _editor;

        private bool _showText = true;
        private bool _showPreview = false;
        private Size  _previewSize = new Size(24, 20);
        private bool  _drawPreviewBorder = true;

        TypeConverter _typeConverter = null;
        UITypeEditor  _typeEditor = null;
        bool _useDefaultTypeConverter = true;
        bool _useDefaultTypeEditor = true;

        #endregion

        #region Public Interface

        /// <summary>
        /// Create a new cell data object for the given <see cref="Column"/>
        /// </summary>
        /// <param name="column">The column the cell data is for</param>
        public CellData(Column column)
        {
            _editor = column.CellEditor;
            _oddStyle = column.CellOddStyle;
            _evenStyle = column.CellEvenStyle;
            _printOddStyle = column.CellPrintOddStyle;
            _printEvenStyle = column.CellPrintEvenStyle;
        }

        /// <summary>
        /// Set/Get the value to be displayed in the cell 
        /// </summary>
        public object Value
        {
            get { return _value; }
            set { _value = value; }
        }

        /// <summary>
        /// Set/Get the format string to use to convert the value to a string 
        /// </summary>
        /// <remarks>
        /// Typically this includes a placeholder {0} for the <see cref="Value"/>.
        /// If this property is null then the <see cref="TypeConverter"/> is used to convert 
        /// the value to a string.   If <see cref="IsRichTextFormat"/> is set to true when this
        /// property is set then the string will be automatically parsed to escape brackets used
        /// in RTF
        /// </remarks>
        public string Format
        {
            get { return _format; }
            set 
            { 
                _format = StringUtilities.BlankToNull(value);
                if (IsRichTextFormat && _format != null)
                {
                    _format = _format.Replace(@"\{", "^[");
                    _format = _format.Replace(@"\}", "^]");
                    _format = _format.Replace("{", "{{");
                    _format = _format.Replace("}", "}}");
                    _format = _format.Replace("^[", "{");
                    _format = _format.Replace("^]", "}");
                }
            }
        }

        /// <summary>
        /// Is the string value in Rich Text Format (RTF) 
        /// </summary>
        /// <remarks>
        /// Determines whether the string value displayed in the cell will be treated as RTF
        /// </remarks>
        public bool IsRichTextFormat
        {
            get { return _isRichTextFormat; }
            set { _isRichTextFormat = value; }
        }

        /// <summary>
        /// Set/Get the error string associated with the cell. 
        /// </summary>
        /// <remarks>
        /// If set then an error icon is displayed in the cell and a tooltip with this
        /// string is displayed when the mouse is hovered over the cell.  The error text 
        /// can include a {0} placeholder for the <see cref="Value"/>.
        /// </remarks>
        /// <seealso cref="ToolTip"/>
        public string Error
        {
            get { return _error; }
            set { _error = StringUtilities.BlankToNull(value); }
        }

        /// <summary>
        /// Set/Get the ToolTip to display for this cell (if any)
        /// </summary>
        /// <remarks>
        /// <para>
        /// If an <see cref="Error"/> string is set then this will be displayed as 
        /// the tooltip in preference to this text.  If no tooltip is set then
        /// Virtual Tree will automatically display a tooltip containing the cell 
        /// text if it is truncated.
        /// </para>
        /// <para>
        /// The tooltip text can include a {0} placeholder for the <see cref="Value"/>.
        /// </para>
        /// </remarks>
        public string ToolTip
        {
            get { return _toolTip; }
            set { _toolTip = StringUtilities.BlankToNull(value); }
        }

        /// <summary>
        /// If set the tooltip is displayed regardless of whether the text fits in the cell
        /// </summary>
        /// <remarks>
        /// The default value is false - so that tooltips are only displayed when the displayed text
        /// does not fit in the cell.
        /// </remarks>
        public bool AlwaysDisplayToolTip
        {
            get { return _alwaysDisplayToolTip; }
            set { _alwaysDisplayToolTip = value; }
        }

        /// <summary>
        /// Set/Get the style to use for drawing cells in odd rows
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
        /// Set/Get the style to use for drawing cells in even rows
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
        /// Set/Get the style to use for printing cells in odd rows
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
        /// Set/Get the style to use for printing cells in even rows
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
        /// Set/Get cell editor to use to edit this cell
        /// </summary>
        public CellEditor Editor
        {
            get { return _editor; }
            set { _editor = value; }
        }

        /// <summary>
        /// Get/Set whether the cell should display a graphical preview of the value
        /// if value type supports this
        /// </summary>
        /// <remarks>
        /// Graphic previews can be displayed for value types which have a <see cref="UITypeEditor"/>
        /// that supports painting, associated with them using <see cref="EditorAttribute"/>.   The
        /// standard .NET types <see cref="Image"/>, <see cref="Icon"/>, <see cref="Bitmap"/>, 
        /// <see cref="Color"/> support graphical previewing.
        /// </remarks>
        public bool ShowPreview
        {
            get { return _showPreview; }
            set { _showPreview = value; }
        }

        /// <summary>
        /// Get/Set whether the cell should display a text representation of the value
        /// </summary>
        /// <remarks>
        /// Set this to false and <see cref="ShowPreview"/> to true to display a graphical preview only 
        /// </remarks>
        public bool ShowText
        {
            get { return _showText; }
            set { _showText = value; }
        }

        /// <summary>
        /// Get/Set the size of the graphical preview
        /// </summary>
        /// <seealso cref="ShowPreview"/>
        public Size PreviewSize
        {
            get { return _previewSize; }
            set { _previewSize = value; }
        }

        /// <summary>
        /// Get/Set whether a border should be drawn around the graphical preview
        /// </summary>
        /// <seealso cref="ShowPreview"/>
        public bool DrawPreviewBorder
        {
            get { return _drawPreviewBorder; }
            set { _drawPreviewBorder = value; }
        }

        /// <summary>
        /// Get/Set the type converter used to convert the Value to a string if the Format is null  
        /// </summary>
        /// <remarks>
        /// If this is not set then the default type converter for the Value is used.
        /// </remarks>
        public TypeConverter TypeConverter
        {
            get 
            {
                if (_useDefaultTypeConverter && _typeConverter == null && _value != null && !Convert.IsDBNull(_value))
                {
                    _typeConverter = TypeDescriptor.GetConverter(_value.GetType());
                }
                return _typeConverter; 
            }
            set 
            {
                _useDefaultTypeConverter = false;
                _typeConverter = value; 
            }
        }

        /// <summary>
        /// Should the default type converter for the <see cref="Value"/> be used
        /// </summary>
        public bool UseDefaultTypeConverter
        {
            get { return _useDefaultTypeConverter; }
            set { _useDefaultTypeConverter = value; }
        }

        /// <summary>
        /// Get/Set the Type Editor to used to painting and editing the value.
        /// </summary>
        /// <remarks>
        /// If this is not set then the default type editor for the Value is used
        /// </remarks>
        public UITypeEditor TypeEditor
        {
            get 
            {
                if (_useDefaultTypeEditor && _typeEditor == null)
                {
                    if (Value != null)
                    {
                        _typeEditor = (UITypeEditor)TypeDescriptor.GetEditor(Value.GetType(), typeof(UITypeEditor));
                    }
                    if (_typeEditor == null)
                    {
                        if (TypeConverter != null && TypeConverter.GetStandardValuesSupported())
                        {
                            _typeEditor = new StandardValueEditor(TypeConverter);
                        }
                    }
                }
                return _typeEditor; 
            }
            set 
            {
                _useDefaultTypeEditor = false;
                _typeEditor = value; 
            }
        }

        /// <summary>
        /// Should the default type editor for the <see cref="Value"/> be used
        /// </summary>
        public bool UseDefaultTypeEditor
        {
            get { return _useDefaultTypeEditor; }
            set { _useDefaultTypeEditor = value; }
        }

        /// <summary>
        /// Return the text to display for the value
        /// </summary>
        /// <remarks>
        /// Converts the <see cref="Value"/> to a string using the <see cref="Format"/>.  If 
        /// <see cref="Format"/> is null then the <see cref="TypeConverter"/> is used to
        /// convert the value to a string.
        /// </remarks>
        /// <returns>The text representation of <see cref="Value"/></returns>
        public virtual string GetText()
        {
            if (_format == null)
            {
                TypeConverter converter = TypeConverter;
                if (converter != null && converter.CanConvertTo(typeof(string))) 
                {
                    try
                    {
                        return converter.ConvertToString(_value);
                    }
                    catch
                    {
                    }
                }
                return (_value == null) ? null : _value.ToString();
            }
            return String.Format(_format, _value);
        }

        /// <summary>
        /// Return the tooltip text to display for the value
        /// </summary>
        /// <remarks>
        /// If the <see cref="ToolTip"/> is null returns <see cref="GetText"/> 
        /// otherwise returns the <see cref="ToolTip"/> formatted with the 
        /// <see cref="Value"/>
        /// </remarks>
        /// <returns>The tooltip text to display</returns>
        public virtual string GetToolTipText()
        {
            string result = null;
            if (ToolTip == null)
                result = GetText();
            else
                result = String.Format(ToolTip, Value);
            return result;
        }

        /// <summary>
        /// Return the error text to display for the value
        /// </summary>
        /// <remarks>
        /// Returns the <see cref="Error"/> formatted with the <see cref="Value"/>
        /// </remarks>
        /// <returns>The error text to display</returns>
        public virtual string GetErrorText()
        {
            if (Error == null) return null;
            return String.Format(Error, Value);        
        }

        #endregion

    }

}
