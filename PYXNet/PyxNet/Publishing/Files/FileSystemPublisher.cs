using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Pyxis.Utilities;
using PyxNet.DataHandling;

namespace PyxNet.Publishing.Files
{
    /// <summary>
    /// This class is responsible to publish files and directories
    /// Directories published using this class will be watched for any changes
    /// and the published files will be updated accordingly
    /// </summary>
    internal class FileSystemPublisher
    {
        #region Fields And Properties

        public const int DefaultChunkSize = 65536;

        private readonly List<DelayedFileSystemWatcher> m_watchers = new List<DelayedFileSystemWatcher>();

        private Object m_publishedFilesLock = new Object();

        private PublishedFileRepository m_publishedRepository = new PublishedFileRepository();

        public PublishedFileRepository FileRepository
        {
            get
            {
                lock (m_publishedFilesLock)
                {
                    return m_publishedRepository;
                }
            }
        }

        public IEnumerable<FileInfo> PublishedFiles
        {
            get
            {
                lock (m_publishedFilesLock)
                {
                    return m_publishedRepository.PublishedFileInfos.SelectMany(x => x.SourceFiles).Select(x => x.ToFileInfo()).ToList();
                }
            }
        }

        #endregion Fields And Properties

        /// <summary>
        /// Constructor.
        /// </summary>
        public FileSystemPublisher()
        {
        }

        public PublishedFileInfo GetPFIByDataSetID(DataGuid dataGuid)
        {
            lock (m_publishedFilesLock)
            {
                return m_publishedRepository.GetPFIByDataSetID(dataGuid);
            }
        }

        public DataInfo Publish(FileInfo file)
        {
            return Publish(file, file.Name, DefaultChunkSize);
        }

        /// <summary>
        /// Publish a file on PyxNet.  This form of publish file should only be used if
        /// you are publishing a file that you think is unique and never been seen on PyxNet
        /// before.
        /// </summary>
        /// <param name="file">The file that you want to publish.</param>
        /// <param name="description">A text description that will be used for PyxNet query matching.</param>
        /// <param name="chunkSize">The size of chunks that the data will be transferred in.</param>
        /// <returns>The DataInfo that was created for this file which includes the DataID.</returns>
        public DataInfo Publish(FileInfo file, string description, int chunkSize)
        {
            try
            {
                var fileInfo = new FileInformation(file);

                DataInfo dataInfo = new DataInfo();
                dataInfo.DataChunkSize = chunkSize;
                dataInfo.DataLength = fileInfo.Length;
                dataInfo.AllAvailable = true;
                dataInfo.UseEncryption = false;
                dataInfo.UseSigning = false;
                dataInfo.UsesHashCodes = false;

                return Publish(fileInfo, description, dataInfo);
            }
            catch (FileNotFoundException)
            {
                // The file didn't exist, or disappeared during the operation.  Skip publishing.
                return null;
            }
        }

        public DataInfo Publish(FileInformation file, string description, DataInfo dataInfo)
        {
            lock (m_publishedFilesLock)
            {
                return m_publishedRepository.Publish(file, description, dataInfo);
            }
        }

        /// <summary>
        /// Publishes all the files in the given directory and then watches the
        /// directory for changes so that new files will also be published, and
        /// deleted files will cease to be published.
        /// </summary>
        /// <param name="directory">The directory that you want to publish.</param>
        /// <param name="recurse">if set to <c>true</c> [recurse].</param>
        /// <returns></returns>
        public DelayedFileSystemWatcher PublishDirectory(DirectoryInfo directory, bool recurse)
        {
            if (!directory.Exists)
            {
                return null;
            }

            //Check if we already publish this directory
            //or we publish its parents with subdirectories enabled
            var lowerCaseDirName = directory.FullName.ToLower();


            lock (m_publishedFilesLock)
            {
                var watcher = m_watchers.FirstOrDefault(x => lowerCaseDirName == x.Path
                 || (x.IncludeSubdirectories && lowerCaseDirName.StartsWith(x.Path + Path.DirectorySeparatorChar)));
                if (watcher != null)
                {
                    if (watcher.IncludeSubdirectories == false && recurse == true)
                    {
                        watcher.IncludeSubdirectories = true;
                    }
                    return watcher;
                }

                // look at all the files in the directory and publish them
                foreach (FileInfo file in directory.GetFiles("*",
                    recurse ? SearchOption.AllDirectories : SearchOption.TopDirectoryOnly))
                {
                    Publish(file);
                }

                watcher = new DelayedFileSystemWatcher(directory.FullName);
                watcher.IncludeSubdirectories = recurse;
                watcher.Changed += delegate(object sender, DelayedFileSystemEventArgs e)
                {
                    var syncMe = sender as DelayedFileSystemWatcher;
                    if (syncMe != null)
                    {
                        syncMe.Enabled = false;
                        SyncDirectory(syncMe.Path, syncMe.IncludeSubdirectories);
                        syncMe.Enabled = true;
                    }
                };
                watcher.Enabled = true;
                m_watchers.Add(watcher);
                return watcher;
            }
        }

        /// <summary>
        /// Unhook from the stack and stop publishing.
        /// </summary>
        public void StopPublishing()
        {
            lock (m_publishedFilesLock)
            {
                // clear the file list
                m_publishedRepository.Clear();
                // clear the watchers list.
                m_watchers.Clear();
            }
        }

        public bool Unpublish(FileInfo fileInfo)
        {
            lock (m_publishedFilesLock)
            {
                return m_publishedRepository.Unpublish(new FileInformation(fileInfo));
            }
        }

        /// <summary>
        /// Unpublish a directory.  Remove all of the files in this
        /// directory from the list of published files, and stop watching
        /// for changes in the directory.
        /// </summary>
        /// <param name="directory"></param>
        public void UnpublishDirectory(DirectoryInfo directory)
        {
            lock (m_publishedFilesLock)
            {
                var directoryWatcher = m_watchers.FirstOrDefault(
                    watcher => watcher.Path.Equals(directory.FullName.ToLower()));
                if (directoryWatcher == null)
                {
                    return;
                }
                directoryWatcher.Enabled = false;
                m_watchers.Remove(directoryWatcher);

                // Unpublish all files in the unpublished directories
                var filesToUnpublish = m_publishedRepository.PublishedFileInfos.SelectMany(x => x.SourceFiles).Where(
                    info => info.IsInDirectory(directoryWatcher.Path, directoryWatcher.IncludeSubdirectories)).ToList();

                foreach (var item in filesToUnpublish)
                {
                    Unpublish(item.ToFileInfo());
                }
            }
        }

        /// <summary>
        /// Syncronize a published directory with the list of published files.
        /// This should be called any time that a directory has changed, and is
        /// triggered through the FileSystemWatcher mechanism provided with .NET
        /// </summary>
        /// <param name="path">The directory that is being watched.</param>
        private void SyncDirectory(string path, bool recurse)
        {
            path = path.ToLower();
            if (String.IsNullOrEmpty(path))
            {
                return;
            }

            lock (m_publishedFilesLock)
            {
                //first unpublish the files that no longer exist
                var filesInThisDir = m_publishedRepository.PublishedFileInfos.SelectMany(x => x.SourceFiles)
                    .Where(x => x.IsInDirectory(path, recurse)).ToList();

                foreach (var item in filesInThisDir)
                {
                    m_publishedRepository.Invalidate(item);
                }

                DirectoryInfo directory = new DirectoryInfo(path);
                foreach (FileInfo file in directory.GetFiles("*",
                    recurse ? SearchOption.AllDirectories : SearchOption.TopDirectoryOnly))
                {
                    if (filesInThisDir.Any(x => (x.FullName == file.FullName.ToLower()) && !x.IsModified()))
                    {
                        continue;
                    }
                    try
                    {
                        Publish(file);
                    }
                    catch (Exception)
                    {
                        // Sleep again, or correct the logic in this system.
                        try
                        {
                            System.Threading.Thread.Sleep(1000);
                            Publish(file);
                        }
                        catch (Exception ex)
                        {
                            System.Diagnostics.Trace.WriteLine(string.Format(
                                "Error publishing file: {0}", ex.Message));
                        }
                    }
                }
            }
        }
    }
}