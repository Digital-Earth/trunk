#region File Header
//
//      FILE:   GetAllowedRowDropLocationsEventArgs.cs.
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
    /// Allows <see cref="VirtualTree.GetAllowedRowDropLocations"/> clients to specify the ]
    /// <see cref="RowDropLocation">Drop Locations</see> supported by the given <see cref="Row"/>.
    /// </summary>
    public class GetAllowedRowDropLocationsEventArgs : System.EventArgs
    {
        #region Member Variables

        private Row              _row;
        private IDataObject      _data;
        private RowDropLocation  _allowedDropLocations;
  
        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="row">The row to get the allowed drop locations for</param>
        /// <param name="data">The data to be dropped</param>
        public GetAllowedRowDropLocationsEventArgs(Row row, IDataObject data)
        {
            _row = row;                
            _data = data;
        }

        /// <summary>
        /// The row to get the allowed drop locations for
        /// </summary>
        public Row Row
        {
            get { return _row; }
        }

        /// <summary>
        /// The data being dropped
        /// </summary>
        public IDataObject Data
        {
            get { return _data; }
        }

        /// <summary>
        /// Set/get the allowed drop locations
        /// </summary>
        public RowDropLocation AllowedDropLocations
        {
            get { return _allowedDropLocations; }
            set { _allowedDropLocations = value; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="VirtualTree.GetAllowedRowDropLocations"/> event 
    /// </summary>
    public delegate void GetAllowedRowDropLocationsHandler(object sender, GetAllowedRowDropLocationsEventArgs e);


}
