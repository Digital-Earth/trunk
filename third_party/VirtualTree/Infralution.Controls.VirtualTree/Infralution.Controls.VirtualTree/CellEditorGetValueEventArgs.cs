#region File Header
//
//      FILE:   CellEditorGetValueEventArgs.cs.
//
//    AUTHOR:   Grant Frisken
//
// COPYRIGHT:   Copyright 2006
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
#endregion
using System;
using System.Windows.Forms;
namespace Infralution.Controls.VirtualTree
{
    /// <summary>
    /// Allows <see cref="CellEditor.GetControlValue"/> clients to programmatically handle getting a cell value from an editor control
    /// </summary>
    public class CellEditorGetValueEventArgs : CellEditorEventArgs
    {

        #region Member Variables

        private object _value;
  
        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="cellWidget">The CellWidget the editor control is associated with</param>
        /// <param name="control">The editor control</param>
        public CellEditorGetValueEventArgs(CellWidget cellWidget, Control control)
            : base(cellWidget, control)
        {
        }

        /// <summary>
        /// The value set by the control
        /// </summary>
        /// <remarks>
        /// This is the value that is set back into the cell and passed to the <see cref="VirtualTree.SetCellValue"/> event
        /// </remarks>
        public object Value
        {
            get { return _value; }
            set { _value = value; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="CellEditor.GetControlValue"/> event   
    /// </summary>
    public delegate void CellEditorGetValueHandler(object sender, CellEditorGetValueEventArgs e);


}
