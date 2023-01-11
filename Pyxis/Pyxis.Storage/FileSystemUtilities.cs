using System;
using System.Collections.Generic;
using System.IO;

namespace Pyxis.Storage
{
    public static class FileSystemUtilities
    {
        /// <summary>
        /// Return the total size of all files in a directory tree.
        /// </summary>
        /// <param name="dir">Directory information to query</param>
        /// <returns>The size of the directory tree contents in bytes</returns>
        static public long GetDirectorySize(string dirPath)
        {
            return GetDirectorySize(new DirectoryInfo(dirPath));
        }

        /// <summary>
        /// Return the total size of all files in a directory tree.
        /// </summary>
        /// <param name="dir">Directory information to query</param>
        /// <returns>The size of the directory tree contents in bytes</returns>
        static public long GetDirectorySize(DirectoryInfo dir)
        {
            long size = 0;
            try
            {
                foreach (FileInfo fileInfo in dir.GetFiles())
                {
                    long fileSize;
                    if (TryGetFileSize(fileInfo.FullName, out fileSize))
                    {
                        size += fileSize;
                    }
                }

                foreach (DirectoryInfo dirInfo in dir.GetDirectories())
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

        // Rename Directory
        public static bool TryRenameDirectory(string dirPath, string newName, bool overwrite)
        {
            var parentDir = Directory.GetParent(dirPath).FullName;
            var newPath = Path.Combine(parentDir, newName);
            try
            {
                if (Directory.Exists(newPath) && overwrite)
                {
                    Directory.Delete(newPath, true);
                }
                Directory.Move(dirPath, newPath);
                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }

        private class Folders
        {
            public string Source { get; private set; }

            public string Target { get; private set; }

            public Folders(string source, string target)
            {
                Source = source;
                Target = target;
            }
        }

        public static bool TryMoveDirectory(string source, string target, bool overwrite)
        {
            var crossDriveMove = Path.GetPathRoot(source) != Path.GetPathRoot(target);
            try
            {
                if (Directory.Exists(target) && overwrite || crossDriveMove)
                {
                    return ForceMoveDirectory(source, target);
                }
                Directory.Move(source, target);
            }
            catch (Exception e)
            {
                System.Diagnostics.Trace.WriteLine("Can not move directory " + source);
                System.Diagnostics.Trace.WriteLine(e.Message);
                System.Diagnostics.Trace.WriteLine(e.StackTrace);
                return false;
            }

            return true;
        }

        private static bool ForceMoveDirectory(string source, string target)
        {
            var stack = new Stack<Folders>();
            stack.Push(new Folders(source, target));

            while (stack.Count > 0)
            {
                var folders = stack.Pop();
                Directory.CreateDirectory(folders.Target);
                foreach (var file in Directory.GetFiles(folders.Source, "*.*"))
                {
                    string targetFile = Path.Combine(folders.Target, Path.GetFileName(file));
                    if (File.Exists(targetFile))
                    {
                        File.Delete(targetFile);
                    }
                    File.Move(file, targetFile);
                }

                foreach (var folder in Directory.GetDirectories(folders.Source))
                {
                    stack.Push(new Folders(folder, Path.Combine(folders.Target, Path.GetFileName(folder))));
                }
            }
            Directory.Delete(source, true);
            return true;
        }

        /// <summary>
        /// Exception safe method for creating a directory.
        /// </summary>
        /// <param name="path">The directory</param>
        /// <param name="overwrite">Whether or not to overwrite and existing directory.</param>
        /// <returns>true if the directory was created or it already exists and overwrite was false, otherwise false</returns>
        internal static bool TryCreateDirectory(string path, bool overwrite)
        {
            try
            {
                if (Directory.Exists(path) && !overwrite)
                {
                    return true;
                }
                if (Directory.Exists(path))
                {
                    Directory.Delete(path);
                }
                Directory.CreateDirectory(path);
                return true;
            }
            catch (Exception e)
            {
                System.Diagnostics.Trace.WriteLine("Can not create directory " + path);
                System.Diagnostics.Trace.WriteLine(e.Message);
                System.Diagnostics.Trace.WriteLine(e.StackTrace);
                return false;
            }
        }

        public static bool TryDeleteDirectory(string path)
        {
            try
            {
                Directory.Delete(path, true);
                return true;
            }
            catch (Exception e)
            {
                System.Diagnostics.Trace.WriteLine("Can not delete directory " + path);
                System.Diagnostics.Trace.WriteLine(e.Message);
                System.Diagnostics.Trace.WriteLine(e.StackTrace);
                return false;
            }
        }

        public static bool Retry(Func<bool> action, int numberOfTrys, int milisecondsPause)
        {
            for (int i = 0; i < numberOfTrys; i++)
            {
                if (action())
                {
                    return true;
                }
                System.Threading.Thread.Sleep(milisecondsPause * i);
            }
            return false;
        }
    }
}