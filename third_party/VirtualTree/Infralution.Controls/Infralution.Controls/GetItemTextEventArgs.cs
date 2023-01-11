#region File Header
//
//      FILE:   GetItemTextEventArgs.cs.
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
namespace Infralution.Controls
{

    /// <summary>
    /// Allows <see cref="VirtualListBox.GetItemText"/> clients to specify the text to be displayed
    /// for a given item
    /// </summary>
    public class GetItemTextEventArgs : System.EventArgs
    {
        #region Member Variables

        private object  _item;
        private string  _text;
 
        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="item">The item to get the text for</param>
        public GetItemTextEventArgs(object item)
        {
            _item = item;                
        }

        /// <summary>
        /// The item to get the text for
        /// </summary>
        public object Item
        {
            get { return _item; }
        }

        /// <summary>
        /// Set/get the text to display for the given item.
        /// </summary>
        public string Text
        {
            get { return _text; }
            set { _text = value; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="VirtualListBox.GetItemText"/> event 
    /// </summary>
    public delegate void GetItemTextHandler(object sender, GetItemTextEventArgs e);

}
