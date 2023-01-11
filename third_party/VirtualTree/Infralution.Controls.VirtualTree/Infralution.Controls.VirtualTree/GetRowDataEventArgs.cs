#region File Header
//
//      FILE:   GetRowDataEventArgs.cs.
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
    /// Allows <see cref="VirtualTree.GetRowData"/> clients to specify how a given <see cref="Row"/> should be displayed 
    /// by the <see cref="VirtualTree"/>
    /// </summary>
    public class GetRowDataEventArgs : System.EventArgs
    {
        #region Member Variables

        private Row             _row;
        private RowData         _rowData;

        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="row">The row to get the data for</param>
        /// <param name="rowData">The data to be displayed in the row (populated by the client)</param>
        public GetRowDataEventArgs(Row row, RowData rowData)
        {
            _row = row;        
            _rowData = rowData;
        }

        /// <summary>
        /// The Row to be displayed
        /// </summary>
        public Row Row
        {
            get { return _row; }
        }

        /// <summary>
        /// The data to be displayed in the row (populated by the client)
        /// </summary>
        public RowData RowData
        {
            get { return _rowData; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="VirtualTree.GetRowData"/> event  
    /// </summary>
    public delegate void GetRowDataHandler(object sender, GetRowDataEventArgs e);

}
