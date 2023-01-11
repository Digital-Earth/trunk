using System;
using NUnit.Framework;
using PyxNet.FileTransfer;
using PyxNet.Service;
using PyxNet.Test.Service;

namespace PyxNet.Test.FileTransfer
{
    /// <summary>
    /// Unit tests for ManifestManager
    /// </summary>
    [TestFixture]
    public class ManifestManagerTests
    {
        [Test]
        public void Create()
        {
            ManifestManager myManifestManager = null;
            try
            {
                myManifestManager = new ManifestManager("Test");
            }
            finally
            {
                if (myManifestManager != null)
                {
                    myManifestManager.DeleteAllEntries();
                }
            }
        }

        [Test]
        public void ReadEmpty()
        {
            ManifestManager myManifestManager = null;
            try
            {
                myManifestManager = new ManifestManager("Test");
                ResourceInstanceFact result = myManifestManager["ATestApplication.Manifest"];
                Assert.IsNull(result, "Reading a non-existent key should return null.");
            }
            finally
            {
                if (myManifestManager != null)
                {
                    myManifestManager.DeleteAllEntries();
                }
            }
        }

        [Test]
        public void WriteResourceInstanceFact()
        {
            ManifestManager myManifestManager = null;
            try
            {
                myManifestManager = new ManifestManager("Test");
                ResourceInstanceFact testValue = new ResourceInstanceFact();
                myManifestManager["ATestApplication.Manifest"] = testValue;

                ResourceInstanceFact result = myManifestManager["ATestApplication.Manifest"];

                // TODO: Use ResourceInstanceFact.Equals() when it is properly implemented.
                Assert.IsTrue(result != null && testValue.Id.Equals(result.Id), "The returned resource instance fact did not match.");
            }
            finally
            {
                if (myManifestManager != null)
                {
                    myManifestManager.DeleteAllEntries();
                }
            }
        }

        [Test]
        public void WriteCertificate()
        {
            ManifestManager myManifestManager = null;
            try
            {
                myManifestManager = new ManifestManager("Test");
                ResourceInstanceFact testValue = 
                    ResourceInstanceFactTests.CreateFakeResourceInstanceFact();
                PyxNet.DLM.RSATool testAuthorityKey = new PyxNet.DLM.RSATool();
                ServiceInstance testAuthority =
                    ServiceInstanceTests.CreateRandomServiceInstance(testAuthorityKey);
                Certificate testCertificate = new Certificate(
                    testAuthority, DateTime.Now + TimeSpan.FromMinutes(2), testValue);
                testCertificate.SignCertificate(testAuthorityKey.GetPrivateKey());
                Assert.IsInstanceOf<ResourceInstanceFact>(testCertificate.FactList[0], "Error building certificate.");
                myManifestManager["ATestApplication.Manifest"] =
                    testCertificate.FactList[0] as ResourceInstanceFact;

                ResourceInstanceFact result = myManifestManager["ATestApplication.Manifest"];

                // TODO: Use ResourceInstanceFact.Equals() when it is properly implemented.
                Assert.IsTrue(result != null && testValue.Id.Equals(result.Id), "The returned resource instance fact did not match.");
                Assert.IsNotNull(result.Certificate);
                Assert.AreEqual(result.Certificate.IssuedTime, testCertificate.IssuedTime);
            }
            finally
            {
                if (myManifestManager != null)
                {
                    myManifestManager.DeleteAllEntries();
                }
            }
        }
    }
}