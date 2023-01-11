#region File Header
//
//      FILE:   ColumnCustomizeWidget.cs.
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
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Collections;
using NS=Infralution.Controls.VirtualTree;
using System.Diagnostics;
namespace Infralution.Controls.VirtualTree
{

    /// <summary>
    /// Defines a <see cref="Widget"/> for displaying the columns to be customized 
    /// </summary>
    /// <remarks>
    /// <para>
    /// This class is used by the <see cref="ColumnCustomizeControl"/> to display the column
    /// headers to allow them to be dragged and dropped by the user.
    /// </para>
    /// </remarks>
    /// <seealso cref="NS.VirtualTree"/>
    /// <seealso cref="NS.ColumnHeaderWidget"/>
    public class ColumnCustomizeWidget : Widget
    {

        #region Member Variables
        
        /// <summary>
        /// The column header widgets indexed by column
        /// </summary>
        private Hashtable _columnWidgets = new Hashtable();

        #endregion

        #region Public Interface

        /// <summary>
        /// Creates a new widget.
        /// </summary>
        /// <param name="control">The control that owns the widget</param>
        public ColumnCustomizeWidget(ColumnCustomizeControl control)
            : base(control)
        {
        }

        /// <summary>
        /// Return the control the widget is associated with
        /// </summary>
        public ColumnCustomizeControl ColumnCustomizeControl
        {
            get { return WidgetControl as ColumnCustomizeControl; }
        }

        /// <summary>
        /// Return the height required to display all of the column headers
        /// </summary>
        public int RequiredHeight
        {
            get 
            {
                int requiredHeight = 0;
                VirtualTree tree = ColumnCustomizeControl.Tree;
                if (tree != null)
                {
                    int headerHeight = tree.HeaderHeight;
                    foreach (Column column in tree.Columns)
                    {
                        if (!column.Active && column.InContext && !column.PrefixColumn)
                        {
                            requiredHeight += headerHeight;
                        }
                    }            
                }
                return requiredHeight;
            }
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Get the column header widget for the given column
        /// </summary>
        /// <param name="column">The column to get the widget for</param>
        /// <returns>The widget for the given column</returns>
        protected virtual ColumnHeaderWidget GetColumnHeaderWidget(Column column)
        {
            ColumnHeaderWidget widget = (ColumnHeaderWidget)_columnWidgets[column];
            if (widget == null)
            {
                widget = ColumnCustomizeControl.Tree.CreateColumnHeaderWidget(this, column);
                _columnWidgets[column] = widget;
            }
            return widget;
        }

        /// <summary>
        /// Update this widget and its children on layout
        /// </summary>
        public override void OnLayout()
        {

            VirtualTree tree = ColumnCustomizeControl.Tree;
            if (tree == null) return;
            Rectangle bounds = Bounds;
            bounds.Height = tree.HeaderHeight;
            this.ChildWidgets.Clear();
            foreach (Column column in tree.Columns)
            {
                if (!column.Active && column.InContext && !column.PrefixColumn && column.Movable)
                {
                    ColumnHeaderWidget widget = GetColumnHeaderWidget(column);
                    widget.Bounds = bounds;
                    ChildWidgets.Add(widget);
                    bounds.Y += bounds.Height;
                }
            }
        }

        #endregion
    }
}
