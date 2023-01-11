/******************************************************************************
SimpleTimer.cs

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
    /// A class that will hold on to pending timers to ensure they don't 
    /// get GC'ed before they elapse.
    /// </summary>
    public class SimpleTimer
    {
        /// <summary>
        /// Stores all of the pending timers, to ensure they don't 
        /// get GC'ed too quickly.
        /// </summary>
        private static readonly List<System.Timers.Timer> s_pendingTimers =
            new List<System.Timers.Timer>();

        /// <summary>
        /// The actual timer.
        /// </summary>
        private readonly System.Timers.Timer m_timer;

        /// <summary>
        /// Gets the timer.
        /// </summary>
        /// <value>The timer.</value>
        public System.Timers.Timer Timer
        {
            get { return m_timer; }
        }

        /// <summary>
        /// Creates a SimpleTimer with the specified timeout.  Note that 
        /// this timer will not be started, but it will start 
        /// automatically when an <see cref="Elapsed"/> event handler is 
        /// attached.
        /// </summary>
        /// <param name="timeout">The timeout.</param>
        /// <returns></returns>
        public static SimpleTimer Create(TimeSpan timeout)
        {
            return new SimpleTimer(timeout);
        }

        /// <summary>
        /// Creates a SimpleTimer with the specified timeout, and attaches
        /// the specified handler.  Note that the timer is automatically
        /// started.
        /// </summary>
        /// <param name="timeout">The timeout.</param>
        /// <param name="handler">
        /// The handler to call when the timeout has elapsed.
        /// </param>
        /// <returns></returns>
        public static SimpleTimer Create(TimeSpan timeout,
            System.Timers.ElapsedEventHandler handler)
        {
            return new SimpleTimer(timeout, handler);
        }

        /// <summary>
        /// Occurs when the interval elapses.  (Maps to the actual timer's
        /// Elapsed event.)
        /// </summary>
        public event System.Timers.ElapsedEventHandler Elapsed
        {
            add
            {
                m_timer.Elapsed += value;
                if (!m_timer.Enabled)
                {
                    m_timer.Start();
                }
            }
            remove
            {
                m_timer.Elapsed -= value;
            }
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="SimpleTimer"/> class.
        /// Note that this timer will not be started, but it will start 
        /// automatically when an <see cref="Elapsed"/> event handler is 
        /// attached.
        /// </summary>
        /// <param name="timeout">The timeout.</param>
        public SimpleTimer(TimeSpan timeout)
        {
            m_timer = new System.Timers.Timer(timeout.TotalMilliseconds);
            m_timer.AutoReset = false;
            lock (s_pendingTimers)
            {
                s_pendingTimers.Add(m_timer);
            }
            m_timer.Elapsed += delegate(object sender, System.Timers.ElapsedEventArgs e)
            {
                lock (s_pendingTimers)
                {
                    s_pendingTimers.Remove(m_timer);
                }
            };
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="SimpleTimer"/> class
        /// with the specified timeout, and attaches the specified handler.  
        /// Note that the timer is automatically started.
        /// </summary>
        /// <param name="timeout">The timeout.</param>
        /// <param name="handler">
        /// The handler to call when the timeout has elapsed.
        /// </param>
        public SimpleTimer(TimeSpan timeout, System.Timers.ElapsedEventHandler handler)
            :
            this(timeout)
        {
            this.Elapsed += handler;
        }
    }
}

//TODO: unit test.