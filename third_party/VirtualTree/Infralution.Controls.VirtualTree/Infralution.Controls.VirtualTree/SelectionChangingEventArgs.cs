#region File Header
//
//      FILE:   SelectionChangingEventArgs.cs.
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
    /// Allows the <see cref="VirtualTree.SelectionChanging"/> clients to cancel a change in selection 
    /// for a <see cref="VirtualTree"/> or display a warning if the selection is very large.
    /// </summary>
    public class SelectionChangingEventArgs : System.EventArgs
    {
        #region Member Variables

        private Row  _startRow;
        private Row  _endRow;
        private SelectionChange _change;
        private bool _cancel = false;
  
        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="startRow">The first row affected by the selection change</param>
        /// <param name="endRow">The last row affected by the selection change</param>
        /// <param name="change">The change to the selection</param>
        public SelectionChangingEventArgs(Row startRow, Row endRow, SelectionChange change)
        {
            _startRow = startRow;                
            _endRow = endRow;                
            _change = change;
        }

        /// <summary>
        /// The first row affected by the selection change
        /// </summary>
        public Row StartRow
        {
            get { return _startRow; }
        }

        /// <summary>
        /// The last row affected by the selection change
        /// </summary>
        public Row EndRow
        {
            get { return _endRow; }
        }

        /// <summary>
        /// Returns the change to the selection
        /// </summary>
        public SelectionChange Change
        {
            get { return _change; }
        }

        /// <summary>
        /// Set/Get whether the selection change should be allowed
        /// </summary>
        public bool Cancel
        {
            get { return _cancel; }
            set { _cancel = value; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="VirtualTree.SelectionChanging"/> event.  
    /// </summary>
    public delegate void SelectionChangingHandler(object sender, SelectionChangingEventArgs e);


}
