using System;
using System.Collections.Generic;
using System.IO;

namespace PyxisCLI.Workspaces
{
    internal class LocalFileWatcher
    {
        private class FileEntry
        {
            public Action<string> Callback { get; set; }
            public DateTime LastWriteTime { get; set; }
        }

        private class DirectoryEntry
        {
            public Action<string,string> Callback { get; set; }
            public DateTime LastWriteTime { get; set; }
        }

        private readonly Dictionary<string, FileSystemWatcher> m_watchers;
        private readonly Dictionary<string, FileEntry> m_filesCallbacks;
        private readonly Dictionary<string, DirectoryEntry> m_directoryCallbacks;

        public LocalFileWatcher()
        {
            m_watchers = new Dictionary<string, FileSystemWatcher>();
            m_filesCallbacks = new Dictionary<string, FileEntry>();
            m_directoryCallbacks = new Dictionary<string, DirectoryEntry>();
        }

        private void CreateWatcher(string directory)
        {
            if (m_watchers.ContainsKey(directory))
            {
                return;
            }

            var watcher = new FileSystemWatcher(directory);
            watcher.IncludeSubdirectories = false;
            watcher.Changed += EventHandler;
            watcher.Created += CreatedEventHandler;
            watcher.Deleted += DeletedEventHandler;
            watcher.Renamed += RenameEventHandler;
            watcher.EnableRaisingEvents = true;
            m_watchers[directory] = watcher;
        }

        public void Watch(string file, Action<string> action)
        {
            file = Path.GetFullPath(file);
            var directory = Path.GetDirectoryName(file);

            CreateWatcher(directory);

            m_filesCallbacks[file] = new FileEntry()
            {
                Callback = action,
                LastWriteTime = File.GetLastWriteTime(file)
            };
        }

        public void WatchDirectory(string directory, Action<string, string> action)
        {
            CreateWatcher(directory);

            m_directoryCallbacks[directory] = new DirectoryEntry()
            {
                Callback = action,
                LastWriteTime = File.GetLastWriteTime(directory)
            };            
        }

        private void RenameEventHandler(object sender, RenamedEventArgs renamedEventArgs)
        {
            HandlePath(renamedEventArgs.OldFullPath);
            HandlePath(renamedEventArgs.FullPath);
            HandleDirectory(renamedEventArgs.FullPath, "created");
        }

        private void EventHandler(object sender, FileSystemEventArgs fileSystemEventArgs)
        {
            HandlePath(fileSystemEventArgs.FullPath);
        }

        private void CreatedEventHandler(object sender, FileSystemEventArgs fileSystemEventArgs)
        {
            HandlePath(fileSystemEventArgs.FullPath);
            HandleDirectory(fileSystemEventArgs.FullPath,"created");
        }

        private void DeletedEventHandler(object sender, FileSystemEventArgs fileSystemEventArgs)
        {
            HandlePath(fileSystemEventArgs.FullPath);
            HandleDirectory(fileSystemEventArgs.FullPath, "deleted");
        }

        private void HandleDirectory(string path, string eventType)
        {
            var directory = Path.GetDirectoryName(path);

            if (!m_directoryCallbacks.ContainsKey(directory))
            {
                return;
            }

            System.Threading.Thread.Sleep(100);

            var lastWriteTime = File.GetLastWriteTime(directory);
            var entry = m_directoryCallbacks[directory];
            
            if (lastWriteTime == entry.LastWriteTime)
            {
                return;
            }

            entry.LastWriteTime = lastWriteTime;
            entry.Callback(path,eventType);
        }

        private void HandlePath(string path)
        {
            if (!m_filesCallbacks.ContainsKey(path))
            {
                return;
            }

            System.Threading.Thread.Sleep(100);

            var lastWriteTime = File.GetLastWriteTime(path);
            var entry = m_filesCallbacks[path];
            if (lastWriteTime == entry.LastWriteTime)
            {
                return;
            }

            entry.LastWriteTime = lastWriteTime;
            entry.Callback(path);
        }
    }
}