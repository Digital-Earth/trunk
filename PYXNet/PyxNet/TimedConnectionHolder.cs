using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet
{
    /// <summary>
    /// A collection that holds on to a list of connections for a period of time.  This is
    /// used to help keep temporary connections alive for a period of time after they have 
    /// last bgeen used.
    /// </summary>
    public class TimedConnectionHolder
    {
        /// <summary>
        /// Local class to hang on to a connection and the time that we want it to go away.
        /// </summary>
        private class ConnectionWithTimeout : IComparable
        {
            public DateTime m_expireTime;
            public StackConnection m_connection;

            public ConnectionWithTimeout(StackConnection connection, DateTime expireTime) 
            {
                m_expireTime = expireTime;
                m_connection = connection;
            }
        
            public int CompareTo(object obj)
            {
                ConnectionWithTimeout compareTo = obj as ConnectionWithTimeout;
                if (compareTo == null)
                    throw (new ArgumentException("can not compare with a different class"));

                return m_expireTime.CompareTo(compareTo.m_expireTime);
            }
        }

        /// <summary>
        /// The list of connections that we are holding.
        /// </summary>
        private List<ConnectionWithTimeout> m_connectionList = new List<ConnectionWithTimeout>();

        /// <summary>
        /// A timer set to go off at the soonest time that we need to let go of a connection.
        /// </summary>
        private System.Timers.Timer m_nextExpire = new System.Timers.Timer();

        /// <summary>
        /// A simple object that we lock to make this class thread safe.
        /// </summary>
        private System.Object m_lock = new System.Object();

        /// <summary>
        /// Constructor
        /// </summary>
        public TimedConnectionHolder()
        {
            m_nextExpire.Elapsed += nextExpire_Elapsed;
        }

        /// <summary>
        /// Handle the timer going off, get rid of any entries in the list that are now expired,
        /// and reset the timer for the next soonest expiration time.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void nextExpire_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            lock (m_lock)
            {
                while (m_connectionList.Count > 0 && m_connectionList[0].m_expireTime <= DateTime.Now)
                {
                    m_connectionList.RemoveAt(0);
                }
                UpdateTimer();
            }
        }

        /// <summary>
        /// Update the timer so that it will elapse at the soonest expiration time.
        /// </summary>
        private void UpdateTimer()
        {
            lock (m_lock)
            {
                m_nextExpire.Enabled = false;
                if (m_connectionList.Count > 0)
                {
                    TimeSpan timeToFirst = 
                        m_connectionList[0].m_expireTime.Subtract(DateTime.Now);

                    int millisecondsToFirst = timeToFirst.Milliseconds;

                    if (millisecondsToFirst <= 0)
                    {
                        millisecondsToFirst = 1;
                    }

                    m_nextExpire.Interval = millisecondsToFirst;
                    m_nextExpire.Enabled = true;
                }
            }
        }

        /// <summary>
        /// Call to have the connection kept alive for a length of time.  Just keeps
        /// a reference to the connection for a period of time.
        /// </summary>
        /// <param name="connection">The connection that you want to hold.</param>
        /// <param name="holdTime">The length of time that the connection is to be held.</param>
        public void HoldConnection(StackConnection connection, TimeSpan holdTime)
        {
            // bail out if we don't have sensible parameters.
            if (connection == null || holdTime == TimeSpan.Zero)
            {
                return;
            }

            DateTime expireTime = DateTime.Now.Add(holdTime);
            ConnectionWithTimeout holdConnection =
                new ConnectionWithTimeout(connection, expireTime);
            lock (m_lock)
            {
                for (int index = 0; index < m_connectionList.Count; ++index)
                {
                    // if it is in the list already
                    if (m_connectionList[index].m_connection == connection)
                    {
                        // and it is to be held longer than we asked
                        if (m_connectionList[index].m_expireTime >= expireTime)
                        {
                            // we are done
                            return;
                        }
                        
                        // get rid of the shorter hold time
                        m_connectionList.RemoveAt(index);
                        break;
                    }
                }

                // add it to the list and sort.
                System.Diagnostics.Debug.Assert(
                    !m_connectionList.Contains(holdConnection),
                    "If the connection was already in there, this code should either have returned or removed the connection.");
                m_connectionList.Add(holdConnection);
                m_connectionList.Sort();
                UpdateTimer();
            }
        }

        /// <summary>
        /// Remove a connection from the holding list.
        /// </summary>
        /// <param name="connection"></param>
        public void RemoveConnection(StackConnection connection)
        {
            lock (m_lock)
            {
                m_connectionList.RemoveAll(delegate(ConnectionWithTimeout cwt)
                {
                    return (cwt.m_connection == connection);
                });
            }
        }

        /// <summary>
        /// How many connections we are holding on to
        /// </summary>
        public int Count
        {
            get
            {
                lock (m_lock)
                {
                    return m_connectionList.Count;
                }
            }
        }

        /// <summary>
        /// Clear the list of all connections.
        /// </summary>
        public void Clear()
        {
            lock (m_lock)
            {
                m_connectionList.Clear();
                UpdateTimer();
            }
        }
    }
}