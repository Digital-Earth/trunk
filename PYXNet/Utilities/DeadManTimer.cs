/******************************************************************************
DeadManTimer.cs

begin      : March 26, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Utilities
{
    public class DeadManTimer
    {
        /// <summary>
        /// Internal timer used to implement dead man timer.
        /// </summary>
        private readonly System.Timers.Timer m_timer;

        /// <summary>
        /// Flags the timer as being alive.  This will postpone the "timeout", for a while.
        /// </summary>
        public void KeepAlive()
        {
            lock (m_timer)
            {
                if (m_timer.Enabled)
                {
                    m_timer.Stop();
                    m_timer.Start();
                }
            }
        }

        /// <summary>
        /// Stops the timer.
        /// </summary>
        public void Stop()
        {
            lock (m_timer)
            {
                m_timer.Stop();
            }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="DeadManTimer"/> class.
        /// </summary>
        /// <param name="timeout">The timeout.</param>
        public DeadManTimer(TimeSpan timeout)
        {
            System.Timers.Timer timer = new System.Timers.Timer(timeout.TotalMilliseconds);
            timer.Elapsed += OnElapsed;
            timer.AutoReset = false;
            timer.Start();
            m_timer = timer;
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="DeadManTimer"/> class.
        /// </summary>
        /// <param name="timeout">The timeout.</param>
        /// <param name="onTimeout">The delegate to call when the timer times out.</param>
        public DeadManTimer(TimeSpan timeout, System.Timers.ElapsedEventHandler onTimeout)
            : this( timeout)
        {
            m_timer.Elapsed += onTimeout;
        }

        #region Elapsed Event

        /// <summary> Event handler for Elapsed. </summary>
        public event EventHandler<System.Timers.ElapsedEventArgs> Elapsed;

        /// <summary>
        /// Raises the Elapsed event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="thedecimal"></param>
        public void OnElapsed(object sender, System.Timers.ElapsedEventArgs args)
        {
            m_hasElapsed = true;

            EventHandler<System.Timers.ElapsedEventArgs> handler = Elapsed;
            if (handler != null)
            {
                handler(sender, args);
            }
        }

        private bool m_hasElapsed = false;

        /// <summary>
        /// Gets a value indicating whether this instance has elapsed.
        /// </summary>
        /// <value>
        /// 	<c>true</c> if this instance has elapsed; otherwise, <c>false</c>.
        /// </value>
        public bool HasElapsed
        {
            get { return m_hasElapsed; }
        }

        #endregion Elapsed Event
    }

    
    namespace Test
    {
        using NUnit.Framework;

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
            public void CompleteTest()
            {
                Assert.IsFalse(RunTest(100, 25, 200));
                Assert.IsTrue(RunTest(100, 250, 500));
            }
        }
    }

}
