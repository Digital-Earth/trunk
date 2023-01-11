/******************************************************************************
Itransmissible.cs

begin      : July 18, 2007
copyright  : (c) 2007 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;

namespace PyxNet
{
    /// <summary>
    /// Can be converted to/from a message, transmissible on PyxNet.
    /// </summary>
    public interface ITransmissible
    {
        /// <summary>
        /// Build a message.
        /// </summary>
        /// <returns>The resulting message.</returns>
        Message ToMessage();

        /// <summary>
        /// Append to an existing message.
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to (will be modified).</param>
        /// <returns></returns>
        void ToMessage(Message message);

// Each class implementing this interface should containg code like the following.
// The class name is indicated below by T.
// TODO: Either factor this out in a better way (eg. instead of interface implementation,
// possibly derive from a templated base class) or, failing that, maybe create a code snippet 
// that generates this code.
#if false
        #region Convert to/from message

        /// <summary>
        /// Construct from a message reader.
        /// </summary>
        /// <param name="reader">
        /// The message reader to construct from.
        /// This doesn't include the message header.
        /// </param>
        public T(MessageReader reader)
        {
            // TODO: Set member values from those read from 'reader'.
        }

        /// <summary>
        /// Initialize the members from a message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        public static T FromMessage(Message message)
        {
            if (null == message)
            {
                throw new System.ArgumentNullException("message");
            }
            if (message.Identifier != MessageID)
            {
                throw new System.ArgumentException(
                    String.Format("Incorrect message type: {0} instead of {1}.", message.Identifier, MessageID));
            }

            MessageReader reader = new MessageReader(message);
            T result = new T(reader);
            reader.AssertAtEnd( "Extra data in message.");
            return result;
        }

        /// <summary>
        /// Build a message.
        /// </summary>
        /// <returns>The resulting message.</returns>
        Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append to an existing message.
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to (will be modified).</param>
        /// <returns></returns>
        void ToMessage(Message message)
        {
            if (null == message)
            {
                throw new System.ArgumentNullException("message");
            }

            // TODO: Append member values to the message.
        }

        #endregion
#endif
    }

    /// <summary>
    /// A Transmissible object that can be read back.
    /// </summary>
    public interface ITransmissibleWithReader : ITransmissible
    {
        /// <summary>
        /// Initialize the members from a message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        void FromMessage(Message message);

        /// <summary>
        /// Initialize the members from a message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        void FromMessage(MessageReader message);
    }
}
