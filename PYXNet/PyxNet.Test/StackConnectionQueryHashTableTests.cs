using System;

namespace PyxNet.Test
{
    /// <summary>
    /// Test Class to test Sending/Receiving Query Hash Tables.
    /// </summary>
    [NUnit.Framework.TestFixture]
    public class StackConnectionQueryHashTableTests : TestStackConnection
    {
        /// <summary>
        /// Set to true when a message is seen from stack one.
        /// </summary>
        private bool m_wasReceivedOne = false;

        /// <summary>
        /// Set to true when a message is seen from stack two.
        /// </summary>
        private bool m_wasReceivedTwo = false;

        /// <summary>
        /// Handle a message coming from stack one.
        /// </summary>
        public void HandleOne(StackConnection connection, Message message)
        {
            m_wasReceivedOne = true;
        }

        /// <summary>
        /// Handle a message coming from stack two.
        /// </summary>
        public void HandleTwo(StackConnection connection, Message message)
        {
            m_wasReceivedTwo = true;
        }

        /// <summary>
        /// Entry point for the test code.
        /// </summary>
        [NUnit.Framework.Test]
        public void Test()
        {
            const string s1 = "This is one thing I could add.";
            const string s2 = "Blue Marble";
            const string s3 = "Fred Borland";
            const string s4 = "Some longer string that we can see if it hashes out to something.";
            const string s5 = "Just a little bit of nonsense.";
            const string s6 = "One more little string to test with.";

            m_one.OnQueryHashTable += HandleOne;
            m_two.OnQueryHashTable += HandleTwo;

            QueryHashTable payloadOne = new QueryHashTable();
            payloadOne.Add(s1);
            payloadOne.Add(s3);
            payloadOne.Add(s5);
            QueryHashTable payloadTwo = new QueryHashTable();
            payloadTwo.Add(s2);
            payloadTwo.Add(s4);
            payloadTwo.Add(s6);

            m_one.SendMessage(payloadOne.ToMessage());
            DateTime startTime = DateTime.Now;
            while (!m_wasReceivedTwo)
            {
                // hang out for a 1/10th of a second
                System.Threading.Thread.Sleep(100);
                TimeSpan elapsedTime = DateTime.Now - startTime;

                // fail the test if it takes longer than 5 seconds to see the connection
                NUnit.Framework.Assert.IsTrue(elapsedTime.TotalSeconds < 5,
                    "Connection two did not get a message.");
            }

            // Compare what was sent with the result.
            NUnit.Framework.Assert.IsTrue(m_two.RemoteQueryHashTable.MayContain(s1));
            NUnit.Framework.Assert.IsTrue(m_two.RemoteQueryHashTable.MayContain(s3));
            NUnit.Framework.Assert.IsTrue(m_two.RemoteQueryHashTable.MayContain(s5));

            m_two.SendMessage(payloadTwo.ToMessage());
            startTime = DateTime.Now;
            while (!m_wasReceivedOne)
            {
                // hang out for a 1/10th of a second
                System.Threading.Thread.Sleep(100);
                TimeSpan elapsedTime = DateTime.Now - startTime;

                // fail the test if it takes longer than 5 seconds to see the connection
                NUnit.Framework.Assert.IsTrue(elapsedTime.TotalSeconds < 5,
                    "Connection one did not get a message.");
            }

            // Compare what was sent with the result.
            NUnit.Framework.Assert.IsTrue(m_one.RemoteQueryHashTable.MayContain(s2));
            NUnit.Framework.Assert.IsTrue(m_one.RemoteQueryHashTable.MayContain(s4));
            NUnit.Framework.Assert.IsTrue(m_one.RemoteQueryHashTable.MayContain(s6));

            Close();
        }
    }
}