#region File Header
//
//      FILE:   WidgetCreator.cs.
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
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a method used to create PanelWidgets
    /// </summary>
    /// <param name="tree">The VirtualTree the PanelWidget should be created for</param>
    /// <returns>A new PanelWidget</returns>
    public delegate PanelWidget PanelWidgetCreator(VirtualTree tree);

    /// <summary>
    /// Defines a method used to create RowWidgets
    /// </summary>
    /// <param name="panelWidget">The PanelWidget the RowWidget should be created for</param>
    /// <param name="row">The row the widget is for</param>
    /// <returns>A new RowWidget</returns>
    public delegate RowWidget RowWidgetCreator(PanelWidget panelWidget, Row row);

    /// <summary>
    /// Defines a method used to create CellWidgets
    /// </summary>
    /// <param name="rowWidget">The RowWidget the CellWidget should be created for</param>
    /// <param name="column">The column the widget is for</param>
    /// <returns>A new CellWidget</returns>
    public delegate CellWidget CellWidgetCreator(RowWidget rowWidget, Column column);

    /// <summary>
    /// Defines a method used to create HeaderWidgets
    /// </summary>
    /// <param name="panelWidget">The PanelWidget the HeaderWidget should be created for</param>
    /// <returns>A new HeaderWidget</returns>
    public delegate HeaderWidget HeaderWidgetCreator(PanelWidget panelWidget);

    /// <summary>
    /// Defines a method used to create ColumnHeaderWidgets
    /// </summary>
    /// <param name="parentWidget">The parentWidget for the ColumnHeader</param>
    /// <param name="column">The column the widget is for</param>
    /// <returns>A new ColumnHeaderWidget</returns>
    public delegate ColumnHeaderWidget ColumnHeaderWidgetCreator(Widget parentWidget, Column column);

    /// <summary>
    /// Defines a method used to create ColumnDividerWidgets
    /// </summary>
    /// <param name="columnHeaderWidget">The columnHeaderWidget the widget belongs to</param>
    /// <returns>A new ColumnDividerWidget</returns>
    public delegate ColumnDividerWidget ColumnDividerWidgetCreator(ColumnHeaderWidget columnHeaderWidget);
        
    /// <summary>
    /// Defines a method used to create RowHeaderWidgets
    /// </summary>
    /// <param name="rowWidget">The RowWidget the RowHeaderWidget should be created for</param>
    /// <returns>A new RowHeaderWidget</returns>
    public delegate RowHeaderWidget RowHeaderWidgetCreator(RowWidget rowWidget);

    /// <summary>
    /// Defines a method used to create RowDividerWidgets
    /// </summary>
    /// <param name="rowHeaderWidget">The RowHeaderWidget the RowDividerWidget should be created for</param>
    /// <returns>A new RowDividerWidget</returns>
    public delegate RowDividerWidget RowDividerWidgetCreator(RowHeaderWidget rowHeaderWidget);

    /// <summary>
    /// Defines a method used to create ExpansionWidgets
    /// </summary>
    /// <param name="rowWidget">The RowWidget the ExpansionWidget should be created for</param>
    /// <returns>A new ExpansionWidget</returns>
    public delegate ExpansionWidget ExpansionWidgetCreator(RowWidget rowWidget);


}
