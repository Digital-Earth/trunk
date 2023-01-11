using System;
using System.Text;
using Pyxis.Utilities;

namespace PyxNet.Diagnostics
{
    public class TrafficListener 
    {
        public class RecordedMessage
        {
            public Message Message { get; private set;  }
            public NodeInfo Source { get; private set; }
            public NodeInfo Destination { get; private set; }
            public DateTime TimeStamp { get; private set;  }

            //Used by TrafficListenerForm to show additional information about the message
            public string Description { get; set; }

            public RecordedMessage(Message message ,StackConnection connection, Stack stack,bool outgoingMessage)
            {
                if (message == null)
                {
                    System.Diagnostics.Trace.WriteLine("message is null, why?");
                }
                Message = message;
                if (outgoingMessage)
                {
                    Source = stack.NodeInfo;
                    if (connection != null)
                    {
                        Destination = connection.RemoteNodeInfo;
                    }
                }
                else
                {
                    Destination = stack.NodeInfo;
                    if (connection != null)
                    {
                        Source = connection.RemoteNodeInfo;
                    }
                }
                TimeStamp = DateTime.Now;
            }
        }

        public DynamicList<RecordedMessage> Messages { get; set; }
        public int MaxMessages { get; set; }

        private bool m_recording = false;
        public bool Recording
        {
            get { return m_recording; }
            set
            {
                if (m_recording != value)
                {
                    if (value)
                    {
                        StartRecording();
                    }
                    else
                    {
                        StopRecording();
                    }
                }
            }
        }

        private Stack m_stack;

        public TrafficListener(Stack stack)
        {
            m_stack = stack;
            Messages=  new DynamicList<RecordedMessage>();
            MaxMessages = 1000;
        }

        private void OnOnAnyMessage(object sender, Stack.AnyMessageEventArgs anyMessageEventArgs)
        {   
            //record message
            Messages.Add(new RecordedMessage(anyMessageEventArgs.Message,anyMessageEventArgs.Connection,anyMessageEventArgs.Stack,false));

            //limit ammount
            if (Messages.Count > MaxMessages)
            {
                Messages.RemoveAt(0);
            }
        }

        private void OnSendingMessageToConnection(Stack stack, StackConnection connection, Message message)
        {
            //record message
            Messages.Add(new RecordedMessage(message,connection, stack, true));

            //limit ammount
            if (Messages.Count > MaxMessages)
            {
                Messages.RemoveAt(0);
            }
        }

        public void Dispose()
        {
            StopRecording();
        }

        public void StartRecording()
        {
            if (!m_recording)
            {
                Messages.Clear();
                m_stack.OnAnyMessage += OnOnAnyMessage;
                m_stack.OnSendingMessageToConnection += OnSendingMessageToConnection;
                m_stack.OnSendingMessageToAll += OnSendingMessageToConnection; 
                m_recording = true;
            }
        }

        public void StopRecording()
        {
            if (m_recording)
            {
                m_stack.OnAnyMessage -= OnOnAnyMessage;
                m_stack.OnSendingMessageToConnection -= OnSendingMessageToConnection;
                m_stack.OnSendingMessageToAll -= OnSendingMessageToConnection; 
                m_recording = false;
            }
        }
    }
}
