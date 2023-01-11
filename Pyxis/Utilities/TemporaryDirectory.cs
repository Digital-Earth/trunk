/******************************************************************************
TemporaryDirectory.cs

begin      : May 23, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace Pyxis.Utilities
{
    /// <summary>
    /// Manages the lifetime of a "temporary" directory.  Removes it on Dispose.
    /// </summary>
    public class TemporaryDirectory: IDisposable
    {
        #region Properties
        private readonly string m_name;

        /// <summary>
        /// Gets the name of the directory (does not include a trailing slash).
        /// </summary>
        /// <value>The name.</value>
        public string Name
        {
            get { return m_name; }
        }
        #endregion Properties

        /// <summary>
        /// Initializes a new instance of the <see cref="TemporaryDirectory"/> class.
        /// </summary>
        /// <param name="name">The name of the directory.</param>
        public TemporaryDirectory(string name)
        {
            m_name = name;
            Initialize();
            TemporaryFile.DeleteOnReboot(m_name);
        }

        /// <summary>
        /// Resets the directory, ensuring it exists, and is empty.
        /// </summary>
        public void Reset()
        {
            Delete();
            Initialize();
        }

        /// <summary>
        /// Ensures that the directory exists.  Any existing contents are
        /// left unchanged.  (Call <see cref="TemporaryDirectory.Reset"/>
        /// if you want to clear the directory as well.)
        /// </summary>
        public void Initialize()
        {
            if (!System.IO.Directory.Exists(this.Name))
            {
                System.IO.Directory.CreateDirectory(this.Name);
            }
        }

        /// <summary>
        /// Deletes this instance, removing any contents. Throws an exception on failure.
        /// </summary>
        /// <remarks>
        /// An alternative (static) routine TemporaryDirectory.Delete does not throw exceptions.
        /// </remarks>
        public void Delete()
        {
            DeleteDirectory(this.Name);
        }

        /// <summary>
        /// Deletes the specified directory name.  Returns true if the directory no longer exists.  
        /// </summary>
        /// <param name="directoryName">Name of the directory.</param>
        /// <returns></returns>
        public static bool Delete(string directoryName)
        {
            try
            {
                return DeleteDirectory(directoryName);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Trace.WriteLine(String.Format(
                    "Exception deleting directory {0}: {1}",
                    directoryName, ex.ToString()));
            }
            return false;                
        }

        /// <summary>
        /// Deletes the directory.  (Internal/utility function to do the hard work.  May throw.)
        /// </summary>
        /// <param name="directoryName">Name of the directory.</param>
        /// <returns></returns>
        private static bool DeleteDirectory(string directoryName)
        {
            if (System.IO.Directory.Exists(directoryName))
            {
                try
                { 
                    // We will try to do this twice.  In many cases, the file may
                    // still be in use (actual locks have not been released.)
                    System.IO.Directory.Delete(directoryName, true);
                    return true;
                }
                catch (System.IO.IOException)
                {
                }
                System.Threading.Thread.Sleep(500);
                System.IO.Directory.Delete(directoryName, true);
            }
            return true;
        }

        /// <summary>
        /// Deletes the named directory on reboot.
        /// </summary>
        /// <param name="fileName">Name of the directory.</param>
        public static void DeleteOnReboot(string directoryName)
        {
            // Actually, the same operation works on files or directories.  Shhhh!
            TemporaryFile.DeleteOnReboot(directoryName);
        }

        /// <summary>
        /// Recursively deletes the named directory on reboot.  
        /// Necessary because DeleteOnReboot cannot delete non-empty directories directly.
        /// </summary>
        /// <param name="fileName">Name of the directory.</param>
        public static void DeleteDirectoryOnReboot(string directoryName)
        {
            string[] files = Directory.GetFiles(directoryName);
            string[] directories = Directory.GetDirectories(directoryName);

            foreach (string file in files)
            {
                File.SetAttributes(file, FileAttributes.Normal);
                TemporaryFile.DeleteOnReboot(file);
            }

            foreach (string directory in directories)
            {
                DeleteDirectoryOnReboot(directory);
            }

            TemporaryFile.DeleteOnReboot(directoryName);
        }

        /// <summary>
        /// Destructor
        /// </summary>
        ~TemporaryDirectory()
        {
            Dispose(false);
        }

        #region IDisposable
        private bool m_disposed = false;

        /// <summary>
        /// Dispose of this object (as per IDisposable)
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Implementation of Dispose - will be called from Dispose or destructor.
        /// </summary>
        /// <remarks>Do NOT touch member variables if disposing is false!</remarks>
        /// <param name="disposing"></param>
        private void Dispose(bool disposing)
        {
            if (!this.m_disposed)
            {
                if (disposing)
                {
                    Delete();
                }
            }
            m_disposed = true;
        }

        #endregion /* IDispose */
    }
}
