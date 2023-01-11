/******************************************************************************
ChecksumCalculator.cs

begin      : 14/09/2008 9:40:10 AM
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;
using System.Collections.Generic;
using System.Text;

namespace ApplicationUtility
{
    /// <summary>
    /// This class implements a checksum which is used to expose the .NET checksum capabilites to
    /// the C++ classes. Checksums can be very expensive to calculate can easily take minutes to
    /// complete.  To deal with this, this class has a cache of checksums that have been previously
    /// calculated so that a file will not have to be checksummed twice.  If it has not previously
    /// calculated the checksum, it will return an empty string for the checksum and then start a
    /// background thread to calculate the requested checksum and save the answer in the cache when
    /// the calculation is done.
    /// 
    /// This class also provides a method for reading in previously saved checksums from a file.  If
    /// this mechanism is used, the same file will be updated with all calculted checksums when the
    /// class is destructed.
    /// </summary>
    public class ManagedChecksumCalculator : ChecksumCalculator
    {
        private readonly Pyxis.Utilities.IChecksum m_checksummer;

        /// <summary>
        /// Calculates a checksum on the contents of the string passed in then santitizes 
        /// the checksum by converting to a base 64 string to ensure that only
        /// UTF characters remain in the checksum string.
        /// </summary>
        /// <param name="data">The data that will be checksumed.</param>
        /// <returns>Returns the sanitized SHA 256 checksum, as a base 64 string. </returns>
        public override String calculateCheckSum(String data)
        {
            return m_checksummer.GetCheckSum(data);
        }

        /// <summary>
        /// Gets the checksum for the file path specified in the parameter. If a checksum for the file already
        /// exists then it is immediately returned. If not then a process to calculate and remember the 
        /// checksum is started asynchronously.
        /// </summary>
        /// <param name="file">The file path to calculate a checksum for.</param>
        /// <returns>The checksum of the file.</returns>
        public override String calculateFileCheckSum(String file)
        {
            return m_checksummer.GetFileCheckSum(file);
        }

        /// <summary>
        /// Finds the file matching the given checksum.
        /// </summary>
        /// <param name="checksum">The checksum.</param>
        /// <returns>The matching file's path, or empty string if no match found.</returns>
        public override string findFileMatchingChecksum(string checksum)
        {
            return m_checksummer.FindFileMatchingChecksum(checksum);
        }

        #region Default Checksum Cache Storage Location

        static public string GetDefaultCacheDirectory()
        {
            return Properties.Resources.ChecksumCacheDir;
        }

        static public string GetDefaultCacheFilename()
        {
            return System.IO.Path.DirectorySeparatorChar +
                Properties.Resources.ChecksumCacheFile;
        }
        
        #endregion Default Checksum Cache Storage Location

        #region PYXObject Lifetime Management
        
        public void Trace_WriteLine(string format, params object[] args)
        {
            System.Diagnostics.Trace.WriteLine(String.Format(format, args));
        }

        /// <summary>
        /// We keep a list of objects that are referenced in the unmanaged world.
        /// </summary>
        private static readonly List<ManagedChecksumCalculator> s_unmanagedReferences =
            new List<ManagedChecksumCalculator>();

        #region PYXObject

        /// <summary>
        /// Override the reference-counting addRef.  This is not called 
        /// directly!
        /// </summary>
        /// <returns>Current reference count (after increment).</returns>
        public override int addRef()
        {
            // Increment reference count.
            int referenceCount = base.addRef();

            // If the reference count is now 2, that means that an unmanaged
            // reference now exists.  Create a managed reference to this 
            // object, to ensure that it will survive at least as long 
            // as the unmanaged reference.
            if (referenceCount == 2)
            {
                lock (s_unmanagedReferences)
                {
                    s_unmanagedReferences.Add(this);
                }
            }

            return referenceCount;
        }

        /// <summary>
        /// Override the reference-counting release.  This is not called 
        /// directly!
        /// </summary>
        /// <returns>Current reference count (after decrement).</returns>
        public override int release()
        {
            // If this has not been disposed, decrement the reference count.
            int referenceCount = (getCPtr(this).Handle == IntPtr.Zero) ? 1 : base.release();

            // If the last unmanaged reference has been released.
            // Remove the managed reference to this object,
            // which may lead to the object's garbage collection.
            if (referenceCount < 1)
            {
                lock (s_unmanagedReferences)
                {
                    s_unmanagedReferences.Remove(this);
                }
                System.Diagnostics.Debug.Assert(0 < referenceCount,
                    "The reference count should never get to 0 while there is a managed reference.");
                referenceCount = 1;
            }

            return referenceCount;
        }

        #endregion

        /// <summary>
        /// Constructor.
        /// </summary>
        public ManagedChecksumCalculator(Pyxis.Utilities.IChecksum checksummer)
        {
            m_checksummer = checksummer;

            // Inform SWIG that we don't want them to ever destruct us.
            swigCMemOwn = false;
        }

        #endregion PYXObject Lifetime Management
    }
}
