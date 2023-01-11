#region File Header
//
//      FILE:   SpanningRowWidget.cs.
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
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Windows.Forms;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a <see cref="RowWidget"/> that displays a single <see cref="CellWidget"/> spanning
    /// all columns (both pinned and scrollable).
    /// </summary>
    /// <remarks>
    /// Use this RowWidget for rows where you want to display a single cell that spans all columns.  The
    /// SpanningRowWidget creates a single <see cref="CellWidget"/> for the <see cref="RowWidget.MainColumn"/>
    /// and sizes it to fill the entire row.
    /// </remarks>
    public class SpanningRowWidget : RowWidget
    {
        /// <summary>
        /// Create a new widget
        /// </summary>
        /// <param name="panelWidget">The parent PanelWidget</param>
        /// <param name="row">The row the widget is for</param>
        public SpanningRowWidget(PanelWidget panelWidget, Row row)
            : base(panelWidget, row)
        {
        }

        /// <summary>
        /// Overridden to allow the widget to span both the pinned and scrollable panels
        /// </summary>
        /// <returns></returns>
        public override Rectangle PanelClipBounds()
        {
            if (Columns.Contains(MainColumn))
            {
                return Tree.DisplayRectangle;
            }
            return base.PanelClipBounds();
        }

        /// <summary>
        /// Overridden to get the optimal row height based on the spanned column width
        /// </summary>
        /// <param name="graphics">The graphics context</param>
        public override int GetOptimalRowHeight(Graphics graphics)
        {
            CellWidget cellWidget = this.GetCellWidget(MainColumn);
            return cellWidget.GetOptimalHeight(graphics, cellWidget.Bounds.Width);
        }

        /// <summary>
        /// Overridden so that spanning rows return zero for all but the main column
        /// </summary>
        /// <param name="column">The column to get the width for</param>
        /// <param name="graphics">The graphics context</param>
        /// <returns>The optimal column width</returns>
        public override int GetOptimalColumnWidth(Column column, Graphics graphics)
        {
            if (column == MainColumn)
                return base.GetOptimalColumnWidth(column, graphics);
            else
                return 0;
        }

        /// <summary>
        /// Return the width of the spanning row
        /// </summary>
        public virtual int SpanningRowWidth
        {
            get
            {
                if (PanelWidget.Printing)
                    return Bounds.Width;
                else if (MainColumn.Pinned)
                {
                    int width = Tree.TotalColumnWidth;
                    if (ShowRowHeader)
                        width += Tree.RowHeaderWidth;
                    return Math.Max(width, Tree.DisplayWidth);
                }
                else
                {
                    return Bounds.Width;
                }
            }
        }

        private bool inLayout = false;


        /// <summary>
        /// Overridden to display data for the <see cref="RowWidget.MainColumn"/> across the width of the 
        /// whole row.
        /// </summary>
        /// <remarks>
        /// Modifies the <see cref="RowWidget.Columns"/> so that only the <see cref="CellWidget"/>  for
        /// the <see cref="RowWidget.MainColumn"/> is displayed.  Adjusts the bounds of the CellWidget 
        /// so that it fills the whole row.
        /// </remarks>
        public override void OnLayout()
        {
            if (inLayout) return;
            inLayout = true;

            Rectangle cellBounds = Bounds;
            ChildWidgets.Clear();

            // add the row header if any
            //
            if (ShowRowHeader)
            {
                cellBounds.Width = Tree.RowHeaderWidth;
                RowHeaderWidget.Bounds = RtlTranslateRect(cellBounds);
                ChildWidgets.Add(RowHeaderWidget);
                cellBounds.X += cellBounds.Width;
            }

            if (Columns.Contains(MainColumn))
            {
                Rectangle rowBounds = Bounds;
                int rowWidth = SpanningRowWidth;
                if (RightToLeft == RightToLeft.Yes)
                {
                    rowBounds.X += rowBounds.Width - rowWidth;
                }
                rowBounds.Width = rowWidth;
                Bounds = rowBounds;
                if (this.RightToLeft == RightToLeft.Yes)
                {
                    cellBounds.Width = rowBounds.Width - (rowBounds.Right - cellBounds.Right);
                    cellBounds.X = rowBounds.X;
                }
                else
                {
                    cellBounds.Width = rowBounds.Width - cellBounds.X;
                }
                CellWidget cellWidget = GetCellWidget(MainColumn);
                cellWidget.Bounds = cellBounds;
                ChildWidgets.Add(cellWidget);

                // add the prefix cell widget if any
                //
                Column prefixColumn = Tree.PrefixColumn;
                if (RowData.ShowPrefixColumn && prefixColumn != null && prefixColumn.Visible)
                {
                    Rectangle prefixBounds = cellWidget.GetPrefixBounds(prefixColumn);

                    // check the widget is visible before adding it
                    //
                    if (prefixBounds.Width > 0)
                    {
                        CellWidget prefixWidget = GetCellWidget(prefixColumn);

                        // if both the cell editor and value are null then don't display
                        // the prefix 
                        //
                        CellData cellData = prefixWidget.CellData;
                        if (cellData.Editor != null || cellData.Value != null)
                        {
                            prefixWidget.Bounds = prefixBounds;
                            ChildWidgets.Add(prefixWidget);

                            // set the width to reserve for the prefix widget
                            //
                            cellWidget.PrefixWidth = prefixBounds.Width;
                        }
                    }
                }
            }
            else
            {
                // Park widgets that are not active so that their editor (if any) is hidden
                //
                foreach (CellWidget cW in CellWidgets)
                {
                    cW.Bounds = Rectangle.Empty;
                }
            }
            inLayout = false;
        }   // OnLayout

    }   // SpanningRowWidget
}   // namespace
