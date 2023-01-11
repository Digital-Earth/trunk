/******************************************************************************
DataPickedEventArgs.cs

begin      : November 13, 2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace ApplicationUtility
{
    /// <summary>
    /// The argument to pass when a DataPicked event is raised.
    /// </summary>
    public class DataPickedEventArgs : EventArgs
    {
        public DataPickedEventArgs(
            Dictionary<IProcess_SPtr, PYXCell> pickedProcs)
        {
            m_pickedProcesses = pickedProcs;
        }

        /// <summary>
        /// The processes that have been picked and for each process, the 
        /// cell they have been picked at.
        /// </summary>
        public Dictionary<IProcess_SPtr, PYXCell> PickedProcesses
        {
            get
            {
                return m_pickedProcesses;
            }
        }
        private Dictionary<IProcess_SPtr, PYXCell> m_pickedProcesses =
            new Dictionary<IProcess_SPtr, PYXCell>();
    }
}
