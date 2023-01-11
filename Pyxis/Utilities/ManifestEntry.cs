using System;
using System.Text;

namespace Pyxis.Utilities
{
    /// <summary>
    /// Holds all of the information for a single file (name, location, checksum).
    /// </summary>
    [Serializable]
    public class ManifestEntry
    {
        private string m_filePath;

        /// <summary>
        /// Gets the name of the file.
        /// </summary>
        /// <value>The name of the file.</value>
        public string FileName
        { get; set; }

        /// <summary>
        /// Gets the file path.
        /// </summary>
        /// <value>The file path.</value>
        public string FilePath
        {
            get
            {
                return m_filePath;
            }

            set
            {
                //--
                //-- path should not be absolute (eg. \bingo ).
                //-- remove starting slash from path to convert it to relative.
                //--
                if (value != null && System.IO.Path.IsPathRooted(value))
                {
                    System.Diagnostics.Trace.WriteLine(
                        string.Format("ManifestEntry assigned rooted path: filename=\"{0}\" path=\"{1}\"",
                        FileName, value));

                    string pathRoot = System.IO.Path.GetPathRoot(value);
                    m_filePath = value.Substring(pathRoot.Length);

                    System.Diagnostics.Trace.WriteLine(
                        string.Format("Removing path root from \"{0}\" => \"{1}\"",
                        value, m_filePath));

                    System.Diagnostics.Trace.Assert(!System.IO.Path.IsPathRooted(m_filePath));
                }
                else
                {
                    m_filePath = value;
                }
            }
        }

        /// <summary>
        /// Gets the file stamp.  This is a string representing an MD5 or SHA256 Hash.
        /// </summary>
        /// <value>The file stamp.</value>
        public string FileStamp
        { get; set; }

        /// <summary>
        /// Return a Uniqe directory name based on the file stamp
        /// </summary>
        public string FileStampAsDirectoryName
        {
            get
            {
                return FileStamp.Replace('/', '_').Replace('\\', '_');
            }
        }

        /// <summary>
        /// Gets the size of the file.
        /// </summary>
        /// <value>The size of the file.</value>
        public long FileSize
        { get; set; }

        /// <summary>
        /// return an identity object that is containing FileStamp and FileSize only
        /// </summary>
        /// <returns>identity object for this entry</returns>
        public Identity GetIdentity()
        {
            return new Identity(this);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ManifestEntry"/> class.
        /// Used for serialization.
        /// </summary>
        public ManifestEntry()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ManifestEntry"/> class.
        /// </summary>
        /// <param name="filename">The filename.</param>
        /// <param name="filepath">The filepath.</param>
        /// <param name="filestamp">The filestamp.</param>
        /// <param name="filesize">The filesize.</param>
        private ManifestEntry(string filename, string filepath, long filesize)
        {
            this.FileName = filename;
            this.FilePath = filepath;
            this.FileSize = filesize;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ManifestEntry"/> class.
        /// </summary>
        /// <param name="filename">The filename.</param>
        /// <param name="filepath">The filepath.</param>
        /// <param name="filestamp">The SHA256 filestamp.</param>
        /// <param name="filesize">The filesize.</param>
        public ManifestEntry(string filename, string filepath, string filestamp, long filesize)
            : this(filename, filepath, filesize)
        {
            this.FileStamp = filestamp;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ManifestEntry"/> class.
        /// </summary>
        /// <param name="file">The file.</param>
        /// <param name="relativeDir">The relative dir.</param>
        public ManifestEntry(System.IO.FileInfo file, string relativeDir) :
            this(file.Name, relativeDir, file.Length)
        {
            this.FileStamp = SHA256Checksummer.GetFileCheckSum(file.FullName);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="ManifestEntry"/> class.
        /// Uses the forceSynchronousCheckSums flag to calculate check sum with non-threaded
        /// solution if necessary.
        /// </summary>
        /// <param name="file">The file.</param>
        /// <param name="relativeDir">The relative dir.</param>
        /// <param name="forceSynchronousCheckSums">if set to <c>true</c> [force synchronous check sums].</param>
        public ManifestEntry(System.IO.FileInfo file, string relativeDir, bool forceSynchronousCheckSums) :
            this(file.Name, relativeDir, file.Length)
        {
            if (forceSynchronousCheckSums)
            {
                this.FileStamp = Pyxis.Utilities.ChecksumSingleton.Checksummer.getFileCheckSum_synch(file.FullName);
            }
            else
            {
                this.FileStamp = SHA256Checksummer.GetFileCheckSum(file.FullName);
            }
        }

        /// <summary>
        /// Gets the relative path (directory plus filename).
        /// </summary>
        /// <value>The relative path.</value>
        public string RelativePath
        {
            get
            {
                if ((this.FilePath != null) && (this.FilePath.Length > 0)
                    && (this.FilePath != "."))
                {
                    if (this.FilePath.EndsWith(System.IO.Path.DirectorySeparatorChar.ToString()) ||
                        this.FilePath.EndsWith(System.IO.Path.AltDirectorySeparatorChar.ToString()))
                    {
                        return this.FilePath + this.FileName;
                    }
                    else
                    {
                        return this.FilePath + System.IO.Path.DirectorySeparatorChar + this.FileName;
                    }
                }
                return this.FileName;
            }
        }

        /// <summary>
        /// Gets the full path for this file, relative to the given base directory.
        /// </summary>
        /// <param name="baseDirectory">The base directory.</param>
        /// <returns></returns>
        private string FullPath(string baseDirectory)
        {
            StringBuilder result = new StringBuilder(baseDirectory);

            //--
            //-- add directory seperator only if necessary
            //--
            if (!baseDirectory.EndsWith(System.IO.Path.DirectorySeparatorChar.ToString()) &&
                !baseDirectory.EndsWith(System.IO.Path.AltDirectorySeparatorChar.ToString()))
            {
                result.Append(System.IO.Path.DirectorySeparatorChar);
            }

            //--
            //-- add relative file path
            //--
            if (this.FilePath.Length > 0)
            {
                result.AppendFormat("{0}{1}", this.FilePath,
                    System.IO.Path.DirectorySeparatorChar);
            }

            //--
            //-- add file name
            //--
            result.Append(this.FileName);

            return result.ToString();
        }

        private static IChecksum SHA256Checksummer = Pyxis.Utilities.ChecksumSingleton.Checksummer;

        /// <summary>
        /// Verifies that the file in the specified base directory matches our checksum.
        /// </summary>
        /// <param name="baseDirectory">The base directory.</param>
        /// <returns></returns>
        public bool Verify(string baseDirectory)
        {
            // Verify that the downloaded file's checksum matches what we expect.
            if (this.FileStamp != null)
            {
                String fileChecksum = SHA256Checksummer.GetFileCheckSum(this.FullPath(baseDirectory));

                return ((fileChecksum != "") && this.FileStamp.Equals(fileChecksum));
            }
            else
            {
                // No checksum specified, so we'll match any file.
                Manifest.Trace.WriteLine("Warning!  No checksum specified for file {0}.", this.FullPath(baseDirectory));
                return System.IO.File.Exists(this.FullPath(baseDirectory));
            }
        }

        #region Equality

        public static bool operator ==(ManifestEntry a, ManifestEntry b)
        {
            return object.Equals(a, b);
        }

        public static bool operator !=(ManifestEntry a, ManifestEntry b)
        {
            return !(a == b);
        }

        // override object.Equals
        public override bool Equals(object obj)
        {
            //
            // See the full list of guidelines at
            //   http://msdn.microsoft.com/library/default.asp?url=/library/en-us/cpgenref/html/cpconequals.asp
            // and also the guidance for operator== at
            //   http://msdn.microsoft.com/library/default.asp?url=/library/en-us/cpgenref/html/cpconimplementingequalsoperator.asp
            //

            if (obj == null || GetType() != obj.GetType())
            {
                return false;
            }

            ManifestEntry other = obj as ManifestEntry;
            return other.FileName.Equals(FileName) &&
                other.FilePath.Equals(FilePath) &&
                other.FileSize.Equals(FileSize) &&
                other.FileStamp.Equals(FileStamp);
        }

        // override object.GetHashCode
        public override int GetHashCode()
        {
            return FileName.GetHashCode() ^ FilePath.GetHashCode() ^
                FileSize.GetHashCode() ^ FileStamp.GetHashCode();
        }

        #endregion Equality

        public class Identity
        {
            public string FileStamp { get; private set; }

            public long FileSize { get; private set; }

            public Identity(ManifestEntry entry)
            {
                FileStamp = entry.FileStamp;
                FileSize = entry.FileSize;
            }

            public static bool operator ==(Identity a, Identity b)
            {
                return object.Equals(a, b);
            }

            public static bool operator !=(Identity a, Identity b)
            {
                return !(a == b);
            }

            public override bool Equals(object obj)
            {
                if (obj == null)
                {
                    return false;
                }

                if (obj is Identity)
                {
                    var other = obj as Identity;
                    return other.FileSize.Equals(FileSize) &&
                           other.FileStamp.Equals(FileStamp);
                }

                return base.Equals(obj);
            }

            public override int GetHashCode()
            {
                return FileSize.GetHashCode() ^ FileStamp.GetHashCode();
            }
        }
    }
}