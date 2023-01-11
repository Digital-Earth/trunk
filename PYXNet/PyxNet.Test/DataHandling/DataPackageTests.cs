using NUnit.Framework;
using PyxNet.DataHandling;

namespace PyxNet.Test.DataHandling
{
    [TestFixture]
    public class DataPackageTests
    {
        public void TestDataPackage(DataPackage package)
        {
            Assert.IsTrue(package.ValidCheckSum, "Bad checksum.");

            // Add it to a message.
            Message aMessage = new Message("XXXX");
            package.ToMessage(aMessage);

            // Get it back from the message.
            MessageReader aReader = new MessageReader(aMessage);
            DataPackage reconstructedPackage = new DataPackage(aReader);

            //Check that we recontruct from a message and get back the same thing.
            Assert.IsTrue(reconstructedPackage.ValidCheckSum, "Bad checksum after reconstruction.");
            Assert.IsTrue(package.Data.Length == reconstructedPackage.Data.Length, "Not the same amount of data.");
            for (int index = 0; index < package.Data.Length; ++index)
            {
                Assert.IsTrue(package.Data[index] == reconstructedPackage.Data[index],
                    "Data was not reconstructed.");
            }
            Assert.IsTrue(package.UseCompression == reconstructedPackage.UseCompression, "UseCompression not the same.");
            Assert.IsTrue(package.DoCheckSum == reconstructedPackage.DoCheckSum, "DoCheckSum not the same.");
        }

        public void TestDataPackage(byte[] byteSource)
        {
            DataPackage p1 = new DataPackage(byteSource);
            TestDataPackage(p1);
            DataPackage p2 = new DataPackage(byteSource, true, true);
            TestDataPackage(p2);
            DataPackage p3 = new DataPackage(byteSource, true, false);
            TestDataPackage(p3);
            DataPackage p4 = new DataPackage(byteSource, false, true);
            TestDataPackage(p4);
            DataPackage p5 = new DataPackage(byteSource, false, false);
            TestDataPackage(p5);
        }

        /// <summary>
        /// Test construction and moving to/from a message.
        /// </summary>
        [Test]
        public void TestDataPackage()
        {
            byte[] smallByteArray = { 10, 4, 8, 7, 6, 9, 5, 3, 2, 1, 0, 11 };
            TestDataPackage(smallByteArray);

            const int bigTestSize = 10000;
            byte[] largeByteArray = new byte[bigTestSize];
            for (int index = 0; index < bigTestSize; ++index)
            {
                largeByteArray[index] = (byte)(65 + index % 26);
            }
            TestDataPackage(largeByteArray);
        }
    }
}