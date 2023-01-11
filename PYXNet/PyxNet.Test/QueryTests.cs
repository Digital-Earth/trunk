namespace PyxNet.Test
{
    /// <summary>
    /// Test class for Query class.
    /// </summary>
    [NUnit.Framework.TestFixture]
    public class QueryTests
    {
        /// <summary>
        /// Test construction and moving to/from a message.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestQuery()
        {
            // Create a stack object.
            NodeInfo info = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 10, 972);
            info.Address = new NetworkAddress(new System.Net.IPEndPoint(System.Net.IPAddress.Parse("127.0.0.1"), 37037));
            info.FriendlyName = "Your Friendly Neighbourhood Hub";

            // Create a query object.
            Query query = new Query(
                info,
                "What are hot dog weiners made from?");
            query = query.GetHoppedQuery();

            // Convert it to a message.
            Message subMessage = query.ToMessage();

            // add in itself as a qualifier message
            query.QueryQualifiers.Add(subMessage);

            // Convert it to a message.
            Message aMessage = query.ToMessage();

            // Construct a new one from the message.
            Query reconstructedQuery = new Query(aMessage);

            // Ensure that the new one contains the same information as the old one.
            NUnit.Framework.Assert.IsTrue(query.OriginNode.Equals(reconstructedQuery.OriginNode));
            NUnit.Framework.Assert.AreEqual(query.Guid, reconstructedQuery.Guid);
            NUnit.Framework.Assert.AreEqual(query.HopCount, reconstructedQuery.HopCount);
            NUnit.Framework.Assert.AreEqual(query.Contents, reconstructedQuery.Contents);
            NUnit.Framework.Assert.IsTrue(query.QueryQualifiers[0].CompareTo(subMessage) == 0);
            NUnit.Framework.Assert.IsTrue(reconstructedQuery.QueryQualifiers[0].CompareTo(subMessage) == 0);
        }
    }
}