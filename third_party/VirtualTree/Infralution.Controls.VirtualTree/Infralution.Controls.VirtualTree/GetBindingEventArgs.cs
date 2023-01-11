#region File Header
//
//      FILE:   GetBindingEventArgs.cs.
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
    /// Allows <see cref="VirtualTree.GetBinding"/> clients to specify the <see cref="RowBinding"/> to be
    /// used for a particular item.
    /// </summary>
    public class GetBindingEventArgs : System.EventArgs
    {
        #region Member Variables

        private Row              _row;
        private object          _item;
        private RowBinding      _rowBinding;

        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="item">The item to get the binding for </param>
        public GetBindingEventArgs(object item)
        {
            _item = item;        
        }

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="row">The row to get the binding for </param>
        public GetBindingEventArgs(Row row)
        {
            _row = row;        
            _item = row.Item;
        }

        /// <summary>
        /// The Item to be displayed
        /// </summary>
        public object Item
        {
            get { return _item; }
        }

        /// <summary>
        /// The Row containing the <see cref="Item"/> (if known)
        /// </summary>
        /// <remarks>
        /// This may be null if the position of the item in the tree has not yet been 
        /// established - typically when finding rows.
        /// </remarks>
        public Row Row
        {
            get { return _row; }
        }

        /// <summary>
        /// The row binding to use for this item (set by the client)
        /// </summary>
        public RowBinding RowBinding
        {
            get { return _rowBinding; }
            set { _rowBinding = value; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="VirtualTree.GetBinding"/> event  
    /// </summary>
    public delegate void GetBindingHandler(object sender, GetBindingEventArgs e);

}
