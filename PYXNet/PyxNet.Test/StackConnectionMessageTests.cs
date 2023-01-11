using System;

namespace PyxNet.Test
{
    public class StackConnectionMessageTests : TestStackConnection
    {
        /// <summary>
        /// Incremented when an OnPong is seen from stack one.
        /// </summary>
        private int m_PongReceivedOneCount = 0;

        /// <summary>
        /// Incremented when an OnPing is seen from stack one.
        /// </summary>
        private int m_PingReceivedOneCount = 0;

        /// <summary>
        /// Incremented when an OnAnyMessage is seen from stack one.
        /// </summary>
        private int m_AnyReceivedOneCount = 0;

        /// <summary>
        /// Incremented when an OnUnknownMessage is seen from stack one.
        /// </summary>
        private int m_UnknownReceivedOneCount = 0;

        /// <summary>
        /// Incremented when an OnPong is seen from stack two.
        /// </summary>
        private int m_PongReceivedTwoCount = 0;

        /// <summary>
        /// Incremented when an OnPing is seen from stack two.
        /// </summary>
        private int m_PingReceivedTwoCount = 0;

        /// <summary>
        /// Incremented when an OnAnyMessage is seen from stack two.
        /// </summary>
        private int m_AnyReceivedTwoCount = 0;

        /// <summary>
        /// Incremented when an OnUnknownMessage is seen from stack two.
        /// </summary>
        private int m_UnknownReceivedTwoCount = 0;

        /// <summary>
        /// Entry point for the test code for OnAnyMessage and OnUnknownMessage.
        /// </summary>
        [NUnit.Framework.Test]
        public void TestAnyAndUnknown()
        {
            m_one.OnPong += delegate(StackConnection c, Message m) 
            { 
                System.Threading.Interlocked.Increment(ref m_PongReceivedOneCount); 
            };
            m_one.OnPing += delegate(StackConnection c, Message m) 
            {
                System.Threading.Interlocked.Increment(ref m_PingReceivedOneCount); 
            };
            m_one.OnUnknownMessage += delegate(StackConnection c, Message m) 
            {
                System.Threading.Interlocked.Increment(ref m_UnknownReceivedOneCount); 
            };
            m_one.OnAnyMessage += delegate(StackConnection c, Message m) 
            {
                System.Threading.Interlocked.Increment(ref m_AnyReceivedOneCount); 
            };
            m_two.OnPong += delegate(StackConnection c, Message m) 
            {
                System.Threading.Interlocked.Increment(ref m_PongReceivedTwoCount); 
            };
            m_two.OnPing += delegate(StackConnection c, Message m) 
            {
                System.Threading.Interlocked.Increment(ref m_PingReceivedTwoCount); 
            };
            m_two.OnUnknownMessage += delegate(StackConnection c, Message m) 
            {
                System.Threading.Interlocked.Increment(ref m_UnknownReceivedTwoCount); 
            };
            m_two.OnAnyMessage += delegate(StackConnection c, Message m) 
            {
                System.Threading.Interlocked.Increment(ref m_AnyReceivedTwoCount); 
            };

            const int PingCount = 25;
            for (int count = 0; count < PingCount; ++count)
            {
                m_one.SendPing();
                m_two.SendPing();
            }

            DateTime startTime = DateTime.Now;
            while (m_PingReceivedOneCount < PingCount || m_PongReceivedOneCount < PingCount ||
                   m_PingReceivedTwoCount < PingCount || m_PongReceivedTwoCount < PingCount)
            {
                // hang out for a moment
                System.Threading.Thread.Sleep(100);
                TimeSpan elapsedTime = DateTime.Now - startTime;

                // fail the test if it takes too long to see the connection
                NUnit.Framework.Assert.IsTrue(elapsedTime.TotalSeconds < 15,
                    "Not enough Pings recieved.");
            }

            // Now check out the message counts.
            m_one.Tracer.WriteLine("PingCount = {0}", PingCount);
            m_one.Tracer.WriteLine("Ping Received One Count = {0}", m_PingReceivedOneCount);
            m_one.Tracer.WriteLine("Pong Received One Count = {0}", m_PongReceivedOneCount);
            m_one.Tracer.WriteLine("Unknown Received One Count = {0}", m_UnknownReceivedOneCount);
            m_one.Tracer.WriteLine("Any Received One Count = {0}", m_AnyReceivedOneCount);
            m_two.Tracer.WriteLine("Pong Received Two Count = {0}", m_PongReceivedTwoCount);
            m_two.Tracer.WriteLine("Ping Received Two Count = {0}", m_PingReceivedTwoCount);
            m_two.Tracer.WriteLine("Unknown Received Two Count = {0}", m_UnknownReceivedTwoCount);
            m_two.Tracer.WriteLine("Any Received Two Count = {0}", m_AnyReceivedTwoCount);

            NUnit.Framework.Assert.AreEqual(PingCount, m_PongReceivedTwoCount, "m_PongReceivedTwoCount has the wrong value.");
            NUnit.Framework.Assert.AreEqual(PingCount, m_PingReceivedTwoCount, "m_PingReceivedTwoCount has the wrong value.");
            NUnit.Framework.Assert.AreEqual(0, m_UnknownReceivedTwoCount, "m_UnknownReceivedTwoCount has the wrong value.");
            NUnit.Framework.Assert.AreEqual((PingCount * 2), m_AnyReceivedTwoCount, "m_AnyReceivedTwoCount has the wrong value.");
            NUnit.Framework.Assert.AreEqual(PingCount, m_PongReceivedOneCount, "m_PongReceivedOneCount has the wrong value.");
            NUnit.Framework.Assert.AreEqual(PingCount, m_PingReceivedOneCount, "m_PingReceivedOneCount has the wrong value.");
            NUnit.Framework.Assert.AreEqual(0, m_UnknownReceivedOneCount, "m_UnknownReceivedOneCount has the wrong value.");
            NUnit.Framework.Assert.AreEqual((PingCount * 2), m_AnyReceivedOneCount, "m_AnyReceivedOneCount has the wrong value.");
            Close();
        }
    }
}