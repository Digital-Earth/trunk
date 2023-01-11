/******************************************************************************
Exceptions.cs

begin      : 07/03/2007 3:02:41 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;
using System.Collections.Generic;
using System.Text;

namespace ApplicationUtility
{
    /// <summary>
    /// The generic exception that is thrown when the LibraryViewer encounters
    /// an error it can not handle. 
    /// </summary>
    public class WorldViewException : System.ApplicationException
    {
        /// <summary>
        /// Generic exception for the application.
        /// </summary>
        protected WorldViewException(string str) : base(str) { }
    }

    /// <summary>
    /// Exception indicating an error occurred during the search for or 
    /// addition of a pipeline to the library.
    /// </summary>
    public class DragDropException : WorldViewException
    {
        /// <summary>
        /// Application exception relating to an error with a drag and drop operation.
        /// </summary>
        public DragDropException(string str) : base(str) { }
    }
}

