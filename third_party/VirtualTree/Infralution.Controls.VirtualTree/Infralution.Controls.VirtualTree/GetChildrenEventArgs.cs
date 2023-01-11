#region File Header
//
//      FILE:   GetChildrenEventArgs.cs.
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
    /// Allows <see cref="VirtualTree.GetChildren"/> clients to specify the children for a <see cref="Row"/> programmatically
    /// rather than using databinding to do this.
    /// </summary>
    public class GetChildrenEventArgs : System.EventArgs
    {
        #region Member Variables

        private Row    _row;
        private IList  _children;
  
        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="row">The row to get the children of</param>
        public GetChildrenEventArgs(Row row)
        {
            _row = row;                
        }

        /// <summary>
        /// The row to get the children of
        /// </summary>
        public Row Row
        {
            get { return _row; }
        }

        /// <summary>
        /// Set/get the children of the given row
        /// </summary>
        public IList Children
        {
            get { return _children; }
            set { _children = value; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="VirtualTree.GetChildren"/> event  
    /// </summary>
    public delegate void GetChildrenHandler(object sender, GetChildrenEventArgs e);


}
