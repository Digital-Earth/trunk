/******************************************************************************
StopwatchProfiler.cs

begin      : November 26, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities
{
    /// <summary>
    /// A manual profile tool, which logs the time elapsed between calls to 
    /// LogCompletion.
    /// </summary>
    public class StopwatchProfiler
    {
        private string m_name;

        /// <summary>
        /// Gets or sets the name.
        /// </summary>
        /// <value>The name.</value>
        public string Name
        {
            get { return m_name; }
            set { m_name = value; }
        }

        private Pyxis.Utilities.TraceTool m_externalLog = null;

        /// <summary>
        /// Gets or sets the log.
        /// </summary>
        /// <value>The log.</value>
        private Pyxis.Utilities.TraceTool ExternalLog
        {
            get { return m_externalLog; }
            set { m_externalLog = value; }
        }

        private StringBuilder m_output = new StringBuilder();

        /// <summary>
        /// Gets the output.
        /// </summary>
        /// <value>The output.</value>
        public String Output
        {
            get
            {
                return m_output.ToString();
            }
        }

        /// <summary>
        /// Gets the output and clears away the contents.
        /// </summary>
        /// <returns></returns>
        public string GetOutputAndClear()
        {
            string result = m_output.ToString();
            m_output = new StringBuilder();
            return result;
        }

        private object m_timingMutex = new object();

        /// <summary>
        /// Initializes a new instance of the <see cref="StopwatchProfiler"/> class.
        /// </summary>
        /// <param name="name">The name.</param>
        /// <param name="log">The log.</param>
        public StopwatchProfiler(string name, Pyxis.Utilities.TraceTool log): this( name)
        {
            ExternalLog = log;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="StopwatchProfiler"/> class.
        /// </summary>
        /// <param name="name">The name.</param>
        public StopwatchProfiler(string name)
        {
            Name = name;
            StartStopwatch();
        }

        private System.Diagnostics.Stopwatch m_timeFromStart = new System.Diagnostics.Stopwatch();
        private System.Diagnostics.Stopwatch m_timeFromLastEvent = new System.Diagnostics.Stopwatch();

        /// <summary>
        /// Starts the stopwatch.
        /// </summary>
        private void StartStopwatch()
        {
            m_timeFromStart.Start();
            m_timeFromLastEvent.Start();
        }

        /// <summary>
        /// Logs the completion of a timed event, along with its timing information.
        /// </summary>
        /// <remarks>
        /// You will almost certainly want to enable this in a release build.  
        /// Profiling the debug build is not terribly informative.
        /// </remarks>
        /// <param name="operationName">Name of the operation.</param>
        //[System.Diagnostics.Conditional("DEBUG")]
        public void LogCompletion(string operationName)
        {
            string currentMessage;
            lock (m_timingMutex)
            {
                m_timeFromLastEvent.Stop();
                m_timeFromStart.Stop();
                currentMessage = string.Format("{0}:{1} took {2} ms ({3} total elapsed)",
                    Name, operationName, 
                    m_timeFromLastEvent.ElapsedMilliseconds,
                    m_timeFromStart.ElapsedMilliseconds);
                m_timeFromLastEvent.Reset();
                m_timeFromLastEvent.Start();
                m_timeFromStart.Start();
            }
            if (ExternalLog != null)
            {
                ExternalLog.WriteLine(currentMessage);
            }
            m_output.AppendLine(currentMessage);
        }
    }
}
