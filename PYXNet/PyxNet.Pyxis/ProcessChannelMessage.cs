/******************************************************************************
CoverageRequestMessage.cs

begin      : July 14, 2012
copyright  : (c) 2012 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;

namespace PyxNet.Pyxis
{

    public class ProcessChannelInfoMessage : ITransmissible
    {
        public const string MessageID = "PChI";

        #region Properties

        /// <summary>
        /// The version of the process that we are publishing.
        /// </summary>
        public int ProcessVersion { get; set; }

        /// <summary>
        /// list of all avilalbe channels we are publishing.
        /// </summary>
        public List<string> DataChannels { get; set; }

        /// <summary>
        /// The pipeline definition of this process.
        /// </summary>
        public string PipelineDefinition { get; set; }

        /// <summary>
        /// Storage for the geometry that this coverage request is interested in.
        /// </summary>
        private PYXGeometry_SPtr m_geometry = null;

        /// <summary>
        /// Cache the serialized version of the geometry.
        /// </summary>
        private string m_serializedGeometry = null;

        /// <summary>
        /// The geometry that this coverage request is interested in.
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

        //TODO: need to understand how this certificates get into play.
        //the current CoverageMessage has them - but no one ever use them.

        /*
        /// <summary>
        /// Gets or sets the usage certificate.
        /// </summary>
        /// <value>The certificate.</value>
        public PyxNet.Service.Certificate UsageCertificate
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the Published certificate.
        /// </summary>
        /// <value>The certificate.</value>
        public PyxNet.Service.Certificate PublishedCertificate
        {
            get;
            set;
        }
        */

        

        #endregion

        #region Construction

        /// <summary>
        /// Default Constructor
        /// </summary>
        public ProcessChannelInfoMessage()
        {
            DataChannels = new List<string>();
        }

        /// <summary>
        /// Construct a CoverageRequestMessage from a message.  The message must be a 
        /// PyxNet TileRequestMessage.
        /// </summary>
        /// <param name="message"></param>
        public ProcessChannelInfoMessage(Message message)
        {
            FromMessage(message);
        }

        #endregion

        #region Convert to/from message format
        /// <summary>
        /// Build a PyxNet message that contains the CoverageRequestMessage.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the CoverageRequestMessage to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            message.Append(ProcessVersion);
            message.Append(PipelineDefinition);

            if (Geometry != null && Geometry.get() != null)
            {
                // Cache the serialized geometry to eliminate multiple serializations (optimization)
                if (m_serializedGeometry == null)
                {
                    try
                    {
                        m_serializedGeometry = PYXGeometrySerializer.serialize(Geometry.get());
                    }
                    catch (Exception)
                    {
                        // Eat any exceptions here - infer that this is an invalid geometry.
                    }
                }
            }
            if (m_serializedGeometry != null)
            {
                message.Append(true);
                message.Append(m_serializedGeometry);
            }
            else
            {
                message.Append(false);
            }

            message.Append(DataChannels.Count);

            foreach(var code in DataChannels)
            {
                message.Append(code);
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
                    "Message is not a ProcessChannelMessage message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd( "Extra data in a ProcessChannelMessage message.");
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a CoverageRequestMessage.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            ProcessVersion = reader.ExtractInt();
            PipelineDefinition = reader.ExtractUTF8();

            bool hasGeometry = reader.ExtractBool();
            Geometry = null;
            if (hasGeometry)
            {
                m_serializedGeometry = reader.ExtractUTF8();
                Geometry = PYXGeometrySerializer.deserialize(m_serializedGeometry);
            }

            int count = reader.ExtractInt();
            DataChannels = new List<string>();
            for(var i=0;i<count;++i)
            {
                DataChannels.Add(reader.ExtractUTF8());
            }
        }
        #endregion
    }

    public class ProcessChannelRequestMessage : ITransmissible
    {
        public const string MessageID = "PChR";

        #region Properties

        /// <summary>
        /// The version of the process that we are publishing.
        /// </summary>
        public int ProcessVersion { get; set; }

        /// <summary>
        /// The channel code we are publishing.
        /// </summary>
        public string DataCode { get; set; }

        /// <summary>
        /// The requested data key
        /// </summary>
        public string RequestedKey { get; set; }

        #endregion

        #region Construction

        /// <summary>
        /// Default Constructor
        /// </summary>
        public ProcessChannelRequestMessage()
        {
        }

        /// <summary>
        /// Construct a CoverageRequestMessage from a message.  The message must be a 
        /// PyxNet TileRequestMessage.
        /// </summary>
        /// <param name="message"></param>
        public ProcessChannelRequestMessage(Message message)
        {
            FromMessage(message);
        }

        #endregion

        #region Convert to/from message format
        /// <summary>
        /// Build a PyxNet message that contains the CoverageRequestMessage.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the CoverageRequestMessage to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            message.Append(ProcessVersion);
            message.Append(DataCode);
            message.Append(RequestedKey);
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
                    "Message is not a ProcessChannelMessage message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd( "Extra data in a ProcessChannelMessage message.");
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a CoverageRequestMessage.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {            
            ProcessVersion = reader.ExtractInt();
            DataCode = reader.ExtractUTF8();
            RequestedKey = reader.ExtractUTF8();
        }
        #endregion
    }

    public class ProcessChannelMultiKeyRequestMessage : ITransmissible
    {
        public const string MessageID = "PCKR";

        #region Properties

        /// <summary>
        /// The version of the process that we are publishing.
        /// </summary>
        public int ProcessVersion { get; set; }

        /// <summary>
        /// The channel code we are publishing.
        /// </summary>
        public string DataCode { get; set; }

        /// <summary>
        /// The requested data key
        /// </summary>
        public List<string> RequestedKeys { get; set; }

        #endregion

        #region Construction

        /// <summary>
        /// Default Constructor
        /// </summary>
        public ProcessChannelMultiKeyRequestMessage()
        {
        }

        /// <summary>
        /// Construct a CoverageRequestMessage from a message.  The message must be a 
        /// PyxNet TileRequestMessage.
        /// </summary>
        /// <param name="message"></param>
        public ProcessChannelMultiKeyRequestMessage(Message message)
        {
            FromMessage(message);
        }

        #endregion

        #region Convert to/from message format
        /// <summary>
        /// Build a PyxNet message that contains the CoverageRequestMessage.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the CoverageRequestMessage to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            message.Append(ProcessVersion);
            message.Append(DataCode);
            message.Append(RequestedKeys.Count);
            foreach(var key in RequestedKeys)
            {
                message.Append(key);
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
                    "Message is not a ProcessChannelMessage message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd("Extra data in a ProcessChannelMessage message.");
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a CoverageRequestMessage.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            ProcessVersion = reader.ExtractInt();
            DataCode = reader.ExtractUTF8();
            int count = reader.ExtractInt();
            RequestedKeys = new List<string>();
            for (var i = 0; i < count; ++i )
            {
                RequestedKeys.Add(reader.ExtractUTF8());
            }
        }
        #endregion
    }

    public class ProcessChannelMultiKeyInfoMessage : ITransmissible
    {
        public class KeyInfo
        {
            public string Key { get; set; }
            public bool Found { get; set; }
            public int Length { get; set; }

            public KeyInfo(string key)
            {
                Key = key;
                Found = false;
                Length = 0;
            }

            public KeyInfo(string key,int length)
            {
                Key = key;
                Found = true;
                Length = length;
            }

            public override string ToString()
            {
                if (Found)
                {
                    return Key + " [" + Length + "]";
                }
                else
                {
                    return Key + " [Not Found]";
                }
            }
        }

        public const string MessageID = "PCKI";

        #region Properties

        /// <summary>
        /// The version of the process that we are publishing.
        /// </summary>
        public int ProcessVersion { get; set; }

        /// <summary>
        /// The channel code we are publishing.
        /// </summary>
        public string DataCode { get; set; }

        /// <summary>
        /// The requested data key info
        /// </summary>
        public List<KeyInfo> Keys { get; set; }

        //TODO: need to understand how this certificates get into play.
        //the current CoverageMessage has them - but no one ever use them.

        /*
        /// <summary>
        /// Gets or sets the usage certificate.
        /// </summary>
        /// <value>The certificate.</value>
        public PyxNet.Service.Certificate UsageCertificate
        {
            get;
            set;
        }

        /// <summary>
        /// Gets or sets the Published certificate.
        /// </summary>
        /// <value>The certificate.</value>
        public PyxNet.Service.Certificate PublishedCertificate
        {
            get;
            set;
        }
        */


        #endregion

        #region Construction

        /// <summary>
        /// Default Constructor
        /// </summary>
        public ProcessChannelMultiKeyInfoMessage()
        {
        }

        /// <summary>
        /// Construct a CoverageRequestMessage from a message.  The message must be a 
        /// PyxNet TileRequestMessage.
        /// </summary>
        /// <param name="message"></param>
        public ProcessChannelMultiKeyInfoMessage(Message message)
        {
            FromMessage(message);
        }

        #endregion

        #region Convert to/from message format
        /// <summary>
        /// Build a PyxNet message that contains the CoverageRequestMessage.
        /// </summary>
        /// <returns></returns>
        public Message ToMessage()
        {
            Message message = new Message(MessageID);
            ToMessage(message);
            return message;
        }

        /// <summary>
        /// Append the CoverageRequestMessage to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.  (will be modified)</param>
        /// <returns></returns>
        public void ToMessage(Message message)
        {
            message.Append(ProcessVersion);
            message.Append(DataCode);
            message.Append(Keys.Count);
            foreach (var keyInfo in Keys)
            {
                message.Append(keyInfo.Key);
                message.Append(keyInfo.Found);
                if (keyInfo.Found)
                {
                    message.Append(keyInfo.Length);
                }
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
                    "Message is not a ProcessChannelMessage message.");
            }

            MessageReader reader = new MessageReader(message);
            FromMessageReader(reader);
            reader.AssertAtEnd("Extra data in a ProcessChannelMessage message.");
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of a CoverageRequestMessage.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public void FromMessageReader(MessageReader reader)
        {
            ProcessVersion = reader.ExtractInt();
            DataCode = reader.ExtractUTF8();
            int count = reader.ExtractInt();
            Keys = new List<KeyInfo>();
            for (var i = 0; i < count; ++i)
            {
                var info = new KeyInfo(reader.ExtractUTF8());
                info.Found = reader.ExtractBool();
                if (info.Found)
                {
                    info.Length = reader.ExtractInt();
                }
                else
                {
                    info.Length = 0;
                }
                Keys.Add(info);
            }
        }
        #endregion
    }
}