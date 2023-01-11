using NUnit.Framework;
using PyxNet.DataHandling;

namespace PyxNet.Test.DataHandling
{
    [TestFixture]
    public class DataInfoMessagesTests
    {
        private void Compare(DataInfo test, DataInfo reconstructed)
        {
            Assert.IsTrue(test.DataLength == reconstructed.DataLength,
                "The data length did not agree after reconstruction.");
            Assert.IsTrue(test.DataChunkSize == reconstructed.DataChunkSize,
                "The data chunk size did not agree after reconstruction.");
            Assert.IsTrue(test.UseEncryption == reconstructed.UseEncryption,
                "The encryption flag did not agree after reconstruction.");
            Assert.IsTrue(test.UseSigning == reconstructed.UseSigning,
                "The signing flag did not agree after reconstruction.");
            Assert.IsTrue(test.UsesHashCodes == reconstructed.UsesHashCodes,
                "The use hash codes flag did not agree after reconstruction.");
            Assert.IsTrue(test.AllAvailable == reconstructed.AllAvailable,
                "The all available flag did not agree after reconstruction.");
            // test the extra message section
            Assert.IsTrue(reconstructed.ExtraInfo.Equals(test.ExtraInfo), "The extra info message did not transfer properly.");
        }

        /// <summary>
        /// Test creating a DataInfo and move to and from a message.
        /// </summary>
        [Test]
        public void TestDataInfoConstruction()
        {
            DataInfo test = new DataInfo();

            test.DataLength = 100000;
            test.DataChunkSize = 32768;
            test.UseEncryption = false;
            test.UseSigning = false;
            test.UsesHashCodes = false;
            test.AllAvailable = true;

            Message aMessage = test.ToMessage();
            test.ExtraInfo = aMessage;

            DataInfo reconstructed = new DataInfo(test.ToMessage());
            Compare(test, reconstructed);

            DataInfo copied = new DataInfo(test);
            Compare(test, copied);
        }

        /// <summary>
        /// Test creating a DataInfoRequest and move to and from a message.
        /// </summary>
        [Test]
        public void TestDataInfoRequestConstruction()
        {
            DataInfoRequest test = new DataInfoRequest(new DataGuid());
            Message aMessage = test.ToMessage();
            DataInfoRequest reconstructed = new DataInfoRequest(aMessage);

            Assert.IsTrue(test.DataSetID == reconstructed.DataSetID,
                "The DataGuid was not the same after copy.");

            // test the extra message section
            test.ExtraInfo = aMessage;
            Message bMessage = test.ToMessage();
            reconstructed = new DataInfoRequest(bMessage);

            Assert.IsTrue(reconstructed.ExtraInfo.Equals(aMessage), "The extra info message did not transfer properly.");
        }

        /// <summary>
        /// Test creating a DataNoInfo and move to and from a message.
        /// </summary>
        [Test]
        public void TestDataNoInfoConstruction()
        {
            DataNoInfo test = new DataNoInfo(new DataGuid());
            Message aMessage = test.ToMessage();
            DataNoInfo reconstructed = new DataNoInfo(aMessage);

            Assert.IsTrue(test.DataSetID == reconstructed.DataSetID,
                "The DataGuid was not the same after copy.");

            // test the extra message section
            test.ExtraInfo = aMessage;
            Message bMessage = test.ToMessage();
            reconstructed = new DataNoInfo(bMessage);

            Assert.IsTrue(reconstructed.ExtraInfo.Equals(aMessage), "The extra info message did not transfer properly.");
        }
    }
}