using NUnit.Framework;

namespace PyxNet.Test
{
    /// <summary>
    /// Unit tests for XPathQuery
    /// </summary>
    [TestFixture]
    public class XPathQueryTests
    {
        [Test]
        public void Serialization()
        {
            NodeInfo node = Test.NodeInfoHelper.CreateNodeInfo(NodeInfo.OperatingMode.Leaf, 0, 0);
            XPathQuery myXPathQuery = new XPathQuery( node, "test/*/query");
            Message message = myXPathQuery.ToMessage();
            XPathQuery extractedXPathQuery = new XPathQuery( message);

            Assert.AreEqual(myXPathQuery.XPathExpression,
                extractedXPathQuery.XPathExpression);
        }
    }
}