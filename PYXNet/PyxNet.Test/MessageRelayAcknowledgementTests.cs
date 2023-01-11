using System.Collections.Generic;

namespace PyxNet.Test
{
    /// <summary>
    /// Test class for relay acknowledgement class.
    /// </summary>
    [NUnit.Framework.TestFixture]
    public class MessageRelayAcknowledgementTests
    {
        private readonly System.Guid m_relayGuid = new System.Guid("0FFBA677-B35F-4df7-806F-FFE9AA64DB7A");
        private readonly System.Guid m_toNodeGuid = new System.Guid("8D1F2EC5-E86F-438a-9B9D-EB5EC8568753");

        private MessageRelayAcknowledgement CreateMessageRelayAcknowledgement()
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

            // Create the acknowledgement object.
            MessageRelayAcknowledgement acknowledgement = new MessageRelayAcknowledgement(
                m_relayGuid, m_toNodeGuid, visitedHubs, candidateHubs);

            NUnit.Framework.Assert.IsTrue(acknowledgement.CandidateHubs.Count == 2);

            return acknowledgement;
        }

        /// <summary>
        /// Test construction and moving to/from a message.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestMessageRelayAcknowledgement()
        {
            // Create an acknowledgement object.
            MessageRelayAcknowledgement acknowledgement = CreateMessageRelayAcknowledgement();

            // Convert it to a message.
            Message message = acknowledgement.ToMessage();

            // Construct a new one from the message.
            MessageRelayAcknowledgement reconstructedAcknowledgement = MessageRelayAcknowledgement.FromMessage(message);

            // Ensure that the new one contains the same information as the old one.
            NUnit.Framework.Assert.IsTrue(acknowledgement.RelayGuid.Equals(m_relayGuid));
            NUnit.Framework.Assert.IsTrue(reconstructedAcknowledgement.RelayGuid.Equals(m_relayGuid));
            NUnit.Framework.Assert.IsTrue(acknowledgement.ToNodeGuid.Equals(m_toNodeGuid));
            NUnit.Framework.Assert.IsTrue(reconstructedAcknowledgement.ToNodeGuid.Equals(m_toNodeGuid));

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