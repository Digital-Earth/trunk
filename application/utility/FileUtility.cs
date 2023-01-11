/******************************************************************************
FileUtility.cs

begin      : 28/09/2007 3:25:17 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;

namespace ApplicationUtility
{
    /// <summary>
    /// Static functions to help with file operations.
    /// </summary>
    public class FileUtility
    {
        private static string[] s_postFix = { "B", "KB", "MB", "GB", "TB", "PB" };

        /// <summary>
        /// Create a single directory of files. Usually used in testing. All files
        /// are named 'File Number n.test' where n has a range of 0 to count - 1.
        /// If the root directory does not exist when the method is called, it is
        /// created.
        /// </summary>
        /// <param name="dirPath">The path to place the file in.</param>
        /// <param name="count">The number of files to create</param>
        /// <param name="size">The size in bytes to make each file</param>
        public static void CreateFlatDirectoryOfFiles(string dirPath, int count, int size)
        {
            // verify that the passed directory exists
            if (!System.IO.Directory.Exists(dirPath))
            {
                System.IO.Directory.CreateDirectory(dirPath);
            }

            int createdFileCount = 0;
            while (createdFileCount < count)
            {
                System.IO.FileInfo fileInfo = new System.IO.FileInfo(
                    dirPath + "\\" + string.Format("File number {0}.test", createdFileCount));
                if (fileInfo.Exists)
                {
                    throw new System.IO.IOException(
                        string.Format("File already exists, can't create another {0} file", fileInfo.FullName));
                }
                System.IO.FileStream stream = fileInfo.Create();
                stream.SetLength(size);
                stream.Close();

                ++createdFileCount;
            }
        }

        /// <summary>
        /// Return the total size of all files in a directory tree.
        /// </summary>
        /// <param name="dir">Directory information to query</param>
        /// <returns>The size of the directory tree contents in bytes</returns>
        static public long GetDirectorySize(System.IO.DirectoryInfo dir)
        {
            long size = 0;
            try
            {
                foreach (System.IO.FileInfo fileInfo in dir.GetFiles())
                {
                    size += fileInfo.Length;
                }

                foreach (System.IO.DirectoryInfo dirInfo in dir.GetDirectories())
                {
                    size += GetDirectorySize(dirInfo);
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Trace.WriteLine(string.Format(
                    "Ignoring unexpected failure in getDirectorySize( {0}): {1}",
                    dir.FullName, ex.ToString()));
            }

            return size;
        }

        /// <summary>
        /// Remove all of the least recently accessed files in the directory
        /// structure until it is smaller than a specified size.
        /// </summary>
        /// <param name="dirInfo">
        /// The directory object to act upon
        /// </param>
        /// <param name="sizeToRemove">
        /// The number of bytes to remove from the directory.
        /// </param>
        /// <param name="bw">
        /// The background worker thread that is running the method. This allows
        /// the operation to be cancelled upon request. This value is allowed to
        /// be null.
        /// </param>
        /// <returns>The number of bytes that were removed.</returns>
        static public long RemoveLeastRecentlyWrittenFiles(
            System.IO.DirectoryInfo dirInfo,
            long sizeToRemove,
            BackgroundWorker bw)
        {
            long sizeRemoved = 0;
            System.Diagnostics.Debug.Assert(0 < sizeToRemove);

            // the path must be a directory here
            if (!dirInfo.Exists)
            {
                throw new System.IO.DirectoryNotFoundException(
                    "Could not find directory '" + dirInfo.FullName + "' during removal operation");
            }

            if (bw != null && bw.CancellationPending)
            {
                return 0;
            }

            // sort all directory elements according to their last accessed time
            SortedDictionary<DateTime, List<string>> objectsByWriteTime =
                new SortedDictionary<DateTime, List<string>>();
            foreach (System.IO.FileInfo fileInfo in dirInfo.GetFiles())
            {
                AddValue(objectsByWriteTime, fileInfo.LastWriteTime, fileInfo.FullName);
            }
            foreach (System.IO.DirectoryInfo subdirInfo in dirInfo.GetDirectories())
            {
                AddValue(objectsByWriteTime, subdirInfo.LastWriteTime, subdirInfo.FullName);
            }

            // iterate over each item in order of least recently accessed
            foreach (KeyValuePair<System.DateTime, List<string>> keyVal in objectsByWriteTime)
            {
                List<string> stringList = keyVal.Value;
                foreach (string pathString in stringList)
                {
                    // check to see if enough has already been removed
                    if (sizeToRemove < sizeRemoved)
                    {
                        return sizeRemoved;
                    }

                    if (System.IO.File.Exists(pathString))
                    {
                        // process the file
                        System.IO.FileInfo deleteFile =
                            new System.IO.FileInfo(pathString);
                        System.Diagnostics.Debug.Assert(deleteFile.Exists);
                        sizeRemoved += deleteFile.Length;
                        System.IO.File.Delete(pathString);
                    }
                    else if (System.IO.Directory.Exists(pathString))
                    {
                        // process the directory
                        System.IO.DirectoryInfo deleteDirInfo =
                            new System.IO.DirectoryInfo(pathString);
                        System.Diagnostics.Debug.Assert(deleteDirInfo.Exists);

                        long dirSize = FileUtility.GetDirectorySize(deleteDirInfo);
                        if (dirSize < sizeToRemove - sizeRemoved)
                        {
                            // the whole directory (recusive) can go
                            System.IO.Directory.Delete(pathString, true);
                        }
                        else
                        {
                            // the directory is larger than what we need to delete
                            dirSize = RemoveLeastRecentlyWrittenFiles(
                                deleteDirInfo, sizeToRemove - sizeRemoved, bw);
                        }
                        sizeRemoved += dirSize;
                    }

                    // check to see if the operation should be aborted
                    if (bw != null && bw.CancellationPending)
                    {
                        break;
                    }
                }
            }

            return sizeRemoved;
        }

        public static string SizeToString(long size)
        {
            int i = 0;
            while (size > 1024 && i < 5)
            {
                i++;
                size /= 1024;
            }
            return size + s_postFix[i];
        }

        public static bool TryDeleteFile(string path)
        {
            try
            {
                File.Delete(path);
                return true;
            }
            catch (Exception e)
            {
                Console.WriteLine("Can not delete file: " + path + " " + e.Message);
            }
            return false;
        }

        public static bool TryDeleteDirectory(string path)
        {
            try
            {
                Directory.Delete(path);
                return true;
            }
            catch (Exception e)
            {
                Console.WriteLine("Can not delete Directory: " + path + " " + e.Message);
            }
            return false;
        }

        public static long DeleteAccessibleItems(string path)
        {
            long deletedSize = 0;
            foreach (var file in GetAccessableFiles(path))
            {
                long size = 0;
                TryGetFileSize(file, out size);
                if (TryDeleteFile(file))
                {
                    deletedSize += size;
                }
            }
            foreach (var directory in GetAccessableDirectories(path))
            {
                long size = TryGetDirectorySize(directory);
                if (TryDeleteDirectory(directory))
                {
                    deletedSize += size;
                }
            }

            return deletedSize;
        }

        public static long TryGetDirectorySize(string path)
        {
            long total = 0;
            foreach (string name in GetAccessableFiles(path))
            {
                long size = 0;
                TryGetFileSize(name, out size);
                total += size;
            }
            return total;
        }

        public static IEnumerable<string> GetAccessableFiles(string path)
        {
            List<string> files = new List<string>();
            try
            {
                foreach (string file in Directory.GetFiles(path))
                {
                    files.Add(file);
                }

                foreach (string d in Directory.GetDirectories(path))
                {
                    files.AddRange(GetAccessableFiles(d));
                }
            }
            catch (System.Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
            return files;
        }

        public static IEnumerable<string> GetAccessableDirectories(string path)
        {
            List<string> directories = new List<string>();
            try
            {
                foreach (string file in Directory.GetDirectories(path))
                {
                    directories.Add(file);
                }

                foreach (string d in Directory.GetDirectories(path))
                {
                    directories.AddRange(GetAccessableDirectories(d));
                }
            }
            catch (System.Exception ex)
            {
                Console.WriteLine(ex.Message);
            }
            return directories;
        }

        public static bool TryGetFileSize(string path, out long size)
        {
            size = 0;
            try
            {
                var fileInfo = new FileInfo(path);
                size = fileInfo.Length;

                //Check if the file is a symbolic link
                if (size == 0)
                {
                    using (var fileStream = File.Open(fileInfo.FullName, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
                    {
                        size = fileStream.Length;
                    }
                }
            }
            catch (Exception e)
            {
                System.Diagnostics.Trace.WriteLine("Can not get the file size for : " + path + " " + e.Message);
                return false;
            }
            return true;
        }

        /// <summary>
        /// Add a string value and time stamp to a data structure that acts
        /// like a std::multimap.
        /// </summary>
        /// <param name="objectsByAccessTime">
        /// The data structure to add to.
        /// </param>
        /// <param name="timeStamp">
        /// The key value to sort by.
        /// </param>
        /// <param name="value">
        /// The value to insert.
        /// </param>
        private static void AddValue(
            SortedDictionary<DateTime, List<string>> objectsByAccessTime,
            DateTime timeStamp,
            string value)
        {
            if (!objectsByAccessTime.ContainsKey(timeStamp))
            {
                objectsByAccessTime.Add(timeStamp, new List<string>());
            }
            objectsByAccessTime[timeStamp].Add(value);
        }
    }
}