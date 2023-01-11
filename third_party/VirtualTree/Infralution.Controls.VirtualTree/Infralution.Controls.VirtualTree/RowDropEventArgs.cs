#region File Header
//
//      FILE:   RowDropEventArgs.cs.
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
    /// Allows <see cref="VirtualTree.RowDrop"/> clients to handle dropping of data onto the 
    /// given <see cref="Row"/>.
    /// </summary>
    public class RowDropEventArgs : System.EventArgs
    {
        #region Member Variables

        private Row              _row;
        private RowDropLocation  _dropLocation;
        private IDataObject      _data;
        private DragDropEffects  _dropEffect;
  
        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="row">The row being dropped on</param>
        /// <param name="dropLocation">The drop location</param>
        /// <param name="data">The data to be dropped</param>
        /// <param name="dropEffect">The type of drop operation</param>
        public RowDropEventArgs(Row row, 
                                RowDropLocation dropLocation, 
                                IDataObject data,
                                DragDropEffects dropEffect)
        {
            _row = row;                
            _dropLocation = dropLocation;
            _data = data;
            _dropEffect = dropEffect;
        }

        /// <summary>
        /// The row being dropped on.  Maybe null when dropping data onto the
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
        /// The type of drop operation
        /// </summary>
        public DragDropEffects DropEffect
        {
            get { return _dropEffect; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="VirtualTree.RowDrop"/>  event  
    /// </summary>
    public delegate void RowDropHandler(object sender, RowDropEventArgs e);


}
