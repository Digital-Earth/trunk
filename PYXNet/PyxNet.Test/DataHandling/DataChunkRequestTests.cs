using NUnit.Framework;
using PyxNet.DataHandling;

namespace PyxNet.Test.DataHandling
{
    [TestFixture]
    public class DataChunkRequestTests
    {
        /// <summary>
        /// Test creating a DataChunkRequest and move to and from a message.
        /// </summary>
        [Test]
        public void TestDataChunkRequestConstruction()
        {
            DataChunkRequest testChunkRequest = new DataChunkRequest(new DataGuid(), 0, 10000, false, true, null);

            Message aMessage = testChunkRequest.ToMessage();
            DataChunkRequest reconstructedChunkRequest = new DataChunkRequest(aMessage);

            Assert.AreEqual(reconstructedChunkRequest.Encrypt, testChunkRequest.Encrypt, 
                "The encryption flag did not agree after reconstruction.");
            Assert.AreEqual(reconstructedChunkRequest.Sign, testChunkRequest.Sign, 
                "The signing flag did not agree after reconstruction.");
            Assert.AreEqual(reconstructedChunkRequest.ChunkSize, testChunkRequest.ChunkSize,
                "Improper data length after reconstruction.");
            Assert.AreEqual(reconstructedChunkRequest.Offset, testChunkRequest.Offset,
                "Improper data offset after reconstruction.");
            Assert.IsTrue(reconstructedChunkRequest.DataSetID.Equals(testChunkRequest.DataSetID),
                "The Data Set ID did not transfer properly.");
            Assert.AreEqual(reconstructedChunkRequest.Certificate, testChunkRequest.Certificate,
                "Reconstructed certificate doesn't match.");

            // test the extra message section
            testChunkRequest.ExtraInfo = aMessage;
            Message bMessage = testChunkRequest.ToMessage();
            reconstructedChunkRequest = new DataChunkRequest(bMessage);

            Assert.IsTrue(reconstructedChunkRequest.ExtraInfo.Equals(aMessage), "The extra info message did not transfer properly.");

            // test the certificate section
            testChunkRequest.Certificate = Service.CertificateTests.CreateCertificate( 120);
            Message cMessage = testChunkRequest.ToMessage();
            reconstructedChunkRequest = new DataChunkRequest(cMessage);

            Assert.AreEqual(reconstructedChunkRequest.Certificate, testChunkRequest.Certificate,
                "Reconstructed certificate doesn't match.");
        }
    }
}