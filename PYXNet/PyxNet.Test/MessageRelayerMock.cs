using System;
using System.Collections.Generic;

namespace PyxNet.Test
{
    /// <summary>
    /// This class encapsulates a MessageRelayer class test.
    /// </summary>
    class MessageRelayerMock
    {
        const string m_testMessageID = "FUNK";

        public readonly Pyxis.Utilities.TraceTool Tracer = new Pyxis.Utilities.TraceTool(false, "MessageRelayer Test: ");

        /// <summary>
        /// The list of stacks to test.
        /// </summary>
        private readonly List<Stack> m_stacks = null;

        /// <summary>
        /// The message relayers run during tests.
        /// </summary>
        private readonly List<MessageRelayer> m_relayers = new List<MessageRelayer>();

        public MessageRelayerMock(List<Stack> stacks)
        {
            m_stacks = stacks;
        }

        public void Run()
        {
            m_stacks.ForEach(delegate(Stack stack)
            {
                TestFromStack(stack);
            });
        }

        private void AttachTestStacks()
        {
            // Set events for each stack.
            m_stacks.ForEach(delegate(Stack receivingStack)
            {
                receivingStack.RegisterHandler(MessageRelay.MessageID, HandleMessageRelay);
                receivingStack.RegisterHandler(m_testMessageID, HandleRelayedMessage);
            });
        }

        private void DetachTestStacks()
        {
            // Unhook events from each stack.
            m_stacks.ForEach(delegate(Stack receivingStack)
            {
                receivingStack.UnregisterHandler(MessageRelay.MessageID, HandleMessageRelay);
                receivingStack.UnregisterHandler(m_testMessageID, HandleRelayedMessage);
            });
        }

        /// <summary>
        /// Run queries on stack as part of the test.
        /// </summary>
        /// <param name="stack">The stack to run query tests on.</param>
        private void TestFromStack(Stack stack)
        {
            Tracer.DebugWriteLine("Starting from {0}", stack.NodeInfo.FriendlyName);

            int stackCount = m_stacks.Count;

            // Set events for each stack.
            AttachTestStacks();

            m_stacks.ForEach(delegate(Stack destinationStack)
            {
                // Create a message to relay.
                Message messageToRelay = new Message(m_testMessageID);

                // Append to the message the node info of the stack we're starting on.
                stack.NodeInfo.ToMessage(messageToRelay);

                // Create the message relay, destined for the destination stack.
                MessageRelay relay = new MessageRelay(messageToRelay, destinationStack.NodeInfo.NodeGUID);

                MessageRelayer relayer = new MessageRelayer(stack, relay, 1000);

                // Add the relayer to the relayers list.
                lock (m_relayers)
                {
                    m_relayers.Add(relayer);
                }

                // Start your engines.
                relayer.Start();

                // Wait for the test to finish, or fail after a certain amount of time.
                if (!WaitForTestFromStackToFinish(relayer))
                {
                    relayer.Stop();

                    NUnit.Framework.Assert.Fail(String.Format(
                        "{0} did not get relay message from {1}.  Log saved as '{2}'.",
                        destinationStack.NodeInfo.FriendlyName, stack.NodeInfo.FriendlyName, Pyxis.Utilities.TraceTool.SaveToTempFile()));
                }
            });

            int messagesNotRelayed;
            lock (m_relayers)
            {
                messagesNotRelayed = m_relayers.Count;

                m_relayers.ForEach(delegate(MessageRelayer relayer)
                {
                    Tracer.DebugWriteLine("Couldn't relay {0}", relayer.Relay.ToNodeGuid);
                });
            }
            if (0 < messagesNotRelayed)
            {
                NUnit.Framework.Assert.Fail(
                    messagesNotRelayed.ToString() + " messages were not relayed, starting on stack " +
                    stack.NodeInfo.FriendlyName);
            }

            // Unhook events from each stack.
            DetachTestStacks();
        }

        /// <summary>
        /// Wait for the test to finish, or time out.
        /// </summary>
        private bool WaitForTestFromStackToFinish(MessageRelayer relayer)
        {
            // If the relayer is not in the list, it has finished.
            DateTime startTime = DateTime.Now;
            for (; ; )
            {
                lock (m_relayers)
                {
                    if (!m_relayers.Contains(relayer))
                    {
                        break;
                    }
                }

                // Wait for a moment.
                System.Threading.Thread.Sleep(2000);
                TimeSpan elapsedTime = DateTime.Now - startTime;

                // TODO: Expose this timeout as a tunable parameter.
                if (elapsedTime.TotalSeconds >= 20)
                {
                    return false;
                }
            }
            return true;
        }

        /// <summary>
        /// This is called when a node receives a message relay.
        /// </summary>
        private void HandleMessageRelay(object sender, MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            Tracer.DebugWriteLine("Got a relay on connection {0}", args.Context.Sender);
        }

        /// <summary>
        /// This is called when the relayed message reaches its intended target.
        /// </summary>
        private void HandleRelayedMessage(object sender, MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            Message relayedMessage = args.Message;
            Tracer.DebugWriteLine("Got a relayed message {0} on connection {1}", relayedMessage.Identifier, args.Context.Sender);
            NUnit.Framework.Assert.AreEqual(relayedMessage.Identifier, m_testMessageID);

            MessageReader reader = new MessageReader(relayedMessage);
            NodeInfo originNode = new NodeInfo(reader);

            int relayersFound;
            lock (m_relayers)
            {
                relayersFound = m_relayers.RemoveAll(delegate(MessageRelayer element)
                {
                    if (element.Stack.NodeInfo.Equals(originNode))
                    {
                        element.Stop();
                        return true;
                    }
                    return false;
                });
            }
            NUnit.Framework.Assert.IsTrue(1 == relayersFound,
                String.Format("Relayer from '{0}' was found {1} time(s).", originNode.FriendlyName, relayersFound));
        }
    }
}