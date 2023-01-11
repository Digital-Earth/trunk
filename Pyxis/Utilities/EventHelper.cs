/******************************************************************************
EventHelper.cs

begin      : February 17, 2010
copyright  : (c) 2010 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;

namespace Pyxis.Utilities
{
    /// <summary>
    /// Helper class that wraps up an event, along with support for locking.
    /// </summary>
    /// <typeparam name="TEventArgs">The type of the event args.</typeparam>
    public class EventHelper<TEventArgs> where TEventArgs : EventArgs
    {
        public object m_lock = new object();
        private event EventHandler<TEventArgs> m_event;

        /// <summary>
        /// Initializes a new instance of the <see cref="EventHelper&lt;TEventArgs&gt;"/> class.
        /// </summary>
        public EventHelper()
        {
        }

        /// <summary>
        /// Initializes a new instance of the <see cref="EventHelper&lt;TEventArgs&gt;"/> 
        /// class by duplicating an existing object.
        /// </summary>
        /// <param name="copy">The object to copy.</param>
        public EventHelper(EventHelper<TEventArgs> copy)
        {
            lock (copy.m_lock)
            {
                foreach (var item in copy.m_event.GetInvocationList())
                {
                    Add(item as EventHandler<TEventArgs>);
                }
            }
        }
        /// <summary>
        /// Safely adds to the event handler.
        /// </summary>
        /// <param name="newValue">The new value.</param>
        public void Add( EventHandler<TEventArgs> newValue)
        {
            lock (m_lock)
            {
                m_event += newValue;
            }
        }

        /// <summary>
        /// Safely removes a delegate the event handler.
        /// </summary>
        /// <param name="newValue">The new value.</param>
        public void Remove(EventHandler<TEventArgs> handlerToRemove)
        {
            lock (m_lock)
            {
                m_event -= handlerToRemove;
            }
        }

        /// <summary>
        /// Safely invokes the handler with the given arguments.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="e">The <see cref="TEventArgs"/> instance containing the event data.</param>
        public void Invoke( object sender, TEventArgs e)
        {
            EventHandler<TEventArgs> handler;
            lock (m_lock)
            {
                handler = m_event;
            }
            if (handler != null)
            {
                handler(sender, e);
            }
        }
    }
}
