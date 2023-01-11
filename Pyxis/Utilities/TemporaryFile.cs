/******************************************************************************
TemporaryFile.cs

begin      : October 21, 2008
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
    /// Manages the lifetime of a "temporary" file.  Removes it on Dispose.
    /// </summary>
    public class TemporaryFile: IDisposable
    {
        private static class TemporaryFileCleaner
        {
            private static object Locker { get; set; }
            private static List<string> FilesToDelete { get; set; }
            private static List<string> CurrentFilesToDelete { get; set; }
            private static bool CleaingUp { get; set; }

            static TemporaryFileCleaner()
            {
                Locker = new object();
                CleaingUp = false;
                FilesToDelete = new List<string>();
                CurrentFilesToDelete = new List<string>();
            }

            public static void DeleteFile(string name)
            {
                if (!File.Exists(name))
                {
                    return;
                }
                {
                    try
                    {
                        // We will try to do this twice.  In many cases, the file may
                        // still be in use (actual locks have not been released.)
                        File.Delete(name);
                    }
                    catch (IOException)
                    {
                        lock (Locker)
                        {
                            FilesToDelete.Add(name);

                            if (!CleaingUp)
                            {
                                CleaingUp = true;
                                new System.Threading.Thread(CleanFiles).Start();
                            }
                        }
                    }

                }
            }

            private static void CleanFiles()
            {
                //delete temp files every minute
                System.Threading.Thread.Sleep(TimeSpan.FromMinutes(1));
                try
                {
                    lock(Locker)
                    {
                        //swap lists...
                        CurrentFilesToDelete = FilesToDelete;
                        FilesToDelete = new List<string>();
                    }

                    foreach(var name in CurrentFilesToDelete)
                    {
                        DeleteFile(name);
                    }
                }
                finally 
                {
                    lock (Locker)
                    {
                        CleaingUp = false;
                    }
                }
            }
        }

        #region Properties
        private readonly string m_name;

        /// <summary>
        /// Gets the name of the file.
        /// </summary>
        /// <value>The name.</value>
        public string Name
        {
            get { return m_name; }
        }
        #endregion Properties

        /// <summary>
        /// Initializes a new instance of the <see cref="TemporaryFile"/> class.
        /// </summary>
        /// <param name="name">The name of the file.</param>
        public TemporaryFile(string name)
        {
            m_name = name;
            Initialize();
            DeleteOnReboot(m_name);
        }

        public TemporaryFile():
            this(Path.Combine(Path.GetTempPath(),Path.GetRandomFileName()))
        {
        }

        /// <summary>
        /// Resets the file, ensuring it exists, and is empty.
        /// </summary>
        public void Reset()
        {
            Delete();
            Initialize();
        }

        /// <summary>
        /// Does nothing.  Any existing contents are
        /// left unchanged.  (Call <see cref="TemporaryFile.Reset"/>
        /// if you want to clear the file as well.)
        /// </summary>
        public void Initialize()
        {
        }

        /// <summary>
        /// Deletes this instance.  Any contents are removed.
        /// </summary>
        public bool Delete()
        {
            if (!File.Exists(this.Name))
            {
                return true;
            }
            //we let the file cleaner do it job (if file is been used - the temporaryFileCleaner will retry to remove this file)
            TemporaryFileCleaner.DeleteFile(this.Name);
            return !File.Exists(this.Name);
        }

        #region DeleteOnReboot
        [System.Runtime.InteropServices.DllImport("kernel32.dll")]
        private static extern bool MoveFileEx(string lpExistingFileName, string lpNewFileName, int dwFlags);
        const int MOVEFILE_DELAY_UNTIL_REBOOT = 0x00000004;

        /// <summary>
        /// Deletes the named file on reboot.
        /// </summary>
        /// <param name="fileName">Name of the file.</param>
        public static void DeleteOnReboot(string fileName)
        {
            MoveFileEx(fileName, null, MOVEFILE_DELAY_UNTIL_REBOOT);
        }
        #endregion DeleteOnReboot

        /// <summary>
        /// Destructor
        /// </summary>
        ~TemporaryFile()
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
                    if (!Delete())
                    {
                        DeleteOnReboot(this.Name);
                    }
                }
            }
            m_disposed = true;
        }

        #endregion /* IDispose */
    }
}
