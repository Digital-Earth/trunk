using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities
{
    /// <summary>
    /// Helper class to encapsulate logging functions.
    /// Ultimately a call to the virtual function "Logger" is made which passes
    /// control to the application to define what happens with the log message.
    /// A companion trace object in maintained in parallel.
    /// </summary>
    public class LogHelper
    {
        public enum LogLevel { Info = 0, Warning, Error, Debug, Verbose, TraceOnly };

        public delegate void LogFunc(LogLevel serverId, string msg);

        private LogFunc m_HelperFunc = null;
        private Pyxis.Utilities.TraceTool m_Tracer = new Pyxis.Utilities.TraceTool(true);

        static readonly string[] m_levelStr = new string[] { "INFO", "WARNING", "ERROR", "DEBUG", "VERBOSE", "TRACE" };

        /// <summary>
        /// Initializes a new instance of the <see cref="LogHelper"/> class.
        /// Keeps reference to parent helper object.
        /// </summary>
        /// <param name="parentHelper">The parent helper object.</param>
        public LogHelper(LogFunc helperFunc)
        {
            m_HelperFunc = helperFunc;
        }

        /// <summary>
        /// Format log message, add severity level indicator, and thread id where appropriate.
        /// Add resulting message to trace object, and call parent's logging function.
        /// </summary>
        /// <param name="level">Message severity level.</param>
        /// <param name="fmt">Message format string.</param>
        /// <param name="args">Arguments for format string.</param>
        private void ProcessMessage(LogLevel level, string fmt, params object[] args)
        {
            string message = string.Format("{0,-7} {1}", m_levelStr[(int)level], string.Format(fmt, args));

            m_Tracer.WriteLine(message);
            if( level != LogLevel.TraceOnly && m_HelperFunc != null)
            {
                //--
                //-- add thread id to logger string
                //--
                string threadId = string.Format("[{0:0000}] ", System.Threading.Thread.CurrentThread.ManagedThreadId);
                m_HelperFunc(level, threadId + message);
            }
        }

        //--
        //-- public logging routines
        //-- each accepts a formating directive, and accompanying arguments
        //--
        #region logging routines with variable arguments

        public void Info(string fmt, params object[] args)
        {
            ProcessMessage(LogLevel.Info, fmt, args);
        }

        public void Warning(string fmt, params object[] args)
        {
            ProcessMessage(LogLevel.Warning, fmt, args);
        }

        public void Error(string fmt, params object[] args)
        {
            ProcessMessage(LogLevel.Error, fmt, args);
        }

        public void Debug(string fmt, params object[] args)
        {
#if DEBUG
            ProcessMessage(LogLevel.Debug, fmt, args);
#endif
        }

        public void Verbose(string fmt, params object[] args)
        {
            ProcessMessage(LogLevel.Verbose, fmt, args);
        }

        public void Trace(string fmt, params object[] args)
        {
            ProcessMessage(LogLevel.TraceOnly, fmt, args);
        }

        #endregion

        //--
        //-- public logging routines
        //-- each accepts a simple message string
        //--
        #region simple logging routines

        public void Info(string message)
        {
            Info("{0}", message);
        }

        public void Warning(string message)
        {
            Warning("{0}", message);
        }
        
        public void Error(string message)
        {
            Error("{0}", message);
        }
        
        public void Debug(string message)
        {
            Debug("{0}", message);
        }

        public void Verbose(string message)
        {
            Verbose("{0}", message);
        }

        public void Trace(string message)
        {
            Trace("{0}", message);
        }

        #endregion
    }
 }
