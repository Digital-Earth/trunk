#region File Header
//
//      FILE:   GetColumnInContextEventArgs.cs.
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
    /// Allows <see cref="VirtualTree.GetColumnInContext"/> clients to specify the whether a given <see cref="Column"/>
    /// should be displayed given the current state of the <see cref="VirtualTree"/>.
    /// </summary>
    public class GetColumnInContextEventArgs : System.EventArgs
    {
        #region Member Variables

        private Column _column;
        private bool  _inContext = true;
 
        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="column">The column to get the information for</param>
        public GetColumnInContextEventArgs(Column column)
        {
            _column = column;                
        }

        /// <summary>
        /// The column to determine the context for
        /// </summary>
        public Column Column
        {
            get { return _column; }
        }

        /// <summary>
        /// Set/get whether the column should be displayed in the current context.
        /// </summary>
        public bool InContext
        {
            get { return _inContext; }
            set { _inContext = value; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="VirtualTree.GetColumnInContext"/> event  
    /// </summary>
    public delegate void GetColumnInContextHandler(object sender, GetColumnInContextEventArgs e);


}
