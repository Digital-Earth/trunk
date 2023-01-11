/******************************************************************************
TraceTool.cs

begin      : February 23, 2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet
{
    /// <summary>
    /// Encapsulates a specific trace.  Each trace can be enabled or disabled.
    /// </summary>
    public class TraceTool
    {
        /// <summary>
        /// Global control of enabling TraceTools.  This should always be checked in as false,
        /// and set to true if you need to do seom debugging in PyxNet.  GlobalTraceLogEnabled
        /// should be used to initialize all newly constructed TraceTools or NumberedTraceTools
        /// in the system.
        /// </summary>
        public static readonly bool GlobalTraceLogEnabled = false;

        /// <summary>
        /// This list of strings holds all the trace messages, even if the trace to the 
        /// debug window is turned off.  To easily access this list, in the debugger's
        /// command window type "? TraceTool.m_logMessages"
        /// 
        /// If the log is longer than 100 lines, then you can use ?? TraceTool.m_logMessages
        /// and this will open it in the Quick Watch window so you can scroll through the
        /// whole thing.
        /// </summary>
        private static List<string> s_logMessages = new List<string>();

        public static List<string> LogMessages
        {
            get { return TraceTool.s_logMessages; }
        }

        private List<string> m_localLogMessages = new List<string>();

        public List<string> LocalLogMessages
        {
            get { return m_localLogMessages; }
        }
        private bool m_logEnabled = true;

        private bool m_enabled = false;

        /// <summary>
        /// Is the trace enabled?  If not, no output will be logged.
        /// </summary>
        public bool Enabled
        {
            get { return m_enabled; }
            set { m_enabled = value; }
        }

        /// <summary>
        /// Storage for the trace prefix string.
        /// </summary>
        private string m_tracePrefix = "";

        /// <summary>
        /// A string that will be added to the begining of trace lines.
        /// Use this to identify which trace the information is coming from.
        /// </summary>
        public string TracePrefix
        {
            get { return m_tracePrefix; }
            set { m_tracePrefix = value; }
        }

        /// <summary>
        /// Constuct an enabled/disabled trace.
        /// </summary>
        /// <param name="enabled">Set to true to make the trace output.</param>
        public TraceTool(bool enabled)
        {
            m_enabled = enabled;
        }

        /// <summary>
        /// Constuct an enabled/disabled trace with a prefix message.
        /// </summary>
        /// <param name="enabled">Set to true to make the trace output.</param>
        /// <param name="prefixMessage">Text that will be written to the 
        /// beginning of each WriteLine of this trace object. (Not for blank lines.)</param>
        public TraceTool(bool enabled, string prefixMessage) : this (enabled)
        {
            TracePrefix = prefixMessage;
        }

        /// <summary>
        /// Output the log to a temporary file.
        /// </summary>
        /// <returns>The name of the temporary file.</returns>
        public static string SaveToTempFile()
        {
            string tempFileName = System.IO.Path.GetTempFileName();
            System.IO.File.WriteAllLines(tempFileName, LogMessages.ToArray());
            return tempFileName;
        }

        /// <summary>
        /// Writes a line to the output (diagnostic window).
        /// </summary>
        /// <param name="text">The text.</param>
        public void WriteLine(string text)
        {
            WriteLine("{0}", text);
        }

        /// <summary>
        /// Writes a line to the output (diagnostic window).
        /// </summary>
        /// <param name="format"></param>
        /// <param name="args"></param>
        public void WriteLine(string format, params object[] args)
        {
            StringBuilder outputLine = new StringBuilder(TracePrefix);
            outputLine.Append(String.Format("Thread {0}: ",
                    System.Threading.Thread.CurrentThread.ManagedThreadId));
            outputLine.Append(String.Format(format, args));

            if (Enabled)
            {
                System.Diagnostics.Trace.WriteLine(outputLine.ToString());
                m_localLogMessages.Add(outputLine.ToString());
            }

            // Question: Should we log if we're not enabled?
            if (m_logEnabled)
            {
                lock (s_logMessages)
                {
                    s_logMessages.Add(outputLine.ToString());
                }
            }
        }

        /// <summary>
        /// Writes a line to the output (diagnostic window).
        /// </summary>
        /// <param name="format"></param>
        /// <param name="args"></param>
        public void Write(string format, params object[] args)
        {
            if (Enabled)
            {
                System.Diagnostics.Trace.Write(
                    String.Format(format, args));
            }
        }

        /// <summary>
        /// Produces a blank line of output.
        /// </summary>
        public void WriteLine()
        {
            if (Enabled)
            {
                System.Diagnostics.Trace.WriteLine("");
            }
        }

        // TODO: Move process info methods someplace cleaner.
        /// <summary>
        /// Writes information about all process threads in an OS process.
        /// </summary>
        /// <param name="processName">The process.  If null, nothing is written.</param>
        public void WriteProcessInfo(System.Diagnostics.Process process)
        {
            if (Enabled && process != null)
            {
                WriteLine("process: {0},  id: {1}", process.ProcessName, process.Id);

                System.Diagnostics.ProcessThreadCollection threads = process.Threads;
                foreach (System.Diagnostics.ProcessThread pt in threads)
                {
                    try
                    {
                        DateTime startTime = pt.StartTime;
                        IntPtr startAddress = pt.StartAddress;
                        TimeSpan cpuTime = pt.TotalProcessorTime;
                        int priority = pt.BasePriority;
                        System.Diagnostics.ThreadState ts = pt.ThreadState;

                        WriteLine("  thread:  {0}", pt.Id);
                        WriteLine("    started: {0}", startTime.ToString());
                        WriteLine("    start address: {0}", startAddress.ToString());
                        WriteLine("    CPU time: {0}", cpuTime);
                        WriteLine("    priority: {0}", priority);
                        WriteLine("    thread state: {0}", ts.ToString());
                    }
                    catch (Exception e)
                    {
                        WriteLine("Exception accessing thread properties: {0}", e);
                    }
                }
            }
        }

        // TODO: Move process info methods someplace cleaner.
        /// <summary>
        /// Writes information about all process threads in the current OS process.
        /// </summary>
        public void WriteProcessInfo()
        {
            WriteProcessInfo(System.Diagnostics.Process.GetCurrentProcess());
        }

        // TODO: Move process info methods someplace cleaner.
        /// <summary>
        /// Writes information about all OS processes.
        /// </summary>
        public void WriteProcessesInfo()
        {
            if (Enabled)
            {
                // Write process info for all processes.
                System.Diagnostics.Process[] processes = System.Diagnostics.Process.GetProcesses();
                foreach (System.Diagnostics.Process process in processes)
                {
                    WriteProcessInfo(process);
                }
            }
        }

        /// <summary>
        /// Produces output for debug builds only.
        /// </summary>
        /// <param name="format"></param>
        /// <param name="args"></param>
        [System.Diagnostics.Conditional("DEBUG")]
        public void DebugWriteLine(string format, params object[] args)
        {
            WriteLine(format, args);
        }

        /// <summary>
        /// Produces output for debug builds only.
        /// </summary>
        /// <param name="format"></param>
        /// <param name="args"></param>
        [System.Diagnostics.Conditional("DEBUG")]
        public void DebugWrite(string format, params object[] args)
        {
            Write(format, args);
        }

        /// <summary>
        /// Produces a blank line of output for debug builds only.
        /// </summary>
        [System.Diagnostics.Conditional("DEBUG")]
        public void DebugWriteLine()
        {
            WriteLine();
        }

        // TODO: Move process info methods someplace cleaner.
        /// <summary>
        /// Writes information about all process threads in an OS process for debug builds only.
        /// </summary>
        /// <param name="processName">The process.  If null, nothing is written.</param>
        [System.Diagnostics.Conditional("DEBUG")]
        public void DebugWriteProcessInfo(System.Diagnostics.Process process)
        {
            WriteProcessInfo(process);
        }

        // TODO: Move process info methods someplace cleaner.
        /// <summary>
        /// Writes information about all process threads in the current OS process for debug builds only.
        /// </summary>
        [System.Diagnostics.Conditional("DEBUG")]
        public void DebugWriteProcessInfo()
        {
            WriteProcessInfo();
        }

        // TODO: Move process info methods someplace cleaner.
        /// <summary>
        /// Writes information about all OS processes for debug builds only.
        /// </summary>
        [System.Diagnostics.Conditional("DEBUG")]
        public void DebugWriteProcessesInfo()
        {
            WriteProcessesInfo();
        }
    }

    public class NumberedTraceTool<T>: TraceTool
    {
        private static int s_count = 0;

        public NumberedTraceTool( bool enabled, string prefixMessage): 
            base( enabled, String.Format( "{0} #{1}:", prefixMessage, s_count++))
        {
        }

        public NumberedTraceTool(bool enabled)
            :
            this(enabled, typeof(T).Name)
        {
        }
    }
}
