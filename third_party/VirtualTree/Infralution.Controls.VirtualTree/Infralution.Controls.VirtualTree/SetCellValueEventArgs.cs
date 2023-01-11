#region File Header
//
//      FILE:   SetCellValueEventArgs.cs.
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
    /// Allows <see cref="VirtualTree.SetCellValue"/> clients to handle changes to a Cell value and change the
    /// underlying value in the <see cref="VirtualTree.DataSource"/>.
    /// </summary>
    public class SetCellValueEventArgs : System.EventArgs
    {
        #region Member Variables

        private Row             _row;
        private Column          _column;
        private object          _oldValue;
        private object          _newValue;
        private bool            _cancel = false;

        #endregion

        #region Public Interface

        /// <summary>
        /// Initialise a new instance of the event.
        /// </summary>
        /// <param name="row">The row to get the data for</param>
        /// <param name="column">The column to get data for</param>
        /// <param name="oldValue">The old value for the cell</param>
        /// <param name="newValue">The new value for the cell</param>
        public SetCellValueEventArgs(Row row, Column column, object oldValue, object newValue)
        {
            _row = row;        
            _column = column;
            _oldValue = oldValue;
            _newValue = newValue;
        }

        /// <summary>
        /// The Row containing the value
        /// </summary>
        public Row Row
        {
            get { return _row; }
        }

        /// <summary>
        /// The Column containing the value
        /// </summary>
        public Column Column
        {
            get { return _column; }
        }

        /// <summary>
        /// The old value for the cell
        /// </summary>
        public object OldValue
        {
            get { return _oldValue; }
        }

        /// <summary>
        /// The new value for the cell
        /// </summary>
        public object NewValue
        {
            get { return _newValue; }
        }

        /// <summary>
        /// Allows the client to cancel the change to the data value.
        /// </summary>
        public bool Cancel
        {
            get { return _cancel; }
            set { _cancel = value; }
        }

        #endregion
    }

    /// <summary>
    /// Represents the method that will handle the <see cref="VirtualTree.SetCellValue"/> event  
    /// </summary>
    public delegate void SetCellValueHandler(object sender, SetCellValueEventArgs e);

}
