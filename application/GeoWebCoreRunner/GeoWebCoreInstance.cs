using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GeoWebCoreRunner
{
    /// <summary>
    /// Represnts a GeoWebCore instance (process).
    /// 
    /// This class allow to get std input and out from GeoWebCore and also function to terminate (kill) the process.
    /// </summary>
    public class GeoWebCoreInstance
    {
        public enum InstanceState
        {
            Starting,
            Running,
            Exiting,
            Exited
        }

        public string Message { get; set; }

        /// <summary>
        /// The Process running the GeoWebCore
        /// </summary>
        public Process Process { get; private set; }
        
        private object m_lastOutputLock = new object();
        private List<string> m_lastOutput = new List<string>();

        /// <summary>
        /// Setting been used to start the GeoWebCore instance
        /// </summary>
        public GeoWebCoreInstanceSettings Settings { get; private set; }

        /// <summary>
        /// Event triggered when StdOut/StdError provide an output.
        /// </summary>
        public event EventHandler OutputChanged;

        /// <summary>
        /// Return a string represent the last few lines from the process output
        /// </summary>
        public string Output
        {
            get
            {
                lock (m_lastOutputLock)
                {
                    return string.Join("\n", m_lastOutput);
                }
            }
        }

        /// <summary>
        /// Event triggered when GeoWebCore process state has ben changed
        /// </summary>
        public event EventHandler StateChanged;

        /// <summary>
        /// Return the current state of the GeoWebCore process.
        /// </summary>
        public InstanceState State { get; private set; }
        
        private void UpdateState(InstanceState newState)
        {
            State = newState;
            if (StateChanged != null)
            {
                StateChanged.Invoke(this, null);
            }
        }

        private DateTime m_lastTotalProcessTimeCalled;
        private TimeSpan m_lastTotalProcessTime;

        private DateTime m_lastCpuActivity = DateTime.Now;

        public GeoWebCoreInstance(GeoWebCoreInstanceSettings settings, string message)
            : this(settings)
        {
            Message = message;
        }

        public GeoWebCoreInstance(GeoWebCoreInstanceSettings settings)
        {
            Settings = settings;

            var args = new List<string>();

            if (settings.NodeInfo.NodeId != Guid.Empty)
            {
                args.Add("-node=" + settings.NodeInfo.NodeId.ToString());
            }

            if (!String.IsNullOrEmpty(settings.Environment.FilesFolder))
            {
                args.Add("-fd=" + settings.Environment.FilesFolder);
            }

            if (!String.IsNullOrEmpty(settings.Environment.CacheFolder))
            {
                args.Add("-cd=" + settings.Environment.CacheFolder);
            }

            if (settings.Verbose)
            {
                args.Add("-verbose");
            }

            if (settings.Environment.Production)
            {
                args.Add("-env=Production");
            }

            if (!string.IsNullOrEmpty(settings.MasterUrl))
            {
                args.Add("-master="+settings.MasterUrl);
            }

            switch (settings.Mode)
            {
                case GeoWebCoreRunMode.Download:
                    if (settings.Files != null)
                    {
                        args.Add("-d");
                        args.AddRange(settings.Files);
                    }
                    break;
                case GeoWebCoreRunMode.Import:
                    if (settings.Files != null)
                    {
                        args.Add("-import");
                        args.AddRange(settings.Files);
                    }
                    break;
                case GeoWebCoreRunMode.Server:
                    if (!String.IsNullOrEmpty(settings.NodeInfo.Url))
                    {
                        args.Add(settings.NodeInfo.Url);
                    }
                    break;
                case GeoWebCoreRunMode.ValidateChecksums:
                    args.Add("-vc");
                    break;
            }

            Process = new Process();

            Process.StartInfo.FileName = Path.Combine(settings.Environment.GwcDir,settings.Environment.GwcFileName);
            Process.StartInfo.Arguments = String.Join(" ", args);
            Process.StartInfo.UseShellExecute = false;
            Process.StartInfo.WorkingDirectory = settings.Environment.GwcDir;
            Process.StartInfo.RedirectStandardInput = true;
            Process.StartInfo.RedirectStandardError = true;
            Process.StartInfo.RedirectStandardOutput = true;
            Process.StartInfo.CreateNoWindow = true;
            Process.EnableRaisingEvents = true;
            
            Process.ErrorDataReceived += Process_OutputDataReceived;
            Process.OutputDataReceived += Process_OutputDataReceived;

            m_lastTotalProcessTimeCalled = DateTime.Now;
            m_lastTotalProcessTime = TimeSpan.Zero;

            Process.Start();
            Process.BeginErrorReadLine();
            Process.BeginOutputReadLine();

            UpdateState(InstanceState.Running);

            Process.Exited += Process_Exited;
        }

        /// <summary>
        /// Kill the GeoWebCore process
        /// </summary>
        /// <returns>Return true if process has exited.</returns>
        public bool Kill()
        {
            if (Process.HasExited)
            {
                return true;
            }

            UpdateState(InstanceState.Exiting);

            //send exit command
            Process.StandardInput.WriteLine("x");

            if (!Process.WaitForExit(30000))
            {
                //force kill
                Process.Kill();
            }

            return Process.HasExited;
        }

        void Process_Exited(object sender, EventArgs e)
        {
            UpdateState(InstanceState.Exited);
        }

        void Process_OutputDataReceived(object sender, DataReceivedEventArgs e)
        {
            lock (m_lastOutputLock)
            {
                m_lastOutput.Add(e.Data);
                if (m_lastOutput.Count > 100)
                {
                    m_lastOutput.RemoveAt(0);
                }
            }

            if (OutputChanged != null)
            {
                OutputChanged.Invoke(this, null);
            }
        }

        /// <summary>
        /// Return virtual memory allocated by the GeoWebCore Process.
        /// </summary>
        /// <returns>Memory allocated in bytes.</returns>
        public long GetMemory()
        {
            if (Process.HasExited)
            {
                return 0;
            }
            return Process.VirtualMemorySize64;
        }

        /// <summary>
        /// Return percent of cores been used by the process.
        /// </summary>
        /// <returns>Sum of usage of cores (number between 0..1 for every core been used).</returns>
        public double GetProcessUsage()
        {
            Process.Refresh();
            var processTime = Process.TotalProcessorTime;
            var readTime = DateTime.Now;

            var percent = (processTime.TotalSeconds - m_lastTotalProcessTime.TotalSeconds) / (readTime - m_lastTotalProcessTimeCalled).TotalSeconds;

            m_lastTotalProcessTimeCalled = readTime;
            m_lastTotalProcessTime = processTime;

            //less than 1% percent CPU time
            if (percent * 100 >= 1.0)
            {
                m_lastCpuActivity = DateTime.Now;
            }

            return percent;
        }

        public DateTime GetLastCpuActivity()
        {
            return m_lastCpuActivity;
        }
    }
}
