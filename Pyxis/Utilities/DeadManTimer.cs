/******************************************************************************
DeadManTimer.cs

begin      : March 26, 2008
copyright  : (c) 2008 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace Pyxis.Utilities
{
    /// <summary>
    /// Informs the client code when a specified period of time elapses.
    /// </summary>
    public class DeadManTimer
    {
        /// <summary>
        /// Internal timer used to implement dead man timer.
        /// </summary>
        private readonly System.Timers.Timer m_timer;

        private DateTime m_startTime;
        private TimeSpan m_maxElapsedTime = TimeSpan.Zero;

        public TimeSpan MaxElapsedTime
        {
            get { return m_maxElapsedTime; }
        }

        private void Start()
        {
            m_startTime = DateTime.Now;
            m_timer.Start();
        }

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

                    // Check to see if we've found the greatest elapsed time...
                    TimeSpan elapsedTime = DateTime.Now - m_startTime;
                    if (elapsedTime > m_maxElapsedTime)
                    {
                        m_maxElapsedTime = elapsedTime;
                    }

                    Start();
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
            m_timer = timer;
            Start();
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
        private Pyxis.Utilities.EventHelper<System.Timers.ElapsedEventArgs> m_elapsed =
            new EventHelper<System.Timers.ElapsedEventArgs>();

        public event EventHandler<System.Timers.ElapsedEventArgs> Elapsed
        {
            add
            {
                m_elapsed.Add( value);
            }
            remove
            {
                m_elapsed.Remove( value);
            }
        }

        /// <summary>
        /// Raises the Elapsed event.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="thedecimal"></param>
        public void OnElapsed(object sender, System.Timers.ElapsedEventArgs args)
        {
            m_hasElapsed = true;

            m_elapsed.Invoke(sender, args);
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
}
