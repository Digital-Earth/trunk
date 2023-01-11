#region File Header
//
//      FILE:   CellEditorSetValueEventArgs.cs.
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
    /// Allows <see cref="CellEditor.SetControlValue"/> clients to programmatically handle setting the value in an editor control
    /// </summary>
    public class CellEditorSetValueEventArgs : CellEditorEventArgs
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
        /// <param name="value">The cell value to set in the control</param>
        public CellEditorSetValueEventArgs(CellWidget cellWidget, Control control, object value)
            : base(cellWidget, control)
        {
            _value = value;
        }

        /// <summary>
        /// The cell value to set the control to
        /// </summary>
        public object Value
        {
            get { return _value; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="CellEditor.SetControlValue"/> event   
    /// </summary>
    public delegate void CellEditorSetValueHandler(object sender, CellEditorSetValueEventArgs e);


}
