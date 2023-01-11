using System;
using NUnit.Framework;

namespace PyxNet.Test
{
    [TestFixture]
    public class TimedConnectionHolderTests
    {
        // TODO[kabiraman]: Needs to reviewed.
        /// <summary>
        /// Tests the timed connection holder.
        /// </summary>
        [Test]
        [Ignore]
        public void TestHolder()
        {
            TimedConnectionHolder holder = new TimedConnectionHolder();

            StackConnection sc1 = new StackConnection(new DummyConnection());
            StackConnection sc2 = new StackConnection(new DummyConnection());
            StackConnection sc3 = new StackConnection(new DummyConnection());
            StackConnection sc4 = new StackConnection(new DummyConnection());

            holder.HoldConnection(sc1, TimeSpan.FromSeconds(1));
            holder.HoldConnection(sc2, TimeSpan.FromSeconds(2));
            holder.HoldConnection(sc3, TimeSpan.FromSeconds(1));
            holder.HoldConnection(sc4, TimeSpan.FromSeconds(2));
            Assert.IsTrue(holder.Count == 4, "Expecting 4 connections in the list.");
            System.Threading.Thread.Sleep(3000);
            Assert.IsTrue(holder.Count == 0, "Expecting 0 connections in the list.");

            holder.HoldConnection(sc1, TimeSpan.FromSeconds(2));
            holder.HoldConnection(sc2, TimeSpan.FromSeconds(4));
            holder.HoldConnection(sc1, TimeSpan.FromSeconds(2));
            holder.HoldConnection(sc2, TimeSpan.FromSeconds(4));
            holder.HoldConnection(sc1, TimeSpan.FromSeconds(2));
            holder.HoldConnection(sc2, TimeSpan.FromSeconds(4));
            Assert.IsTrue(holder.Count == 2, "Expecting 2 connections in the list.");
            System.Threading.Thread.Sleep(3000);
            Assert.IsTrue(holder.Count == 1, "Expecting 1 connections in the list.");
            System.Threading.Thread.Sleep(2000);
            Assert.IsTrue(holder.Count == 0, "Expecting 0 connections in the list.");
        }
    }
}