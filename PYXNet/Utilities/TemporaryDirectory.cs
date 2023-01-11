/******************************************************************************
TemporaryDirectory.cs

begin      : May 23, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Utilities
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
        /// Deletes this instance.  Any contents are removed.
        /// </summary>
        public void Delete()
        {
            if (System.IO.Directory.Exists(this.Name))
            {                
                try
                {
                    // We will try to do this twice.  In many cases, the file may
                    // still be in use (actual locks have not been released.)
                    System.IO.Directory.Delete(this.Name, true);
                    return;
                }
                catch (System.IO.IOException)
                {
                }
                System.Threading.Thread.Sleep(500);
                System.IO.Directory.Delete(this.Name, true);
            }
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

    
    namespace Test
    {
        using NUnit.Framework;

        /// <summary>
        /// Unit tests for TemporaryDirectory
        /// </summary>
        [TestFixture]
        public class TemporaryDirectoryTests
        {
            private string GenerateBaseDir()
            {
                return string.Format("{0}{1}", System.IO.Path.GetTempPath(),
                    System.IO.Path.GetRandomFileName());
            }

            [Test]
            public void EmptyDirectory()
            {
                string dirName;
                using (TemporaryDirectory temporaryDirectory = new TemporaryDirectory(GenerateBaseDir()))
                {
                    dirName = temporaryDirectory.Name;
                    Assert.IsTrue(System.IO.Directory.Exists(dirName));
                }
                Assert.IsFalse(System.IO.Directory.Exists(dirName));
            }

            public void FilledDirectory()
            {
                string dirName;
                using (TemporaryDirectory temporaryDirectory = 
                    new TemporaryDirectory(PyxNet.Test.TestData.CreateRandomDirectory()))
                {
                    dirName = temporaryDirectory.Name;
                    Assert.IsTrue(System.IO.Directory.Exists(dirName));
                    Assert.IsTrue(System.IO.Directory.GetFiles(dirName).Length > 0);
                }
                PyxNet.TimedTest.Verify(
                    delegate() {return !System.IO.Directory.Exists(dirName);}, 
                    TimeSpan.FromSeconds( 10)); 
            }
        }
    }

}
