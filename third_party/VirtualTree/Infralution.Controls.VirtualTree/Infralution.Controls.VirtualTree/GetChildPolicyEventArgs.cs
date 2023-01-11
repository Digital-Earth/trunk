#region File Header
//
//      FILE:   GetChildPolicyEventArgs.cs.
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
    /// Allows <see cref="VirtualTree.GetChildPolicy"/> clients to specify the policy to be used for handling
    /// children of the given <see cref="Row"/> (ie AutoExpand, LoadOnExpand or Normal)
    /// </summary>
    public class GetChildPolicyEventArgs : System.EventArgs
    {
        #region Member Variables

        private Row             _row;
        private RowChildPolicy  _childPolicy = RowChildPolicy.Normal;
 
        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="row">The row to get the child policy for</param>
        public GetChildPolicyEventArgs(Row row)
        {
            _row = row;                
        }

        /// <summary>
        /// The row to get the ChildPolicy for
        /// </summary>
        public Row Row
        {
            get { return _row; }
        }

        /// <summary>
        /// Set/get the ChildPolicy for the given row.
        /// </summary>
        public RowChildPolicy ChildPolicy
        {
            get { return _childPolicy; }
            set { _childPolicy = value; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="VirtualTree.GetChildPolicy"/> event 
    /// </summary>
    public delegate void GetChildPolicyHandler(object sender, GetChildPolicyEventArgs e);


}
