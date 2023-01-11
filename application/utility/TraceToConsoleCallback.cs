/******************************************************************************
begin		: 2006-10-13
copyright	: (C) 2006 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace ApplicationUtility
{  
    /// <summary>
    /// The sole responsibility of this class is to direct a copy
    /// of the PYXIS trace to the Console window. There can only
    /// ever be one trace callback attached to the PYXIS trace
    /// at one time.
    /// </summary>
    public class TraceToConsoleCallback : TraceCallback
    {
        public Trace.eLevel EnabledLevels { get; set; }

        /// <summary>
        /// The method that is called when a trace statement is executed.
        /// </summary>
        /// <param name="strMessage">
        /// The trace message.
        /// </param>
        public override void message(Trace.eLevel level, string strMessage)
        {
            //do bit-wise and on EnabledLevels and given level
            if ((EnabledLevels & level) != 0)
            {
                Console.WriteLine(strMessage);
            }
        }
    }
}
