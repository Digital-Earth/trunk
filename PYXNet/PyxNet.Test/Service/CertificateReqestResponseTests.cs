using System;
using NUnit.Framework;
using PyxNet.Service;

namespace PyxNet.Test.Service
{
    /// <summary>
    /// Testing for the class.
    /// </summary>
    [TestFixture]
    public class CertificateReqestResponseTests
    {
        /// <summary>
        /// Create an object to test with.
        /// </summary>
        /// <returns>The new object.</returns>
        private CertificateRequestResponse CreateCertificateReqestResponse()
        {
            String url = "http://pyxisinnovation.com";
            bool permissionGranted = true;
            Guid requestGuid = Guid.NewGuid();

            CertificateRequestResponse instance = new CertificateRequestResponse(
                requestGuid, permissionGranted, url);
            Assert.AreEqual(requestGuid, instance.Id);
            Assert.AreEqual(permissionGranted, instance.PermissionGranted);
            Assert.AreEqual(url, instance.Url);
            return instance;
        }

        /// <summary>
        /// Test construction and moving to/from a message.
        /// </summary>
        [Test]
        public void TestPublishRequest()
        {
            // Create the orignal object.
            CertificateRequestResponse original = CreateCertificateReqestResponse();

            // Convert it to a message.
            Message message = original.ToMessage();

            // Reconstruct from the message.
            CertificateRequestResponse reconstructed = CertificateRequestResponse.FromMessage(message);

            // Ensure that the new one contains the same information as the old one.
            Assert.AreEqual(original.Id, reconstructed.Id);
            Assert.AreEqual(original.PermissionGranted, reconstructed.PermissionGranted);
            Assert.AreEqual(original.Url, reconstructed.Url);

            // TODO: Test serialization of Remappings, Certificates.            
        }
    }
}