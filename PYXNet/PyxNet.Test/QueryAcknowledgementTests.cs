using System.Collections.Generic;

namespace PyxNet.Test
{
    /// <summary>
    /// Test class for query acknowledgement class.
    /// </summary>
    [NUnit.Framework.TestFixture]
    public class QueryAcknowledgementTests
    {
        private QueryAcknowledgement CreateQueryAcknowledgement()
        {
            List<NodeInfo> visitedHubs = new List<NodeInfo>();
            List<NodeInfo> candidateHubs = new List<NodeInfo>();

            NodeInfo info1 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 12, 150);
            visitedHubs.Add(info1);

            NodeInfo info2 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 14, 160);
            visitedHubs.Add(info2);

            NodeInfo info3 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 16, 170);
            visitedHubs.Add(info3);

            NodeInfo info4 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 18, 180);
            candidateHubs.Add(info4);

            NodeInfo info5 = NodeInfoHelper.CreateNodeInfo(
                NodeInfo.OperatingMode.Hub, 20, 190);
            candidateHubs.Add(info5);

            // Create the query acknowledgement object.
            QueryAcknowledgement acknowledgement = new QueryAcknowledgement(
                new System.Guid("8D1F2EC5-E86F-438a-9B9D-EB5EC8568753"),
                visitedHubs, candidateHubs);

            acknowledgement.IsDeadEnd = true;

            NUnit.Framework.Assert.IsTrue(acknowledgement.VisitedHubs.Count == 3);
            NUnit.Framework.Assert.IsTrue(acknowledgement.CandidateHubs.Count == 2);

            return acknowledgement;
        }

        /// <summary>
        /// Test construction and moving to/from a message.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestQueryAcknowledgement()
        {
            // Create a query acknowledgement object.
            QueryAcknowledgement acknowledgement = CreateQueryAcknowledgement();

            // Convert it to a message.
            Message message = acknowledgement.ToMessage();

            // Construct a new one from the message.
            QueryAcknowledgement reconstructedAcknowledgement = QueryAcknowledgement.FromMessage(message);

            // Ensure that the new one contains the same information as the old one.
            NUnit.Framework.Assert.IsTrue(acknowledgement.QueryGuid == reconstructedAcknowledgement.QueryGuid);

            // Check the original against the reconstructed.
            NUnit.Framework.Assert.IsTrue(
                acknowledgement.VisitedHubs.Count == reconstructedAcknowledgement.VisitedHubs.Count);
            for (int count = 0; count < acknowledgement.VisitedHubs.Count; ++count)
            {
                NUnit.Framework.Assert.IsTrue(
                    acknowledgement.VisitedHubs[count].LeafCount ==
                    reconstructedAcknowledgement.VisitedHubs[count].LeafCount);
                NUnit.Framework.Assert.IsTrue(
                    acknowledgement.VisitedHubs[count].HubCount ==
                    reconstructedAcknowledgement.VisitedHubs[count].HubCount);
            }
            NUnit.Framework.Assert.IsTrue(
                acknowledgement.CandidateHubs.Count == reconstructedAcknowledgement.CandidateHubs.Count);
            for (int count = 0; count < acknowledgement.CandidateHubs.Count; ++count)
            {
                NUnit.Framework.Assert.IsTrue(
                    acknowledgement.CandidateHubs[count].Mode ==
                    reconstructedAcknowledgement.CandidateHubs[count].Mode);
                NUnit.Framework.Assert.IsTrue(
                    acknowledgement.CandidateHubs[count].NodeGUID ==
                    reconstructedAcknowledgement.CandidateHubs[count].NodeGUID);
            }
            NUnit.Framework.Assert.IsTrue(
                acknowledgement.IsDeadEnd == reconstructedAcknowledgement.IsDeadEnd);
        }
    }
}