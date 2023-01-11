using System;
using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    /// <summary>
    /// Unit tests for SynchronizationEvent
    /// </summary>
    [TestFixture]
    public class SynchronizationEventTests
    {
        [Test]
        public void SynchronizationEvent_TimeExpires_CallbackIsCalled()
        {
            bool synchronizationFired = false;
            SynchronizationEvent mySynchronizationEvent =
                new SynchronizationEvent(TimeSpan.FromTicks(1),
                    delegate(object sender, System.Timers.ElapsedEventArgs a) 
                    { synchronizationFired = true; }
                    );

            mySynchronizationEvent.Wait();
            Assert.IsTrue( synchronizationFired);
            Assert.IsTrue(mySynchronizationEvent.TimedOut);
        }

        [Test]
        public void SynchronizationEvent_ManuallyFired_CallbackIsCalled()
        {
            bool synchronizationFired = false;
            SynchronizationEvent mySynchronizationEvent =
                new SynchronizationEvent(TimeSpan.FromSeconds( 20),
                    delegate(object sender, System.Timers.ElapsedEventArgs a)
                    { synchronizationFired = true; }
                    );

            Assert.IsFalse(synchronizationFired);
            Assert.IsFalse(mySynchronizationEvent.TimedOut);

            mySynchronizationEvent.TimedOut = true;
            Assert.IsTrue(synchronizationFired);
            Assert.IsTrue(mySynchronizationEvent.TimedOut);
        }
    }
}