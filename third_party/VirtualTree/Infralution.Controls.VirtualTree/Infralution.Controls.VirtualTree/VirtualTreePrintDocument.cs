#region File Header
//
//      FILE:   VirtualTreePrintDocument.cs.
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
using System.Drawing.Printing;
using System.Collections;
using System.ComponentModel;
namespace Infralution.Controls.VirtualTree
{
	/// <summary>
	/// Defines a PrintDocument for printing the contents of a VirtualTree.
	/// </summary>
    /// <seealso href="XtraPrinting.html">Printing</seealso>
    [ToolboxItem(false)]
    public class VirtualTreePrintDocument : PrintDocument
	{

        #region Member Variables

        VirtualTree _tree;
        
        int _currentPage = 0;
        int _currentRowIndex = 0;
        int _currentColumnIndex = 0;

        int _startRowIndex = 0;
        int _endRowIndex = 0;
        SimpleColumnList _columns;
        bool _printColumnHeaders = true;
        bool _printRowHeaders = false;

        string _footerText = "Page No: {0}";
        string _headerText;
  
        #endregion

        #region Public Interface

        /// <summary>
        /// Default constructor
        /// </summary>
        public VirtualTreePrintDocument()
        {
        }

        /// <summary>
        /// Create a new print document for the given tree
        /// </summary>
        /// <param name="virtualTree">The tree to be printed</param>
        public VirtualTreePrintDocument(VirtualTree virtualTree)
        {
            _tree = virtualTree;
            if (_tree != null)
            {
                _printColumnHeaders = _tree.ShowColumnHeaders;
            }
        }

        /// <summary>
        /// Get/Set the tree that the document is to print
        /// </summary>
        public VirtualTree Tree
        {
            get { return _tree; }
            set 
            { 
                // reset the columns if the tree is changed
                //
                if (value != _tree)
                {
                    _columns = null;
                }
                _tree = value; 
                if (_tree != null)
                {
                    _printColumnHeaders = _tree.ShowColumnHeaders;
                }
            }
        }

        /// <summary>
        /// The absolute row index of the first row to print
        /// </summary>
        public int StartRowIndex
        {
            get 
            { 
                if (_tree != null && _startRowIndex == 0)
                {
                    return _tree.FirstRowIndex;
                }
                return _startRowIndex;  
            }
            set { _startRowIndex = value; }
        }

        /// <summary>
        /// The absolute row index of the last row to print
        /// </summary>
        public int EndRowIndex
        {
            get 
            { 
                if (_tree != null && _endRowIndex == 0)
                {
                    return _tree.LastRowIndex;
                }
                return _endRowIndex; 
            }
            set { _endRowIndex = value; }
        }

        /// <summary>
        /// The columns to print.
        /// </summary>
        /// <remarks>If not set then then prints the currently visible columns for the tree</remarks>
        public SimpleColumnList Columns
        {
            get
            {
                if (_tree != null && _columns == null)
                {
                    _columns = _tree.Columns.GetVisibleColumns();
                }
                return _columns;
            }
            set { _columns = value; }
        }

        /// <summary>
        /// Should Column Headers be printed
        /// </summary>
        public bool PrintColumnHeaders
        {
            get { return _printColumnHeaders; }
            set { _printColumnHeaders = value; }
        }

        /// <summary>
        /// Should Row Headers be printed on each page
        /// </summary>
        public bool PrintRowHeaders
        {
            get { return _printRowHeaders; }
            set { _printRowHeaders = value; }
        }

        /// <summary>
        /// The text to print in the footer
        /// </summary>
        /// <remarks>This can include a placeholder {0} for the page number</remarks>
        /// <seealso cref="PrintPageFooter"/>
        public string FooterText
        {
            get { return _footerText; }
            set { _footerText = value; }
        }

        /// <summary>
        /// The text to print in the header
        /// </summary>
        /// <remarks>This can include a placeholder {0} for the page number</remarks>
        /// <seealso cref="PrintPageHeader"/>
        public string HeaderText
        {
            get { return _headerText; }
            set { _headerText = value; }
        }

        #endregion

        #region Local Methods and Overrides

        /// <summary>
        /// Return the zero based page number of the current page being printed
        /// </summary>
        protected int CurrentPage
        {
            get { return _currentPage; }
        }

        /// <summary>
        /// Return the row index of the first row on the current page
        /// </summary>
        protected int CurrentRowIndex
        {
            get { return _currentRowIndex; }
        }

        /// <summary>
        /// Return the index of the first column on the current page
        /// </summary>
        protected int CurrentColumnIndex
        {
            get { return _currentColumnIndex; }
        }

        /// <summary>
        /// Return the columns to print on the current page 
        /// </summary>
        /// <remarks>
        /// Updates the <see cref="CurrentColumnIndex"/> to the index of the column to start printing
        /// the next page on.
        /// </remarks>
        /// <param name="pageWidth">The width of the page</param>
        /// <returns>A list of columns to print</returns>
        protected SimpleColumnList GetColumnsForPage(int pageWidth)
        {
            SimpleColumnList columns = new SimpleColumnList();
            int width = 0;
            if (PrintRowHeaders && _tree != null)
            {
                width +=  _tree.RowHeaderWidth;
            }
            while (width < pageWidth && _currentColumnIndex < Columns.Count)
            {
                Column column = Columns[_currentColumnIndex];
                width += column.Width;
                if (width < pageWidth || width == column.Width)
                {
                    columns.Add(column);
                    _currentColumnIndex++;
                }
            }

            if (_currentColumnIndex >= Columns.Count)
                _currentColumnIndex = 0;             
            return columns;
        }

        /// <summary>
        /// Return the bounds to print the VirtualTree data into
        /// </summary>
        /// <remarks>
        /// Allows derived classes to adjust the area that the VirtualTree prints into to make
        /// room for headers/footers etc.  The default method returns e.MarginBounds.
        /// </remarks>
        /// <param name="e">The print page event</param>
        /// <returns>Rectangle bounding the area to print the VirtualTree data</returns>
        protected virtual Rectangle GetAdjustedMarginBounds(PrintPageEventArgs e)
        {
            return e.MarginBounds;
        }

        /// <summary>
        /// Reset the current page to zero
        /// </summary>
        /// <param name="e"></param>
        protected override void OnBeginPrint(PrintEventArgs e)
        {
            base.OnBeginPrint(e);
            _currentPage = 0;
            _currentRowIndex = StartRowIndex;
            _currentColumnIndex = 0;
        }

        /// <summary>
        /// Print the header for the page
        /// </summary>
        /// <param name="e"></param>
        /// <remarks>By default prints the <see cref="HeaderText"/> at the top left</remarks>
        protected virtual void PrintPageHeader(PrintPageEventArgs e)
        {
            if (HeaderText != null)
            {
                Font font = Tree.HeaderPrintStyle.Font;
                string text = String.Format(HeaderText, CurrentPage); 
                e.Graphics.DrawString(text, font, Brushes.Black, e.MarginBounds.Left, e.MarginBounds.Top - font.Height); 
            }
        }

        /// <summary>
        /// Print the footer for the page
        /// </summary>
        /// <param name="e"></param>
        /// <remarks>By default prints the <see cref="FooterText"/> at the bottom left</remarks>
        protected virtual void PrintPageFooter(PrintPageEventArgs e)
        {
            if (FooterText != null)
            {
                string text = String.Format(FooterText, CurrentPage); 
                e.Graphics.DrawString(text, Tree.Font, Brushes.Black, e.MarginBounds.Left, e.MarginBounds.Bottom); 
            }
       }

        /// <summary>
        /// Prints the current page (including headers and footers) to the given graphics context (e.Graphics)
        /// </summary>
        /// <param name="e"></param>
        /// <remarks>
        /// Prints the contents of the page within the adjusted margin bounds 
        /// (given by <see cref="GetAdjustedMarginBounds"/>) and then calls <see cref="PrintPageHeader"/>
        /// and <see cref="PrintPageFooter"/>
        /// </remarks>
        protected virtual void PrintPageContents(PrintPageEventArgs e)
        {
            if (_tree == null) return;

            // Get the bounds to print into
            //
            Rectangle bounds = GetAdjustedMarginBounds(e);

            int headerHeight = (PrintColumnHeaders) ? _tree.HeaderHeight : 0;
            bool printRowHeaders = PrintRowHeaders && (CurrentColumnIndex == 0);
            SimpleColumnList columns = GetColumnsForPage(bounds.Width);
            int endRow = _tree.Print(e.Graphics, bounds, CurrentRowIndex, EndRowIndex, columns, PrintColumnHeaders, printRowHeaders);

            // have we finished ?
            //
            if ((endRow >= 0 && endRow < EndRowIndex) || CurrentColumnIndex > 0)
                e.HasMorePages = true;

            // if we have finished printing all the columns then increment the row index
            //
            if (CurrentColumnIndex == 0)
            {
                _currentRowIndex = endRow + 1;
            }

            PrintPageHeader(e);
            PrintPageFooter(e);
        }

        /// <summary>
        /// Adjust the page scale of the printer graphics context to match that of the
        /// display.
        /// </summary>
        /// <param name="graphics">The printer context</param>
        /// <remarks>
        /// This is called by <see cref="OnPrintPage"/> to ensure that the printed text 
        /// matches that displayed
        /// </remarks>
        protected virtual void AdjustPageScale(Graphics graphics)
        {
            using (Graphics screenGraphics = Graphics.FromHwnd(IntPtr.Zero))
            {
                graphics.PageScale = 100.0f / screenGraphics.DpiX;
            }
        }

        /// <summary>
        /// Handle printing of the current page with support for printing selected pages
        /// </summary>
        /// <param name="e"></param>
        /// <remarks>
        /// This method increments the <see cref="CurrentPage"/> and handles printing only those pages
        /// withing the given PrinterSettings.PrintRange.  It calls <see cref="PrintPageContents"/> 
        /// to do the actual printing.  Pages before the selected print range are printed to a dummy
        /// graphics context.
        /// </remarks>
        protected override void OnPrintPage(PrintPageEventArgs e)
        {
            AdjustPageScale(e.Graphics);
            base.OnPrintPage(e);
            _currentPage++;

            if (PrinterSettings.PrintRange == PrintRange.SomePages)
            {
                // create a dummy graphics context to print pages before the selected range to
                //
                Graphics dummyGraphics = Graphics.FromImage(new Bitmap(1, 1));
                PrintPageEventArgs args = new PrintPageEventArgs(dummyGraphics, e.MarginBounds, e.PageBounds, e.PageSettings);
                while (_currentPage < PrinterSettings.FromPage)
                {
                    PrintPageContents(args);
                    _currentPage++;
                    if (!args.HasMorePages) return;
                } 
                PrintPageContents(e);
                if (_currentPage >= PrinterSettings.ToPage)
                {
                    e.HasMorePages = false; 
                }
            }
            else
            {
                PrintPageContents(e);
            }
        }

        #endregion

    }
}
