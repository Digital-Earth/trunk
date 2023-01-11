namespace PyxNet
{
    /// <summary>
    /// This class is used to forward a message though PyxNet by contacting all the hubs we
    /// know of and asking them to forward the message to a particular node.  The action
    /// continues until one of the hubs returns an acknowledgment that the message got though, 
    /// or if we run out of known hubs to contact.
    /// </summary>
    public class MessageRelayer : ProgressiveBroadcast
    {
        /// <summary>
        /// This is the trace tool that one should use for all things to do with a MessageRelayer.
        /// </summary>
        private Pyxis.Utilities.NumberedTraceTool<MessageRelayer> m_tracer
            = new Pyxis.Utilities.NumberedTraceTool<MessageRelayer>(Pyxis.Utilities.TraceTool.GlobalTraceLogEnabled);

        public new Pyxis.Utilities.NumberedTraceTool<MessageRelayer> Tracer
        {
            get
            {
                return m_tracer;
            }
        }

        #region Fields and properties

        #region MessageRelay

        /// <summary>
        /// The relay message to be sent.
        /// </summary>
        private readonly MessageRelay m_relay;

        /// <summary>
        /// The relay message to be sent.
        /// </summary>
        public MessageRelay Relay
        {
            get
            {
                return m_relay;
            }
        }

        #endregion

        #endregion

        #region Constructors

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="stack">The stack to act upon.</param>
        /// <param name="relay">The relay.</param>
        /// <param name="timeOut">The time to wait for an acknowledgement, in milliseconds.</param>
        public MessageRelayer(Stack stack, MessageRelay relay, int timeOut) : base(stack, timeOut)
        {
            m_relay = relay;
        }

        #endregion

        #region Methods

        /// <summary>
        /// Send to the connection.
        /// </summary>
        /// <param name="connection">The connection to send to.  If null, send to the stack.</param>
        /// <returns>True if successful.</returns>
        protected override bool Send(StackConnection connection)
        {
            if (null == connection)
            {
                // Pass the relay to the stack, and handle the acknowledgement.
                HandleAcknowledgement(null, Stack.ProcessMessageRelay(null, m_relay));
                return true;
            }

            // Send to the connection.
            return connection.SendMessage(m_relay.ToMessage());
        }

        /// <summary>
        /// Determines whether or not this sender can handle the acknowledgement.
        /// </summary>
        /// <param name="acknowledgement">The acknowledgement to handle.</param>
        /// <returns>True if this sender can handle the acknowledgement.</returns>
        protected override bool CanHandleAcknowledgement(ProgressiveBroadcastAcknowledgement acknowledgement)
        {
            return CanHandleAcknowledgement(acknowledgement as MessageRelayAcknowledgement);
        }

        /// <summary>
        /// Determines whether or not this sender can handle the acknowledgement.
        /// </summary>
        /// <param name="acknowledgement">The acknowledgement to handle.</param>
        /// <returns>True if this sender can handle the acknowledgement.</returns>
        private bool CanHandleAcknowledgement(MessageRelayAcknowledgement acknowledgement)
        {
            return (null != acknowledgement) && (m_relay.Guid == acknowledgement.RelayGuid);
        }

        /// <summary>
        /// Handle an acknowledgement event coming from one of the connections in the stack.
        /// </summary>
        /// <param name="connectionIndex">The index of the stack connection.</param>
        /// <param name="acknowledgement">The acknowledgement.</param>
        protected override void HandleAcknowledgement(StackConnection connection, ProgressiveBroadcastAcknowledgement acknowledgement)
        {
            // Only continue if a dead end was found; otherwise, we've found it.
            if (acknowledgement.IsDeadEnd)
            {
                if (connection != null)
                {
                    Tracer.DebugWriteLine("Acknowledgement from {0} was a dead end, continuing relay.", connection.RemoteNodeInfo.FriendlyName);
                }
                else
                {
                    Tracer.DebugWriteLine("Acknowledgement from self was a dead end, continuing relay.");
                }
                base.HandleAcknowledgement(connection, acknowledgement);
            }
            else
            {
                // We got the relay message through.  Stop trying any other hubs.
                if (connection != null)
                {
                    Tracer.DebugWriteLine("Acknowledgement from {0} confirmed delivery, stopping relay.", connection.RemoteNodeInfo.FriendlyName);
                }
                else
                {
                    Tracer.DebugWriteLine("Acknowledgement from self confirmed delivery, stopping relay.  Why did we relay a message to ourselves?");
                }
                Stop();
            }
        }

        /// <summary>
        /// Handle an acknowledgement message.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="args"></param>
        private void HandleAcknowledgementMessage(object sender, MessageHandlerCollection.MessageReceivedEventArgs args)
        {
            MessageRelayAcknowledgement acknowledgement = MessageRelayAcknowledgement.FromMessage(args.Message);
            StackConnection sendingConnection = args.Context.Sender;
            HandleAcknowledgement(sendingConnection, acknowledgement);
        }

        /// <summary>
        /// Connect the handlers to the stack events.
        /// </summary>
        protected override void ConnectHandlers()
        {
            lock (Stack)
            {
                if (!AreHandlersConnected)
                {
                    // Handle message relay acknowledgement message in stack.
                    Stack.RegisterHandler(MessageRelayAcknowledgement.MessageID, HandleAcknowledgementMessage);

                    base.ConnectHandlers();
                }
            }
        }

        /// <summary>
        /// Disconnect the handlers from the stack events.
        /// </summary>
        protected override void DisconnectHandlers()
        {
            lock (Stack)
            {
                // Unhook handler from stack's message relay acknowledgement event.
                Stack.UnregisterHandler(MessageRelayAcknowledgement.MessageID, HandleAcknowledgementMessage);

                base.DisconnectHandlers();
            }
        }

        #endregion
    }
}