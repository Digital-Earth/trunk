using System;

namespace PyxNet.Test
{
    /// <summary>
    /// Test Class to test the behaviour of Ping and Pong.
    /// </summary>
    //[NUnit.Framework.TestFixture]
    public class StackConnectionPingPongTests : TestStackConnection
    {
        /// <summary>
        /// Set to true when an OnPong is seen from stack one.
        /// </summary>
        private bool m_wasPongReceivedOne = false;

        /// <summary>
        /// Set to true when an OnPing is seen from stack one.
        /// </summary>
        private bool m_wasPingReceivedOne = false;

        /// <summary>
        /// Set to true when an OnPong is seen from stack two.
        /// </summary>
        private bool m_wasPongReceivedTwo = false;

        /// <summary>
        /// Set to true when an OnPing is seen from stack two.
        /// </summary>
        private bool m_wasPingReceivedTwo = false;

        /// <summary>
        /// Handle an OnPong coming from stack one.
        /// </summary>
        public void HandlePongOne(StackConnection connection, Message message)
        {
            m_wasPongReceivedOne = true;
        }

        /// <summary>
        /// Handle an OnPing coming from stack one.
        /// </summary>
        public void HandlePingOne(StackConnection connection, Message message)
        {
            m_wasPingReceivedOne = true;
        }

        /// <summary>
        /// Handle an OnPong coming from stack two.
        /// </summary>
        public void HandlePongTwo(StackConnection connection, Message message)
        {
            m_wasPongReceivedTwo = true;
        }

        /// <summary>
        /// Handle an OnPing coming from stack two.
        /// </summary>
        public void HandlePingTwo(StackConnection connection, Message message)
        {
            m_wasPingReceivedTwo = true;
        }

        /// <summary>
        /// Entry point for the test code for Ping/Pong.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestPingPong()
        {
            m_one.OnPong += HandlePongOne;
            m_one.OnPing += HandlePingOne;
            m_two.OnPong += HandlePongTwo;
            m_two.OnPing += HandlePingTwo;

            m_one.SendPing();

            // we sent a Ping, so we should get back a Pong
            DateTime startTime = DateTime.Now;
            while (!m_wasPingReceivedTwo || !m_wasPongReceivedOne)
            {
                // hang out for a 1/10th of a second
                System.Threading.Thread.Sleep(100);
                TimeSpan elapsedTime = DateTime.Now - startTime;

                // fail the test if it takes longer than 5 seconds to see the connection
                NUnit.Framework.Assert.IsTrue(elapsedTime.TotalSeconds < 5,
                    "Connection one did not get a Pong back from a Ping.");
            }

            // Connection one should have received a pong, but not connection two
            NUnit.Framework.Assert.IsTrue(m_wasPongReceivedOne, "Did not receive a pong on connection one.");
            NUnit.Framework.Assert.IsFalse(m_wasPongReceivedTwo);

            // Connection two should have received a Ping but not connection one.
            NUnit.Framework.Assert.IsFalse(m_wasPingReceivedOne);
            NUnit.Framework.Assert.IsTrue(m_wasPingReceivedTwo, "Did not receive a ping on connection two.");

            m_two.SendPing();

            // we sent a Ping, so we should get back a Pong
            startTime = DateTime.Now;
            while (!m_wasPingReceivedOne || !m_wasPongReceivedTwo)
            {
                // hang out for a 1/10th of a second
                System.Threading.Thread.Sleep(100);
                TimeSpan elapsedTime = DateTime.Now - startTime;

                // fail the test if it takes longer than 5 seconds to see the connection
                NUnit.Framework.Assert.IsTrue(elapsedTime.TotalSeconds < 5,
                    "Connection two did not get a Pong back from a Ping.");
            }

            // Now everyone should have received both a Ping and a Pong.
            NUnit.Framework.Assert.IsTrue(m_wasPongReceivedOne);
            NUnit.Framework.Assert.IsTrue(m_wasPongReceivedTwo);
            NUnit.Framework.Assert.IsTrue(m_wasPingReceivedOne);
            NUnit.Framework.Assert.IsTrue(m_wasPingReceivedTwo);

            Close();
        }
    }
}