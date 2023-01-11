/******************************************************************************
PublishedPipelineFact.cs (PyxNet.Service)

begin      : December 17, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

//--
//-- this fact is a simplified version of the one found in PyxNet.Pyxis
//-- unfortunately, having facts defined outside of the base PyxNet
//-- assembly cause grief with reflection services
//--
//-- facts type are known and registered inside the certificate code.
//--

using System;
using System.Collections.Generic;
using System.Text;
//using PyxNet.Service;


namespace PyxNet.Service
{
    /// <summary>
    /// Notes: Need to get rid of RubberStamp and TransactionId.  Need to add PipelineGuid?
    /// </summary>
    public class PublishedPipelineFact : ICertifiableFact
    {
        #region Members
  
        public string Name
        { get; set; }

        public string Description
        { get; set; }

        public string Metadata
        { get; set; }

        public string PipelineDefinitionPPL
        { get; set; }

        public string PipelineIdentityXML
        { get; set; }

        public string PipelineIdentityChecksum
        { get; set; }

        public int ProcessVersion
        { get; set; }

        public bool RubberStamp
        { get; set; }

        public Guid TransactionId
        { get; set; }

        #endregion

        public PublishedPipelineFact()
        {
            RubberStamp = false;
            TransactionId = Guid.Empty;
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
            get; set;
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
                if (this.PipelineIdentityXML == null)
                {
                    System.Diagnostics.Trace.WriteLine("Warning! Pipeline Identity XML is not set.");
                }

                yield return this.PipelineIdentityXML;

                yield return this.Name;

                foreach( string token in Unique( this.Description.Split(DescriptionTokenSeparators)))
                {
                    yield return token;
                }

                // TODO: Consider adding Metadata to our keywords.
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

        public const string MessageId = "nnPP";

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
            message.Append(this.Name);
            message.Append(this.Description);
            message.Append(this.Metadata);

            message.Append( this.PipelineDefinitionPPL);
            message.Append( this.PipelineIdentityXML);
            message.Append( this.PipelineIdentityChecksum);
          
            //message.Append( this.SerializedGeometry);

            message.Append(this.RubberStamp);
            message.Append(this.TransactionId);
            message.Append(this.ProcessVersion);

            message.Append((bool)(this.Certificate != null));
            if (this.Certificate != null)
            {
                this.Certificate.ToMessage(message);
            }
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
                    "Message is not a PublishedPipelineFact message.");
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
            this.Name = reader.ExtractUTF8();
            this.Description = reader.ExtractUTF8();
            this.Metadata = reader.ExtractUTF8();

            this.PipelineDefinitionPPL = reader.ExtractUTF8();
            this.PipelineIdentityXML = reader.ExtractUTF8();
            this.PipelineIdentityChecksum = reader.ExtractUTF8();
          
            //this.SerializedGeometry = reader.ExtractUTF8();

            this.RubberStamp = reader.ExtractBool();
            this.TransactionId = reader.ExtractGuid();
            this.ProcessVersion = reader.ExtractInt();

            if (reader.ExtractBool())
            {
                this.Certificate = new Certificate(reader);
            }
            else
            {
                this.Certificate = null;
            }
        }

        public void FromMessage(MessageReader messageReader)
        {
            FromMessageReader(messageReader);
        }

        #endregion
    }
}
