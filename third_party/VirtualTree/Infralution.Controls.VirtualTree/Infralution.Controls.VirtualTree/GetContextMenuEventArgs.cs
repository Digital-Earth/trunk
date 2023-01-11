#region File Header
//
//      FILE:   GetContextMenuStripEventArgs.cs.
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
    /// Allows <see cref="VirtualTree.GetContextMenuStrip"/> clients to specify the <see cref="ContextMenuStrip"/> to be used for a
    /// specific <see cref="Row"/>.
    /// </summary>
    public class GetContextMenuStripEventArgs : System.EventArgs
    {
        #region Member Variables

        private Row _row;
        private ContextMenuStrip _contextMenuStrip;
 
        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="row">The row to get the context menu for</param>
        public GetContextMenuStripEventArgs(Row row)
        {
            _row = row;                
        }

        /// <summary>
        /// The row to get the ContextMenu for
        /// </summary>
        public Row Row
        {
            get { return _row; }
        }

        /// <summary>
        /// Set/get the ContextMenu to use for the given row.
        /// </summary>
        public ContextMenuStrip ContextMenuStrip
        {
            get { return _contextMenuStrip; }
            set { _contextMenuStrip = value; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="VirtualTree.GetContextMenuStrip"/> event   
    /// </summary>
    public delegate void GetContextMenuStripHandler(object sender, GetContextMenuStripEventArgs e);


}
