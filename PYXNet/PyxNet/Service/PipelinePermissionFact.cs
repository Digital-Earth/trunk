/******************************************************************************
PipelinePermissionFact.cs (PyxNet.Service)

begin      : April 21, 2010
copyright  : (c) 2010 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;


namespace PyxNet.Service
{
    /// <summary>
    /// A node has been authorized to access a specific pipeline.
    /// </summary>
    public class PipelinePermissionFact : ICertifiableFact
    {
        #region Members

        public int ProcessVersion
        { get; set; }

        public NodeId AuthorizedNode
        { get; set; }

        #endregion

        public PipelinePermissionFact()
        {
        }

        public PipelinePermissionFact( Message message)
        {
            FromMessage(message);
        }

        /// <summary>
        /// Initializes the <see cref="PipelinePermissionFact"/> class, by registering its type.
        /// </summary>
        static PipelinePermissionFact()
        {
            RegisterType();
        }

        /// <summary>
        /// Registers the type.
        /// </summary>
        public static void RegisterType()
        {
            Certificate.KnownTypes.RegisterType(PipelinePermissionFact.MessageId, typeof(PipelinePermissionFact));
        }
        #region ICertifiableFact Members

        private Guid m_id = Guid.NewGuid();

        /// <summary>
        /// Gets the underlying pipeline's id.
        /// </summary>
        /// <value>The id.</value>
        public Guid Id
        {
            get { return m_id; }
            set { m_id = value; }
        }

        public Certificate Certificate
        {
            get;
            set;
        }

        private static IEnumerable<string> Unique(string[] values)
        {
            List<string> temporaryList = new List<string>(values);
            temporaryList.Sort();

            string previousResult = "";
            foreach (string item in temporaryList)
            {
                if (item != previousResult)
                {
                    yield return item;
                }
                previousResult = item;
            }
        }

        private static char[] DescriptionTokenSeparators = { ' ', '.', ',', ':' };
        public IEnumerable<string> Keywords
        {
            get
            {
                yield return string.Format("PPF:{0}-{1}[{2}]",
                    this.AuthorizedNode.Identity, this.Id, this.ProcessVersion);

                // Enable this section if we want to be able to find all licenses for a dataset.
//                yield return string.Format("PPF:{0}[{1}]",
//                    this.Id, this.ProcessVersion);
            }
        }

        public String UniqueKeyword
        {
            get
            {
                foreach (string keyword in Keywords)
                {
                    return keyword;
                }
                return null;
            }
        }

        #endregion

        #region ITransmissibleWithReader Members

        public const string MessageId = "PPer";

        /// <summary>
        /// Build a PyxNet message that contains "this".
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageId);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append "this" to an existing message.  
        /// Does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            message.Append(this.m_id);
            message.Append(this.ProcessVersion);
            this.AuthorizedNode.ToMessage(message);
        }

        /// <summary>
        /// Initialize the members from a PyxNet message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        public void FromMessage(Message message)
        {
            if (message == null || !message.StartsWith(MessageId))
            {
                throw new System.ArgumentException(
                    "Message is not a PipelinePermissionFact message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a CoverageRequestMessage.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            this.m_id = reader.ExtractGuid();
            this.ProcessVersion = reader.ExtractInt();
            this.AuthorizedNode = new NodeId(reader);
        }

        public void FromMessage(MessageReader messageReader)
        {
            FromMessageReader(messageReader);
        }

        #endregion
    }
}
