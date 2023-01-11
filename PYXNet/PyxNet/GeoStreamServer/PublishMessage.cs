/******************************************************************************
PublishMessage.cs

begin      : February 8, 2010
copyright  : (c) 2010 by the PYXIS innovation inc.
web        : www.pyxisinnovation.com
******************************************************************************/

using System;
using System.Collections.Generic;
using System.Text;

namespace PyxNet.GeoStreamServer
{
    /// <summary>
    /// Message sent from License Server to GeoStream server 
    /// to publish data set on GeoStream server.  Message 
    /// includes enough information for GeoStream server to 
    /// start publishing the data set.  Key component is the
    /// pipeline definition.
    /// </summary>
    public class PublishMessage : GeoStreamServerMessage
    {
        public const string MessageID = "GSpm";

        public override string GetMsgID()
        {
            return MessageID;
        }

        public override string GetMsgName()
        {
            return "GeoStream-Publish";
        }

        public string Name
        { get; set; }

        public string Description
        { get; set; }

        public Guid DataSetId
        { get; set; }

        public int Version
        { get; set; }

        public string PipelineDefinition
        { get; set; }

        #region Convert to/from message format

        /// <summary>
        /// Append the publication data to an existing message.  
        /// This does not include the message header.
        /// </summary>
        /// <param name="message">The message to append to.(will be modified)</param>
        /// <returns></returns>
        public override void ToMessage(Message message)
        {
            base.ToMessage(message);

            message.Append(Name);
            message.Append(Description);
            message.Append(DataSetId);
            message.Append(Version);
            message.Append(PipelineDefinition);
        }

        /// <summary>
        /// Initialize the members from a message reader.  The message reader
        /// should be properly set to point at the start of message.
        /// </summary>
        /// <param name="reader">The message reader to read from.</param>
        public override void FromMessageReader(MessageReader reader)
        {
            base.FromMessageReader(reader);

            Name = reader.ExtractUTF8();
            Description = reader.ExtractUTF8();
            DataSetId = reader.ExtractGuid();
            Version = reader.ExtractInt();
            PipelineDefinition = reader.ExtractUTF8();
        }

        #endregion
    }
}
