using System;
using System.Collections.Generic;
using Pyxis.Utilities;

namespace PyxNet.Test
{
    /// <summary>
    /// A class to contain helper functions which will be used by many of the
    /// tests.  
    /// </summary>
    public class StackTestHelper : IDisposable
    {
        public readonly Pyxis.Utilities.TraceTool Tracer = new Pyxis.Utilities.TraceTool(false, "Stack Test: ");

        /// <summary>
        /// The number of milliseconds between messages.
        /// </summary>
        private int m_interval;

        /// <summary>
        /// The list of stacks to test.
        /// </summary>
        private readonly List<Stack> m_stacks;

        /// <summary>
        /// The list of stacks to test with.
        /// </summary>
        public List<Stack> Stacks
        {
            get { return m_stacks; }
        }

        /// <summary>
        /// Keep track of the number of connection calls that we
        /// make when buidling a topology so that we can make sure
        /// that all connections are in effect before the test proceeds.
        /// </summary>
        private int m_numberOfConnectionsMade;

        /// <summary>
        /// Topologies to test.
        /// </summary>
        public enum Topology
        {
            One = 1,
            Two = 2,
            Three = 3,
            Four = 4
        }

        /// <summary>
        /// Construct a stack test.
        /// </summary>
        /// <param name="topology">The topology to test.</param>
        /// <param name="lniMaxTime">The milliseconds between messages.</param>
        public StackTestHelper(Topology topology, int interval)
        {
            switch (topology)
            {
                case Topology.One:
                    m_stacks = CreateTopologyOneStacks(interval);
                    m_numberOfConnectionsMade = 6;
                    break;
                case Topology.Two:
                    m_stacks = CreateTopologyTwoStacks(interval);
                    m_numberOfConnectionsMade = 10;
                    break;
                case Topology.Three:
                    m_stacks = CreateTopologyThreeStacks(interval);
                    m_numberOfConnectionsMade = 14;
                    break;
                case Topology.Four:
                    m_stacks = CreateTopologyFourStacks(interval);
                    m_numberOfConnectionsMade = 30;
                    break;
                default:
                    throw new System.ArgumentException("Invalid topology.");
            }

            if (interval == -1)
            {
                m_interval = (int)Stack.DefaultPingTimerInterval;
            }
            else
            {
                m_interval = interval;
            }

            // Confirm the connections.
            ConfirmConnections();
        }

        public void TestConnectionHandshake()
        {
            Stack outgoing = m_stacks[1];
            Stack incoming = m_stacks[m_stacks.Count - 1];

            // TODO: this locic is slightly broken.  Just because we say it is a hub on this end 
            // shouldn't mean that it is a hub!

            // Get the info of the first stack.
            NodeInfo info = new NodeInfo();
            info.IsHub = true;  // So that we can make sure it adds to the known hub list.
            info.Address = incoming.NodeInfo.Address;
            info.NodeGUID = incoming.NodeInfo.NodeGUID;

            StackConnection temporaryConnection = outgoing.FindConnection(info, false);
            NUnit.Framework.Assert.IsNull(temporaryConnection,
                "The connection already exists and shouldn't exist.");

            // Connect temporarily and verify the connection.
            try
            {
                temporaryConnection = outgoing.GetConnection(info, false, TimeSpan.FromSeconds(5));
                NUnit.Framework.Assert.IsNotNull(temporaryConnection,
                    "Unable to create a connection within 5 seconds.");
                NUnit.Framework.Assert.IsTrue(temporaryConnection == outgoing.FindConnection(info, false),
                    "We found a different connection than we expected.  Do we have two connections?");
            }
            catch (Stack.ConnectionException e)
            {
                NUnit.Framework.Assert.Fail(String.Format("Exception {0} when connecting to {1}.",
                    e.ErrorType, info));
            }

            // Change the guid to the incorrect one.
            info.NodeGUID = Guid.NewGuid();

            // Add this to the known hubs.
            outgoing.KnownHubList.Add(info);
            NUnit.Framework.Assert.IsTrue(outgoing.KnownHubList.KnownHubs.Contains(info),
                "The new node was not added to the known hub list.");
            NUnit.Framework.Assert.IsFalse(outgoing.KnownHubList.ConnectedHubs.Contains(info),
                "Somehow the bad node was added to the connected hub list.");

            // Create the connection and see that the connection is not there and an error was returned.
            try
            {
                temporaryConnection = outgoing.CreateConnection(info, false);
                NUnit.Framework.Assert.Fail("The connection to the wrong node was created.");
            }
            catch (Stack.ConnectionException e)
            {
                NUnit.Framework.Assert.AreEqual(StackConnectionResponse.ErrorType.IncorrectNode, e.ErrorType);
                NUnit.Framework.Assert.IsNull(outgoing.FindConnection(info, false),
                    "The connection exists in a non-pending list.");
            }

            // Ensure that the incorrect node is no longer in the known hub list.
            NUnit.Framework.Assert.IsFalse(outgoing.KnownHubList.KnownHubs.Contains(info),
                "The bad node was not removed from the known hub list.");
            NUnit.Framework.Assert.IsFalse(outgoing.KnownHubList.ConnectedHubs.Contains(info),
                "Somehow the bad node ended up in the connected hub list.");

            // Ensure that the temporary connection isn't garbage collected before we're done with it.
            GC.KeepAlive(temporaryConnection);
        }

        /// <summary>
        /// Used in the TestMultiConnection() test as the Node we are to connect to.
        /// </summary>
        private NodeInfo m_multiConnectInfo;

        private Stack m_connectFrom;

        private int connectionCount = 0;
        private int passsedCount = 0;
        private int exitCount = 0;

        private void TestConnect()
        {
            StackConnection createdConnection;
            StackConnection foundConnection;

            // Connect temporarily and verify the connection.
            try
            {
                createdConnection = m_connectFrom.GetConnection(m_multiConnectInfo, false, TimeSpan.FromSeconds(5));
                System.Threading.Interlocked.Increment(ref connectionCount);
                foundConnection = m_connectFrom.FindConnection(m_multiConnectInfo, false);
                if (createdConnection == foundConnection)
                {
                    System.Threading.Interlocked.Increment(ref passsedCount);
                }
                else
                {
                    NUnit.Framework.Assert.Fail("Did not find the same connection.");
                }

                GC.KeepAlive(createdConnection);
                GC.KeepAlive(foundConnection);
            }
            catch (Stack.ConnectionException e)
            {
                NUnit.Framework.Assert.Fail(String.Format("Exception {0} when connecting to {1}.",
                    e.ErrorType, m_multiConnectInfo));
            }
            System.Threading.Interlocked.Increment(ref exitCount);
        }

        /// <summary>
        /// Try and connect from one node to another many times from different threads
        /// and make sure that you only get one connection.
        /// </summary>
        public void TestMultiConnection()
        {
            m_connectFrom = m_stacks[1];
            Stack connectTo = m_stacks[m_stacks.Count - 1];

            // Get the info of the first stack.
            m_multiConnectInfo = new NodeInfo();
            m_multiConnectInfo.IsHub = connectTo.NodeInfo.IsHub;
            m_multiConnectInfo.Address = connectTo.NodeInfo.Address;
            m_multiConnectInfo.NodeGUID = connectTo.NodeInfo.NodeGUID;

            StackConnection temporaryConnection = m_connectFrom.FindConnection(m_multiConnectInfo, false);
            NUnit.Framework.Assert.IsTrue(null == temporaryConnection,
                "The connection already exists and shouldn't exist.");

            int initialConnectionCount = m_connectFrom.ConnectionManager.TemporaryConnections.Count;

            // create a bunch of threads in each direction.
            System.Collections.Generic.List<System.Threading.Thread> threadList
                = new System.Collections.Generic.List<System.Threading.Thread>();
            for (int count = 0; count < 15; ++count)
            {
                threadList.Add(new System.Threading.Thread(new System.Threading.ThreadStart(TestConnect)));
            }

            // start them working
            threadList.ForEach(delegate(System.Threading.Thread t)
            {
                t.Start();
            });

            // wait for them to finish
            threadList.ForEach(delegate(System.Threading.Thread t)
            {
                t.Join();
            });

            NUnit.Framework.Assert.IsTrue(initialConnectionCount + 1 <= m_connectFrom.ConnectionManager.TemporaryConnections.MaxCount,
                "We found more connections that we should have.");
        }

        /// <summary>
        /// Counts the number of connections in the list of stacks and conpares that
        /// to the number of expected connections for this test.
        /// </summary>
        /// <returns>Ture if all expected connections exist</returns>
        private bool AreAllConnectionsMade(out int stackConnections)
        {
            int counter = 0;
            // Count the connections for all stacks.
            m_stacks.ForEach(delegate(Stack stack)
            {
                counter += stack.ConnectionManager.PersistentConnections.Count;
            });
            stackConnections = counter;
            return (stackConnections == m_numberOfConnectionsMade);
        }

        /// <summary>
        /// Tests to make sure that all the expected connections are in the
        /// current list of stacks and causes a test failure if it can not
        /// find the expected number of connections.
        /// </summary>
        private void ConfirmConnections()
        {
            int stackConnections;
            if (!AreAllConnectionsMade(out stackConnections))
            {
                System.Threading.Thread.Sleep(TimeSpan.FromSeconds(1));
                if (!AreAllConnectionsMade(out stackConnections))
                {
                    NUnit.Framework.Assert.Fail("Found " + stackConnections.ToString() +
                                                " connections, " + m_numberOfConnectionsMade.ToString() + " expected.");
                }
            }
        }

        /// <summary>
        /// Run a test for propagating Query Hash Tables and optionally
        /// Message Relays and Queries.
        /// </summary>
        /// <param name="RunLongTesting">Set to true to run more exhaustive tests that take a long time.</param>
        public void RunQHTTest(bool RunLongTesting)
        {
            // have each node publish a string with delays.
            m_stacks.ForEach(delegate(Stack stack)
            {
                stack.LocalQueryHashTable.Add(stack.NodeInfo.FriendlyName);
                System.Threading.Thread.Sleep(50);
            });

            // run for 10 ping intervals so that the messages have a chance to get through the system.
            int pingTimerIntervals = 10;
            for (int tickCounter = 0; tickCounter < pingTimerIntervals; ++tickCounter)
            {
                // sleep for an interval.
                System.Threading.Thread.Sleep(m_interval);
            }

            m_stacks.ForEach(delegate(Stack stack)
            {
                // All nodes should contain their own string.
                NUnit.Framework.Assert.IsTrue(
                    stack.LocalQueryHashTable.MayContain(stack.NodeInfo.FriendlyName));

                if (stack.NodeInfo.IsHub)
                {
                    // hubs should have the strings of all persistently-attached leaves.
                    stack.ConnectionManager.PersistentConnections.ForEach(delegate(StackConnection sc)
                    {
                        if (sc.RemoteNodeInfo.IsLeaf)
                        {
                            NUnit.Framework.Assert.IsTrue(
                                stack.AmalgamatedQueryHashTable.MayContain(sc.RemoteNodeInfo.FriendlyName),
                                stack.NodeInfo.FriendlyName +
                                "'s Amalgamated Query Hash Table not contain " +
                                sc.RemoteNodeInfo.FriendlyName);

                            // now, all hubs that are directly connected to this hub should have the same leaf
                            // in the remote copy of this hub's query hash table.
                            m_stacks.ForEach(delegate(Stack stack2)
                            {
                                if (stack2.NodeInfo.IsHub)
                                {
                                    stack2.ConnectionManager.PersistentConnections.ForEach(delegate(StackConnection s)
                                    {
                                        if ((s.RemoteNodeInfo != null) && (s.RemoteNodeInfo.Equals(stack.NodeInfo)))
                                        {
                                            // we have found the other end of one of our connections to a hub
                                            NUnit.Framework.Assert.IsTrue(
                                                s.RemoteQueryHashTable.MayContain(
                                                    sc.RemoteNodeInfo.FriendlyName),
                                                stack2.NodeInfo.FriendlyName +
                                                ": Remote Query Hash Table for connection " +
                                                s.RemoteNodeInfo.FriendlyName +
                                                " does not hit for contents " +
                                                sc.RemoteNodeInfo.FriendlyName);
                                        }
                                    });
                                }
                            });
                        }
                    });
                }
            });

            // TODO: add a mode for the MessageRelayerMock and QuerierTest
            // that can do a shorter version of the test for every run testing.
            if (RunLongTesting)
            {
                // Create bad stack.
                CreateBadStack("Unfriendly Stack", true);

                // Send message relays.
                new MessageRelayerMock(m_stacks).Run();

                // Send queries.
                using (QuerierTest test = new QuerierTest(m_stacks))
                {
                    test.Run();
                }

                // Close bad stack.
                DisposeBadStack();
            }
        }

        /// <summary>
        /// Run the test.
        /// </summary>
        public void Run(int pingTimerIntervals)
        {
            // initialize the counters
            int stackCount = m_stacks.Count;
            int[] numPings = new int[stackCount];
            int[] numPongs = new int[stackCount];
            for (int index = 0; index < m_stacks.Count; ++index)
            {
                numPings[index] = 0;
                numPongs[index] = 0;
            }

            for (int tickCounter = 0; tickCounter < pingTimerIntervals; ++tickCounter)
            {
                // sleep for an interval.
                System.Threading.Thread.Sleep(m_interval);

                // for each stack, check the first connection to make sure 
                // that the Ping and Pong count are at least as big as they were before.
                for (int index = 0; index < stackCount; ++index)
                {
                    foreach (StackConnection connection in m_stacks[index].ConnectionManager.PersistentConnections)
                    {
                        NUnit.Framework.Assert.IsTrue(connection.NumPingsReceived >= numPings[index],
                            "Not enough pings received.");
                        numPings[index] = connection.NumPingsReceived;

                        NUnit.Framework.Assert.IsTrue(connection.NumPongsReceived >= numPongs[index],
                            "Not enough pongs received.");
                        numPongs[index] = connection.NumPongsReceived;

                        break;
                    }
                }
                Tracer.DebugWriteLine("Time unit = {0}", (tickCounter + 1).ToString());
                Tracer.DebugWrite("Ping Counts = ");
                for (int index = 0; index < stackCount; ++index)
                {
                    Tracer.Write("{0} ", numPings[index].ToString());
                }
                Tracer.DebugWriteLine();

                Tracer.DebugWrite("Pong Counts = ");
                for (int index = 0; index < stackCount; ++index)
                {
                    Tracer.DebugWrite("{0} ", numPongs[index].ToString());
                }
                Tracer.DebugWriteLine();
            }

            for (int index = 0; index < stackCount; ++index)
            {
                long minMessages = pingTimerIntervals / 2;
                NUnit.Framework.Assert.IsTrue(numPings[index] >= minMessages,
                    String.Format("numPings[{0}] ({1}) < minMessages ({2})",
                        index, numPings[index], minMessages));
                NUnit.Framework.Assert.IsTrue(numPongs[index] >= minMessages,
                    String.Format("numPongs[{0}] ({1}) < minMessages ({2})",
                        index, numPongs[index], minMessages));
            }

            // All of our connections should have transmitted, and received, a Local Node Info.
            // Iterate through stacks.
            string errors = null;
            m_stacks.ForEach(delegate(Stack currentStack)
            {
                // Iterate through stack connections in each.
                currentStack.ConnectionManager.PersistentConnections.ForEach(delegate(StackConnection sc)
                {
                    NodeInfo rlni = sc.RemoteNodeInfo;
                    NUnit.Framework.Assert.IsTrue(rlni != null, "Remote node info is null.");
                });

                if (currentStack.NodeInfo.IsHub)
                {
                    // Test that this hub is known by all other stacks
                    m_stacks.ForEach(delegate(Stack stack)
                    {
                        // if this is not the same stack
                        if (stack.NodeInfo.NodeGUID != currentStack.NodeInfo.NodeGUID)
                        {
                            if (!stack.KnownHubList.Contains(currentStack.NodeInfo))
                            {
                                errors += "Stack " + stack.NodeInfo.FriendlyName +
                                          " did not contain information about hub " +
                                          currentStack.NodeInfo.FriendlyName + "\n";
                            }
                        }
                    });
                }
            });

            // Send bad messages on each connection and ensure that it doesn't crash.
            m_stacks.ForEach(
                delegate(Stack stack)
                {
                    // Create a bad message.
                    // Start with the message ID for a known hub list, and follow by garbage bytes.
                    System.Diagnostics.Debug.Assert(StackConnection.KnownHubListMessageID == "KHLi");
                    Message badMessage = new Message(new Byte[] { (byte)'K', (byte)'H', (byte)'L', (byte)'i', 6, 6, 6, 0, 42 });

                    // Send it.
                    stack.SendMessage(badMessage);

                    // Send unknown message.
                    Message unknownMessage = new Message("none");
                    stack.SendMessage(unknownMessage);
                });

            NUnit.Framework.Assert.IsNull(errors, errors);
        }

        #region Bad Stack

        /// <summary>
        /// A malicious stack for testing.
        /// </summary>
        private Stack m_badStack = null;

        /// <summary>
        /// A malicious stack for testing.
        /// </summary>
        public Stack BadStack
        {
            get
            {
                return m_badStack;
            }
        }

        /// <summary>
        /// Create a malicious stack, connected to the specified stacks.
        /// </summary>
        /// <param name="friendlyName">The friendly name of the stack.</param>
        /// <param name="start">Whether to start the stack.</param>
        public void CreateBadStack(String friendlyName, bool start)
        {
            // Create bad stack member.
            Stack badStack = CreateStack(friendlyName, m_interval, start);
            m_badStack = badStack;

            // Connect the bad stack to each stack in stack list.
            m_stacks.ForEach(
                delegate(Stack stack)
                {
                    badStack.CreateConnection(stack.NodeInfo, true);
                });

            NUnit.Framework.Assert.Less(0, badStack.ConnectionManager.PersistentHubConnectionCount);

            // Connect handler to badStack's disconnect that reconnects.
            badStack.OnClosedConnection += ReconnectBadStack;

            // Intercept each message that's about to be sent by this stack, and mess it up.
            badStack.OnSendingMessageToConnection += MangleOutgoingMessage;
        }

        /// <summary>
        /// Disposes the bad stack and set the member to null.
        /// </summary>
        public void DisposeBadStack()
        {
            // Clear bad stack member.
            Stack badStack = m_badStack;
            m_badStack = null;

            if (null != badStack)
            {
                // Disconnect closed connection handler.
                badStack.OnClosedConnection -= ReconnectBadStack;

                // Wait a moment for the thread to finish, and confirm that the bad stack has no connections.
                System.Threading.Thread.Sleep(5000);
                NUnit.Framework.Assert.AreEqual(0, badStack.ConnectionManager.PersistentHubConnectionCount);

                // Close the stack.
                badStack.Dispose();
            }
        }

        /// <summary>
        /// A handler for the bad stack's closed connection event, which reopens the connection.
        /// </summary>
        /// <param name="stack">The bad stack.</param>
        /// <param name="connection">The connection that has closed.</param>
        private void ReconnectBadStack(Stack stack, StackConnection connection)
        {
            System.Threading.Thread thread = new System.Threading.Thread(
                delegate()
                {
                    // While the test is still running, attempt a reconnection.
                    while (stack == m_badStack)
                    {
                        // Wait a moment.
                        System.Threading.Thread.Sleep(1000);

                        // Disconnect message mangler to allow for connection.
                        stack.OnSendingMessageToConnection -= MangleOutgoingMessage;

                        // Reconnect.
                        StackConnection newConnection = stack.GetConnection(connection.RemoteNodeInfo, true, TimeSpan.Zero);
                        if (null == newConnection)
                        {
                            Tracer.DebugWriteLine("Bad stack: Not reconnected; no exception.");
                        }
                        else
                        {
                            Tracer.DebugWriteLine("Bad stack: Connected to {0}.", connection.RemoteNodeInfo.FriendlyName);
                        }

                        // Reconnect message mangler.
                        stack.OnSendingMessageToConnection += MangleOutgoingMessage;

                        // If we reconnected, break.
                        if (null != newConnection)
                        {
                            break;
                        }
                    }
                });
            thread.IsBackground = true;
            thread.Start();
        }

        /// <summary>
        /// A handler for the bad stack's send message event, which garbles the message in a semi-controlled way.
        /// </summary>
        /// <param name="stack">The bad stack.</param>
        /// <param name="connection">The connection to receive the message.</param>
        /// <param name="message">The message to be garbled and sent.</param>
        private void MangleOutgoingMessage(Stack stack, StackConnection connection, Message message)
        {
            // Mess up the message (after the message ID, so that it doesn't get rejected outright).
            Random random = new Random();
            for (int index = 4; index < message.Bytes.Array.Length; ++index)
            {
                message.Bytes.Array.SetValue(
                    (byte)random.Next(200),
                    (byte)random.Next(4, message.Bytes.Array.Length));
            }
            Tracer.DebugWriteLine("Mangled message: {0}", message.ToString());
        }

        #endregion

        #region Dispose

        /// <summary>
        /// Track whether Dispose has been called.
        /// </summary>
        private bool m_disposed = false;

        /// <summary>
        /// Implement IDisposable.
        /// </summary>
        /// <remarks>
        /// Do not make this method virtual.
        /// A derived class should not be able to override this method. 
        /// </remarks>
        public void Dispose()
        {
            Dispose(true);

            // This object will be cleaned up by the Dispose method.
            // Therefore, you should call GC.SupressFinalize to
            // take this object off the finalization queue 
            // and prevent finalization code for this object
            // from executing a second time.
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Dispose(bool disposing) executes in two distinct scenarios, 
        /// indicated by the "disposing" argument.
        /// </summary>
        /// <param name="disposing">
        /// If disposing equals true, the method has been called directly
        /// or indirectly by a user's code. Managed and unmanaged resources
        /// can be disposed.
        /// If disposing equals false, the method has been called by the 
        /// runtime from inside the finalizer and you should not reference 
        /// other objects. Only unmanaged resources can be disposed.
        /// </param>
        private void Dispose(bool disposing)
        {
            // Check to see if Dispose has already been called.
            if (!m_disposed)
            {
                // If disposing equals true, dispose all managed 
                // and unmanaged resources.
                if (disposing)
                {
                    // Dispose managed resources.
                    CloseStacks(m_stacks);
                }

                // Call the appropriate methods to clean up 
                // unmanaged resources here.
                // If disposing is false, 
                // only the following code is executed.
            }
            m_disposed = true;
        }

        /// <summary>
        /// The finalization code.
        /// </summary>
        /// <remarks>
        /// Use C# destructor syntax for finalization code.
        /// This destructor will run only if the Dispose method 
        /// does not get called.
        /// It gives your base class the opportunity to finalize.
        /// Do not provide destructors in types derived from this class.
        /// </remarks>
        ~StackTestHelper()
        {
            // Do not re-create Dispose clean-up code here.
            // Calling Dispose(false) is optimal in terms of
            // readability and maintainability.
            Dispose(false);
        }

        #endregion

        /// <summary>
        /// the next port number to use in creating PyxNet stacks.
        /// </summary>
        private static ThreadSafeInt portNumber = 45000;

        /// <summary>
        /// Helper function for creating a stack object.  This is for test-use
        /// only.
        /// </summary>
        /// <param name="friendlyName">The human readable name for this node.</param>
        /// <param name="interval">The number of milliseconds between messages.</param>
        /// <param name="start">Set to true if the stack should also be started.</param>
        /// <returns></returns>
        public static Stack CreateStack(string friendlyName, long interval, bool start)
        {
            Stack newStack = (interval == -1) ?
                new Stack(new PyxNet.DLM.PrivateKey()) :
                new Stack(new PyxNet.DLM.PrivateKey(),
                    interval, interval, interval * 4, interval * 2, interval * 2);
            newStack.NodeInfo.FriendlyName = friendlyName;
            newStack.PrivateKey.AssignKeyName(newStack.NodeInfo.FriendlyName);
            if (start)
            {
                if (portNumber > 55000)
                {
                    portNumber = 45000;
                }
                System.Net.IPEndPoint listenOn = new System.Net.IPEndPoint(
                    System.Net.IPAddress.Parse("127.0.0.1"), ++portNumber);
                NetworkAddress listenOnAddress = new NetworkAddress(listenOn);
                try
                {
                    newStack.Start(listenOnAddress);
                }
                catch (System.Net.Sockets.SocketException e)
                {
                    NUnit.Framework.Assert.Fail(String.Format("Starting stack {0} threw exception {1}.",
                        newStack.NodeInfo.FriendlyName, e.Message));
                }
            }

            return newStack;
        }

        /// <summary>
        /// Create a list of Stacks on sequential ports on the local host IP.
        /// </summary>
        /// <param name="numStacks">The number of Stacks to create.</param>
        /// <param name="interval">The number of milliseconds between messages.</param>
        /// <returns></returns>
        private static List<Stack> CreateStacks(int numStacks, long interval)
        {
            List<Stack> stacks = new List<Stack>(numStacks);
            for (int count = 0; count < numStacks; ++count)
            {
                Stack newStack = CreateStack(GenerateFriendlyName(count), interval, true);

                // Create a new (empty) certificate repository.
                string certificateRepositoryName = String.Format("{0}.Certificates",
                    GenerateFriendlyName(count));
                if (System.IO.File.Exists(certificateRepositoryName))
                {
                    System.IO.File.Delete(certificateRepositoryName);
                }
                newStack.CertificateRepository =
                    new PyxNet.Service.CertificateRepository(
                        certificateRepositoryName);

                stacks.Add(newStack);

                // Log creation of a node.  (For diagnostics.)
                EncryptedMessageHelper.Trace.DebugWriteLine(
                    "Created a node {0} with {1} and {2}",
                    newStack.NodeInfo.FriendlyName,
                    DLM.PrivateKey.GetKeyDescription(newStack.PrivateKey),
                    DLM.PrivateKey.GetKeyDescription(newStack.PrivateKey.PublicKey));
            }
            return stacks;
        }

        private static string GenerateFriendlyName(int nodeIndex)
        {
            return "Node " + nodeIndex.ToString();
        }

        /// We have defined a standard set of test topologies for testing PYXNet.
        /// There are four topologies, and each one is an extension of the previous
        /// one adding new kinds of barriers to overcome in finding data on the network.
        /// For details on how the nodes are connected for each topology see:
        /// https://www.pyxisinnovation.com/pyxinternalwiki/index.php?title=PYXNet_Communications_Testing

        /// <summary>
        /// Generate PYXNet test topology one.
        /// </summary>
        /// <returns>A list of Stacks configured in a PYXNet.</returns>
        public static List<Stack> CreateTopologyOneStacks(long interval)
        {
            List<Stack> stacks = CreateStacks(4, interval);
            InitTopologyOne(stacks);
            return stacks;
        }

        /// <summary>
        /// Generate PYXNet test topology two.
        /// </summary>
        /// <returns>A list of Stacks configured in a PYXNet.</returns>
        public static List<Stack> CreateTopologyTwoStacks(long interval)
        {
            List<Stack> stacks = CreateStacks(6, interval);
            InitTopologyOne(stacks);
            AddTopologyTwo(stacks);
            return stacks;
        }

        /// <summary>
        /// Generate PYXNet test topology three.
        /// </summary>
        /// <returns>A list of Stacks configured in a PYXNet.</returns>
        public static List<Stack> CreateTopologyThreeStacks(long interval)
        {
            List<Stack> stacks = CreateStacks(8, interval);
            InitTopologyOne(stacks);
            AddTopologyTwo(stacks);
            AddTopologyThree(stacks);
            return stacks;
        }

        /// <summary>
        /// Generate PYXNet test topology four.
        /// </summary>
        /// <returns>A list of Stacks configured in a PYXNet.</returns>
        public static List<Stack> CreateTopologyFourStacks(long interval)
        {
            List<Stack> stacks = CreateStacks(14, interval);
            InitTopologyOne(stacks);
            AddTopologyTwo(stacks);
            AddTopologyThree(stacks);
            AddTopologyFour(stacks);
            return stacks;
        }

        /// <summary>
        /// Set up the connections and node types needed for Topology One
        /// </summary>
        /// <param name="stacks">The stacks that you wish to configure.</param>
        private static void InitTopologyOne(List<Stack> stacks)
        {
            System.Diagnostics.Debug.Assert(3 < stacks.Count);

            stacks[0].NodeInfo.Mode = NodeInfo.OperatingMode.Hub;
            stacks[1].NodeInfo.Mode = NodeInfo.OperatingMode.Leaf;
            stacks[2].NodeInfo.Mode = NodeInfo.OperatingMode.Leaf;
            stacks[3].NodeInfo.Mode = NodeInfo.OperatingMode.Leaf;

            stacks[1].CreateConnection(stacks[0].NodeInfo, true);
            stacks[2].CreateConnection(stacks[0].NodeInfo, true);
            stacks[3].CreateConnection(stacks[0].NodeInfo, true);
        }

        /// <summary>
        /// Add the connections and node types needed for Topology Two
        /// </summary>
        /// <param name="stacks">The stacks that you wish to configure.</param>
        private static void AddTopologyTwo(List<Stack> stacks)
        {
            System.Diagnostics.Debug.Assert(5 < stacks.Count);

            stacks[4].NodeInfo.Mode = NodeInfo.OperatingMode.Hub;
            stacks[5].NodeInfo.Mode = NodeInfo.OperatingMode.Leaf;

            stacks[4].CreateConnection(stacks[0].NodeInfo, true);
            stacks[5].CreateConnection(stacks[4].NodeInfo, true);
        }

        /// <summary>
        /// Add the connections and node types needed for Topology Three
        /// </summary>
        /// <param name="stacks">The stacks that you wish to configure.</param>
        private static void AddTopologyThree(List<Stack> stacks)
        {
            System.Diagnostics.Debug.Assert(7 < stacks.Count);

            stacks[6].NodeInfo.Mode = NodeInfo.OperatingMode.Hub;
            stacks[7].NodeInfo.Mode = NodeInfo.OperatingMode.Leaf;

            stacks[1].CreateConnection(stacks[6].NodeInfo, true);
            stacks[7].CreateConnection(stacks[6].NodeInfo, true);
        }

        /// <summary>
        /// Add the connections and node types needed for Topology Four
        /// </summary>
        /// <param name="stacks">The stacks that you wish to configure.</param>
        private static void AddTopologyFour(List<Stack> stacks)
        {
            System.Diagnostics.Debug.Assert(13 < stacks.Count);

            stacks[8].NodeInfo.Mode = NodeInfo.OperatingMode.Hub;
            stacks[9].NodeInfo.Mode = NodeInfo.OperatingMode.Hub;
            stacks[10].NodeInfo.Mode = NodeInfo.OperatingMode.Hub;
            stacks[11].NodeInfo.Mode = NodeInfo.OperatingMode.Leaf;
            stacks[12].NodeInfo.Mode = NodeInfo.OperatingMode.Leaf;
            stacks[13].NodeInfo.Mode = NodeInfo.OperatingMode.Leaf;

            stacks[5].CreateConnection(stacks[8].NodeInfo, true);
            stacks[8].CreateConnection(stacks[9].NodeInfo, true);
            stacks[9].CreateConnection(stacks[10].NodeInfo, true);
            stacks[10].CreateConnection(stacks[8].NodeInfo, true);
            stacks[11].CreateConnection(stacks[8].NodeInfo, true);
            stacks[11].CreateConnection(stacks[9].NodeInfo, true);
            stacks[12].CreateConnection(stacks[9].NodeInfo, true);
            stacks[13].CreateConnection(stacks[10].NodeInfo, true);
        }

        /// <summary>
        /// Close all of the Stacks in the list.
        /// </summary>
        /// <param name="stacks"></param>
        public static void CloseStacks(List<Stack> stacks)
        {
            stacks.ForEach(delegate(Stack stack)
            {
                stack.Dispose();
            });
            // give all the threads a chance to shut down the listeners.
            System.Threading.Thread.Sleep(100);
            stacks.Clear();
        }

        /// <summary>
        /// Waits for all hash updates.  This ensures that queries should work.
        /// </summary>
        public void WaitForHashUpdates()
        {
            WaitForHashUpdates(Stacks);
        }

        internal static void WaitForHashUpdates(List<Stack> stacks)
        {
            int timeoutPeriod = 10;
            DateTime timeout = DateTime.Now + TimeSpan.FromSeconds(
                timeoutPeriod);

            bool dirty;
            do
            {
                dirty = false;
                System.Threading.Thread.Sleep(50);
                foreach (Stack s in stacks)
                {
                    if (s.IsQueryHashTableDirty)
                    {
                        dirty = true;
                    }
                }
            } while (dirty && (DateTime.Now < timeout));

            NUnit.Framework.Assert.IsFalse(dirty,
                string.Format(
                    "Unable to update hash tables within the timeout period of {0} seconds.",
                    timeoutPeriod.ToString()));
        }
    }
}