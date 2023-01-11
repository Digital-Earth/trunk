using System;
using NUnit.Framework;

namespace Pyxis.Utilities.Test
{
    /// <summary>
    /// Unit tests for DeadManTimer
    /// </summary>
    [TestFixture]
    public class DeadManTimerTests
    {
        /// <summary>
        /// Runs the test.
        /// </summary>
        /// <param name="timeOut">The deadman timer's time out, in ms.</param>
        /// <param name="pingInterval">The ping interval, in ms.</param>
        /// <param name="timeToRun">The total time to run, in ms.</param>
        /// <returns></returns>
        private bool RunTest(int timeOut, int pingInterval, int timeToRun)
        {
            bool timedOut = false;
            int keepAlivesFired = 0;
            DeadManTimer myDeadManTimer = new DeadManTimer(
                TimeSpan.FromMilliseconds(timeOut),
                delegate(object o, System.Timers.ElapsedEventArgs a)
                {
                    timedOut = true;
                }
                );

            System.Timers.Timer pingTimer = new System.Timers.Timer(pingInterval);
            pingTimer.Elapsed +=
                delegate(object pingObject, System.Timers.ElapsedEventArgs pingArgs)
                {
                    ++keepAlivesFired;
                    myDeadManTimer.KeepAlive();
                };
            pingTimer.AutoReset = true;
            pingTimer.Start();

            SynchronizationEvent testTimer = 
                new SynchronizationEvent(TimeSpan.FromMilliseconds(timeToRun));

            testTimer.Wait();
            GC.KeepAlive( pingTimer);

            return timedOut;
        }

        [Test]
        public void DeadManTimerFires()
        {
            Assert.IsFalse(RunTest(100, 25, 200));
        }

        [Test]
        public void DeadManTimerTimesOut()
        {
            Assert.IsTrue(RunTest(100, 250, 500));
        }
    }
}