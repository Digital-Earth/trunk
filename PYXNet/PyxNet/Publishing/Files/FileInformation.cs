using System;
using System.IO;

namespace PyxNet.Publishing.Files
{
    public class FileInformation
    {
        public string DirectoryName { get; private set; }

        public string FullName { get; private set; }

        public DateTime LastModified { get; private set; }

        public long Length { get; private set; }

        public string Name { get; private set; }

        public FileInformation(FileInfo fileInfo)
        {
            fileInfo.Refresh();
            Name = fileInfo.Name.ToLower();
            FullName = fileInfo.FullName.ToLower();
            LastModified = fileInfo.LastWriteTime;
            DirectoryName = fileInfo.DirectoryName.ToLower();

            Length = fileInfo.Length;
            if (Length == 0)
            {
                try
                {
                    using (var fileStream = File.Open(fileInfo.FullName, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
                    {
                        Length = fileStream.Length;
                    }
                }
                catch (Exception)
                {
                }
            }
        }

        public FileInformation(string path)
            : this(new FileInfo(path))
        {
        }

        public static bool operator !=(FileInformation a, FileInformation b)
        {
            return !(a == b);
        }

        public static bool operator ==(FileInformation a, FileInformation b)
        {
            // If both are null, or both are same instance, return true.
            if (System.Object.ReferenceEquals(a, b))
            {
                return true;
            }

            // If one is null, but not both, return false.
            if (((object)a == null) || ((object)b == null))
            {
                return false;
            }

            // Return true if they are equal.
            return a.Equals(b);
        }

        public override bool Equals(object obj)
        {
            if (obj is FileInformation)
            {
                return FullName == ((FileInformation)obj).FullName;
            }
            return false;
        }

        public override int GetHashCode()
        {
            return FullName.GetHashCode();
        }

        public bool IsModified()
        {
            var fileInfo = new FileInformation(FullName);
            return (fileInfo.Length != Length) || (LastModified != fileInfo.LastModified);
        }

        public FileInfo ToFileInfo()
        {
            return new FileInfo(FullName);
        }

        internal bool IsInDirectory(string path, bool recurse)
        {
            path = path.ToLower();
            return ((DirectoryName == path) ||
                (recurse &&
                DirectoryName.StartsWith(path + Path.DirectorySeparatorChar)));
        }
    }
}