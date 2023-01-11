#region File Header
//
//      FILE:   GetAllowRowDragEventArgs.cs.
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
namespace Infralution.Controls.VirtualTree
{
    /// <summary>
    /// Allows <see cref="VirtualTree.GetAllowRowDrag"/> clients to determine whether the given <see cref="Row"/> can
    /// be dragged and dropped
    /// </summary>
    public class GetAllowRowDragEventArgs : System.EventArgs
    {
        #region Member Variables

        private Row   _row;
        private bool  _allowDrag = false;
  
        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="row">The row being dragged</param>
        public GetAllowRowDragEventArgs(Row row)
        {
            _row = row;                
        }

        /// <summary>
        /// The row being dragged
        /// </summary>
        public Row Row
        {
            get { return _row; }
        }

        /// <summary>
        /// Set/get whether the drag operation should be allowed
        /// </summary>
        public bool AllowDrag
        {
            get { return _allowDrag; }
            set { _allowDrag = value; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="VirtualTree.GetAllowRowDrag"/>  event  
    /// </summary>
    public delegate void GetAllowRowDragHandler(object sender, GetAllowRowDragEventArgs e);


}
