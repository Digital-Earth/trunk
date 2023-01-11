#region File Header
//
//      FILE:   GetParentEventArgs.cs.
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
    /// Allows <see cref="VirtualTree.GetParent"/> clients to specify the parent for a <see cref="Row"/> programmatically
    /// rather than using databinding to do this.
    /// </summary>
    public class GetParentEventArgs : System.EventArgs
    {

        #region Member Variables

        private object  _item;
        private object  _parent;
  
        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="item">The item to get the parent of</param>
        public GetParentEventArgs(object item)
        {
            _item = item;                
        }

        /// <summary>
        /// The item to get the parent of
        /// </summary>
        public object Item
        {
            get { return _item; }
        }

        /// <summary>
        /// Set/get the parent of the given item
        /// </summary>
        public object Parent
        {
            get { return _parent; }
            set { _parent = value; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="VirtualTree.GetParent"/> event   
    /// </summary>
    public delegate void GetParentHandler(object sender, GetParentEventArgs e);


}
