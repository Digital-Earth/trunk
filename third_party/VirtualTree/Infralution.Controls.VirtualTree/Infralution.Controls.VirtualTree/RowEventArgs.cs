#region File Header
//
//      FILE:   RowEventArgs.cs.
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
    /// Allows clients to handle events related to a <see cref="Row"/>
    /// </summary>
    public class RowEventArgs : System.EventArgs
    {
        #region Member Variables

        private Row             _row;

        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="row">The row the event involves</param>
        public RowEventArgs(Row row)
        {
            _row = row;        
        }

        /// <summary>
        /// The Row the event is associated with
        /// </summary>
        public Row Row
        {
            get { return _row; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the event  
    /// </summary>
    public delegate void RowEventHandler(object sender, RowEventArgs e);

}
