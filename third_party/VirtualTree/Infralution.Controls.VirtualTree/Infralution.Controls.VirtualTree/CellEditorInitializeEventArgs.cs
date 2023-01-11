#region File Header
//
//      FILE:   CellEditorInitializeEventArgs.cs.
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
using System.Windows.Forms;
namespace Infralution.Controls.VirtualTree
{
    /// <summary>
    /// Allows <see cref="CellEditor"/> clients to control the initialization 
    /// of controls managed by the CellEditor.
    /// </summary>
    public class CellEditorInitializeEventArgs : CellEditorEventArgs
    {

        #region Member Variables

        private bool _newControl;
  
        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="cellWidget">The CellWidget the editor control is associated with</param>
        /// <param name="control">The editor control</param>
        /// <param name="newControl">Is the control new (or a previously cached control)</param>
        public CellEditorInitializeEventArgs(CellWidget cellWidget, Control control, bool newControl)
            : base(cellWidget, control)
        {
            _newControl = newControl;
        }

        /// <summary>
        /// Is the control a new control (ie not a previously cached control).
        /// </summary>
        /// <remarks>
        /// This allows you to perform different initialization for cached controls - which
        /// may only require certain properties to be reset that may have been modified by the
        /// user.
        /// </remarks>
        public bool NewControl
        {
            get { return _newControl; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="CellEditor.InitializeControl"/> event   
    /// </summary>
    public delegate void CellEditorInitializeHandler(object sender, CellEditorInitializeEventArgs e);


}
