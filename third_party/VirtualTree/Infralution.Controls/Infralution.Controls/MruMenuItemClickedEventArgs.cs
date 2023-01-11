#region File Header
//
//      FILE:   MruMenuItemClickedEventArgs.cs.
//
// COPYRIGHT:   Copyright 2007 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
#endregion
using System;
using System.Windows.Forms;
namespace Infralution.Controls
{

    /// <summary>
    /// Arguments passed to clients of MruMenuItem events
    /// </summary>
    /// <seealso cref="MruToolStripMenuItem"/>
    public class MruMenuItemClickedEventArgs : ToolStripItemClickedEventArgs
    {
        #region Member Variables

        private string _entry;
 
        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="entry">The entry</param>
        /// <param name="clickedItem">The corresponding menu item</param>
        public MruMenuItemClickedEventArgs(string entry, ToolStripItem clickedItem)
            : base(clickedItem)
        {
            _entry = entry;
        }

        /// <summary>
        /// The MRU Menu Entry
        /// </summary>
        public string Entry
        {
            get { return _entry; }
        }


        #endregion
    }

    /// <summary>
    /// Represents a method that will handle the MruMenuItemClicked event  
    /// </summary>
    public delegate void MruMenuItemClickedHandler(object sender, MruMenuItemClickedEventArgs e);


}
