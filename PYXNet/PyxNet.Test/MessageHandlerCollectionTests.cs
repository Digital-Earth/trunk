using System;
using NUnit.Framework;

namespace PyxNet.Test
{
    internal class HandlerEmbedded
    {
        private MessageHandlerCollection m_handlers = new MessageHandlerCollection();

        public event EventHandler<MessageHandlerCollection.MessageReceivedEventArgs> OnUnhandledMessage
        {
            add
            {
                m_handlers.OnUnhandledMessage += value;
            }
            remove
            {
                m_handlers.OnUnhandledMessage -= value;
            }
        }

        #region Constructors

        public HandlerEmbedded()
        {
        }

        public HandlerEmbedded(HandlerEmbedded internalHandler)
        {
            if (internalHandler != null)
            {
                internalHandler.OnUnhandledMessage += m_handlers.HandleMessage;
            }
        }

        #endregion

        public void RegisterHandler(string messageIdentifier,
            EventHandler<MessageHandlerCollection.MessageReceivedEventArgs> handler)
        {
            m_handlers.RegisterHandler(messageIdentifier, handler);
        }

        public void HandleMessage(Message m)
        {
            m_handlers.HandleMessage(this, new MessageHandlerCollection.MessageReceivedEventArgs(m, null));
        }
    }

    internal class HandlerInherited : MessageHandlerCollection
    {
        #region Constructors

        public HandlerInherited()
        {
        }

        public HandlerInherited(MessageHandlerCollection internalHandler)
        {
            if (internalHandler != null)
            {
                internalHandler.OnUnhandledMessage += HandleMessage;
            }
        }

        #endregion

        public void HandleMessage(Message m)
        {
            HandleMessage(this, new MessageHandlerCollection.MessageReceivedEventArgs(m, null));
        }
    }

    [TestFixture]
    public class MessageHandlerCollectionTests
    {
        [Test]
        public void SingleMessageHandler()
        {
            HandlerEmbedded handler = new HandlerEmbedded();
            bool received = false;
            handler.RegisterHandler("ABCD", delegate(object sender, MessageHandlerCollection.MessageReceivedEventArgs args) { received = true; });
            Message m = new Message("ABCD");
            handler.HandleMessage(m);
            Assert.IsTrue(received);
        }

        [Test]
        public void ComplexMessageHandler()
        {
            HandlerEmbedded first = new HandlerEmbedded();
            HandlerEmbedded second = new HandlerEmbedded(first);

            bool received = false;
            second.RegisterHandler("ABCD", delegate(object sender, MessageHandlerCollection.MessageReceivedEventArgs args) { received = true; });
            Message m = new Message("ABCD");
            first.HandleMessage(m);
            Assert.IsTrue(received);
        }

        [Test]
        public void SingleMessageHandler2()
        {
            HandlerInherited handler = new HandlerInherited();
            bool received = false;
            handler.RegisterHandler("ABCD", delegate(object sender, MessageHandlerCollection.MessageReceivedEventArgs args) { received = true; });
            Message m = new Message("ABCD");
            handler.HandleMessage(m);
            Assert.IsTrue(received);
        }

        [Test]
        public void ComplexMessageHandler2()
        {
            HandlerInherited first = new HandlerInherited();
            HandlerInherited second = new HandlerInherited(first);

            bool received = false;
            second.RegisterHandler("ABCD", delegate(object sender, MessageHandlerCollection.MessageReceivedEventArgs args) { received = true; });
            Message m = new Message("ABCD");
            first.HandleMessage(m);
            Assert.IsTrue(received);
        }

        [Test]
        public void MultipleMessageHandler()
        {
            HandlerEmbedded handler = new HandlerEmbedded();

            bool received1 = false;
            bool received2 = false;
            handler.RegisterHandler("ABCD", delegate(object sender, MessageHandlerCollection.MessageReceivedEventArgs args) { received1 = true; });
            handler.RegisterHandler("ABCD", delegate(object sender, MessageHandlerCollection.MessageReceivedEventArgs args) { received2 = true; });

            Message m = new Message("ABCD");
            handler.HandleMessage(m);

            Assert.IsTrue(received1);
            Assert.IsTrue(received2);
        }
    }
}