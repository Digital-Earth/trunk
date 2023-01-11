/******************************************************************************
SynchronizationEvent.cs

begin      : March 18, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Utilities
{
    /// <summary>
    /// A wrapper for simple synchronization tasks.  It incorporates a timeout
    /// to permit asynch operations to not run too long.
    /// </summary>
    /// <remarks>
    /// Caution!  Do not lock this class externally, since it will likely lead
    /// to deadlock.
    /// </remarks>
    public class SynchronizationEvent
    {
        private bool m_pulseCalled = false;
        private bool m_timedOut = false;

        public event EventHandler<System.Timers.ElapsedEventArgs> Elapsed;

        /// <summary>
        /// Raises the <see cref="E:Elapsed"/> event.
        /// </summary>
        /// <param name="e">
        /// The <see cref="System.Timers.ElapsedEventArgs"/> instance containing 
        /// the event data.
        /// </param>
        private void OnElapsed(System.Timers.ElapsedEventArgs e)
        {
            EventHandler<System.Timers.ElapsedEventArgs> elapsed = Elapsed;
            if (elapsed != null)
            {
                elapsed(this, e);
            }
        }

        /// <summary>
        /// Pulses this instance.  Call this from a worker thread/task to signal
        /// the event owner to continue.
        /// </summary>
        public void Pulse()
        {
            lock (this)
            {
                m_pulseCalled = true;
                System.Threading.Monitor.Pulse(this);
            }
        }

        /// <summary>
        /// Waits this instance.  Call this on the owner thread to wait for either 
        /// a timout or a pulse from a worker thread.
        /// </summary>
        public void Wait()
        {
            lock (this)
            {
                if (!m_pulseCalled)
                {
                    System.Threading.Monitor.Wait(this);
                }
            }
        }

        /// <summary>
        /// Tests to see if a wait will succeed immediately.  
        /// </summary>
        /// <returns>True iff a wait will return immediately.</returns>
        public bool TryWait()
        {
            return m_pulseCalled;
        }

        /// <summary>
        /// Gets a value indicating whether the operation timed out.
        /// </summary>
        /// <value><c>true</c> if [timed out]; otherwise, <c>false</c>.</value>
        public bool TimedOut
        {
            get { return m_timedOut; }
            set
            {
                if (value == true)
                {
                    OnElapsed(null);
                }
            }
        }

        /// <summary>
        /// Makes sure that the event will signal after the elapsed time
        /// if it has not signalled before then.
        /// </summary>
        /// <param name="timeBeforePulse">The max time before pulse.</param>
        public void AddTimedPulse(TimeSpan timeBeforePulse)
        {
            Elapsed +=
                delegate(object timeout_sender, System.Timers.ElapsedEventArgs timeout_e)
                {
                    if (!m_pulseCalled)
                    {
                        m_timedOut = true;
                        Pulse();
                    }
                };

            new PyxNet.Utilities.SimpleTimer(timeBeforePulse,
                delegate(object timeout_sender, System.Timers.ElapsedEventArgs timeout_e)
                {
                    OnElapsed(timeout_e);
                });
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="SynchronizationEvent"/> 
        /// class.  This object will never time out.
        /// </summary>
        public SynchronizationEvent()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="SynchronizationEvent"/> 
        /// class.  This object will signal within the given timespan (check 
        /// <see cref="TimedOut"/> to see if it timed out.
        /// </summary>
        /// <param name="maxTimeBeforePulse">The max time before pulse.</param>
        public SynchronizationEvent(TimeSpan maxTimeBeforePulse)
        {
            AddTimedPulse(maxTimeBeforePulse);
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="SynchronizationEvent"/> 
        /// class.  This object will signal within the given timespan (check 
        /// <see cref="TimedOut"/> to see if it timed out.
        /// </summary>
        /// <param name="maxTimeBeforePulse">The max time before pulse.</param>
        /// <param name="onTimeout">The function to call upon timeout.</param>
        public SynchronizationEvent(TimeSpan maxTimeBeforePulse,
            System.Timers.ElapsedEventHandler onTimeout)
        {
            Elapsed +=
                delegate(object timeout_sender, System.Timers.ElapsedEventArgs timeout_e)
                {
                    if (!m_pulseCalled)
                    {
                        m_timedOut = true;
                        onTimeout(timeout_sender, timeout_e);
                        Pulse();
                    }
                };

            new PyxNet.Utilities.SimpleTimer(maxTimeBeforePulse,
                delegate(object timeout_sender, System.Timers.ElapsedEventArgs timeout_e)
                {
                    OnElapsed(timeout_e);
                });
        }
    }
}


//TODO: unit test.