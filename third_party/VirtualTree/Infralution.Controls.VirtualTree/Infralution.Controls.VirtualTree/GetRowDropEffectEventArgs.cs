#region File Header
//
//      FILE:   GetRowDropEffectEventArgs.cs.
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
using System.Windows.Forms;
namespace Infralution.Controls.VirtualTree
{
    /// <summary>
    /// Allows <see cref="VirtualTree.GetRowDropEffect"/> clients to specify the supported <see cref="DragDropEffects"/>
    /// when drag-dropping data onto a <see cref="Row"/>
    /// </summary>
    public class GetRowDropEffectEventArgs : System.EventArgs
    {
        #region Member Variables

        private Row              _row;
        private RowDropLocation  _dropLocation;
        private IDataObject      _data;
        private DragDropEffects  _dropEffect = DragDropEffects.None;
  
        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="row">The row the data will be dropped on</param>
        /// <param name="dropLocation">The drop location</param>
        /// <param name="data">The data to be dropped</param>
        public GetRowDropEffectEventArgs(Row row, 
                                         RowDropLocation dropLocation, 
                                         IDataObject data)
        {
            _row = row;                
            _dropLocation = dropLocation;
            _data = data;
        }

        /// <summary>
        /// The row to determine the drop effects for.  Maybe null when dropping data onto the
        /// empty space below the rows.
        /// </summary>
        public Row Row
        {
            get { return _row; }
        }

        /// <summary>
        /// Get the drop location
        /// </summary>
        public RowDropLocation DropLocation
        {
            get { return _dropLocation; }
        }

        /// <summary>
        /// The data being dropped
        /// </summary>
        public IDataObject Data
        {
            get { return _data; }
        }

        /// <summary>
        /// Set this to determine the type of drop operation to use for the 
        /// given row, data and location
        /// </summary>
        public DragDropEffects DropEffect
        {
            get { return _dropEffect; }
            set { _dropEffect = value; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="VirtualTree.GetRowDropEffect"/> event   
    /// </summary>
    public delegate void GetRowDropEffectHandler(object sender, GetRowDropEffectEventArgs e);


}
