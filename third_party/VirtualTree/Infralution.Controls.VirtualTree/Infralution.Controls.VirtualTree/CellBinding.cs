#region File Header
//
//      FILE:   CellBinding.cs.
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
using System.Collections;
using System.Drawing;
using System.ComponentModel;
using System.Windows.Forms.Design;
using System.Drawing.Design;
using System.Windows.Forms;
using NS = Infralution.Controls.VirtualTree;
using Infralution.Common;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a base class for binding information displayed in a cell of a <see cref="VirtualTree"/> 
    /// (defined by a <see cref="Row"/> and <see cref="NS.Column"/>) to the <see cref="VirtualTree.DataSource"/>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// CellBindings belong to a <see cref="NS.RowBinding"/> and are not directly created by the user.   
    /// CellBindings are usually created using the <see cref="VirtualTree"/> visual designer. 
    /// </para>
    /// <para>
    /// Each <see cref="NS.RowBinding"/> contains a <see cref="CellBindingList"/> which maps the supported 
    /// <see cref="NS.Column">Columns</see> to CellBindings.  
    /// The <see cref="NS.RowBinding"/> calls the <see cref="GetCellData"/> method to get the 
    /// <see cref="CellData"/> (which defines the visual appearance of a cell) for a given 
    /// <see cref="NS.Column"/>.
    /// </para>
    /// <para>
    /// Derived classes are typically defined which map <see cref="CellData"/> properties to attributes of the
    /// <see cref="VirtualTree.DataSource"/> for a particular type of DataSource.
    /// This base class provides a mechanism to allow the user to set the <see cref="CellData"/> properties 
    /// that are not typically data driven.  For instance the <see cref="Format"/> and <see cref="Style"/> of 
    /// the cells for a given <see cref="NS.RowBinding"/> can be set using the <see cref="VirtualTree"/> designer.
    /// </para>
    /// </remarks>
    /// <seealso href="XtraDatasetBinding.html">Data Binding to ADO.NET Datasets and DataViews</seealso>
    /// <seealso href="XtraObjectBinding.html">Data Binding to Object Properties</seealso>
    /// <seealso cref="VirtualTree"/>
    /// <seealso cref="NS.RowBinding"/>
    /// <seealso cref="NS.Column"/>
    /// <seealso cref="Row"/>
    /// <seealso cref="CellData"/>
    [DesignTimeVisible(false)]
    public abstract class CellBinding 
    {
        #region Member Variables

        /// <summary>
        /// The row binding this cell binding belongs to
        /// </summary>
        private RowBinding _rowBinding;

        /// <summary>
        /// The column this cell is associated with
        /// </summary>
        private Column _column;
        
        /// <summary>
        /// The string used to format the data value for a cell
        /// </summary>
        private string _format = "{0}";

        /// <summary>
        /// Should the string be displayed as RTF
        /// </summary>
        private bool _isRichTextFormat = false;

        /// <summary>
        /// The tooltip to display for this cell
        /// </summary>
        private string _toolTip;

        /// <summary>
        /// Should tooltips always be displayed
        /// </summary>
        private bool _alwaysDisplayToolTip = false;

        /// <summary>
        /// The style for this row
        /// </summary>
        private Style _style = new Style();

        /// <summary>
        /// The style for odd rows
        /// </summary>
        private Style _oddStyle = new Style();

        /// <summary>
        /// The style for even rows
        /// </summary>
        private Style _evenStyle = new Style();

        /// <summary>
        /// The style to use when printing this type of row
        /// </summary>
        Style _printStyle = new Style();

        /// <summary>
        /// The style to use when printing even rows 
        /// </summary>
        Style _printEvenStyle = new Style();

        /// <summary>
        /// The style to use when printing odd rows 
        /// </summary>
        Style _printOddStyle = new Style();

        /// <summary>
        /// The Editor to use for this cell (if _useDefaultEditor is false)
        /// </summary>
        private CellEditor _editor = null;

        /// <summary>
        /// Should the default column editor be used
        /// </summary>
        private bool _useDefaultEditor = true;

        /// <summary>
        /// Should a graphic preview be displayed if the value type supports it
        /// </summary>
        private bool _showPreview = false;

        /// <summary>
        /// Should the text representation of the value be displayed 
        /// </summary>
        private bool _showText = true;

        /// <summary>
        /// The width of the graphic preview
        /// </summary>
        private Size  _previewSize = DefaultPreviewSize;
        private static Size DefaultPreviewSize = new Size(22, 14);

        /// <summary>
        /// Should a border be drawn around the graphical preview
        /// </summary>
        private bool _drawPreviewBorder = true;

        #endregion

        #region Public Interface
 
        /// <summary>
        /// Default constructor
        /// </summary>
        public CellBinding()
        {
        }

        /// <summary>
        /// Get the <see cref="NS.RowBinding"/> this cell binding belongs to
        /// </summary>
        [Browsable(false)]
        public RowBinding RowBinding
        {
            get { return _rowBinding; }
        }

        /// <summary>
        /// Return the <see cref="VirtualTree"/> this binding belongs to.
        /// </summary>
        [Browsable(false)]
        public VirtualTree Tree
        {
            get { return (RowBinding == null) ? null : RowBinding.Tree; }
        }

        /// <summary>
        /// Return the string to be displayed in the designer for this binding
        /// </summary>
        [Browsable(false)]
        public virtual string DisplayName
        {
            get 
            { 
                if (Column == null)
                    return "(none)";
                else
                    return Column.ToString();
            }
        }

        /// <summary>
        /// Set/Get the column this cell binding applies to
        /// </summary>
        [Category("Data")]
        [DefaultValue(null)]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Description("The column that this binding applies to")]
        public Column Column
        {
            get { return _column; }
            set 
            { 
                _column = value; 
                InitializeStyles();
            }
        }

        /// <summary>
        /// Set/Get the format string to use to display the cell data value 
        /// </summary>
        /// <remarks>
        /// This is a standard .NET <see cref="String.Format(String, object)"/> string which is used to format the
        /// data value.  This gives a great deal of flexibility in how the data is displayed
        /// in the cell.    If the Format is set to null/nothing then the <see cref="TypeConverter"/>
        /// associated with the value type is used to convert the data value to a string.  
        /// This can be useful where the format strings and type converters for an object type
        /// expose different functionality. 
        /// </remarks>
        /// <example>
        /// Some examples of format strings are given below:
        /// <para>"{0:dd MMM yyyy}" - display a date value</para>
        /// <para>"${0:C}" - display monetary values</para>
        /// </example>
        [Category("Appearance")]
        [DefaultValue("{0}")]
        [Description("The format string used to display the cell value")]
        [Localizable(true)]           
        public virtual string Format
        {
            get { return _format; }
            set { _format = StringUtilities.BlankToNull(value); }
        }

        /// <summary>
        /// Is the stringto be displayed in Rich Text Format (RTF) 
        /// </summary>
        /// <remarks>
        /// Determines whether the string value displayed in the cell will be treated as RTF
        /// </remarks>
        [Category("Appearance")]
        [DefaultValue(false)]
        [Description("Is the string to be displayed in Rich Text Format (RTF) ")]
        [Localizable(true)]
        public bool IsRichTextFormat
        {
            get { return _isRichTextFormat; }
            set { _isRichTextFormat = value; }
        }

        /// <summary>
        /// Set/Get the ToolTip to display for this cell. 
        /// </summary>
        /// <remarks>
        /// <para>
        /// The tooltip text can include a {0} placeholder for the cell data value.
        /// </para>
        /// <para>
        /// If an error is set for a cell then this will be displayed as 
        /// the tooltip in preference to this text.  If no tooltip is set then
        /// Virtual Tree will automatically display a tooltip containing the cell 
        /// text if it is truncated.
        /// </para>
        /// </remarks>
        /// <seealso cref="AlwaysDisplayToolTip"/>
        [Category("Appearance")]
        [DefaultValue(null)]
        [Description("The tooltip to display for this cell")]           
        [Localizable(true)]
        public virtual string ToolTip
        {
            get { return _toolTip; }
            set { _toolTip = StringUtilities.BlankToNull(value); }
        }

        /// <summary>
        /// Should the tooltip for this cell always be displayed
        /// </summary>
        /// <remarks>
        /// If false (the default) then tooltip is only displayed when the text does not fit in the
        /// available space.
        /// </remarks>
        [Category("Behavior")]
        [DefaultValue(false)]
        [Description("Should the tooltip always be displayed")]
        [Localizable(true)]
        public virtual bool AlwaysDisplayToolTip
        {
            get { return _alwaysDisplayToolTip; }
            set { _alwaysDisplayToolTip = value; }
        }

        #region Styles

        /// <summary>
        /// The style to draw cells with
        /// </summary>
        /// <remarks>
        /// Use this to set style properties that should be applied to cells in both odd and
        /// even rows.   This style derives from <see cref="NS.Column.CellStyle">Column.CellStyle</see>.  
        /// Use <see cref="NS.Column.CellStyle">Column.CellStyle</see> to set properties which
        /// apply to all cells in a column. 
        /// </remarks>
        /// <seealso cref="OddStyle"/>
        /// <seealso cref="EvenStyle"/>
        [Category("Appearance")]
        [Description("The style to draw cells with")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public virtual Style Style
        {
            get { return _style; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeStyle()
        {
            return _style.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        private void ResetStyle()
        {
            _style.Reset();
        }

        /// <summary>
        /// The style to draw cells in odd rows with
        /// </summary>
        /// <remarks>
        /// Use this to override style properties that apply only to cells in odd rows.   
        /// Derives from <see cref="Style"/> and <see cref="NS.Column.CellOddStyle">Column.CellOddStyle</see>.   
        /// Use <see cref="NS.Column.CellOddStyle">Column.CellOddStyle</see> to set properties which apply to all
        /// cells in a column.   Use <see cref="Style"/> to set properties which apply 
        /// to both odd and even rows of this cell type. 
        /// </remarks>
        /// <seealso cref="Style"/>
        /// <seealso cref="EvenStyle"/>
        /// <seealso cref="NS.RowBinding.OddStyle">RowBinding.OddStyle</seealso>
        [Category("Appearance"),
        Description("The style to draw cells in odd rows with"),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public virtual Style OddStyle
        {
            get { return _oddStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeOddStyle()
        {
            return _oddStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        private void ResetOddStyle()
        {
            _oddStyle.Reset();
        }

        /// <summary>
        /// The style to draw cells in even rows with
        /// </summary>
        /// <remarks>
        /// Use this to override style properties that apply only to cells in even rows.   
        /// Derives from <see cref="Style"/> and <see cref="NS.Column.CellEvenStyle">Column.CellEvenStyle</see>.   
        /// Use <see cref="NS.Column.CellEvenStyle">Column.CellEvenStyle</see> to set properties which apply to all
        /// cells in this column.   Use <see cref="Style"/> to set properties which apply 
        /// to both odd and even rows of this cell type. 
        /// </remarks>
        /// <seealso cref="Style"/>
        /// <seealso cref="OddStyle"/>
        /// <seealso cref="NS.RowBinding.EvenStyle">RowBinding.EvenStyle</seealso>
        [Category("Appearance"),
        Description("The style to draw cells in even rows with"),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public virtual Style EvenStyle
        {
            get { return _evenStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeEvenStyle()
        {
            return _evenStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        private void ResetEvenStyle()
        {
            _evenStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use when printing 
        /// </summary>
        [Category("Appearance"),
        Description("The drawing attributes to use when printing"),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style PrintStyle
        {
            get { return _printStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializePrintStyle()
        {
            return _printStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetPrintStyle()
        {
            _printStyle.Reset();
        }

 
        /// <summary>
        /// The drawing attributes to use when printing for odd rows
        /// </summary>
        [Category("Appearance"),
        Description("The drawing attributes to use when printing odd rows"),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style PrintOddStyle
        {
            get { return _printOddStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializePrintOddStyle()
        {
            return _printOddStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetPrintOddStyle()
        {
            _printOddStyle.Reset();
        }

        /// <summary>
        /// The drawing attributes to use when printing even rows
        /// </summary>
        [Category("Appearance"),
        Description("The drawing attributes to use when printing even rows"),
        DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public Style PrintEvenStyle
        {
            get { return _printEvenStyle; }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializePrintEvenStyle()
        {
            return _printEvenStyle.ShouldSerialize();;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetPrintEvenStyle()
        {
            _printEvenStyle.Reset();
        }

        #endregion

        /// <summary>
        /// The editor to use to allow the user to edit the cells value
        /// </summary>
        [Category("Behavior"),
        Description("The editor to use to edit the cells value")]
        public virtual CellEditor Editor
        {
            get 
            { 
                if (_useDefaultEditor && _column != null)
                {
                    return _column.CellEditor;
                }
                return _editor; 
            }
            set 
            { 
                _useDefaultEditor = false;
                _editor = value; 
            }
        }

        /// <summary>
        /// Called by framework to determine whether the style should be serialised
        /// </summary>
        /// <returns>True if the style should be serialized</returns>
        private bool ShouldSerializeEditor()
        {
            return !_useDefaultEditor;
        }

        /// <summary>
        /// Called by framework to reset the style defaults
        /// </summary>
        public virtual void ResetEditor()
        {
            _useDefaultEditor = true;
        }


        /// <summary>
        /// Should a graphic preview of the value be displayed (if the value type supports this)
        /// </summary>
        /// <remarks>
        /// Graphic previews can be displayed for value types which have a <see cref="UITypeEditor"/>
        /// that supports painting, associated with them using <see cref="EditorAttribute"/>.   The
        /// standard .NET types <see cref="Image"/>, <see cref="Icon"/>, <see cref="Bitmap"/>, 
        /// <see cref="Color"/> support graphical previewing.
        /// </remarks>
        [Category("Appearance"),
        Description("Should a graphic preview of the value be displayed (if the value type supports this)"),
        DefaultValue(false)]
        [Localizable(true)]
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
        [Category("Appearance"),
        Description("Should the text representation of the value be displayed"),
        DefaultValue(true)]
        [Localizable(true)]
        public bool ShowText
        {
            get { return _showText; }
            set { _showText = value; }
        }

        /// <summary>
        /// Get/Set the size of the graphical preview
        /// </summary>
        /// <seealso cref="ShowPreview"/>
        [Category("Appearance")]
        [RefreshProperties(RefreshProperties.Repaint)]
        [Description("The size of the graphic preview")]
        [Localizable(true)]
        public Size PreviewSize
        {
            get { return _previewSize; }
            set { _previewSize = value; }
        }

        /// <summary>
        /// Called by framework to determine whether the preview size should be serialised
        /// </summary>
        /// <returns>True if the property should be serialized</returns>
        private bool ShouldSerializePreviewSize()
        {
            return (_previewSize != DefaultPreviewSize);
        }

        /// <summary>
        /// Called by framework to reset the property
        /// </summary>
        private void ResetPreviewSize()
        {
            PreviewSize = DefaultPreviewSize;
        }

        /// <summary>
        /// Get/Set whether a border should be drawn around the graphical preview
        /// </summary>
        /// <seealso cref="ShowPreview"/>
        /// <seealso cref="PreviewSize"/>
        [Category("Appearance"),
        Description("Should a border be drawn around the graphic preview"),
        DefaultValue(true)]
        public bool DrawPreviewBorder
        {
            get { return _drawPreviewBorder; }
            set { _drawPreviewBorder = value; }
        }

        /// <summary>
        /// Set the <see cref="CellData"/> properties for cells that this binding 
        /// applies to.
        /// </summary>
        /// <remarks>
        /// This method is typically overridden by derived classes to set the 
        /// <see cref="CellData.Value"/> property based on the particular data mapping.
        /// The base method is usually called by the derived methods to set the
        /// non-data driven properties of the <see cref="CellData"/>
        /// </remarks>
        /// <param name="row">The row that the cell belongs to</param>
        /// <param name="cellData">The data to be displayed in the cell</param>
        public virtual void GetCellData(Row row, CellData cellData)
        {
            cellData.Editor = Editor;
            cellData.IsRichTextFormat = IsRichTextFormat;
            cellData.Format = Format;
            cellData.ToolTip = ToolTip;
            cellData.AlwaysDisplayToolTip = AlwaysDisplayToolTip;
            cellData.EvenStyle = EvenStyle;
            cellData.OddStyle = OddStyle;
            cellData.PrintOddStyle = PrintOddStyle;
            cellData.PrintEvenStyle = PrintEvenStyle;
            cellData.ShowPreview = ShowPreview;
            cellData.ShowText = ShowText;
            cellData.PreviewSize = PreviewSize;
            cellData.DrawPreviewBorder = DrawPreviewBorder;
        }

        /// <summary>
        /// Change the value of the cell this binding applies to in the given row
        /// </summary>
        /// <param name="row">The row to set the value for</param>
        /// <param name="oldValue">The old value of the cell</param>
        /// <param name="newValue">The new value of the cell</param>
        /// <returns>True if the value is successfully changed</returns>
        /// <remarks>
        /// This method allows a cell binding to support mapping changes to values
        /// back into the underlying <see cref="VirtualTree.DataSource"/> when the user 
        /// uses the associated <see cref="CellEditor"/> to modify displayed values.   The 
        /// default implementation does not support this and always returns false
        /// </remarks>
        public virtual bool SetValue(Row row, object oldValue, object newValue)
        {
            return false;
        }

        #endregion

        #region Local Methods

        
        /// <summary>
        /// Attach the cell binding to the given row binding
        /// </summary>
        internal void SetRowBinding(RowBinding binding)
        {
            _rowBinding = binding;
            InitializeStyles();
        }

        /// <summary>
        /// Setup the relationships between styles
        /// </summary>
        internal protected virtual void InitializeStyles()
        {
            // only update the styles when we are fully configured
            if (_column == null || _rowBinding == null || _rowBinding.Tree == null) return;

            _style.ParentStyle = _column.CellStyle; 
            _oddStyle.ParentStyle = new Style(_column.CellOddStyle, _style.Delta);
            _evenStyle.ParentStyle = new Style(_column.CellEvenStyle, _style.Delta);

            _printStyle.ParentStyle = new Style(_column.CellPrintStyle, _style.Delta);
            _printOddStyle.ParentStyle = _printStyle.Copy(_column.CellPrintStyle, _column.CellPrintOddStyle);
            _printEvenStyle.ParentStyle = _printStyle.Copy(_column.CellPrintStyle, _column.CellPrintEvenStyle);
        }

        /// <summary>
        /// Calls RowBinding.DisplayErrorMessage to display an error message
        /// </summary>
        /// <param name="message">The message to display</param>
        protected virtual void DisplayErrorMessage(string message)
        {
            if (RowBinding != null)
            {
                RowBinding.DisplayErrorMessage(message);
            }
        }

        /// <summary>
        /// Should binding exceptions be suppressed
        /// </summary>
        protected bool SuppressBindingExceptions
        {
            get { return (Tree == null) ? false : Tree.SuppressBindingExceptions; }
        }

        #endregion
    }


}
