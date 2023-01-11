using System;
using System.Collections.Generic;
using System.Text;

using Pyxis.Utilities;
using Pyxis.Utilities.Test;
using PyxNet.Pyxis;
using PyxNet.Test;
using PyxNet.FileTransfer;
using PyxNet.Publishing.Files;

// Test routines for functionality spanning Pyxis SDK and PyxNet.
namespace PyxNet.Pyxis.Test
{
    /// <summary>
    /// Test helper class to forge certificates.
    /// </summary>
    public class CertificateForger
    {
        /// <summary>
        /// Create a certificate and inject it into the stack's local repository.
        /// </summary>
        /// <param name="fact"></param>
        /// <param name="duration"></param>
        /// <param name="stack"></param>
        public static PyxNet.Service.Certificate CreateCertificateOnStack(
            PyxNet.Service.ICertifiableFact fact, Stack stack)
        {
            PyxNet.Service.ServiceInstance localService =
                PyxNet.Service.ServiceInstance.Create(
                PyxNet.Service.CertificateServer.CertificateAuthorityServiceId,
                stack.NodeInfo.NodeId);
            PyxNet.Service.Certificate forgedCertificate = new PyxNet.Service.Certificate(
                localService, DateTime.Now + TimeSpan.FromDays(1));
            forgedCertificate.Add(fact);
            forgedCertificate.SignCertificate(stack.PrivateKey);
            stack.CertificateRepository.Add(forgedCertificate);
            return forgedCertificate;
        }

        public static PyxNet.Service.ResourceDefinitionFact CreateResourceDefinitionFact(
            CoveragePublisher.PublishedCoverageInfo pubInfo)
        {
            PyxNet.Service.ResourceDefinitionFact newFact =
                new PyxNet.Service.ResourceDefinitionFact(pubInfo.CoverageInfo.PipelineDefinition);
            newFact.ResourceId = new PyxNet.Service.ResourceId(pubInfo.DataInfo.DataSetID.Guid);
            return newFact;
        }

        public static PyxNet.Service.ResourceDefinitionFact CreateResourceDefinitionFact(
            ManifestEntry manifestEntry)
        {
            PyxNet.Service.ResourceDefinitionFact newFact =
                new PyxNet.Service.ResourceDefinitionFact(manifestEntry.FileName);
            return newFact;
        }

        public static PyxNet.Service.ResourceInstanceFact CreateResourceInstanceFact(
            ManifestEntry manifestEntry)
        {
            PyxNet.Service.ResourceInstanceFact newFact =
                new PyxNet.Service.ResourceInstanceFact(manifestEntry);
            return newFact;
        }

        public static PyxNet.Service.ResourcePermissionFact CreateResourcePermissionFact(
            Manifest resource, NodeId authorizedNode)
        {
            PyxNet.Service.ResourcePermissionFact fact =
                new PyxNet.Service.ResourcePermissionFact(
                    new PyxNet.Service.ResourceId(resource.Id.Guid), authorizedNode);
            return fact;
        }

        public static PyxNet.Service.ResourcePermissionFact CreateResourcePermissionFact(
            CoveragePublisher.PublishedCoverageInfo pubInfo, Stack stack)
        {
            PyxNet.Service.ResourceId resourceID = new PyxNet.Service.ResourceId(pubInfo.DataInfo.DataSetID.Guid);
            PyxNet.Service.ResourcePermissionFact newFact =
                new PyxNet.Service.ResourcePermissionFact(resourceID,
                    stack.NodeInfo.NodeId);
            return newFact;
        }
    }

    /// <summary>
    /// Tests for the CoveragePublisher and Coverage Downloader
    /// </summary>
    [NUnit.Framework.TestFixture]
    public class PyxisIntegrationTests : IDisposable
    {
        const string WellKnownDataDirectory = "C:\\TESTDATA\\";

        // TODO: A temporary solution to work around track issue #654
        private static IList<object> m_holdOnToMe = new DynamicList<object>();

        /// <summary>
        /// A place to trace to.
        /// </summary>
        TraceCallback m_traceCallback;

        /// <summary>
        /// Constructor -- initializes the PyxLib and tracing.
        /// </summary>
        public PyxisIntegrationTests()
        {
            PYXLibInstance.initialize(
                System.Reflection.Assembly.GetExecutingAssembly().GetName().Name, false);
            m_traceCallback = new ApplicationUtility.TraceToConsoleCallback();
            Trace.setTraceCallback(m_traceCallback);
            ApplicationUtility.ManagedChecksumCalculator managedChecksumCalculator = 
                new ApplicationUtility.ManagedChecksumCalculator(ChecksumSingleton.Checksummer);
            ChecksumCalculator.setChecksumCalculator(new ChecksumCalculator_SPtr(
                managedChecksumCalculator));
            ApplicationUtility.ManagedCSharpFunctionProvider provider =
                new ApplicationUtility.ManagedCSharpFunctionProvider();
            CSharpFunctionProvider.setCSharpFunctionProvider(new CSharpFunctionProvider_SPtr(
                provider));
            IExcel.Implementation(ApplicationUtility.Excel.Instance);
        }

        #region Dispose

        /// <summary>
        /// Track whether Dispose has been called.
        /// </summary>
        private bool m_disposed = false;

        /// <summary>
        /// Implement IDisposable.
        /// </summary>
        /// <remarks>
        /// Do not make this method virtual.
        /// A derived class should not be able to override this method. 
        /// </remarks>
        public void Dispose()
        {
            Dispose(true);

            // This object will be cleaned up by the Dispose method.
            // Therefore, you should call GC.SupressFinalize to
            // take this object off the finalization queue 
            // and prevent finalization code for this object
            // from executing a second time.
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Dispose(bool disposing) executes in two distinct scenarios, 
        /// indicated by the "disposing" argument.
        /// </summary>
        /// <param name="disposing">
        /// If disposing equals true, the method has been called directly
        /// or indirectly by a user's code. Managed and unmanaged resources
        /// can be disposed.
        /// If disposing equals false, the method has been called by the 
        /// runtime from inside the finalizer and you should not reference 
        /// other objects. Only unmanaged resources can be disposed.
        /// </param>
        private void Dispose(bool disposing)
        {
            // Check to see if Dispose has already been called.
            if (!m_disposed)
            {
                // If disposing equals true, dispose all managed 
                // and unmanaged resources.
                if (disposing)
                {
                    // Dispose managed resources.
                }

                // Call the appropriate methods to clean up 
                // unmanaged resources here.
                // If disposing is false, 
                // only the following code is executed.
                try
                {
                    IExcel.Implementation();
                    CSharpFunctionProvider.setCSharpFunctionProvider(new CSharpFunctionProvider_SPtr());
                    ChecksumCalculator.setChecksumCalculator(new ChecksumCalculator_SPtr());
                    Trace.setTraceCallback(null);
                }
                catch
                {
                    // Finalizers can never throw.
                }
                try
                {
                    PYXLibInstance.uninitialize();
                }
                catch
                {
                    // Finalizers can never throw.
                }
            }
            m_disposed = true;
        }

        /// <summary>
        /// The finalization code.
        /// </summary>
        /// <remarks>
        /// Use C# destructor syntax for finalization code.
        /// This destructor will run only if the Dispose method 
        /// does not get called.
        /// It gives your base class the opportunity to finalize.
        /// Do not provide destructors in types derived from this class.
        /// </remarks>
        ~PyxisIntegrationTests()
        {
            // Do not re-create Dispose clean-up code here.
            // Calling Dispose(false) is optimal in terms of
            // readability and maintainability.
            Dispose(false);
        }

        #endregion

        private Stack ConstructStack()
        {
            return new Stack(new PyxNet.DLM.PrivateKey());
        }

        /// <summary>
        /// See if we can construct a CoveragePublisher.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestConstruction()
        {
            using (Stack stack = ConstructStack())
            {
                CoveragePublisher myPublisher = new CoveragePublisher(stack);
            }
        }

        /// <summary>
        /// See if we can construct a CoverageDownloader.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestDownloaderConstruction()
        {
            using (Stack stack = ConstructStack())
            {
                // create a coverage cache process.
                IUnknown_SPtr coverageCacheIUnknown =
                    pyxlib.PYXCOMhelpCreate(pyxlib.strToGuid("{83F35C37-5D0A-41c9-A937-F8C9C1E86850}"));
                m_holdOnToMe.Add(coverageCacheIUnknown);
                IProcess_SPtr spCacheProc = pyxlib.QueryInterface_IProcess(coverageCacheIUnknown);

                CoverageDownloader myDownloader = new CoverageDownloader(stack, spCacheProc);
            }
        }

        /// <summary>
        /// Make sure that the CoverageDownloader constructor will throw if it is constructed
        /// without a cache process.
        /// </summary>
        [NUnit.Framework.Test]
        [NUnit.Framework.ExpectedException(typeof(ArgumentException))]
        public void TestDownloaderConstructionFailure()
        {
            using (Stack stack = ConstructStack())
            {
                // create a const coverage process.
                IUnknown_SPtr constConverageIUnknown =
                    pyxlib.PYXCOMhelpCreate(pyxlib.strToGuid("{8517369E-B91F-46be-BC8A-82E3F414D6AA}"));
                m_holdOnToMe.Add(constConverageIUnknown);
                IProcess_SPtr spNotCacheProc = pyxlib.QueryInterface_IProcess(constConverageIUnknown);

                // this should blow up because the spNotCacheProc does not refer to a cache in this case.
                CoverageDownloader myDownloader = new CoverageDownloader(stack, spNotCacheProc);
            }
        }

        /// <summary>
        /// Test that publishing fails with no publishing certificate in the PyxNet system.
        /// </summary>
        [NUnit.Framework.Test]
        [NUnit.Framework.Ignore]
        public void TestPublishAConstantCoverageNoRights()
        {
            using (Stack stack = ConstructStack())
            {
                CoveragePublisher myPublisher = new CoveragePublisher(stack);

                IUnknown_SPtr constConverageIUnknown =
                    pyxlib.PYXCOMhelpCreate(pyxlib.strToGuid("{8517369E-B91F-46be-BC8A-82E3F414D6AA}"));
                m_holdOnToMe.Add(constConverageIUnknown);
                IProcess_SPtr spConstProc = pyxlib.QueryInterface_IProcess(constConverageIUnknown);
                NUnit.Framework.Assert.IsFalse(myPublisher.Publish(spConstProc, null).get() != null);
            }
        }

        /// <summary>
        /// Test that publishing is OK with a publishing certificate on the local stack.
        /// </summary>
        [NUnit.Framework.Test]
        [NUnit.Framework.Ignore]
        public void TestPublishAConstantCoverageLocalRights()
        {
            using (Stack stack = ConstructStack())
            {
                CoveragePublisher myPublisher = new CoveragePublisher(stack);

                IUnknown_SPtr constConverageIUnknown =
                    pyxlib.PYXCOMhelpCreate(pyxlib.strToGuid("{8517369E-B91F-46be-BC8A-82E3F414D6AA}"));
                m_holdOnToMe.Add(constConverageIUnknown);
                IProcess_SPtr spConstProc = pyxlib.QueryInterface_IProcess(constConverageIUnknown);

                // Forge and add the appropriate certificate to the stack so that it can be found.
                CoveragePublisher.PublishedCoverageInfo pubInfo =
                    new CoveragePublisher.PublishedCoverageInfo(spConstProc);
                PyxNet.Service.ResourceDefinitionFact fact = CertificateForger.CreateResourceDefinitionFact(pubInfo);
                CertificateForger.CreateCertificateOnStack(fact, stack);

                // We need both a ResourceDefinitionFact and a ResourcePermissionFact but we only have a 
                // resource definition fact so publishing should fail.
                NUnit.Framework.Assert.IsFalse(myPublisher.Publish(spConstProc, null).get() != null);

                // now add the appropriate ResourcePermissionFact to the local stack.
                PyxNet.Service.ResourcePermissionFact fact2 = CertificateForger.CreateResourcePermissionFact(pubInfo, stack);
                CertificateForger.CreateCertificateOnStack(fact2, stack);

                // finally we should be OK to publish.
                NUnit.Framework.Assert.IsTrue(myPublisher.Publish(spConstProc, null).get() != null);
            }
        }

        /// <summary>
        /// Test that publishing is OK with a publishing certificate on a remote stack.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestPublishAConstantCoverageRemoteRights()
        {
            using (StackTestHelper testHelper = new StackTestHelper(StackTestHelper.Topology.Two, 50))
            {
                CoveragePublisher myPublisher = new CoveragePublisher(testHelper.Stacks[1]);

                IUnknown_SPtr constConverageIUnknown =
                    pyxlib.PYXCOMhelpCreate(pyxlib.strToGuid("{8517369E-B91F-46be-BC8A-82E3F414D6AA}"));
                m_holdOnToMe.Add(constConverageIUnknown);
                IProcess_SPtr spConstProc = pyxlib.QueryInterface_IProcess(constConverageIUnknown);

                // Forge and add the appropriate certificates to a stack so that it can be found over PyxNet.
                CoveragePublisher.PublishedCoverageInfo pubInfo = 
                    new CoveragePublisher.PublishedCoverageInfo(spConstProc);
                PyxNet.Service.ResourceDefinitionFact fact = 
                    CertificateForger.CreateResourceDefinitionFact(pubInfo);
                CertificateForger.CreateCertificateOnStack(fact, testHelper.Stacks[2]);
                // create a fact that stack 1 is allowed access.
                PyxNet.Service.ResourcePermissionFact fact2 =
                    CertificateForger.CreateResourcePermissionFact(pubInfo, testHelper.Stacks[1]);
                // put that fact on stack 2.
                CertificateForger.CreateCertificateOnStack(fact2, testHelper.Stacks[2]);

                // need to wait here for query hash tables to be updated.
                System.Threading.Thread.Sleep(TimeSpan.FromSeconds(2));

                // we should now succeed in publishing.
                NUnit.Framework.Assert.IsTrue(myPublisher.Publish(spConstProc, null).get() != null);
            }
        }

        /// <summary>
        /// See if we can publish and then download a coverage.
        /// </summary>
        [NUnit.Framework.Test]
        [NUnit.Framework.Ignore]
        public void TestPublishAndDownloadAConstantCoverageWithLicenseServer()
        {
            // Simple test to publish and download on a topology two PyxNet stack configuration
            // publishing on stack 1, and downloading on stack 3, and serving certificates on stack 4.

            // We use a StackTestHelper object because it has all kinds of magic in the dispose
            // code to clean up stacks.
            using (StackTestHelper testHelper = new StackTestHelper(StackTestHelper.Topology.Two, 50))
            {
                PyxNet.Service.ServiceInstance licenseServer = null;
                // create a certificate server on node 4 (a hub)
                DemoCertificateServer certServer = new DemoCertificateServer(testHelper.Stacks[4]);
                certServer.PermitPublication = true;
                licenseServer = certServer.Certificate.ServiceInstance;

                // let the query hash tables get built.
                System.Threading.Thread.Sleep(500);
                // publish a constant coverage on Node 1
                CoveragePublisher myPublisher = new CoveragePublisher(testHelper.Stacks[1]);
                IUnknown_SPtr constConverageIUnknown =
                    pyxlib.PYXCOMhelpCreate(pyxlib.strToGuid("{8517369E-B91F-46be-BC8A-82E3F414D6AA}"));
                m_holdOnToMe.Add(constConverageIUnknown);
                IProcess_SPtr spConstProc = pyxlib.QueryInterface_IProcess(constConverageIUnknown);
                spConstProc.initProc();
                IProcess_SPtr spPublishedProc = myPublisher.Publish(spConstProc, licenseServer);
                NUnit.Framework.Assert.IsTrue(spPublishedProc.get() != null);

                // let the query hash tables get built.
                System.Threading.Thread.Sleep(500);

                // create a coverage cache process.
                IUnknown_SPtr coverageCacheIUnknown =
                    pyxlib.PYXCOMhelpCreate(pyxlib.strToGuid("{83F35C37-5D0A-41c9-A937-F8C9C1E86850}"));
                m_holdOnToMe.Add(coverageCacheIUnknown);
                IProcess_SPtr spCacheProc = pyxlib.QueryInterface_IProcess(coverageCacheIUnknown);
                spCacheProc.setProcID(spPublishedProc.getProcID());
                spCacheProc.setProcVersion(spPublishedProc.getProcVersion());
                spCacheProc.setProcDescription("Coverage from PyxNet.");

                // create a process (ICoverage) that won't initialize to plug into the cache as an input.
                IUnknown_SPtr badCoverageIUnknown =
                    pyxlib.PYXCOMhelpCreate(pyxlib.strToGuid("{BF12B7D4-9FC7-4640-A5C5-141C97EB4639}"));
                m_holdOnToMe.Add(badCoverageIUnknown);
                IProcess_SPtr spBadProc = pyxlib.QueryInterface_IProcess(badCoverageIUnknown);
                spCacheProc.getParameter(0).addValue(spBadProc);

                ICoverage_SPtr spCacheCoverage = pyxlib.QueryInterface_ICoverage(spCacheProc.getOutput());

                spCacheProc.initProc();

                // create the Coverage Downloader
                CoverageDownloader myDownloader = 
                    new CoverageDownloader(testHelper.Stacks[3], spCacheProc);

                // This next line fails, because the logic of this testHelper is broken.  EG.
                NUnit.Framework.Assert.IsTrue(myDownloader.Initialize());

                PYXIcosIndex pii = new PYXIcosIndex("B-0000");
                PYXValue returnValue = spCacheCoverage.getCoverageValue(pii);

                GC.KeepAlive(myPublisher);
            }
        }

        /// <summary>
        /// Test construction and moving to/from a message.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestCoverageRequestMessage()
        {
            // Create a CoverageRequestMessage object.
            CoverageRequestMessage crMessage = new CoverageRequestMessage();
            crMessage.CacheTileDepth = 10;
            crMessage.Mode = CoverageRequestMessage.TransferMode.DataSourceValues;
            crMessage.ProcessVersion = 42;
            crMessage.TileIndex = "A-00";
            crMessage.CellResolution = 5;
            crMessage.PipelineDefinition = "This would be an <XML> string of PPL definition.";
            NUnit.Framework.Assert.IsTrue(crMessage.Geometry == null);

            // Convert it to a message.
            Message aMessage = crMessage.ToMessage();

            // Construct a new one from the message.
            CoverageRequestMessage reconstructed = new CoverageRequestMessage(aMessage);

            // Ensure that the new one contains the same information as the old.
            NUnit.Framework.Assert.AreEqual(crMessage.CacheTileDepth, reconstructed.CacheTileDepth);
            NUnit.Framework.Assert.AreEqual(crMessage.Mode, reconstructed.Mode);
            NUnit.Framework.Assert.AreEqual(crMessage.ProcessVersion, reconstructed.ProcessVersion);
            NUnit.Framework.Assert.AreEqual(crMessage.TileIndex, reconstructed.TileIndex);
            NUnit.Framework.Assert.AreEqual(crMessage.CellResolution, reconstructed.CellResolution);
            NUnit.Framework.Assert.AreEqual(crMessage.PipelineDefinition, reconstructed.PipelineDefinition);
            NUnit.Framework.Assert.AreEqual(reconstructed.Geometry, null);

            // Test with a geometry.
            PYXTileCollection_SPtr aGeometry = PYXTileCollection.create();
            aGeometry.addTile(PYXTile.create(new PYXIcosIndex("A-000"), 5));
            crMessage.Geometry = new PYXGeometry_SPtr(aGeometry.get());

            // Convert it to a message.
            Message aMessage2 = crMessage.ToMessage();

            // Construct a new one from the message.
            CoverageRequestMessage reconstructed2 = new CoverageRequestMessage(aMessage2);

            // see if the reconstructed geometry interects the original geometry.
            PYXGeometry g = reconstructed2.Geometry.get() as PYXGeometry;
            NUnit.Framework.Assert.IsTrue(g.intersects(aGeometry.get()));

            // look for equality between the original geometry and the reconstructed geometry
            PYXTileCollection_SPtr reconGeometry = 
                pyxlib.DynamicPointerCast_PYXTileCollection(reconstructed2.Geometry);
            NUnit.Framework.Assert.IsTrue(reconGeometry.isEqual(aGeometry.get()));
        }

        [NUnit.Framework.Test]
        public void DownloadManifestEntry()
        {
            // Note the stacking of "using" here - all of these things will get disposed.
            using (StackTestHelper testHelper = new StackTestHelper(StackTestHelper.Topology.Two, 50))
            using (TemporaryDirectory stackOneTemp =
                new TemporaryDirectory(GenerateTempDirectoryName(testHelper.Stacks[1])))
            using (TemporaryDirectory stackFourTemp =
                new TemporaryDirectory(GenerateTempDirectoryName(testHelper.Stacks[4])))
            {
                //testHelper.Stacks[1].Tracer.Enabled = true;
                //testHelper.Stacks[4].Tracer.Enabled = true;

                // Generate a file to share, and place it on node 1.
                FilePublisher myPublisher = new FilePublisher(testHelper.Stacks[1]);
                System.IO.FileInfo publishedFile = new System.IO.FileInfo(
                    TestData.CreateRandomFile( stackOneTemp.Name));
                ManifestEntry manifest =
                    new ManifestEntry(publishedFile, "");
                myPublisher.Publish(publishedFile, "", 4096);

                // let the query hash tables get built.
                System.Threading.Thread.Sleep(500);

                string baseDirectory = stackFourTemp.Name;
                string temporaryDirectory =
                    string.Format("{0}{1}{2}.Temp{1}{3}",
                    baseDirectory,
                    System.IO.Path.DirectorySeparatorChar,
                    manifest.FilePath,
                    manifest.FileName);
                string outputDirectory =
                    string.Format("{0}{1}{2}{1}{3}",
                    baseDirectory,
                    System.IO.Path.DirectorySeparatorChar,
                    manifest.FilePath,
                    manifest.FileName);
                PyxNet.FileTransfer.DownloadContext downloadContext =
                    new PyxNet.FileTransfer.DownloadContext(outputDirectory, temporaryDirectory);

                // Now download the file.
                DownloadManifestEntryImpl(testHelper.Stacks[4], stackFourTemp, manifest, downloadContext);

                myPublisher.StopPublishing();
                System.Threading.Thread.Sleep(100);
            }
        }

        private static bool DownloadManifestEntryImpl(
            Stack stack,
            TemporaryDirectory stackFourTemp,
            ManifestEntry manifest,
            PyxNet.FileTransfer.DownloadContext downloadContext)
        {
            return DownloadManifestEntryImpl(stack, stackFourTemp, manifest, downloadContext, null);
        }

        private static bool DownloadManifestEntryImpl(
            Stack stack, 
            TemporaryDirectory stackFourTemp, 
            ManifestEntry manifest,
            PyxNet.FileTransfer.DownloadContext downloadContext,
            DeadManTimer keepAlive)
        {
            stack.Tracer.Enabled = true;

            PyxNet.Service.ResourceDefinitionFact fact =
                CertificateForger.CreateResourceDefinitionFact(manifest);
            PyxNet.Service.Certificate certificate =
                CertificateForger.CreateCertificateOnStack(fact, stack);

            ManifestDownloader downloader = new ManifestDownloader(stack,manifest,certificate,downloadContext);

            downloader.Download();
            if (!downloader.Failed)
            {
                stack.Tracer.WriteLine("Download Complete.  Success.");
            }
            else
            { 
                stack.Tracer.WriteLine("Download Failed.");
            }

            return !downloader.Failed;
        }

        
        [NUnit.Framework.Test]
        [NUnit.Framework.Ignore]
        public void DownloadManifest()
        {
            // Note the stacking of "using" here - all of these things will get disposed.
            using (StackTestHelper testHelper = new StackTestHelper(StackTestHelper.Topology.Two, 50))
            using (TemporaryDirectory stackOneTemp =
                TestData.CreateRandomTemporaryDirectory())
            using (TemporaryDirectory stackFourTemp =
                new TemporaryDirectory(GenerateTempDirectoryName(testHelper.Stacks[4])))
            {
                //testHelper.Stacks[1].Tracer.Enabled = true;
                //testHelper.Stacks[4].Tracer.Enabled = true;

                // Generate a file to share, and place it on node 1.
                string manifestShortName = "DownloadManifest.manifest";
                string manifestFileName = stackOneTemp.Name +
                    System.IO.Path.DirectorySeparatorChar +
                    manifestShortName;

                PyxNet.FileTransfer.ManifestBuilder manifestBuilder =
                    new PyxNet.FileTransfer.ManifestBuilder(stackOneTemp.Name,
                    manifestFileName);
                manifestBuilder.Build();

                // Forge a certificate for that manifest.
                ManifestEntry manifestEntry =
                    new ManifestEntry(
                    new System.IO.FileInfo(manifestFileName), "");
                PyxNet.Service.ResourceInstanceFact fact =
                    CertificateForger.CreateResourceInstanceFact(manifestEntry);
                // The created certificate will automatically publish on the stack.
                PyxNet.Service.Certificate publishingCertificate =
                    CertificateForger.CreateCertificateOnStack(fact, testHelper.Stacks[1]);

                // Publish everything in the directory.
                FilePublisher myPublisher = new FilePublisher(testHelper.Stacks[1]);
                myPublisher.PublishDirectory(
                    new System.IO.DirectoryInfo(stackOneTemp.Name), true);

                IDictionary<PyxNet.Service.ResourceInstanceFact,
                    PyxNet.FileTransfer.ManifestDownloader> downloaders =
                    new Dictionary<PyxNet.Service.ResourceInstanceFact,
                    PyxNet.FileTransfer.ManifestDownloader>();

                List<PyxNet.FileTransfer.ManifestDownloader> finishedDownloaders =
                    new List<PyxNet.FileTransfer.ManifestDownloader>();

                try
                {
                    // let the query hash tables get built.
                    System.Threading.Thread.Sleep(500);

                    SynchronizationEvent testRunning =
                        new SynchronizationEvent(TimeSpan.FromSeconds(30));

                    // Create a "finder" to do a query.
                    PyxNet.Service.ResourceFinder resourceFinder =
                        new PyxNet.Service.ResourceFinder(testHelper.Stacks[4], manifestShortName);

                    // Attach to the finder's events.
                    Object lockFinderResult = new Object();
                    PyxNet.Service.ResourceInstanceFact finderResult = null;
                    Manifest manifest = null;
                    resourceFinder.FactFound +=
                        delegate(object o, PyxNet.Service.ResourceFinder.FactFoundEventArgs arg)
                        {
                            lock (lockFinderResult)
                            {
                                if (finderResult != null)
                                {
                                    NUnit.Framework.Assert.IsNull(arg.ResourceInstanceFact, "Found two facts.  The testHelper should only have one.");
                                    return;
                                }
                                finderResult = arg.ResourceInstanceFact;
                            }

							// TODO: Background thread?
                            new System.Threading.Thread(
                                delegate()
                                {
                                    string baseDirectory = stackFourTemp.Name;
                                    string temporaryDirectory =
                                        string.Format("{0}{1}{2}.Temp{1}{3}",
                                        baseDirectory,
                                        System.IO.Path.DirectorySeparatorChar,
                                        manifestEntry.FilePath,
                                        manifestEntry.FileName);
                                    string outputDirectory =
                                        string.Format("{0}{1}{2}{1}{3}",
                                        baseDirectory,
                                        System.IO.Path.DirectorySeparatorChar,
                                        manifestEntry.FilePath,
                                        manifestEntry.FileName);
                                    PyxNet.FileTransfer.DownloadContext downloadContext =
                                        new PyxNet.FileTransfer.DownloadContext(outputDirectory, temporaryDirectory);

                                    // Now download the manifest file.
                                    if (DownloadManifestEntryImpl(testHelper.Stacks[4], stackFourTemp, manifestEntry, downloadContext) == false)
                                    {
                                        // What to do on failure?
                                        return;
                                    }

                                    if (downloaders.ContainsKey(arg.ResourceInstanceFact) == false)
                                    {
                                        // Load the manifest file into memory.
                                        string downloadedManifestFileName =
                                            downloadContext.ConstructTempPath(manifestEntry);
                                        manifest = Manifest.ReadFromFile(
                                            downloadedManifestFileName);

                                        // Forge a permission certificate.
                                        PyxNet.Service.Certificate permissionCertificate =
                                            CertificateForger.CreateCertificateOnStack(
                                                CertificateForger.CreateResourcePermissionFact(
                                                    manifest, testHelper.Stacks[4].NodeInfo.NodeId),
                                                testHelper.Stacks[0]);

                                        PyxNet.FileTransfer.ManifestDownloader downloader =
                                            new PyxNet.FileTransfer.ManifestDownloader(
                                            testHelper.Stacks[4], manifest, permissionCertificate,
                                            testHelper.Stacks[4].NodeInfo.FriendlyName + "." + System.IO.Path.GetRandomFileName());

                                        downloaders[arg.ResourceInstanceFact] = downloader;

                                        downloader.MaximumParallelDownloads = 3;

                                        downloader.DownloadFinished +=
                                            delegate(object sender, PyxNet.FileTransfer.ManifestDownloader.DownloadFinishedEventArgs e)
                                            {
                                                finishedDownloaders.Add(e.ManifestDownloader);
                                                testRunning.Pulse();
                                            };

                                        downloader.Start();
                                    }
                                }).Start();
                        };

                    // There should not be any local certificates.
                    resourceFinder.FindLocal();
                    NUnit.Framework.Assert.AreEqual(0, resourceFinder.Results.Count);

                    resourceFinder.StartRemoteQuery(TimeSpan.FromSeconds(10));

                    testRunning.Wait();

                    foreach (PyxNet.FileTransfer.ManifestDownloader downloader in finishedDownloaders)
                    {
                        PyxNet.FileTransfer.DownloadContext.DownloadedManifest tester =
                            new PyxNet.FileTransfer.DownloadContext.DownloadedManifest(
                            downloader.DownloadContext, downloader.Manifest);
                        if (tester.Valid)
                        {
                            // All is well.  We downloaded a full installation.
                            return;
                        }
                    }

                    NUnit.Framework.Assert.Fail("Manifest contents were not downloaded.");
                }
                finally
                {
                    myPublisher.StopPublishing();
                    System.Threading.Thread.Sleep(100);

                    foreach (PyxNet.FileTransfer.ManifestDownloader downloader in downloaders.Values)
                    {
                        // TODO: Actually stop the downloads.
                        try
                        {
                            System.IO.Directory.Delete(downloader.DownloadContext.FinalDirectory, true);
                        }
                        catch { }
                        try
                        {
                            System.IO.Directory.Delete(downloader.DownloadContext.TempDirectory, true);
                        }
                        catch { }
                    }
                }
            }
        }

        private string GenerateTempDirectoryName(Stack stack)
        {
            System.Reflection.Assembly assembly = System.Reflection.Assembly.GetCallingAssembly();

            return string.Format("{0}{1}{2}{1}CPDTests",
                System.IO.Path.GetDirectoryName(assembly.Location),
                System.IO.Path.DirectorySeparatorChar,
                stack.NodeInfo.FriendlyName);
        }

        /// <summary>
        /// Testing PYXValue over the swig boundary.
        /// </summary>
        [NUnit.Framework.Test]
        public void PYXValueSetting()
        {
            // NOTE: if you use new PYXValue() without a type it will
            // create a null PYXValue and setDouble() will have no effect,
            // so we set the type and then set the value.
            PYXValue val1 = new PYXValue();
            NUnit.Framework.Assert.AreEqual(PYXValue.eType.knNull, val1.getType());
            NUnit.Framework.Assert.IsTrue(val1.isNull(), "The value should be null.");
            val1.setType(PYXValue.eType.knDouble);
            val1.setDouble(42.3);
            NUnit.Framework.Assert.AreEqual(PYXValue.eType.knDouble, val1.getType());
            NUnit.Framework.Assert.AreEqual(42.3, val1.getDouble());

            PYXValue val2 = new PYXValue(0.0F);
            val2.setDouble(42.4F);
            NUnit.Framework.Assert.AreEqual(PYXValue.eType.knFloat, val2.getType());
            NUnit.Framework.Assert.AreEqual(42.4F, val2.getDouble());

            double myDouble = 35.6;
            PYXValue val3 = new PYXValue(myDouble);
            NUnit.Framework.Assert.AreEqual(PYXValue.eType.knDouble, val3.getType());
            NUnit.Framework.Assert.AreEqual(myDouble, val3.getDouble());

            // this PYXValue is an int and should thus truncate the double value 
            // that is stored to it.
            PYXValue val4 = new PYXValue(2);
            val4.setDouble(35.2);
            NUnit.Framework.Assert.AreEqual(PYXValue.eType.knInt32, val4.getType());
            NUnit.Framework.Assert.AreEqual(35.0, val4.getDouble());
        }

        [NUnit.Framework.Test]
        public void MultipleMessageHandling()
        {
            // Note the stacking of "using" here - all of these things will get disposed.
            using (StackTestHelper testHelper = new StackTestHelper(StackTestHelper.Topology.One, 50))
            {
                SynchronizationEvent finishHandlingFirstMessage =
                    new SynchronizationEvent();

                bool messageOneHandled = false;
                bool messageTwoHandled = false;
                bool messageTwoHandledBeforeMessageOne = false;

                testHelper.Stacks[0].RegisterHandler("MSG1",
                    delegate(object sender, PyxNet.MessageHandlerCollection.MessageReceivedEventArgs args)
                    {
                        finishHandlingFirstMessage.Wait();
                        messageOneHandled = true;
                    }
                );

                testHelper.Stacks[0].RegisterHandler("MSG2",
                    delegate(object sender, PyxNet.MessageHandlerCollection.MessageReceivedEventArgs args)
                    {
                        messageTwoHandled = true;
                        messageTwoHandledBeforeMessageOne = !messageOneHandled;
                    }
                );

                Message messageOne = new Message("MSG1");
                Message messageTwo = new Message("MSG2");

                StackConnection connection = testHelper.Stacks[1].GetConnection(testHelper.Stacks[0].NodeInfo, false, TimeSpan.Zero);
                NUnit.Framework.Assert.IsNotNull(connection, "Unable to connect stack 1 to 0.");
                bool messageOneSent = connection.SendMessage(messageOne);
                bool messageTwoSent = connection.SendMessage(messageTwo);
                NUnit.Framework.Assert.IsTrue(messageOneSent);
                NUnit.Framework.Assert.IsTrue(messageTwoSent);

                NUnit.Framework.Assert.IsFalse(messageOneHandled);
                NUnit.Framework.Assert.IsTrue(
                    TcpNetworkConnection.AsynchronousMessageHandling ||
                    !messageTwoHandled);

                System.Threading.Thread.Sleep(TimeSpan.FromMilliseconds(500));
                finishHandlingFirstMessage.Pulse();
                System.Threading.Thread.Sleep(TimeSpan.FromMilliseconds(50));

                NUnit.Framework.Assert.IsTrue(messageOneHandled);
                NUnit.Framework.Assert.IsTrue(messageTwoHandled);
                NUnit.Framework.Assert.AreEqual(TcpNetworkConnection.AsynchronousMessageHandling, messageTwoHandledBeforeMessageOne);
            }
        }

        /// <summary>
        /// Time the loading of the BMNG data set.
        /// This data set uses a First Non Null process to stitch together
        /// 8 large bitmap files containing Blue Marble Next Generation data.
        /// All tiles at res 2 with a depth of 11.  First 10 tiles for now.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestBMNG()
        {
            string inputPPL = WellKnownDataDirectory + "Blue Marble 500m\\BMNG.ppl";
            if (!System.IO.File.Exists(inputPPL))
            {
                System.Diagnostics.Trace.TraceWarning("TestWarning: could not open file " + inputPPL);
                return;
            }
            IProcess_SPtr pipeline = PipeManager.readPipelineFromFile(inputPPL);
            NUnit.Framework.Assert.AreEqual(IProcess.eInitStatus.knInitialized, pipeline.initProc(true), "Can't initialize pipeline.");
            ICoverage_SPtr coverage = pyxlib.QueryInterface_ICoverage(pipeline.getOutput());
            NUnit.Framework.Assert.IsNotNull(coverage.get(), "Could not query interface for a coverage.");

            PYXIcosIterator iter = new PYXIcosIterator(2);
            int tileCount = 0;
            while (!iter.end() && tileCount < 10)
            {
                PYXValueTile_SPtr spResult = coverage.getFieldTile(iter.getIndex(), 13, 0);
                iter.next();
                ++tileCount;
            }
        }

        /// <summary>
        /// Time the loading of the World2Minute data set.
        /// All tiles at res 2 with a depth of 5.  This is 
        /// equivelent to reading all the elevation for the 
        /// globe level view in WorldView for the entire earth.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestWorld2Minute()
        {
            string inputPPL = WellKnownDataDirectory + "World Terrain\\world_2minute.ppl";
            if (!System.IO.File.Exists(inputPPL))
            {
                System.Console.WriteLine("TestWarning: could not open file " + inputPPL);
                return;
            }
            IProcess_SPtr pipeline = PipeManager.readPipelineFromFile(inputPPL);
            NUnit.Framework.Assert.AreEqual(IProcess.eInitStatus.knInitialized, pipeline.initProc(true), "Can't initialize pipeline.");
            ICoverage_SPtr coverage = pyxlib.QueryInterface_ICoverage(pipeline.getOutput());
            NUnit.Framework.Assert.IsNotNull(coverage.get(), "Could not query interface for a coverage.");

            PYXIcosIterator iter = new PYXIcosIterator(2);
            while (!iter.end())
            {
                PYXValueTile_SPtr spResult = coverage.getFieldTile(iter.getIndex(), 7, 0);
                iter.next();
            }
        }

        /// <summary>
        /// Time the loading of the Kingston High Res data set.
        /// This test uses a blender to combine four high res images of dupont
        /// in Kingston.
        /// Iterate over depth 11 tiles for the complete geometry getting data 
        /// for every 110th tile so that the test will run to completion in around 2
        /// minutes.
        /// </summary>
        [NUnit.Framework.Test]
        [NUnit.Framework.Ignore("Trying to get autobuild to work.")]
        public void RenamedTestKingstonHighResolution()
        {
            string inputPPL = WellKnownDataDirectory + "Kingston High res\\Dupont blended.ppl";

            if (!System.IO.File.Exists(inputPPL))
            {
                System.Console.WriteLine("TestWarning: could not open file " + inputPPL);
                return;
            }

            IProcess_SPtr pipeline = PipeManager.readPipelineFromFile(inputPPL);
            NUnit.Framework.Assert.AreEqual(IProcess.eInitStatus.knInitialized, pipeline.initProc(true), "Can't initialize pipeline.");
            ICoverage_SPtr coverage = pyxlib.QueryInterface_ICoverage(pipeline.getOutput());
            NUnit.Framework.Assert.IsNotNull(coverage.get(), "Could not query interface for a coverage.");

            PYXGeometry_SPtr spGeometry = coverage.getGeometry().clone();
            int dataResolution = spGeometry.getCellResolution();
            spGeometry.setCellResolution(dataResolution - 11);
            PYXIterator_SPtr spIter = spGeometry.getIterator();
            int numberOfTilesInDataset = 0;
            int loadEveryNth = 110;
            while (!spIter.end())
            {
                if ((numberOfTilesInDataset % loadEveryNth) == 0)
                {
                    PYXValueTile_SPtr spResult = coverage.getFieldTile(spIter.getIndex(), dataResolution, 0);
                }
                ++numberOfTilesInDataset;
                spIter.next();
            }
        }

        /// <summary>
        /// Time the loading of the World Political Boundaries data set.
        /// Iterate through all points in the set without restricting by geometry then,
        /// iterate through all points in the set for all tiles at res 2 with a depth of 11.
        /// </summary>
        [NUnit.Framework.Test]
        [NUnit.Framework.Ignore("Memory management issue causes this to crash at shut-down.")]
        public void TestWorldPoliticalBoundaries()
        {
            string inputPPL = WellKnownDataDirectory + "World Political Boundaries\\World Political Boundaries.ppl";
            if (!System.IO.File.Exists(inputPPL))
            {
                System.Console.WriteLine("TestWarning: could not open file " + inputPPL);
                return;
            }

            IProcess_SPtr pipeline = PipeManager.readPipelineFromFile(inputPPL);
            NUnit.Framework.Assert.AreEqual(IProcess.eInitStatus.knInitialized, pipeline.initProc(true), "Can't initialize pipeline.");
            IFeatureCollection_SPtr spFeatureCollection = pyxlib.QueryInterface_IFeatureCollection(pipeline.getOutput());
            NUnit.Framework.Assert.IsNotNull(spFeatureCollection.get(), "Could not query interface for a feature collection.");

            // Iterate through all points in the set without restricting by geometry.
            FeatureIterator_SPtr spIter = spFeatureCollection.getIterator();
            int featureCount1 = 0;
            while (!spIter.end())
            {
                IFeature_SPtr spFeature = spIter.getFeature();
                NUnit.Framework.Assert.IsNotNull(spFeature.get(), "Returned feature was null.");
                ++featureCount1;
                spIter.next();
            }

            // Iterate through all points in the set for all tiles at res 2 with a depth of 11.
            PYXIcosIterator worldIter = new PYXIcosIterator(2);
            int featureCount2 = 0;
            while (!worldIter.end())
            {
                PYXTile_SPtr spTile = PYXTile.create(worldIter.getIndex(), 13);
                FeatureIterator_SPtr spGeometryIter = spFeatureCollection.getIterator(spTile.get());
                while (!spGeometryIter.end())
                {
                    IFeature_SPtr spFeature = spGeometryIter.getFeature();
                    NUnit.Framework.Assert.IsNotNull(spFeature.get(), "Returned feature was null.");
                    NUnit.Framework.Assert.Greater(spFeature.getDefinition().getFieldCount(), 0, "We should have some field data");
                    PYXGeometry_SPtr spGeometry = spFeature.getGeometry();
                    NUnit.Framework.Assert.IsNotNull(spGeometry.get(), "Returned geometry was null.");
                    ++featureCount2;
                    spGeometryIter.next();
                }
                worldIter.next();
            }

            NUnit.Framework.Assert.IsTrue(featureCount1 <= featureCount2, "There must be the same number or more features when done tile by tile.");
        }

        /// <summary>
        /// Time the loading of the World Political Boundaries data set.
        /// All tiles at res 2 with a depth of 11.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestRasterizingWorldPoliticalBoundaries()
        {
            string inputPPL = WellKnownDataDirectory + "World Political Boundaries\\Colourizer.ppl";
            if (!System.IO.File.Exists(inputPPL))
            {
                System.Console.WriteLine("TestWarning: could not open file " + inputPPL);
                return;
            }

            IProcess_SPtr pipeline = PipeManager.readPipelineFromFile(inputPPL);
            NUnit.Framework.Assert.AreEqual(IProcess.eInitStatus.knInitialized, pipeline.initProc(true), "Can't initialize pipeline.");
            ICoverage_SPtr coverage = pyxlib.QueryInterface_ICoverage(pipeline.getOutput());
            NUnit.Framework.Assert.IsNotNull(coverage.get(), "Could not query interface for a coverage.");

            PYXIcosIterator iter = new PYXIcosIterator(2);
            while (!iter.end())
            {
                PYXValueTile_SPtr spResult = coverage.getFieldTile(iter.getIndex(), 13, 0);
                iter.next();
            }
        }

        /// <summary>
        /// Time the loading of a point data set for 10,000 points.
        /// Iterate through all points in the set without restricting by geometry then,
        /// iterate through all points in the set for all tiles at res 2 with a depth of 11.
        /// </summary>
        [NUnit.Framework.Test]
        [NUnit.Framework.Ignore("Memory management issue causes this to crash at shut-down.")]
        public void TestTenThousandWellSites()
        {
            string inputPPL = WellKnownDataDirectory + "well heads\\wells.ppl";
            if (!System.IO.File.Exists(inputPPL))
            {
                System.Console.WriteLine("TestWarning: could not open file " + inputPPL);
                return;
            }

            IProcess_SPtr pipeline = PipeManager.readPipelineFromFile(inputPPL);
            NUnit.Framework.Assert.AreEqual(IProcess.eInitStatus.knInitialized, pipeline.initProc(true), "Can't initialize pipeline.");
            IFeatureCollection_SPtr spFeatureCollection = pyxlib.QueryInterface_IFeatureCollection(pipeline.getOutput());
            NUnit.Framework.Assert.IsNotNull(spFeatureCollection.get(), "Could not query interface for a feature collection.");

            // Iterate through all points in the set without restricting by geometry.
            FeatureIterator_SPtr spIter = spFeatureCollection.getIterator();
            int featureCount1 = 0;
            while (!spIter.end())
            {
                IFeature_SPtr spFeature = spIter.getFeature();
                NUnit.Framework.Assert.IsNotNull(spFeature.get(), "Returned feature was null.");
                ++featureCount1;
                spIter.next();
            }

            // Iterate through all points in the set for all tiles at res 2 with a depth of 11.
            PYXIcosIterator worldIter = new PYXIcosIterator(2);
            int featureCount2 = 0;
            while (!worldIter.end())
            {
                PYXTile_SPtr spTile = PYXTile.create(worldIter.getIndex(), 13);
                FeatureIterator_SPtr spGeometryIter = spFeatureCollection.getIterator(spTile.get());
                while (!spGeometryIter.end())
                {
                    IFeature_SPtr spFeature = spGeometryIter.getFeature();
                    NUnit.Framework.Assert.IsNotNull(spFeature.get(), "Returned feature was null.");
                    NUnit.Framework.Assert.Greater(spFeature.getDefinition().getFieldCount(), 0, "We should have some field data");
                    PYXGeometry_SPtr spGeometry = spFeature.getGeometry();
                    NUnit.Framework.Assert.IsNotNull(spGeometry.get(), "Returned geometry was null.");
                    ++featureCount2;
                    spGeometryIter.next();
                }
                worldIter.next();
            }

            // The number of features that are in the data set when you iterate over the whole data set should be
            // the same as the number of features you get back when you iterate over the data a tile at a time
            // for the whole world.
            NUnit.Framework.Assert.AreEqual(featureCount1, featureCount2, "Inconsistant data from the two types of iterators.");
        }

    }

    [NUnit.Framework.TestFixture]
    public class LSCertificateValidatorTests
    {
        private static LSCertificateValidator m_lsCertificateValidator = new LSCertificateValidator();
        private static readonly string s_trustedNodeGuid = "091b5336-9e65-45a3-9eac-a6b33e81e329";
        private static readonly string s_trustedKeyString = "\0\0\0¤\0\0RSA1\0\0\0\0\0q1SüÚ\b<»9—É„\b#¿z½ŽiUæ@ùBUøsTf†/´§Le¸êÖw6ÍÑ&gç@>ö%iÖ÷yzÍe‚'œúR8e$&¤q°“ùcÌÜëUÖ»€ùŸi‰ÄÚ¶I¼_\tI—\to*ü4\b,\vâ:Ïx½y‰Úûáê‚";

        static LSCertificateValidatorTests()
        {
            var trustedNode = new NodeId { Identity = new Guid(s_trustedNodeGuid), PublicKey = new DLM.PublicKey(Encoding.Default.GetBytes(s_trustedKeyString)) };
            m_lsCertificateValidator.SetTrustedAuthorities(new List<NodeId> { trustedNode });
        }

        [NUnit.Framework.Test]
        public void IsTrustedAuthority()
        {
            // service instance message for s_trustedNodeGuid authority
            var trustedServiceInstanceMessageString = "M4V5);C93&PEEGJ-%GJRFLSZ!XRF4````!@(```\"D``!24T$Q``0```$``0!Q\r\n"
            + "M,5/\\V@@%/+L?.9?)A`@COWJ]CFE5YD#Y0E7X<U0#9I\"&+[2G&$QEN.K6=S:/\r\n"
            + "MS=$F9^=`/@+V)6G6]WEZS66\"`R><^E(\"'#AE)\":D'G&PD_EC`<S<ZU76`[N`\r\n"
            + "M^9]IB<3:MDF\\7PE)EPEO%BK\\-`@L\"^(9.L\\%>+T&>8G:^^'J@JHNV(_8#>M%\r\n"
            + "HL=78KZ1O*G\\`````````````````````R9USA:6?,D6*=\\:9]2N-5P``\r\n";
            var serviceInstanceMessage = new Message() { SerializationString = trustedServiceInstanceMessageString };
            var serviceInstance = new PyxNet.Service.ServiceInstance();
            serviceInstance.FromMessage(serviceInstanceMessage);
            var certificate = new PyxNet.Service.Certificate(serviceInstance, DateTime.MaxValue, new PyxNet.Service.GeoSourcePermissionFact());

            var isTrusted = m_lsCertificateValidator.IsCertificateValid(certificate);

            NUnit.Framework.Assert.IsTrue(isTrusted);
        }

        [NUnit.Framework.Test]
        public void IsNotTrustedAuthority()
        {
            // service instance message for untrusted node
            var untrustedServiceInstanceMessageString = "M4V5);C93&PAEGJ-%GJRFLSZ!XRF4````!@(```\"D``!24T$Q``0```$``0!Q\r\n"
            + "M,5/\\V@@%/+L?.9?)A`@COWJ]CFE5YD#Y0E7X<U0#9I\"&+[2G&$QEN.K6=S:/\r\n"
            + "MS=$F9^=`/@+V)6G6]WEZS66\"`R><^E(\"'#AE)\":D'G&PD_EC`<S<ZU76`[N`\r\n"
            + "M^9]IB<3:MDF\\7PE)EPEO%BK\\-`@L\"^(9.L\\%>+T&>8G:^^'J@JHNV(_8#>M%\r\n"
            + "HL=78KZ1O*G\\`````````````````````R9USA:6?,D6*=\\:9]2N-5P``\r\n";
            var serviceInstanceMessage = new Message() { SerializationString = untrustedServiceInstanceMessageString };
            var serviceInstance = new PyxNet.Service.ServiceInstance();
            serviceInstance.FromMessage(serviceInstanceMessage);
            var certificate = new PyxNet.Service.Certificate(serviceInstance, DateTime.MaxValue, new PyxNet.Service.GeoSourcePermissionFact());

            var isTrusted = m_lsCertificateValidator.IsCertificateValid(certificate);

            NUnit.Framework.Assert.IsFalse(isTrusted);
        }
    }
}
