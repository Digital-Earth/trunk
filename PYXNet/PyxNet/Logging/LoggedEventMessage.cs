using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Logging
{
    /// <summary>
    /// A logged event is a generic representation of an event that
    /// will be sent to the LoggingService.  These logged messages
    /// are always signed, so the sender will be known by the 
    /// recipient (thus sender doesn't need to be included in the
    /// message.)
    /// </summary>
    public class LoggedEventMessage : ITransmissible
    {
        /// <summary>
        /// The Message ID.  (This is a Log-General)
        /// </summary>
        public const string MessageID = "LogG";

        #region Fields and properties

        private DateTime m_timeStamp = DateTime.Now;

        /// <summary>
        /// Gets or sets the time stamp.
        /// </summary>
        /// <value>The time stamp.</value>
        public DateTime TimeStamp
        {
            get { return m_timeStamp; }
            set { m_timeStamp = value; }
        }

        private string m_category = "Unknown";

        /// <summary>
        /// Gets or sets the category, which is an arbitrary string description 
        /// of the event.
        /// </summary>
        /// <value>The category.</value>
        public string Category
        {
            get { return m_category; }
            set { m_category = value; }
        }

        private string m_description;

        public string Description
        {
            get { return m_description; }
            set { m_description = value; }
        }

        #endregion

        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="LoggedEventMessage"/> class.
        /// </summary>
        public LoggedEventMessage(string category, string description)
        {
            this.Category = category;
            this.Description = description;
        }

        /// <summary>
        /// Construct from a message reader.
        /// </summary>
        /// <param name="reader">
        /// The message reader to construct from.
        /// This doesn't include the message header.
        /// </param>
        public LoggedEventMessage(MessageReader reader)
        {
            this.Category = reader.ExtractUTF8();
            this.Description = reader.ExtractUTF8();
            this.TimeStamp = DateTime.FromBinary(reader.ExtractInt64());
        }

        #endregion

        #region Convert to/from message

        /// <summary>
        /// Build a message that contains the object.
        /// </summary>
        /// <returns>The resulting message.</returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the object to an existing message.
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to (will be modified).</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            if (null == message)
            {
                throw new System.ArgumentNullException("message");
            }

            message.Append(this.Category);
            message.Append(this.Description);
            message.Append(this.TimeStamp.ToBinary());
        }

        /// <summary>
        /// Initialize the members from a message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        public static LoggedEventMessage FromMessage(Message message)
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
            LoggedEventMessage result = new LoggedEventMessage(reader);
            reader.AssertAtEnd( "Extra data in message.");
            return result;
        }

        #endregion
    }
}
