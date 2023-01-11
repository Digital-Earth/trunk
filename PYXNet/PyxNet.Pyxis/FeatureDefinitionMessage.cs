/******************************************************************************
FeatureDefinitionMessage.cs

begin      : 26/08/2009
copyright  : (c) 2009 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/
using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.Pyxis
{
    /// <summary>
    /// This message is used as embedded extra information into the query
    /// messages to identify the published IProcess.
    /// </summary>
    public class FeatureDefinitionMessage : ITransmissible
    {
        public const string MessageID = "FDef";

        #region Properties

        /// <summary>
        /// Storage for the Proc Ref.
        /// </summary>
        private ProcRef m_procRef;

        /// <summary>
        /// The Process Reference for this Feature.
        /// </summary>
        public ProcRef ProcRef
        {
            get { return m_procRef; }
            set { m_procRef = value; }
        }

        /// <summary>
        /// Storage for the pipeline definition of this feature.
        /// </summary>
        private string m_pipelineDefinition;

        /// <summary>
        /// The pipeline definition of this feature.
        /// </summary>
        public string PipelineDefinition
        {
            get { return m_pipelineDefinition; }
            set { m_pipelineDefinition = value; }
        }

        /// <summary>
        /// Storage for the geometry for this feature.
        /// </summary>
        private PYXGeometry_SPtr m_geometry = null;

        /// <summary>
        /// Cache the serialized version of the geometry.
        /// </summary>
        private string m_serializedGeometry = null;

        /// <summary>
        /// The geometry for this feature.
        /// </summary>
        public PYXGeometry_SPtr Geometry
        {
            get { return m_geometry; }
            set
            {
                m_geometry = value;
                // clear the cached serialized version of the geometry.
                m_serializedGeometry = null;
            }
        }

         private PyxNet.Service.Certificate m_certificate;

        /// <summary>
        /// Gets or sets the certificate.
        /// </summary>
        /// <value>The certificate.</value>
        public PyxNet.Service.Certificate Certificate
        {
            get { return m_certificate; }
            set { m_certificate = value; }
        }

        #endregion

        #region Construction

        /// <summary>
        /// Default Constructor
        /// </summary>
        public FeatureDefinitionMessage()
        {
        }

        /// <summary>
        /// Construct a FeatureDefinitionMessage from a message.  The message must be a 
        /// PyxNet FeatureDefinitionMessage.
        /// </summary>
        /// <param name="message"></param>
        public FeatureDefinitionMessage(Message message)
        {
            FromMessage(message);
        }

        #endregion

        #region Convert to/from message format
        /// <summary>
        /// Build a PyxNet message that contains the FeatureDefinitionMessage.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the FeatureDefinitionMessage to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            message.Append(ProcRef.getProcID());
            message.Append(ProcRef.getProcVersion());

            message.Append(PipelineDefinition);
            if (Geometry != null && Geometry.get() != null)
            {
                message.Append(true);
                // Cache the serialized geometry to eliminate multiple serializations (optimization)
                if (m_serializedGeometry == null)
                {
                    m_serializedGeometry = PYXGeometrySerializer.serialize(Geometry.get());
                }
                message.Append(m_serializedGeometry);
            }
            else
            {
                message.Append(false);
            }
            message.Append((bool)(Certificate != null));
            if (Certificate != null)
            {
                Certificate.ToMessage(message);
            }
        }

        /// <summary>
        /// Initialize the members from a PyxNet message and verify message type.
        /// </summary>
        /// <param name="message">The message to read from.</param>
        public void FromMessage(Message message)
        {
            if (message == null || !message.StartsWith(MessageID))
            {
                throw new System.ArgumentException(
                    "Message is not a FeatureDefinitionMessage message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd( "Extra data in a FeatureDefinitionMessage message.");
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a FeatureDefinitionMessage.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            ProcRef = new ProcRef(pyxlib.strToGuid(reader.ExtractGuid().ToString()), reader.ExtractInt());

            PipelineDefinition = reader.ExtractUTF8();
            bool hasGeometry = reader.ExtractBool();
            Geometry = null;
            if (hasGeometry)
            {
                m_serializedGeometry = reader.ExtractUTF8();
                Geometry = PYXGeometrySerializer.deserialize(m_serializedGeometry);
            }
            Certificate = null;
            bool hasCertificate = reader.ExtractBool();
            if (hasCertificate)
            {
                Certificate = new PyxNet.Service.Certificate(reader);
            }
        }
        #endregion
   }
}
