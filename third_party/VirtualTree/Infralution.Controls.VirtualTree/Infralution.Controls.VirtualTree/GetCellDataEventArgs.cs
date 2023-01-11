#region File Header
//
//      FILE:   GetCellDataEventArgs.cs.
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
    /// Allows <see cref="VirtualTree.GetCellData"/> clients to specify how a given cell (defined by a
    /// <see cref="Row"/> and <see cref="Column"/>) should be displayed by the <see cref="VirtualTree"/>
    /// </summary>
    public class GetCellDataEventArgs : System.EventArgs
    {
        #region Member Variables

        private Row             _row;
        private Column          _column;
        private CellData        _cellData;

        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="row">The row to get the data for</param>
        /// <param name="column">The column to get data for</param>
        /// <param name="cellData">The data to be displayed in the cell (populated by the client)</param>
        public GetCellDataEventArgs(Row row, Column column, CellData cellData)
        {
            _row = row;        
            _column = column;
            _cellData = cellData;
        }

        /// <summary>
        /// The Row to be displayed
        /// </summary>
        public Row Row
        {
            get { return _row; }
        }

        /// <summary>
        /// The Column to be displayed
        /// </summary>
        public Column Column
        {
            get { return _column; }
        }

        /// <summary>
        /// The data to be displayed in the cell (populated by the client)
        /// </summary>
        public CellData CellData
        {
            get { return _cellData; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="VirtualTree.GetCellData"/> event   
    /// </summary>
    public delegate void GetCellDataHandler(object sender, GetCellDataEventArgs e);

}
