/******************************************************************************
ApplicationUtilityTests.cs

begin      : 08/15/2007 3:00:00 PM
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;

namespace ApplicationUtility.Test
{
    [TestFixture]
    public class ApplicationUtilityTester
    {
        /// <summary>
        /// The class that directs trace statements to the output window.
        /// </summary>
        static TraceCallback m_traceCallback;

        /// <summary>
        /// This setup is run before the entire series of tests. The cooresponding tear
        /// down is only called when the application goes out of scope.
        /// </summary>
        [TestFixtureSetUpAttribute]
        public void TestSetup()
        {
            try
            {
                // Initialize all of the SDK classes
                PYXLibInstance.initialize(
                    System.Reflection.Assembly.GetExecutingAssembly().GetName().Name, false);

                // assign the callback so that trace is redirected as soon as it is enabled
                m_traceCallback = new ApplicationUtility.TraceToConsoleCallback();
                Trace.setTraceCallback(m_traceCallback);

                Trace.getInstance().setLevels(519);
            }
            catch (Exception ex)
            {
                NUnit.Framework.Assert.IsTrue(false, 
                    string.Format("An error occurred during program initialization:\n{0}", ex.ToString()));
            }
        }

        // would use TestFixtureTearDown but it does not always get called on a cancel.
        [TestFixtureTearDownAttribute]
        public void TestCleanUp()
        {
            try
            {
                PYXLibInstance.uninitialize();
                Trace.setTraceCallback(null);
            }
            catch
            {
                NUnit.Framework.Assert.IsTrue(false, "An error occurred during program clean-up.");
            }
        }

        [Test]
        public void FlatDirectoryOfFiles()
        {
            // create a temp directory and fill it with files
            int fileCount = 15;
            int fileSize = 2500;
            System.IO.DirectoryInfo dirInfo = new System.IO.DirectoryInfo(AppServices.makeTempDir());
            FileUtility.CreateFlatDirectoryOfFiles(dirInfo.FullName, fileCount, fileSize);

            long dirSize = ApplicationUtility.FileUtility.GetDirectorySize(dirInfo);
            NUnit.Framework.Assert.AreEqual(dirSize, (fileCount * fileSize));
        }

        /// <summary>
        /// Test the limits of the
        /// </summary>
        [Test]
        public void MaxDirectoryCleanSize()
        {
            // create a cleaner to remove half of the bulk
            CacheCleaner cleaner = new CacheCleaner();

            long maxSize = 1000;
            cleaner.MaxDirectorySizeInBytes = 1000;
            NUnit.Framework.Assert.AreEqual(cleaner.MaxDirectorySizeInBytes, maxSize);

            maxSize = 2000 * 1024 * 1024;
            cleaner.MaxDirectorySizeInBytes = maxSize;
            NUnit.Framework.Assert.AreEqual(cleaner.MaxDirectorySizeInBytes, maxSize);
            NUnit.Framework.Assert.AreEqual(cleaner.MaxDirectorySizeInMB, 2000);

            maxSize = 3000;
            maxSize *= 1024;
            maxSize *= 1024;
            cleaner.MaxDirectorySizeInBytes = maxSize;
            NUnit.Framework.Assert.AreEqual(cleaner.MaxDirectorySizeInBytes, maxSize);
            cleaner.MaxDirectorySizeInMB = 6000;
            maxSize *= 2;
            NUnit.Framework.Assert.AreEqual(cleaner.MaxDirectorySizeInBytes, maxSize);

        }

        /// <summary>
        /// Tests the automated directory cleanup method
        /// </summary>
        [Test]
        public void FlatDirectoryClean()
        {
            // create a temp directory and fill it with files
            int fileCount = 250;
            int fileSize = 2500;
            System.IO.DirectoryInfo dirInfo = new System.IO.DirectoryInfo(AppServices.makeTempDir());
            FileUtility.CreateFlatDirectoryOfFiles(dirInfo.FullName, fileCount, fileSize);

            long dirSize = ApplicationUtility.FileUtility.GetDirectorySize(dirInfo);
            long targetSize = dirSize / 3;

            // create a cleaner to remove half of the bulk
            CacheCleaner cleaner = new CacheCleaner();
            cleaner.Interval = 0;
            cleaner.MaxDirectorySizeInBytes = targetSize;
            cleaner.CacheDirectory = dirInfo.FullName;

            // verify the cleaner is started
            NUnit.Framework.Assert.IsTrue(cleaner.StartCleaner());

            // cant start cleaning while already cleaning
            NUnit.Framework.Assert.IsTrue(!cleaner.StartCleaner());

            // wait until the cleaner is done
            while (cleaner.IsCleaning)
            {
                System.Threading.Thread.Sleep(100);
            }

            // verify the cleaner only ran once
            NUnit.Framework.Assert.IsTrue(!cleaner.IsCleaning);

            // verify the size is reduced
            long newSize = FileUtility.GetDirectorySize(dirInfo);
            NUnit.Framework.Assert.LessOrEqual(newSize, targetSize);
        }

        /// <summary>
        /// Test the buffer size setting on the directory cleaner
        /// </summary>
        [Test]
        public void DirectoryBufferClean()
        {
            // create a temp directory and fill it with files
            System.IO.DirectoryInfo dirInfo = new System.IO.DirectoryInfo(AppServices.makeTempDir());
            FileUtility.CreateFlatDirectoryOfFiles(dirInfo.FullName, 100, 100);

            // verify the cleaner does not start when between the max and the buffer
            CacheCleaner cleaner = new CacheCleaner();
            cleaner.Interval = 0;
            cleaner.MaxDirectorySizeInBytes = 11000;
            cleaner.DeleteBuffer = 10;
            cleaner.CacheDirectory = dirInfo.FullName;
            cleaner.StartCleaner();

            // wait until the cleaner is done
            while (cleaner.IsCleaning)
            {
                System.Threading.Thread.Sleep(100);
            }
            long newSize = FileUtility.GetDirectorySize(dirInfo);
            NUnit.Framework.Assert.AreEqual(newSize, 10000);

            // verify the size is reduced properly accounting for the buffer
            cleaner.MaxDirectorySizeInBytes = 5000;
            cleaner.DeleteBuffer = 20;
            cleaner.StartCleaner();

            // wait until the cleaner is done
            while (cleaner.IsCleaning)
            {
                System.Threading.Thread.Sleep(100);
            }

            // verify the size is reduced
            newSize = FileUtility.GetDirectorySize(dirInfo);
            NUnit.Framework.Assert.LessOrEqual(newSize, 4000);
        }

        /// <summary>
        /// Test to see if the directory cleaner can be cancelled.  
        /// This is determined by checking to see that the number of bytes cleaned is less than what was requested.
        /// </summary>
        [Test]
        public void StopDirectoryCleanerTest()
        {
            // NOTE[kabiraman]: I've changed fileCount from 250 to 2500 since frequently the cache cleaner 
            // has completed if the file count is 250 before it is asked to be cancelled.
            int fileCount = 2500;
            int fileSize = 2500;
            System.IO.DirectoryInfo dirInfo = new System.IO.DirectoryInfo(AppServices.makeTempDir());
            FileUtility.CreateFlatDirectoryOfFiles(dirInfo.FullName, fileCount, fileSize);

            int targetSize = fileCount / 10 * fileSize;

            // create a cleaner to remove 90% of the files
            CacheCleaner cleaner = new CacheCleaner();
            cleaner.Interval = 0;
            cleaner.MaxDirectorySizeInBytes = targetSize;
            cleaner.CacheDirectory = dirInfo.FullName;

            // cant start cleaning while already cleaning
            NUnit.Framework.Assert.IsTrue(cleaner.StartCleaner());
            System.Threading.Thread.Sleep(1);
            cleaner.StopCleaner();
            
            // wait until the cleaner is done
            while (cleaner.IsCleaning)
            {
                System.Threading.Thread.Sleep(100);
            }

            long newDirSize = FileUtility.GetDirectorySize(dirInfo);
            NUnit.Framework.Assert.Less(targetSize, newDirSize);
        }

        /// <summary>
        /// Verify the least recently accessed files are intact.
        /// </summary>
        [Test]
        public void FlatLeastRecentlyUsedDirectoryCleanTest()
        {
            // verify the newest files are still there
            // create a temp directory and fill it with files
            int fileCount = 250;
            int fileSize = 2500;
            System.IO.DirectoryInfo dirInfo = new System.IO.DirectoryInfo(AppServices.makeTempDir());
            FileUtility.CreateFlatDirectoryOfFiles(dirInfo.FullName, fileCount, fileSize);

            // delete all but 10 files
            int targetSize = fileSize * 10 + fileSize / 2;

            // touch 10 files throughout the range of files
            System.Threading.Thread.Sleep(250);

            
            //This test should pass but does not because, by default, the system
            //does not set a file as accessed when it is accessed. You have to
            //change a registry setting to get that behavior.
            //System.IO.FileStream stream = System.IO.File.OpenRead(dirInfo.FullName + "\\File Number 1.test");
            //stream = System.IO.File.OpenRead(dirInfo.FullName + "\\File Number 8.test");
            //stream = System.IO.File.OpenRead(dirInfo.FullName + "\\File Number 17.test");
            //stream = System.IO.File.OpenRead(dirInfo.FullName + "\\File Number 27.test");
            //stream = System.IO.File.OpenRead(dirInfo.FullName + "\\File Number 33.test");
            //stream = System.IO.File.OpenRead(dirInfo.FullName + "\\File Number 34.test");
            //stream = System.IO.File.OpenRead(dirInfo.FullName + "\\File Number 35.test");
            //stream = System.IO.File.OpenRead(dirInfo.FullName + "\\File Number 51.test");
            //stream = System.IO.File.OpenRead(dirInfo.FullName + "\\File Number 63.test");
            //stream = System.IO.File.OpenRead(dirInfo.FullName + "\\File Number 70.test");

            System.IO.FileInfo info =
                new System.IO.FileInfo(dirInfo.FullName + "\\File Number 1.test");
            info.LastWriteTime = DateTime.Now;
            info = new System.IO.FileInfo(dirInfo.FullName + "\\File Number 8.test");
            info.LastWriteTime = DateTime.Now;
            info = new System.IO.FileInfo(dirInfo.FullName + "\\File Number 17.test");
            info.LastWriteTime = DateTime.Now;
            info = new System.IO.FileInfo(dirInfo.FullName + "\\File Number 27.test");
            info.LastWriteTime = DateTime.Now;
            info = new System.IO.FileInfo(dirInfo.FullName + "\\File Number 33.test");
            info.LastWriteTime = DateTime.Now;
            info = new System.IO.FileInfo(dirInfo.FullName + "\\File Number 34.test");
            info.LastWriteTime = DateTime.Now;
            info = new System.IO.FileInfo(dirInfo.FullName + "\\File Number 35.test");
            info.LastWriteTime = DateTime.Now;
            info = new System.IO.FileInfo(dirInfo.FullName + "\\File Number 51.test");
            info.LastWriteTime = DateTime.Now;
            info = new System.IO.FileInfo(dirInfo.FullName + "\\File Number 63.test");
            info.LastWriteTime = DateTime.Now;
            info = new System.IO.FileInfo(dirInfo.FullName + "\\File Number 70.test");
            info.LastWriteTime = DateTime.Now;

            // run the cleaner
            CacheCleaner cleaner = new CacheCleaner();
            cleaner.Interval = 0;
            cleaner.MaxDirectorySizeInBytes = targetSize;
            cleaner.CacheDirectory = dirInfo.FullName;
            NUnit.Framework.Assert.IsTrue(cleaner.StartCleaner());
            while (cleaner.IsCleaning)
            {
                System.Threading.Thread.Sleep(100);
            }

            // verify the 10 touched files are still there
            dirInfo.Refresh();
            NUnit.Framework.Assert.AreEqual(dirInfo.GetFiles().Length, 10);
            NUnit.Framework.Assert.IsTrue(System.IO.File.Exists(
                dirInfo.FullName + "\\File Number 1.test"));
            NUnit.Framework.Assert.IsTrue(System.IO.File.Exists(
                dirInfo.FullName + "\\File Number 8.test"));
            NUnit.Framework.Assert.IsTrue(System.IO.File.Exists(
                dirInfo.FullName + "\\File Number 17.test"));
            NUnit.Framework.Assert.IsTrue(System.IO.File.Exists(
                dirInfo.FullName + "\\File Number 27.test"));
            NUnit.Framework.Assert.IsTrue(System.IO.File.Exists(
                dirInfo.FullName + "\\File Number 33.test"));
            NUnit.Framework.Assert.IsTrue(System.IO.File.Exists(
                dirInfo.FullName + "\\File Number 34.test"));
            NUnit.Framework.Assert.IsTrue(System.IO.File.Exists(
                dirInfo.FullName + "\\File Number 35.test"));
            NUnit.Framework.Assert.IsTrue(System.IO.File.Exists(
                dirInfo.FullName + "\\File Number 51.test"));
            NUnit.Framework.Assert.IsTrue(System.IO.File.Exists(
                dirInfo.FullName + "\\File Number 63.test"));
            NUnit.Framework.Assert.IsTrue(System.IO.File.Exists(
                dirInfo.FullName + "\\File Number 70.test"));
        }

        /// <summary>
        /// Verify multiple nested directories are properly cleaned up.
        /// </summary>
        [Test]
        public void DirectoryTreeCleanTest()
        {
            System.IO.DirectoryInfo rootDirInfo = new System.IO.DirectoryInfo(AppServices.makeTempDir());

            // fill the root directory with files (Size 10000 bytes)
            FileUtility.CreateFlatDirectoryOfFiles(rootDirInfo.FullName, 10, 1000);

            // create a system of nested directories (Size 10000 bytes)
            FileUtility.CreateFlatDirectoryOfFiles(rootDirInfo.FullName + "\\dir_10000_bytes", 30, 100);
            FileUtility.CreateFlatDirectoryOfFiles(rootDirInfo.FullName + "\\dir_10000_bytes\\dir_2000_bytes", 20, 100);
            FileUtility.CreateFlatDirectoryOfFiles(rootDirInfo.FullName + "\\dir_10000_bytes\\dir_5000_bytes", 50, 100);

            // create a second directory of nested directories (Size 5000 bytes)
            FileUtility.CreateFlatDirectoryOfFiles(rootDirInfo.FullName + "\\dir_5000_bytes", 10, 100);
            FileUtility.CreateFlatDirectoryOfFiles(rootDirInfo.FullName + "\\dir_5000_bytes\\dir_4000_bytes", 40, 100);

            // create a third directory of nested dirs (Size 6000 bytes)
            FileUtility.CreateFlatDirectoryOfFiles(rootDirInfo.FullName + "\\dir_6000_bytes", 20, 100);
            FileUtility.CreateFlatDirectoryOfFiles(rootDirInfo.FullName + "\\dir_6000_bytes\\dir_1000_bytes", 10, 100);
            FileUtility.CreateFlatDirectoryOfFiles(rootDirInfo.FullName + "\\dir_6000_bytes\\dir_3000_bytes", 30, 100);

            // verify the assumed size is correct
            long dirSize = FileUtility.GetDirectorySize(rootDirInfo);
            NUnit.Framework.Assert.AreEqual(31000, dirSize);

            // limit the directory size to 7500 bytes
            CacheCleaner cleaner = new CacheCleaner();
            cleaner.Interval = 0;
            cleaner.MaxDirectorySizeInBytes = 7501;
            cleaner.CacheDirectory = rootDirInfo.FullName;
            
            NUnit.Framework.Assert.IsTrue(
                cleaner.StartCleaner(), 
                "CacheCleaner failed to start!");
            
            while (cleaner.IsCleaning)
            {
                System.Threading.Thread.Sleep(100);
            }

            rootDirInfo.Refresh();

            // verify the overall size
            dirSize = FileUtility.GetDirectorySize(rootDirInfo);
            NUnit.Framework.Assert.AreEqual(7500, dirSize);

            // verify the oldest files were deleted from the root directory
            NUnit.Framework.Assert.AreEqual(
                0, 
                rootDirInfo.GetFiles().Length, 
                "CacheCleaner failed to delete oldest files as expected!");

            // verify the entire older nested directory tree is deleted
            NUnit.Framework.Assert.IsTrue(
                !System.IO.Directory.Exists(rootDirInfo.FullName + "\\dir_10000_bytes"), 
                "dir_10000_bytes not deleted as expected!");

            NUnit.Framework.Assert.IsTrue(
                System.IO.Directory.Exists(rootDirInfo.FullName + "\\dir_5000_bytes"),
                "dir_5000_bytes deleted when it shouldn't have been!");
            
            System.IO.DirectoryInfo tempDirInfo = 
                new System.IO.DirectoryInfo(rootDirInfo.FullName + "\\dir_5000_bytes");

            NUnit.Framework.Assert.AreEqual(0, tempDirInfo.GetFiles().Length,
                "\\dir_5000_bytes should have no files!");

            tempDirInfo = new System.IO.DirectoryInfo(rootDirInfo.FullName + "\\dir_5000_bytes");
            dirSize = FileUtility.GetDirectorySize(tempDirInfo);
            NUnit.Framework.Assert.AreEqual(1500, dirSize);

            // verify the newest directory is untouched
            tempDirInfo = new System.IO.DirectoryInfo(rootDirInfo.FullName + "\\dir_6000_bytes");
            NUnit.Framework.Assert.IsTrue(tempDirInfo.Exists, 
                "\\dir_6000_bytes unexpectedly deleted!");
            
            dirSize = FileUtility.GetDirectorySize(tempDirInfo);
            NUnit.Framework.Assert.AreEqual(6000, dirSize);
        }
    }
}
