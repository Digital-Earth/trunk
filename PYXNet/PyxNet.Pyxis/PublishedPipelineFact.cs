/******************************************************************************
PublishedPipelineFact.cs

begin      : November 30, 2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;
using PyxNet.Service;


namespace PyxNet.Pyxis
{
    public class PublishedPipelineFact : ICertifiableFact
    {
        #region Members

        public ProcRef ProcRef
        { get; set; }

        public string PipelineDefinitionPPL
        { get; set; }

        public string PipelineIdentityXML
        { get; set; }

        public string PipelineIdentityChecksum
        { get; set; }

        public string Name
        { get; set; }
        
        public string Description
        { get; set; }
        
        public string Metadata
        { get; set; }

        #endregion

        #region Geometry
        /// <summary>
        /// This pipeline's geometry.  
        /// </summary>
        private PYXGeometry_SPtr m_geometry = null;

        /// <summary>
        /// Cache the serialized version of the geometry.
        /// </summary>
        private string m_serializedGeometry = null;

        public string SerializedGeometry
        {
            get 
            {               
                if (m_serializedGeometry == null)
                {
                    if (Geometry != null && Geometry.get() != null)
                    {
                        // Cache the serialized geometry to eliminate multiple serializations (optimization)
                        m_serializedGeometry = PYXGeometrySerializer.serialize(Geometry.get());
                    }
                    else
                    {
                        // Store an empty geometry.
                        m_serializedGeometry = "";
                    }
                }
                return m_serializedGeometry; 
            }
            private set
            {
                m_serializedGeometry = value;
                // Clear the geometry cache...
                m_geometry = null;
            }
        }

        /// <summary>
        /// This pipeline's geometry.  
        /// </summary>
        public PYXGeometry_SPtr Geometry
        {
            get 
            { 
                if (((m_geometry == null) || (m_geometry.get() == null)) && (m_serializedGeometry != ""))
                {
                    this.m_geometry = PYXGeometrySerializer.deserialize(m_serializedGeometry);
                }
                return m_geometry; 
            }
            set
            {
                m_geometry = value;
                // clear the cached serialized version of the geometry.
                m_serializedGeometry = null;
            }
        }

        #endregion

        #region ICertifiableFact Members

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
                yield return this.Name;

                foreach (string token in Unique(this.Description.Split(DescriptionTokenSeparators)))
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

        public const string MessageId = "PPLF";

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
            message.Append( this.Name);
            message.Append( this.Description);
            message.Append( this.Metadata);
            message.Append( this.PipelineDefinitionPPL);
            message.Append( this.PipelineIdentityXML);
            message.Append( this.PipelineIdentityChecksum);
            message.Append( this.SerializedGeometry);
            message.Append(this.m_id);

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
                    "Message is not a CoverageRequestMessage message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd( "Extra data in a PublishedPipelineFact message.");
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a CoverageRequestMessage.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            this.Name = reader.ExtractUTF8();
            this.Description = reader.ExtractUTF8();
            this.Metadata = reader.ExtractUTF8();
            this.PipelineDefinitionPPL = reader.ExtractUTF8();
            this.PipelineIdentityXML = reader.ExtractUTF8();
            this.PipelineIdentityChecksum = reader.ExtractUTF8();
            this.SerializedGeometry = reader.ExtractUTF8();
            this.m_id = reader.ExtractGuid();

            if (reader.ExtractBool())
            {
                this.Certificate = new Certificate( reader);
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


        #region IIdentifiable Members

        private Guid m_id = Guid.NewGuid();

        Guid IIdentifiable.Id
        {
            get { return m_id; }
        }

        #endregion
    }
}
