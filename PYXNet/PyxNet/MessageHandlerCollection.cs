using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet
{
    public class MessageContext {
        private StackConnection m_sender;

        public StackConnection Sender
        {
            get { return m_sender; }
            set { m_sender = value; }
        }

        public MessageContext(StackConnection sender)
        {
            Sender = sender;
        }
    }

    /// <summary>
    /// This is a generic message-handler class.
    /// </summary>
    public class MessageHandlerCollection
    {
        #region Tracer

        private readonly Pyxis.Utilities.NumberedTraceTool<MessageHandlerCollection> m_tracer =
           new Pyxis.Utilities.NumberedTraceTool<MessageHandlerCollection>(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

        public Pyxis.Utilities.NumberedTraceTool<MessageHandlerCollection> Tracer
        {
            get
            {
                return m_tracer;
            }
        }

        #endregion

        public class MessageReceivedEventArgs : EventArgs
        {
            private Message m_Message;

            public Message Message
            {
                get { return m_Message; }
            }

            private MessageContext m_Context;

            public MessageContext Context
            {
                get { return m_Context; }
            }

            public MessageReceivedEventArgs(Message message, MessageContext context)
            {
                m_Message = message;
                m_Context = context;
            }
        }

        #region UnhandledMessage Event

        public event EventHandler<MessageReceivedEventArgs> OnUnhandledMessage
        {
            add
            {
                m_OnUnhandledMessage.Add(value);
            }
            remove
            {
                m_OnUnhandledMessage.Remove(value);
            }
        }
        private Pyxis.Utilities.EventHelper<MessageReceivedEventArgs> m_OnUnhandledMessage = new Pyxis.Utilities.EventHelper<MessageReceivedEventArgs>();

        #endregion UnhandledMessage Event

        public MessageHandlerCollection()
        {
        }

        //TODO: Move these comments up into the handler/event class.
        /// <summary>
        /// Generic "message handler" - this function will be called whenever
        /// the corresponding message is received.
        /// </summary>
        /// <param name="msg">The message received.</param>
        /// <param name="context">
        /// The "context" in which the message was received - typically this 
        /// is the tcp connection.
        /// </param>
        //        public delegate void MessageHandler( Message msg, IMessageContext context);

        /// <summary>
        /// This dictionary maps a message type string to a "message receipt handler" delegate,
        /// which handles the message.
        /// </summary>
        private readonly Dictionary<string, EventHandler<MessageReceivedEventArgs>> m_handlerTable =
            new Dictionary<string, EventHandler<MessageReceivedEventArgs>>();

        /// <summary>
        /// Register a handler with the handler collection, mapped to a message identifier.
        /// </summary>
        /// <param name="messageIdentifier">The message identifier to map to a handler.</param>
        /// <param name="handler">The handler.</param>
        public void RegisterHandler(string messageIdentifier, EventHandler<MessageReceivedEventArgs> handler)
        {
            if (messageIdentifier == null)
            {
                throw new ArgumentNullException("messageIdentifier",
                    "Must pass a four-character message identifier.");
            }
            if (messageIdentifier.Length != 4)
            {
                throw new ArgumentException("messageIdentifier",
                    "Must pass a four-character message identifier.");
            }

            lock (m_handlerTable)
            {
                if (m_handlerTable.ContainsKey(messageIdentifier))
                {
                    m_handlerTable[messageIdentifier] += handler;
                }
                else
                {
                    m_handlerTable.Add(messageIdentifier, handler);
                }
            }
        }

        /// <summary>
        /// Unregister a handler with the handler collection, mapped to a message identifier.
        /// </summary>
        /// <param name="messageIdentifier">The message identifier to unmap from.</param>
        /// <param name="handler">The handler you wish to unmap.</param>
        public void UnregisterHandler(string messageIdentifier, EventHandler<MessageReceivedEventArgs> handler)
        {
            if (messageIdentifier == null)
            {
                throw new ArgumentNullException("messageIdentifier",
                    "Must pass a four-character message identifier.");
            }
            if (messageIdentifier.Length != 4)
            {
                throw new ArgumentException("messageIdentifier",
                    "Must pass a four-character message identifier.");
            }

            lock (m_handlerTable)
            {
                if (m_handlerTable.ContainsKey(messageIdentifier))
                {
                    m_handlerTable[messageIdentifier] -= handler;
                    if (null == m_handlerTable[messageIdentifier])
                    {
                        m_handlerTable.Remove(messageIdentifier);
                    }
                }
            }
        }

        /// <summary>
        /// Unregister all handlers.
        /// </summary>
        public void UnregisterHandlers()
        {
            lock (m_handlerTable)
            {
                m_handlerTable.Clear();
            }
        }

        /// <summary>
        /// Simple test to see if this collection contains a handler for
        /// this message.
        /// </summary>
        /// <param name="msg"></param>
        /// <returns></returns>
        public bool CanHandleMessage(Message msg)
        {
            if (msg == null)
            {
                throw new ArgumentNullException("msg");
            }

            lock (m_handlerTable)
            {
                return (m_handlerTable.ContainsKey(msg.Identifier));
            }
        }

        /// <summary>
        /// Handle a message.
        /// </summary>
        /// <param name="sender">The sender.</param>
        /// <param name="args">The "message received" event arguments.</param>
        public void HandleMessage(object sender, MessageReceivedEventArgs args)
        {
            if (args == null)
            {
                throw new ArgumentNullException("args");
            }

            EventHandler<MessageReceivedEventArgs> handler;
            String strFromNode = "unknown sender";
            if (args.Context != null && args.Context.Sender != null)
            {
                strFromNode = args.Context.Sender.ToString();
            }
            m_tracer.DebugWriteLine("Trying to handle message ID {0} from {1}", args.Message.Identifier, strFromNode);
            lock (m_handlerTable)
            {
                // Will set handler to null if not found.
                m_handlerTable.TryGetValue(args.Message.Identifier, out handler);
            }
            if (handler != null)
            {
                m_tracer.DebugWriteLine("Got non-null handler for {0}; calling.", args.Message.Identifier);
                handler(sender, args);
            }
            else
            {
                m_tracer.DebugWriteLine("Got null handler for {0}, meaning it is unhandled.", args.Message.Identifier);
                m_OnUnhandledMessage.Invoke(sender, args);
            }
        }
    }
}
