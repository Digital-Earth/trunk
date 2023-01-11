using NUnit.Framework;

namespace PyxNet.Test.DataHandling
{
    [TestFixture]
    public class DataGuidTests
    {
        /// <summary>
        /// Test creating a DataGuid and move to and from a message.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestDataGuidConstruction()
        {
            PyxNet.DataHandling.DataGuid test = new PyxNet.DataHandling.DataGuid();

            Message aMessage = new Message("XXXX");
            test.ToMessage(aMessage);
            MessageReader aReader = new MessageReader(aMessage);
            PyxNet.DataHandling.DataGuid reconstructed = new PyxNet.DataHandling.DataGuid(aReader);

            NUnit.Framework.Assert.IsTrue(test == reconstructed,
                "The DataGuid was not the same after reconstruction.");
        }

        /// <summary>
        /// Test the copy constructor for DataGuid.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestDataGuidCopyConstruction()
        {
            PyxNet.DataHandling.DataGuid test = new PyxNet.DataHandling.DataGuid();
            PyxNet.DataHandling.DataGuid reconstructed = new PyxNet.DataHandling.DataGuid(test);

            NUnit.Framework.Assert.IsTrue(test == reconstructed,
                "The DataGuid was not the same after copy.");
        }
    }
}