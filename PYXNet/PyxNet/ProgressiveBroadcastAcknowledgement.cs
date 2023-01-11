/******************************************************************************
ProgressiveBroadcastAcknowledgement.cs

begin      : April 16, 2007
copyright  : (c) 2010 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;

namespace PyxNet
{
    /// <summary>
    /// An acknowledgement for a progressive broadcast.
    /// </summary>
    public abstract class ProgressiveBroadcastAcknowledgement : ITransmissible
    {
        #region Fields and properties

        /// <summary>
        /// The hubs which were visited by this broadcast.
        /// </summary>
        private readonly List<NodeInfo> m_visitedHubs;

        /// <summary>
        /// The hubs which were visited by this broadcast.
        /// </summary>
        public List<NodeInfo> VisitedHubs
        {
            get
            {
                return m_visitedHubs;
            }
        }

        /// <summary>
        /// The hubs that are candidates to further this broadcast.
        /// </summary>
        private readonly List<NodeInfo> m_candidateHubs;

        /// <summary>
        /// The hubs that are candidates to further this broadcast.
        /// </summary>
        public List<NodeInfo> CandidateHubs
        {
            get
            {
                return m_candidateHubs;
            }
        }

        /// <summary>
        /// If this is true, the node is a dead end for the broadcast.
        /// </summary>
        private bool m_isDeadEnd = false;

        /// <summary>
        /// If this is true, the node is a dead end for the broadcast.
        /// </summary>
        public bool IsDeadEnd
        {
            get
            {
                return m_isDeadEnd;
            }
            set
            {
                m_isDeadEnd = value;
            }
        }

        #endregion

        #region Constructors

        /// <summary>
        /// Default constructor.
        /// </summary>
        /// <param name="visitedHubs">The hubs already visited.</param>
        /// <param name="candidateHubs">Some hubs that haven't been visited.</param>
        public ProgressiveBroadcastAcknowledgement(
            List<NodeInfo> visitedHubs, List<NodeInfo> candidateHubs)
        {
            if (null == visitedHubs)
            {
                throw new ArgumentNullException("visitedHubs");
            }
            if (null == candidateHubs)
            {
                throw new ArgumentNullException("candidateHubs");
            }

#if DEBUG
            // Guarantee no duplicates between two lists.
            // No candidates should be in visitedHubs.
            visitedHubs.ForEach(delegate(NodeInfo visitedHub)
            {
                System.Diagnostics.Debug.Assert(!candidateHubs.Contains(visitedHub),
                    "A visited hub list cannot contain anything in a candidate hub list.");
            });
#endif

            // Set the two list fields.
            m_visitedHubs = visitedHubs;
            m_candidateHubs = candidateHubs;
        }

        /// <summary>
        /// Construct from a message.
        /// </summary>
        /// <param name="reader">
        /// The message reader to construct from.
        /// This doesn't include the message header.
        /// </param>
        public ProgressiveBroadcastAcknowledgement(MessageReader reader)
        {
            // Read in the visited hubs.
            int visitedHubCount = reader.ExtractInt();
            m_visitedHubs = new List<NodeInfo>(visitedHubCount);
            for (int count = 0; count < visitedHubCount; ++count)
            {
                m_visitedHubs.Add(new NodeInfo(reader));
            }

            // Read in the candidate hubs.
            int candidateHubCount = reader.ExtractInt();
            m_candidateHubs = new List<NodeInfo>(candidateHubCount);
            for (int count = 0; count < candidateHubCount; ++count)
            {
                m_candidateHubs.Add(new NodeInfo(reader));
            }

            // Read in the "is dead end" flag.
            m_isDeadEnd = reader.ExtractBool();
        }

        #endregion

        #region Convert to message

        /// <summary>
        /// Build a message that contains the acknowledgement.
        /// </summary>
        /// <returns>The resulting message.</returns>
        public abstract Message ToMessage();

        /// <summary>
        /// Append the query acknowledgement to an existing message.
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to (will be modified).</param>
        /// <returns></returns>
        public virtual void ToMessage(Message message)
        {
            if (null == message)
            {
                throw new System.ArgumentNullException("message");
            }

            // Append the visited hubs.
            message.Append(VisitedHubs.Count);
            VisitedHubs.ForEach(delegate(NodeInfo visitedHub)
            {
                visitedHub.ToMessage(message);
            });

            // Append the candidate hubs.
            message.Append(CandidateHubs.Count);
            CandidateHubs.ForEach(delegate(NodeInfo candidateHub)
            {
                candidateHub.ToMessage(message);
            });

            // Read in the "is dead end" flag.
            message.Append(IsDeadEnd);
        }

        #endregion
    }
}
