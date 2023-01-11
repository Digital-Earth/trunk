#region File Header
//
//      FILE:   CellEditorEventArgs.cs.
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
    /// Allows <see cref="CellEditor"/> clients to control the  
    /// of controls managed by the CellEditor.
    /// </summary>
    public class CellEditorEventArgs : System.EventArgs
    {

        #region Member Variables

        private CellWidget _cellWidget;
        private Control _control;
  
        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="cellWidget">The CellWidget the editor control is associated with</param>
        /// <param name="control">The editor control</param>
        public CellEditorEventArgs(CellWidget cellWidget, Control control)
        {
            _cellWidget = cellWidget;
            _control = control;                
        }

        /// <summary>
        /// The CellWidget the editor control is associated with
        /// </summary>
        public CellWidget CellWidget
        {
            get { return _cellWidget; }
        }

        /// <summary>
        /// The editor control
        /// </summary>
        public Control Control
        {
            get { return _control; }
        }

        #endregion
    }


}
