using System;
using System.Collections.Generic;
using System.IO;

namespace Pyxis.Utilities
{
    public class DelayedFileSystemEventArgs : EventArgs
    {
        private List<FileSystemEventArgs> m_events;

        public IEnumerable<FileSystemEventArgs> Events
        {
            get
            {
                return m_events;
            }
        }

        public DelayedFileSystemEventArgs()
        {
            m_events = new List<FileSystemEventArgs>();
        }

        public void Add(FileSystemEventArgs eventArg)
        {
            m_events.Add(eventArg);
        }
    }

    /// <summary>
    /// A filesystem watcher with a timer to avoid raising
    /// multiple events in case frequent changes happen the same folder
    /// </summary>
    public class DelayedFileSystemWatcher
    {
        private TimeSpan m_delay = TimeSpan.FromSeconds(5);

        private DelayedFileSystemEventArgs m_fileSystemEventArg;

        private object m_fileSystemEventArgLock = new object();

        private System.Timers.Timer m_timer;

        private FileSystemWatcher m_watcher;

        public TimeSpan Delay
        {
            get
            {
                return m_delay;
            }
            set
            {
                m_delay = value;
                m_timer.Interval = m_delay.TotalMilliseconds;
            }
        }
        public bool Enabled
        {
            get
            {
                return m_watcher.EnableRaisingEvents;
            }
            set
            {
                m_timer.Enabled = (value) ? m_timer.Enabled : false;
                m_watcher.EnableRaisingEvents = value;
            }
        }

        public bool IncludeSubdirectories
        {
            get
            {
                return m_watcher.IncludeSubdirectories;
            }
            set
            {
                m_watcher.IncludeSubdirectories = value;
            }
        }
        public string Path
        {
            get
            {
                return m_watcher.Path.ToLower();
            }
        }

        public DelayedFileSystemWatcher(string path)
        {
            m_timer = new System.Timers.Timer(Delay.TotalMilliseconds);
            m_timer.Elapsed += TimerElapsed;

            m_watcher = new FileSystemWatcher(path);

            m_watcher.NotifyFilter = NotifyFilters.FileName | NotifyFilters.LastWrite |
                NotifyFilters.Size | NotifyFilters.Attributes | NotifyFilters.LastAccess;
            m_watcher.Changed += HandleChange;
            m_watcher.Created += HandleChange;
            m_watcher.Deleted += HandleChange;
            m_watcher.Renamed += HandleChange;

            m_watcher.EnableRaisingEvents = false;
        }

        public delegate void DelayedFileSystemEventHandler(object sender, DelayedFileSystemEventArgs e);

        public event DelayedFileSystemEventHandler Changed;

        private void HandleChange(object sender, FileSystemEventArgs e)
        {
            lock (m_fileSystemEventArgLock)
            {
                if (!m_timer.Enabled)
                {
                    m_timer.Enabled = true;
                    m_fileSystemEventArg = new DelayedFileSystemEventArgs();
                }
                m_fileSystemEventArg.Add(e);
            }
        }

        private void TimerElapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            DelayedFileSystemEventArgs eventArgs;
            lock (m_fileSystemEventArgLock)
            {
                eventArgs = m_fileSystemEventArg;
                m_fileSystemEventArg = new DelayedFileSystemEventArgs();
            }
            System.Threading.Tasks.Task.Factory.StartNew(
                    () => Changed.Invoke(this, eventArgs));
        }
    }
}