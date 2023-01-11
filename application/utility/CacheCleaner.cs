/******************************************************************************
CacheCleaner.cs

begin      : 28/09/2007 9:40:10 AM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Text;

namespace ApplicationUtility
{
    /// <summary>
    /// This class is designed to maintain a maximum (user configurable) size of
    /// a users persistant cache. All time consuming work is done in a separate
    /// thread. The cleaner will start removing files until the directory is
    /// some number of bytes smaller than the maximum size.
    /// </summary>
    public class CacheCleaner
    {
        /// <summary>
        /// Sets the cleaner to default value of a 1000MB cache size, 0%
        /// buffer and an interval of 0 so that cleaning happen 
        /// once for each call and no timer will be started.
        /// </summary>
        public CacheCleaner()
        {
            m_buffer = 0;
            m_intervalSeconds = 0;
            m_dirSize = 1024000;
        }

        /// <summary>
        /// Create a new cleaner with user defined settings
        /// </summary>
        /// <param name="directory">The path to monitor and clean</param>
        /// <param name="interval">The interval for checking the size (seconds)</param>
        /// <param name="size">The maximum size of the directory (MB)</param>
        /// <param name="buffer">The % below maximum to reduce the directory size to (0 - 99)</param>
        /// <returns></returns>
        public CacheCleaner(string directory, int interval, long size, int buffer)
        {
            CacheDirectory = directory;
            Interval = interval;
            MaxDirectorySizeInMB = size;
            DeleteBuffer = buffer;
        }

        #region Settings

        /// <summary>
        /// The directory to act upon. 
        /// </summary>
        public string CacheDirectory
        {
            get { return m_path; }
            set 
            {
                Trace.debug("Setting the cleaning directory to: " + value);
                m_path = value;
            }
        }
        private string m_path;

        /// <summary>
        /// The number of seconds to wait between each size check on 
        /// the cache directory.
        /// </summary>
        public int Interval
        {
            get { return m_intervalSeconds; }
            set
            {
                if (value < 0)
                {
                    throw new System.Exception("Interval must be 0 or greater.");
                }
                Trace.debug(string.Format("Setting new interval of {0}.", value));
                m_intervalSeconds = value;
            }
        }
        int m_intervalSeconds;


        /// <summary>
        /// The maximum size in megabytes the directory can be before the cleaner will
        /// start to delete files.
        /// </summary>
        public long MaxDirectorySizeInMB
        {
            get { return m_dirSize / 1024 / 1024; }
            set
            {
                value *= 1024;
                value *= 1024;
                MaxDirectorySizeInBytes = value;
            }
        }

        /// <summary>
        /// The maximum size in bytes the directory can be before the cleaner will
        /// start to delete files.
        /// </summary>
        public long MaxDirectorySizeInBytes
        {
            get { return m_dirSize; }
            set 
            {
                if (value < 0)
                {
                    throw new System.Exception("Max directory size must be positive.");
                }

                m_dirSize = value;
            }
        }
        long m_dirSize;

        /// <summary>
        /// The percentage (0 - 99) of the maximum size to remove below the max limit once
        /// a cleaning cycle begins.
        /// 
        /// Example:
        /// Maximum Size 1000
        /// Buffer 10%
        /// 
        /// The cleaner will start when the size reaches 1001 or greater. The cleaner
        /// will continue to run until the size is 10% below the maximum, 900 or less.
        /// </summary>
        public int DeleteBuffer
        {
            get { return m_buffer; }
            set
            {
                if (value < 100 && 0 <= value)
                {
                    m_buffer = value;
                }
                else
                {
                    throw new System.Exception("Delete buffer is a % and must be between 0 and 99.");
                }
            }
        }
        int m_buffer;

        #endregion

        #region Starting and Stopping

        /// <summary>
        /// Start the cleaner with a delay.
        /// </summary>
        /// <param name="delay">Number of seconds to delay starting.</param>
        public void StartCleaner(int delay)
        {
            if (m_startDelayTimer != null)
            {
                throw new ApplicationException(
                    "Delay Timer for cache cleaner already running. Can't start again.");
            }
            m_startDelayTimer = new System.Windows.Forms.Timer();
            m_startDelayTimer.Tick += new EventHandler(DelayStartTick);
            m_startDelayTimer.Interval = delay * 1000;
            m_startDelayTimer.Start();            
        }
        private System.Windows.Forms.Timer m_startDelayTimer;

        private void DelayStartTick(object source, EventArgs e)
        {
            m_startDelayTimer.Stop();
            m_startDelayTimer.Tick -= new EventHandler(DelayStartTick);
            m_startDelayTimer = null;
            if (!StartCleaner())
            {
                throw new ApplicationException("Failed to delay start the cache cleaner.");
            }
        }

        /// <summary>
        /// Start performing the cleaning operation at the set interval. The object
        /// will continue to perform the action at the specified interval until it
        /// is stopped. Starting the cleaner with an interval of 0 will perform a 
        /// single pass through the specified directory.
        /// </summary>
        /// <returns>True if the cleaning was started otherwise false.</returns>
        public bool StartCleaner()
        {
            if (m_isCleaning)
            {
                Trace.error("The object is already cleaning '" + m_path + ", can't start again.");
                return false;
            }
            else if (!System.IO.Directory.Exists(m_path))
            {
                Trace.error("Can't start cleaning a directory that does not exist: " + m_path);
                return false;
            }

            m_isCleaning = true;
            System.Diagnostics.Debug.Assert(m_cleaningTimer == null);

            if (m_intervalSeconds == 0)
            {
                // Start a thread to perform a single clean
                StartCleaningThread();
            }
            else
            {
                // Start a timer to clean periodically
                Trace.info("Creating a new directory cleaning timer for: " + m_path);
                m_cleaningTimer = new System.Windows.Forms.Timer();
                m_cleaningTimer.Tick += new EventHandler(CleanTimerTick);
                m_cleaningTimer.Interval = m_intervalSeconds * 1000;
                m_cleaningTimer.Start();
            }
            return true;
        }

        /// <summary>
        /// Indicates that a thread is currently cleaning a directory. This
        /// property can be true even after the StopCleaner method since it
        /// remains true as long as threads or timers are still running.
        /// </summary>
        public bool IsCleaning
        {
            get { return m_isCleaning; }
        }
        bool m_isCleaning;

        /// <summary>
        /// This method will stop any further timers from firing and send cancel
        /// notifications to any running threads. This does not mean that all 
        /// operations will stop immediately. All operations will be complete
        /// when the 
        /// </summary>
        public void StopCleaner()
        {
            // Cancel the timer.
            if (m_cleaningTimer != null)
            {
                Trace.info("Stopping the directory cleaning timer for: " + m_path);
                m_cleaningTimer.Stop();
                m_cleaningTimer.Tick -= new EventHandler(CleanTimerTick);
                m_cleaningTimer = null;
            }

            // Cancel the thread if it is running
            if (m_cleaningThread != null)
            {
                m_cleaningThread.CancelAsync();
            }
        }

        #endregion

        #region Timer

        /// <summary>
        /// The timer that fires periodically and starts a cache cleaning.
        /// </summary>
        private System.Windows.Forms.Timer m_cleaningTimer;

        /// <summary>
        /// When the timer goes off a new thread will be created to 
        /// clean out a directory. The timer will also be stopped until
        /// the thread has completed running.
        /// </summary>
        private void CleanTimerTick(object source, EventArgs e)
        {
            m_cleaningTimer.Stop();
            StartCleaningThread();
        }

        #endregion

        #region Thread

        /// <summary>
        /// The thread that performs the work of cleaning a directory.
        /// </summary>
        private BackgroundWorker m_cleaningThread;

        /// <summary>
        /// Create a new thread object to reduce the size of a directory down
        /// to a maximum size. The method will throw an exception if a thread 
        /// is already running.
        /// </summary>
        private void StartCleaningThread()
        {
            System.Diagnostics.Debug.Assert(m_cleaningThread == null);

            Trace.info("Starting to clean directory " + m_path);
            m_cleaningThread = new BackgroundWorker();
            m_cleaningThread.WorkerSupportsCancellation = true;
            m_cleaningThread.DoWork += new DoWorkEventHandler(CleaningThread_DoWork);
            m_cleaningThread.RunWorkerCompleted += new RunWorkerCompletedEventHandler(CleaningThread_RunWorkerCompleted);
            CleanInfo info = new CleanInfo(m_path, m_dirSize, m_buffer);
            m_cleaningThread.RunWorkerAsync(info);
        }

        /// <summary>
        /// Structure that holds all of the information that is needed by
        /// the 
        /// </summary>
        private class CleanInfo
        {
            public CleanInfo(string path, long size, long buffer)
            {
                m_path = path;
                m_size = size;
                m_buffer = buffer;
            }

            public CleanInfo(string path, long size)
            {
                m_path = path;
                m_size = size;
                m_buffer = 0;
            }

            public string m_path;
            public long m_size;
            public long m_buffer;
        };

        /// <summary>
        /// Perform the actual culling of the files down to the maximum size.
        /// </summary>
        private void CleaningThread_DoWork(object sender, DoWorkEventArgs e)
        {
            BackgroundWorker bw = sender as BackgroundWorker;
            CleanInfo cleanInfo = e.Argument as CleanInfo;
            CleanInfo returnInfo = new CleanInfo(cleanInfo.m_path, 0);

            // determine the current size of the directory
            System.IO.DirectoryInfo dirInfo = new System.IO.DirectoryInfo(cleanInfo.m_path);
            if (!dirInfo.Exists)
            {
                throw new System.IO.DirectoryNotFoundException("Directory not found for cleaning thread: " + dirInfo.FullName);
            }
            long dirSize = FileUtility.GetDirectorySize(dirInfo);

            // determine if the size is over the limit
            if (dirSize < cleanInfo.m_size)
            {
                // directory within parameters
                Trace.info(
                    string.Format("Directory '{0}' with size '{1}' bytes ({2} MB) has not reached its limit of '{3}' bytes ({4} MB).",
                    cleanInfo.m_path, dirSize, (dirSize / 1024 / 1024),
                    cleanInfo.m_size, (cleanInfo.m_size / 1024 / 1024)));
            }
            else
            {
                // covert the buffer size in % of maximum to a number of bytes
                long bufferInBytes = cleanInfo.m_buffer * cleanInfo.m_size / 100;
                long sizeToRemove = dirSize - cleanInfo.m_size + bufferInBytes;

                // directory needs cleaning
                Trace.info(string.Format("Attempting to remove at least '{0}' bytes ({1} MB) from directory '{2}'",
                    sizeToRemove, (sizeToRemove / 1024 / 1024), cleanInfo.m_path));

                // attempt to remove the desired amount
                returnInfo.m_size = FileUtility.RemoveLeastRecentlyWrittenFiles(
                    dirInfo, sizeToRemove, bw);

                if (bw.CancellationPending)
                {
                    // the tread was cancelled during operation
                    e.Cancel = true;
                }
            }
            e.Result = returnInfo;
        }
        
        /// <summary>
        /// The thread has returned from deleting files 
        /// </summary>
        private void CleaningThread_RunWorkerCompleted(
            object sender,
            RunWorkerCompletedEventArgs e)
        {
            try
            {
                if (e.Error != null)
                {
                    // an exception occurred during thread operation
                    Trace.error("An error was encountered while cleaning cache: \n" +
                        e.Error.Message);
                }
                else if (e.Cancelled)
                {
                    // the thread was cancelled before completion
                    Trace.info("Cache directory cleaning thread cancelled before completed.");
                }
                else
                {
                    // thread completed normally
                    CleanInfo info = e.Result as CleanInfo;
                    System.Diagnostics.Debug.Assert(info != null);

                    Trace.info(string.Format("Finished cleaning {0} bytes ({1} MB) from directory '{2}'.",
                        info.m_size, (info.m_size / 1024 / 1024), info.m_path));
                }

                // delete the thread
                m_cleaningThread = null;

                // restart the timer if it exists
                if (m_cleaningTimer != null)
                {
                    Trace.debug("Re-starting cache cleaning timer");
                    m_cleaningTimer.Start();
                }
                else
                {
                    // there is no timer so all cleaning is completed
                    m_isCleaning = false;
                }
            }
            catch (Exception ex)
            {
                // Don't call Trace here, because we might have shut down.
                System.Diagnostics.Trace.WriteLine(string.Format(
                    "Unexpected failure in CacheCleaner: {0}", ex.ToString()));
            }
        }

        #endregion
    }
}
