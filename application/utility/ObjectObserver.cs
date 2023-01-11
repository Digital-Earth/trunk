/******************************************************************************
ObjectObserver.cs

begin      : January 10, 2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace ApplicationUtility
{
    /// <summary>
    /// This class observes a single PYXIS notifier object for changes. The class
    /// uses delegates to pass on the messages to other C# classes without
    /// any pre-processing.  An ObjectObserver object must call 
    /// ReleaseObject() before it goes out of scope.
    /// </summary>
    public class ObjectObserver : Observer
    {
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="observerName">
        /// The descriptive name of the object. Used in logging.
        /// </param>
        public ObjectObserver(string observerName)
        {
            m_name = observerName;
        }

        /// <summary>
        /// The descriptive name of the observer object.
        /// </summary>
        public string Name
        {
            get { return m_name; }
            set { m_name = value; }
        }
        string m_name = "Not Named";

        /// <summary>
        /// The object that is being observed for notifications.
        /// </summary>
        private Notifier m_notifierObject;

        /// <summary>
        /// Set the object that this ObjectObserver should monitor for changes.
        /// </summary>
        /// <param name="notifier">
        /// The object to observe for changes.
        /// </param>
        public void SetObject(Notifier notifier)
        {
            ReleaseObject();

            // store a copy of the pointer.
            m_notifierObject = notifier;
            if (m_notifierObject != null)
            {
                Trace.info(string.Format(
                    "Setting notifier object '{0}'  for ObjectObserver '{1}'.",
                    notifier.getNotifierDescription(), Name));

                m_notifierObject.attach(this);
            }
            else
            {
                Trace.info(string.Format(
                    "Removing notifier from ObjectObserver '{0}'.", Name));
            }
        }

        /// <summary>
        /// Detach from any observed objects. If the observer is not attached
        /// to any objects the call performs no action.
        /// </summary>
        public void ReleaseObject()
        {
            if (m_notifierObject != null)
            {
                Trace.info(
                    "Releasing notifier object '" +
                    m_notifierObject.getNotifierDescription() +
                    "' from ObjectObserver.");
                m_notifierObject.detach(this);
                m_notifierObject = null;
            }
        }

        /// <summary>
        /// Definition of of the callback function delegate.
        /// </summary>
        /// <param name="spEvent">The event that has occurred</param>
        public delegate void NotificationCallback(NotifierEvent_SPtr spEvent);

        /// <summary>
        /// The callback delegate associated with the interpreter.
        /// </summary>
        private NotificationCallback m_notifyCallback;

        /// <summary>
        /// Associate a particular callback with the observer. This
        /// method is called every time the observed object sends a
        /// notification.
        /// </summary>
        /// <param name="callback">
        /// The notification callback method
        /// </param>
        public void SetNotificationCallback(NotificationCallback callback)
        {
            Trace.info("Setting callback for object observer.");
            m_notifyCallback = callback;
        }

        /// <summary>
        /// Process the notification from an observed object.
        /// </summary>
        /// <param name="spEvent">
        /// The event that was issued by the notifier.
        /// </param>
        protected override void updateObserverImpl(NotifierEvent_SPtr spEvent)
        {
            // the update observer call
            if (m_notifyCallback != null)
            {
                m_notifyCallback(spEvent);
            }
        }
        
        /// <summary>
        /// Text description of the observer class used for logging.
        /// </summary>
        /// <returns>
        /// The text description of the class.
        /// </returns>
        public override string getObserverDescription()
        {
            return string.Format(Properties.Resources.ObjectObserverName, m_name);
        }        
    }
}
