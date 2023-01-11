/******************************************************************************
ChecksumCache.cs

begin		: October 15, 2009
copyright	: (C) 2009 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.Utilities
{
    /// <summary>
    /// This class calculates checksums of files. Checksums can be very expensive to calculate can 
    /// easily take minutes to complete.  To deal with this, this class has a cache of checksums 
    /// that have been previously calculated so that a file will not have to be checksummed twice.  
    /// If it has not previously calculated the checksum, it will return an empty string for the 
    /// checksum and then start a background thread to calculate the requested checksum and save 
    /// the answer in the cache when the calculation is done.
    /// 
    /// This class also provides a method for reading in previously saved checksums from a file.  If
    /// this mechanism is used, the same file will be updated with all calculated checksums when the
    /// write cache method is called.
    /// 
    /// This class can also checksum a string, but no caching is done for this operation.
    /// </summary>
    public class ChecksumCache : IChecksum 
    {
        /// <summary>
        /// A bunch of information about a file, including checksum.
        /// </summary>
        [Serializable]
        public class FileChecksum
        {
            // The name of the file
            public string Filename
            {   get; set;}

            // The checksum for this file.  May be empty if the calculation is pending.
            public string ChecksumSHA256
            { get; set; }

            // The size of the file in bytes
            public long Filesize
            { get; set; }

            // When the file was last written to.
            public DateTime LastModified
            { get; set; }

            /// <summary>
            /// Gets a value indicating whether this checksum has expired (IE: the file
            /// has changed size or timestamp.)
            /// </summary>
            /// <value><c>true</c> if this checksum has expired; otherwise, <c>false</c>.</value>
            public bool HasChecksumExpired
            {
                get
                {
                    System.IO.FileInfo info = new System.IO.FileInfo(this.Filename);
                    return (this.LastModified != info.LastWriteTime) ||
                        (this.Filesize != info.Length);
                }
            }

            // Default constructor for deserialization.
            public FileChecksum()
            {
            }

            /// <summary>
            /// Constructor
            ///
            /// Throws an ArgumentException if the file name does not refer to
            /// a valid file.
            /// </summary>
            public FileChecksum(string filename)
            {
                this.Filename = filename;
                this.ChecksumSHA256 = "";
                UpdateFileInfo();
            }

            public FileChecksum(FileInfo info)
            {
                this.Filename = info.FullName;
                this.ChecksumSHA256 = "";
                UpdateFileInfo(info);
            }

            /// <summary>
            /// Sets the modified date and the file length.
            /// May throw an ArgumentException if the file name does not refer to
            /// a valid file.
            /// </summary>
            public void UpdateFileInfo()
            {
                try
                {
                    System.IO.FileInfo info =  new System.IO.FileInfo(this.Filename);
                    UpdateFileInfo(info);
                }
                catch (Exception)
                {
                    throw (new ArgumentException("Bad file name" + this.Filename));
                }
            }

            private void UpdateFileInfo(System.IO.FileInfo info)
            {
                this.LastModified = info.LastWriteTime;
                this.Filesize = info.Length;
            }
        }

        /// <summary>
        /// Data structure to map the file paths, to their checksum in memory.
        /// </summary>
        /// <remarks>
        /// TODO: Switch to ThreadSafeDictionary (EG: April 9, 2010)
        /// </remarks>
        private Dictionary<String, FileChecksum> m_checksumCache = new Dictionary<string, FileChecksum>();

        /// <summary>
        /// Keep information on the cacheFileInfo when it was last been modified.
        /// </summary>
        private FileChecksum m_cacheFileInfo;

        /// <summary>
        /// We are using Tasks to generate a checksum calculation Queue.
        /// 
        /// Calculating checksum is a heavy operation both in terms of CPU and IO.
        /// Especially if we calcualte checksum for big files.
        /// In order to control the resource usage, we perform all checksum
        /// operations in sequence using this Task queue.
        /// </summary>
        private Task<string> m_calculatingCheckSumQueue = Task.FromResult<string>("");

        /// <summary>
        /// The location on the internal disk where the cache is to be kept and loaded from when 
        /// serializing and deserializing the cache.
        /// </summary>
        private String m_cacheFileName = "";

        private Object m_cacheDirLockObject = new object();

        /// <summary>
        /// Lock object to ensure thread synchronization.
        /// </summary>
        private object m_dictionaryLock = new object();

        /// <summary>
        /// Calculates a SHA 256 checksum on the file passed in then santitizes the checksum by converting to a 
        /// base 64 string to ensure that only UTF characters remain in the checksum string.
        /// </summary>
        /// <param name="data">The name of the file that will be checksumed.</param>
        /// <returns>Returns the sanitized SHA 256 checksum, as a base 64 string. </returns>
        public String CalculateFileCheckSumNoCache(String filename)
        {
            byte[] checksum = null;
            try
            {
                var sha256ManagedChecksum = new System.Security.Cryptography.SHA256Managed();

                // Create a file stream for the given file.
                using(var fileStream = new FileStream(filename, FileMode.Open, FileAccess.Read , FileShare.ReadWrite))
                {
                    checksum = sha256ManagedChecksum.ComputeHash(fileStream);
                    return Convert.ToBase64String(checksum);
                }
            }
            catch (Exception)
            {
                return "ERROR";
            }
        }

        /// <summary>
        /// Writes the checksum cache to the disk for persistent storage.
        /// Writes to the file that was passed to the read function.
        /// </summary>
        public void WriteChecksumCache()
        {
            lock (m_cacheDirLockObject)
            {
                // TODO: do the magic to make XML serialization precomputed and fast.

                // Only try and write it out if we have read it in.
                if (m_cacheFileName.Length > 0)
                {
                    ReloadCacheFileIfNeeded();

                    var temporaryFileName = m_cacheFileName + "." + System.Diagnostics.Process.GetCurrentProcess().Id + ".tmp";

                    try
                    {
                        var readList = new List<FileChecksum>();
                        // Create a list of the checksums for serializing.  Yes -- Dictionaries don't serialize. Sigh.
                        lock (m_dictionaryLock)
                        {
                            foreach (FileChecksum checksum in m_checksumCache.Values)
                            {
                                if (checksum != null && checksum.ChecksumSHA256.Length != 0)
                                {
                                    readList.Add(checksum);
                                }
                            }
                        }
                        File.WriteAllText(temporaryFileName, XmlTool.ToXml(readList));

                        if (File.Exists(m_cacheFileName))
                        {
                            File.Delete(m_cacheFileName);
                        }

                        File.Move(temporaryFileName, m_cacheFileName);

                    }
                    catch (Exception)
                    {
                        // There was an error writing the file.
                        return;
                    }
                    finally
                    {
                        if (File.Exists(temporaryFileName))
                        {
                            File.Delete(temporaryFileName);
                        }
                    }
                }
            }
        }

        private List<FileChecksum> ParseChecksumFile(string cacheFileName)
        {
            StreamReader cacheReader = null;

            //if file doesn't exists - wait one second (in case it has just been moved)
            if (!File.Exists(cacheFileName))
            {
                System.Threading.Thread.Sleep(TimeSpan.FromSeconds(1));
            }

            try
            {
                cacheReader = new StreamReader(cacheFileName);

                try
                {
                    var readList = XmlTool.FromXml<List<FileChecksum>>(cacheReader);
                    return readList;
                }
                finally
                {
                    cacheReader.Close();
                }
            }
            catch(Exception)
            {
                return null;
            }
        }

        public void ReloadCacheFileIfNeeded()
        {
            lock (m_cacheDirLockObject)
            {
                if (String.IsNullOrEmpty(m_cacheFileName))
                {
                    return;
                }

                if (!File.Exists(m_cacheFileName))
                {
                    System.Threading.Thread.Sleep(TimeSpan.FromSeconds(1));
                }

                if (!File.Exists(m_cacheFileName))
                {
                    return;
                }

                var updatedFileInfo = new FileInfo(m_cacheFileName);

                if (updatedFileInfo.Length == m_cacheFileInfo.Filesize && updatedFileInfo.LastWriteTime == m_cacheFileInfo.LastModified)
                {
                    return;
                }
                var readList = ParseChecksumFile(m_cacheFileName);
                if (readList == null)
                {
                    return;
                }
                m_cacheFileInfo = new FileChecksum(updatedFileInfo);

                lock (m_dictionaryLock)
                {
                    foreach (FileChecksum checksum in readList)
                    {
                        if (checksum.ChecksumSHA256.Length != 0)
                        {
                            FileChecksum cacheEntry;
                            if (m_checksumCache.TryGetValue(checksum.Filename, out cacheEntry))
                            {
                                //file entry is newer
                                if (cacheEntry.LastModified < checksum.LastModified)
                                {
                                    m_checksumCache.Add(checksum.Filename, checksum);
                                }
                            }
                            else
                            {
                                //add entry if still valid
                                if (!checksum.HasChecksumExpired)
                                {
                                    m_checksumCache.Add(checksum.Filename, checksum);
                                }
                            }
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Loads the cache of files and their checksums from persistent storage on the disk back into memory.
        /// The file that is read will be updated automatically when this class is destroyed.
        /// </summary>
        public void ReadChecksumCache(string cacheFileName)
        {
            lock (m_cacheDirLockObject)
            {
                // remember where the cache is stored because we will save to that file when we are destroyed.
                m_cacheFileName = cacheFileName;

                var readList = ParseChecksumFile(m_cacheFileName);

                if (File.Exists(m_cacheFileName))
                {
                    m_cacheFileInfo = new FileChecksum(cacheFileName);
                }

                if (readList == null)
                {
                    return;
                }

                lock (m_dictionaryLock)
                {
                    m_checksumCache = new Dictionary<string, FileChecksum>();
                    foreach (FileChecksum checksum in readList)
                    {
                        if (checksum.ChecksumSHA256.Length != 0)
                        {
                            m_checksumCache.Add(checksum.Filename, checksum);
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Calculate the checksum on the file represented by the file path. Once the checksum
        /// is calculated, the checksum and the file are stored in the cache. If an error or exception is encountered 
        /// or the calculation of the checksum fails then nothing is stored in the cache and this calculation will
        /// be tried again the next time the checksum is requested.
        /// </summary>
        /// <returns>Calculated checksum string.</returns>
        /// <param name="file">The path to the file on the local disk of the file to calculate the checksum on.</param>
        private string CalculateFileChecksumAndCache(object file)
        {
            String filePath = file as String;
            try
            {
                //Note: we create the file checksum before we calculate the checksum.
                //Therefore, if the file was update wile we calculated the checksum, it will be invalidated next time we calcualte the checksum
                FileChecksum checksum = new FileChecksum(filePath);
                String strChecksum = CalculateFileCheckSumNoCache(filePath);

                if (!strChecksum.Equals("ERROR"))
                {
                    lock (m_dictionaryLock)
                    {
                        //overwrite the old checksum cache
                        checksum.ChecksumSHA256 = strChecksum;
                        m_checksumCache[filePath] = checksum;
                    }

                    return strChecksum;
                }
            }
            catch (Exception)
            {
                // Eat any exceptions with calculating the checksum here.
                // But we need to remove the entry we were checksumming because it will need to be done again.
                lock (m_dictionaryLock)
                {
                    m_checksumCache.Remove(filePath);
                }
            }

            return "";
        }

        #region IChecksum Members

        /// <summary>
        /// Calculates a SHA 256 checksum on the contents of the string passed in then santitizes 
        /// the checksum by converting to a base 64 string to ensure that only
        /// UTF characters remain in the checksum string.
        /// </summary>
        /// <param name="data">The data that will be checksumed.</param>
        /// <returns>Returns the sanitized SHA 256 checksum, as a base 64 string. 
        /// Returns the string "ERROR" if the cryptography functions throw an exception.</returns>
        public string GetCheckSum(string data)
        {
            byte[] checksum = null;
            try
            {
                System.Security.Cryptography.SHA256Managed sha256ManagedChecksum = new System.Security.Cryptography.SHA256Managed();
                checksum = sha256ManagedChecksum.ComputeHash(ASCIIEncoding.Default.GetBytes(data));
            }
            catch (System.Exception)
            {
                return "ERROR";
            }
            return Convert.ToBase64String(checksum);
        }

        /// <summary>
        /// Gets the checksum for the file path specified in the parameter. If a checksum for the file already
        /// exists then it is immediately returned. If not then the process to calculte and remember  the 
        /// checksum is started asynchronously.
        /// </summary>
        /// <param name="file">The file path to calculate a checksum for.</param>
        /// <returns>The checksum of the file.</returns>
        public string GetFileCheckSum(string file)
        {
            return GetFileCheckSum(file, true);
        }

        /// <summary>
        /// Gets the checksum for the file path specified in the parameter. If a checksum for the file already
        /// exists then it is immediately returned. If not then the process to calculte and remember  the 
        /// checksum is started asynchronously.
        /// </summary>
        /// <param name="file">The file path to calculate a checksum for.</param>
        /// <param name="isThreaded">True if uncached checksums should be calculated in a background thread.</param>
        /// <returns>The checksum of the file.</returns>
        private string GetFileCheckSum(string file, bool isThreaded)
        {
            // information about the file we want to checksum.
            FileChecksum newChecksum;
            try
            {
                newChecksum = new FileChecksum(file);
            }
            catch (ArgumentException)
            {
                // the file does not exist, so we can't do a checksum.
                return "";
            }

            // the information we have precalculated about this file.
            FileChecksum storedChecksum;

            String strReturnChecksum = "";

            bool needToCompute = false;
            lock (m_dictionaryLock)
            {
                // If it is not in the list, then we need to add it and start a computation.
                if (!m_checksumCache.TryGetValue(file, out storedChecksum))
                {
                    //try reload checksum if - maybe someone else update it for us.
                    ReloadCacheFileIfNeeded();
                }

                // If it is not in the list, then we need to add it and start a computation.
                if (!m_checksumCache.TryGetValue(file, out storedChecksum))
                {
                    m_checksumCache.Add(file, newChecksum);
                    needToCompute = true;
                }
                else
                {
                    // If it has changed since the last time we calculated a checksum, then blank out the checksum
                    // and start a new calculation.
                    if (newChecksum.LastModified != storedChecksum.LastModified || newChecksum.Filesize != storedChecksum.Filesize)
                    {
                        // If we have a modified file, and it has a blank checksum, then we still have a thread
                        // calculating that checksum.  We will wait until that thread is done and then start
                        // a new thread to calculate the the modified checksum by only starting a thread to
                        // calculate the checksum if the checksum is already finished being calculated.
                        if (storedChecksum.ChecksumSHA256 != null && storedChecksum.ChecksumSHA256.Length > 0)
                        {
                            storedChecksum.LastModified = newChecksum.LastModified;
                            storedChecksum.Filesize = newChecksum.Filesize;
                            storedChecksum.ChecksumSHA256 = "";
                            needToCompute = true;
                        }
                    }
                    // Return the cached checksum.  This may still be an empty string if the
                    // calculating thread isn't done yet.
                    strReturnChecksum = storedChecksum.ChecksumSHA256;
                }
            }

            // Start a background thread calculating the checksum.
            if (needToCompute)
            {
                if (isThreaded)
                {
                    //adding the next task under the queue of tasks.
                    m_calculatingCheckSumQueue = m_calculatingCheckSumQueue.ContinueWith(task =>
                    {
                        var checksum = CalculateFileChecksumAndCache(file);
                        System.Diagnostics.Trace.WriteLine(string.Format("{0} checksum is {1}", file, checksum));
                        return checksum;
                    });
                    return "";
                }

                // we are not threaded so calculate the checksum using this thread.
                CalculateFileChecksumAndCache(file);
                if (m_checksumCache.TryGetValue(file, out storedChecksum))
                {
                    strReturnChecksum = storedChecksum.ChecksumSHA256;
                }
            }

            return strReturnChecksum;
        }

        /// <summary>
        /// Gets the file check sum (synchronously).
        /// </summary>
        /// <param name="file">The file.</param>
        /// <returns></returns>
        public string getFileCheckSum_synch(string file)
        {
            return GetFileCheckSum(file, false);
        }

        /// <summary>
        /// Return the checksum for the file from the cache or an empty string if no checksum was found
        /// </summary>
        /// <param name="file"></param>
        /// <returns></returns>
        public string getFileCheckSumFromCache(string file)
        {
            // information about the file we want to get checksum.
            FileChecksum fileInfo;
            try
            {
                fileInfo = new FileChecksum(file);
            }
            catch (ArgumentException)
            {
                // the file does not exist, so we can't do a checksum.
                return "";
            }

            // the information we have precalculated about this file.
            FileChecksum storedChecksum;

            lock (m_dictionaryLock)
            {
                // If it is not in the list, then we need to add it and start a computation.
                if (m_checksumCache.TryGetValue(file, out storedChecksum))
                {
                    if (fileInfo.LastModified == storedChecksum.LastModified && fileInfo.Filesize == storedChecksum.Filesize)
                    {
                        return storedChecksum.ChecksumSHA256;
                    }
                }
            }
            return "";
        }

        public string getLocallyResolvedFilename(string serializedManifest, int fileIndex)
        {
            if (serializedManifest.Length > 0)
            {
                Pyxis.Utilities.Manifest manifest = 
                    Pyxis.Utilities.Manifest.ReadFromString(serializedManifest);
                if (manifest.Entries.Count > fileIndex)
                {
                    string filePath = FindFileMatchingChecksum(
                        manifest.Entries[fileIndex].FileStamp);
                    return filePath;
                }
            }
            return "";
        }

        /// <summary>
        /// Finds the file matching the given checksum.
        /// </summary>
        /// <param name="checksum">The checksum.</param>
        /// <returns>The matching file's path, or empty string if no match found.</returns>
        public string FindFileMatchingChecksum(string checksum)
        {
            lock (m_dictionaryLock)
            {
                foreach (var item in m_checksumCache)
                {
                    if (item.Value.ChecksumSHA256 == checksum)
                    {
                        // TODO: Purge invalid timestamps...
                        if (!item.Value.HasChecksumExpired)
                        {
                            return item.Value.Filename;
                        }
                    }
                }
            }

            // Didn't find it...
            return "";
        }

        #endregion
    }
}
