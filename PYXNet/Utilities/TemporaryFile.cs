/******************************************************************************
TemporaryFile.cs

begin      : October 21, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Utilities
{
    /// <summary>
    /// Manages the lifetime of a "temporary" file.  Removes it on Dispose.
    /// </summary>
    public class TemporaryFile: IDisposable
    {
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
            this(System.IO.Path.Combine(System.IO.Path.GetTempPath(),
                    System.IO.Path.GetRandomFileName()))
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
        public void Delete()
        {
            if (System.IO.File.Exists(this.Name))
            {                
                try
                {
                    // We will try to do this twice.  In many cases, the file may
                    // still be in use (actual locks have not been released.)
                    System.IO.File.Delete(this.Name);
                    return;
                }
                catch (System.IO.IOException)
                {
                }
                System.Threading.Thread.Sleep(500);
                System.IO.File.Delete(this.Name);
            }
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
                    Delete();
                }
            }
            m_disposed = true;
        }

        #endregion /* IDispose */
    }

    
    namespace Test
    {
        using NUnit.Framework;

        /// <summary>
        /// Unit tests for TemporaryFile
        /// </summary>
        [TestFixture]
        public class TemporaryFileTests
        {
            [Test]
            public void FileDoesntExist()
            {
                string fileName;
                using (TemporaryFile TemporaryFile = new TemporaryFile())
                {
                    fileName = TemporaryFile.Name;
                    Assert.IsFalse(System.IO.File.Exists( fileName));
                }
                Assert.IsFalse(System.IO.File.Exists(fileName));
            }

            //[NUnit.Framework.Ignore("The file sometimes exists after the 'using' scope.")] 
            [Test]
            public void FileExists()
            {
                string fileName;
                using (TemporaryFile temporaryFile = new TemporaryFile())
                {                    
                    fileName = temporaryFile.Name;
                    using (System.IO.FileStream fileStream = System.IO.File.OpenWrite(fileName))
                    {
                        fileStream.WriteByte(42);
                    }

                    Assert.IsTrue(System.IO.File.Exists(fileName));
                }
                Assert.IsFalse(System.IO.Directory.Exists(fileName));
            }
        }
    }

}
